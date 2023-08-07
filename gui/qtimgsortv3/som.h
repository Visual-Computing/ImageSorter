#pragma once
#include "array.h"
#include <math.h>

//typedef fixedarray<T,dim>* ;
/*!
a dummy mutex to instantiate the som<> when a 'real' mutex is not needed
*/
class somDummyMutex
{
public:
	somDummyMutex() {;}
	~somDummyMutex() {;}
	void lock() {;}
	void unlock() {;}
};

/*!
a node of the som

m_uX and m_uY are the coordinates of the node
m_Data is the node data of type T and of dimension dim
dimension() returns the dimension
*/

template <typename T,unsigned dim> class somNode
{
public:
	somNode() : m_uX(0), m_uY(0) {;}
	somNode(const somNode& rOther);
	~somNode() {;}
	somNode& operator=(const somNode& rOther);
	unsigned m_uX;
	unsigned m_uY;
	fixedArray<T,dim> m_Data;
	unsigned dimension() const {return dim;}
	T& operator[](unsigned index) {return m_Data[index];}
};

/*!
the type for the callback to be registered by som<T>::registerCallback()
*/
typedef void(*somCallBack)(void *pUserData);

/*!
the type for the callback to be registered by som<T>::registerPauseCallback()
*/
typedef void(*somPauseCallBack)(void *pUserData);

/*!
a super class for the som<> template. 

defines static functions for the som
*/
class somStatics
{
public:
	somStatics() {;}
	~somStatics() {;}
	static unsigned mapPlaces(unsigned numItems,double dimFactor) {return (unsigned)ceil(sqrt((double)numItems*dimFactor));}
};
/*!
a som (self organizing map)

type T is the type of the data to be ordered
dim is the dimension of the vectors to be ordered

TMutex is a class which implements a mutex. TMutex::lock() and TMutex::unlock()
are required and have to operate like a mutex. see init()
*/
template <typename T,typename TMutex,unsigned dim> class som : public somStatics
{
public:
	som();
	~som() {_destroy();}
	/*
	inits the som

	rNodes are the nodes to sort

	pMutex is a ptr to a TMutex instance, pAbort a ptr to a bool flag, pPause 
	a ptr to another bool flag.

	if you want to abort train() (which does all the work) from another thread,
	lock() the TMutex instance, set the abort flag true and unlock() the TMutex
	instance from within the other thread. 

	if you want to pause train() from another thread, lock() the TMutex instance, 
	set the pause flag true and unlock() the TMutex instance from within the other thread.

	train() checks for the presence of all three ptrs. if they pMutex and pAbort or pPause are
	set, it will peridically lock(), test the flags, and unlock().

	if the abort flag was set, it will return false.
	if the abort flag was not set, but the pause flag was set, it will call the registered pause callback
	if you don't need abortion at all, pass NULL ptrs for these three parameters.

	note that even in this case you need a dummy TMutex class anyway to instantiate 
	the som template, so you may use class somDummyMutex.
	*/
	void init(array<somNode<T,dim> >& rNodes,TMutex* pMutex=0,bool *pAbort=0,bool *pPause=0,bool bRandom=true);
	void reset() {_destroy();}
	unsigned xDim() const {return m_uXDim;}
	unsigned yDim() const {return m_uYDim;}
	unsigned size() const {return m_uSize;}
	unsigned nodeDim() const {return dim;}
	unsigned numNodes() const {return m_uNumNodes;}
	void setIters(unsigned iters) {m_uIters=iters;}
	unsigned iters() const {return m_uIters;}
	// void setChangedThreshold(T changed) {m_changedThreshold=changed;}
	// T changedThreshold() const {return m_changedThreshold;}
	somNode<T,dim>& operator[](unsigned index) {return m_pNodes[index];}
	bool initialized() const {return m_bInitialized;}
	/*!
	returns true if done

	returns false if !initialized() or if aborted, see init()
	*/
	bool train(void);
	void setDimFactor(double f) {f=(f<1.0)?1.0:f; m_dDimFactor=f;}
	void setApectRatio(double r) {r=(r<=0.0)?1.0:r; m_dAspectRatio=r;}
	void registerCallback(somCallBack pCallBack,void* pUserData,unsigned interval);
	void registerPauseCallback(somPauseCallBack pPauseCallBack,void* pUserData);
protected:
	unsigned m_uXDim;
	unsigned m_uYDim;
	unsigned m_uSize;
	unsigned m_uIters;
	// T m_changedThreshold;
	T m_r0;
	T m_a0;
	T m_kA;
	T m_kR;
	// T m_nbrRadius;
	bool *m_pTaken;
	unsigned m_uNumNodes;
	somNode<T,dim> *m_pNodes;
	fixedArray<T,dim> *m_pNet;

	// Kai
	fixedArray<T,dim> **m_pNetUpdate;
	fixedArray<T,dim> *m_pNetUpdateHor;
	unsigned *m_pWeight;
	unsigned *m_pWeightHor;
	T *m_pSum;

	
	bool m_bInitialized;
	void _destroy();
	// note this changes state during train()...
	bool m_bTakenState;
	// a factor to make the som a bit bigger than the nodes passed
	double m_dDimFactor;
	// the aspect ratio (x/y) of the map
	double m_dAspectRatio;

	// a callback to call each m_uIntervall train steps
	somCallBack m_pCallBack;
	// the user data to call the callback with
	void *m_pUserData;
	// the interval the callback is called
	unsigned m_uInterval;

	// a callback to call in train steps
	// this either returns immediately or pauses the thread
	// note the callback *has to* run in  the same thread as this
	somPauseCallBack m_pPauseCallBack;
	// the user data to call the callback with
	void *m_pPauseUserData;
	// a pointer to a general mutex. this has to implement lock()
	// and unlock()
	TMutex* m_pMutex;
	// a pointer to a bool guarded by a TMutex
	// if both m_pMutex and m_pAbort are given in init(),
	// the sort can be aborted (from a different thread)
	bool *m_pAbort;
	// a pointer to a bool guarded by a TMutex
	// if both m_pMutex and m_pPause are given in init() and if the pause callback is registered,
	// the sort can be paused (from a different thread)
	bool *m_pPause;
private:
	som(const som<T,TMutex,dim>& rOther){;}
	som<T,TMutex,dim>& operator=(const som<T,TMutex,dim>& rOther) {return *this;}
};
