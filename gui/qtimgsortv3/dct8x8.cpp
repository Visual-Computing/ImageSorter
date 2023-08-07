#include "dct8x8.h"
#define _USE_MATH_DEFINES 
#include <cmath>

// ctor specializations for float and double
#if defined(__GNUC__)
template <>
#endif
dct8x8<float>::dct8x8(){;}

#if defined(__GNUC__)
template <>
#endif
dct8x8<double>::dct8x8(){;}

// initialize static consts
template <typename T> const T dct8x8<T>::m_R2=(T)(M_SQRT2);
template <typename T> const T dct8x8<T>::m_scaleFactor[]={
	(T)1.0,
	(T)cos((T)1. * (T)M_PI / (T)16.) * m_R2,	// 1.3870398453221475
	(T)cos((T)2. * (T)M_PI / (T)16.) * m_R2,	// 1.3065629648763766
	(T)cos((T)3. * (T)M_PI / (T)16.) * m_R2,	// 1.1758756024193588
	(T)cos((T)4. * (T)M_PI / (T)16.) * m_R2,	// 1.0 (really???)
	(T)cos((T)5. * (T)M_PI / (T)16.) * m_R2,	// 0.7856949583871023
	(T)cos((T)6. * (T)M_PI / (T)16.) * m_R2,	// 0.5411961001461971
	(T)cos((T)7. * (T)M_PI / (T)16.) * m_R2  	// 0.2758993792829431
};
//--------------------------------------------------------------------------------------
// these values are used in the inverse
template <typename T> const T dct8x8<T>::m_K2=(T)2*(T)cos((T)M_PI/(T)8);	// 1.8477590650225735
template <typename T> const T dct8x8<T>::m_K6=(T)2*(T)sin((T)M_PI/(T)8);	// 0.7653668647301796
template <typename T> const T dct8x8<T>::m_M26=m_K2-m_K6;					// 1.0823922002923938
template <typename T> const T dct8x8<T>::m_P26=-(m_K2+m_K6);				// -2.613125929752753
//--------------------------------------------------------------------------------------
// these values are used in the transform

