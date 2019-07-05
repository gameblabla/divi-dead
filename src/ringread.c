#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>
#include "ringread.h"

int RING_fill_chunk(RING *ring, int length) {
	int readed;
	int pos_w = ring->pos_w % ring->length;
	int pos_r = ring->pos_r % ring->length;
	
	if (length > ring->length) length = ring->length;
	if (pos_w + length >= ring->length) length = ring->length - pos_w;
	if (pos_w < pos_r) {
		if (length > pos_r - pos_w) {
			length = pos_r - pos_w;
		}
	}
	if ((pos_r == pos_w) && (ring->pos_w != ring->pos_r)) length = 0;
	
	readed = SDL_RWread(ring->rw, ring->data + pos_w, 1, length);
	ring->pos_w = SDL_RWtell(ring->rw);
	ring->eof = (readed < length);
	
	if (readed) {
		RING_debug("RING_fill_chunk: %d (%d) : R:%d | W_S:%d | W_E:%d | T:%d\n", readed, length, pos_r, pos_w, pos_w + readed, ring->length);
	}
	
	return readed;
}

#ifdef DREAMCAST
#include <kos/thread.h>
void SDL_thread_pass() {
	thd_pass();
}
#else
void SDL_thread_pass() {
	SDL_Delay(1);
}
#endif

int RING_fill_thread(void *_ring) { RING *ring = _ring;
	#ifdef RING_PREBUFFER
	int ring_percent = 50;
	int reading = 0;
	#endif
	for (; !ring->thread_end; SDL_thread_pass()) {
		if (ring->thread_read) {
			#ifndef RING_PREBUFFER
				READING_START();
			#else
			if (!reading) {
				reading = 1;
				READING_START();
			}
			#endif
			
			RING_M_Begin();
				if (ring->thread_read) RING_fill_chunk(ring, ring->read_l);
			RING_M_End();
			
			//

			#ifndef RING_PREBUFFER
				READING_END();
			#else
			if ((ring->pos_w - ring->pos_r) >= ((ring->length * 20) / 100)) {
				if (reading) {
					READING_END();
					reading = 0;
				}
			} else {
				int t1, t2, t3;
				t1 = (ring->pos_w - ring->pos_r);
				t2 = ((ring->length * 20) / 100);
				t3 = ring->length;
				printf("Buffering: %d%%/%d%%\n", (t1 * 100) / t3, (t2 * 100) / t3);
				
				if (ring->pos_w >= ring->pos_end) {
					printf("  End of Stream\n");
					READING_END();
					reading = 0;		
				}
			}
			#endif
		}
		#ifdef RING_PREBUFFER
		else {
			if (reading) {
				printf("Stop Buffering\n");
				READING_END();
				reading = 0;
			}
		}
		#endif
	}

	return 0;
}

int RING_read_chunk(RING *ring, char *ptr, int length) {
	int posr = ring->pos_r % ring->length;
	
	//RING_debug("RING_read_chunk:S: %d : (%d, %d, %d, %d)\n", length, ring->length, ring->pos_r, ring->pos_w, ring->pos_end);
	
	if (length > ring->length) length = ring->length;
	if (length > (ring->pos_w - ring->pos_r)) length = (ring->pos_w - ring->pos_r);
	if (length > ring->length - posr) length = ring->length - posr;

	memcpy(ptr, ring->data + posr, length);
	ring->pos_r += length;
	
	//RING_debug("RING_read_chunk:E: %d\n", length);
	
	return length;
}

int RING_read(RING *ring, char *ptr, int left) {
	int readed = 0;
	
	ring->thread_read = 1;
	
	while (1) {
		int length;
		RING_M_Begin();
			length = RING_read_chunk(ring, ptr, left);
		RING_M_End();
		ptr += length;
		left -= length;
		readed += length;
		
		//printf("Left:%d\n", left);
		
		if (left <= 0) {
			//RING_debug("RING_read (all)\n");
			break;
		}
		
		if (length <= 0) {
			if (ring->eof) {
				//RING_debug("RING_read ####################### EOF #######################\n");
				break;
			}
			SDL_thread_pass();
		}
	}
	
	//RING_debug("RING_read:E: %d\n", readed);
	
	return readed;
}

void RING_delete(RING *r) {
	RING_debug("RING_delete(0x%08X)\n", r);
	r->thread_end = 1;
	SDL_WaitThread(r->thread, NULL);
	SDL_DestroyMutex(r->mutex);
	free(r->data);
	SDL_RWclose(r->rw);	
	free(r);
	RING_debug("RING_delete(0x%08X) END\n", r);
}

#define GET_RING(rw) ((RING *)((rw)->hidden.unknown.data1))
#define PREPARE_RING() RING *ring = GET_RING(rw);

