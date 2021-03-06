#include "simpleFlow.hpp"

#include <fstream>
#include <sstream>

using namespace sflow;

#define LINESIZE 1024
#define INDEX_OFFSET 1	// subtract one from all node ids in data file


static int load_nodes(std::string filename, std::vector<node_coords>& coords);
static int load_tets(std::string filename, std::vector<tet_nodes>& faces, std::vector<int>& tags);
static int load_boundaries(std::string filename, std::vector<tri_nodes>& bnds, std::vector<int>& tags);


int LocalGrid::loadStarCD(std::string basename)
{

    if (load_nodes(basename + ".vrt", coords) == 0)
        return 0;

    int maxNode = load_tets(basename + ".cel", tets, tetTags);

    if (maxNode < 0)
    {
        return 0;
    }

    if (maxNode >= (int)coords.size())
    {
        ERROR("tet refers to illegal node: " << maxNode);
        return 0;
    }
        
    maxNode = load_boundaries(basename + ".bnd", bnds, bndTags);

    if (maxNode < 0)
        return 0;

    if (maxNode >= (int)coords.size())
    {
        ERROR("boundary refers to illegal node: " << maxNode);
        return 0;
    }

    return 0;
}





// starcd routines -- return 1 on success

static int load_nodes(std::string filename,
                     std::vector<node_coords>& coords)
{
    std::ifstream in;
    char linebuf[LINESIZE];

    in.open(filename.c_str());
    if (! in.is_open() )
    {
        ERROR("error opening " << filename);
        return 0;
    }


    coords.clear();
    
    while ( in.getline(linebuf, LINESIZE) )
    {
	std::stringstream ss(linebuf);

	int dummy;
	
	ss >> dummy;

	array<double,3> tmp;
	ss >> tmp;

	coords.push_back(tmp);
    }

    LOG("node count: " << coords.size());

    if (in.bad())
    {
        ERROR("input stream went bad -- aborting node read!");
        in.close();
        return 0;
    }

    if (in.eof())
    {
        LOG("success reading " << filename);
    }
    else if (in.fail())
    {
        ERROR("input stream failed -- cutting node read short!");
        in.close();
        return 0;
    }


    in.close();
    return 1;
}




static int load_tets(std::string filename, 
			 std::vector<tet_nodes>& tets,
			 std::vector<int>& tags)
{
    std::ifstream in;
    char linebuf[LINESIZE];
    int data_offset = INDEX_OFFSET;
    int count = 0;
    int maxnode = 0;

    in.open(filename.c_str());
    if (! in.is_open() )
    {
        ERROR("error opening " << filename);
        return -1;
    }

    tets.clear();
    tags.clear();

    
    // while we're not at the end of our part, and we can continue reading lines..
    while (in.getline(linebuf, LINESIZE))
    {
	int conn_scratch[8];
	std::stringstream ss(linebuf);
	int p;
	int vtag;		// volume(cell) tag
	
	ss >> p;		// cell id
	// p -= data_offset;-- thrown away

	for (int n=0; n<8; n++)
	{
	    ss >> p;
	    p-= data_offset;
	    conn_scratch[n] = p;

	    if (p > maxnode) maxnode=p;
	}


	// analyze node type based on Sagar's heuristic?
	if ( (conn_scratch[2] == conn_scratch[3]) &&
	     (conn_scratch[4] == conn_scratch[5]) )
	{
	    // tet
	    conn_scratch[3]=conn_scratch[4];

	    array<int, 4> tet_array;
	    for (int i=0; i<4; i++)
	    {
		tet_array[i]=conn_scratch[i];
	    }
	    
            tets.push_back(tet_array);

	    // save volume condition in container
	    ss >> vtag;
	    tags.push_back(vtag);
	}
	else
	{
            LOG(linebuf);
	    ERROR("error. only tets implemented currently, near byte " <<
		  (long int)in.tellg()<< "; " << tets.size()/4);
	    return -1;
	}

	count++;
    }

    if (in.bad())
    {
        ERROR("input stream went bad -- aborting tet read!");
        in.close();
        return 0;
    }

    if (in.eof())
    {
        LOG("success reading " << filename);
    }
    else if (in.fail())
    {
        ERROR("input stream failed -- cutting tet read short!");
        in.close();
        return 0;
    }

 
    in.close();
    return maxnode;
}






static int load_boundaries(std::string filename, 
                           std::vector<tri_nodes>& bnds,
                           std::vector<int>& tags)
{
    std::ifstream in;
    char linebuf[LINESIZE];
    int data_offset=INDEX_OFFSET;


    in.open(filename.c_str());
    if (! in.is_open() )
    {
        ERROR("error opening " << filename);
        return -1;
    }

    bnds.clear();
    tags.clear();

    int maxnode=0;

    while (in.getline(linebuf, LINESIZE))
    {
	std::stringstream ss(linebuf);

	array<int,3> face;
	int btype;
	int dummy;

	ss>>dummy;
	ss>>face;
	ss>>dummy;
	ss>>btype;

        for (int i=0;i<3; i++)
        {
            if (face[i] > maxnode)
                maxnode = face[i];
        }

        bnds.push_back(face);
	tags.push_back(btype);
	
    }

    if (in.bad())
    {
        ERROR("input stream went bad -- aborting bnd read!");

        in.close();
        return -1;
    }

    if (in.eof())
    {
        LOG("success reading " << filename);
    }
    else if (in.fail())
    {
        ERROR("input stream failed -- cutting bnd read short!");
        
        in.close();
        return -1;
    }


    in.close();
    return maxnode;
}


void sflow::buildCSR(const LocalGrid& grid,
                     std::vector<int>& ia,
                     std::vector<int>& ja)
{
    std::vector<intset> hash(grid.coords.size());

    for (unsigned int n=0; n<hash.size(); n++)
    {
        hash[n].insert(n);      // all values self-dependent
    }

    /* resolve tet dependencies */
    for (unsigned int t=0; t<grid.tets.size(); t++)
    {
        const array<int,4>& tet = grid.tets[t];

        for (int n1=0; n1<4; n1++)
        {
            for (int n2=0; n2<4; n2++)
            {
                if (n1 != n2)
                {
                    hash[tet[n1]].insert(tet[n2]);
                }
            }
        }
    }


    // resolve bnd dependencies -- unnecessary assumes all boundaries are part
    // of a tet

    // for (int t=0; t<grid.bnds.size(); t++)
    // {
    //     const array<int,3>& tri = grid.bnds[t];

    //     for (int n1=0; n1<3; n1++)
    //     {
    //         for (int n2=0; n2<3; n2++)
    //         {
    //             hash[tri[n1]].insert(tri[n2]);
    //         }
    //     }
    // }


    /* flatten hash */
    ia.reserve(hash.size()+1);

    int total=0;
    for (unsigned int n=0; n<hash.size(); n++)
    {
        total += hash[n].size();
    }
    ja.reserve(total);

    for (unsigned int n=0; n<hash.size(); n++)
    {
        ia.push_back(ja.size());

        for (unsigned int v=0; v<hash[n].size(); v++)
        {
            ja.push_back(hash[n][v]);
        }
    }
    ia.push_back(ja.size());

}

