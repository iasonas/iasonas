//Iasonas
#include "TRouter2.h"

//Receive Process
void TRouter2::rxProcess()
{
  if(reset.read())
    {
      // Clear outputs and indexes of receiving protocol
      for(int i=0; i<DIRECTIONS_2; i++)	{
		  ack_rx[i].write(0);
		  current_level_rx[i] = 0;
		}
    reservation_table.clear();
    routed_flits = 0;
    local_drained = 0;
  }
  else {
      // For each channel decide if a new flit can be accepted
      // This process simply sees a flow of incoming flits. All arbitration
      // and wormhole related issues are addressed in the txProcess()
      for(int i=0; i<DIRECTIONS_2; i++) {
		// To accept a new flit, the following conditions must match:
		// 1) there is an incoming request
		// 2) there is a free slot in the input buffer of direction i
		  if ( (req_rx[i].read()==!(current_level_rx[i])) && !buffer[i].IsFull() )
			{
			  TFlit received_flit = flit_rx[i].read();
			  if(TGlobalParams::verbose_mode > VERBOSE_OFF) 
				cout << sc_time_stamp().to_double()/1000 << ": BIG ROUTER" <<", Input[" << i <<"], Received flit: " << received_flit << endl ; 
			  // Store the incoming flit in the circular buffer
			  buffer[i].Push(received_flit);            
			  // Negate the old value for Alternating Bit Protocol (ABP)
			  //current_level_rx[i] = 1-current_level_rx[i];
			  current_level_rx[i] = !(current_level_rx[i]);
			  // Incoming flit
			  stats.power.Incoming();
			}
		  ack_rx[i].write(current_level_rx[i]);
	  }
    }
  stats.power.Standby();
}
//Transmit
void TRouter2::txProcess()
{
  if (reset.read()) {
      // Clear outputs and indexes of transmitting protocol
      for(int i=0; i<DIRECTIONS_2; i++) {
		req_tx[i].write(0);
		current_level_tx[i] = 0;
	  }
  }
  else
    {
      // 1st phase: Search for flit waiting for transmission & Reservation of the Output
      for(int j=0; j<DIRECTIONS_2; j++)	{
			int i = (start_from_port + j) % (DIRECTIONS_2);
			//If there is smth in the buffer waiting
			if ( !buffer[i].IsEmpty() ) {
				TFlit flit = buffer[i].Front();
				if (flit.flit_type==FLIT_TYPE_HEAD) {
				  
				  int o = flit.dst_id ;
				  if(TGlobalParams::verbose_mode > VERBOSE_OFF) 
				  cout << "\033[22;34mTo direction pou antistoixei sto o einai: \033[22;30m "<< o << "\tme local:" << local_id 
					   <<"\t source:"<< flit.src_id << "\t destination "<< flit.dst_id << "\t direction"<<i<< endl;

				  if (reservation_table.isAvailable(o)) {
					  reservation_table.reserve(i, o);
					  if(TGlobalParams::verbose_mode > VERBOSE_OFF)
						cout << sc_time_stamp().to_double()/1000 << ": BIG ROUTER" << ", Input[" << i << "] (" << buffer[i].Size() << " flits)" 
						   << ", reserved Output[" << o << "], flit: " << flit << endl;		      
				 }
			   }
	    }
	}
      start_from_port++;

      // 2nd phase: Forwarding
      for(int i=0; i<DIRECTIONS_2; i++) {
	  	if ( !buffer[i].IsEmpty() )
	    {
	      TFlit flit = buffer[i].Front();

	      int o = reservation_table.getOutputPort(i);
	      if (o != NOT_RESERVED) {
		  	if ( current_level_tx[o] == ack_tx[o].read() )
		    {
            if(TGlobalParams::verbose_mode > VERBOSE_OFF)
				  cout << sc_time_stamp().to_double()/1000<< ": BIG ROUTER" << ", Input[" << i 
				  		<< "] forward to Output[" << o << "], flit: " << flit << endl;
			flit_tx[o].write(flit);
		    current_level_tx[o] = 1 - current_level_tx[o];
		    req_tx[o].write(current_level_tx[o]);
		    buffer[i].Pop();

		    stats.power.Forward();

		    if (flit.flit_type == FLIT_TYPE_TAIL) 
				reservation_table.release(o);
			// Update stats
		    if (o == DIRECTION_LOCAL) {
			  stats.receivedFlit(sc_time_stamp().to_double()/1000, flit);
			  if (TGlobalParams::max_volume_to_be_drained)
			  {
			      if (drained_volume >= TGlobalParams::max_volume_to_be_drained)
						sc_stop();
			      else
			      {
					drained_volume++;
					local_drained++;
			      }
			    }
			}
			//Iasonas***I count flits routed to local PE
		    //else if (i != DIRECTION_LOCAL)
				{
			  	// Increment routed flits counter
			  	routed_flits++;
				}
		    }
		}
	    }
	}
    } // else
  stats.power.Standby();
}	// tx process

