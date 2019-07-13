/*
 * Dreamroq by Mike Melanson
 *
 * This is the header file to be included in the programs wishing to
 * use the Dreamroq playback engine.
 */

#ifndef NEWROQ_H
#define NEWROQ_H

#include <stdint.h>

#define ROQ_SUCCESS           0
#define ROQ_FILE_OPEN_FAILURE 1
#define ROQ_FILE_READ_FAILURE 2
#define ROQ_CHUNK_TOO_LARGE   3
#define ROQ_BAD_CODEBOOK      4
#define ROQ_INVALID_PIC_SIZE  5
#define ROQ_NO_MEMORY         6
#define ROQ_BAD_VQ_STREAM     7
#define ROQ_INVALID_DIMENSION 8
#define ROQ_RENDER_PROBLEM    9
#define ROQ_CLIENT_PROBLEM    10

#define ROQ_RGB565 0
#define ROQ_RGBA   1

/* The library calls this function when it has a frame ready for display. */
typedef int (*render_callback)(uint16_t *buf, int32_t width, int32_t height,
    int32_t stride, int32_t texture_height, int32_t colorspace);

/* The library calls this function when it has pcm samples ready for output. */
typedef int32_t (*audio_callback)(uint8_t *buf, int32_t samples, int32_t channels);

/* The library calls this function to ask whether it should quit playback.
 * Return non-zero if it's time to quit. */
typedef int32_t (*quit_callback)();

/* The library calls this function to indicate that playback of the movie is
 * complete. */
typedef int32_t (*finish_callback)(void);

typedef struct
{
    render_callback render_cb;
    audio_callback  audio_cb;
    quit_callback   quit_cb;
    finish_callback finish_cb;
} roq_callbacks_t;

int32_t dreamroq_play(char *filename, int32_t colorspace, int32_t loop,
    roq_callbacks_t *callbacks);

#endif  /* NEWROQ_H */
