#include <cell/error.h>
#include <cell/sysmodule.h>
#include <sysutil/sysutil_sysparam.h>

#include <stdio.h>
#include <string.h>
#include <fastmath.h>

#include <sys/timer.h>
#include <cell/cell_fs.h>

#include <cell/gcm.h>

#include <cell/control_console.h>

#include <sysutil/sysutil_sysparam.h>
#include <netex/libnetctl.h>

#include "graphics.h"

#define ROUNDUP(x, a) (((x)+((a)-1))&(~((a)-1)))

CellDbgFontConsoleId consoleID = CELL_DBGFONT_STDOUT_ID;

extern u32 frame_index;
extern int V_WIDTH;
extern int V_HEIGHT;
extern float overscan;
extern int cover_mode;
extern int bounce;

extern int xmb_slide_y;
extern bool is_remoteplay;
extern u8 video_mode;
extern int date_format;

extern bool th_device_list;
extern bool th_device_separator;
extern u16 th_device_separator_y;
extern bool th_legend;
extern u16 th_legend_y;
extern bool th_drive_icon;
extern u16 th_drive_icon_x;
extern u16 th_drive_icon_y;
extern bool use_depth;
extern u8 hide_bd;
typedef struct
{
	float x, y, z;
	u32 color;
} vtx_color;

typedef struct {
	float x, y, z;
	float tx, ty;
} vtx_texture;

u32 screen_width;
u32 screen_height;

u32 color_pitch;
u32 depth_pitch;
u32 color_offset[V_BUFFERS];
u32 depth_offset;

extern u32 _binary_vpshader_vpo_start;
extern u32 _binary_vpshader_vpo_end;
extern u32 _binary_fpshader_fpo_start;
extern u32 _binary_fpshader_fpo_end;
extern u32 video_buffer;

static unsigned char *vertex_program_ptr =
(unsigned char *)&_binary_vpshader_vpo_start;
static unsigned char *fragment_program_ptr =
(unsigned char *)&_binary_fpshader_fpo_start;

static CGprogram vertex_program;
static CGprogram fragment_program;

extern struct _CGprogram _binary_vpshader2_vpo_start;
extern struct _CGprogram _binary_fpshader2_fpo_start;

extern void *color_base_addr;

extern char *tmhour(int _hour);
extern void put_label(uint8_t *buffer, uint32_t width, uint32_t height, char *str1p, char *str2p, char *str3p, uint32_t color);
extern void put_texture( uint8_t *buffer_to, uint8_t *buffer_from, uint32_t width, uint32_t height, int from_width, int x, int y, int border, uint32_t border_color);
extern void print_label(float x, float y, float scale, uint32_t color, char *str1p);
extern void print_label_ex(float x, float y, float scale, uint32_t color, char *str1p, float weight, float slant, int font, float hscale, float vscale, int centered);
extern void flush_ttf(uint8_t *buffer, uint32_t _V_WIDTH, uint32_t _V_HEIGHT);
extern void draw_box( uint8_t *buffer_to, uint32_t width, uint32_t height, int x, int y, uint32_t border_color);
extern void put_texture_Galpha( uint8_t *buffer_to, uint32_t Twidth, uint32_t Theight, uint8_t *buffer_from, uint32_t _width, uint32_t _height, int from_width, int x, int y, int border, uint32_t border_color);
extern void put_texture_with_alpha( uint8_t *buffer_to, uint8_t *buffer_from, uint32_t _width, uint32_t _height, int from_width, int x, int y, int border, uint32_t border_color);
extern int max_ttf_label;
extern u8 *text_bmp;
extern u8 *text_USB;
extern u8 *text_HDD;
extern u8 *text_BLU_1;
extern u8 *text_legend;
extern u8 *text_DEVS;
extern u8 browse_column_active;
extern u8 *text_FONT;
extern int legend_y, legend_h;
extern int last_selected;
extern int b_box_opaq;
extern int b_box_step;
extern int draw_legend;

static void *vertex_program_ucode;
static void *fragment_program_ucode;
static u32 fragment_offset;

static void *text_vertex_prg_ucode;
static void *text_fragment_prg_ucode;
static u32 text_fragment_offset;

static u32 vertex_offset[2];
static u32 color_index;
static u32 position_index;

static vtx_color *vertex_color;
extern int vert_indx;
extern int vert_texture_indx;

