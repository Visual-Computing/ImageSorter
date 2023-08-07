#pragma once

#include <QtCore>

#if defined(QT_NO_DATASTREAM)
#error cannot compile with QT_NO_DATASTREAM
#endif

class imageSortCoor
{
public:
	imageSortCoor() : m_uX(0),m_uY(0) {;}
	uint m_uX;
	uint m_uY;
	bool m_bValid;
};


class imageSortResult: public QHash<QString,imageSortCoor>
{
public:
	imageSortResult() {;}
	~imageSortResult() {;}
	QDataStream& load(QDataStream& iStr); 
	QDataStream& save(QDataStream& oStr) const;
};

QDataStream& operator>>(QDataStream &in,imageSortCoor &r);
QDataStream& operator<<(QDataStream &out,const imageSortCoor &r);

