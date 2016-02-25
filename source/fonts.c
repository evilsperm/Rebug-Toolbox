#include <stdio.h>
#include <stdlib.h>
#include <sys/paths.h>

#include <cell/font.h>
#include <cell/fontFT.h>

#include "fonts.h"

static Fonts_t Fonts;
extern char app_usrdir;
static void* fonts_malloc( void*, uint32_t size );
static void  fonts_free( void*, void*p );
static void* fonts_realloc( void*, void* p, uint32_t size );
static void* fonts_calloc( void*, uint32_t numb, uint32_t blockSize );
//static void* loadFileF( uint8_t* fname, size_t *size, int offset, int addSize );

static void* fonts_malloc( void*obj, uint32_t size )
{
	obj=NULL;
	return memalign(16, size );
}
static void  fonts_free( void*obj, void*p )
{
	obj=NULL;
	free( p );
}
static void* fonts_realloc( void*obj, void* p, uint32_t size )
{
	obj=NULL;
	return realloc( p, size );
}
static void* fonts_calloc( void*obj, uint32_t numb, uint32_t blockSize )
{
	obj=NULL;
	return calloc( numb, blockSize );
}

/*
static void* loadFileF( uint8_t* fname, size_t *size, int offset, int addSize )
{
	FILE* fp;
	size_t file_size;
	void* p;

	fp = fopen( (const char*)fname, "rb" );
	if (! fp ) {
//		printf("cannot open %s\n", fname );
		if ( size ) *size = 0;
		return NULL;
	}

	fseek( fp, 0, SEEK_END );
	file_size = ftell( fp );
	fseek( fp, 0, SEEK_SET );
	if ( size ) *size = file_size;

	p = memalign(16, file_size + offset + addSize );
	if ( p ) {
		fread( (unsigned char*)p+offset, file_size, 1, fp );
	}

	fclose( fp );

	return p;
}
*/


int Fonts_LoadModules()
{
	int ret;


	ret = cellSysmoduleLoadModule( CELL_SYSMODULE_FONT );
	if ( ret == CELL_OK ) {

		ret = cellSysmoduleLoadModule( CELL_SYSMODULE_FREETYPE );
		if ( ret == CELL_OK ) {

			ret = cellSysmoduleLoadModule( CELL_SYSMODULE_FONTFT );
			if ( ret == CELL_OK ) {
				return ret;
			}


			cellSysmoduleUnloadModule( CELL_SYSMODULE_FREETYPE );
		}
//		else printf("Fonts: 'CELL_SYSMODULE_FREETYPE' NG! %08x\n",ret);

		cellSysmoduleUnloadModule( CELL_SYSMODULE_FONT );
	}
//	else printf("Fonts: 'CELL_SYSMODULE_FONT' NG! %08x\n",ret);

	return ret;
}


void Fonts_UnloadModules()
{
	cellSysmoduleUnloadModule( CELL_SYSMODULE_FONTFT );
	cellSysmoduleUnloadModule( CELL_SYSMODULE_FREETYPE );
	cellSysmoduleUnloadModule( CELL_SYSMODULE_FONT );
}


Fonts_t* Fonts_Init()
{

	static CellFontEntry UserFontEntrys[USER_FONT_MAX];
	static uint32_t FontFileCache[FONT_FILE_CACHE_SIZE/sizeof(uint32_t)];

	CellFontConfig config;
	int ret;


	//----------------------------------------------------------------------
	CellFontConfig_initialize( &config );


	config.FileCache.buffer = FontFileCache;
	config.FileCache.size   = FONT_FILE_CACHE_SIZE;


	config.userFontEntrys   = UserFontEntrys;
	config.userFontEntryMax = sizeof(UserFontEntrys)/sizeof(CellFontEntry);


	config.flags = 0;


	//----------------------------------------------------------------------
	ret = cellFontInit( &config );
	if ( ret != CELL_OK ) {
//		Fonts_PrintError( "   Fonts:cellFontInit=", ret );
		return (Fonts_t*)0;
	}

	return &Fonts;
}



int Fonts_InitLibraryFreeType( const CellFontLibrary** lib )
{
	CellFontLibraryConfigFT config;
	const CellFontLibrary*  fontLib;
	int ret;


	CellFontLibraryConfigFT_initialize( &config );

	config.MemoryIF.Object  = NULL;
	config.MemoryIF.Malloc  = fonts_malloc;
	config.MemoryIF.Free    = fonts_free;
	config.MemoryIF.Realloc = fonts_realloc;
	config.MemoryIF.Calloc  = fonts_calloc;


	ret = cellFontInitLibraryFreeType( &config, &fontLib );
//	if ( ret != CELL_OK ) {
//		Fonts_PrintError( "   Fonts:cellFontInitLibrary_FreeType=", ret );
//	}
	if ( lib ) *lib = fontLib;

	return ret;
}