template <typename T> const T dct8x8<T>::m_F0=(T)1.0 / m_R2;				// 0.7071067811865475
template <typename T> const T dct8x8<T>::m_F1=(T)cos((T)1*(T)M_PI/(T)16)/(T)2;	// 0.4903926402016152
template <typename T> const T dct8x8<T>::m_F2=(T)cos((T)2*(T)M_PI/(T)16)/(T)2;	// 0.46193976625564337
template <typename T> const T dct8x8<T>::m_F3=(T)cos((T)3*(T)M_PI/(T)16)/(T)2;	// 0.4157348061512726
template <typename T> const T dct8x8<T>::m_F4=(T)cos((T)4*(T)M_PI/(T)16)/(T)2;	// 0.3535533905932738
template <typename T> const T dct8x8<T>::m_F5=(T)cos((T)5*(T)M_PI/(T)16)/(T)2;	// 0.27778511650980114
template <typename T> const T dct8x8<T>::m_F6=(T)cos((T)6*(T)M_PI/(T)16)/(T)2;	// 0.19134171618254492
template <typename T> const T dct8x8<T>::m_F7=(T)cos((T)7*(T)M_PI/(T)16)/(T)2;	// 0.09754516100806417
template <typename T> const T dct8x8<T>::m_D71=m_F7-m_F1;				// -0.39284747919355106
template <typename T> const T dct8x8<T>::m_D35=m_F3-m_F5;				// 0.13794968964147147
template <typename T> const T dct8x8<T>::m_D62=m_F6-m_F2;				// -0.27059805007309845
template <typename T> const T dct8x8<T>::m_S71=m_F7+m_F1;				// 0.5879378012096794
template <typename T> const T dct8x8<T>::m_S35=m_F3+m_F5;				// 0.6935199226610738
template <typename T> const T dct8x8<T>::m_S62=m_F6+m_F2;				// 0.6532814824381883
 
 
/**
This method performs the forward discrete cosine transform (transform).
Both the in and out arrays are 8x8.
*/
template <typename T> void dct8x8<T>::transform(const arr8x8& in,arr8x8& out)
{
	T temp;
	T a0, a1, a2, a3, a4, a5, a6, a7;
	T b0, b1, b2, b3, b4, b5, b6, b7;

	// Horizontal transform
	for(unsigned i = 0; i < 8; ++i) {
		b0 = in[i][0] + in[i][7];
		b7 = in[i][0] - in[i][7];
		b1 = in[i][1] + in[i][6];
		b6 = in[i][1] - in[i][6];
		b2 = in[i][2] + in[i][5];
		b5 = in[i][2] - in[i][5];
		b3 = in[i][3] + in[i][4];
		b4 = in[i][3] - in[i][4];

		a0 = b0 + b3;
		a1 = b1 + b2;
		a2 = b1 - b2;
		a3 = b0 - b3;
		a4 = b4;
		a5 = (b6 - b5) * m_F0;
		a6 = (b6 + b5) * m_F0;
		a7 = b7;
		out[i][0] = (a0 + a1) * m_F4;
		out[i][4] = (a0 - a1) * m_F4;

		temp = (a3 + a2) * m_F6;
		out[i][2] = temp - a3 * m_D62;
		out[i][6] = temp - a2 * m_S62;

		b4 = a4 + a5;
		b7 = a7 + a6;
		b5 = a4 - a5;
		b6 = a7 - a6;

		temp = (b7 + b4) * m_F7;
		out[i][1] = temp - b7 * m_D71;
		out[i][7] = temp - b4 * m_S71;

		temp = (b6 + b5) * m_F3;
		out[i][5] = temp - b6 * m_D35;
		out[i][3] = temp - b5 * m_S35;
	}


	// Vertical transform
	for(unsigned i = 0; i < 8; ++i) {
		b0 = out[0][i] + out[7][i];
		b7 = out[0][i] - out[7][i];
		b1 = out[1][i] + out[6][i];
		b6 = out[1][i] - out[6][i];
		b2 = out[2][i] + out[5][i];
		b5 = out[2][i] - out[5][i];
		b3 = out[3][i] + out[4][i];
		b4 = out[3][i] - out[4][i];

		a0 = b0 + b3;
		a1 = b1 + b2;
		a2 = b1 - b2;
		a3 = b0 - b3;
		a4 = b4;
		a5 = (b6 - b5) * m_F0;
		a6 = (b6 + b5) * m_F0;
		a7 = b7;
		out[0][i] = (a0 + a1) * m_F4;
		out[4][i] = (a0 - a1) * m_F4;

		temp = (a3 + a2) * m_F6;
		out[2][i] = temp - a3 * m_D62;
		out[6][i] = temp - a2 * m_S62;

		b4 = a4 + a5;
		b7 = a7 + a6;
		b5 = a4 - a5;
		b6 = a7 - a6;

		temp = (b7 + b4) * m_F7;
		out[1][i] = temp - b7 * m_D71;
		out[7][i] = temp - b4 * m_S71;

		temp = (b6 + b5) * m_F3;
		out[5][i] = temp - b6 * m_D35;
		out[3][i] = temp - b5 * m_S35;
	}
}

template <typename T> void dct8x8<T>::transform1D(const arr8& in,arr8& out) 
{
	T temp;
	T a0, a1, a2, a3, a4, a5, a6, a7;
	T b0, b1, b2, b3, b4, b5, b6, b7;

	b0 = in[0] + in[7];
	b7 = in[0] - in[7];
	b1 = in[1] + in[6];
	b6 = in[1] - in[6];
	b2 = in[2] + in[5];
	b5 = in[2] - in[5];
	b3 = in[3] + in[4];
	b4 = in[3] - in[4];

	a0 = b0 + b3;
	a1 = b1 + b2;
	a2 = b1 - b2;
	a3 = b0 - b3;
	a4 = b4;
	a5 = (b6 - b5) * m_F0;
	a6 = (b6 + b5) * m_F0;
	a7 = b7;
	out[0] = (a0 + a1) * m_F4;
	out[4] = (a0 - a1) * m_F4;

	temp = (a3 + a2) * m_F6;
	out[2] = temp - a3 * m_D62;
	out[6] = temp - a2 * m_S62;

	b4 = a4 + a5;
	b7 = a7 + a6;
	b5 = a4 - a5;
	b6 = a7 - a6;

	temp = (b7 + b4) * m_F7;
	out[1] = temp - b7 * m_D71;
	out[7] = temp - b4 * m_S71;

	temp = (b6 + b5) * m_F3;
	out[5] = temp - b6 * m_D35;
	out[3] = temp - b5 * m_S35;

}

