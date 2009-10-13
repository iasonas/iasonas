#include "TNoC.h"
#include "TGlobalRoutingTable.h"
#include "TGlobalTrafficTable.h"

void TNoC::buildMesh()
{
  // Check for routing table availability
  if (TGlobalParams::routing_algorithm == ROUTING_TABLE_BASED)
    assert(grtable.load(TGlobalParams::routing_table_filename));

  // Check for traffic table availability
  if (TGlobalParams::traffic_distribution == TRAFFIC_TABLE_BASED)
    assert(gttable.load(TGlobalParams::traffic_table_filename));

  // Create the mesh as a matrix of tiles
  for(int i=0; i<TGlobalParams::mesh_dim_x; i++)
    {
      for(int j=0; j<TGlobalParams::mesh_dim_y; j++)
	{
	if ((TGlobalParams::topology==1) ) {
	  // Create the single Tile with a proper name
	  char tile_name[20];
	  sprintf(tile_name, "Tile[%02d][%02d]", i, j); //tile_name=Tile[i][j]
	  t[i][j] = new TTile(tile_name);

	  // Tell to the router its coordinates
	  t[i][j]->r->configure(j * TGlobalParams::mesh_dim_x + i,TGlobalParams::stats_warm_up_time,
				TGlobalParams::buffer_depth,grtable);
	  // Tell to the PE its coordinates
	  t[i][j]->pe->local_id = j * TGlobalParams::mesh_dim_x + i;
	  cout << "Dimiourgi8iki o kombos: " << t[i][j]->pe->local_id << "gia i,j : " << i << j << endl;
	  //DEN einai aparaitita afou den orizo dikous mou pinakes!	??
      t[i][j]->pe->traffic_table = &gttable;  // Needed to choose destination
      t[i][j]->pe->never_transmit = (gttable.occurrencesAsSource(t[i][j]->pe->local_id) == 0);

	  // Map clock and reset
	  t[i][j]->clock(clock);
	  t[i][j]->reset(reset);

	  // Map Rx signals---Receive
	if (TGlobalParams::choice==1) {
	  t[i][j]->req_rx[DIRECTION_NORTH](req_to_south[i][j]);	
	  t[i][j]->flit_rx[DIRECTION_NORTH](flit_to_south[i][j]);
	  t[i][j]->ack_rx[DIRECTION_NORTH](ack_to_north[i][j]);	

	  t[i][j]->req_rx[DIRECTION_EAST](req_to_west[i+1][j]);
	  t[i][j]->flit_rx[DIRECTION_EAST](flit_to_west[i+1][j]);
	  t[i][j]->ack_rx[DIRECTION_EAST](ack_to_east[i+1][j]);

	  t[i][j]->req_rx[DIRECTION_SOUTH](req_to_north[i][j+1]);
	  t[i][j]->flit_rx[DIRECTION_SOUTH](flit_to_north[i][j+1]);
	  t[i][j]->ack_rx[DIRECTION_SOUTH](ack_to_south[i][j+1]);

	  t[i][j]->req_rx[DIRECTION_WEST](req_to_east[i][j]);
	  t[i][j]->flit_rx[DIRECTION_WEST](flit_to_east[i][j]);
	  t[i][j]->ack_rx[DIRECTION_WEST](ack_to_west[i][j]);
	}
	//Iasonas
	//My **************************************************************
	if (TGlobalParams::choice==2) {
	/*
	Named association:The INPUT PORT req_rx[DIRECTION_NORTH], which belongs to tile t[i][j], connects to SIGNAL
	req_to_south[i][j]. 
	*/
	  t[i][j]->req_rx[DIRECTION_NORTH](req_to_south[i][j]);	
	  t[i][j]->flit_rx[DIRECTION_NORTH](flit_to_south[i][j]);	
	  t[i][j]->ack_rx[DIRECTION_NORTH](ack_to_north[i][j]);	

	  t[i][j]->req_rx[DIRECTION_EAST](req_to_west[i][j]);
	  t[i][j]->flit_rx[DIRECTION_EAST](flit_to_west[i][j]);
	  t[i][j]->ack_rx[DIRECTION_EAST](ack_to_east[i][j]);

	  t[i][j]->req_rx[DIRECTION_SOUTH](req_to_north[i][j]);
	  t[i][j]->flit_rx[DIRECTION_SOUTH](flit_to_north[i][j]);
	  t[i][j]->ack_rx[DIRECTION_SOUTH](ack_to_south[i][j]);

	  t[i][j]->req_rx[DIRECTION_WEST](req_to_east[i][j]);
	  t[i][j]->flit_rx[DIRECTION_WEST](flit_to_east[i][j]);
	  t[i][j]->ack_rx[DIRECTION_WEST](ack_to_west[i][j]);
	}
	  // Map Tx signals---Transmit
	if (TGlobalParams::choice==1) {
	  t[i][j]->req_tx[DIRECTION_NORTH](req_to_north[i][j]);
	  t[i][j]->flit_tx[DIRECTION_NORTH](flit_to_north[i][j]);
	  t[i][j]->ack_tx[DIRECTION_NORTH](ack_to_south[i][j]);

	  t[i][j]->req_tx[DIRECTION_EAST](req_to_east[i+1][j]);
	  t[i][j]->flit_tx[DIRECTION_EAST](flit_to_east[i+1][j]);
	  t[i][j]->ack_tx[DIRECTION_EAST](ack_to_west[i+1][j]);

	  t[i][j]->req_tx[DIRECTION_SOUTH](req_to_south[i][j+1]);
	  t[i][j]->flit_tx[DIRECTION_SOUTH](flit_to_south[i][j+1]);
	  t[i][j]->ack_tx[DIRECTION_SOUTH](ack_to_north[i][j+1]);

	  t[i][j]->req_tx[DIRECTION_WEST](req_to_west[i][j]);
	  t[i][j]->flit_tx[DIRECTION_WEST](flit_to_west[i][j]);
	  t[i][j]->ack_tx[DIRECTION_WEST](ack_to_east[i][j]);
	}
	  //Iasonas
	  //My *********************************************************
	if (TGlobalParams::choice==2) {
		if (j!=0) {
	  		t[i][j]->req_tx[DIRECTION_NORTH](req_to_north[i][j-1]);
	  		t[i][j]->flit_tx[DIRECTION_NORTH](flit_to_north[i][j-1]);
	  		t[i][j]->ack_tx[DIRECTION_NORTH](ack_to_south[i][j-1]);
		} 
		else {
			t[i][j]->req_tx[DIRECTION_NORTH](dummy_signal);
	  		t[i][j]->flit_tx[DIRECTION_NORTH](dummy_flit);
	  		t[i][j]->ack_tx[DIRECTION_NORTH](dummy_signal);
	  	} 	
	  		t[i][j]->req_tx[DIRECTION_EAST](req_to_east[i+1][j]);
	  		t[i][j]->flit_tx[DIRECTION_EAST](flit_to_east[i+1][j]);
	  		t[i][j]->ack_tx[DIRECTION_EAST](ack_to_west[i+1][j]);
				
		/*
		Named association:The OUTPUT PORT req_tx[DIRECTION_SOUTH] connects to SIGNAL req_to_south[i][j+1]
		So, input port req_rx[DIRECTION_NORTH] of tile[i][j] connects to 
		output port req_tx[DIRECTION_SOUTH] of tile[i][j-1]
		because both of them are connected to the same signal (signal req_to_south[i][j])
		*/
	  		t[i][j]->req_tx[DIRECTION_SOUTH](req_to_south[i][j+1]);
	  		t[i][j]->flit_tx[DIRECTION_SOUTH](flit_to_south[i][j+1]);
	  		t[i][j]->ack_tx[DIRECTION_SOUTH](ack_to_north[i][j+1]);
	  		  		
		if (i!=0) {
		  t[i][j]->req_tx[DIRECTION_WEST](req_to_west[i-1][j]);
		  t[i][j]->flit_tx[DIRECTION_WEST](flit_to_west[i-1][j]);
		  t[i][j]->ack_tx[DIRECTION_WEST](ack_to_east[i-1][j]);
		}
		 else {
		  t[i][j]->req_tx[DIRECTION_WEST](dummy_signal);
		  t[i][j]->flit_tx[DIRECTION_WEST](dummy_flit);
		  t[i][j]->ack_tx[DIRECTION_WEST](dummy_signal);
		}  
	}

	  // Map buffer level signals (analogy with req_tx/rx port mapping)
	if (TGlobalParams::choice==1) {
	  t[i][j]->free_slots[DIRECTION_NORTH](free_slots_to_north[i][j]);
	  t[i][j]->free_slots[DIRECTION_EAST](free_slots_to_east[i+1][j]);
	  t[i][j]->free_slots[DIRECTION_SOUTH](free_slots_to_south[i][j+1]);
	  t[i][j]->free_slots[DIRECTION_WEST](free_slots_to_west[i][j]);

	  t[i][j]->free_slots_neighbor[DIRECTION_NORTH](free_slots_to_south[i][j]);
	  t[i][j]->free_slots_neighbor[DIRECTION_EAST](free_slots_to_west[i+1][j]);
	  t[i][j]->free_slots_neighbor[DIRECTION_SOUTH](free_slots_to_north[i][j+1]);
	  t[i][j]->free_slots_neighbor[DIRECTION_WEST](free_slots_to_east[i][j]);
	}
	  //Iasonas
	  //My *************FREE SLOTS
	if (TGlobalParams::choice==2) {
	  t[i][j]->free_slots[DIRECTION_NORTH](free_slots_to_north[i][j]);
	  t[i][j]->free_slots[DIRECTION_EAST](free_slots_to_east[i][j]);
	  t[i][j]->free_slots[DIRECTION_SOUTH](free_slots_to_south[i][j]);
	  t[i][j]->free_slots[DIRECTION_WEST](free_slots_to_west[i][j]);
	  
	  if (j!=0)	t[i][j]->free_slots_neighbor[DIRECTION_NORTH](free_slots_to_south[i][j-1]);
		else t[i][j]->free_slots_neighbor[DIRECTION_NORTH](dummy_slots);
	  t[i][j]->free_slots_neighbor[DIRECTION_EAST](free_slots_to_west[i+1][j]);
	  t[i][j]->free_slots_neighbor[DIRECTION_SOUTH](free_slots_to_north[i][j+1]);
	  if (i!=0) t[i][j]->free_slots_neighbor[DIRECTION_WEST](free_slots_to_east[i-1][j]);
		else t[i][j]->free_slots_neighbor[DIRECTION_WEST](dummy_slots);	
	}

	  // NoP 
	if (TGlobalParams::choice==1) {
	  t[i][j]->NoP_data_out[DIRECTION_NORTH](NoP_data_to_north[i][j]);
	  t[i][j]->NoP_data_out[DIRECTION_EAST](NoP_data_to_east[i+1][j]);
	  t[i][j]->NoP_data_out[DIRECTION_SOUTH](NoP_data_to_south[i][j+1]);
	  t[i][j]->NoP_data_out[DIRECTION_WEST](NoP_data_to_west[i][j]);

	  t[i][j]->NoP_data_in[DIRECTION_NORTH](NoP_data_to_south[i][j]);
	  t[i][j]->NoP_data_in[DIRECTION_EAST](NoP_data_to_west[i+1][j]);
	  t[i][j]->NoP_data_in[DIRECTION_SOUTH](NoP_data_to_north[i][j+1]);
	  t[i][j]->NoP_data_in[DIRECTION_WEST](NoP_data_to_east[i][j]);
	}
	  //My ******************************************************************
	if (TGlobalParams::choice==2) {
	  t[i][j]->NoP_data_out[DIRECTION_NORTH](NoP_data_to_north[i][j]);
	  t[i][j]->NoP_data_out[DIRECTION_EAST](NoP_data_to_east[i][j]);
	  t[i][j]->NoP_data_out[DIRECTION_SOUTH](NoP_data_to_south[i][j]);
	  t[i][j]->NoP_data_out[DIRECTION_WEST](NoP_data_to_west[i][j]);

	  if (j!=0) t[i][j]->NoP_data_in[DIRECTION_NORTH](NoP_data_to_south[i][j-1]);
		else t[i][j]->NoP_data_in[DIRECTION_NORTH](dummy_data);
	  t[i][j]->NoP_data_in[DIRECTION_EAST](NoP_data_to_west[i+1][j]);
	  t[i][j]->NoP_data_in[DIRECTION_SOUTH](NoP_data_to_north[i][j+1]);
	  if (i!=0) t[i][j]->NoP_data_in[DIRECTION_WEST](NoP_data_to_east[i-1][j]);
		else t[i][j]->NoP_data_in[DIRECTION_WEST](dummy_data);	
	}

	} //if TGlobalParams::topology==1
  }//j
 }//i	
	//Iasonas
	//TOPOLOGY == 2
	//********************
	if ((TGlobalParams::topology==2)) {
		for(int i=0; i<TGlobalParams::mesh_dim_x; i++) {
			for(int j=0; j<TGlobalParams::mesh_dim_y; j++) {
				char tile_name[20];
				sprintf(tile_name, "Tile[%02d][%02d]", i, j);
				t[i][j] = new TTile(tile_name);
				// Tell to the router its coordinates
				t[i][j]->r->configure(j * TGlobalParams::mesh_dim_x + i, TGlobalParams::stats_warm_up_time,
						TGlobalParams::buffer_depth,grtable);
				// Tell to the PE its coordinates
				t[i][j]->pe->local_id = j * TGlobalParams::mesh_dim_x + i;
				//t[i][j]->pe->traffic_table = &gttable;  // Needed to choose destination
				//t[i][j]->pe->never_transmit = (gttable.occurrencesAsSource(t[i][j]->pe->local_id) == 0);
	  		  	t[i][j]->clock(clock);
				t[i][j]->reset(reset);
				//RECEIVE
				t[i][j]->req_rx[DIRECTION_NORTH](req_to_south[i][j]);	
			    t[i][j]->flit_rx[DIRECTION_NORTH](flit_to_south[i][j]);	
			    t[i][j]->ack_rx[DIRECTION_NORTH](ack_to_north[i][j]);	

				t[i][j]->req_rx[DIRECTION_EAST](req_to_west[i][j]);
				t[i][j]->flit_rx[DIRECTION_EAST](flit_to_west[i][j]);
				t[i][j]->ack_rx[DIRECTION_EAST](ack_to_east[i][j]);

				t[i][j]->req_rx[DIRECTION_SOUTH](req_to_north[i][j]);
				t[i][j]->flit_rx[DIRECTION_SOUTH](flit_to_north[i][j]);
				t[i][j]->ack_rx[DIRECTION_SOUTH](ack_to_south[i][j]);

				t[i][j]->req_rx[DIRECTION_WEST](req_to_east[i][j]);
				t[i][j]->flit_rx[DIRECTION_WEST](flit_to_east[i][j]);
				t[i][j]->ack_rx[DIRECTION_WEST](ack_to_west[i][j]);
				//TRANSMIT
				t[i][j]->req_tx[DIRECTION_NORTH](req_to_north[i][j+2]);	
			    t[i][j]->flit_tx[DIRECTION_NORTH](flit_to_north[i][j+2]);	
			    t[i][j]->ack_tx[DIRECTION_NORTH](ack_to_south[i][j+2]);	

				t[i][j]->req_tx[DIRECTION_EAST](req_to_east[i+1][j]);
				t[i][j]->flit_tx[DIRECTION_EAST](flit_to_east[i+1][j]);
				t[i][j]->ack_tx[DIRECTION_EAST](ack_to_west[i+1][j]);

				t[i][j]->req_tx[DIRECTION_SOUTH](req_to_south[i][j+2]);
				t[i][j]->flit_tx[DIRECTION_SOUTH](flit_to_south[i][j+2]);
				t[i][j]->ack_tx[DIRECTION_SOUTH](ack_to_north[i][j+2]);
				if (i!=0) {
					t[i][j]->req_tx[DIRECTION_WEST](req_to_west[i-1][j]);
					t[i][j]->flit_tx[DIRECTION_WEST](flit_to_west[i-1][j]);
					t[i][j]->ack_tx[DIRECTION_WEST](ack_to_east[i-1][j]); }
				else {
					t[i][j]->req_tx[DIRECTION_WEST](dummy_signal);
					t[i][j]->flit_tx[DIRECTION_WEST](dummy_flit);
					t[i][j]->ack_tx[DIRECTION_WEST](dummy_signal); }
				//FREE SLOTS
				t[i][j]->free_slots[DIRECTION_NORTH](free_slots_to_north[i][j]);
				t[i][j]->free_slots[DIRECTION_EAST](free_slots_to_east[i][j]);
				t[i][j]->free_slots[DIRECTION_SOUTH](free_slots_to_south[i][j]);
				t[i][j]->free_slots[DIRECTION_WEST](free_slots_to_west[i][j]);
						
				t[i][j]->free_slots_neighbor[DIRECTION_NORTH](free_slots_to_south[i][j+2]);
				t[i][j]->free_slots_neighbor[DIRECTION_EAST](free_slots_to_west[i+1][j]);
				t[i][j]->free_slots_neighbor[DIRECTION_SOUTH](free_slots_to_north[i][j+2]);
				if (i!=0) t[i][j]->free_slots_neighbor[DIRECTION_WEST](free_slots_to_east[i-1][j]);
				else t[i][j]->free_slots_neighbor[DIRECTION_WEST](dummy_slots);
				//NOP DATA
				t[i][j]->NoP_data_out[DIRECTION_NORTH](NoP_data_to_north[i][j]);
				t[i][j]->NoP_data_out[DIRECTION_EAST](NoP_data_to_east[i][j]);
				t[i][j]->NoP_data_out[DIRECTION_SOUTH](NoP_data_to_south[i][j]);
				t[i][j]->NoP_data_out[DIRECTION_WEST](NoP_data_to_west[i][j]);
				 
				t[i][j]->NoP_data_in[DIRECTION_NORTH](NoP_data_to_south[i][j+2]);
				t[i][j]->NoP_data_in[DIRECTION_EAST](NoP_data_to_west[i+1][j]);
				t[i][j]->NoP_data_in[DIRECTION_SOUTH](NoP_data_to_north[i][j+2]);
				if (i!=0) t[i][j]->NoP_data_in[DIRECTION_WEST](NoP_data_to_east[i-1][j]);
				else t[i][j]->NoP_data_in[DIRECTION_WEST](dummy_data);
				
				
				
		  }//j
		 }//i
				tbig=new TTile2("BigRouter");
				tbig->r->configure(10, TGlobalParams::stats_warm_up_time,
						TGlobalParams::buffer_depth,grtable);
				tbig->clock(clock);
				tbig->reset(reset);
				
				//RECEIVE
				tbig->req_rx[DIRECTION_NORTH_0](req_to_south[0][2]);	
			    tbig->flit_rx[DIRECTION_NORTH_0](flit_to_south[0][2]);	
			    tbig->ack_rx[DIRECTION_NORTH_0](ack_to_north[0][2]);	
				
				tbig->req_rx[DIRECTION_NORTH_1](req_to_south[1][2]);	
			    tbig->flit_rx[DIRECTION_NORTH_1](flit_to_south[1][2]);	
			    tbig->ack_rx[DIRECTION_NORTH_1](ack_to_north[1][2]);
				
				tbig->req_rx[DIRECTION_NORTH_2](req_to_south[2][2]);	
			    tbig->flit_rx[DIRECTION_NORTH_2](flit_to_south[2][2]);	
			    tbig->ack_rx[DIRECTION_NORTH_2](ack_to_north[2][2]);
				
				tbig->req_rx[DIRECTION_SOUTH_0](req_to_north[0][3]);	
			    tbig->flit_rx[DIRECTION_SOUTH_0](flit_to_north[0][3]);	
			    tbig->ack_rx[DIRECTION_SOUTH_0](ack_to_south[0][3]);	
				
				tbig->req_rx[DIRECTION_SOUTH_1](req_to_north[1][3]);	
			    tbig->flit_rx[DIRECTION_SOUTH_1](flit_to_north[1][3]);	
			    tbig->ack_rx[DIRECTION_SOUTH_1](ack_to_south[1][3]);
				
				tbig->req_rx[DIRECTION_SOUTH_2](req_to_north[2][3]);	
			    tbig->flit_rx[DIRECTION_SOUTH_2](flit_to_north[2][3]);	
			    tbig->ack_rx[DIRECTION_SOUTH_2](ack_to_south[2][3]);
				
				//cout << "CHECK POINT 1" ;
				
				//TRANSMIT
				tbig->req_tx[DIRECTION_NORTH_0](req_to_north[0][0]);	
			    tbig->flit_tx[DIRECTION_NORTH_0](flit_to_north[0][0]);	
			    tbig->ack_tx[DIRECTION_NORTH_0](ack_to_south[0][0]);	
				
				tbig->req_tx[DIRECTION_NORTH_1](req_to_north[1][0]);	
			    tbig->flit_tx[DIRECTION_NORTH_1](flit_to_north[1][0]);	
			    tbig->ack_tx[DIRECTION_NORTH_1](ack_to_south[1][0]);
				
				tbig->req_tx[DIRECTION_NORTH_2](req_to_north[2][0]);	
			    tbig->flit_tx[DIRECTION_NORTH_2](flit_to_north[2][0]);	
			    tbig->ack_tx[DIRECTION_NORTH_2](ack_to_south[2][0]);
				
				tbig->req_tx[DIRECTION_SOUTH_0](req_to_south[0][1]);	
			    tbig->flit_tx[DIRECTION_SOUTH_0](flit_to_south[0][1]);	
			    tbig->ack_tx[DIRECTION_SOUTH_0](ack_to_north[0][1]);	
				
				tbig->req_tx[DIRECTION_SOUTH_1](req_to_south[1][1]);	
			    tbig->flit_tx[DIRECTION_SOUTH_1](flit_to_south[1][1]);	
			    tbig->ack_tx[DIRECTION_SOUTH_1](ack_to_north[1][1]);
				
				tbig->req_tx[DIRECTION_SOUTH_2](req_to_south[2][1]);	
			    tbig->flit_tx[DIRECTION_SOUTH_2](flit_to_south[2][1]);	
			    tbig->ack_tx[DIRECTION_SOUTH_2](ack_to_north[2][1]);

				//FREE SLOTS
				tbig->free_slots[DIRECTION_NORTH_0](free_slots_to_north[0][2]);
				tbig->free_slots[DIRECTION_NORTH_1](free_slots_to_north[1][2]);
				tbig->free_slots[DIRECTION_NORTH_2](free_slots_to_north[2][2]);
				
				tbig->free_slots[DIRECTION_SOUTH_0](free_slots_to_south[0][3]);
				tbig->free_slots[DIRECTION_SOUTH_1](free_slots_to_south[1][3]);
				tbig->free_slots[DIRECTION_SOUTH_2](free_slots_to_south[2][3]);
				
				tbig->free_slots_neighbor[DIRECTION_NORTH_0](free_slots_to_south[0][1]);
				tbig->free_slots_neighbor[DIRECTION_NORTH_1](free_slots_to_south[1][1]);
				tbig->free_slots_neighbor[DIRECTION_NORTH_2](free_slots_to_south[2][1]);
				
				tbig->free_slots_neighbor[DIRECTION_SOUTH_0](free_slots_to_north[0][1]);
				tbig->free_slots_neighbor[DIRECTION_SOUTH_1](free_slots_to_north[1][1]);
				tbig->free_slots_neighbor[DIRECTION_SOUTH_2](free_slots_to_north[2][1]);
			
				//NOP DATA
				tbig->NoP_data_out[DIRECTION_NORTH_0](NoP_data_to_north[0][2]);
				tbig->NoP_data_out[DIRECTION_NORTH_1](NoP_data_to_north[1][2]);
				tbig->NoP_data_out[DIRECTION_NORTH_2](NoP_data_to_north[2][2]);
				
				tbig->NoP_data_out[DIRECTION_SOUTH_0](NoP_data_to_south[0][3]);
				tbig->NoP_data_out[DIRECTION_SOUTH_1](NoP_data_to_south[1][3]);
				tbig->NoP_data_out[DIRECTION_SOUTH_2](NoP_data_to_south[2][3]);
				
				tbig->NoP_data_in[DIRECTION_NORTH_0](NoP_data_to_south[0][1]);
				tbig->NoP_data_in[DIRECTION_NORTH_1](NoP_data_to_south[1][1]);
				tbig->NoP_data_in[DIRECTION_NORTH_2](NoP_data_to_south[2][1]);
				
				tbig->NoP_data_in[DIRECTION_SOUTH_0](NoP_data_to_north[0][1]);
				tbig->NoP_data_in[DIRECTION_SOUTH_1](NoP_data_to_north[1][1]);
				tbig->NoP_data_in[DIRECTION_SOUTH_2](NoP_data_to_north[2][1]);
				
				//*/ 
	} //topoology==2
	

    
 	//Iasonas	
	//To NoC symfona me tis parapano katey8ynseis nomizo oti exei tn parakato morfi
	printf("To NoC poy dimiourgeitai einai:");
	printf("\n");
	printf("\n");
	if (TGlobalParams::topology==1) {
	printf("dim_x = %d\n",TGlobalParams::mesh_dim_x);
	printf("dim_y = %d\n",TGlobalParams::mesh_dim_y);
	for (int j=0; j<TGlobalParams::mesh_dim_y;j++){
		cout << endl;
		for (int i=0; i<TGlobalParams::mesh_dim_x;i++)
			cout << "\t" << t[i][j]->pe->local_id ;
		}
	} 
	if (TGlobalParams::topology==2) {
	printf("dim_x = %d\n",TGlobalParams::mesh_dim_x);
	printf("dim_y = %d\n",TGlobalParams::mesh_dim_y);
	for (int j=0; j<TGlobalParams::mesh_dim_y;j++){
		cout << endl;
		if (j==1) cout << "\t     ROUTER" << endl; 
		for (int i=0; i<TGlobalParams::mesh_dim_x;i++)
			cout << "\t" << t[i][j]->pe->local_id ;
		}
	} 
	cout << endl;
	cout << "\t \t NORTH(i,j-1)" << endl;
	cout << "\t \t  ^  " << endl;
	cout << "\t \t  |  " << endl;
	cout << "WEST(i-1,j) <-- (i,j) --> EAST(i+1,j)" << endl;
	cout << "\t \t  |  " << endl;
	cout << "\t \t  |  " << endl;
	cout << "\t \t SOUTH(i,j+1)" << endl;
	cout<<endl;		
	printf("\n");
	


  // dummy TNoP_data structure
  TNoP_data tmp_NoP;

  tmp_NoP.sender_id = NOT_VALID;

  for (int i=0; i<DIRECTIONS; i++)
  {
      tmp_NoP.channel_status_neighbor[i].free_slots = NOT_VALID;
      tmp_NoP.channel_status_neighbor[i].available = false;
  }
 
  // Clear signals for borderline nodes ---Kai na min ta midenize douleyei alla 8ewritika dn antistoixei se RTL
  for(int i=0; i<=TGlobalParams::mesh_dim_x; i++)
    {
      req_to_south[i][0] = 0;
      ack_to_north[i][0] = 0;
      //if (choice=2) {
      //	req_to_south[i][0] = 0;
      //	ack_to_north[i][0] = 0;
      
      req_to_north[i][TGlobalParams::mesh_dim_y] = 0;
      ack_to_south[i][TGlobalParams::mesh_dim_y] = 0;

      free_slots_to_south[i][0].write(NOT_VALID);
      //Test:
      //free_slots_to_south[i][TGlobalParams::mesh_dim_y].write(NOT_VALID);
      //
      free_slots_to_north[i][TGlobalParams::mesh_dim_y].write(NOT_VALID);

      NoP_data_to_south[i][0].write(tmp_NoP);
      NoP_data_to_north[i][TGlobalParams::mesh_dim_y].write(tmp_NoP);

    }

  for(int j=0; j<=TGlobalParams::mesh_dim_y; j++)
    {
      req_to_east[0][j] = 0;
      ack_to_west[0][j] = 0;
      req_to_west[TGlobalParams::mesh_dim_x][j] = 0;
      ack_to_east[TGlobalParams::mesh_dim_x][j] = 0;

      free_slots_to_east[0][j].write(NOT_VALID);
      free_slots_to_west[TGlobalParams::mesh_dim_x][j].write(NOT_VALID);

      NoP_data_to_east[0][j].write(tmp_NoP);
      NoP_data_to_west[TGlobalParams::mesh_dim_x][j].write(tmp_NoP);

    }
    
  // invalidate reservation table entries for non-exhistent channels
  for(int i=0; i<TGlobalParams::mesh_dim_x; i++)
    {
      t[i][0]->r->reservation_table.invalidate(DIRECTION_NORTH);
      t[i][TGlobalParams::mesh_dim_y-1]->r->reservation_table.invalidate(DIRECTION_SOUTH);
    }
  for(int j=0; j<TGlobalParams::mesh_dim_y; j++)
    {
      t[0][j]->r->reservation_table.invalidate(DIRECTION_WEST);
      t[TGlobalParams::mesh_dim_x-1][j]->r->reservation_table.invalidate(DIRECTION_EAST);
    }

}

//---------------------------------------------------------------------------

TTile* TNoC::searchNode(const int id) const
{
  for (int i=0; i<TGlobalParams::mesh_dim_x; i++)
    for (int j=0; j<TGlobalParams::mesh_dim_y; j++)
      if (t[i][j]->r->local_id == id)
	return t[i][j];

  return false;
}

//---------------------------------------------------------------------------

