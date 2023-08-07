#pragma once

// if USETIFF is defined, TIFF IO is compiled into
// NOTE in ImageSorter, we don't need TIFF I/O, this is done by the QT Loaders. We don't want this
// code then, and we don't want another license issue
// TODO: make two classes of thei, one for JPEG, one for TIFF
// define USETIFF

#include "imagev2.h"
#if defined(USETIFF)
#include "tiff.h"
#include "tiffio.h"
#endif
//#if !defined(HAVE_BOOLEAN)
//#define HAVE_BOOLEAN
//#endif
#include "jpeglib.h"
#include <string>

/*!
imageIO is a class performing input/output for imagev2<T>

currently, this is TIFF and JPEG i/o only.

note that all member functions are static, thus, you don't need to instantiate an 
imagIO. however, if you do so, the (annoying) tifflib warning and error messages are turned
off until the (last) instance goes out of scope.

TODO: change this. Tiff IO error message shall be present as strings

imageIO is no template class, that is, you must know the type T of the 
imagev2<T> you want to load/store. note that this is unsigned char only, currently.

thus, use getTiffTags() for a file you want to open, adjust the imagev2<> you want to load
to accordingly and call e.g. load8BitTiff().

note that imageIO can handle imagev2<>::preChannels() and imagev2<>::postChannels, thus, it is
possible to load a single channel image file to a dedicated channel of a multichannel image as well
as to store a specific channel of a multichannel image to a single channel image file...

note that imageIO can deal with imagev2<>::roiEnabled(), thus, it is possible to load/store to and from
roi's.

note that imageIO handles imagev2<>::hasBorders() transparently. borders are newer loaded and never stored.

jpeg loading
************

*/

class imageIO
{
public:
	imageIO();
	~imageIO();
#if defined(USETIFF)
	/*!
	(a subset of) the tiff tags
	*/
	struct TiffTags{
		TiffTags() : valid(false) {;}
		unsigned width;
		unsigned height;
		unsigned channels;
		unsigned bytesPerChannel;
		/*!
		this is false if the tiff file has any property not supported here
		*/
		bool valid;
		bool is8Bit() const {return (bytesPerChannel==1);}
	};

	// tiff io
	/*!
	gets the relevant tiff tags from a tiff file
	*/
	TiffTags getTiffTags(const char* name);
	/*!
	a string version for convenience
	*/
	TiffTags getTiffTags(const string& name) {return getTiffTags(name.c_str());}
	/*!
	loads an 8 bit tiff image from a file

	note the image has to be created and has to fit getTiffTags(name)
	*/
	static void load8BitTiff(const char* name,byteImage *pImg);
	/*!
	a string version for convenience
	*/
	static void load8BitTiff(const string& name,byteImage *pImg) {load8BitTiff(name.c_str(),pImg);}
	/*!
	save an 8 bit tiff image to a file
	*/
	static void save8BitTiff(const char* name,byteImage *pImg);
	/*!
	a string version for convenience
	*/
	static void save8BitTiff(const string& name,byteImage *pImg) {save8BitTiff(name.c_str(),pImg);}
#endif

	// jpeg io
	/*!
	this wraps a jpeg_decompress_struct and a jpeg_error_mgr (used by libjpeg)
	and the associated FILE* 
	*/
	class jpegLoader{
	public:
		jpegLoader() : m_pFile(NULL),m_pSrcMng(NULL) {;}
		~jpegLoader();
		// after loader is aquired by jpegGetLoader(), these are valid:
		unsigned inWidth() const {return m_cinfo.image_width;}
		unsigned inHeight() const {return m_cinfo.image_height;}
		unsigned inChannels() const {return (unsigned)(m_cinfo.num_components);}
		bool isGrey() const {return m_cinfo.jpeg_color_space==JCS_GRAYSCALE;}
		// ...and these may be used
		// void setNom(unsigned nom) {m_cinfo.scale_num=nom;}
		void setDeNom(unsigned denom) {m_cinfo.scale_denom=denom;}
		void calc() {jpeg_calc_output_dimensions(&m_cinfo);}
		// after calc(), these are valid:
		unsigned outWidth() const {return m_cinfo.output_width;}
		unsigned outHeight() const {return m_cinfo.output_height;}
		// this sets the internal scaling so that outWidth() or outHeight() (whichever is bigger)
		// is equal or smaller than dim, if possible, else bigger
		// if minDim is given, the smaller side will not be smaller than minDim. if this is not possible
		// (libjpeg cannot scale up), false is returned, else true
		bool scaleBelow(unsigned biggerBelow,unsigned biggerAbove=0,unsigned smallerAbove=0);
		// sets several options to have fastest mode
		void setFastMode() {m_cinfo.do_fancy_upsampling=false;m_cinfo.do_block_smoothing=false;}
		// sets dct method used
		void setDctMode(unsigned mode) {
			switch(mode){
				case 2:
					m_cinfo.dct_method=JDCT_FLOAT;
					break;
				case 1:
					m_cinfo.dct_method=JDCT_IFAST;
					break;
				case 0:
				default:
					m_cinfo.dct_method=JDCT_ISLOW;
					break;
			}
		}
		void setOutColorSpace(unsigned mode){m_cinfo.out_color_space=mode?JCS_RGB:JCS_GRAYSCALE;}
		void close();
	//protected:
		// friend imageIO::jpegLoader* imageIO::jpegGetLoader(const char *name);
		// note this reflects the jpeg naming conventions...
		jpeg_decompress_struct m_cinfo;
		jpeg_error_mgr m_jerr;
		FILE* m_pFile;
		// data and callbacks for reading from a data src manager (in fact, from memory) instead from a file...
		jpeg_source_mgr* m_pSrcMng;
		// source is initialized at all...
		static void init_source(j_decompress_ptr cinfo) {;}
		// the input buffer is full an contains the whole data stream
		// if this is called, we have an error
		static JBOOLEAN fill_input_buffer(j_decompress_ptr cinfo) {fqImageError(imageError::jpegError);return TRUE;}
		// called to skip data
		static void skip_input_data(j_decompress_ptr cinfo, long num_bytes) {cinfo->src->next_input_byte+=num_bytes;cinfo->src->bytes_in_buffer-=num_bytes;}
		// calls the standard...
		static JBOOLEAN resync_to_restart(j_decompress_ptr cinfo, int desired) {return jpeg_resync_to_restart(cinfo,desired);}
		// a NOP
		static void term_source(j_decompress_ptr cinfo) {;}

		class exifThumbData{
		public:
			exifThumbData() : _sz(0),_pdata(NULL) {;}
			~exifThumbData(){if(_pdata){delete[] _pdata;}}
			size_t _sz;
			const char* _pdata;
		};
	};
	/*!
	this wraps a jpeg_compress_struct and a jpeg_error_mgr (used by libjpeg)
	and the associated FILE* 
	*/
	class jpegSaver{
	public:
		jpegSaver() : m_pFile(NULL),m_pImg(NULL) {;}
		~jpegSaver() {;}
	//protected:
		// friend imageIO::jpegLoader* imageIO::jpegGetLoader(const char *name);
		// note this reflects the jpeg naming conventions...
		jpeg_compress_struct m_cinfo;
		jpeg_error_mgr m_jerr;
		FILE* m_pFile;
		byteImage* m_pImg;
	};
	/*!
	the jpeg error handler used here

	this throws an exception of type imageError
	*/
	static void jpegErrHandler(j_common_ptr cinfo);
	/*!
	the first part of loading/decompression

	returns a ptr to a jpegLoader (see above) 

	after this has succeeded, you may probe 

	jpegLoader::m_cinfo.image_width
	jpegLoader::m_cinfo.image_height
	jpegLoader::m_cinfo.num_components
	jpegLoader::m_cinfo.jpeg_color_space

	to get information about the image to be decompressed

	if you decide not to decompress the image, call jpegLoader::close() for the pointer returned.

	otherwise, call

	throws imageError::jpegOpen on error

	todo: better error handling: pass message from libjpeg.lib
	*/
	jpegLoader* jpegGetLoader(const char* name);
	/*!
	a string version for convenience
	*/
	jpegLoader* jpegGetLoader(const string& name) {return jpegGetLoader(name.c_str());}
	/*!
	a loader for reading from memory buffers
	*/
	jpegLoader* jpegGetLoader(const char *pBuf,size_t size);
	/*!
	loads a jpeg image

	pass a loader aquired with jpegGetLoader()

	pass a byteImage. the byteImage has either to be created or not.
	if the byteImage is created, it has to fit the loader.
	if it is not created, it will be created and will thus fit the loader
	note that in the latter case, extended byteImage features like roiEnabled(), preChannels(),
	postChannels(), hasBorders() and isBased() cannot be used.
	*/
	void jpegLoad(jpegLoader* pLoader,byteImage *pImg);
	/*!
	first part of jpeg saving/compression
	*/
	jpegSaver* jpegGetSaver(const char* name,byteImage& rImg);
	/*!
	saves a jpeg image through the saver passed
	*/
	void jpegSave(jpegSaver* saver);
protected:
#if defined(USETIFF)
	static void getTiffTags(TIFF* pTiff,TiffTags* pTags);
#endif
private:
#if defined(USETIFF)
	TIFFErrorHandler m_oldTiffErrHandler;
	TIFFErrorHandler m_oldTiffWarnHandler;
	static unsigned m_uId;
#endif
	// helper class
	// heap constructed in jpeg i/o members, destroyed (also) by throw stack unwind...
	class autoCloseJpegLoader{
	public:
		autoCloseJpegLoader(jpegLoader* ps) : p(ps) {;} 
		~autoCloseJpegLoader() {p->close(); delete p;}
	private:
		jpegLoader* p;
		autoCloseJpegLoader() {;}
	};


};



