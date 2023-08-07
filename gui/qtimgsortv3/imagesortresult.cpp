#include "imagesortresult.h"

#if defined(QT_NO_DATASTREAM)
#error cannot compile with QT_NO_DATASTREAM
#endif

inline QDataStream& operator>>(QDataStream &in,imageSortCoor &r)
{
	in >> r.m_uX;
	in >> r.m_uY;
	in >> r.m_bValid;

	return in;
}

inline QDataStream& operator<<(QDataStream &out,const imageSortCoor &r)
{
	out << r.m_uX;
	out << r.m_uY;
	out << r.m_bValid;

	return out;
}

QDataStream& imageSortResult::load(QDataStream& iStr)
{
	QHash<QString,imageSortCoor>*p=(QHash<QString,imageSortCoor>*)this; 
	iStr >> *p;
	return iStr;
}

QDataStream& imageSortResult::save(QDataStream& oStr) const
{
	QHash<QString,imageSortCoor>*p=(QHash<QString,imageSortCoor>*)this;
	oStr << *p;
	return oStr;
}

