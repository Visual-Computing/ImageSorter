#pragma once

#include <stdlib.h>  // for NULL
/*!
define ARRAY_CHECK_BOUNDS to do bounds check in operator[], 
this will throw an exception if the index is exceeded then
*/
// #define ARRAY_CHECK_BOUNDS

/*!
an arrayV5 with a fixed but variable number of elements of type T

use operator[] to read/write access elemts.
use at() to read access elements.
these may throw an exception if ARR_CHECK_BOUNDS is defined and the index is exceeding the arrayV5
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
arrayV5<int> values(numValues);
\endcode

TODO: how to pass args to the T ctor?
*/
template <typename T> class arrayV5
{
public:
	/*!
	creates an empty array

	use resize() to set size
	*/
	arrayV5() : m_uNumElems(0),m_pVals(NULL) {;}
	/*!
	resizes the array

	note throws std::bad_alloc if allocation fails
	*/
	void resize(unsigned sz);
	/*!
	creates the arrayV5. each element will be preset by it's default ctor, if any

	note it is allowed to pass 0, but not very useful...

	note throws std::bad_alloc if allocation fails
	*/
	arrayV5(unsigned sz);
	/*!
	presets the arrayV5 with val

	note throws std::bad_alloc if allocation fails
	*/
	arrayV5(unsigned sz,const T& val);
	/*!
	dtor
	*/
	~arrayV5();
	/*!
	copy ctor

	note throws std::bad_alloc if allocation fails
	*/
	arrayV5(const arrayV5<T>& rOther);
	/*!
	assignment
	*/
	arrayV5<T>& operator=(const arrayV5<T>& rOther);

#if defined(ARR_CHECK_BOUNDS)
	T& operator[](unsigned index) throw(...) {if(index>=m_uNumElems) throw("out of bounds in arrayV5<>::operator[]"); return m_pVals[index];}
	const T& at(unsigned index) throw(...) {if(index>=m_uNumElems) throw("out of bounds in arrayV5<>::operator[]"); return m_pVals[index];}
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
};


/*!
a 2 dimensional array with fixed but variable number of elements per row and column of type T

use tAt() to access elemts

note: how can i implement operator[][] ???

this may throw an exception if ARR_CHECK_BOUNDS is defined and the index is exceeding the arrayV5 bounds
note the index is zero based

call size() to get the number of elements

call xSize() to get the number of 'rows'

call ySize() to get the number of 'columns'

call operator[] to get a pointer to T to the 'row' number x (a 1d arrayV5 of size ySize()...)

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
array2DV5<int> values(numXValues,numYValues);
\endcode

*/
template <typename T> class array2DV5
{
public:
	/*!
	creates an empty array2DV5

	use resize() to set the sizes
	*/
	array2DV5() : m_uNumElems(0),m_uNumXElems(0),m_uNumYElems(0),m_pVals(NULL) {;}
	/*!
	resizes the array2DV5

	note throws std::bad_alloc if allocation fails
	*/
	void resize(unsigned x,unsigned y);
	/*!
	creates the array2DV5. each element will be preset by it's default ctor, if any

	note it is allowed to pass 0, but not very useful...

	note throws std::bad_alloc if allocation fails
	*/
	array2DV5(unsigned x,unsigned y);
	/*!
	presets the arrayV5 with val

	note throws std::bad_alloc if allocation fails
	*/
	array2DV5(unsigned x,unsigned y,const T& val);
	/*!
	dtor
	*/
	~array2DV5();
	/*!
	returns a reference to the T at postion (x,y)
	*/
#if defined(ARR_CHECK_BOUND)
	T& at(unsigned x,unsigned y) throw(...) {if(x>=m_uNumXElems||y>=m_uNumYElems) throw("out of bounds in array2DV5<>::at()"); return m_pVals[x*m_uNumYElems+y];}
#else
	T& at(unsigned x,unsigned y) throw() {return m_pVals[x*m_uNumYElems+y];}
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
	array2DV5(const array2DV5<T>& rOther);
	array2DV5<T>& operator=(const array2DV5<T>& rOther);
};

