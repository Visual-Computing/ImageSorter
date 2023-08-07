/*
V1: initial version, used up to ImageSorter V4.2 beta, using 99 float features
V2: version used in ImageSorter V5, using 44 features of type TDATA, took out pause option (never used), throwing
std::bad_alloc on all memory allocation failures
*/
#if !defined(HIERARCHIC_SOM_H)
#define HIERARCHIC_SOM_H

#include <vector>
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
the type for the callback to be registered by som<T>::registerCallback()
*/
typedef void(*somCallBack)(void *pUserData);

/*!
the type for the callback to be registered by som<T>::registerPauseCallback()
*/
typedef void(*somPauseCallBack)(void *pUserData);

/*!
a hierarchic som (self organizing map)

type T is the type for internal calculations (in general, float or double)

type TData is the type of the featuredata encapsulated in TSort

type TSort is a type wich implements the following functions:

	const TData *featureData() const;

	unsigned __int32 featureDataByteSize();

	unsigned __int32 featureDataSize();

this type thus contains the data to be sorted.

type TMutex is a type which implements a mutex. TMutex::lock() and TMutex::unlock()
are required and have to operate like a mutex. see init()
*/

template <typename T,typename TDATA,typename TSORT,typename TMUTEX> class hierarchicSom
{
public:
	// V2: took out pPause parameter, never really used
	hierarchicSom(std::vector<TSORT> *pSortData,TMUTEX* pMutex=NULL,bool *pAbort=NULL/*,bool *pPause=NULL*/);
	~hierarchicSom();
	void registerCallback(somCallBack pCallBack,void* pUserData,unsigned interval);
	/*
	V2: took out, never really used
	void registerPauseCallback(somPauseCallBack pCallBack,void* pUserData);
	*/
	// call before sort()
	// call again if mapPlacesX, mapPlacesY changed before re sort()
	// note throws std::bad_alloc if allocation fails
	bool init(unsigned mapPlacesX,unsigned mapPlacesY);
	// returns true if initilaized
	bool initialized() const {return m_bInitialized;}
	// returns false if allocation fails
	// note throws std::bad_alloc if allocation fails
	bool sort();
	// returns the mapPlaces or 0 if !initilaized()
	unsigned mapPlacesX(void) const {return m_pMapPlacesX?m_pMapPlacesX[0]:0;}
	unsigned mapPlacesY(void) const {return m_pMapPlacesY?m_pMapPlacesY[0]:0;}

private:
	// note no copy ctor, no assignment
	hierarchicSom(const hierarchicSom& rOther);
	const hierarchicSom& operator=(const hierarchicSom& rOther);

	std::vector<TSORT> *m_pSortData;

	// weight for a assigned feature vector, unassigned weight is 1
	static const unsigned m_Weigth;
	// weight for Color Layout Component
	static const T m_wCL;
	// Number of Color Layout comonents in the feature vector
	static const unsigned m_ColorLayoutLen; 
	// factor to compute the initial filter radius (half of the SOM size)
	static const T m_R0;
	static const T	m_UpdateDiv; 
	static const unsigned m_MaxIter;
	static const int m_dx[9];
	static const int m_dy[9];
	
	unsigned m_HierarchySteps; // hierarchy steps 
	
	// dim: 54
	static const unsigned m_Dims[54];
	unsigned m_ActDim;
	unsigned m_NextDim;
	
	T m_InitialRadiusX;  // initial filter radius
	T m_InitialRadiusY;

	// KAI: signed
	// SEPP: no, unsigned
	unsigned m_ActRadiusX, m_ActRadiusY;  // the actual filter radius

	unsigned *m_pMapPlacesX;
	unsigned *m_pMapPlacesY;
	unsigned *m_pMapPlaces;
	unsigned m_TotalMapPlaces;		// used for allocating the multidim arrays below..
	unsigned m_TotalMapPlacesX;
	unsigned m_TotalMapPlacesY;

	
	T*** m_pNet;					// SOM net
	T** m_pNets;					// the underlying 2nd dim buffer
	T* m_pNodes;					// the underlying nodes buffer (3rd dim)

	// note these where signed in Kai's java code, i think (?) these can be unsigned here
	unsigned** m_pCount;			// count of taken places
	unsigned* m_pCounts;			// underlying buffer

	unsigned** m_pCapacity;			// number of places of the next level
	unsigned* m_pCapacities;		// underlying buffer

	unsigned** m_pCapacityX;		// number of places of the next level
	unsigned* m_pCapacitiesX;		// underlying buffer
	unsigned** m_pCapacityY;		// number of places of the next level
	unsigned* m_pCapacitiesY;		// underlying buffer
	
	unsigned* m_pWeight;			// weights for the filtering
	
	T** m_pNetI;					// Integral image of the SOM net
	T* m_pNetIs;						// underlying buffer
	unsigned m_uNetISize;
	// note leave these signed, check if ever negative...
	int* m_pWeightI;				// Integral image of the weights for the filtering

	bool m_bInitialized;

	void dealloc();

	bool calculateFeatureIntegralImage(); 
	void calculateWeightIntegralImage();
	void averageNet(unsigned h);

	// a callback to call each m_uIntervall sort steps
	somCallBack m_pCallBack;
	// the user data to call the callback with
	void *m_pUserData;
	// the interval the callback is called
	unsigned m_uInterval;

	/*
	V2: took this out, never really used
	// a callback to call in sort steps
	// this either returns immediately or pauses the thread
	// note the callback *has to* run in the same thread as this
	somPauseCallBack m_pPauseCallBack;
	// the user data to call the callback with
	void *m_pPauseUserData;
	*/

	// a pointer to a general mutex. this has to implement lock()
	// and unlock()
	TMUTEX* m_pMutex;
	// a pointer to a bool guarded by a TMutex
	// if both m_pMutex and m_pAbort are given in init(),
	// the sort can be aborted (from a different thread)
	bool *m_pAbort;
	/*
	V2: took this out, never really used
	// a pointer to a bool guarded by a TMutex
	// if both m_pMutex and m_pPause are given in init() and if the pause callback is registered,
	// the sort can be paused (from a different thread)
	bool *m_pPause;
	*/

};
	


#endif