//---------------------------------------------------------------------------

TNoP_data TRouter2::getCurrentNoPData() const 
{
    TNoP_data NoP_data;

    for (int j=0; j<DIRECTIONS_2; j++)
    {
	NoP_data.channel_status_neighbor[j].free_slots = free_slots_neighbor[j].read();
	NoP_data.channel_status_neighbor[j].available = (reservation_table.isAvailable(j));
    }

    NoP_data.sender_id = local_id;

    return NoP_data;
}

//---------------------------------------------------------------------------

void TRouter2::bufferMonitor()
{
  if (reset.read())
  {
    for (int i=0; i<DIRECTIONS_2; i++) free_slots[i].write(buffer[i].GetMaxBufferSize());
  }
  else
  {

    if (TGlobalParams::selection_strategy==SEL_BUFFER_LEVEL ||
	TGlobalParams::selection_strategy==SEL_NOP)
    {

      // update current input buffers level to neighbors
      for (int i=0; i<DIRECTIONS+1; i++)
	free_slots[i].write(buffer[i].getCurrentFreeSlots());

      // NoP selection: send neighbor info to each direction 'i'
      TNoP_data current_NoP_data = getCurrentNoPData();

      for (int i=0; i<DIRECTIONS; i++)
	NoP_data_out[i].write(current_NoP_data);

      if (TGlobalParams::verbose_mode == -57) 
	  NoP_report();
    }
  }
}

//---------------------------------------------------------------------------

vector<int> TRouter2::routingFunction(const TRouteData& route_data) 
{
  TCoord position  = id2Coord(route_data.current_id);
  TCoord src_coord = id2Coord(route_data.src_id);
  TCoord dst_coord = id2Coord(route_data.dst_id);
  int dir_in = route_data.dir_in;

  return routingXY(position, dst_coord);

}

//---------------------------------------------------------------------------

int TRouter2::route(const TRouteData& route_data)
{
  stats.power.Routing();
  //if (route_data.dst_id == local_id)
  //  return DIRECTION_LOCAL;

  vector<int> candidate_channels = routingFunction(route_data);

  return selectionFunction(candidate_channels,route_data);
}

//---------------------------------------------------------------------------

void TRouter2::NoP_report() const
{
    TNoP_data NoP_tmp;
      cout << sc_time_stamp().to_double()/1000 << "BIG ROUTER" << local_id << " NoP report: " << endl;

      for (int i=0;i<DIRECTIONS; i++) 
      {
	  NoP_tmp = NoP_data_in[i].read();
	  if (NoP_tmp.sender_id!=NOT_VALID)
	    cout << NoP_tmp;
      }
}
//---------------------------------------------------------------------------

int TRouter2::NoPScore(const TNoP_data& nop_data, const vector<int>& nop_channels) const
{
    int score = 0;

    if (TGlobalParams::verbose_mode==-58)
    {
	cout << nop_data;
	cout << "      On-Path channels: " << endl;
    }

    for (unsigned int i=0;i<nop_channels.size();i++)
    {
	int available;

	if (nop_data.channel_status_neighbor[nop_channels[i]].available)
	    available = 1; 
	else available = 0;

	int free_slots = nop_data.channel_status_neighbor[nop_channels[i]].free_slots;

	if (TGlobalParams::verbose_mode==-58)
	{
	    cout << "       channel " << nop_channels[i] << " -> score: ";
	    cout << " + " << available << " * (" << free_slots << ")" << endl;
	}

	score += available*free_slots;
    }

    return score;
}
//---------------------------------------------------------------------------