/*!
a 3 dimensional arrayV5 with fixed but variable number of elements per row and column of type T

use tAt()) to access elemts

note: how can i implement operator[][] ???

this may throw an exception if ARR_CHECK_BOUNDS is defined and the index is exceeding the arrayV5 bounds
note the index is zero based

call size() to get the number of elements

call xSize() to get the number of 'rows'

call ySize() to get the number of 'columns'

call operator[] to get a pointer to T to the 'row' number x (a 1d arrayV5 of size ySize()...)

note that this is a convenience class to deal with the fact that you cannot write c++ code like

\code
unsigned numXValues=someUnsignedReturningFunction();
unsigned numYValues=someUnsignedReturningFunction();
unsigned numZValues=someUnsignedReturningFunction();
int values[numXValues][numYValues][numZValues];
\endcode

because numXValues and numYValues are not const. furthermore, for multidimensional arrays,
you cannot even use

\code
unsigned numXValues=someUnsignedReturningFunction();
unsigned numYValues=someUnsignedReturningFunction();
unsigned numZValues=someUnsignedReturningFunction();
int* pValues = new int[numXValues][numYValues][numZValues];
.
.
.
delete[] pValues;
\endcode

because numYValues and numZvalues are not const.

you can use:

\code
unsigned numXValues=someUnsignedReturningFunction();
unsigned numYValues=someUnsignedReturningFunction();
unsigned numZValues=someUnsignedReturningFunction();
array3DV5<int> values(numXValues,numYValues,numZValues);
\endcode

*/
template <typename T> class array3DV5
{
public:
	/*!
	creates an empty array3DV5

	use resize() to set the sizes
	*/
	array3DV5() : m_uNumElems(0),m_uNumXElems(0),m_uNumYElems(0),m_uNumZElems(0),m_uNumXYElems(0),m_pVals(NULL) {;}
	/*!
	resizes the array3DV5

	note throws std::bad_alloc if allocation fails
	*/
	void resize(unsigned x,unsigned y,unsigned z);
	/*!
	sets each elem to val
	*/
	void set(const T& val);
	/*!
	creates the array3DV5. each element will be preset by it's default ctor, if any

	note it is allowed to pass 0, but not very useful...

	note throws std::bad_alloc if allocation fails
	*/
	array3DV5(unsigned x,unsigned y,unsigned z);
	/*!
	presets the array3DV5 with val

	note throws std::bad_alloc if allocation fails
	*/
	array3DV5(unsigned x,unsigned y,unsigned z,const T& val);

	/*!
	dtor
	*/
	~array3DV5();
	/*!
	returns a reference to the T at postion (x,y)
	*/
#if defined(ARR_CHECK_BOUND)
	T& at(unsigned x,unsigned y,unsigned z) throw(...) {if(x>=m_uNumXElems||y>=m_uNumYElems||z>=m_uNumZElems) throw("out of bounds in array3DV5<>::at()"); return m_pVals[z*(m_uNumXYElems)+x*m_uNumYElems+y];}
#else
	T& at(unsigned x,unsigned y,unsigned z) throw() {return m_pVals[z*m_uNumXYElems+x*m_uNumYElems+y];}
#endif
	unsigned xSize() const {return m_uNumXElems;}
	unsigned ySize() const {return m_uNumYElems;}
	unsigned zSize() const {return m_uNumZElems;}
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
	unsigned m_uNumZElems;
	unsigned m_uNumXYElems;
	T* m_pVals;
private:
	array3DV5(const array3DV5<T>& rOther);
	array3DV5<T>& operator=(const array3DV5<T>& rOther);
};


template <typename T,unsigned dim> class fixedArrayV5
{
public:
	/*!
	creates the fixedArrayV5. each element will be preset by it's default ctor, if any
	*/
	fixedArrayV5(){;}
	/*!
	presets the fixedArrayV5 with val
	*/
	fixedArrayV5(const T& val);
	/*!
	dtor
	*/
	~fixedArrayV5(){;}
#if defined(ARR_CHECK_BOUND)
	T& operator[](unsigned index) throw(...) {if(index>=m_uNumElems) throw("out of bounds in arrayV5<>::operator[]"); return m_Vals[index];}
#else
	T& operator[](unsigned index) throw() {return m_Vals[index];}
	const T& operator[](unsigned index) const throw() {return m_Vals[index];}
#endif
	unsigned size() const {return dim;}
	/*!
	assignment
	*/
	fixedArrayV5& operator=(const fixedArrayV5& rOther);
	/*!
	copy ctor
	*/
	fixedArrayV5(const fixedArrayV5& rOther);
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


