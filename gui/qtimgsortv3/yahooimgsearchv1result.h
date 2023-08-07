#pragma once
#include <QString>
class yahooImgSearchV1Result
{
public:
	yahooImgSearchV1Result() {;}
	~yahooImgSearchV1Result() {;}
	void clear() {m_Url.clear();m_ClickUrl.clear();m_RefererUrl.clear();m_ThumbnailUrl.clear();m_Height=m_Width=0;}
	// all public in this rapid version...
	QString m_Url;
	QString m_ClickUrl;
	QString m_RefererUrl;
	QString m_ThumbnailUrl;
	// TODO: this is also in thumbnail, so we should recode thumbnail
	// note 0 means: not available...
	unsigned m_Height;
	unsigned m_Width;
};

#if !defined(QT_NO_DATASTREAM)
#include <qtcore>
QDataStream &operator<<(QDataStream &, const yahooImgSearchV1Result &);
QDataStream &operator>>(QDataStream &, yahooImgSearchV1Result &);
#endif
