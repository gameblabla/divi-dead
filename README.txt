Here's a Divi-Dead interpreter by SoyWiz.

Copyright status is unknown, except for smpeg which is under the LGPL.
This was the thread he created when he posted the source code :
https://foro.tales-tra.com/viewtopic.php?f=73&t=1647

The only thing he said is that he will no give any support for the source code.

I revamped the source code a bit because it was a mess and it was trying to put everything
inside of a main.c with includes and the likes...

So yeah this should be a lot more straight forward to compile & run.

Note that i removed the tools for translating and yadada...
This is only focused on the game engine part and getting the game up and running, nothing else.

=========
Building
=========

You will need PHP (with the GD module), Wine and ffmpeg.
Make sure to compile bin2c in Utils/bin2c too.

Run compress_graphics.bat with wine (you can do so with wineconsole).
Then run ./build_graphics.sh.

Finally you can compile the resulting executable by invoking make -j.