int Fonts_CreateRenderer( const CellFontLibrary* lib, uint32_t initSize, CellFontRenderer* rend )
{
	CellFontRendererConfig config;
	int ret;

	CellFontRendererConfig_initialize( &config );
	CellFontRendererConfig_setAllocateBuffer( &config, initSize, 0 );


	ret = cellFontCreateRenderer( lib, &config, rend );
	if ( ret != CELL_OK ) {
//		Fonts_PrintError( "   Fonts:cellFontCreateRenderer=", ret );
		return ret;
	}

	return ret;
}


int Fonts_OpenFonts( const CellFontLibrary*lib, Fonts_t* fonts, char* _app_usrdir )
{
	(void)_app_usrdir;
	int n;
	int ret=CELL_OK;


	{
		static struct {
			uint32_t isMemory;
			int      fontsetType;
		} openSystemFont[ SYSTEM_FONT_MAX ] = {

			{ 0, CELL_FONT_TYPE_GOTHIC_JAPANESE_CJK_LATIN2_SET }, //CELL_FONT_TYPE_DEFAULT_GOTHIC_LATIN_SET
			{ 0, CELL_FONT_TYPE_DEFAULT_GOTHIC_JP_SET    },
			{ 0, CELL_FONT_TYPE_DEFAULT_SANS_SERIF       },
			{ 0, CELL_FONT_TYPE_DEFAULT_SERIF            },
			{ 0, CELL_FONT_TYPE_ROUND_SANS_EUROPEAN_CJK_LATIN_SET },
			{ 0, CELL_FONT_TYPE_GOTHIC_SCHINESE_CJK_JP_SET },
			{ 0, CELL_FONT_TYPE_GOTHIC_TCHINESE_CJK_JP_SET },
			{ 0, CELL_FONT_TYPE_GOTHIC_JAPANESE_CJK_JP_SET },
			{ 0, CELL_FONT_TYPE_YD_GOTHIC_KOREAN },
			{ 0, CELL_FONT_TYPE_DEFAULT_SANS_SERIF       }
		};
		CellFontType type;

		fonts->sysFontMax = 9;

		for ( n=0; n < fonts->sysFontMax; n++ ) {
			if (! openSystemFont[n].isMemory ) continue;

			type.type = openSystemFont[n].fontsetType;
			type.map  = CELL_FONT_MAP_UNICODE;

			ret = cellFontOpenFontsetOnMemory( lib, &type, &fonts->SystemFont[n] );
			if ( ret != CELL_OK ) {
//				Fonts_PrintError( "   Fonts:cellFontOpenFontset=", ret );
				Fonts_CloseFonts( fonts );
				return ret;
			}
			fonts->openState |= (1<<n);

			//cellFontSetResolutionDpi( &fonts->SystemFont[n], 36, 36 );
			//cellFontSetScalePoint( &fonts->SystemFont[n], 18.f, 10.125f );

			cellFontSetResolutionDpi( &fonts->SystemFont[n], 72, 72 );
			cellFontSetScalePoint( &fonts->SystemFont[n], 32.f, 32.f );
		}

		for ( n=0; n < fonts->sysFontMax; n++ ) {
			if ( openSystemFont[n].isMemory ) continue;

			type.type = openSystemFont[n].fontsetType;
			type.map  = CELL_FONT_MAP_UNICODE;

			ret = cellFontOpenFontset( lib, &type, &fonts->SystemFont[n] );
			if ( ret != CELL_OK ) {
//				Fonts_PrintError( "   Fonts:cellFontOpenFontset=", ret );
				Fonts_CloseFonts( fonts );
				return ret;
			}
			fonts->openState |= (1<<n);

			//cellFontSetResolutionDpi( &fonts->SystemFont[n], 36, 36 );
			//cellFontSetScalePoint( &fonts->SystemFont[n], 18.f, 10.125f );
			cellFontSetResolutionDpi( &fonts->SystemFont[n], 72, 72 );
			cellFontSetScalePoint( &fonts->SystemFont[n], 32.f, 32.f );
		}
	}

	fonts->userFontMax = 0;
/*
	{
		char ufontpath[256];
		static struct {
			uint32_t isMemory;
			const char* filePath;
		} userFont[ USER_FONT_MAX ] = {
			{ 0, "font0.ttf" }, //arial
			{ 0, "font1.ttf" }, //tekton
			{ 0, "font2.ttf" }, //coperplate
			{ 0, "font3.ttf" }, //ps3 rodin
			{ 0, "font4.ttf" }, //kristen
			{ 1, "fontA.ttf" }, //bank gothic
			{ 1, "fontB.ttf" }, //typewriter
			{ 0, "fontC.ttf" }, //monospaced
			{ 0, "fontD.ttf" }, //spiderman
			{ 1, "fontE.ttf" }, //arial narro
		};
		uint32_t fontUniqueId = 0;

		fonts->userFontMax = 10;

		for ( n=0; n < fonts->userFontMax; n++ ) {
			if(n<5)
				sprintf(ufontpath, "%s/fonts/user/font%i.ttf", _app_usrdir, n);// userFont[n].filePath);
			else
				sprintf(ufontpath, "%s/fonts/system/font%c.ttf", _app_usrdir, n+60);// userFont[n].filePath);
			uint8_t* path = (uint8_t*)ufontpath; //userFont[n].filePath

			if (! userFont[n].isMemory ) {

				ret = cellFontOpenFontFile( lib, path, 0, fontUniqueId, &fonts->UserFont[n] );
				if ( ret != CELL_OK ) {
//					Fonts_PrintError( "    Fonts:cellFontOpenFile=", ret );
					Fonts_CloseFonts( fonts );
//					printf("    Fonts:   [%s]\n", userFont[n].filePath);
					return ret;
				}
			}
			else {

				size_t size;
				void *p = loadFileF( path, &size, 0, 0 );

				ret = cellFontOpenFontMemory( lib, p, size, 0, fontUniqueId, &fonts->UserFont[n] );
				if ( ret != CELL_OK ) {
					if (p) free( p );
//					Fonts_PrintError( "    Fonts:cellFontMemory=", ret );
					Fonts_CloseFonts( fonts );
//					printf("    Fonts:   [%s]\n", userFont[n].filePath);
					return ret;
				}
			}
			fonts->openState |= (1<<(FONT_USER_FONT0+n));
			fontUniqueId++;

			if ( ret == CELL_OK ) {

				cellFontSetResolutionDpi( &fonts->UserFont[n], 72, 72 );
				cellFontSetScalePoint( &fonts->UserFont[n], 32.f, 32.f );
			}
		}
	}
	*/

	return ret;
}


