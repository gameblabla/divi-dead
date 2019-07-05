#!/bin/sh
cd RES
ffmpeg -i SPLASH.psd SPLASH.JPG
cd DC/icons
php build.php
cd ../../../
utils/bin2c/bin2c.elf RES/FONT.TTF RES/FONT_TTF.c font_ttf
#utils/bin2c/bin2c.elf RES/FONT2.TTF RES/FONT2_TTF.c font2_ttf
utils/bin2c/bin2c.elf RES/CLICK.WAV RES/CLICK_WAV.c click_wav
utils/bin2c/bin2c.elf RES/SPLASH.JPG RES/SPLASH_JPG.c splash_jpg
utils/bin2c/bin2c.elf RES/ICMP.DAT RES/ICMP_DAT.c ichars_data
utils/bin2c/bin2c.elf RES/VMU.CHARS RES/VMU_CHARS.c vmu_chars
utils/bin2c/bin2c.elf RES/END/END0.png RES/END0_PNG.c end0_png
utils/bin2c/bin2c.elf RES/END/END1.png RES/END1_PNG.c end1_png
utils/bin2c/bin2c.elf RES/END/ROLL0.bmp.lz RES/ROLL0_PNG.c roll0_png
utils/bin2c/bin2c.elf RES/END/ROLL1.bmp.lz RES/ROLL1_PNG.c roll1_png
utils/bin2c/bin2c.elf RES/END/ROLL2.bmp.lz RES/ROLL2_PNG.c roll2_png
utils/bin2c/bin2c.elf RES/END/ROLL.txt.lz RES/ROLL_TXT.c roll_txt
