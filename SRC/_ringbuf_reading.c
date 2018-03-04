#include <stdio.h>
#include <SDL.h>

/*
#define ubyte unsigned char

typedef struct {
	SDL_Thread *thread;
	SDL_RWops *file;            // Source RW
	ubyte *data;
	int data_length;
	int data_read;
	int data_write;
	int data_writed;
	int eof;
	SDL_mutex *mutex; // Used to lock members of struct
	int thread_end;
} RRBUFFER;

int readring_thread(void *_rrd) { RR_DATA *rrd = _rrd;
	return 0;
}

void init_buffer(RRBUFFER *b, SDL_RWops *file, int length) {
	b->file = file;
	b->data = malloc(length);
	b->data_length = length;
	b->data_read = 0;
	b->data_write = 0;
	b->data_writed = 0;
	b->eof = 0;
	b->mutex = SDL_CreateMutex();
	b->thread = SDL_CreateThread(readring_thread, b);
	b->thread_end = 0;
}

int readring_fillbuffer_tick(RRBUFFER *b) {
	int chunk_len = 0x8000;
	int readed = 0;
	
	// Limit by allocated memory
	if (b->data_length < chunk_len) chunk_len = data_length;
	
	// Limit by end of linear memory
	if ((data_length - b->data_write) < chunk_len) chunk_len = (data_length - b->data_write);
	
	// Rest of buffer
	if ((b->data_length - b->data_writed) < chunk_len) chunk_len = b->data_length - b->data_writed;
	
	// Limit by data_write
	if ((b->data_read > b->data_write) && (b->data_read - b->data_write) < chunk_len) chunk_len = b->data_read - b->data_write;
	
	readed = SDL_RWread(b->file, b->data + b->data_write, 1, chunk_len);
	
	SDL_mutexP(b->mutex);
		b->data_write += chunk_len;
		b->data_writed += chunk_len;
		
		if (b->data_write >= b->data_length) {
			b->data_write -= b->data_length;
			assert(b->data_write == 0);
		}
	SDL_mutexV(b->mutex);
	
	return readed;
}

int readring_read(BUFFER *b, ubyte *out_ptr, int out_len) {
	int chunk_len = 0;
	int readed = 0;
	
	while (1) {
		// Readed all required data
		if (readed >= out_len) {
			assert(readed == out_len)
			break;
		}
		
		// Readed all data in stream
		if (b->data_writed == 0 && b->eof) {
			break;
		}
		
		// Set max size
		chunk_len = out_len - readed;
		
		// Limit by data_writed
		if (chunk_len > b->data_writed) chunk_len = b->data_writed;
		
		// Limit by data_write
		if ((b->data_write > b->data_read) && (b->data_write - b->data_read) < chunk_len) chunk_len = b->data_write - b->data_read;
		
		// Limit by data_length
		if ((data_length - b->data_read) < chunk_len) chunk_len = (data_length - b->data_read);
		
		// Copy chunk
		memcpy(out_ptr + readed, b->data + b->data_read, chunk_len);
		
		// Update counters
		SDL_mutexP(b->mutex);
			b->data_read += chunk_len;
			b->data_writed -= chunk_len;
			
			if (b->data_read >= b->data_length) {
				b->data_read -= b->data_length;
				assert(b->data_read == 0);
			}
		SDL_mutexV(b->mutex);
		
		readed += chunk_len;
	}
	
	return readed;
}*/

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

/*
typedef struct {
	Uint8 *d; // Data Pointer
	Uint32 l; // Length
} SLICE;

SLICE SLICE_create(int size) { SLICE r; r.d = malloc(r.l = size); return r; }
void  SLICE_delete(SLICE slice) { free(slice.d); }
*/

/*void SLICE_get_ring_slices(SLICE base, int from, int to, SLICE *_s1, SLICE *_s2) {
	SLICE s1 = {0}, s2 = {0};
	
	// One slice
	if (from < to) {
		s1.d = base.d + from;
		s1.l = to - from;
	}
	// Two slices
	else {
		s1.d = base.d + from;
		s1.l = base.l - from;
		
		s2.d = base.d;
		s2.l = to;
	}
	
	if (_s1) *_s1 = s1; if (_s2) *_s2 = s2;
}*/

/*
void SLICE_print(SLICE slice) {
	printf("%08X:%d\n", slice.d, slice.l);
}
*/

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

