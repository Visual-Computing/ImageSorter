#include "array.h"
#include "imageerror.h"
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
template <typename T> array<T>::array(unsigned sz) : m_uNumElems(sz)
{
	m_pVals=NULL;
	m_pVals=new T[m_uNumElems];
	if(!m_pVals)
		throw fqImageError(imageError::badAlloc);
}


template <typename T> array<T>::array(unsigned sz,const T& val) : m_uNumElems(sz)
{
	m_pVals=NULL;
	m_pVals=new T[m_uNumElems];
	if(!m_pVals)
		throw fqImageError(imageError::badAlloc);
	unsigned i;
	const unsigned s=size();
	for(i=0;i<s;i++)
		m_pVals[i]=val;
}


template <typename T> array<T>::~array()
{
	if(m_pVals)
		delete[] m_pVals;
}

template <typename T,unsigned dim> fixedArray<T,dim>::fixedArray(const T& val)
{
	unsigned i;
	const unsigned s=size();
	for(i=0;i<s;i++)
		m_Vals[i]=val;
}

template <typename T,unsigned dim> fixedArray<T,dim>& fixedArray<T,dim>::operator=(const fixedArray& rOther)
{
	if(this!=&rOther)
		memcpy(m_Vals,rOther.m_Vals,sizeof(m_Vals));

	return *this;
}

 
template <typename T> array2D<T>::array2D(unsigned x,unsigned y) : m_uNumXElems(x),m_uNumYElems(y),m_uNumElems(x*y)
{
	m_pVals=NULL;
	if(m_uNumElems){
		m_pVals=new T[m_uNumElems];
		if(!m_pVals)
			throw fqImageError(imageError::badAlloc);
	}
}

template <typename T> array2D<T>::array2D(unsigned x,unsigned y,const T& val) : m_uNumXElems(x),m_uNumYElems(y),m_uNumElems(x*y)
{
	m_pVals=NULL;
	m_pVals=new T[m_uNumElems];
	if(!m_pVals)
		throw fqImageError(imageError::badAlloc);
	unsigned i;
	const unsigned s=size();
	for(i=0;i<s;i++)
		m_pVals[i]=val;
}

template <typename T> array2D<T>::~array2D()
{
	if(m_pVals)
		delete[] m_pVals;
}


template <typename T,unsigned dim> void fixedArray<T,dim>::set(const T* pVals)
{
	if(!pVals)
		return;
	
	memcpy(m_Vals,pVals,sizeof(m_Vals));
}

#pragma function(memcpy)

#if defined(_LEAK_H)
	#define new new
#endif
// excplicit instantiation
#include "array.expl_templ_inst"

