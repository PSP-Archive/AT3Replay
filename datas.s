					.global RES_MUSIC
					.global RES_MUSIC_LEN

					.align 8
RES_MUSIC_LEN:		.long	FRES_MUSIC - RES_MUSIC
					.align 8
RES_MUSIC:			.incbin	"__SCE__DEMO/dafunk - background tune.at3"
FRES_MUSIC:
