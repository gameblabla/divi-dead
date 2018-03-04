@echo off
md RELEASE
del RELEASE\*.7z
REM 7z a RELEASE/dividead_release.7z LANG/* dividead_win.exe dividead_dc.bin EBOOT.PBP FONT.TTF CLICK.WAV DLL/*.DLL
7z a RELEASE/dividead_release.7z LANG/* dividead_win.exe dividead_dc.bin EBOOT.PBP DLL/*.DLL
REM 7z a RELEASE/dividead_ogg.7z OGG/*.OGG
REM 7z a RELEASE/dividead_it.7z IT/*.IT
7z a RELEASE/dividead_dev.7z *.bat *.sh translate.exe comp.exe 7z.exe makefile* RES/PSP/* RES/MAP/* ACME/* ACME/SRC* LANG/*.txt SRC/*.c SRC/*.S UTIL/*.d UTIL/*.c UTIL/*.php UTIL/*.bat