@echo off
del dividead_win.exe > NUL 2> NUL
bin2c RES\FONT.TTF RES\FONT_TTF.c font_ttf
bin2c RES\FONT2.TTF RES\FONT2_TTF.c font2_ttf
bin2c RES\CLICK.WAV RES\CLICK_WAV.c click_wav
bin2c RES\SPLASH.JPG RES\SPLASH_JPG.c splash_jpg
bin2c ICMP.DAT RES\ICMP_DAT.c ichars_data
bin2c RES\VMU.CHARS RES\VMU_CHARS.c vmu_chars

comp c RES\END\ROLL0.bmp RES\END\ROLL0.bmp.lz > NUL
comp c RES\END\ROLL1.bmp RES\END\ROLL1.bmp.lz > NUL
comp c RES\END\ROLL2.bmp RES\END\ROLL2.bmp.lz > NUL
comp c RES\END\ROLL.txt RES\END\ROLL.txt.lz > NUL

bin2c RES\END\END0.PNG RES\END0_PNG.c end0_png
bin2c RES\END\END1.PNG RES\END1_PNG.c end1_png
bin2c RES\END\ROLL0.bmp.lz RES\ROLL0_PNG.c roll0_png
bin2c RES\END\ROLL1.bmp.lz RES\ROLL1_PNG.c roll1_png
bin2c RES\END\ROLL2.bmp.lz RES\ROLL2_PNG.c roll2_png
bin2c RES\END\ROLL.TXT.lz RES\ROLL_TXT.c roll_txt
REM tcc\tcc.exe -I"SRC" SRC\main.c -I"TCC\include" -L"TCC\lib" -luser32 -l"SDL" -l"SDL_image" -l"SDL_mixer" -l"SDL_ttf" -l"SDL_gfx" -o dividead_win.exe
tcc\tcc.exe -I"SRC" SRC\main.c -I"TCC\include" -L"TCC\lib" -luser32 -lkernel32 -l"SDL" -l"SDL_image" -l"SDL_mixer" -l"SDL_ttf" -l"smpeg" -o dividead_win.exe
set PATH=%PATH%;DLL
dividead_win.exe