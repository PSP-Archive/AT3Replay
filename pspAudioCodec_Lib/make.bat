@echo off
del libpspAudiocodec.a
psp-gcc -O2 -G0 -fmerge-all-constants -ffunction-sections -fdata-sections -c -I ./ -I C:\pspdev\psp\sdk\include pspAudiocodec.s
psp-ar rcs libpspAudiocodec.a *.o
del *.o
