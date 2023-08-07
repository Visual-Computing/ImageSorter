#include "imageio.h"
#if defined(_WIN32)	// note also defined on win64... 
#include "leak.h"
#endif
#if defined(USETIFF)
// initialize statics
unsigned imageIO::m_uId=0;
#endif

// use inline memcpy() for this source file
#pragma intrinsic(memcpy)

imageIO::imageIO()
{
#if defined(USETIFF)
	++m_uId;
	if(m_uId==1){
		m_oldTiffWarnHandler=TIFFSetWarningHandler(NULL);
		m_oldTiffErrHandler=TIFFSetErrorHandler(NULL);
	}
#endif
}
imageIO::~imageIO()
{
#if defined(USETIFF)
	--m_uId;
	if(!m_uId){
		TIFFSetWarningHandler(m_oldTiffWarnHandler);
		TIFFSetErrorHandler(m_oldTiffErrHandler);
	}
#endif
}

#if defined(USETIFF)
imageIO::TiffTags imageIO::getTiffTags(const char *name)
{
	if(!name)
		throw fqImageError(imageError::badParameter);

    TIFF *pTiff=TIFFOpen(name,"r");

	if(!pTiff)
		throw fqImageError(imageError::tiffOpen);

	TiffTags tags;
	getTiffTags(pTiff,&tags);

	TIFFClose(pTiff);

	return tags;
}


void imageIO::getTiffTags(TIFF *pTiff,TiffTags* pTags)
{
	if(!pTiff||!pTags)
		throw fqImageError(imageError::badParameter);

	// note tifflib documentation states to use a value only if TIFFGetField() returns 1
	uint16 bps,cmp,pmt,spp,pcf;
	uint32 w,h;
    if(TIFFGetField(pTiff,TIFFTAG_BITSPERSAMPLE,&bps)!=1||
       TIFFGetField(pTiff,TIFFTAG_COMPRESSION,&cmp)!=1||
	   TIFFGetField(pTiff,TIFFTAG_PHOTOMETRIC,&pmt)!=1||
	   TIFFGetField(pTiff,TIFFTAG_SAMPLESPERPIXEL,&spp)!=1||
	   TIFFGetField(pTiff,TIFFTAG_PLANARCONFIG,&pcf)!=1||
	   TIFFGetField(pTiff,TIFFTAG_IMAGEWIDTH,&w)!=1||
       TIFFGetField(pTiff,TIFFTAG_IMAGELENGTH,&h)!=1){
		throw fqImageError(imageError::tiffRead);
	}

	pTags->valid=true;
	// bytes per channel: currently one
	switch(bps){
		case 8:
			pTags->bytesPerChannel=1;
			break;
		case 16:
			// pTags->bytesPerChannel=2;
			// break;
		default:
			pTags->valid=false;
			break;
	}

	// any compression?
	if(cmp!=1)
		pTags->valid=false;

	// samples per pixel: currently 1 or 3
	pTags->channels=spp;
	/*
	switch(spp){
		case 1:
			pTags->channels=1;
			break;
		case 3:
			pTags->channels=3;
			break;
		default:
			pTags->valid=false;
			break;
	}
	*/
	// check if color space is supported
	switch(pTags->channels){
		case 1:
			// single channel: assume black is 0
			if(pmt!=1)
				pTags->valid=false;
			break;
		case 3:
			// 3 channels: assume rgb
			if(pmt!=2)
				pTags->valid=false;
			break;
		default:
			pTags->valid=false;
			break;
	}

	// planar configuration
	if(pcf!=1)
		pTags->valid=false;

	// width and height
	pTags->width=w;
	pTags->height=h;

	return;
}

