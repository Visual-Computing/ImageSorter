#include "previewcanvas.h"
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
PreviewCanvas::PreviewCanvas(QFrame *parent, Qt::WFlags f) : QFrame(parent, f) 
{
	setAutoFillBackground(true);
	m_BGColor = QColor(120,120,120);
	setPalette(QPalette(m_BGColor));
	m_bLoadFailure=false;
	m_LoadFailureImage=QImage(":/loadFailureImage");
}


QSize PreviewCanvas::minimumSizeHint() const 
{
	return QSize(50, 50);
}

QString PreviewCanvas::formatImageSize(qint64 size) 
{
	double s=(double)size;

	// return Bytes
	if (s < 1024.)
		return QString("%L1").arg(s,0,'f',0)+" Byte";


	// no rounding on KByte or MByte, just cutoff...

	// calculate KB
	s /= 1024.;

	// return KB
	if (s < 1000.)
		return QString("%L1").arg(s,0,'f',0)+" KByte";
	
	// calculate MB
	s /= 1024.;

	// return MB
	return QString("%L1").arg(s,0,'f',0)+" MByte";
}


void PreviewCanvas::paintEvent(QPaintEvent *e) 
{
	if(m_Img.isNull())
	{	
		QFrame::paintEvent(e);
		return;
	}
	// compute draw size
	computeSize();

	// canvas center
	const int centerX = (width() / 2); 
	const int centerY = (height() / 2);

	// draw position
	int x = centerX - drawWidth / 2;
	int y = centerY - drawHeight / 2;

	QRect target(x,y,drawWidth,drawHeight);
	QRect source(0,0,imgWidth,imgHeight);

	// TODOV4: don't use new here, just have a painter... 
	QPainter *painter = new QPainter(this);

	//// perhaps this should be an option (faster/ better preview)
	//if(m_bHighRes){
	//	QImage temp = m_Img.scaled(drawWidth, drawHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	//	painter->drawImage(target, temp, source);
	//		
	//}
	//else
	//{
		painter->drawImage(target, m_Img, source);
		//QFrame::paintEvent(e);
	//}
	delete painter;
	QFrame::paintEvent(e);
}


//void PreviewCanvas::enableHighResPreview(bool bEnable)
//{
//	m_bHighRes=bEnable;
//	repaint();
//}

void PreviewCanvas::setImageFileInfo(QFileInfo qFileInfo) 
{
	m_Img.load(qFileInfo.absoluteFilePath());
	if(m_Img.isNull()){
		m_bLoadFailure=true;
		m_Img=m_LoadFailureImage;
	}
	else
		m_bLoadFailure=false;

	imgWidth = m_Img.width();
	imgHeight = m_Img.height();
	
	imageInfo i;
	i.m_PathOrURL=qFileInfo.absoluteFilePath();
	
	i.m_FileName=qFileInfo.fileName();
	// TODOV4: don't use this...
	i.m_FileSize=this->formatImageSize(qFileInfo.size());
	i.m_XSize=imgWidth;
	i.m_YSize=imgHeight;
	i.m_bLoadFailure=m_bLoadFailure;
	i.m_bIsFromNet=false;
	i.m_bIsValid=true;

	emit showImageInfo(i);

	repaint();
}


void PreviewCanvas::setThumbnail(const thumbnail* pThumb) 
{
	if(!pThumb){
		m_Img=QImage();
		repaint();
		imageInfo i;
		i.m_bIsValid=false;
		emit showImageInfo(i);
		return;
	}

	// if the thumb is not loaded from the net,
	// load the image through the file info (this opens the image in original size...)
	if(!pThumb->isLoadedFromNet()){
		//setImageFileInfo(pThumb->m_FileInfo);
		QFileInfo finfo(pThumb->m_FileName);
		setImageFileInfo(finfo);
		return;
	}

	m_Img=pThumb->m_Img;
	assert(!m_Img.isNull());
	m_bLoadFailure=false;
	imgWidth = m_Img.width();
	imgHeight = m_Img.height();
	
	imageInfo i;
	
	if(pThumb->isFromYahoo()){
		i.m_XSize = pThumb->m_YahooURLs.m_Width;
		i.m_YSize = pThumb->m_YahooURLs.m_Height;
		i.m_PathOrURL=pThumb->m_YahooURLs.m_ClickUrl;
	}
	else{
		i.m_XSize = 0;
		i.m_YSize = 0;
		i.m_PathOrURL=pThumb->m_flickrURLs.clickURL();
	}

	i.m_bIsFromNet=true;
	i.m_bLoadFailure=false;
	i.m_FileSize="";
	i.m_LastModified=QDateTime();
	i.m_bIsValid=true;

	emit showImageInfo(i);

	repaint();
}


void PreviewCanvas::clearCanvas() 
{
	m_Img=QImage();
	m_bLoadFailure=false;
	repaint();
	imageInfo i;
	i.m_bIsValid=false;
	emit showImageInfo(i);
}

void PreviewCanvas::computeSize() 
{
	double tmpW = (double)width() / imgWidth;
	double tmpH = (double)height() / imgHeight;

	double zoom = (tmpW < tmpH) ? tmpW : tmpH;
	zoom = (zoom > 1) ? 1.0 : zoom;

	drawWidth = imgWidth * zoom;
	drawHeight = imgHeight * zoom;
}


void PreviewCanvas::setBackgroundColor(const QColor &color)
{
	m_BGColor=color;
	setPalette(QPalette(m_BGColor));
}