#pragma once

#ifdef __GNUC__
#	include <math.h>
#endif

/*!
a template struct for points
*/
template <class T> struct point
{
	point() {;}
	point(T tx,T ty) : x(tx), y(ty) {;}
	T x;
	T y;
};
typedef point<int> iPoint;

#if defined(_MSC_VER)
typedef point<__int16> i16Point;
#elif defined(__GNUC__)
typedef point<__int16_t> i16Point;
#else
#error neither GNU nor MSC compiler
#endif

typedef point<unsigned> uPoint;
/*!
a template struct for rects
*/
template <typename T> class rect
{
public:
	rect(T l,T t,T w,T h) : left(l),top(t),width(w),height(h) {;}
	rect(){;}
	T left;
	T top;
	T width;
	T height;
	void shift(const point<T>& rPt){left+=rPt.x;top+=rPt.y;}
	bool operator==(const rect<T>& rOther) const {return left==rOther.left&&top==rOther.top&&width==rOther.width&&height==rOther.height;} 
	/*!
	return the distance from this to the rect passed

	if the rectangles overlap, the returned value is 0
	
	otherwise, the smallest distance from any corner/border of this
	to any corner/border of the rect passed is reurned

	*/
	T distance(const rect<T>& rOther);
};

#if defined(_MSC_VER)
template <typename T> __forceinline T rect<T>::distance(const rect<T>& rOther)
#elif defined(__GNUC__) 
template <typename T> T rect<T>::distance(const rect<T>& rOther) // GNU does not know about __forceinline
#else
#error neither GNU nor MSC compiler
#endif
{
	T dist=0;
	T tmp1,tmp2;

	if((tmp1=(rOther.top+rOther.height-1))<top){
		if((tmp2=(rOther.left+rOther.width-1))<left)
			dist=(T)sqrt((double)(top-tmp1)*(double)(top-tmp1)+(double)(left-tmp2)*(double)(left-tmp2));
		else if(rOther.left>(tmp2=(left+width-1)))
			dist=(T)sqrt((double)(top-tmp1)*(double)(top-tmp1)+(double)(rOther.left-tmp2)*(double)(rOther.left-tmp2));
		else
			dist=top-tmp1;
	}
	else if((tmp1=(top+height-1))<rOther.top){
		if((tmp2=(rOther.left+rOther.width-1))<left)
			dist=(T)sqrt((double)(rOther.top-tmp1)*(double)(rOther.top-tmp1)+(double)(left-tmp2)*(double)(left-tmp2));
		else if(rOther.left>(tmp2=(left+width-1)))
			dist=(T)sqrt((double)(rOther.top-tmp1)*(double)(rOther.top-tmp1)+(double)(rOther.left-tmp2)*(double)(rOther.left-tmp2));
		else
			dist=rOther.top-tmp1;
	}
	else{
		if((tmp2=(rOther.left+rOther.width-1))<left)
			dist=left-tmp2;
		else if(rOther.left>(tmp2=(left+width-1)))
			dist=rOther.left-tmp2;
		// else dist=0;
	}
	
	return dist;
}

typedef point<int> iPoint;
typedef point<unsigned> uPoint;
typedef rect<int> iRect;
typedef rect<unsigned> uRect;