template <typename T> void dct8x8<T>::transform3D(const arr8x8x8& in,arr8x8x8& out)
{
	
	for (unsigned k=0;k<8;++k){
		transform(in[k],out[k]);
	}
	
	T in8[8];
	T out8[8];
	
	for(unsigned i=0;i<8;++i){
		for(unsigned j=0;j<8;++j) {
			for(unsigned k=0;k<8;++k) {
				in8[k]=out[k][i][j];
			}
			transform1D(in8,out8);
			for (unsigned k=0;k<8;++k) {
				out[k][i][j]=out8[k];
			}
		}
	}
}

// this method is actually a little faster than the [][] version
template <typename T> void dct8x8<T>::inverse(const arr64& in,const arr64& quant,arr64& out) 
{
	T tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
	T tmp7, /*tmp8, tmp9,*/ tmp10, tmp11, tmp12, tmp13;
	T z5, z10, z11, z12, z13;

	for(unsigned i=0;i<8;++i) {
		// note do we have to cast 0 to T?
		if(in[8+i] == 0 && in[16+i] == 0 && in[24+i] == 0 && in[32+i] == 0 && in[40+i] == 0 && in[48+i] == 0 && in[56+i] == 0) {
			T dc = in[i] * quant[i];
			out[i] = dc;
			out[8+i] = dc;
			out[16+i] = dc;
			out[24+i] = dc;
			out[32+i] = dc;
			out[40+i] = dc;
			out[48+i] = dc;
			out[56+i] = dc;
			continue;
		}

		tmp0 = in[i] * quant[i];
		tmp1 = in[16+i] * quant[16+i];
		tmp2 = in[32+i] * quant[32+i];
		tmp3 = in[48+i] * quant[48+i];

		tmp10 = tmp0 + tmp2;
		tmp11 = tmp0 - tmp2;

		tmp13 = tmp1 + tmp3;
		tmp12 = (tmp1 - tmp3) * m_R2 - tmp13;

		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		tmp4 = in[8+i] * quant[8+i];
		tmp5 = in[24+i] * quant[24+i];
		tmp6 = in[40+i] * quant[40+i];
		tmp7 = in[56+i] * quant[56+i];

		z13 = tmp6 + tmp5;
		z10 = tmp6 - tmp5;
		z11 = tmp4 + tmp7;
		z12 = tmp4 - tmp7;

		tmp7 = z11 + z13;
		tmp11 = (z11 - z13) * m_R2;

		z5 = (z10 + z12) * m_K2;
		tmp10 = m_M26 * z12 - z5;
		tmp12 = m_P26 * z10 + z5;

		tmp6 = tmp12 - tmp7;
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		out[i] = tmp0 + tmp7;
		out[56+i] = tmp0 - tmp7;
		out[8+i] = tmp1 + tmp6;
		out[48+i] = tmp1 - tmp6;
		out[16+i] = tmp2 + tmp5;
		out[40+i] = tmp2 - tmp5;
		out[32+i] = tmp3 + tmp4;
		out[24+i] = tmp3 - tmp4;
	}

	unsigned row = 0;
	for (unsigned i=0;i<8;++i) {
		row = i * 8;

		tmp10 = out[row] + out[row+4];
		tmp11 = out[row] - out[row+4];

		tmp13 = out[row+2] + out[row+6];
		tmp12 = (out[row+2] - out[row+6]) * m_R2 - tmp13;

		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		z13 = out[row+5] + out[row+3];
		z10 = out[row+5] - out[row+3];
		z11 = out[row+1] + out[row+7];
		z12 = out[row+1] - out[row+7];

		tmp7 = z11 + z13;
		tmp11 = (z11 - z13) * m_R2;

		z5 = (z10 + z12) * m_K2;
		tmp10 = m_M26 * z12 - z5;
		tmp12 = m_P26 * z10 + z5;

		tmp6 = tmp12 - tmp7;
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		out[row] = tmp0 + tmp7;
		out[row+7] = tmp0 - tmp7;
		out[row+1] = tmp1 + tmp6;
		out[row+6] = tmp1 - tmp6;
		out[row+2] = tmp2 + tmp5;
		out[row+5] = tmp2 - tmp5;
		out[row+4] = tmp3 + tmp4;
		out[row+3] = tmp3 - tmp4;
	}
}


