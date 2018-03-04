# MPEG.o MPEGfilter.o MPEGlist.o MPEGring.o MPEGstream.o MPEGsystem.o

rm *.o
sh-elf-g++ -c \
	MPEGsystem.cpp MPEGstream.cpp MPEG.cpp MPEGfilter.c MPEGlist.cpp MPEGring.cpp \
	audio/mpegtoraw.cpp audio/mpegtable.cpp audio/mpeglayer3.cpp audio/mpeglayer2.cpp audio/mpeglayer1.cpp audio/MPEGaudio.cpp audio/hufftable.cpp audio/filter.cpp audio/filter_2.cpp audio/bitwindow.cpp \
	video/video.cpp video/vhar128.cpp video/util.cpp video/readfile.cpp video/parseblock.cpp video/MPEGvideo.cpp video/motionvec.cpp video/jrevdct.cpp video/gdith.cpp video/decoders.cpp video/floatdct.cpp \
	smpeg.cpp \
	-O2 -DFRAME_POINTERS -ml -m4-single-only -fno-optimize-sibling-calls -D_arch_dreamcast -D_arch_sub_pristine -Wall -g -fno-builtin -fno-strict-aliasing -ml -m4-single-only -Wl -nostartfiles -nostdlib \
	-I"/usr/local/dc/kos/kos/include" \
	-I"/usr/local/dc/kos/kos-ports/include" -I"/usr/local/dc/kos/kos-ports/include/SDL" -I.
#

sh-elf-ar rcs libsmpeg-dc.a *.o