void imageIO::load8BitTiff(const char *name, byteImage *pImg)
{
	if(!name|!pImg||!pImg->created())
		throw fqImageError(imageError::badParameter);

    TIFF *pTiff=TIFFOpen(name,"r");

	if(!pTiff)
		throw fqImageError(imageError::tiffOpen);

	// get the tiff tags
	TiffTags tags;
	getTiffTags(pTiff,&tags);

	// invalid anyway?
	// image fits tags?
	if(!tags.valid||
	   !tags.is8Bit()||
	   pImg->width()!=tags.width||
	   pImg->height()!=tags.height||
	   pImg->primChannels()!=tags.channels){
		TIFFClose(pTiff);
		throw fqImageError(imageError::tiffBadTags);
	}

	// ok, load the image
	assert(pImg->primChannels()*tags.width==TIFFScanlineSize(pTiff));
	// if the destiantion has no pre/post channels, we can load lines directly
	// otherwise we have to load to a buffer and write each primChannels() into
	// each pixel
	if(pImg->hasPreChannels()||pImg->hasPostChannels()){
		unsigned char *pBuf=new unsigned char[pImg->primChannels()*tags.width];
		if(!pBuf){
			TIFFClose(pTiff);
			throw fqImageError(imageError::badAlloc);
		}

		unsigned y,x;
		unsigned char *pSrc;
		unsigned char *pPix=pImg->pFirstPixel();
		const unsigned preChans=pImg->preChannels();
		const unsigned incChans=pImg->primChannels()+pImg->postChannels();
		const unsigned wrap=pImg->lineWrap();
		const unsigned nBytes=pImg->primChannels();
		for(y=0;y<tags.height;++y){
			if(TIFFReadScanline(pTiff,pBuf,y)!=1){
				TIFFClose(pTiff);
				throw fqImageError(imageError::tiffRead);
				return;
			}
			pSrc=pBuf;
			for(x=0;x<tags.width;++x){
				pPix+=preChans;
				memcpy(pPix,pSrc,nBytes);
				pPix+=incChans;
				pSrc+=nBytes;
			}
			pPix+=wrap;
		}

		delete[] pBuf;
	}
	else{
		unsigned y;
		unsigned char *pPix=pImg->pFirstPixel();
		const unsigned offs=pImg->lineOffset();
		for(y=0;y<tags.height;++y){
			if(TIFFReadScanline(pTiff,pPix,y)!=1){
				TIFFClose(pTiff);
				throw fqImageError(imageError::tiffRead);
				return;
			}
			pPix+=offs;
		}
	}

	// set colorspace
	/*
	if(tags.channels==1)
		pImg->setColorSpace(byteImage::colorSpaceGrey);
	else if(tags.channels==3)
		pImg->setColorSpace(byteImage::colorSpaceRGB);
	else
		assert(false);
	*/

	TIFFClose(pTiff);

return;
}

void imageIO::save8BitTiff(const char *name, byteImage *pImg)
{
	if(!name|!pImg)
		throw fqImageError(imageError::badParameter);

	// check color space
	if(!(pImg->isGrey()||pImg->isRGB()))
		throw fqImageError(imageError::tiffBadColorSpace);

    TIFF *pTiff=TIFFOpen(name,"w");

	if(!pTiff)
		throw fqImageError(imageError::tiffOpen);

	// set tiff tags
	uint16 ttag=(pImg->isGrey())?1:2;
    if(TIFFSetField(pTiff,TIFFTAG_BITSPERSAMPLE,8)!=1||
       TIFFSetField(pTiff,TIFFTAG_COMPRESSION,1)!=1||
	   TIFFSetField(pTiff,TIFFTAG_ORIENTATION,1)!=1||
	   TIFFSetField(pTiff,TIFFTAG_SAMPLESPERPIXEL,pImg->primChannels())!=1||
	   TIFFSetField(pTiff,TIFFTAG_PLANARCONFIG,1)!=1||
	   TIFFSetField(pTiff,TIFFTAG_PHOTOMETRIC,ttag)!=1||
	   TIFFSetField(pTiff,TIFFTAG_IMAGEWIDTH,pImg->width())!=1||
       TIFFSetField(pTiff,TIFFTAG_IMAGELENGTH,pImg->height())!=1){
		TIFFClose(pTiff);
		throw fqImageError(imageError::tiffWrite);
	}

	// write scan lines
	assert(pImg->width()*pImg->primChannels()==TIFFScanlineSize(pTiff));
	// if the image has pre/post channels, we need to write eachs pixels
	// primChannels() to a buffer and write back the buffer,
	// otherwise we can write image lines
	const unsigned h=pImg->height();
	if(pImg->hasPreChannels()||pImg->hasPostChannels()){
		const unsigned w=pImg->width();
		unsigned char* pBuf=new unsigned char[w*pImg->primChannels()];
		if(!pBuf){
			TIFFClose(pTiff);
			throw fqImageError(imageError::badAlloc);
		}

		unsigned x,y;
		unsigned char* pPix=pImg->pFirstPixel();
		const unsigned preChans=pImg->preChannels();
		const unsigned incChans=pImg->primChannels()+pImg->postChannels();
		const unsigned wrap=pImg->lineWrap();
		const unsigned nBytes=pImg->primChannels();
		unsigned char* pDst;
		for(y=0;y<h;++y){
			pDst=pBuf;
			for(x=0;x<w;++x){
				pPix+=preChans;
				memcpy(pDst,pPix,nBytes);
				pPix+=incChans;
				pDst+=nBytes;
			}
			if(TIFFWriteScanline(pTiff,pBuf,y)!=1){
				TIFFClose(pTiff);
				throw fqImageError(imageError::tiffWrite);
				return;
			}
			pPix+=wrap;
		}
		delete[] pBuf;
	}
	else{
		unsigned y;
		unsigned char* pPix=pImg->pFirstPixel();
		const unsigned offs=pImg->lineOffset();
		for(y=0;y<h;++y){
			if(TIFFWriteScanline(pTiff,pPix,y)!=1){
				TIFFClose(pTiff);
				throw fqImageError(imageError::tiffWrite);
				return;
			}
			pPix+=offs;
		}
	}

	TIFFClose(pTiff);
}
#endif

