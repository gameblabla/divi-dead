#ifndef RINGREAD_H
#define RINGREAD_H

#include "shared.h"

//#define RING_DEBUG

//#define RING_PREBUFFER

#ifdef RING_test
	#define READING_START()
	#define READING_END()
#endif

#ifdef RING_DEBUG
	//#define RING_debug(format, ...) fprintf (stdout, format, __VA_ARGS__)
	#define RING_debug(...) fprintf (stdout, __VA_ARGS__)
#else
	#define RING_debug(...)
#endif

#define STRING_SEEK(whence) (whence == SEEK_CUR) ? "SEEK_CUR" : ((whence == SEEK_END) ? "SEEK_END" : "SEEK_SET")

typedef struct {
	char *data;
	int length;
	int pos_r;
	int pos_w;
	int pos_end;
	int eof;
	int read_l;
	SDL_mutex *mutex;
	SDL_RWops *rw;
	int thread_end;
	int thread_read;
	SDL_Thread *thread;
} RING;

#define RING_M_Begin() SDL_mutexP(ring->mutex);
#define RING_M_End() SDL_mutexV(ring->mutex);

//#define RING_M_Begin()
//#define RING_M_End()


extern int RING_fill_chunk(RING *ring, int length);
extern void SDL_thread_pass();
extern int RING_fill_thread(void *_ring);
extern int RING_read_chunk(RING *ring, char *ptr, int length);
extern int RING_read(RING *ring, char *ptr, int left);
extern void RING_delete(RING *r);
extern RING *RING_create(SDL_RWops *rw, int length, int read_l);
extern int RING_RW_close(SDL_RWops *rw);
extern int RING_RW_read(SDL_RWops *rw, void *ptr, int size, int maxnum);
extern int RING_RW_write(SDL_RWops *rw, const void *ptr, int size, int maxnum);
extern int RING_RW_seek(SDL_RWops *rw, int offset, int whence);


#endif