int TRouter2::selectionNoP(const vector<int>& directions, const TRouteData& route_data)
{
  vector<int> neighbors_on_path;
  vector<int> score;
  int direction_selected = NOT_VALID;

  int current_id = route_data.current_id;

  if (TGlobalParams::verbose_mode==-58)
  {
    cout << endl;
    cout << sc_time_stamp().to_double()/1000 << ": BIG Router" << local_id << " NoP SELECTION ----------------" << endl;
    cout << "      Packet: " << route_data.src_id << " --> " << route_data.dst_id << endl;
  }

  for (uint i=0; i<directions.size(); i++)
  {
    // get id of adjacent candidate
    int candidate_id = getNeighborId(current_id,directions[i]);

  // apply routing function to the adjacent candidate node
    TRouteData tmp_route_data;
    tmp_route_data.current_id = candidate_id;
    tmp_route_data.src_id = route_data.src_id;
    tmp_route_data.dst_id = route_data.dst_id;
    tmp_route_data.dir_in = reflexDirection(directions[i]);

    if (TGlobalParams::verbose_mode==-58)
    {
	cout << "\n    -> Adjacent candidate: " << candidate_id << " (direction " << directions[i] << ")" << endl;
    }

    vector<int> next_candidate_channels = routingFunction(tmp_route_data);

    // select useful data from Neighbor-on-Path input 
    TNoP_data nop_tmp = NoP_data_in[directions[i]].read();

    // store the score of node in the direction[i]
    score.push_back(NoPScore(nop_tmp,next_candidate_channels));
  }

  // check for direction with higher score
  int max_direction = directions[0];
  int max = score[0];
  for (unsigned int i = 0;i<directions.size();i++)
  {
      if (score[i]>max)
      {
	  max_direction = directions[i];
	  max = score[i];
      }
  }

  // if multiple direction have the same score = max, choose randomly.
  
  vector<int> equivalent_directions;

  for (unsigned int i = 0;i<directions.size();i++)
      if (score[i]==max)
	  equivalent_directions.push_back(directions[i]);

  direction_selected =  equivalent_directions[rand() % equivalent_directions.size()]; 

  if (TGlobalParams::verbose_mode==-58)
  {
      if (equivalent_directions.size()>1)
      {
	  cout << "\n    equivalent directions found! : ";
	  for (unsigned int i =0;i<equivalent_directions.size();i++)
	      cout  << " " << equivalent_directions[i];
      }
      cout << "\n CHOICE: node " << getNeighborId(current_id,direction_selected) << " (direction " << direction_selected << ")" << endl;
  }
  return direction_selected; 
}

//---------------------------------------------------------------------------

int TRouter2::selectionBufferLevel(const vector<int>& directions)
{
  vector<int>  best_dirs;
  int          max_free_slots = 0;
  for (unsigned int i=0; i<directions.size(); i++)
    {
      int free_slots = free_slots_neighbor[directions[i]].read();
      bool available = reservation_table.isAvailable(directions[i]);
      if (available)
	{
	  if (free_slots > max_free_slots) 
	    {
	      max_free_slots = free_slots;
	      best_dirs.clear();
	      best_dirs.push_back(directions[i]);
	    }
	  else if (free_slots == max_free_slots)
	    best_dirs.push_back(directions[i]);
	}
    }

  if (best_dirs.size())
    return(best_dirs[rand() % best_dirs.size()]);
  else
    return(directions[rand() % directions.size()]);

  //-------------------------
  // TODO: unfair if multiple directions have same buffer level
  // TODO: to check when both available
//   unsigned int max_free_slots = 0;
//   int direction_choosen = NOT_VALID;

//   for (unsigned int i=0;i<directions.size();i++)
//     {
//       int free_slots = free_slots_neighbor[directions[i]].read();
//       if ((free_slots >= max_free_slots) &&
// 	  (reservation_table.isAvailable(directions[i])))
// 	{
// 	  direction_choosen = directions[i];
// 	  max_free_slots = free_slots;
// 	}
//     }

//   // No available channel 
//   if (direction_choosen==NOT_VALID)
//     direction_choosen = directions[rand() % directions.size()]; 

//   if(TGlobalParams::verbose_mode>VERBOSE_OFF)
//     {
//       TChannelStatus tmp;

//       cout << sc_time_stamp().to_double()/1000 << ": Router[" << local_id << "] SELECTION between: " << endl;
//       for (unsigned int i=0;i<directions.size();i++)
// 	{
// 	  tmp.free_slots = free_slots_neighbor[directions[i]].read();
// 	  tmp.available = (reservation_table.isAvailable(directions[i]));
// 	  cout << "    -> direction " << directions[i] << ", channel status: " << tmp << endl;
// 	}
//       cout << " direction choosen: " << direction_choosen << endl;
//     }

//   assert(direction_choosen>=0);
//   return direction_choosen;
}