static vtx_texture *vertex_text;
static u32 vertex_text_offset;

static u32 text_obj_coord_indx;
static u32 text_tex_coord_indx;

static CGresource tindex;
static CGprogram vertex_prg;
static CGprogram fragment_prg;
static CellGcmTexture text_param;

static u32 local_heap = 0;

static void *localAlloc(const u32 size)
{
	u32 align = (size + 1023) & (~1023);
	u32 base = local_heap;

	local_heap += align;
	return (void*)base;
}

static void *localAllocAlign(const u32 alignment, const u32 size)
{
	local_heap = (local_heap + alignment-1) & (~(alignment-1));

	return (void*)localAlloc(size);
}


void setRenderTarget(u8 index)
{

	main_surface[index].colorFormat 	 = CELL_GCM_SURFACE_A8R8G8B8;
	main_surface[index].colorTarget		 = CELL_GCM_SURFACE_TARGET_0;
	main_surface[index].colorLocation[0] = CELL_GCM_LOCATION_LOCAL;
	main_surface[index].colorOffset[0] 	 = color_offset[index];
	main_surface[index].colorPitch[0] 	 = color_pitch;

	main_surface[index].colorLocation[1] = CELL_GCM_LOCATION_LOCAL;
	main_surface[index].colorLocation[2] = CELL_GCM_LOCATION_LOCAL;
	main_surface[index].colorLocation[3] = CELL_GCM_LOCATION_LOCAL;

	main_surface[index].colorOffset[1] 	 = 0;
	main_surface[index].colorOffset[2] 	 = 0;
	main_surface[index].colorOffset[3] 	 = 0;
	main_surface[index].colorPitch[1]	 = 64;
	main_surface[index].colorPitch[2]	 = 64;
	main_surface[index].colorPitch[3]	 = 64;

	main_surface[index].depthFormat 	 = CELL_GCM_SURFACE_Z24S8;
	main_surface[index].depthLocation	 = CELL_GCM_LOCATION_LOCAL;
	main_surface[index].depthOffset	     = depth_offset;
	main_surface[index].depthPitch 	     = depth_pitch;

	main_surface[index].type		     = CELL_GCM_SURFACE_PITCH;
	main_surface[index].antialias	     = CELL_GCM_SURFACE_CENTER_1;//CELL_GCM_SURFACE_SQUARE_ROTATED_4;//

	main_surface[index].width 		     = screen_width;
	main_surface[index].height 	 	     = screen_height;
	main_surface[index].x 		         = 0;
	main_surface[index].y 		         = 0;

}



void initShader(void)
{
	vertex_program   = (CGprogram)vertex_program_ptr;
	fragment_program = (CGprogram)fragment_program_ptr;

	cellGcmCgInitProgram(vertex_program);
	cellGcmCgInitProgram(fragment_program);

	u32 ucode_size;
	void *ucode;
	cellGcmCgGetUCode(fragment_program, &ucode, &ucode_size);

	void *ret = localAllocAlign(64, ucode_size);
	fragment_program_ucode = ret;
	memcpy(fragment_program_ucode, ucode, ucode_size);

	cellGcmCgGetUCode(vertex_program, &ucode, &ucode_size);
	vertex_program_ucode = ucode;
}


int text_create();