RING *RING_create(SDL_RWops *rw, int length, int read_l) {
	RING_debug("RING_create(0x%08X, %d, %d)\n", rw, length, read_l);
	RING *ring = malloc(sizeof(RING));
	int rw_start;
	
	ring->data = malloc(length);
	ring->length = length;
	ring->rw = rw;
	ring->pos_w = ring->pos_r = SDL_RWtell(rw);
	ring->eof = 0;
	ring->thread_end = 0;
	ring->thread_read = 0;
	if (read_l > length) read_l = length;
	ring->read_l = read_l;
	
	rw_start = SDL_RWtell(rw);
	ring->pos_end = SDL_RWseek(rw, 0, SEEK_END);
	SDL_RWseek(rw, rw_start, SEEK_SET);
	
	ring->mutex = SDL_CreateMutex();
	ring->thread = SDL_CreateThread(RING_fill_thread, ring);
	return ring;
}

int RING_RW_close(SDL_RWops *rw) { PREPARE_RING();
	RING_debug("RING_RW_close()\n");
	RING_delete(ring);
	SDL_FreeRW(rw);
	return 0;
}

int RING_RW_read(SDL_RWops *rw, void *ptr, int size, int maxnum) { PREPARE_RING();
	RING_debug("RING_RW_read(%d)\n", size * maxnum);
	return RING_read(ring, ptr, size * maxnum);
}

int RING_RW_write(SDL_RWops *rw, const void *ptr, int size, int maxnum) {// PREPARE_RING();
	//RING_debug("RING_RW_write()\n");
	return 0;
}

int RING_RW_seek(SDL_RWops *rw, int offset, int whence) { PREPARE_RING();
	RING_debug("RING_RW_seek(%d, %s)\n", offset, STRING_SEEK(whence));
	
	switch (whence) {
		case SEEK_END:
		
		break;
		case SEEK_SET:
		break;
		case SEEK_CUR:
			whence = SEEK_SET;
			offset += ring->pos_r;
		break;
	}
	
	RING_M_Begin();
		ring->thread_read = 0;
		ring->eof = 0;
		ring->pos_w = ring->pos_r = SDL_RWseek(ring->rw, offset, whence);
	RING_M_End();
	
	//printf("ring->eof = %d\n", ring->eof);
	
	return ring->pos_r;
}


#define TEST_RING_RW_PREPARE() SDL_RWops *rw = _rw->hidden.unknown.data1;

int TEST_RING_RW_close(SDL_RWops *_rw) { TEST_RING_RW_PREPARE();
	RING_debug("RING_RW_close()\n");
	SDL_RWclose(rw);
	SDL_FreeRW(_rw);
	return 0;
}

int TEST_RING_RW_read(SDL_RWops *_rw, void *ptr, int size, int maxnum) { TEST_RING_RW_PREPARE();
	//RING_debug("RING_RW_read(0x%08X, 0x%08X, %d)\n", rw, ptr, size * maxnum);
	RING_debug("RING_RW_read(%d)\n", size * maxnum);
	return SDL_RWread(rw, ptr, size, maxnum);
}

int TEST_RING_RW_write(SDL_RWops *_rw, const void *ptr, int size, int maxnum) { TEST_RING_RW_PREPARE();
	RING_debug("RING_RW_write(...)\n");
	return SDL_RWwrite(rw, ptr, size, maxnum);
}

int TEST_RING_RW_seek(SDL_RWops *_rw, int offset, int whence) {TEST_RING_RW_PREPARE();
	RING_debug("RING_RW_seek(%d, %s)\n", offset, STRING_SEEK(whence));
	return SDL_RWseek(rw, offset, whence);
}

//#define RING_TEST_READ

#ifdef RING_TEST_READ
	SDL_RWops *RING_RW_open(SDL_RWops *_rw, int length) {
		SDL_RWops *rw;
		if (!_rw) return NULL;
		rw = SDL_AllocRW();
		rw->hidden.unknown.data1 = _rw;
		rw->seek  = TEST_RING_RW_seek;
		rw->read  = TEST_RING_RW_read;
		rw->write = TEST_RING_RW_write;
		rw->close = TEST_RING_RW_close;	
		return rw;
	}
#else
	SDL_RWops *RING_RW_open(SDL_RWops *_rw, int length, int read_l) {
		SDL_RWops *rw;
		RING *ring;
		
		if (!_rw) return NULL;
		
		RING_debug("RING_RW_open(0x%08X, %d, %d)\n", _rw, length, read_l);
		
		rw = SDL_AllocRW();
		ring = RING_create(_rw, length, read_l);
		
		rw->hidden.unknown.data1 = ring;
		
		rw->seek  = RING_RW_seek;
		rw->read  = RING_RW_read;
		rw->write = RING_RW_write;
		rw->close = RING_RW_close;	
		
		return rw;
	}
#endif

#ifdef RING_test
void main() {
	char data[0x1000];
	FILE *f = fopen("tt", "wb");
	SDL_RWops *ring = RING_RW_open(SDL_RWFromFile("OPEN-PSP.MPG", "rb"), 0x20000);
	
	SDL_RWread(ring, data, 1, sizeof(data));
	
	SDL_RWseek(ring, 100, SEEK_SET);
	
	while (1) {
		int readed = SDL_RWread(ring, data, 1, sizeof(data));
		fwrite(data, 1, readed, f);
		if (readed < sizeof(data)) break;
	}
}
#endif
