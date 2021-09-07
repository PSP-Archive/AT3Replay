// -------------------------------------------------------
// AT3 replayer v0.1
// Written by hitchhikr/Neural

// -------------------------------------------------------
// Constants
#define AT3_CURRENT_BUFFER 6
#define AT3_POS_INPUT_BUFFER 7
#define AT3_TEMPORARY_BUFFER 8
#define AT3_INITIAL_BUFFER 59
#define AT3_LENGTH_BUFFER 60

#define AT3_SAMPLES 1024
#define AT3_THREAD_PRIORITY 16 - 2

#define AT3_TYPE_ATRAC3 0x1001

// -------------------------------------------------------
// Functions
int AT3_Init(void *File_Mem, int File_Length, int Loop_Flag, void (*Mixing_CallBack)(short *Audio_Buffer, int Buffer_Size));
void AT3_Play(int Play_State);
void AT3_Stop(void);
unsigned long long AT3_Get_Samples_Played(void);
short *AT3_Get_Mixing_Buffer(void);
void AT3_Set_Replay_Volume(int Volume);
