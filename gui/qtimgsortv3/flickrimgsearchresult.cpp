#include "flickrimgsearchresult.h"

#if !defined(QT_NO_DATASTREAM)
QDataStream &operator<<(QDataStream &out, const flickrImgSearchResult &t)
{
	out << t.m_Id;
	out << t.m_Owner;
	out << t.m_Secret;
	out << t.m_Server;
	out << t.m_Farm;
	out << t.m_Title;
	out << t.m_bPublic;
	out << t.m_bFriend;
	out << t.m_bFamily;
	return out;
}

QDataStream &operator>>(QDataStream &in,flickrImgSearchResult &t)
{
	in >> t.m_Id;
	in >> t.m_Owner;
	in >> t.m_Secret;
	in >> t.m_Server;
	in >> t.m_Farm;
	in >> t.m_Title;
	in >> t.m_bPublic;
	in >> t.m_bFriend;
	in >> t.m_bFamily;
	return in;
}
#endif