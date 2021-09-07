// -------------------------------------------------------
// AT3 replayer v0.1
// Written by hitchhikr/Neural

// -------------------------------------------------------
// Includes
#include <pspkernel.h>
#include <pspAudio.h>
#include <pspAudioCodec.h>
#include <pspsdk.h>
#include <string.h>
#include <pspaudio.h>
#include <pspaudiolib.h>
#include <pspaudiocodec.h>

#include "AT3.h"

// -------------------------------------------------------
// Variables
SceUID AT3_ThreadID;
SceUID AT3_Channel;
unsigned long AT3_Codec_Buffer[65] __attribute__((aligned(64)));
short AT3_Mix_Buffer[AT3_SAMPLES * 2] __attribute__((aligned(64)));
int AT3_Playing;
int AT3_Loop;
int AT3_Datas_Start;	
int AT3_Volume = PSP_AUDIO_VOLUME_MAX;
unsigned long long AT3_Samples_Played;
void (*AT3_CallBack)(short *Audio_Buffer, int Buffer_Size);

// -------------------------------------------------------
// Set the replay state
void AT3_Play(int Play_State) {
	AT3_Playing = Play_State;
}

// -------------------------------------------------------
// Set the volume of the AT3 replaying
void AT3_Set_Replay_Volume(int Volume) {
	AT3_Volume = Volume;
}

// -------------------------------------------------------
// Return the number of played samples
unsigned long long AT3_Get_Samples_Played(void) {
	return(AT3_Samples_Played);
}

// -------------------------------------------------------
// Return the current mixing buffer
short *AT3_Get_Mixing_Buffer(void) {
	return((short *) ((int) AT3_Mix_Buffer | 0x40000000));
}

// -------------------------------------------------------
// Atrac3 replay thread
SceInt32 AT3_Thread(SceSize args, ScePVoid argp) {
	for(;;) {
		if(AT3_CallBack) AT3_CallBack((short *) ((int) AT3_Mix_Buffer | 0x40000000), AT3_SAMPLES);
		AT3_Samples_Played += sceAudioOutputBlocking(AT3_Channel, AT3_Volume, (void *) ((int) AT3_Mix_Buffer | 0x40000000));
		if(AT3_Playing) {
			sceAudiocodecDecode(AT3_Codec_Buffer, AT3_TYPE_ATRAC3);
			AT3_Codec_Buffer[AT3_CURRENT_BUFFER] += AT3_Codec_Buffer[AT3_POS_INPUT_BUFFER];
			if(AT3_Codec_Buffer[AT3_CURRENT_BUFFER] >= ((AT3_Codec_Buffer[AT3_LENGTH_BUFFER] - AT3_Datas_Start) + AT3_Codec_Buffer[AT3_INITIAL_BUFFER])) {
				if(AT3_Loop) AT3_Codec_Buffer[AT3_CURRENT_BUFFER] = AT3_Codec_Buffer[AT3_INITIAL_BUFFER] + AT3_Datas_Start;
				else AT3_Playing = 0;
			}
		}
		sceKernelDcacheWritebackInvalidateAll();
		sceKernelDelayThread(10);
	}
}

// -------------------------------------------------------
// Initialize the AT3 replay
int AT3_Init(void *File_Mem, int File_Length, int Loop_Flag, void (*Mixing_CallBack)(short *AT3_Audio_Buffer, int AT3_Buffer_Size)) {
	unsigned int *dwFile_Mem;
	unsigned char *tmpFile_Mem;

	AT3_Play(0);
	AT3_Loop = Loop_Flag;
	AT3_CallBack = Mixing_CallBack;

	dwFile_Mem = (unsigned int *) File_Mem;
	if(*dwFile_Mem++ == 'FFIR') {
		dwFile_Mem++;
		if(*dwFile_Mem++ == 'EVAW') {
			tmpFile_Mem = (unsigned char *) (dwFile_Mem + 2);
			while(*dwFile_Mem++ != 'atad') {
				dwFile_Mem = (unsigned int *) (tmpFile_Mem + *dwFile_Mem);
				tmpFile_Mem = (unsigned char *) (dwFile_Mem + 2);
			}
			AT3_Datas_Start = (int) tmpFile_Mem - (int) File_Mem;
			memset(AT3_Codec_Buffer, 0, sizeof(AT3_Codec_Buffer));
			memset(AT3_Mix_Buffer, 0, AT3_SAMPLES * 2 * 2);
		
			AT3_Codec_Buffer[AT3_CURRENT_BUFFER] = (int) (File_Mem + AT3_Datas_Start);
			AT3_Codec_Buffer[AT3_TEMPORARY_BUFFER] = (int) AT3_Mix_Buffer;
			AT3_Codec_Buffer[10] = 4;
			AT3_Codec_Buffer[44] = 2;
			AT3_Codec_Buffer[AT3_INITIAL_BUFFER] = (int) File_Mem;
			AT3_Codec_Buffer[AT3_LENGTH_BUFFER] = File_Length;
		
			if(sceAudiocodecCheckNeedMem(AT3_Codec_Buffer, AT3_TYPE_ATRAC3) < 0) return(0);
			if(sceAudiocodecGetEDRAM(AT3_Codec_Buffer, AT3_TYPE_ATRAC3) < 0) return(0);
			if(sceAudiocodecInit(AT3_Codec_Buffer, AT3_TYPE_ATRAC3) < 0) return(0);
		
			AT3_Channel = sceAudioChReserve(0, AT3_SAMPLES, PSP_AUDIO_FORMAT_STEREO);
			if(AT3_Channel < 0) return(0);
		
			AT3_ThreadID = sceKernelCreateThread("Audio", (void *) AT3_Thread, AT3_THREAD_PRIORITY, (1024 * 4), PSP_THREAD_ATTR_USER, NULL);
			if(AT3_ThreadID < 0) return(0);
		
			sceKernelStartThread(AT3_ThreadID, 0, NULL);
			return(1);
		}
	}
	return(0);
}

// -------------------------------------------------------
// Free the allocated resources
void AT3_Stop(void) {
	AT3_Play(0);
	sceAudioChRelease(AT3_Channel);
	sceAudiocodecReleaseEDRAM(AT3_Codec_Buffer);
}