/*
typedef struct {
	SDL_RWops  *file;            // Source RW
	
	// Buffer Related
	SLICE       buffer;          // Vector Buffer
	Uint32      br;              // Reading pos
	Uint32      bw;              // Writting pos
	Uint32      bf;              // Buffer filled
	Uint8       eof;             // Reached end of file
	
	// Thread Related
	SDL_Thread *thread;          // Thread
	Uint8       thread_finish;   // Thread finishing?
	Uint8       thread_fill;     // Must write in buffer?
} RR_DATA;

#define readring_len_common(from, to) return (rrd->from < rrd->to) ? (rrd->to - rrd->from) : ((rrd->buffer.l - rrd->from) + rrd->to);
int readring_len_w(RR_DATA *rrd) { if ( rrd->bf && (rrd->bw == rrd->br)) return 0; readring_len_common(bw, br); }
int readring_len_r(RR_DATA *rrd) { if (!rrd->bf && (rrd->bw == rrd->br)) return 0; readring_len_common(br, bw); }

#define RR_SET_RW(type, rrd, v) ((rrd)->type = (v) % (rrd)->buffer.l)
#define RR_SET_W(rrd, v) RR_SET_RW(bw, rrd, v);
#define RR_SET_R(rrd, v) RR_SET_RW(br, rrd, v);

int readring_len_write(RR_DATA *rrd) {
	if (rrd->bw < rrd->br) return rrd->br - rrd->bw;
	return rrd->buffer.l - rrd->bw + rrd->br;
}

int readring_seek(SDL_RWops *ctx, int offset, int whence) { RR_DATA *rrd = ctx->hidden.unknown.data1;
	int r;
	if (!(whence == SEEK_CUR && offset == 0)) {
		rrd->thread_fill = 0; // Stop reading buffer
		rrd->eof = 0;
	}
	r = SDL_RWseek(rrd->file, offset, whence);
	return r;
}

int readring_read(SDL_RWops *ctx, void *ptr, int size, int maxnum) { RR_DATA *rrd = ctx->hidden.unknown.data1;
	int read = size * maxnum;
	//rrd->thread_fill = 1;
	while (!rrd->eof) {
		//printf("%d\n", readring_len_r(rrd));
		//SDL_Delay(10);
	}
	return 0;
}

int readring_write(SDL_RWops *ctx, const void *ptr, int size, int num) { RR_DATA *rrd = ctx->hidden.unknown.data1;
	return -1;
}

int readring_close(SDL_RWops *ctx) { RR_DATA *rrd = ctx->hidden.unknown.data1;
	rrd->thread_finish = 1;
	SDL_WaitThread(rrd->thread, NULL);
	free(rrd);
	SDL_FreeRW(ctx);
}

void readring_thread_cycle(RR_DATA *rrd) {
	SLICE s[2];
	SLICE_get_ring_slices(rrd->buffer, rrd->bw, rrd->br, &s[0], &s[1]);
	printf("tick\n");
}

void readring_fill_buffer(RR_DATA *rrd, int maxLen) {
	if (readring_len_w(rrd) == 0) return; // No se puede escribir nada más

	readed = SDL_RWread(rrd->file, s[n].d, 1, final ? len : s[n].l);
}

#define readring_can_write(rrd) ((rrd)->thread_fill && (readring_len_w(rrd) != 0))

int readring_thread(void *_rrd) { RR_DATA *rrd = _rrd;
	while (1) {
		while (!rrd->thread_finish && readring_can_write(rrd)) readring_fill_buffer(rrd, 0x800);
		if (rrd->thread_finish) return 0;
		SDL_Delay(1);
	}
	return 0;
}

SDL_RWops *readring_open(SDL_RWops *file, int size) {
	SDL_RWops *ctx = SDL_AllocRW();
	RR_DATA *rrd;
	
	// RWOPs
	ctx->seek  = readring_seek;
	ctx->read  = readring_read;
	ctx->write = readring_write;
	ctx->close = readring_close;
	
	// RRD
	memset(ctx->hidden.unknown.data1 = rrd = malloc(sizeof(RR_DATA)), 0, sizeof(RR_DATA));
	rrd->buffer = SLICE_create(size);
	rrd->file   = file;
	rrd->thread = SDL_CreateThread(readring_thread, rrd);
	//rrd->mutex  = SDL_CreateMutex();
	
	return ctx;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

int main() {
	/*
	SLICE slice, s1, s2;
	slice = SLICE_create(1000);
	SLICE_print(slice);
	SLICE_get_ring_slices(slice, 120, 100, &s1, &s2);
	SLICE_print(s1);
	SLICE_print(s2);
	*/
	
	SLICE slice = SLICE_create(10000);
	
	int size;
	SDL_RWops *file = SDL_RWFromFile("OPEN-PSP.MPG", "rb");
	//SDL_RWops *rw = readring_open(file, 5 * 1024 * 1024);
	SDL_RWops *rw = readring_open(file, 10000);
	
	size = SDL_RWseek(rw, 0, SEEK_END);
	SDL_RWseek(rw, 0, SEEK_SET);
	
	SDL_RWread(rw, slice.d, 1, slice.l);
	SDL_Delay(10);
	
	//printf("%d\n", size);
	
	SDL_RWclose(rw);
	
	return 0;
}