int Fonts_GetFontsHorizontalLayout( Fonts_t* fonts, uint32_t fontmask, float scale, float* lineHeight, float*baseLineY )
{
	float ascent  = 0.0f;
	float descent = 0.0f;
	int ret = CELL_OK;

	if ( fonts ) {
		CellFont* cf;
		CellFontHorizontalLayout Layout;
		int n;


		for ( n=0; n < fonts->sysFontMax; n++ ) {
			if ( (fontmask & (1<<n))==0x00000000 ) continue;

			cf = &fonts->SystemFont[n];

			ret = cellFontSetScalePixel( cf, scale, scale );
//			if ( ret != CELL_OK ) {
//				Fonts_PrintError( "    Fonts.SystemFont:cellFontSetScalePixel=", ret );
//			}

			ret = cellFontGetHorizontalLayout( cf, &Layout );
//			if ( ret != CELL_OK ) {
//				Fonts_PrintError( "    Fonts.SystemFont:cellFontGetHorizontalLayout=", ret );
//			}

			if ( Layout.baseLineY > ascent ) {
				ascent = Layout.baseLineY;
			}
			if ( Layout.lineHeight - Layout.baseLineY > descent ) {
				descent = Layout.lineHeight - Layout.baseLineY;
			}
		}

		for ( n=0; n < fonts->userFontMax; n++ ) {
			if ( (fontmask & (1<<(FONT_USER_FONT0+n)))==0x00000000 ) continue;

			cf = &fonts->UserFont[n];

			ret = cellFontSetScalePixel( cf, scale, scale );
//			if ( ret != CELL_OK ) {
//				Fonts_PrintError( "    Fonts.SystemFont:cellFontSetScalePixel=", ret );
//			}

			ret = cellFontGetHorizontalLayout( cf, &Layout );
//			if ( ret != CELL_OK ) {
//				Fonts_PrintError( "    Fonts.UserFont:cellFontGetHorizontalLayout=", ret );
//			}

			if ( Layout.baseLineY > ascent ) {
				ascent = Layout.baseLineY;
			}
			if ( Layout.lineHeight - Layout.baseLineY > descent ) {
				descent = Layout.lineHeight - Layout.baseLineY;
			}
		}
	}
	if ( lineHeight ) *lineHeight = ascent + descent;
	if ( baseLineY  ) *baseLineY  = ascent;

	return CELL_OK;
}


