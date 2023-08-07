#if !defined(EXIFTHUMBNAILEXTRACTOR_H)
#define EXIFTHUMBNAILEXTRACTOR_H

#include <string>
#include "imageio.h"
class ifstream;
//#include <fstream>
class exifThumbnailExtractor
{
public:
	exifThumbnailExtractor(){;}
	~exifThumbnailExtractor(){;}
	/*!
	reads the exif (jpeg) thumbnail from the file fname

	returns a ptr to a buffer containing the jpeg data

	note it's the callers responsability to delete() the buffer

	returns NULL if no exif jpeg in fname
	*/
	imageIO::jpegLoader::exifThumbData *extractExifThumbnail(const char* fname);
	/*!
	a convenience function
	*/
	imageIO::jpegLoader::exifThumbData *extractExifThumbnail(const std::string& fname) {return extractExifThumbnail(fname.c_str());}
protected:
	imageIO::jpegLoader::exifThumbData *extractExifThumbnail(std::ifstream& is);
private:
	exifThumbnailExtractor(const exifThumbnailExtractor& rOther);
	exifThumbnailExtractor& operator=(const exifThumbnailExtractor& rOther);
};
#endif