int initDisplay(void)
{
	int ret, i;
	u32 color_size, depth_size, color_depth= 4, z_depth= 4;

	color_base_addr=NULL;
	void *depth_base_addr, *color_addr[V_BUFFERS];

	CellVideoOutResolution resolution;
	CellVideoOutState videoState;

	ret = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
	if (ret != CELL_OK)	return -1;

	u8 output3d=0;
	CellVideoOutConfiguration videocfg;

	if (cellVideoOutGetResolutionAvailability(CELL_VIDEO_OUT_PRIMARY, CELL_VIDEO_OUT_RESOLUTION_720_3D_FRAME_PACKING, CELL_VIDEO_OUT_ASPECT_AUTO, 0) == 0)
	{
		//3d not available
		output3d=0;
	}
	else
	{
		//3d available but switch to 1920x1080
		output3d=1;

		if(videoState.displayMode.resolutionId>0x80) // if currently in 3d mode switch to 1080p or 720p
		{
			memset(&videocfg, 0, sizeof(CellVideoOutConfiguration));
			videocfg.resolutionId = CELL_VIDEO_OUT_RESOLUTION_1080;
			videocfg.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
			videocfg.pitch = 1920*4;
			videocfg.aspect = CELL_VIDEO_OUT_ASPECT_16_9;
			if(cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videocfg, NULL, 0)!=0)
			{
				memset(&videocfg, 0, sizeof(CellVideoOutConfiguration));
				videocfg.resolutionId = CELL_VIDEO_OUT_RESOLUTION_720;
				videocfg.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
				videocfg.pitch = 1280*4;
				videocfg.aspect = CELL_VIDEO_OUT_ASPECT_16_9;
				cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videocfg, NULL, 0);
			}

			cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
		}
	}

	is_remoteplay = ( videoState.displayMode.conversion == CELL_VIDEO_OUT_DISPLAY_CONVERSION_TO_REMOTEPLAY );

	cellVideoOutGetResolution(videoState.displayMode.resolutionId, &resolution);

	screen_width = resolution.width;
	screen_height = resolution.height;
	V_WIDTH  = screen_width;
	V_HEIGHT = screen_height;

	color_pitch = screen_width*color_depth;
	depth_pitch = screen_width*z_depth;
	color_size  = color_pitch*screen_height;
	depth_size  = depth_pitch*screen_height;

	memset(&videocfg, 0, sizeof(CellVideoOutConfiguration));
	videocfg.resolutionId = videoState.displayMode.resolutionId;
	videocfg.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
	videocfg.pitch = color_pitch;
	videocfg.aspect = CELL_VIDEO_OUT_ASPECT_AUTO;

	ret = cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videocfg, NULL, 0);
	if (ret != CELL_OK) return -1;

//if(cover_mode==8 || !video_mode)
	cellGcmSetFlipMode(CELL_GCM_DISPLAY_VSYNC);
//else
//	cellGcmSetFlipMode(CELL_GCM_DISPLAY_HSYNC);
//	cellGcmSetFlipMode(CELL_GCM_DISPLAY_HSYNC_WITH_NOISE);

	CellGcmConfig config;
	cellGcmGetConfiguration(&config);

	local_heap = (u32) config.localAddress;

	color_base_addr = localAllocAlign(16, V_BUFFERS * color_size);
	video_buffer=color_size;

	for (i = 0; i < V_BUFFERS; i++)
		{
		color_addr[i]= (void *)((u32)color_base_addr+ (i*color_size));
		ret = cellGcmAddressToOffset(color_addr[i], &color_offset[i]);
		if(ret != CELL_OK) return -1;
		ret = cellGcmSetDisplayBuffer(i, color_offset[i], color_pitch, screen_width, screen_height);
		if(ret != CELL_OK) return -1;
		}

	depth_base_addr = localAllocAlign(16, depth_size);
	ret = cellGcmAddressToOffset(depth_base_addr, &depth_offset);
	if(ret != CELL_OK) return -1;

	cellGcmSetZcull(0, depth_offset,
					ROUNDUP(screen_width, 64), ROUNDUP(screen_height, 64), 0,
					CELL_GCM_ZCULL_Z24S8, CELL_GCM_SURFACE_CENTER_1,
					CELL_GCM_ZCULL_LESS, CELL_GCM_ZCULL_LONES,
					CELL_GCM_ZCULL_LESS, 0x80, 0xff);

	text_create();

	return 0;
}