int Fonts_AttachFont( Fonts_t* fonts, int fontEnum, CellFont*cf )
{
	CellFont* openedFont = (CellFont*)0;
	int ret;

	if ( fonts ) {
		if ( fontEnum < FONT_USER_FONT0 ) {
			uint32_t n = fontEnum;

			if ( n < (uint32_t)fonts->sysFontMax ) {
				if ( fonts->openState & (1<<fontEnum) ) {
					openedFont = &fonts->SystemFont[ n ];
				}
			}
		}
		else {
			uint32_t n = fontEnum - FONT_USER_FONT0;

			if ( n < (uint32_t)fonts->userFontMax ) {
				if ( fonts->openState & (1<<fontEnum) ) {
					openedFont = &fonts->UserFont[ n ];
				}
			}
		}
	}

	ret = cellFontOpenFontInstance( openedFont, cf );
//	if ( ret != CELL_OK ) {
//		Fonts_PrintError( "    Fonts:AttachFont:cellFontOpenFontInstance=", ret );
//	}
	return ret;
}


int Fonts_SetFontScale( CellFont* cf, float scale )
{
	int ret;

	ret = cellFontSetScalePixel( cf, scale, scale );
//	if ( ret != CELL_OK ) {
//		Fonts_PrintError( "    Fonts:cellFontSetScalePixel=", ret );
//	}
	return ret;
}


int Fonts_SetFontEffectWeight( CellFont* cf, float effWeight )
{
	int ret;

	ret = cellFontSetEffectWeight( cf, effWeight );
//	if ( ret != CELL_OK ) {
//		Fonts_PrintError( "    Fonts:cellFontSetEffectWeight=", ret );
//	}
	return ret;
}


int Fonts_SetFontEffectSlant( CellFont* cf, float effSlant )
{
	int ret;

	ret = cellFontSetEffectSlant( cf, effSlant );
//	if ( ret != CELL_OK ) {
//		Fonts_PrintError( "    Fonts:cellFontSetEffectSlant=", ret );
//	}
	return ret;
}


int Fonts_GetFontHorizontalLayout( CellFont* cf, float* lineHeight, float*baseLineY )
{
	CellFontHorizontalLayout Layout;
	int ret;

	ret = cellFontGetHorizontalLayout( cf, &Layout );
	if ( ret != CELL_OK ) {
//		Fonts_PrintError( "    Fonts:cellFontGetHorizontalLayout=", ret );
		return ret;
	}
	if ( lineHeight ) *lineHeight = Layout.lineHeight;
	if ( baseLineY  ) *baseLineY  = Layout.baseLineY;

	return ret;
}

/*
int Fonts_GetFontVerticalLayout( CellFont* cf, float* lineWidth, float*baseLineX )
{
	CellFontVerticalLayout Layout;
	int ret;

	ret = cellFontGetVerticalLayout( cf, &Layout );
	if ( ret != CELL_OK ) {
//		Fonts_PrintError( "    Fonts:cellFontGetVerticalLayout=", ret );
		return ret;
	}
	if ( lineWidth ) *lineWidth = Layout.lineWidth;
	if ( baseLineX  ) *baseLineX  = Layout.baseLineX;

	return ret;
}
*/

int Fonts_DetachFont( CellFont*cf )
{
	int ret = cellFontCloseFont( cf );

//	if ( ret != CELL_OK ) {
//		Fonts_PrintError( "    Fonts:cellFontCloseFont=", ret );
//	}
	return ret;
}


int Fonts_CloseFonts( Fonts_t* fonts )
{
	int n;
	int ret, err = CELL_FONT_OK;

	if (! fonts ) return err;


	for ( n=0; n < fonts->sysFontMax; n++ ) {
		uint32_t checkBit = (1<<n);

		if ( fonts->openState & checkBit ) {
			ret = cellFontCloseFont( &fonts->SystemFont[n] );
			if ( ret != CELL_OK ) {
//				Fonts_PrintError( "    Fonts.SystemFont:cellFontCloseFont=", ret );
				err = ret;
				continue;
			}
			fonts->openState &= (~checkBit);
		}
	}

	for ( n=0; n < fonts->userFontMax; n++ ) {
		uint32_t checkBit = (1<<(FONT_USER_FONT0+n));

		if ( fonts->openState & checkBit ) {
			ret = cellFontCloseFont( &fonts->UserFont[n] );
			if ( ret != CELL_OK ) {
//				Fonts_PrintError( "    Fonts.UserFont:cellFontCloseFont=", ret );
				err = ret;
				continue;
			}
			fonts->openState &= (~checkBit);
		}
	}

	return err;
}


int Fonts_DestroyRenderer( CellFontRenderer* renderer )
{
	int ret = cellFontDestroyRenderer( renderer );

	return ret;
}


int Fonts_EndLibrary( const CellFontLibrary* lib )
{
	int ret = cellFontEndLibrary( lib );

	return ret;
}


int Fonts_End()
{
	int ret = cellFontEnd();

	return ret;
}


