#include "exifthumbnailextractor.h"

#include <fstream>
#include <assert.h>

// makros for byte swapping
#define SWAP32(x) (((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8)  | ((x & 0x0000FF00) << 8)  | ((x & 0x000000FF) << 24))
#define SWAP16(x) (((x & 0xFF00) >> 8) | ((x & 0x00FF) << 8))

imageIO::jpegLoader::exifThumbData * exifThumbnailExtractor::extractExifThumbnail(const char *fname)
{
	// TODO: build in error handling
	if(!fname)
		return NULL;

	std::ifstream is;
	imageIO::jpegLoader::exifThumbData* pData;
	// TODO make sure ifstream throws on error
	try{
		is.open(fname,std::ios_base::in|std::ios_base::binary);
		assert(is.good());
		if(!is.good())
			return NULL;
		pData=extractExifThumbnail(is);
		is.close();
	}
	catch(...){
		return NULL;
	}

	return pData;

}


imageIO::jpegLoader::exifThumbData * exifThumbnailExtractor::extractExifThumbnail(std::ifstream& is)
{
	unsigned char c1,c2;
#if defined(__GNUC__)
	uint16_t exifSize;
	uint32_t fourBytes;
	uint16_t twoBytes;
	uint32_t jpegIFOffset;
	uint32_t jpegIFByteCount;
	uint32_t dataOrOffset;
	uint32_t numComps;
	uint16_t tag;
	uint16_t format;
#elif defined(_WIN32)
	unsigned __int16 exifSize;
	unsigned __int32 fourBytes;
	unsigned __int16 twoBytes;
	unsigned __int32 jpegIFOffset;
	unsigned __int32 jpegIFByteCount;
	unsigned __int32 dataOrOffset;
	unsigned __int32 numComps;
	unsigned __int16 tag;
	unsigned __int16 format;
#else
#error no platform specified...
#endif
	char magicExif[6];
	bool bIntel=true;
	unsigned offset=0;
	unsigned TIFFHeaderOffset;
	
	char *pThumb=NULL;
	imageIO::jpegLoader::exifThumbData* pData=NULL;
	try{
		// first 2 Bytes have to be 'FFD8' (JPEG Start of Image Marker)
		is >> c1 >> c2;
		if(c1!=0xff||c2!=0xd8)
			return NULL;

		// next 2 Bytes have to be FFE1 (Exif APP1 Marker)
		is >> c1 >> c2;
		if(c1!=0xFF||c2!=0xE1)
			return NULL;

		// next 4 Bytes exifSize (with the 4 Bytes)
		// TODO: this is propably wrong, if we read into an __int16, the bytes are swapped on LSB byte order systems... 
		is.read((char*)&exifSize,sizeof(exifSize));
		// TODO: must be bigger than 4 at least...
		if(exifSize<4)
			return NULL;

		// next 6 Bytes 'EXIF00'
		// TODO: check against const str
		is.read(magicExif,sizeof(magicExif));

		// NOTE all offsetts in the Exif Data are relative to *this* position
		TIFFHeaderOffset=12;

		// next 8 byte TIFF header
		// first 2 Bytes: either 0x4949 (II, Intel) or 0x4d4d (MM, Motorola) Byte Order

		is >> c1 >> c2;
		assert(c1==c2);
		if(c1==0x49)
			bIntel=true;
		else if(c1==0x4d)
			bIntel=false;
		else
			return NULL;

		// NOTE: *all* following code is *only* correct if we are on a LSB system (intel) and the format is intel...

		// next 2 Bytes either '2A00' (Intel) or '002A' (Motorola)
		is >> c1 >> c2;
		if(bIntel){
			if(c1!=0x2A||c2!=0x00)
				return NULL;
		}
		else{
			if(c1!=0x00||c2!=0x2A)
				return NULL;
		}

		// next 4 Bytes offset to first IFD (IFD0)
		// this usually is 8, meaning that the first IFD (IFD0) follows directly
		// (because all offset values are offsets from the first 'Intel' or 'Motorola' byte...)
		is.read((char*)&fourBytes,sizeof(fourBytes));
		if(!bIntel)
			fourBytes=SWAP32(fourBytes);

		// offest to IFD0 must be equal or bigger 8
		if(fourBytes<8)
			return NULL;

		// bigger offset: go forward..
		if(fourBytes!=8){
			is.seekg(fourBytes+TIFFHeaderOffset,std::ios_base::beg);
		}

		// next two bytes: number of entries in IFD0
		is.read((char*)&twoBytes,sizeof(twoBytes));
		if(!bIntel)
			twoBytes=SWAP16(twoBytes);

		// each entry is 12 Byte, we don't need these here
		is.seekg(twoBytes*12,std::ios_base::cur);

		// next 4 bytes: offset to IFD1
		// if this is 0, there is no thumbnail inserted
		is.read((char*)&fourBytes,sizeof(fourBytes));
		if(!bIntel)
			fourBytes=SWAP32(fourBytes);

		if(!fourBytes)
			return NULL;

		// go to IFD1
		is.seekg(fourBytes+TIFFHeaderOffset,std::ios_base::beg);
		
		// next two bytes: number of entries in IFD1
		is.read((char*)&twoBytes,sizeof(twoBytes));
		if(!bIntel)
			twoBytes=SWAP16(twoBytes);

		// we need to find three entries here...
		unsigned numEntries=twoBytes;
		if(numEntries<3)
			return NULL;

		// ...these are:
		// Tag 0x0103 (Compression, has to have value '6', otherwise the thumbnail is not a JPEG)
		// Tag 0x0201 (JpegIFOffset, Offset to JPEG data)
		// Tag 0x0202 (JpegIFByteCount, Size of Jpeg Thumbnail)
		bool b1=false,b2=false,b3=false;
		unsigned i;
		for(i=0;i<numEntries;++i){
			// Tag Number
			is.read((char*)&twoBytes,sizeof(twoBytes));
			if(!bIntel)
				twoBytes=SWAP16(twoBytes);

			tag=twoBytes;
			// Data Format
			is.read((char*)&twoBytes,sizeof(twoBytes));
			if(!bIntel)
				twoBytes=SWAP16(twoBytes);

			format=twoBytes;
			// number of components
			is.read((char*)&fourBytes,sizeof(fourBytes));
			if(!bIntel)
				fourBytes=SWAP32(fourBytes);

			numComps=fourBytes;
			// data or offset to data
			is.read((char*)&fourBytes,sizeof(fourBytes));
			if(!bIntel)
				fourBytes=SWAP32(fourBytes);

			dataOrOffset=fourBytes;

			if(tag==0x0103){
				// Format has to be 'unsigned short' (3)
				if(format!=3)
					return NULL;
				// Number of components has to 1
				if(numComps!=1)
					return NULL;
				// dataOrOffset has to be 6
				if(dataOrOffset!=6)
					return NULL;
				b1=true;
			}
			else if(tag==0x0201){
				// Format has to be 'unsigned long' (4)
				if(format!=4)
					return NULL;
				// Number of components has to 1
				if(numComps!=1)
					return NULL;
				// dataOrOffset hold the offset
				jpegIFOffset=dataOrOffset;
				b2=true;
			}
			else if(tag==0x0202){
				b3=true;
				// Format has to be 'unsigned long' (4)
				if(format!=4)
					return NULL;
				// Number of components has to 1
				if(numComps!=1)
					return NULL;
				// dataOrOffset hold the byte size
				jpegIFByteCount=dataOrOffset;
				b3=true;
			}
		}

		if(!(b1&&b2&&b3))
			return NULL;

		// go to jpeg thumbnail start
		is.seekg(jpegIFOffset+TIFFHeaderOffset,std::ios_base::beg);

		// allocate memory
		// note the caller has to delete[] this...
		pThumb=new char[jpegIFByteCount];
		if(!pThumb){
			assert(false);
			return NULL;
		}

		pData=new imageIO::jpegLoader::exifThumbData;
		if(!pData){
			delete[] pThumb;
			assert(FALSE);
			return NULL;
		}


		// read jpeg thumb
		// note there are some pad 0 bytes in the end...
		is.read(pThumb,jpegIFByteCount);

		pData->_pdata=pThumb;
		pData->_sz=jpegIFByteCount;
	}
	catch(...){
		throw;
	}
	return pData;
}