void setDrawEnv(void)
{
	u16 x,y,w,h;
	float min, max;
	float scale[4],offset[4];

	w = (u16)((float)screen_width*(1.f-overscan/2.f));
	h = (u16)((float)screen_height*(1.f-overscan/2.f));

	x = (u16)((float)screen_width*overscan/2.f);
	y = 0;

	min = 0.0f;
	max = 1.0f;
	scale[0] = w * 0.5f * (1.f-overscan/2.f);
	scale[1] = h * -0.5f * (1.f-overscan/2.f);
	scale[2] = (max - min) * 0.5f;
	scale[3] = 0.0f;
	offset[0] = x + scale[0];
	offset[1] = h - y + scale[1];
	offset[2] = (max + min) * 0.5f;
	offset[3] = 0.0f;

	cellGcmSetSurface(gCellGcmCurrentContext, &main_surface[frame_index]);

	cellGcmSetClearSurface(gCellGcmCurrentContext, CELL_GCM_CLEAR_Z | CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G |	CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A);

	cellGcmSetColorMask(gCellGcmCurrentContext, CELL_GCM_COLOR_MASK_B | CELL_GCM_COLOR_MASK_G | CELL_GCM_COLOR_MASK_R | CELL_GCM_COLOR_MASK_A);
	cellGcmSetColorMaskMrt(gCellGcmCurrentContext, 0); //CELL_GCM_COLOR_MASK_B | CELL_GCM_COLOR_MASK_G | CELL_GCM_COLOR_MASK_R | CELL_GCM_COLOR_MASK_A

	cellGcmSetViewport(gCellGcmCurrentContext, x, y, w, h, min, max, scale, offset);
	cellGcmSetClearColor(gCellGcmCurrentContext, 0xff000000);

	cellGcmSetDepthTestEnable(gCellGcmCurrentContext, CELL_GCM_TRUE);
	cellGcmSetDepthFunc(gCellGcmCurrentContext, CELL_GCM_LESS);

	cellGcmSetBlendFunc(gCellGcmCurrentContext,CELL_GCM_SRC_ALPHA, CELL_GCM_ONE_MINUS_SRC_ALPHA,CELL_GCM_SRC_ALPHA, CELL_GCM_ONE_MINUS_SRC_ALPHA);
	cellGcmSetBlendEquation(gCellGcmCurrentContext,CELL_GCM_FUNC_ADD, CELL_GCM_FUNC_ADD);
	cellGcmSetBlendEnable(gCellGcmCurrentContext,CELL_GCM_TRUE);

	cellGcmSetShadeMode(gCellGcmCurrentContext, CELL_GCM_SMOOTH);

	cellGcmSetAlphaFunc(gCellGcmCurrentContext, CELL_GCM_GREATER, 0x01);

	cellGcmSetAntiAliasingControl(gCellGcmCurrentContext, CELL_GCM_TRUE, CELL_GCM_FALSE, CELL_GCM_FALSE, 0xffff);

}

void setRenderColor(void)
{
	cellGcmSetVertexProgram(gCellGcmCurrentContext, vertex_program, vertex_program_ucode);
	cellGcmSetInvalidateTextureCache( gCellGcmCurrentContext, CELL_GCM_INVALIDATE_VERTEX_TEXTURE );
	cellGcmSetVertexDataArray(gCellGcmCurrentContext, position_index, 0, sizeof(vtx_color), 3, CELL_GCM_VERTEX_F, CELL_GCM_LOCATION_LOCAL, (u32)vertex_offset[0]);
	cellGcmSetVertexDataArray(gCellGcmCurrentContext, color_index, 0, sizeof(vtx_color), 4,	CELL_GCM_VERTEX_UB, CELL_GCM_LOCATION_LOCAL, (u32)vertex_offset[1]);

	cellGcmSetFragmentProgram(gCellGcmCurrentContext, fragment_program, fragment_offset);

}

int setRenderObject(void)
{

	vertex_color = (vtx_color*) localAllocAlign(128, 16384*sizeof(vtx_color)); // 384 quad polygons/textures (384x4 vertices)

	CGparameter position = cellGcmCgGetNamedParameter(vertex_program, "position");
	CGparameter color = cellGcmCgGetNamedParameter(vertex_program, "color");

	position_index = cellGcmCgGetParameterResource(vertex_program, position) - CG_ATTR0;
	color_index = cellGcmCgGetParameterResource(vertex_program, color) - CG_ATTR0;


	if(cellGcmAddressToOffset(fragment_program_ucode, &fragment_offset) != CELL_OK) return -1;

	if (cellGcmAddressToOffset(&vertex_color->x, &vertex_offset[0]) != CELL_OK)	return -1;
	if (cellGcmAddressToOffset(&vertex_color->color, &vertex_offset[1]) != CELL_OK)	return -1;

	return 0;
}