// jpeg io
imageIO::jpegLoader::~jpegLoader()
{
	if(m_pSrcMng)
		delete m_pSrcMng;
}

void imageIO::jpegErrHandler(j_common_ptr cinfo)
{
	// todo: copy error message from cinfo...
	throw fqImageError(imageError::jpegError);
}


imageIO::jpegLoader* imageIO::jpegGetLoader(const char* name)
{
	if(!name)
		throw fqImageError(imageError::badParameter);
	
	jpegLoader* p=new jpegLoader;

	// try to open the file
	if((p->m_pFile=fopen(name,"rb"))==NULL){
		delete p;
		throw fqImageError(imageError::jpegError);
		// return;
	}

	// set up the normal JPEG error routines, then override error_exit
	
	
	p->m_cinfo.err=jpeg_std_error(&(p->m_jerr));
	p->m_jerr.error_exit=imageIO::jpegErrHandler;

	// the static error handler will throw, so try and catch...
	try{
		// initialize the JPEG decompression object
		jpeg_create_decompress(&(p->m_cinfo));

		// specify data source
		jpeg_stdio_src(&(p->m_cinfo),p->m_pFile);

		// read file parameters with jpeg_read_header()
		// we can ignore the return value from jpeg_read_header since
		//  (a) suspension is not possible with a stdio (FILE *) data source, and
		//  (b) we passed TRUE to reject a tables-only JPEG file as an error.
		// See libjpeg.doc for more info.
		jpeg_read_header(&(p->m_cinfo),TRUE);
	}
	catch(...){
		// something went wrong
		// todo: build in error messages
		jpeg_destroy_decompress(&(p->m_cinfo));
		fclose(p->m_pFile);
		delete p;
		throw fqImageError(imageError::jpegError);
	}

return p;
}

// get's a loader to load from a memory buffer
imageIO::jpegLoader* imageIO::jpegGetLoader(const char* pBuf,size_t sz)
{
	if(!pBuf)
		throw fqImageError(imageError::badParameter);
	
	jpegLoader* p=new jpegLoader;

	// set up the normal JPEG error routines, then override error_exit
	p->m_cinfo.err=jpeg_std_error(&(p->m_jerr));
	p->m_jerr.error_exit=imageIO::jpegErrHandler;

	// create a src manager
	p->m_pSrcMng=new jpeg_source_mgr;
	if(!p->m_pSrcMng){
		delete p;
		throw fqImageError(imageError::jpegError);
	}

	// set up the src manager: fill in the static callbacks
	p->m_pSrcMng->init_source=&p->init_source;
	p->m_pSrcMng->fill_input_buffer=&p->fill_input_buffer;
	p->m_pSrcMng->skip_input_data=&p->skip_input_data;
	p->m_pSrcMng->resync_to_restart=&p->resync_to_restart;
	p->m_pSrcMng->term_source=&p->term_source;

	// set up the src manager: set the buffer...
	p->m_pSrcMng->next_input_byte=(const JOCTET *)pBuf;
	p->m_pSrcMng->bytes_in_buffer=sz;


	// the static error handler will throw, so try and catch...
	try{
		// initialize the JPEG decompression object
		jpeg_create_decompress(&(p->m_cinfo));

		// specify data source
		// jpeg_stdio_src(&(p->m_cinfo),p->m_pFile);
		p->m_cinfo.src=p->m_pSrcMng;

		// read file parameters with jpeg_read_header()
		// we can ignore the return value from jpeg_read_header since
		//  (a) suspension is not possible with a stdio (FILE *) data source, and
		//  (b) we passed TRUE to reject a tables-only JPEG file as an error.
		// See libjpeg.doc for more info.
		jpeg_read_header(&(p->m_cinfo),TRUE);
	}
	catch(...){
		// something went wrong
		// todo: build in error messages
		jpeg_destroy_decompress(&(p->m_cinfo));
		delete p;
		throw fqImageError(imageError::jpegError);
	}

return p;
}


