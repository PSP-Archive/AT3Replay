	.set noreorder

	.include "pspimport.s"

	IMPORT_START "sceAudiocodec",0x40080011
	IMPORT_FUNC	"sceAudiocodec",0x9d3f790c,sceAudiocodecCheckNeedMem
	IMPORT_FUNC	"sceAudiocodec",0x5b37eb1d,sceAudiocodecInit
	IMPORT_FUNC	"sceAudiocodec",0x70a703f8,sceAudiocodecDecode
	IMPORT_FUNC	"sceAudiocodec",0x3a20a200,sceAudiocodecGetEDRAM
	IMPORT_FUNC	"sceAudiocodec",0x29681260,sceAudiocodecReleaseEDRAM
   