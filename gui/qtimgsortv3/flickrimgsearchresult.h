#pragma once
#include <QString>
class flickrImgSearchResult
{
public:
	flickrImgSearchResult() {;}
	~flickrImgSearchResult() {;}
	void clear() {m_Id.clear();m_Owner.clear();m_Server.clear();m_Farm.clear();m_Title.clear();m_Secret.clear();m_bPublic=m_bFriend=m_bFamily=FALSE;}
	// all public in this rapid version...
	QString m_Id;
	QString m_Owner;
	QString m_Secret;
	QString m_Server;
	QString m_Farm;
	QString m_Title;
	bool m_bPublic;
	bool m_bFriend;
	bool m_bFamily;
	// note the _t in the end denotes thumbnails with 100 px longer side, the _m thumbnails with 240 px longer side
	QString thumbnailURL() const {return "http://farm"+m_Farm+".static.flickr.com/"+m_Server+"/"+m_Id+"_"+m_Secret+"_m.jpg";}
	QString clickURL() const {return "http://www.flickr.com/photos/"+m_Owner+"/"+m_Id;}
};


#if !defined(QT_NO_DATASTREAM)
#include <qtcore>
QDataStream &operator<<(QDataStream &, const flickrImgSearchResult &);
QDataStream &operator>>(QDataStream &, flickrImgSearchResult &);
#endif
