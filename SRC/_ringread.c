#define RING_CHUNK_SIZE 0x10000 /* 64KB */

typedef struct _RING_CHUNK {
	int pos;
	int count;
	char data[RING_CHUNK_SIZE];
	int readed;
	int writed;
	struct _RING_CHUNK *next;
} RING_CHUNK;

typedef struct {
	int thread_finish;
	SDL_thread *thread;
	SDL_mutex  *mutex;
	SDL_RWops *file;
	RING_CHUNK *first, *last;
	int eof;
} RING_READ;

#define PREPARE_RING_CHUNK() RING_READ *rr = ((SDL_RWops *)ctx)->hidden.unknown.data1;

RING_CHUNK *RING_CHUNK_create(int count) {
	RING_CHUNK *chunk = malloc(sizeof(RING_CHUNK));
	if (!chunk) return NULL;
	chunk->readed = chunk->writed = 0;
	chunk->next = NULL;
	chunk->count = count;
	return chunk;
}

void RING_CHUNK_remove(RING_CHUNK *chunk) {
	if (!chunk) return;
	free(chunk);
}

void RING_CHUNK_remove_cascade(RING_CHUNK *chunk) {
	RING_CHUNK *next;
	if (!chunk) return;
	next = chunk->next;
	chunk->next = NULL;
	if (next) RING_CHUNK_remove_cascade(next);
	free(next);
}

int RING_READ_close(SDL_RWops *ctx) { PREPARE_RING_CHUNK();
	rr->thread_finish = 1;
	SDL_WaitThread(rr->thread, NULL);
	RING_CHUNK_remove_cascade(rr->first);
	free(rr);
	SDL_FreeRW(ctx);
	return 0;
}

int readring_seek(SDL_RWops *ctx, int offset, int whence) { PREPARE_RING_CHUNK();
	return 0;
}

int RING_READ_read(SDL_RWops *ctx, void *ptr, int size, int n) { PREPARE_RING_CHUNK();
	return 0;
}

int RING_READ_write(SDL_RWops *ctx, void *ptr, int size, int n) { PREPARE_RING_CHUNK();
	return 0;
}

int readring_thread(void *_ctx) { SDL_RWops *ctx = _ctx; PREPARE_RING_CHUNK();
	

	return 0;
}

SDL_RWops *RING_READ_open(SDL_RWops *file, int num_chunks) {
	SDL_RWops *ctx = SDL_AllocRW();
	RING_READ *rr = malloc(sizeof(RING_READ));
	
	// RWOPs
	ctx->seek  = RING_READ_seek;
	ctx->read  = RING_READ_read;
	ctx->write = RING_READ_write;
	ctx->close = RING_READ_close;
	
	ctx->hidden.unknown.data1 = rr;
	rr->file = file;
	rr->thread_finish = 0;
	rr->first = NULL;
	rr->last = NULL;
	rr->mutex = SDL_CreateMutex();
	rr->eof = 0;
	rr->thread = SDL_CreateThread(RING_READ_thread, rrd);
	
	/*
	// RRD
	memset(ctx->hidden.unknown.data1 = rrd = malloc(sizeof(RR_DATA)), 0, sizeof(RR_DATA));
	rrd->buffer = SLICE_create(size);
	rrd->file   = file;
	
	//rrd->mutex  = SDL_CreateMutex();
	*/
	
	return ctx;
}