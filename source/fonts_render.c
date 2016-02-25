#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memset

#include <sys/types.h>
#include <cell/font.h>
#include <cell/fontFT.h>

#include "fonts.h"


static uint32_t getUcs4( uint8_t*utf8, uint32_t*ucs4, uint32_t alterCode );

static void cellFontRenderTrans_blendCast_ARGB8( CellFontImageTransInfo* transInfo, uint32_t _color);
//                                                 uint8_t r, uint8_t g, uint8_t b );

//static void cellFontRenderTrans_AlphaCast_ARGB8( CellFontImageTransInfo* transInfo, uint8_t r, uint8_t g, uint8_t b );


static uint32_t getUcs4( uint8_t*utf8, uint32_t*ucs4, uint32_t alterCode )
{
	uint64_t code = 0L;
	uint32_t len = 0;

	code = (uint64_t)*utf8;

	if ( code ) {
		utf8++;
		len++;
		if ( code >= 0x80 ) {
			while (1) {

				if ( code & 0x40 ) {
					uint64_t mask = 0x20L;
					uint64_t encode;
					uint64_t n;

					for ( n=2;;n++ ) {
						if ( (code & mask) == 0 ) {
							len = n;
							mask--;
							if ( mask == 0 ) { // 0xFE or 0xFF

								*ucs4 = 0x00000000;
								return 0;
							}
							break;
						}
						mask = (mask >> 1);
					}
					code &= mask;

					for ( n=1; n<len; n++ ) {
						encode = (uint64_t)*utf8;
						if ( (encode & 0xc0) != 0x80 ) {

							if ( ucs4 ) *ucs4 = alterCode;
							return n;
						}
						code = ( ( code << 6 ) | (encode & 0x3f) );
						utf8++;
					}
					break;
				}
				else {


					for( ;; utf8++ ) {
						code = (uint64_t)*utf8;
						if ( code < 0x80 ) break;
						if ( code & 0x40 ) break;
						len++;
					}
					if ( code < 0x80 ) break;
				}
			}
		}
	}
	if ( ucs4 )  *ucs4 = (uint32_t)code;

	return len;
}



float Fonts_GetPropTextWidth( CellFont* cf,
                              uint8_t* utf8, float w, float h, float slant, float between,
                              float* strWidth, uint32_t* count )
{
	uint32_t code;
	float width=0.0f;
	float d = 0.0f;
	CellFontGlyphMetrics  metrics;
	uint32_t preFontId;
	uint32_t fontId;
	uint32_t cn = 0;
	int ret;

	if ( (! utf8)
	  || CELL_OK != cellFontSetScalePixel( cf, w, h )
	  || CELL_OK != cellFontSetEffectSlant( cf, slant )
	) {
		if ( strWidth ) *strWidth = 0.0f;
		if ( count    ) *count = 0;
		return width;
	}


	utf8 += getUcs4( utf8, &code, 0x3000 );

	ret = cellFontGetFontIdCode( cf, code, &fontId, (uint32_t*)0 );
	if ( ret != CELL_OK || code == 0 ) {
		if ( strWidth ) *strWidth = 0;
		if ( count    ) *count = 0;
		return 0.0f;
	}
	preFontId = fontId;


	if ( CELL_OK == cellFontGetCharGlyphMetrics( cf, code, &metrics ) ) {
		width = -(metrics.Horizontal.bearingX);

		width += metrics.Horizontal.advance + between;
	}
	else {
		width = 0.0f;

		width += w + between;
	}

	for (cn=1;;cn++) {

		utf8 += getUcs4( utf8, &code, 0x3000 );

		if ( code == 0x00000000 ) break;


		ret = cellFontGetFontIdCode( cf, code, &fontId, (uint32_t*)0 );
		if ( ret != CELL_OK ) {
			width += w + between;
			continue;
		}


		if ( fontId != preFontId ) {
			preFontId = fontId;
			cellFontSetEffectSlant( cf, 0.0f );
			ret = cellFontGetCharGlyphMetrics( cf, code, &metrics );
			if ( ret == CELL_OK ) {
				if ( metrics.Horizontal.bearingX < 0.0f ) {
					width += -(metrics.Horizontal.bearingX);
				}
			}
			cellFontSetEffectSlant( cf, slant );
			ret = cellFontGetCharGlyphMetrics( cf, code, &metrics );
		}
		else {

			ret = cellFontGetCharGlyphMetrics( cf, code, &metrics );
		}

		if ( ret == CELL_OK ) {
			width += d = metrics.Horizontal.advance + between;
		}
		else {

			metrics.Horizontal.advance  = w;
			metrics.Horizontal.bearingX = metrics.width = 0;
			width += d = w + between;
		}
	}

	if ( strWidth ) *strWidth = width;

	width += - d + metrics.Horizontal.bearingX + metrics.width;

	return width;
}

/*
float Fonts_GetVerticalTextHeight( CellFont* cf,
                                   uint8_t* utf8, float w, float h, float between,
                                   float* strHeight, uint32_t* count )
{
	uint32_t code;
	CellFontGlyphMetrics metrics;
	float height = 0.0f;
	int flag = 0;
	float d = 0.0f;
	int n;

	if ( (!utf8) || *utf8 == 0x00
	  || CELL_OK != cellFontSetScalePixel( cf, w, h )
	) {
		if ( strHeight ) *strHeight = 0;
		if ( count ) *count = 0;
		return 0;
	}

	for ( n=0;;n++ ) {

		utf8 += getUcs4( utf8, &code, 0x3000 );

		if ( code == 0x00000000 ) break;

		if ( flag <= 0 ) {

			if ( flag && ( code == 0x300d || code == 0x300f || code == 0xfe42 || code == 0xfe44 ) ) {
				height += - d + metrics.Vertical.bearingY + metrics.height;
			}
		}

		d = 0;

		if ( CELL_OK == cellFontGetCharGlyphMetricsVertical( cf, code, &metrics ) ) {

			if ( flag < 0 || metrics.Vertical.bearingY < 0 ) {
				height += -metrics.Vertical.bearingY;
			}
			flag = 1;

			height += d = metrics.Vertical.advance + between;

			if ( code==0x20 || ( code >= 0x3000 && code <= 0x3002 ) ) flag = -1;
		}
		else {
			metrics.Vertical.advance  =
			metrics.Vertical.bearingY =
			metrics.height = 0.0f;

			height += d = h + between;
			flag = -1;
		}
	}
	if ( strHeight ) *strHeight = height;
	if ( count ) *count = n;

	height += - d + metrics.Vertical.bearingY + metrics.height;

	return height;
}
*/


float Fonts_GetTextRescale( float scale, float w, float newW, float*ratio )
{
	float rate, rescale;

	rate = (newW/w);
	rescale = scale * rate;

	if ( ratio ) *ratio = rate;

	return rescale;
}
float Fonts_GetPropTextWidthRescale( float scale, float w, float newW, float*ratio )
{
	return Fonts_GetTextRescale( scale, w, newW, ratio );
}

/*float Fonts_GetVerticalTextHeightRescale( float scale, float h, float newH, float*ratio )
{
	return Fonts_GetTextRescale( scale, h, newH, ratio );
}*/

int Fonts_BindRenderer( CellFont* cf, CellFontRenderer* rend )
{

	int ret = cellFontBindRenderer( cf, rend );

	return ret;
}

int Fonts_UnbindRenderer( CellFont* cf )
{

	int ret = cellFontUnbindRenderer( cf );

	return ret;
}



float Fonts_RenderPropText( CellFont* cf,
                            CellFontRenderSurface* surf, float x, float y,
                            uint8_t* utf8, float w, float h, float slant, float between, int32_t _color )
{
	uint32_t code;
	CellFontGlyphMetrics  metrics;
	CellFontImageTransInfo TransInfo;
	uint32_t preFontId;
//	uint32_t fontId;
	(void) slant;
	int ret;

	if ( (!utf8) || *utf8 == 0x00 ) return x;


	ret = cellFontSetupRenderScalePixel( cf, w, h );
	if ( ret != CELL_OK ) {
//		Fonts_PrintError("Fonts_RenderPropText:",ret);
		return x;
	}


	utf8 += getUcs4( utf8, &code, 0x3000 );


	{
		cellFontGetFontIdCode( cf, code, &preFontId, (uint32_t*)0 );


		if ( CELL_OK == cellFontGetRenderCharGlyphMetrics( cf, code, &metrics ) ) {

			x += -(metrics.Horizontal.bearingX);
		}
	}

	for ( ;; ) {

		ret = cellFontRenderCharGlyphImage( cf, code, surf, x, y, &metrics, &TransInfo );
		if ( ret == CELL_OK ) {

			x += metrics.Horizontal.advance + between;


//			cellFontRenderTrans_blendCast_ARGB8( &TransInfo, 255, 255, 255 );

			cellFontRenderTrans_blendCast_ARGB8( &TransInfo, _color);

//			cellFontRenderTrans_AlphaCast_ARGB8( &TransInfo, 255, 255, 255 );
		}
		else {

			x += w + between;
		}


		utf8 += getUcs4( utf8, &code, 0x3000 );

		if ( code == 0x00000000 ) break;

	}

	return x;
}

/*
float Fonts_RenderVerticalText( CellFont* cf,
                            CellFontRenderSurface* surf, float x, float y,
                            uint8_t* utf8, float w, float h, float slant, float between )
{
	uint32_t code;
	CellFontGlyphMetrics   metrics;
	CellFontImageTransInfo TransInfo;
	int flag = 0;
	float d = 0.0f;
	int ret;

	if ( (!utf8) || *utf8 == 0x00 ) return y;


	ret = cellFontSetupRenderScalePixel( cf, w, h );
	if ( ret != CELL_OK ) {
//		Fonts_PrintError("Fonts_RenderVerticalText:",ret);
		return y;
	}

	for ( ;; ) {

		utf8 += getUcs4( utf8, &code, 0x3000 );

		if ( code == 0x00000000 ) break;

		if ( flag <= 0 ) {

			if ( flag && ( code == 0x300d || code == 0x300f || code == 0xff42 || code == 0xff4f ) ) {
				y += d = -d + metrics.Vertical.bearingY + metrics.height;
				x -= d * slant;
			}
		}
		d = 0;

		if ( CELL_OK == cellFontGetRenderCharGlyphMetricsVertical( cf, code, &metrics ) ) {

			if ( flag < 0 || metrics.Vertical.bearingY < 0 ) {
				y += d = -metrics.Vertical.bearingY;
				x -= d * slant;
			}
		}
		flag = 1;

		ret = cellFontRenderCharGlyphImageVertical( cf, code, surf, x, y, &metrics, &TransInfo );
		if ( ret == CELL_OK ) {

			y += d = metrics.Vertical.advance + between;
			x -= d * slant;


			cellFontRenderTrans_blendCast_ARGB8( &TransInfo, 0x00ffffff);


			if ( code==0x20 || ( code >= 0x3000 && code <= 0x3002 ) ) flag = -1;
		}
		else {

			y -= d;
			x += d * slant;

			metrics.Vertical.advance  =
			metrics.Vertical.bearingY =
			metrics.height = 0.0f;
			y += d = h + between;
			x -= d * slant;
			flag = -1;
		}
	}

	return y;
}*/


