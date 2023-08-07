#pragma once
/**
This class is used to perform the forward and inverse discrete cosine transform (DCT)
as specified by the CCITT TI.81 recommendation (www.w3.org/Graphics/JPEG/itu-t81.pdf).
The implementation of the IDCT and FDCT algorithms are based on the jfdctflt.c and
jidctflt.c implementations written by Thomas G. Lane.

re-engeneered to cpp by sepp
*/

/*!
a template class for computing discrete cosine transform on 8, 8*8 or 8*8*8 values.

T is the data type of the input and output values, which shall be float or double.

note that any other data type will thow an exception.

note that besides the ctors and the dtor all members are static
*/
template <typename T> class dct8x8
{
public:
	typedef T arr8x8x8[8][8][8];
	typedef T arr8x8[8][8];
	typedef T arr8[8];
	typedef T arr64[64];
	/*!
	note that this public ctor throws an exception, anyway there are 
	specializations of this for types double and float...
	*/
	dct8x8() {throw(std::invalid_argument("bad template type"));}
	~dct8x8() {;}
	/*!
	transforms 8x8 input
	*/
	static void transform(const arr8x8& in,arr8x8& out); 
	/*!
	transforms 8x input
	*/
	static void transform1D(const arr8& in,arr8& out);
	/*!
	transforms 8x8x8 input
	*/
	static void transform3D(const arr8x8x8& in,arr8x8x8& out);

	/*!
	inverse 8x8 transformation 
	*/
	static void inverse(const arr8x8& in,const arr8x8& quant,arr8x8& out);
	/*!
	inverse 8x8 transformation, input and output data is arranged linewise in a block

	a little faster that the arr8x8 version
	*/
	static void inverse(const arr64& in,const arr64& quant,arr64& out);
	/*!
	inverse 8x transformation 
	*/
	static void inverse1D(const arr8& in,const arr8& quant,arr8& out);
	/*!
	inverse 8x8x8 transformation 
	*/
	static void inverse3D(arr8x8x8& in,arr8x8x8& out);
protected:
	static void scaleQuantizationTable_1D(arr8& table);

	/**
	This method applies the pre-scaling that the IDCT(T[][], T[][], T[][]) method needs to work correctly.
	The table parameter should be 8x8, non-zigzag order.
	*/
	static void scaleQuantizationTable(arr8x8& table);

	/**
	This method applies the pre-scaling that the IDCT(T[], T[], T[]) method needs to work correctly.
	The table parameter should be 1x64, non-zigzag order.
	*/
	static void scaleQuantizationTable(arr64& table);
private:
	static const T m_R2;
 
	//--------------------------------------------------------------------------------------
	// these values are used in the IDCT
	static const T m_scaleFactor[8];

	static const T m_K2;	
	static const T m_K6;
	static const T m_M26;			
	static const T m_P26;			
 
	//--------------------------------------------------------------------------------------
	// these values are used in the FDCT
 
	static const T m_F0;	
	static const T m_F1;
	static const T m_F2;	
	static const T m_F3;	
	static const T m_F4;	
	static const T m_F5;	
	static const T m_F6;	
	static const T m_F7;	
	static const T m_D71;
	static const T m_D35;
	static const T m_D62;
	static const T m_S71;
	static const T m_S35;
	static const T m_S62;

 
	//--------------------------------------------------------------------------------------
 
};
