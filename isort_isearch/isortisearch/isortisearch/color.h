#pragma once
/*!
a template struct for rgb triples
*/
template <class T> struct rgb
{
	// rgb() : m_red(T(0)),m_green(T(0)),m_blue(T(0)) {;}
	rgb() {;}
	rgb(T r,T g,T b) : m_red(r),m_green(g),m_blue(b) {;}
	T m_red;
	T m_green;
	T m_blue;
	bool operator!=(const rgb<T>& rOther) {return (m_red!=rOther.m_red||m_green!=rOther.m_green||m_blue!=rOther.m_blue);}
};

typedef rgb<unsigned char> byteRGB;
typedef rgb<double> doubleRGB;
typedef rgb<int> intRGB;
