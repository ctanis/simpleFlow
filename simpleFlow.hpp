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


    /** lightweight set of T values (that is, lighter in memory usage than `std::set<T>`)

        @tparam T must be a type that has operator< defined
     **/
    template <typename T>
    class uniqvec
    {
    public:
	uniqvec() {
#ifdef SPLATT_UVEC_INIT_CAPACITY
	    _vals.reserve(SPLATT_UVEC_INIT_CAPACITY);
#endif
	}


        // this is kind of a hacky thing to do
        operator bool()
        {
            return _vals.size() > 0;
        }


	/*! insert value, not allowing duplicates
	  \param val value to be inserted
	**/
	void insert(const T& val)
	{
	    typename std::vector<T>::iterator v=_vals.begin();

	    // insert into _vals sorted ascending
	    for (; v != _vals.end(); v++) // TODO: binary search?
	    {
		if (val == *v) return;
		if (val < *v)
		{
		    _vals.insert(v, val);
		    return;
		}
	    }


	    // tack onto the end if we get this for
	    _vals.push_back(val);	    
	}


	/*! check if value is in set
	  \param val value to check for
	  \return `true` if val is in set, `false` otherwise
	**/
	bool contains(const T& val) const
	{
	    // TODO: binary search?
	    return std::find(_vals.begin(), _vals.end(), val) != _vals.end();
	}

	/*! remove entitys from set that are not in `other`
	  \param other set to intersect with
	**/
	void intersect(const uniqvec<T> other)
	{
	    unsigned int me=0, o=0, nextme=0;;
	    while (nextme<_vals.size())
	    {
		// jump over irrelevant others
		while (o<other.size() && other[o]<_vals[nextme])
		{
		    o++;
		}

		if (o == other.size())
		{
		    break;
		}

		if (_vals[nextme] == other[o])
		{
		    _vals[me]=_vals[nextme];
		    me++; nextme++;
		}
		else
		{
		    while (nextme < _vals.size() && _vals[nextme] < other[o])
		    {
			nextme++;
		    }
		}
	    }

	    _vals.resize(me);
	}


	void remove(const T& v)
	{
	    typename std::vector<T>::iterator match = std::find(_vals.begin(), _vals.end(), v);

	    if (match != _vals.end())
	    {
		_vals.erase(match);
	    }
	}

	void clear()
	{
	    _vals.clear();
	}

	/*! number of entities in set */
	size_t size() const { return _vals.size(); }

	/*! normal entity access
	  \param t index of entity to access
	**/
	T& operator[](int t) { return _vals[t]; }

	/*! const entity access
	  \param t index of entity to access
	**/
	const T& operator[](int t) const { return _vals[t]; }

    private:
	std::vector<T> _vals;
    };

    typedef uniqvec<int> intset;

    
    template <typename T>
    std::ostream& operator<<(std::ostream& out, const uniqvec<T>& v)
    {
	out << "[";
	for (unsigned int n=0; n<v.size(); n++)
	{
	    if (n>0) out << " ";
	    out << v[n];
	}

	out << "]";
	return  out;
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

    void buildCSR(const LocalGrid& grid,
                  std::vector<int>& ia,
                  std::vector<int>& ja);


}


#endif /* SIMPLEFLOW_H */
