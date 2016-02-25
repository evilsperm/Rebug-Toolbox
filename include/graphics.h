#ifdef __cplusplus
extern "C" {
#endif

#include <cell/gcm.h>
#include <cell/dbgfont.h>

typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;

#define CONSOLE_WIDTH		(76+16)
#define CONSOLE_HEIGHT		(31)

#define DISPLAY_WIDTH  1920
#define DISPLAY_HEIGHT 1080

#define V_BUFFERS 2

extern CellGcmSurface main_surface[2];

void put_vertex(float x, float y, float z, u32 color);
void put_texture_vertex(float x, float y, float z, float tx, float ty);

void draw_square(float x, float y, float w, float h, float z, u32 rgba);

int set_texture( u8 *buffer, u32 x_size, u32 y_size );

void display_img(int x, int y, int width, int height, int tx, int ty, float z, int Dtx, int Dty);
void display_img_angle(int x, int y, int width, int height, int tx, int ty, float z, int Dtx, int Dty, float angle);
void display_img_persp(int x, int y, int width, int height, int tx, int ty, float z, int Dtx, int Dty, int keystoneL, int keystoneR);
void display_img_rotate(int x, int y, int width, int height, int tx, int ty, float z, int Dtx, int Dty, int step);

int initConsole(void);
int initConsole2(float pos_top, int c_lines);
int termConsole(void);
int initFont(void);
int termFont(void);

int initDisplay(void);
int setRenderObject(void);

void setRenderColor(void);
void setRenderTarget(u8 index);
void initShader(void);
void setDrawEnv(void);

void DPrintf( const char *string, ... );

#include <types.h>
#include <sys/synchronization.h>

#ifdef __cplusplus
}
#endif