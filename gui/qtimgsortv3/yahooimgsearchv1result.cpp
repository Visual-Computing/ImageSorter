#include "yahooimgsearchv1result.h"

#if !defined(QT_NO_DATASTREAM)
QDataStream &operator<<(QDataStream &out, const yahooImgSearchV1Result &t)
{
	out << t.m_Url;
	out << t.m_ClickUrl;
	out << t.m_RefererUrl;
	out << t.m_ThumbnailUrl;
	return out;
}

QDataStream &operator>>(QDataStream &in,yahooImgSearchV1Result &t)
{
	in >> t.m_Url;
	in >> t.m_ClickUrl;
	in >> t.m_RefererUrl;
	in >> t.m_ThumbnailUrl;
	return in;
}
#endif