int initFont()
{
	CellDbgFontConfigGcm config;

	int size = CELL_DBGFONT_FRAGMENT_SIZE + CELL_DBGFONT_VERTEX_SIZE * CONSOLE_WIDTH * CONSOLE_HEIGHT + CELL_DBGFONT_TEXTURE_SIZE;

	int ret = 0;

	void*localmem = localAllocAlign(128, size);
	if( localmem == NULL ) return -1;

	memset(&config, 0, sizeof(CellDbgFontConfigGcm));

	config.localBufAddr = (sys_addr_t)localmem;
	config.localBufSize = size;
	config.mainBufAddr = NULL;
	config.mainBufSize  = 0;
	config.option = CELL_DBGFONT_VERTEX_LOCAL;
	config.option |= CELL_DBGFONT_TEXTURE_LOCAL;
	config.option |= CELL_DBGFONT_SYNC_ON;
	config.option |= CELL_DBGFONT_MAGFILTER_LINEAR | CELL_DBGFONT_MINFILTER_LINEAR;

	ret = cellDbgFontInitGcm(&config);
	if(ret < 0) return ret;

	return 0;
}

int initConsole()
{
	CellDbgFontConsoleConfig config;
	config.posLeft     = 0.086f;
	config.posTop      = 0.16f;
	config.cnsWidth    = CONSOLE_WIDTH;
	config.cnsHeight   = CONSOLE_HEIGHT;
	config.scale       = 0.72f;
	config.color       = 0xffA0A0A0;
	consoleID = cellDbgFontConsoleOpen(&config);

	if (consoleID < 0) return -1;

	return 0;
}

int termConsole()
{
	int ret;
	ret = cellDbgFontConsoleClose(consoleID);

	if(ret) return -1;

	consoleID = CELL_DBGFONT_STDOUT_ID;

	return ret;
}

int termFont()
{
	int ret;

	ret = cellDbgFontExitGcm();

	if(ret) return -1;

	return ret;
}


void DPrintf( const char *string, ... )
{
	va_list argp;

	va_start(argp, string);
	if(consoleID != CELL_DBGFONT_STDOUT_ID)
		cellDbgFontConsoleVprintf(consoleID, string, argp);
	va_end(argp);
}

void draw_text_stroke(float x, float y, float size, u32 color, const char *str)
{
		cellDbgFontPrintf( x-.0015f, y-0.0015, size+0.0030f, 0xE0101010, str);
		cellDbgFontPrintf( x-.0015f, y+0.0015, size+0.0030f, 0xD0101010, str);
		cellDbgFontPrintf( x, y, size, color, str);
}


static void init_text_shader( void )
{

	void *ucode;
	u32 ucode_size;

	vertex_prg = &_binary_vpshader2_vpo_start;
	fragment_prg = &_binary_fpshader2_fpo_start;

	cellGcmCgInitProgram( vertex_prg );
	cellGcmCgInitProgram( fragment_prg );

	cellGcmCgGetUCode( fragment_prg, &ucode, &ucode_size );

	text_fragment_prg_ucode = localAllocAlign(64, ucode_size );

	cellGcmAddressToOffset( text_fragment_prg_ucode, &text_fragment_offset );

	memcpy( text_fragment_prg_ucode, ucode, ucode_size );

	cellGcmCgGetUCode( vertex_prg, &text_vertex_prg_ucode, &ucode_size );

}


int text_create()
{
	init_text_shader();

	vertex_text = (vtx_texture*) localAllocAlign(128, 16384*sizeof(vtx_texture));

	cellGcmAddressToOffset( (void*)vertex_text,
							&vertex_text_offset );

	text_param.format  = CELL_GCM_TEXTURE_A8R8G8B8;
	text_param.format |= CELL_GCM_TEXTURE_LN;

	text_param.remap = CELL_GCM_TEXTURE_REMAP_REMAP << 14 | CELL_GCM_TEXTURE_REMAP_REMAP << 12 | CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
		CELL_GCM_TEXTURE_REMAP_REMAP <<  8 | CELL_GCM_TEXTURE_REMAP_FROM_G << 6 | CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
		CELL_GCM_TEXTURE_REMAP_FROM_A << 2 | CELL_GCM_TEXTURE_REMAP_FROM_B;

	text_param.mipmap = 1;
	text_param.cubemap = CELL_GCM_FALSE;
	text_param.dimension = CELL_GCM_TEXTURE_DIMENSION_2;

	CGparameter objCoord = cellGcmCgGetNamedParameter( vertex_prg, "a2v.objCoord" );
	if( objCoord == 0 ) return -1;

	CGparameter texCoord = cellGcmCgGetNamedParameter( vertex_prg, "a2v.texCoord" );
	if( texCoord == 0) return -1;

	CGparameter texture = cellGcmCgGetNamedParameter( fragment_prg, "texture" );

	if( texture == 0 ) return -1;

	text_obj_coord_indx = cellGcmCgGetParameterResource( vertex_prg, objCoord) - CG_ATTR0;
	text_tex_coord_indx = cellGcmCgGetParameterResource( vertex_prg, texCoord) - CG_ATTR0;
	tindex = (CGresource) (cellGcmCgGetParameterResource( fragment_prg, texture ) - CG_TEXUNIT0 );

	return 0;

}

