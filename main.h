// ---------------------------------------------------------------
// AT3 Replay test

// ---------------------------------------------------------------
// Includes
#include "AT3.h"

// ---------------------------------------------------------------
// Global behaviour constants
#define DISP_LIST_SIZE 262144
#define PAD_TYPE PSP_CTRL_MODE_DIGITAL			// Consume less power than with the analog pad turned on
#define PIXEL_FORMAT GU_PSM_5551
//#define EXIT_TO_XMB_IN_GTA 					// Test mode when running from GTA *ONLY*
//#define BOOST_333

//#define ALLOW_SCREENSHOT						// To be used with the TRIANGLE button
//#define SCREENSHOT_FILENAME "ms0:/screenshot.tga"

#define NUM_VERTICES 100

// ---------------------------------------------------------------
// Structures
struct Vertex {
	float x;
	float y;
	float z;
};

// ---------------------------------------------------------------
// Variables
char Disp_List[DISP_LIST_SIZE] __attribute__((aligned(16)));
struct Vertex Lines_Left[NUM_VERTICES] __attribute__((aligned(16)));
struct Vertex Lines_Right[NUM_VERTICES] __attribute__((aligned(16)));

// ---------------------------------------------------------------
// Functions

// ---------------------------------------------------------------
// External datas
#ifndef EMBED_DATAS
	#define FILENAME "ms0:/PSP/GAME/__SCE__AT3Replay/Dafunk - Background Tune.at3"
	#define FILENAME_LENGTH 2859336
	unsigned char Tune[FILENAME_LENGTH] __attribute__((aligned(16)));
#else
	extern BYTE *RES_MUSIC;
	extern DWORD RES_MUSIC_LEN;
#endif

// ---------------------------------------------------------------
// Functions triggered before & after taking a screenshot
#ifdef ALLOW_SCREENSHOT
int Before_ScreenShot(void) {

	return(PIXEL_FORMAT);
}
void After_ScreenShot(void) {

}
#endif

// -------------------------------------------------------
// This routine is triggered right before sending the audio buffer to the audio hardware
void Mixing_CallBack(short *Audio_Buffer, int Buffer_Size) {

}

// ---------------------------------------------------------------
// Initialize context
int Initialize(void) {

	sceGuStart(GU_DIRECT, Disp_List);

		sceGuDrawBuffer(PIXEL_FORMAT, VRAM_DRAW_BUFFER, VRAM_SCANLINES);
		sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, VRAM_DISPLAY_BUFFER, VRAM_SCANLINES);
		sceGuDepthBuffer(VRAM_DEPTH_BUFFER, VRAM_SCANLINES);

		sceGuOffset(VRAM_OFFSET_X, VRAM_OFFSET_Y);
		sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
		sceGuDepthRange(65535, 0);
		sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		sceGuEnable(GU_SCISSOR_TEST);

		sceGuDepthFunc(GU_GEQUAL);
		sceGuEnable(GU_DEPTH_TEST);
		sceGuFrontFace(GU_CW);

		sceGuShadeModel(GU_SMOOTH);
		sceGuEnable(GU_CULL_FACE);
		sceGuEnable(GU_CLIP_PLANES);

		sceGuDisable(GU_TEXTURE_2D);

		sceGuClearColor(0);
		sceGuClearDepth(0);
		sceGuClearStencil(0);

	sceGuFinish();

	// -------------------------------------------------------
#ifndef EMBED_DATAS
	SceUID handle = sceIoOpen(FILENAME, PSP_O_RDONLY, 0);
	sceIoRead(handle, Tune, FILENAME_LENGTH);
	sceIoClose(handle);
	// File / Size / Loop on/off / Optional audio callback
	if(AT3_Init(Tune, FILENAME_LENGTH, 1, &Mixing_CallBack)) AT3_Play(1);
#else
	// File / Size / Loop on/off / Optional audio callback
	if(AT3_Init(&RES_MUSIC, RES_MUSIC_LEN, 1, &Mixing_CallBack)) AT3_Play(1);
#endif
	// -------------------------------------------------------

	return(TRUE);
}

// ---------------------------------------------------------------
// Free alloacted resources
void UnInitialize(void) {

	// -------------------------------------------------------
	AT3_Stop();
	// -------------------------------------------------------
}

// ---------------------------------------------------------------
// Main loop
void Draw(void) {
	short *Mixing_Buffer;
	int i;

	sceGuStart(GU_DIRECT, Disp_List);
		sceGuClear(GU_COLOR_BUFFER_BIT);
	
		// Display 2 oscilloscopes
		Mixing_Buffer = AT3_Get_Mixing_Buffer();
	
		for(i = 0; i < NUM_VERTICES; i++) {
			Lines_Left[i].x = (i * ((SCREEN_WIDTH / 2) / NUM_VERTICES)) + 30;
			Lines_Left[i].y = ((Mixing_Buffer[(i * 2)] + 32767) * 0.0015f) + 80.0f;
		}
		for(i = 0; i < NUM_VERTICES; i++) {
			Lines_Right[i].x = i * ((SCREEN_WIDTH / 2) / NUM_VERTICES) + 250;
			Lines_Right[i].y = ((Mixing_Buffer[(i * 2) + 1] + 32767) * 0.0015f) + 80.0f;
		}
		sceGuColor(0xffffff44);
		
		sceGuDrawArray(GU_LINE_STRIP, GU_VERTEX_32BITF | GU_TRANSFORM_2D, NUM_VERTICES - 1, 0, (void *) Lines_Left + 0x40000000);
		sceGuDrawArray(GU_LINE_STRIP, GU_VERTEX_32BITF | GU_TRANSFORM_2D, NUM_VERTICES - 1, 0, (void *) Lines_Right + 0x40000000);

	sceGuFinish();
}
