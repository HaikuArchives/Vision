#ifndef GRAPHIC_DEFS__
#define GRAPHIC_DEFS__

#include "PlatformDefines.h"


const uint8 B_TRANSPARENT_8_BIT = 0;

typedef enum
{
	B_NO_COLOR_SPACE =	0x0000,	/* byte in memory order, high bit first				*/
	
	/* linear color space (little endian is the default) */
	B_RGB32 = 			0x0008,	/* B[7:0]  G[7:0]  R[7:0]  -[7:0]					*/
	B_RGBA32 = 			0x2008,	/* B[7:0]  G[7:0]  R[7:0]  A[7:0]					*/
	B_RGB24 = 			0x0003,	/* B[7:0]  G[7:0]  R[7:0]							*/
	B_RGB16 = 			0x0005,	/* G[2:0],B[4:0]  R[4:0],G[5:3]						*/
	B_RGB15 = 			0x0010,	/* G[2:0],B[4:0]  	   -[0],R[4:0],G[4:3]			*/
	B_RGBA15 = 			0x2010,	/* G[2:0],B[4:0]  	   A[0],R[4:0],G[4:3]			*/
	B_CMAP8 = 			0x0004,	/* D[7:0]  											*/
	B_GRAY8 = 			0x0002,	/* Y[7:0]											*/
	B_GRAY1 = 			0x0001,	/* Y0[0],Y1[0],Y2[0],Y3[0],Y4[0],Y5[0],Y6[0],Y7[0]	*/

	/* big endian version, when the encoding is not endianess independant */
	B_RGB32_BIG =		0x1008,	/* -[7:0]  R[7:0]  G[7:0]  B[7:0]					*/
	B_RGBA32_BIG = 		0x3008,	/* A[7:0]  R[7:0]  G[7:0]  B[7:0]					*/
	B_RGB24_BIG = 		0x1003,	/* R[7:0]  G[7:0]  B[7:0]							*/
	B_RGB16_BIG = 		0x1005,	/* R[4:0],G[5:3]  G[2:0],B[4:0]						*/
	B_RGB15_BIG = 		0x1010,	/* -[0],R[4:0],G[4:3]  G[2:0],B[4:0]				*/
	B_RGBA15_BIG = 		0x3010,	/* A[0],R[4:0],G[4:3]  G[2:0],B[4:0]				*/

	/* little-endian declarations, for completness */
	B_RGB32_LITTLE = 	B_RGB32,
	B_RGBA32_LITTLE =	B_RGBA32,
	B_RGB24_LITTLE =	B_RGB24,
	B_RGB16_LITTLE =	B_RGB16,
	B_RGB15_LITTLE =	B_RGB15,
	B_RGBA15_LITTLE =	B_RGBA15,

	/* non linear color space -- note that these are here for exchange purposes;	*/
	/* a BBitmap or BView may not necessarily support all these color spaces.	*/

	/* Loss/Saturation points are Y 16-235 (absoulte); Cb/Cr 16-240 (center 128) */

	B_YCbCr422 = 		0x4000,	/* Y0[7:0]  Cb0[7:0]  Y1[7:0]  Cr0[7:0]  Y2[7:0]...	*/
								/* Cb2[7:0]  Y3[7:0]  Cr2[7:0]						*/
	B_YCbCr411 = 		0x4001,	/* Cb0[7:0]  Y0[7:0]  Cr0[7:0]  Y1[7:0]  Cb4[7:0]...*/
								/* Y2[7:0]  Cr4[7:0]  Y3[7:0]  Y4[7:0]  Y5[7:0]...	*/
								/* Y6[7:0]  Y7[7:0]	 								*/
	B_YCbCr444 = 		0x4003,	/* Y0[7:0]  Cb0[7:0]  Cr0[7:0]		*/
	B_YCbCr420 = 		0x4004,	/* Non-interlaced only, Cb0  Y0  Y1  Cb2 Y2  Y3  on even scan lines ... */
								/* Cr0  Y0  Y1  Cr2 Y2  Y3  on odd scan lines */

	/* Extrema points are Y 0 - 207 (absolute) U -91 - 91 (offset 128) V -127 - 127 (offset 128) */
	/* note that YUV byte order is different from YCbCr */
	/* USE YCbCr, not YUV, when that's what you mean! */
	B_YUV422 =			0x4020, /* U0[7:0]  Y0[7:0]   V0[7:0]  Y1[7:0] ... */
								/* U2[7:0]  Y2[7:0]   V2[7:0]  Y3[7:0]  */
	B_YUV411 =			0x4021, /* U0[7:0]  Y0[7:0]  Y1[7:0]  V0[7:0]  Y2[7:0]  Y3[7:0]  */
								/* U4[7:0]  Y4[7:0]  Y5[7:0]  V4[7:0]  Y6[7:0]  Y7[7:0]  */
	B_YUV444 =			0x4023,	/* U0[7:0]  Y0[7:0]  V0[7:0]  U1[7:0]  Y1[7:0]  V1[7:0] */
	B_YUV420 = 			0x4024,	/* Non-interlaced only, U0  Y0  Y1  U2 Y2  Y3  on even scan lines ... */
								/* V0  Y0  Y1  V2 Y2  Y3  on odd scan lines */
	B_YUV9 = 			0x402C,	/* planar?	410?								*/
	B_YUV12 = 			0x402D,	/* planar?	420?								*/

	B_UVL24 =			0x4030,	/* U0[7:0] V0[7:0] L0[7:0] ... */
	B_UVL32 =			0x4031,	/* U0[7:0] V0[7:0] L0[7:0] X0[7:0]... */
	B_UVLA32 =			0x6031,	/* U0[7:0] V0[7:0] L0[7:0] A0[7:0]... */

	B_LAB24 =			0x4032,	/* L0[7:0] a0[7:0] b0[7:0] ...  (a is not alpha!) */
	B_LAB32 =			0x4033,	/* L0[7:0] a0[7:0] b0[7:0] X0[7:0] ... (b is not alpha!) */
	B_LABA32 =			0x6033,	/* L0[7:0] a0[7:0] b0[7:0] A0[7:0] ... (A is alpha) */

	/* red is at hue = 0 */

	B_HSI24 =			0x4040,	/* H[7:0]  S[7:0]  I[7:0]							*/
	B_HSI32 =			0x4041,	/* H[7:0]  S[7:0]  I[7:0]  X[7:0]					*/
	B_HSIA32 =			0x6041,	/* H[7:0]  S[7:0]  I[7:0]  A[7:0]					*/

	B_HSV24 =			0x4042,	/* H[7:0]  S[7:0]  V[7:0]							*/
	B_HSV32 =			0x4043,	/* H[7:0]  S[7:0]  V[7:0]  X[7:0]					*/
	B_HSVA32 =			0x6043,	/* H[7:0]  S[7:0]  V[7:0]  A[7:0]					*/

	B_HLS24 =			0x4044,	/* H[7:0]  L[7:0]  S[7:0]							*/
	B_HLS32 =			0x4045,	/* H[7:0]  L[7:0]  S[7:0]  X[7:0]					*/
	B_HLSA32 =			0x6045,	/* H[7:0]  L[7:0]  S[7:0]  A[7:0]					*/

	B_CMY24 =			0xC001,	/* C[7:0]  M[7:0]  Y[7:0]  			No gray removal done		*/
	B_CMY32 =			0xC002,	/* C[7:0]  M[7:0]  Y[7:0]  X[7:0]	No gray removal done		*/
	B_CMYA32 =			0xE002,	/* C[7:0]  M[7:0]  Y[7:0]  A[7:0]	No gray removal done		*/
	B_CMYK32 =			0xC003,	/* C[7:0]  M[7:0]  Y[7:0]  K[7:0]					*/

	/* compatibility declarations */
	B_MONOCHROME_1_BIT = 	B_GRAY1,
	B_GRAYSCALE_8_BIT =		B_GRAY8,
	B_COLOR_8_BIT =			B_CMAP8,
	B_RGB_32_BIT =			B_RGB32,
	B_RGB_16_BIT =			B_RGB15,
	B_BIG_RGB_32_BIT =		B_RGB32_BIG,
	B_BIG_RGB_16_BIT =		B_RGB15_BIG
} color_space;


enum drawing_mode {
	B_OP_COPY,
	B_OP_OVER,
	B_OP_ERASE,
	B_OP_INVERT,
	B_OP_ADD,
	B_OP_SUBTRACT,
	B_OP_BLEND,
	B_OP_MIN,
	B_OP_MAX,
	B_OP_SELECT,
	B_OP_ALPHA
};

enum alpha_function {
	B_ALPHA_OVERLAY=0,
	B_ALPHA_COMPOSITE
};

enum source_alpha {
	B_PIXEL_ALPHA=0,
	B_CONSTANT_ALPHA
};

#endif

