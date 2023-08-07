#pragma once
/*!
define ARRAY_CHECK_BOUNDS to do bounds check in operator[], 
this will throw an exception if the index is exceeded then
*/
#if defined(_DEBUG)
#define ARRAY_CHECK_BOUNDS
#endif

#if defined(ARRAY_CHECK_BOUNDS)
#include "imageerror.h"
#endif

/*!
an array with a fixed but variable number of elements of type T

use operator[] to read/write access elemts.
use at() to read access elements.
these may throw an exception if ARR_CHECK_BOUNDS is defined and the index is exceeding the array
note the index is zero based

call size() to get the number of elements

note that this is a convenience class to deal with the fact that you cannot write c++ code like

\code
unsigned numValues=someUnsignedReturningFunction();
int values[numValues];
\endcode

because numValues is not const. instead of using the standard error prone way:

\code
unsigned numValues=someUnsignedReturningFunction();
int* pValues = new int[numValues];
.
.
.
delete[] pValues;
\endcode

you can use:

\code
unsigned numValues=someUnsignedReturningFunction();
array<int> values(numValues);
\endcode

TODO: how to pass args to the T ctor?
*/
template <typename T> class array
{
public:
	/*!
	creates the array. each element will be preset by it's default ctor, if any

	note it is allowed to pass 0, but not very useful...
	*/
	array(unsigned sz);
	/*!
	presets the array with val
	*/
	array(unsigned sz,const T& val);
	~array();
#if defined(ARR_CHECK_BOUND)
	T& operator[](unsigned index) throw(...) {if(index>=m_uNumElems) throw("out of bounds in array<>::operator[]"); return m_pVals[index];}
	const T& at(unsigned index) throw(...) {if(index>=m_uNumElems) throw("out of bounds in array<>::operator[]"); return m_pVals[index];}
#else
	T& operator[](unsigned index) throw() {return m_pVals[index];}
	const T& at(unsigned index) throw() {return m_pVals[index];}
#endif
	unsigned size() const {return m_uNumElems;}
	bool empty() const {return m_uNumElems==0;}
	/*!
	direct access to underlying elements

	these are size() values in consecutive order
	*/
	T* pVals() {return m_pVals;}
protected:
	unsigned m_uNumElems;
	T* m_pVals;
private:
	array();
	array(const array<T>& rOther);
	array<T>& operator=(const array<T>& rOther);
};


/*!
a 2 dimensional array with fixed but variable number of elements per row and column of type T

use tAt() to access elemts

note: how can i implement operator[][] ???

this may throw an exception if ARR_CHECK_BOUNDS is defined and the index is exceeding the array bounds
note the index is zero based

call size() to get the number of elements

call xSize() to get the number of 'rows'

call ySize() to get the number of 'columns'

call operator[] to get a pointer to T to the 'row' number x (a 1d array of size ySize()...)

note that this is a convenience class to deal with the fact that you cannot write c++ code like

\code
unsigned numXValues=someUnsignedReturningFunction();
unsigned numYValues=someUnsignedReturningFunction();
int values[numXValues][numYValues];
\endcode

because numXValues and numYValues are not const. furthermore, for multidimensional arrays,
you cannot even use

\code
unsigned numXValues=someUnsignedReturningFunction();
unsigned numYValues=someUnsignedReturningFunction();
int* pValues = new int[numXValues][numYValues];
.
.
.
delete[] pValues;
\endcode

because numYValues is not const.
you can use:

\code
unsigned numXValues=someUnsignedReturningFunction();
unsigned numYValues=someUnsignedReturningFunction();
array2D<int> values(numXValues,numYValues);
\endcode

*/
template <typename T> class array2D
{
public:
	/*!
	creates the array2D. each element will be preset by it's default ctor, if any

	note it is allowed to pass 0, but not very useful...
	*/
	array2D(unsigned x,unsigned y);
	/*!
	presets the array with val
	*/
	array2D(unsigned x,unsigned y,const T& val);
	/*!
	dtor
	*/
	~array2D();
	/*!
	returns a reference to the T at postion (x,y)
	*/
	// NOTE i think this is a bug: x*m_uNumXElems+y should be x*m_uNumYElems+y
#if defined(ARR_CHECK_BOUND)
	T& at(unsigned x,unsigned y) throw(...) {if(x>=m_uNumXElems||y>=m_uNumYElems) throw("out of bounds in array2D<>::at()"); return m_pVals[x*m_uNumXElems+y];}
#else
	T& at(unsigned x,unsigned y) throw() {return m_pVals[x*m_uNumXElems+y];}
#endif
	/*!
	returns a pointer to the subarray of size ySize() at the postion [x]
	*/
#if defined(ARR_CHECK_BOUND)
	T* operator[](unsigned x) throw(...) {if(x>=m_uNumXElems) throw("out of bounds in array<>::operator[]"); return &(m_pVals[x*m_uNumXElems];}
#else
	// TODO: this is a bug, i think
	T* operator[](unsigned x) throw() {return &(m_pVals[x*m_uNumXElems]);}
#endif
	unsigned xSize() const {return m_uNumXElems;}
	unsigned ySize() const {return m_uNumYElems;}
	unsigned size() const {return m_uNumElems;}
	bool empty() const {return m_uNumElems==0;}
	/*!
	direct access to underlying elements

	these are size() values in consecutive order
	*/
	T* pVals() {return m_pVals;}
protected:
	unsigned m_uNumElems;
	unsigned m_uNumXElems;
	unsigned m_uNumYElems;
	T* m_pVals;
private:
	array2D();
	array2D(const array2D<T>& rOther);
	array2D<T>& operator=(const array2D<T>& rOther);
};

template <typename T,unsigned dim> class fixedArray
{
public:
	/*!
	creates the array. each element will be preset by it's default ctor, if any
	*/
	fixedArray(){;}
	/*!
	presets the array with val
	*/
	fixedArray(const T& val);
	~fixedArray(){;}
#if defined(ARR_CHECK_BOUND)
	T& operator[](unsigned index) throw(...) {if(index>=m_uNumElems) throw("out of bounds in array<>::operator[]"); return m_Vals[index];}
#else
	T& operator[](unsigned index) throw() {return m_Vals[index];}
	const T& operator[](unsigned index) const throw() {return m_Vals[index];}
#endif
	unsigned size() const {return dim;}
	fixedArray& operator=(const fixedArray& rOther);
	fixedArray(const fixedArray& rOther);
	/*!
	sets the elements directly

	note the caller has to pass enough values

	note passing NULL is NOP
	*/
	void set(const T* pVals);
	/*!
	direct access to underlying elements

	these are size() values in consecutive order
	*/
	T* pVals() {return &m_Vals[0];}
	/*!
	direct read access to underlying elements

	these are size() values in consecutive order
	*/
	const T* pcVals() const {return &m_Vals[0];}
protected:
	T m_Vals[dim];
};