template <typename T> void dct8x8<T>::inverse(const arr8x8& in,const arr8x8& quant,arr8x8& out) 
{
	T tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
	T tmp7, /*tmp8, tmp9,*/ tmp10, tmp11, tmp12, tmp13;
	T z5, z10, z11, z12, z13;

	for (unsigned i = 0; i < 8; ++i) {
		tmp0 = in[0][i] * quant[0][i];
		tmp1 = in[2][i] * quant[2][i];
		tmp2 = in[4][i] * quant[4][i];
		tmp3 = in[6][i] * quant[6][i];

		tmp10 = tmp0 + tmp2;
		tmp11 = tmp0 - tmp2;

		tmp13 = tmp1 + tmp3;
		tmp12 = (tmp1 - tmp3) * m_R2 - tmp13;

		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		tmp4 = in[1][i] * quant[1][i];
		tmp5 = in[3][i] * quant[3][i];
		tmp6 = in[5][i] * quant[5][i];
		tmp7 = in[7][i] * quant[7][i];

		z13 = tmp6 + tmp5;
		z10 = tmp6 - tmp5;
		z11 = tmp4 + tmp7;
		z12 = tmp4 - tmp7;

		tmp7 = z11+ z13;
		tmp11 = (z11 - z13) * m_R2;

		z5 = (z10 + z12) * m_K2;
		tmp10 = m_M26 * z12 - z5;
		tmp12 = m_P26 * z10 + z5;

		tmp6 = tmp12 - tmp7;
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		out[0][i] = tmp0 + tmp7;
		out[7][i] = tmp0 - tmp7;
		out[1][i] = tmp1 + tmp6;
		out[6][i] = tmp1 - tmp6;
		out[2][i] = tmp2 + tmp5;
		out[5][i] = tmp2 - tmp5;
		out[4][i] = tmp3 + tmp4;
		out[3][i] = tmp3 - tmp4;
	}

	for (unsigned i=0;i<8;++i) {
		tmp10 = out[i][0] + out[i][4];
		tmp11 = out[i][0] - out[i][4];

		tmp13 = out[i][2] + out[i][6];
		tmp12 = (out[i][2] - out[i][6]) * m_R2 - tmp13;

		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		z13 = out[i][5] + out[i][3];
		z10 = out[i][5] - out[i][3];
		z11 = out[i][1] + out[i][7];
		z12 = out[i][1] - out[i][7];

		tmp7 = z11 + z13;
		tmp11 = (z11 - z13) * m_R2;

		z5 = (z10 + z12) * m_K2;
		tmp10 = m_M26 * z12 - z5;
		tmp12 = m_P26 * z10 + z5;

		tmp6 = tmp12 - tmp7;
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		out[i][0] = tmp0 + tmp7;
		out[i][7] = tmp0 - tmp7;
		out[i][1] = tmp1 + tmp6;
		out[i][6] = tmp1 - tmp6;
		out[i][2] = tmp2 + tmp5;
		out[i][5] = tmp2 - tmp5;
		out[i][4] = tmp3 + tmp4;
		out[i][3] = tmp3 - tmp4;
	}
}

