@echo off
dmc -c complib.c -o
dmd -v1 -O -release comp.d util.d complib.obj
dmd -v1 -O -release absc.d util.d complib.obj
dmd -v1 -O -release repack.d util.d complib.obj
dmd -v1 -O -release unpack.d util.d complib.obj
dmd -v1 -O -release translate.d util.d complib.obj
dmd -v1 -O -release script.d util.d complib.obj
dmd -v1 -O -release graph.d util.d complib.obj
dmd -v1 -O -release image_rip.d util.d complib.obj
del *.obj
del *.map
upx *.exe