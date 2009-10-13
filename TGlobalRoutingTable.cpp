#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <fstream>
#include "NoximDefs.h"
#include "TGlobalRoutingTable.h"

using namespace std;

//Iasonas:Finds the ID of the next node, given the ID of the current node and direction
//TLinkID is a pair of source and destination node
TLinkId direction2ILinkId(const int node_id, const int dir)
{
  int node_src;

  switch (dir)
    {
    case DIRECTION_NORTH:
      node_src = node_id - TGlobalParams::mesh_dim_x;
      break;

    case DIRECTION_SOUTH:
      node_src = node_id + TGlobalParams::mesh_dim_x;
      break;

    case DIRECTION_EAST:
      node_src = node_id + 1;
      break;

    case DIRECTION_WEST:
      node_src = node_id - 1;
      break;

    case DIRECTION_LOCAL:
      node_src = node_id;
      break;

    default:
      assert(false);
    }


  return TLinkId(node_src, node_id);
}

//Iasonas:founds the direction, given...
//This method is called in table based routing
int oLinkId2Direction(const TLinkId& out_link)
{
  int src = out_link.first;
  int dst = out_link.second;

  if (dst == src)
    return DIRECTION_LOCAL;
  else if (dst == src+1)
    return DIRECTION_EAST;
  else if (dst == src-1)
    return DIRECTION_WEST;
  else if (dst == src - TGlobalParams::mesh_dim_x)
    return DIRECTION_NORTH;
  else if (dst == src + TGlobalParams::mesh_dim_x)
    return DIRECTION_SOUTH;
  else
    assert(false);
  return 0;  
}

//Returns a vector with the integers correspond to directions.

vector<int> admissibleOutputsSet2Vector(const TAdmissibleOutputs& ao)
{
  vector<int> dirs;

  for (TAdmissibleOutputs::iterator i=ao.begin(); i!=ao.end(); i++)
    dirs.push_back(oLinkId2Direction(*i));

  return dirs;
}
//Iasonas...regarding the input routing table this defines the nubmer of column where output link/links(Outs) should be
//Old value: #define COLUMN_AOC 22
//Eleos!! "TAB" is counted as one character, so DON'T use Tab but only space!
#define COLUMN_AOC 22
//??

TGlobalRoutingTable::TGlobalRoutingTable()
{
  valid = false;
}

//---------------------------------------------------------------------------

bool TGlobalRoutingTable::load(const char* fname)
{
  ifstream fin(fname, ios::in);

  if (!fin)
    return false;
  
  rt_noc.clear();

  bool stop = false;
  int epan=1;
  while (!fin.eof() && !stop)
    {
	  //cout << epan << "  line of file ****" <<endl;
	  epan++;	
      char line[128];
      //fin.getline(line, sizeof(line)-1);
	  fin.getline(line, 127);
      //cout<< "H grammi pou diabastike einai:"<<endl << line << endl;
	  //cout<< "line+1 is:"<< endl << line+1<<endl;	
      if (line[0] == '\0') stop = true;
      else {
		if (line[0] != '%')
	    {
	      int node_id, in_src, in_dst, dst_id, out_src, out_dst;
	      if (sscanf(line+1, "%d %d->%d %d", &node_id, &in_src, &in_dst, &dst_id) == 4)
		  {
		  TLinkId lin(in_src, in_dst);
		  
		  char *pstr = line+COLUMN_AOC;
		  //cout<< "To pstr einai:" << pstr << endl;
		  while (sscanf(pstr, "%d->%d", &out_src, &out_dst) == 2)
		    {
		      TLinkId lout(out_src, out_dst);
			//Iasonas:The most important line
		      rt_noc[node_id][lin][dst_id].insert(lout);
		      
		      pstr = strstr(pstr, ",");
		      //cout<< "To pstr einai:" << pstr << endl;
		      pstr++;
		      //cout<< "To pstr einai:" << pstr << endl;
		    }
		  }
	    }
	  }
    }

  valid = true;
  
  return true;
}

//---------------------------------------------------------------------------

// bool TGlobalRoutingTable::seek(const char* fname, const char* rt_label, ifstream& fin)
// {
//   fin.open(fname, ifstream::in);

//   if (!fin.is_open())
//     return false;

//   bool found = false;
//   while (!fin.eof() && !found)
//     {
//       char line[128];
//       fin.getline(line, sizeof(line)-1);

//       if (strstr(line, rt_label) != NULL)
// 	found = true;
//     }
  
//   return found;
// }

//---------------------------------------------------------------------------

TRoutingTableNode TGlobalRoutingTable::getNodeRoutingTable(const int node_id)
{
  return rt_noc[node_id];
}

//---------------------------------------------------------------------------