static void cellFontRenderTrans_blendCast_ARGB8( CellFontImageTransInfo* transInfo, uint32_t _color)
//                                                 uint8_t r, uint8_t g, uint8_t b )
{
	if ( transInfo ) {
		unsigned char* tex;
		unsigned char* img = transInfo->Image;
		int img_bw = transInfo->imageWidthByte;
		int tex_bw = transInfo->surfWidthByte;
		int w = transInfo->imageWidth-1;
		int h = transInfo->imageHeight;
		uint64_t a0,a1,_a;
		uint64_t ARGBx2;
		uint64_t R0, G0, B0;//A0,
		uint64_t A0, A1, R1, G1, B1;
		int x, y;
		float d_alpha1, d_alpha0, d_tmp, d_tmp2;
		uint8_t _r, _b, _g;
		uint8_t _r0, _b0, _g0, _r1, _g1, _b1;

		_a = (_color>>24) & 0xff;
		_b = (_color>>16) & 0xff;
		_g = (_color>> 8) & 0xff;
		_r = (_color    ) & 0xff;


		for ( y=0; y < h; y++ ) {
			tex = ((uint8_t*)transInfo->Surface) + tex_bw*y;
			for ( x=0; x < w; x++ ) {
/*
				if ( (((sys_addr_t)tex) & 7) || x == w-1 ) {
					a1 = img[x];


					if ( a1 ) {
						ARGBx2 = *(uint32_t*)tex;
						//A1=_a;
						//A1 = (255-a1);
						R1 = ((ARGBx2>>16)&0xff);
						G1 = ((ARGBx2>> 8)&0xff);
						B1 = ((ARGBx2    )&0xff);

						d_tmp=(a1 * _a)/65025.f;
						//d_alpha1 = (a1/255.0f)  * (_a/255.0f);
						d_alpha1 = d_tmp;
						_r1 = (int) (_r * d_alpha1);
						_g1 = (int) (_g * d_alpha1);
						_b1 = (int) (_b * d_alpha1);

						d_alpha1 = 1.0f - d_tmp;
						R1 = (int) (R1 * d_alpha1) + _r1;
						G1 = (int) (G1 * d_alpha1) + _g1;
						B1 = (int) (B1 * d_alpha1) + _b1;

						*(uint32_t*)tex =
						             (a1> _a ? a1 : _a) |
										 (R1<<24)|
										 (G1<<16)|
										 (B1<<8);

					}
					tex += 4;
				}
				else*/{
					a0 = img[x]; x++;
					a1 = img[x];
					if ( a0 || a1 ) {
						ARGBx2 = *(uint64_t*)tex;
						//A0 = (255-a0);
						//A1 = (255-a1);


						d_tmp2=(a1 * _a)/65025.f;
						d_alpha1 = d_tmp2;
						//d_alpha1 = (a1/255.0f)  * (_a/255.0f);
						_r1 = (int) (_r * d_alpha1);
						_g1 = (int) (_g * d_alpha1);
						_b1 = (int) (_b * d_alpha1);

						d_tmp=(a0 * _a)/65025.f;
						d_alpha0 = d_tmp;
						//d_alpha0 = (a0/255.0f)  * (_a/255.0f);
						_r0 = (int) (_r * d_alpha0);
						_g0 = (int) (_g * d_alpha0);
						_b0 = (int) (_b * d_alpha0);


						R0 = ((ARGBx2>>56)&0xff);
						G0 = ((ARGBx2>>48)&0xff);
						B0 = ((ARGBx2>>40)&0xff);
						A0 = ((ARGBx2>>32)&0xff);

						R1 = ((ARGBx2>>24)&0xff);
						G1 = ((ARGBx2>>16)&0xff);
						B1 = ((ARGBx2>> 8)&0xff);
						A1 = ((ARGBx2	 )&0xff);

						d_alpha0 = 1.0f - d_tmp;
						//d_alpha0 = 1.0f - (a0/255.0f)  * (_a/255.0f);
						R0 = (int) (R0 * d_alpha0) + _r0;
						G0 = (int) (G0 * d_alpha0) + _g0;
						B0 = (int) (B0 * d_alpha0) + _b0;

						d_alpha1 = 1.0f - d_tmp2;
						//d_alpha1 = 1.0f - (a1/255.0f)  * (_a/255.0f);
						R1 = (int) (R1 * d_alpha1) + _r1;
						G1 = (int) (G1 * d_alpha1) + _g1;
						B1 = (int) (B1 * d_alpha1) + _b1;


/*						*(uint64_t*)tex =
						             (a0<<56) |
						             (( A0 * R0 + a0 * _r )/255<<48)|
						             (( A0 * G0 + a0 * _g )/255<<40)|
						             (( A0 * B0 + a0 * _b )/255<<32)|
						             (a1<<24) |
						             (( A1 * R1 + a1 * _r )/255<<16)|
						             (( A1 * G1 + a1 * _g )/255<< 8)|
						             (( A1 * B1 + a1 * _b )/255    );
*/

						*(uint64_t*)tex =
						             (a0 >= A0 ? a0<<32 : A0<<32) |
										 (R0<<56)|
										 (G0<<48)|
										 (B0<<40)|
						             (a1 >= A1 ? a1 : A1) |
										 (R1<<24)|
										 (G1<<16)|
										 (B1<<8);



					}
					tex += 8;
				}
			}
			img += img_bw;
		}
	}
	return;
}