int set_texture(u8 *buffer, u32 x_size, u32 y_size )
{

	int ret;
	u32 buf_offs;

	ret = cellGcmAddressToOffset( buffer, &buf_offs );
	if( CELL_OK != ret ) return ret;

	text_param.depth  = 1;
	text_param.width  = x_size;
	text_param.height = y_size;
	text_param.pitch  = x_size*4;
	text_param.offset = buf_offs;
	text_param.location = CELL_GCM_LOCATION_MAIN;


	cellGcmSetTexture( gCellGcmCurrentContext, tindex, &text_param );

	cellGcmSetTextureControl( gCellGcmCurrentContext, tindex, 1, 0, 15, CELL_GCM_TEXTURE_MAX_ANISO_1 );

	cellGcmSetTextureAddress( gCellGcmCurrentContext, tindex, CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_CLAMP_TO_EDGE,
		CELL_GCM_TEXTURE_CLAMP_TO_EDGE, CELL_GCM_TEXTURE_UNSIGNED_REMAP_NORMAL, CELL_GCM_TEXTURE_ZFUNC_LESS, 0 );

//	cellGcmSetTextureFilter( gCellGcmCurrentContext, tindex, 0, CELL_GCM_TEXTURE_LINEAR_LINEAR, CELL_GCM_TEXTURE_LINEAR, CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX );
	if(V_WIDTH<1280)
		cellGcmSetTextureFilter( gCellGcmCurrentContext, tindex, 0, CELL_GCM_TEXTURE_CONVOLUTION_MIN, CELL_GCM_TEXTURE_LINEAR, CELL_GCM_TEXTURE_CONVOLUTION_GAUSSIAN );
	else
		cellGcmSetTextureFilter( gCellGcmCurrentContext, tindex, 0, CELL_GCM_TEXTURE_LINEAR_LINEAR, CELL_GCM_TEXTURE_LINEAR, CELL_GCM_TEXTURE_CONVOLUTION_QUINCUNX );

	cellGcmSetVertexProgram( gCellGcmCurrentContext, vertex_prg, text_vertex_prg_ucode );
	cellGcmSetFragmentProgram( gCellGcmCurrentContext, fragment_prg, text_fragment_offset);
	cellGcmSetInvalidateTextureCache( gCellGcmCurrentContext, CELL_GCM_INVALIDATE_TEXTURE );
	cellGcmSetVertexDataArray( gCellGcmCurrentContext, text_obj_coord_indx, 0, sizeof(vtx_texture), 3, CELL_GCM_VERTEX_F,
							   CELL_GCM_LOCATION_LOCAL, vertex_text_offset );
	cellGcmSetVertexDataArray( gCellGcmCurrentContext, text_tex_coord_indx, 0, sizeof(vtx_texture), 2, CELL_GCM_VERTEX_F,
	                           CELL_GCM_LOCATION_LOCAL, ( vertex_text_offset + sizeof(float)*3 ) );

	return ret;

}