template <typename T> void dct8x8<T>::inverse1D(const arr8& in,const arr8& quant,arr8& out) {
	T tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
	T tmp7, /*tmp8, tmp9,*/ tmp10, tmp11, tmp12, tmp13;
	T z5, z10, z11, z12, z13;

	for (unsigned i = 0; i < 8; ++i) {
		
		tmp0 = in[0] * quant[0];
		tmp1 = in[2] * quant[2];
		tmp2 = in[4] * quant[4];
		tmp3 = in[6] * quant[6];

		tmp10 = tmp0 + tmp2;
		tmp11 = tmp0 - tmp2;

		tmp13 = tmp1 + tmp3;
		tmp12 = (tmp1 - tmp3) * m_R2 - tmp13;

		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		tmp4 = in[1] * quant[1];
		tmp5 = in[3] * quant[3];
		tmp6 = in[5] * quant[5];
		tmp7 = in[7] * quant[7];

		z13 = tmp6 + tmp5;
		z10 = tmp6 - tmp5;
		z11 = tmp4 + tmp7;
		z12 = tmp4 - tmp7;

		tmp7 = z11+ z13;
		tmp11 = (z11 - z13) * m_R2;

		z5 = (z10 + z12) * m_K2;
		tmp10 = m_M26 * z12 - z5;
		tmp12 = m_P26 * z10 + z5;

		tmp6 = tmp12 - tmp7;
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		out[0] = tmp0 + tmp7;
		out[7] = tmp0 - tmp7;
		out[1] = tmp1 + tmp6;
		out[6] = tmp1 - tmp6;
		out[2] = tmp2 + tmp5;
		out[5] = tmp2 - tmp5;
		out[4] = tmp3 + tmp4;
		out[3] = tmp3 - tmp4;
	}
}


template <typename T> void dct8x8<T>::inverse3D(arr8x8x8& in,arr8x8x8& out)
{
	T in8[8];
	T out8[8];
	T quant8[8];

	for(unsigned i=0;i<8;++i)
		quant8[i]=(T)1;

	scaleQuantizationTable_1D(quant8); // important step
	
	// inverse 1D-DCT 
	for (unsigned i = 0; i < 8; ++i) {
		for (unsigned j = 0; j < 8; ++j) {
			for (unsigned k = 0; k < 8; ++k) {
				in8[k] = out[k][i][j];
			}
			inverse1D(in8, quant8, out8);
			for (unsigned k = 0; k < 8; ++k) {
				out[k][i][j] = out8[k];
			}
		}
	}

	T quant[8][8];
	for (unsigned i = 0; i < 8; ++i)
		for (unsigned j = 0; j < 8; ++j)
			quant[i][j] = (T)1;

	scaleQuantizationTable(quant); // important step

	for (unsigned k = 0; k < 8; ++k) {
		inverse(out[k], quant, in[k]); // values may be slightly off due to precision errors (e.g. 9.999998 instead of 10)
	}
}

template <typename T> void dct8x8<T>::scaleQuantizationTable_1D(arr8& table)
{
	const T tmp=(T)1/(T)sqrt((T)8);
	for(unsigned i=0;i<8;++i)
		// table[i] = (T) (table[i] * _scaleFactor[i] / Math.sqrt(8));
		table[i] = (T) (table[i] * m_scaleFactor[i] * tmp);
}


/**
This method applies the pre-scaling that the inverse(T[][], T[][], T[][]) method needs to work correctly.
The table parameter should be 8x8, non-zigzag order.
*/
template <typename T> void dct8x8<T>::scaleQuantizationTable(arr8x8& table) {
	
	for (unsigned i = 0; i < 8; ++i)
		for (unsigned j = 0; j < 8; ++j)
			// table[i][j] = table[i][j] * _scaleFactor[i] * _scaleFactor[j] / 8;
			table[i][j] *= (m_scaleFactor[i] * m_scaleFactor[j] * (T)0.125);
}


/**
This method applies the pre-scaling that the inverse(T[], T[], T[]) method needs to work correctly.
The table parameter should be 1x64, non-zigzag order.
*/
template <typename T> void dct8x8<T>::scaleQuantizationTable(arr64& table) {
	unsigned k = 0;
	for (unsigned i = 0; i < 8; ++i) {
		for (unsigned j = 0; j < 8; ++j,++k) {
			// table[k] = table[k] * _scaleFactor[i] * _scaleFactor[j] / 8;
			table[k] *= (m_scaleFactor[i] * m_scaleFactor[j] * (T)0.125);
			// ++k;
		}
	}
}

// excplicit instantiation
#include "dct8x8.expl_templ_inst"
//template class dct8x8<double>;
//template class dct8x8<float>;
