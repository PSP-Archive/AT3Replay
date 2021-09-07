// ---------------------------------------------------------------
// PSP startup code
// Written by hitchhikr / Neural
// Note: doesn't use the standard libraries

// ---------------------------------------------------------------
// Includes
#include <stdio.h>
#include <string.h>
#include <psptypes.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspaudio.h>
#include <pspsdk.h>
#include <vfpumath.h>

// ---------------------------------------------------------------
// Header
PSP_MODULE_INFO("NRL", PSP_MODULE_KERNEL, 1, 1);

// ---------------------------------------------------------------
// Constants
#define TRUE 1
#define FALSE 0
typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef unsigned int BOOL;

#define PSP_KERNEL_HIGHEST_PRIORITY 16
#define PSP_KERNEL_LOWEST_PRIORITY 111
#define PSP_KERNEL_INIT_PRIORITY 32

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272

#define VRAM_SCANLINES 512

#define NOCACHE(x) (x + 0x40000000)
#define VRAM16 (unsigned short *) (NOCACHE(0x04000000))
#define VRAM32 (unsigned int *) (NOCACHE(0x04000000))

#define VRAM_OFFSET_X (2048 - (SCREEN_WIDTH / 2))
#define VRAM_OFFSET_Y (2048 - (SCREEN_HEIGHT / 2))

#define VRAM_SIZE (VRAM_SCANLINES * SCREEN_HEIGHT * 2)
#define VRAM_DRAW_BUFFER (void *) 0
#define VRAM_DISPLAY_BUFFER (void *) VRAM_SIZE
#define VRAM_DEPTH_BUFFER (void *) (VRAM_SIZE * 2)

#define VRAM_ASPECT ((float) SCREEN_WIDTH / (float) SCREEN_HEIGHT)
#define	TORAD(a) ((a) * GU_PI / 180.0f)

// ---------------------------------------------------------------
// Functions
void Display_String(int x, int y, u32 Color, char *String);
int SetupCallbacks(void);
int sceKernelPowerTick(int type);
SceCtrlData Ctrl_Buf;

// ---------------------------------------------------------------
// Main user program
#include "main.h"

// ---------------------------------------------------------------
// Variables
int Looping = FALSE;

// ---------------------------------------------------------------
// Handles home button exit
int Exit_Callback(int Arg1, int Arg2, void *Common) {
	Arg1 = Arg2 = (int) Common;
	Looping = TRUE;
	return(0);
}

int Callback_Thread(void) {
	SceUID cbid = sceKernelCreateCallback("EC", Exit_Callback, NULL);
	if(cbid >= 0) {
		sceKernelRegisterExitCallback(cbid);
		sceKernelSleepThreadCB();
	}
	return(0);
}

void Setup_Callbacks(void) {
	SceUID thid = sceKernelCreateThread("UT", (int (*)(SceSize, void *)) Callback_Thread, PSP_KERNEL_HIGHEST_PRIORITY + 1, 4 * 1024, 0, NULL);
	if(thid >= 0) sceKernelStartThread(thid, 0, 0);
}

// ---------------------------------------------------------------
// Take a screenshot
#ifdef ALLOW_SCREENSHOT
void Write_Byte(int Fd, unsigned char Data) {
    sceIoWrite(Fd, &Data, 1);
}

void Take_ScreenShot(int Screen_Format, char *FileName) {
    const char tgaHeader[] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    const int width = SCREEN_WIDTH;
    const int lineWidth = SCAN_LINE_SIZE;
    const int height = SCREEN_HEIGHT;
    unsigned char lineBuffer[width * 4];
    unsigned short *VRam16 = VRAM16;
    unsigned int *VRam32 = VRAM32;
    int x;
	int y;
	unsigned short Color16;
	unsigned int Color32;
	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
	unsigned char Alpha;

    SceUID fd = sceIoOpen(FileName, PSP_O_CREAT | PSP_O_WRONLY, 0777);
    if(fd < 0) return;
    sceIoWrite(fd, tgaHeader, sizeof(tgaHeader));
    Write_Byte(fd, width & 0xff);
    Write_Byte(fd, width >> 8);
    Write_Byte(fd, height & 0xff);
    Write_Byte(fd, height >> 8);
    Write_Byte(fd, 32);
    Write_Byte(fd, 0);

	Color32 = 0;
	Red = 0;
	Green = 0;
	Blue = 0;
	Alpha = 0;

	for(y = height - 1; y >= 0; y--) {
        for(x = 0; x < width; x++) {
			switch(Screen_Format) {
				case GU_PSM_4444:
					Color16 = VRam16[y * lineWidth + x];
					Red = ((Color16) & 0xf) << 4;
					Green = ((Color16 >> 4) & 0xf) << 4;
					Blue = ((Color16 >> 8) & 0xf) << 4;
					Alpha = ((Color16 >> 12) & 0xf) << 4;
					break;
				case GU_PSM_8888:
					Color32 = VRam32[y * lineWidth + x];
					Red = Color32 & 0xff;
					Green = (Color32 >> 8) & 0xff;
					Blue = (Color32 >> 16) & 0xff;
					Alpha = (Color32 >> 24) & 0xff;
					break;
				case GU_PSM_5551:
					Color16 = VRam16[y * lineWidth + x];
					Red = ((Color16) & 0x1f) << 3;
					Green = ((Color16 >> 5) & 0x1f) << 3;
					Blue = ((Color16 >> 10) & 0x1f) << 3;
					Alpha = ((Color16 >> 15) & 0x1) << 7;
					break;
				case GU_PSM_5650:
					Color16 = VRam16[y * lineWidth + x];
					Red = ((Color16) & 0x1f) << 3;
					Green = ((Color16 >> 5) & 0x3f) << 2;
					Blue = ((Color16 >> 11) & 0x1f) << 3;
					Alpha = 0;
					break;
			}
            lineBuffer[(4 * x)] = Blue;
            lineBuffer[(4 * x) + 1] = Green;
            lineBuffer[(4 * x) + 2] = Red;
            lineBuffer[(4 * x) + 3] = Alpha;
        }
        sceIoWrite(fd, lineBuffer, width * 4);
    }
    sceIoClose(fd);
}
#endif