void imageIO::jpegLoad(jpegLoader* p,byteImage *pImg)
{
	if(!p)
		throw fqImageError(imageError::badParameter);

	// we automatically delete the loader here, even on a throw...
	imageIO::autoCloseJpegLoader a(p);

	if(!pImg)
		throw fqImageError(imageError::badParameter);

	if(pImg->created()){
		// check if image fit's
		if(pImg->width()!=p->m_cinfo.output_width||
		   pImg->height()!=p->m_cinfo.output_height||
		   pImg->primChannels()!=p->m_cinfo.output_components||
		   !(pImg->isBGR()||pImg->isRGB()))
			throw fqImageError(imageError::badParameter);
	}
	// TODO: check if this is really a good idea...
	else{
		pImg->create(p->m_cinfo.output_width,p->m_cinfo.output_height,p->m_cinfo.output_components);
	}

	// some consts
	const unsigned w=pImg->width();
	const unsigned pre=pImg->preChannels();
	const unsigned post=pImg->postChannels();
	const unsigned chans=pImg->primChannels();

	const bool bgr=pImg->isBGR();
	const unsigned redChan=bgr?2:0;
	const unsigned greenChan=1;
	const unsigned blueChan=bgr?0:2;

	int linesRead;
	// check if we can write to the image directly, i.e. if we can
	// can pass the adress of image lines to jpeg_read_scanlines()
	bool bDirectAccess=true;
	if(bgr||pImg->hasBorders()||pre||post||pImg->roiEnabled())
		bDirectAccess=false;

	// this is the number of scan lines we may get at a time, if this is bigger than 1,
	// loading is faster...
	// question: if(bDirectAccess&&(pImg->height()%nScanLines)) 
	// what happens in the last call to jpeg_read_scanlines() (we pass non-existing memory!!!)
	unsigned nScanLines=p->m_cinfo.rec_outbuf_height;
	assert(nScanLines);
	JSAMPARRAY pLines=NULL;
	unsigned char *pBuf=NULL;
	try{
		// note we need to pass an array of pointers to (image) lines
		pLines=new JSAMPROW[nScanLines];
		unsigned i;
		
	   	jpeg_start_decompress(&(p->m_cinfo));

		if(bDirectAccess){
			unsigned char* pDst=pImg->pFirstPixel();
			const unsigned offset=w*chans*nScanLines;

			while(p->m_cinfo.output_scanline<p->m_cinfo.output_height){
				// note since we do direct access, the pLines buffer has to loop through the image...
				for(i=0;i<nScanLines;++i)
					pLines[i]=pDst+i*w*chans;
				
				// note on the bottom of the image, jpeg_read_scanlines() may read less
				// than nScanlines lines (e.g. if the height of the image is odd and nSacnlines is even)
				// this is no issue here, because we write directly to the image passed
				jpeg_read_scanlines(&(p->m_cinfo),pLines,nScanLines);
				pDst+=offset;
			}
		}
		else{
			// we need to allocate some buffer, decompress to that and write to the image :(
			pBuf= new unsigned char[w*chans*nScanLines];
			unsigned char *pDst=pImg->pFirstPixel();
			unsigned char *pSrc;
			const unsigned wrap=pImg->lineWrap();
			unsigned x,y;

			for(i=0;i<nScanLines;++i)
				pLines[i]=pBuf+i*w*chans;

			// if we have no pre/post channels (that is, only roi or borders) and if we are !bgr, we can be faster
			if(!pre&&!post&&!bgr){
				const unsigned sz=chans*w;
				while(p->m_cinfo.output_scanline<p->m_cinfo.output_height){
					// note on the bottom of the image, jpeg_read_scanlines() may read less
					// than nScanlines lines (e.g. if the height of the image is odd and nSacnlines is even)
					// this is an issue here, because we do not write directly to the image passed
					linesRead=jpeg_read_scanlines(&(p->m_cinfo),pLines,nScanLines);
					pSrc=pBuf;
					for(y=0;y<linesRead;++y){
						memcpy(pDst,pSrc,sz);
						pSrc+=sz;
						pDst+=wrap;
					}
				}
			}
			else{
				const unsigned inc=post+chans;
				while(p->m_cinfo.output_scanline<p->m_cinfo.output_height){
					// note on the bottom of the image, jpeg_read_scanlines() may read less
					// than nScanlines lines (e.g. if the height of the image is odd and nSacnlines is even)
					// this is an issue here, because we do not write directly to the image passed
					linesRead=jpeg_read_scanlines(&(p->m_cinfo),pLines,nScanLines);
					pSrc=pBuf;
					for(y=0;y<linesRead;++y){
						for(x=0;x<w;++x){
							pDst+=pre;
							// memcpy(pDst,pSrc,chans);
							*(pDst+redChan)=*pSrc++;
							*(pDst+greenChan)=*pSrc++;
							*(pDst+blueChan)=*pSrc++;
							// pSrc+=chans;
							pDst+=inc;
						}
						pDst+=wrap;
					}
				}
			}
			delete[] pBuf;
		}
		delete[] pLines;
	}
	catch(...){
		// something went wrong, free mem
		if(pLines)
			delete[] pLines;
		if(pBuf)
			delete[] pBuf;

		// todo: build in error messages
		throw fqImageError(imageError::jpegError);
	}
}

