#include "arrayv5.h"
#include <assert.h>
#include <string>
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
template <typename T> arrayV5<T>::arrayV5(unsigned sz) : m_uNumElems(sz),m_pVals(NULL)
{
	m_pVals=new T[m_uNumElems];
}

#pragma intrinsic(memcpy)

template <typename T> arrayV5<T>::arrayV5(const arrayV5<T>& rOther)
{
	if(rOther.m_pVals){
		assert(rOther.size());
		m_pVals=new T[rOther.size()];
		memcpy(m_pVals,rOther.m_pVals,rOther.size()*sizeof(T));
		m_uNumElems=rOther.size();
	}
	else{
		assert(false);
		m_pVals=NULL;
		m_uNumElems=0;
	}
}

template <typename T> arrayV5<T>& arrayV5<T>::operator=(const arrayV5<T>& rOther)
{
	if(&rOther==this)
		return *this;
	resize(rOther.size());
	memcpy(m_pVals,rOther.m_pVals,rOther.size()*sizeof(T));
	return *this;
}

template <typename T> void arrayV5<T>::resize(unsigned sz)
{
	if(sz==m_uNumElems)
		return;
	if(m_pVals){
		delete[] m_pVals;
		m_pVals=NULL;
	}
	m_pVals=new T[sz];
	m_uNumElems=sz;
}


template <typename T> arrayV5<T>::arrayV5(unsigned sz,const T& val) : m_uNumElems(sz),m_pVals(NULL)
{
	m_pVals=new T[m_uNumElems];
	const unsigned s=size();
	for(unsigned i=0;i<s;++i)
		m_pVals[i]=val;
}


template <typename T> arrayV5<T>::~arrayV5()
{
	if(m_pVals)
		delete[] m_pVals;
}

template <typename T> void array2DV5<T>::resize(unsigned x,unsigned y)
{
	if(x==m_uNumXElems&&y==m_uNumYElems)
		return;

	if(m_pVals){
		delete[] m_pVals;
		m_pVals=NULL;
	}

	m_uNumXElems=x;
	m_uNumYElems=y;
	m_uNumElems=x*y;

	if(m_uNumElems){
		m_pVals=new T[m_uNumElems];
	}
}


template <typename T> array2DV5<T>::array2DV5(unsigned x,unsigned y) : m_uNumXElems(x),m_uNumYElems(y),m_uNumElems(x*y)
{
	m_pVals=NULL;
	if(m_uNumElems){
		m_pVals=new T[m_uNumElems];
	}
}

template <typename T> array2DV5<T>::array2DV5(unsigned x,unsigned y,const T& val) : m_uNumXElems(x),m_uNumYElems(y),m_uNumElems(x*y)
{
	m_pVals=NULL;
	m_pVals=new T[m_uNumElems];
	set(val);
}

template <typename T> array2DV5<T>::~array2DV5()
{
	if(m_pVals)
		delete[] m_pVals;
}

template <typename T> void array3DV5<T>::resize(unsigned x,unsigned y,unsigned z)
{
	if(x==m_uNumXElems&&y==m_uNumYElems&&z==m_uNumZElems)
		return;

	if(m_pVals){
		delete[] m_pVals;
		m_pVals=NULL;
	}

	m_uNumXElems=x;
	m_uNumYElems=y;
	m_uNumZElems=z;
	m_uNumElems=x*y*z;
	m_uNumXYElems=x*y;

	if(m_uNumElems){
		m_pVals=new T[m_uNumElems];
	}
}

template <typename T> void array3DV5<T>::set(const T& val)
{
	if(!m_pVals)
		return;

	unsigned i;
	const unsigned s=size();
	for(i=0;i<s;i++)
		m_pVals[i]=val;
}



template <typename T> array3DV5<T>::array3DV5(unsigned x,unsigned y,unsigned z) : m_uNumXElems(x),m_uNumYElems(y),m_uNumZElems(z),m_uNumXYElems(x*y),m_uNumElems(x*y*z)
{
	m_pVals=NULL;
	if(m_uNumElems){
		m_pVals=new T[m_uNumElems];
	}
}

template <typename T> array3DV5<T>::array3DV5(unsigned x,unsigned y,unsigned z,const T& val) : m_uNumXElems(x),m_uNumYElems(y),m_uNumZElems(z),m_uNumXYElems(x*y),m_uNumElems(x*y*z)
{
	m_pVals=NULL;
	m_pVals=new T[m_uNumElems];
	unsigned i;
	const unsigned s=size();
	for(i=0;i<s;i++)
		m_pVals[i]=val;
}

template <typename T> array3DV5<T>::~array3DV5()
{
	if(m_pVals)
		delete[] m_pVals;
}

template <typename T,unsigned dim> fixedArrayV5<T,dim>::fixedArrayV5(const T& val)
{
	const unsigned s=size();
	for(unsigned i=0;i<s;++i)
		m_Vals[i]=val;
}

template <typename T,unsigned dim> fixedArrayV5<T,dim>& fixedArrayV5<T,dim>::operator=(const fixedArrayV5& rOther)
{
	if(&rOther==this)
		return *this;
	
	memcpy(m_Vals,rOther.m_Vals,sizeof(m_Vals));

	return *this;
}

template <typename T,unsigned dim> fixedArrayV5<T,dim>::fixedArrayV5(const fixedArrayV5& rOther)
{
	if(&rOther==this)
		return;
	
	memcpy(m_Vals,rOther.m_Vals,sizeof(m_Vals));

	return;
}

template <typename T,unsigned dim> void fixedArrayV5<T,dim>::set(const T* pVals)
{
	if(!pVals)
		return;
	
	memcpy(m_Vals,pVals,sizeof(m_Vals));
}

#pragma function(memcpy)

// excplicit instantiation
#include "arrayv5.expl_templ_inst"

#if defined(_LEAK_H)
	#define new new
#endif
