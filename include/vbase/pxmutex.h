#if !defined(PXMUTEX_H)
#define PXMUTEX_H

#include "pxdefs.h"

namespace isortisearch {

/*!
This is a base wrapper class for a mutex. 

A client for isortisearch::sorter or isortisearch::searcher has to implement a derivate of mutex, if a %mutex is required.

The virtual lock() and unlock() members of the derivate have to wrap to the %mutex implementation of the clients platform.

For a discussion when a %mutex is required, see isortisearch::sorter::setMutex(), isortisearch::searcher::setMutex() or isortisearch::dynamicSearcher::setMutex().
*/
class ISORT_API mutex{
public:
	/*!
	Constructor
	*/
	mutex(){;}
	/*!
	Destructor
	*/
	~mutex(){;}
	/*!
	The client has to implement this in
	it's derived mutex class. This base class version simply does nothing.
	*/
	virtual void lock() {;}
	/*!
	The client has to implement this in
	it's derived mutex class. This base class version simply does nothing.
	*/
	virtual void unlock() {;}
};

}
#endif