bool imageIO::jpegLoader::scaleBelow(unsigned biggerBelow,unsigned biggerAbove,unsigned smallerAbove)
{
	if(biggerAbove)
		if(biggerAbove>=biggerBelow)
			return false;
	if(smallerAbove)
		if(smallerAbove>=biggerBelow)
			return false;
	// note we assume that this jpegLoader was aquired by imageIO::jpegGetLoader()
	// if not, this will fail, propably dramatically
	// TODO: test this...
	// anyway:
	// assert(m_pFile); 
	// get max/min of original dims:
	const unsigned orgMax=max(inWidth(),inHeight());
	const unsigned orgMin=min(inWidth(),inHeight());

	// something to scale down?
	if(biggerBelow>=orgMax&&orgMax>=biggerAbove&&orgMin>=smallerAbove)	
		return true;

	double dScaleDeNom=(double)orgMax/biggerBelow;
	unsigned int scaleDeNom;

	// note although it is possible to set m_cinfo.scale_num and m_cinfo.scale_denom,
	// the doc says that the only supported scalings are 1/1, 1/2, 1/4, 1/8

	// geaendert Kai
	// old:
	//if(dScaleDeNom>=4.)
	//	scaleDeNom=8;
	//else if(dScaleDeNom>=2.)
	//	scaleDeNom=4;
	//else // see return above...
	//	scaleDeNom=2;

	// new:
	if(dScaleDeNom>=8.)
		scaleDeNom=8;
	else if(dScaleDeNom>=4.)
		scaleDeNom=4;
	else if(dScaleDeNom>=2.)
		scaleDeNom=2;
	else // see return above...
		scaleDeNom=1;

	bool bTooSmall;
	if(biggerAbove){
		// check if the bigger side will be as big as biggerAbove, otherwise scale up if possible
		bTooSmall=(double)orgMax/(double)scaleDeNom<(double)biggerAbove;
		while(bTooSmall&&scaleDeNom>1){
			scaleDeNom/=2;
			bTooSmall=(double)orgMax/(double)scaleDeNom<(double)biggerAbove;
		}
		
		if(bTooSmall)
			return false;
	}

	if(smallerAbove){
		// check if the smaller side will be as big as smallerAbove, otherwise scale up if possible
		bTooSmall=(double)orgMin/(double)scaleDeNom<(double)smallerAbove;
		while(bTooSmall&&scaleDeNom>1){
			scaleDeNom/=2;
			bTooSmall=(double)orgMin/(double)scaleDeNom<(double)smallerAbove;
		}
		
		if(bTooSmall)
			return false;
	}

	m_cinfo.scale_denom=scaleDeNom;

	calc();

	return true;
}

