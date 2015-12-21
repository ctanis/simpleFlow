#ifndef SIMPLEFLOW_H
#define SIMPLEFLOW_H

#include <vector>
#include <string>

#include <iostream>
#include <iomanip>
#include <cassert>
#include <strings.h>

#define LOG(msg) std::cout << "(" << __FILE__ << ":" << __LINE__<< " " << __FUNCTION__ << ")" << std::setprecision(15) << msg << std::endl
#define ERROR(msg) std::cout << "ERROR<<" << __FILE__<<":"<< __LINE__<<" "<<__FUNCTION__ << ">>" << std::setprecision(15) << msg << std::endl; \
    assert(false)

namespace sflow
{

    /** template wrapper around a standard C array, allowing them to
     *  be used in STL containers, and providing MPI types (see get_mpi_type())
     and IO formatting. */

    template <typename T, int SIZE>
    class array
    {
    public:
	/*! create of size specified in template, initialize to all zeros **/
	array()
	{
	    // TODO: decide if memset is preferable
	    bzero(_data, SIZE*sizeof(T));
	}

	/*! create copying from another array **/
	array(T init[])
	{
	    for (int i=0; i<SIZE; i++)
	    {
		_data[i] = init[i];
	    }
	}

	/*! normal entity access
	  \param c index of the entity to access
	**/
	T& operator[](int c) { return _data[c]; }

	/*! const entity access
	  \param c index of the entity to access
	**/
	const T& operator[](int c) const { return _data[c]; }

	array<T,SIZE> operator+(const array<T,SIZE>& other) const
	{
	    array<T,SIZE> rval;
	    for (int n=0; n<SIZE; n++)
	    {
		rval[n] = _data[n] + other[n];
	    }
	    return rval;
	}

	array<T,SIZE> operator-(const array<T,SIZE>& other) const
	{
	    array<T,SIZE> rval;
	    for (int n=0; n<SIZE; n++)
	    {
		rval[n] = _data[n] - other[n];
	    }
	    return rval;
	}

	template <typename T2>
	array<T,SIZE> operator*(const T2& val) const
	{
	    array<T,SIZE> rval;
	    for (int n=0; n<SIZE; n++)
	    {
		rval[n] = _data[n] * val;
	    }
	    return rval;
	}

	template <typename T2>
	array<T,SIZE> operator/(const T2& val) const
	{
	    array<T,SIZE> rval;
	    for (int n=0; n<SIZE; n++)
	    {
		rval[n] = _data[n] / val;
	    }
	    return rval;
	}


    protected:
	T	_data[SIZE]; /*!< the actual data */
    };
    //

    
    template <typename T, int SIZE>
    std::istream& operator>>(std::istream& in, array<T,SIZE>& a)
    {
	for (int i=0; i<SIZE; i++)
	{
	    in >> a[i];
	}

	return in;
    }

    template <typename T, int SIZE>
    std::ostream& operator<<(std::ostream& out, array<T,SIZE>& a)
    {
	for (int i=0; i<SIZE; i++)
	{
	    if (i > 0) out << " ";
	    out << a[i];
	}

	return out;
    }

    typedef array<double, 3> node_coords;
    typedef array<int, 4>    tet_nodes;
    typedef array<int, 3>    tri_nodes;

    class LocalGrid
    {
    public:
        int loadStarCD(std::string basename);

        std::vector<node_coords> coords;
        std::vector<tet_nodes> tets;
        std::vector<int> tetTags;
        std::vector<tri_nodes> bnds;
        std::vector<int> bndTags;
    };


}


#endif /* SIMPLEFLOW_H */