void display_img_persp(int x, int y, int width, int height, int tx, int ty, float z, int Dtx, int Dty, int keystoneL, int keystoneR)
{
    vertex_text[vert_texture_indx].x= ((float) ((x)*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx].y= ((float) ((y-keystoneL)*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx].z= z;
    vertex_text[vert_texture_indx].tx= 0.0f;
    vertex_text[vert_texture_indx].ty= 0.0f;

    vertex_text[vert_texture_indx+1].x= ((float) ((x+width)*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx+1].y= ((float) ((y-keystoneR)*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx+1].z= z;
    vertex_text[vert_texture_indx+1].tx= ((float) tx)/Dtx;
    vertex_text[vert_texture_indx+1].ty= 0.0f;

    vertex_text[vert_texture_indx+2].x= ((float) ((x+width)*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx+2].y= ((float) ((y+height+keystoneR)*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx+2].z= z;
    vertex_text[vert_texture_indx+2].tx= ((float) tx)/Dtx;
    vertex_text[vert_texture_indx+2].ty=((float) ty)/Dty;

    vertex_text[vert_texture_indx+3].x= ((float) ((x)*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx+3].y= ((float) ((y+height+keystoneL)*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx+3].z= z;
    vertex_text[vert_texture_indx+3].tx= 0.0f;
    vertex_text[vert_texture_indx+3].ty= ((float) ty)/Dty;

    cellGcmSetDrawArrays( gCellGcmCurrentContext, CELL_GCM_PRIMITIVE_QUADS, vert_texture_indx, 4 ); //CELL_GCM_PRIMITIVE_TRIANGLE_STRIP
    vert_texture_indx+=4;
}

void display_img(int x, int y, int width, int height, int tx, int ty, float z, int Dtx, int Dty)
{
    vertex_text[vert_texture_indx].x= ((float) ((x)*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx].y= ((float) ((y)*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx].z= z;
    vertex_text[vert_texture_indx].tx= 0.0f;
    vertex_text[vert_texture_indx].ty= 0.0f;

    vertex_text[vert_texture_indx+1].x= ((float) ((x+width)*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx+1].y= ((float) ((y)*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx+1].z= z;
    vertex_text[vert_texture_indx+1].tx= ((float) tx)/Dtx;
    vertex_text[vert_texture_indx+1].ty= 0.0f;

    vertex_text[vert_texture_indx+2].x= ((float) ((x+width)*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx+2].y= ((float) ((y+height)*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx+2].z= z;
    vertex_text[vert_texture_indx+2].tx= ((float) tx)/Dtx;
    vertex_text[vert_texture_indx+2].ty=((float) ty)/Dty;

    vertex_text[vert_texture_indx+3].x= ((float) ((x)*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx+3].y= ((float) ((y+height)*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx+3].z= z;
    vertex_text[vert_texture_indx+3].tx= 0.0f;
    vertex_text[vert_texture_indx+3].ty= ((float) ty)/Dty;

    cellGcmSetDrawArrays( gCellGcmCurrentContext, CELL_GCM_PRIMITIVE_QUADS, vert_texture_indx, 4 ); //CELL_GCM_PRIMITIVE_TRIANGLE_STRIP
    vert_texture_indx+=4;
}

void display_img_nr(int x, int y, int width, int height, int tx, int ty, float z, int Dtx, int Dty)
{
    vertex_text[vert_texture_indx].x= ((float) ((x)*2))/((float) V_WIDTH)-1.0f;
    vertex_text[vert_texture_indx].y= ((float) ((y)*-2))/((float) V_HEIGHT)+1.0f;
    vertex_text[vert_texture_indx].z= z;
    vertex_text[vert_texture_indx].tx= 0.0f;
    vertex_text[vert_texture_indx].ty= 0.0f;

    vertex_text[vert_texture_indx+1].x= ((float) ((x+width)*2))/((float) V_WIDTH)-1.0f;
    vertex_text[vert_texture_indx+1].y= ((float) ((y)*-2))/((float) V_HEIGHT)+1.0f;
    vertex_text[vert_texture_indx+1].z= z;
    vertex_text[vert_texture_indx+1].tx= ((float) tx)/Dtx;
    vertex_text[vert_texture_indx+1].ty= 0.0f;

    vertex_text[vert_texture_indx+2].x= ((float) ((x)*2))/((float) V_WIDTH)-1.0f;
    vertex_text[vert_texture_indx+2].y= ((float) ((y+height)*-2))/((float) V_HEIGHT)+1.0f;
    vertex_text[vert_texture_indx+2].z= z;
    vertex_text[vert_texture_indx+2].tx= 0.0f;
    vertex_text[vert_texture_indx+2].ty= ((float) ty)/Dty;

    vertex_text[vert_texture_indx+3].x= ((float) ((x+width)*2))/((float) V_WIDTH)-1.0f;
    vertex_text[vert_texture_indx+3].y= ((float) ((y+height)*-2))/((float) V_HEIGHT)+1.0f;
    vertex_text[vert_texture_indx+3].z= z;
    vertex_text[vert_texture_indx+3].tx= ((float) tx)/Dtx;
    vertex_text[vert_texture_indx+3].ty=((float) ty)/Dty;

    cellGcmSetDrawArrays( gCellGcmCurrentContext, CELL_GCM_PRIMITIVE_TRIANGLE_STRIP, vert_texture_indx, 4 ); //CELL_GCM_PRIMITIVE_TRIANGLE_STRIP
    vert_texture_indx+=4;
}

int angle_coord_x(int radius, float __angle)
{
	float _angle=__angle;
	if(_angle>=360.f) _angle= __angle - 360.f;
	if(_angle<0.f) _angle=360.f+__angle;
	return (int) ((float) radius * cos((_angle/360.f * 6.283185307179586476925286766559f)));
}

int angle_coord_y(int radius, float __angle)
{
	float _angle=__angle;
	if(_angle>=360.f) _angle= __angle - 360.f;
	if(_angle<0.f) _angle=360.f+__angle;
	return (int) ((float) radius * sin((_angle/360.f * 6.283185307179586476925286766559f)));
}

void display_img_angle(int x, int y, int width, int height, int tx, int ty, float z, int Dtx, int Dty, float _angle)
{
	int _radius;
	_radius = (int)(((float)width*sqrt(2.f))/2.f); // diagonal/2 -> works for square textures at the moment

	// center of rotation
	int xC= x+width/2;
	int yC= y+height/2;

    vertex_text[vert_texture_indx].x= ((float) ((xC+angle_coord_x(_radius, _angle))*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx].y= ((float) ((yC+angle_coord_y(_radius, _angle))*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx].z= z;
    vertex_text[vert_texture_indx].tx= 0.0f;
    vertex_text[vert_texture_indx].ty= 0.0f;

    vertex_text[vert_texture_indx+1].x= ((float) ((xC+angle_coord_x(_radius, _angle+90.f))*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx+1].y= ((float) ((yC+angle_coord_y(_radius, _angle+90.f))*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx+1].z= z;
    vertex_text[vert_texture_indx+1].tx= ((float) tx)/Dtx;
    vertex_text[vert_texture_indx+1].ty= 0.0f;

    vertex_text[vert_texture_indx+2].x= ((float) ((xC+angle_coord_x(_radius, _angle-90.f))*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx+2].y= ((float) ((yC+angle_coord_y(_radius, _angle-90.f))*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx+2].z= z;
    vertex_text[vert_texture_indx+2].tx= 0.0f;
    vertex_text[vert_texture_indx+2].ty= ((float) ty)/Dty;

    vertex_text[vert_texture_indx+3].x= ((float) ((xC+angle_coord_x(_radius, _angle-180.f))*2))/((float) 1920)-1.0f;
    vertex_text[vert_texture_indx+3].y= ((float) ((yC+angle_coord_y(_radius, _angle-180.f))*-2))/((float) 1080)+1.0f;
    vertex_text[vert_texture_indx+3].z= z;
    vertex_text[vert_texture_indx+3].tx= ((float) tx)/Dtx;
    vertex_text[vert_texture_indx+3].ty=((float) ty)/Dty;

    cellGcmSetDrawArrays( gCellGcmCurrentContext, CELL_GCM_PRIMITIVE_TRIANGLE_STRIP, vert_texture_indx, 4 ); //CELL_GCM_PRIMITIVE_TRIANGLE_STRIP
    vert_texture_indx+=4;
}

void draw_square(float x, float y, float w, float h, float z, u32 color)
{

	vertex_color[vert_indx].x = x;
	vertex_color[vert_indx].y = y;
	vertex_color[vert_indx].z = z;
	vertex_color[vert_indx].color=color;

	vertex_color[vert_indx+1].x = x+w;
	vertex_color[vert_indx+1].y = y;
	vertex_color[vert_indx+1].z = z;
	vertex_color[vert_indx+1].color=color;

	vertex_color[vert_indx+2].x = x+w;
	vertex_color[vert_indx+2].y = y-h;
	vertex_color[vert_indx+2].z = z;
	vertex_color[vert_indx+2].color=color;

	vertex_color[vert_indx+3].x = x;
	vertex_color[vert_indx+3].y = y-h;
	vertex_color[vert_indx+3].z = z;
	vertex_color[vert_indx+3].color=color;

	cellGcmSetDrawArrays( gCellGcmCurrentContext, CELL_GCM_PRIMITIVE_QUADS, vert_indx, 4);
	vert_indx+=4;

}