void imageIO::jpegLoader::close()
{
	 jpeg_destroy_decompress(&m_cinfo); 
	 if(m_pFile){
		 fclose(m_pFile);
		 m_pFile=NULL;
	 }
	 if(m_pSrcMng){
		 delete m_pSrcMng;
		 m_pSrcMng=NULL;
	 }
}
imageIO::jpegSaver* imageIO::jpegGetSaver(const char* name,byteImage& rImg)
{
	if(!name||!rImg.created()||!(rImg.isGrey()||rImg.isRGB()||rImg.isBGR()))
		throw fqImageError(imageError::badParameter);

	jpegSaver* p=new jpegSaver;

	// try to open the file
	if((p->m_pFile=fopen(name,"wb"))==NULL){
		delete p;
		throw fqImageError(imageError::jpegError);
		// return;
	}

	p->m_pImg=&rImg;

	// set up the normal JPEG error routines, then override error_exit
	p->m_cinfo.err=jpeg_std_error(&(p->m_jerr));
	p->m_jerr.error_exit=imageIO::jpegErrHandler;

	// the static error handler will throw, so try and catch...
	try{
		// initialize the JPEG decompression object
		jpeg_create_compress(&(p->m_cinfo));

		// specify data source
		jpeg_stdio_dest(&(p->m_cinfo),p->m_pFile);

		// set parameters
		p->m_cinfo.image_width=rImg.width();
		p->m_cinfo.image_height=rImg.height();
		if(rImg.isGrey()){
			p->m_cinfo.input_components=1;
			p->m_cinfo.in_color_space=JCS_GRAYSCALE;
		}
		else{ // isRGB() or isBGR(), see above
			p->m_cinfo.input_components=3;
			p->m_cinfo.in_color_space=JCS_RGB;
		}
		jpeg_set_defaults(&(p->m_cinfo));
	}
	catch(...){
		// something went wrong
		// todo: build in error messages
		jpeg_destroy_compress(&(p->m_cinfo));
		fclose(p->m_pFile);
		delete p;
		throw fqImageError(imageError::jpegError);
	}

return p;
}

void imageIO::jpegSave(imageIO::jpegSaver* pSaver)
{
	if(!pSaver)
		throw fqImageError(imageError::badParameter);

	byteImage& rImg=*(pSaver->m_pImg);
	jpeg_start_compress(&(pSaver->m_cinfo),TRUE);

	jpeg_compress_struct& rInfo=pSaver->m_cinfo;
	// note we write scanline by scanline
	// thus, we need to set up a 1-elem array of row pointers,
	// we can set this to an roi also
	JSAMPROW pRow[1];
	// anyway, we may have pre/post channels or different channel order, so we may have to convert...
	const bool bInplace=!rImg.preChannels()&&!rImg.postChannels()&&!rImg.isBGR();

	unsigned char* pSrc=rImg.pFirstPixel();
	if(bInplace){
		const unsigned offset=rImg.lineOffset();
		while(rInfo.next_scanline<rInfo.image_height){
			pRow[0]=pSrc;
			jpeg_write_scanlines(&(rInfo),pRow,1);
			pSrc+=offset;
		}
	}
	else{
		const unsigned w=rImg.width();
		const unsigned pre=rImg.preChannels();
		const unsigned wrap=rImg.lineWrap();
		const unsigned chans=rImg.primChannels();
		unsigned char* pBuf=new unsigned char[w*chans];
		unsigned char* pDst;
		pRow[0]=pBuf;
		unsigned x;
		if(rImg.isBGR()){
			// we have to swap channels from bgr to rgb
			const int swap=chans-1;
			const unsigned post=rImg.postChannels()+chans;
			int c;
			while(rInfo.next_scanline<rInfo.image_height){
				pDst=pBuf;
				for(x=0;x<w;++x){
					pSrc+=pre;
					for(c=swap;c>=0;--c)
						*pDst++=*(pSrc+c);
					pSrc+=post;
				}
				jpeg_write_scanlines(&(rInfo),pRow,1);
				pSrc+=wrap;
			}
		}
		else{
			const unsigned post=rImg.postChannels();
			unsigned c;
			while(rInfo.next_scanline<rInfo.image_height){
				pDst=pBuf;
				for(x=0;x<w;++x){
					pSrc+=pre;
					for(c=0;c<chans;++c)
						*pDst++=*pSrc++;
					pSrc+=post;
				}
				jpeg_write_scanlines(&(rInfo),pRow,1);
				pSrc+=wrap;
			}
		}
		delete[] pBuf;
	}

	jpeg_finish_compress(&rInfo);
	// clean up
	fclose(pSaver->m_pFile);
	jpeg_destroy_compress(&rInfo);
	delete pSaver;
	return;
}

#pragma function(memcpy)