// ---------------------------------------------------------------
// Wait for the vertical blank interrupt
void Wait_VBlank(void) {
	sceGuSync(0, 0);
	sceDisplayWaitVblankStart();
}

// ---------------------------------------------------------------
// Display a string on the screen
#ifdef ALLOW_DEBUGPRINT
void Display_String(int x, int y, u32 Color, char *string) {
	pspDebugScreenSetXY(x, y);
	pspDebugScreenSetTextColor(Color);
	pspDebugScreenPrintf(string);
}
#endif

// ---------------------------------------------------------------
// Load the audio codec (should be already loaded under gta)
void Load_AudioCodec(void) {
	SceUID PRxId;
	SceKernelLMOption Options;
	SceUID PrxHandle;
	int Start_Result;
	
	PrxHandle = sceIoOpen("flash0:/kd/audiocodec.prx", PSP_O_RDONLY, 0);
	if(PrxHandle >= 0) {
		memset(&Options, 0, sizeof(SceKernelLMOption));
		Options.size = sizeof(SceKernelLMOption);
		Options.mpidtext = 1;
		Options.mpiddata = 1;
		Options.access = 1;
		PRxId = sceKernelLoadModuleByID(PrxHandle, 0, &Options);
		if(PRxId >= 0) sceKernelStartModule(PRxId, 0, NULL, &Start_Result, 0);
		sceIoClose(PrxHandle);
	}
}

// ---------------------------------------------------------------
// Init program
int main(void) {

	sceKernelDcacheWritebackAll();
	// Set pad type
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PAD_TYPE);

	Load_AudioCodec();

#ifdef ALLOW_DEBUGPRINT
	pspDebugScreenInit();
#endif

#ifdef BOOST_333
	// Set processor & bus speed
	scePowerSetClockFrequency(333, 333, 166);
#endif
	Setup_Callbacks();
	sceGuInit();

	if(Initialize()) {
		Wait_VBlank();
		// Turn the screen on now
		sceGuDisplay(GU_TRUE);
		while(!Looping) {
			// Read joypad status
			sceCtrlReadBufferPositive(&Ctrl_Buf, 1);
			if(Ctrl_Buf.Buttons & PSP_CTRL_SELECT) break;
#ifdef ALLOW_SCREENSHOT
			if(Ctrl_Buf.Buttons & PSP_CTRL_TRIANGLE) {
				Take_ScreenShot(Before_ScreenShot(), SCREENSHOT_FILENAME);
				After_ScreenShot();
			}
#endif
			sceKernelDcacheWritebackAll();
			sceKernelPowerTick(6);		// Keep the backlight turned on
			Draw();
			Wait_VBlank();
			sceGuSync(0, 0);
			sceGuSwapBuffers();
		}
	}
	UnInitialize();
	sceGuTerm();

#ifdef EXIT_TO_XMB_IN_GTA
	unsigned char **Quit = (unsigned char **) 0x08803ffc;
	unsigned char *QuitMem = *Quit;	
	*QuitMem++ = 'Q';
	*QuitMem++ = 'U';
	*QuitMem++ = 'I';
	*QuitMem++ = 'T';
#endif

	sceKernelExitGame();
	return(0);
}

// ---------------------------------------------------------------
// Patch the kernel so we can load the audiocodec.prx
void Patcher(SceSize args, void *argp) {
	pspSdkInstallNoDeviceCheckPatch();
	sceKernelExitDeleteThread(0);
}

int _start(void) {
	SceUID thid;
	
	// Create the patcher thread (shouldn't be possible in user level so no patch will occur)
	thid = sceKernelCreateThread("Patcher", (void *) (Patcher + 0x80000000), 0x20, 256 * 1024, 0, 0);
	if(thid >= 0) sceKernelStartThread(thid, 0, NULL);

	thid = sceKernelCreateThread("NRL", (void *) main, PSP_KERNEL_INIT_PRIORITY, 256 * 1024, PSP_THREAD_ATTR_USER | PSP_THREAD_ATTR_VFPU, 0);
	sceKernelStartThread(thid, 0, NULL);
	return(0);
}

static const unsigned int __entrytable[4] __attribute__((section(".rodata.sceResident"))) = {
        0xD632ACDB, 0xF01D73A7, (unsigned int) &_start, (unsigned int) &module_info
};
static const struct _library_entry {
        const char *name;
        unsigned short version;
        unsigned short attribute;
        unsigned char entLen;
        unsigned char varCount;
        unsigned short funcCount;
        void *entrytable;
} _library_entry __attribute__((section(".lib.ent"), used)) = {
        NULL, 0, 0x8000, 4, 1, 1, &__entrytable
};