//---------------------------------------------------------------------------

int TRouter2::selectionRandom(const vector<int>& directions)
{
  return directions[rand() % directions.size()]; 
}

//---------------------------------------------------------------------------

int TRouter2::selectionFunction(const vector<int>& directions, const TRouteData& route_data)
{
  // not so elegant but fast escape ;)
  if (directions.size()==1) return directions[0];
  
  stats.power.Selection();
  switch (TGlobalParams::selection_strategy)
    {
    case SEL_RANDOM:
      return selectionRandom(directions);
    case SEL_BUFFER_LEVEL:
      return selectionBufferLevel(directions);
    case SEL_NOP:
      return selectionNoP(directions,route_data);
    default:
      assert(false);
    }
  
  return 0;	    
}

//---------------------------------------------------------------------------

vector<int> TRouter2::routingXY(const TCoord& current, const TCoord& destination)
{
  vector<int> directions;
  
  
  if (destination.x > current.x)
    directions.push_back(DIRECTION_EAST);
  else if (destination.x < current.x)
    directions.push_back(DIRECTION_WEST);
  else if (destination.y > current.y)
    directions.push_back(DIRECTION_SOUTH);
  else
    directions.push_back(DIRECTION_NORTH);

  return directions;
}

//---------------------------------------------------------------------------

void TRouter2::configure(const int _id, 
			const double _warm_up_time,
			const unsigned int _max_buffer_size,
			TGlobalRoutingTable& grt)
{
  local_id = _id;
  stats.configure(_id, _warm_up_time);
  //iasonas
  start_from_port = DIRECTION_NORTH_0;

  if (grt.isValid())
    routing_table.configure(grt, _id);

  for (int i=0; i<DIRECTIONS_2; i++)
    buffer[i].SetMaxBufferSize(_max_buffer_size);
}

//---------------------------------------------------------------------------

unsigned long TRouter2::getRoutedFlits()
{ 
  return routed_flits; 
}

//---------------------------------------------------------------------------

unsigned int TRouter2::getFlitsCount()
{
  unsigned count = 0;

  for (int i=0; i<DIRECTIONS+1; i++)
    count += buffer[i].Size();

  return count;
}

//---------------------------------------------------------------------------

double TRouter2::getPower()
{
  return stats.power.getPower();
}

//---------------------------------------------------------------------------

int TRouter2::reflexDirection(int direction) const
{
    if (direction == DIRECTION_NORTH) return DIRECTION_SOUTH;
    if (direction == DIRECTION_EAST) return DIRECTION_WEST;
    if (direction == DIRECTION_WEST) return DIRECTION_EAST;
    if (direction == DIRECTION_SOUTH) return DIRECTION_NORTH;

    // you shouldn't be here
    assert(false);
    return NOT_VALID;
}

//---------------------------------------------------------------------------

int TRouter2::getNeighborId(int _id, int direction) const
{
    TCoord my_coord = id2Coord(_id);

    switch (direction)
    {
	case DIRECTION_NORTH:
	    if (my_coord.y==0) return NOT_VALID;
	    my_coord.y--;
	    break;
	case DIRECTION_SOUTH:
	    if (my_coord.y==TGlobalParams::mesh_dim_y-1) return NOT_VALID;
	    my_coord.y++;
	    break;
	case DIRECTION_EAST:
	    if (my_coord.x==TGlobalParams::mesh_dim_x-1) return NOT_VALID;
	    my_coord.x++;
	    break;
	case DIRECTION_WEST:
	    if (my_coord.x==0) return NOT_VALID;
	    my_coord.x--;
	    break;
	default:
	    cout << "direction not valid : " << direction;
	    assert(false);
    }

    int neighbor_id = coord2Id(my_coord);

  return neighbor_id;
}

//---------------------------------------------------------------------------

bool TRouter2::inCongestion()
{
  for (int i=0; i<DIRECTIONS; i++)
    {
      int flits = TGlobalParams::buffer_depth - free_slots_neighbor[i];
      if (flits > (int)(TGlobalParams::buffer_depth * TGlobalParams::dyad_threshold))
	return true;
    }

  return false;
}

//---------------------------------------------------------------------------
