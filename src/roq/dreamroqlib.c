/*
 * Dreamroq by Mike Melanson
 * Audio support by Josh Pearson
 * 
 * This is the main playback engine.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dreamroqlib.h"
#include "platform.h"
#include "shared.h"

#define RoQ_INFO           0x1001
#define RoQ_QUAD_CODEBOOK  0x1002
#define RoQ_QUAD_VQ        0x1011
#define RoQ_JPEG           0x1012
#define RoQ_SOUND_MONO     0x1020
#define RoQ_SOUND_STEREO   0x1021
#define RoQ_PACKET         0x1030
#define RoQ_SIGNATURE      0x1084

#define CHUNK_HEADER_SIZE 8

#define LE_16(buf) (*buf | (*(buf+1) << 8))
#define LE_32(buf) (*buf | (*(buf+1) << 8) | (*(buf+2) << 16) | (*(buf+3) << 24))

#define MAX_BUF_SIZE (64 * 1024)

#define ROQ_CODEBOOK_SIZE 256
#define SQR_ARRAY_SIZE 256

struct roq_audio
{
     int32_t pcm_samples;
     int32_t channels;
     int32_t position;
     short snd_sqr_array[SQR_ARRAY_SIZE];
     uint8_t pcm_sample[MAX_BUF_SIZE];
} roq_audio;

int32_t framerate;

static uint16_t frame_contain[2][307200];

typedef struct
{
    int32_t width;
    int32_t height;
    int32_t mb_width;
    int32_t mb_height;
    int32_t mb_count;
    int32_t alpha;

    int32_t current_frame;
    int32_t stride;
    int32_t texture_height;
    int32_t colorspace;

    uint16_t cb2x2_rgb565[ROQ_CODEBOOK_SIZE][4];
    uint16_t cb4x4_rgb565[ROQ_CODEBOOK_SIZE][16];

	#ifndef CONVERT_16BPP
    uint32_t cb2x2_rgba[ROQ_CODEBOOK_SIZE][4];
    uint32_t cb4x4_rgba[ROQ_CODEBOOK_SIZE][16];
    #endif
} roq_state;

static int32_t roq_unpack_quad_codebook_rgb565(uint8_t *buf, int32_t size,
    int32_t arg, roq_state *state)
{
    int32_t y[4];
    int32_t yp, u, v;
    int32_t r, g, b;
    int32_t count2x2;
    int32_t count4x4;
    int32_t i, j;
    uint16_t *v2x2;
    uint16_t *v4x4;

    count2x2 = (arg >> 8) & 0xFF;
    count4x4 =  arg       & 0xFF;

    if (!count2x2)
        count2x2 = ROQ_CODEBOOK_SIZE;
    /* 0x00 means 256 4x4 vectors iff there is enough space in the chunk
     * after accounting for the 2x2 vectors */
    if (!count4x4 && count2x2 * 6 < size)
        count4x4 = ROQ_CODEBOOK_SIZE;

    /* size sanity check, taking alpha into account */
    if (state->alpha && (count2x2 * 10 + count4x4 * 4) != size)
    {
        return ROQ_BAD_CODEBOOK;
    }
    if (!state->alpha && (count2x2 * 6 + count4x4 * 4) != size)
    {
        return ROQ_BAD_CODEBOOK;
    }

    /* unpack the 2x2 vectors */
    for (i = 0; i < count2x2; i++)
    {
        /* unpack the YUV components from the bytestream */
        for (j = 0; j < 4; j++)
        {
            y[j] = *buf++;
            if (state->alpha)
                buf++;
        }
        u  = *buf++;
        v  = *buf++;

        /* convert to RGB565 */
        for (j = 0; j < 4; j++)
        {
            yp = (y[j] - 16) * 1.164;
            r = (yp + 1.596 * (v - 128)) / 8;
            g = (yp - 0.813 * (v - 128) - 0.391 * (u - 128)) / 4;
            b = (yp + 2.018 * (u - 128)) / 8;

            if (r < 0) r = 0;
            if (r > 31) r = 31;
            if (g < 0) g = 0;
            if (g > 63) g = 63;
            if (b < 0) b = 0;
            if (b > 31) b = 31;

            state->cb2x2_rgb565[i][j] = (
                (r << 11) | 
                (g <<  5) |
                (b <<  0) );
        }
    }

    /* unpack the 4x4 vectors */
    for (i = 0; i < count4x4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            v2x2 = state->cb2x2_rgb565[*buf++];
            v4x4 = state->cb4x4_rgb565[i] + (j / 2) * 8 + (j % 2) * 2;
            v4x4[0] = v2x2[0];
            v4x4[1] = v2x2[1];
            v4x4[4] = v2x2[2];
            v4x4[5] = v2x2[3];
        }
    }

    return ROQ_SUCCESS;
}

#ifndef CONVERT_16BPP
static int32_t roq_unpack_quad_codebook_rgba(uint8_t *buf, int32_t size,
    int32_t arg, roq_state *state)
{
    int32_t y[4];
    /*int32_t a[4];*/
    int32_t yp, u, v;
    int32_t r, g, b;
    int32_t count2x2;
    int32_t count4x4;
    int32_t i, j;
    uint32_t *v2x2;
    uint32_t *v4x4;

    count2x2 = (arg >> 8) & 0xFF;
    count4x4 =  arg       & 0xFF;

    if (!count2x2)
        count2x2 = ROQ_CODEBOOK_SIZE;
    /* 0x00 means 256 4x4 vectors iff there is enough space in the chunk
     * after accounting for the 2x2 vectors */
    if (!count4x4 && count2x2 * 6 < size)
        count4x4 = ROQ_CODEBOOK_SIZE;

    /* size sanity check, taking alpha into account */
    if (state->alpha && (count2x2 * 10 + count4x4 * 4) != size)
    {
        return ROQ_BAD_CODEBOOK;
    }
    if (!state->alpha && (count2x2 * 6 + count4x4 * 4) != size)
    {
        return ROQ_BAD_CODEBOOK;
    }

    /* unpack the 2x2 vectors */
    for (i = 0; i < count2x2; i++)
    {
        /* unpack the YUV components from the bytestream */
        for (j = 0; j < 4; j++)
        {
            y[j] = *buf++;
            /*if (state->alpha)
                a[j] = *buf++;
            else
                a[j] = 255;*/
        }
        u  = *buf++;
        v  = *buf++;

        /* convert to RGBA */
        for (j = 0; j < 4; j++)
        {
            yp = (y[j] - 16) * 1.164;
            r = (yp + 1.596 * (v - 128));
            g = (yp - 0.813 * (v - 128) - 0.391 * (u - 128));
            b = (yp + 2.018 * (u - 128));

            if (r < 0) r = 0;
            if (r > 255) r = 255;
            if (g < 0) g = 0;
            if (g > 255) g = 255;
            if (b < 0) b = 0;
            if (b > 255) b = 255;
            
            state->cb2x2_rgba[i][j] = ((r << 16) | (g << 8) | (b));
			//state->cb2x2_rgba[i][j] = SDL_MapRGBA(screen_video->format, r, g, b, a[j]);
			/* state->cb2x2_rgba[i][j] = ((r << 16) | (g << 8) | (b) | (a[j])); */
            /* state->cb2x2_rgba[i][j] = ((r << 24) | (g << 16) | (b <<  8) | a[j]); */
        }
    }

    /* unpack the 4x4 vectors */
    for (i = 0; i < count4x4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            v2x2 = state->cb2x2_rgba[*buf++];
            v4x4 = state->cb4x4_rgba[i] + (j / 2) * 8 + (j % 2) * 2;
            v4x4[0] = v2x2[0];
            v4x4[1] = v2x2[1];
            v4x4[4] = v2x2[2];
            v4x4[5] = v2x2[3];
        }
    }

    return ROQ_SUCCESS;
}

#endif

#define GET_BYTE(x) \
    if (index >= size) { \
        status = ROQ_BAD_VQ_STREAM; \
        x = 0; \
    } else { \
        x = buf[index++]; \
    }

#define GET_MODE() \
    if (!mode_count) { \
        GET_BYTE(mode_lo); \
        GET_BYTE(mode_hi); \
        mode_set = (mode_hi << 8) | mode_lo; \
        mode_count = 16; \
    } \
    mode_count -= 2; \
    mode = (mode_set >> mode_count) & 0x03;

static int32_t roq_unpack_vq_rgb565(uint8_t *buf, int32_t size, uint32_t arg, 
    roq_state *state)
{
    int32_t status = ROQ_SUCCESS;
    int32_t mb_x, mb_y;
    int32_t block;     /* 8x8 blocks */
    int32_t subblock;  /* 4x4 blocks */
    int32_t stride = state->stride;
    int32_t i;

    /* frame and pixel management */
    uint16_t *this_frame;
    uint16_t *last_frame;

    int32_t line_offset;
    int32_t mb_offset;
    int32_t block_offset;
    int32_t subblock_offset;

    uint16_t *this_ptr;
    uint32_t *this_ptr32;
    uint16_t *last_ptr;
    uint16_t *vector16;
    uint32_t *vector32;
    int32_t stride32 = stride / 2;

    /* bytestream management */
    int32_t index = 0;
    int32_t mode_set = 0;
    int32_t mode, mode_lo, mode_hi;
    int32_t mode_count = 0;

    /* vectors */
    int32_t mx, my;
    int32_t motion_x, motion_y;
    uint8_t data_byte;

    mx = (int8_t)(arg >> 8);
    my = (int8_t)arg;

    if (state->current_frame & 1)
    {
        this_frame = (uint16_t*)frame_contain[1];
        last_frame = (uint16_t*)frame_contain[0];
    }
    else
    {
        this_frame = (uint16_t*)frame_contain[0];
        last_frame = (uint16_t*)frame_contain[1];
    }
    /* special case for frame 1, which needs to begin with frame 0's data */
    if (state->current_frame == 1)
    {
        memcpy(frame_contain[1], frame_contain[0],
            state->texture_height * state->stride * sizeof(uint16_t));
    }

    for (mb_y = 0; mb_y < state->mb_height && status == ROQ_SUCCESS; mb_y++)
    {
        line_offset = mb_y * 16 * stride;
        for (mb_x = 0; mb_x < state->mb_width && status == ROQ_SUCCESS; mb_x++)
        {
            mb_offset = line_offset + mb_x * 16;
            for (block = 0; block < 4 && status == ROQ_SUCCESS; block++)
            {
                block_offset = mb_offset + (block / 2 * 8 * stride) + (block % 2 * 8);
                /* each 8x8 block gets a mode */
                GET_MODE();
                switch (mode)
                {
                case 0:  /* MOT: skip */
                    break;

                case 1:  /* FCC: motion compensation */
                    /* this needs to be done 16 bits at a time due to
                     * data alignment issues on the SH-4 */
                    GET_BYTE(data_byte);
                    motion_x = 8 - (data_byte >>  4) - mx;
                    motion_y = 8 - (data_byte & 0xF) - my;
                    last_ptr = last_frame + block_offset + 
                        (motion_y * stride) + motion_x;
                    this_ptr = this_frame + block_offset;
                    for (i = 0; i < 8; i++)
                    {
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;

                        last_ptr += stride - 8;
                        this_ptr += stride - 8;
                    }
                    break;

                case 2:  /* SLD: upsample 4x4 vector */
                    GET_BYTE(data_byte);
                    vector16 = state->cb4x4_rgb565[data_byte];
                    for (i = 0; i < 4*4; i++)
                    {
                        this_ptr = this_frame + block_offset +
                            (i / 4 * 2 * stride) + (i % 4 * 2);
                        this_ptr[0] = *vector16;
                        this_ptr[1] = *vector16;
                        this_ptr[stride+0] = *vector16;
                        this_ptr[stride+1] = *vector16;
                        vector16++;
                    }
                    break;

                case 3:  /* CCC: subdivide into 4 subblocks */
                    for (subblock = 0; subblock < 4; subblock++)
                    {
                        subblock_offset = block_offset + (subblock / 2 * 4 * stride) + (subblock % 2 * 4);

                        GET_MODE();
                        switch (mode)
                        {
                        case 0:  /* MOT: skip */
                            break;

                        case 1:  /* FCC: motion compensation */
                            GET_BYTE(data_byte);
                            motion_x = 8 - (data_byte >>  4) - mx;
                            motion_y = 8 - (data_byte & 0xF) - my;
                            last_ptr = last_frame + subblock_offset + 
                                (motion_y * stride) + motion_x;
                            this_ptr = this_frame + subblock_offset;
                            for (i = 0; i < 4; i++)
                            {
                                *this_ptr++ = *last_ptr++;
                                *this_ptr++ = *last_ptr++;
                                *this_ptr++ = *last_ptr++;
                                *this_ptr++ = *last_ptr++;

                                last_ptr += stride - 4;
                                this_ptr += stride - 4;
                            }
                            break;

                        case 2:  /* SLD: use 4x4 vector from codebook */
                            GET_BYTE(data_byte);
                            vector32 = (uint32_t*)state->cb4x4_rgb565[data_byte];
                            this_ptr32 = (uint32_t*)this_frame;
                            this_ptr32 += subblock_offset / 2;
                            for (i = 0; i < 4; i++)
                            {
                                *this_ptr32++ = *vector32++;
                                *this_ptr32++ = *vector32++;

                                this_ptr32 += stride32 - 2;
                            }
                            break;

                        case 3:  /* CCC: subdivide into 4 subblocks */
                            GET_BYTE(data_byte);
                            vector16 = state->cb2x2_rgb565[data_byte];
                            this_ptr = this_frame + subblock_offset;
                            this_ptr[0] = vector16[0];
                            this_ptr[1] = vector16[1];
                            this_ptr[stride+0] = vector16[2];
                            this_ptr[stride+1] = vector16[3];

                            GET_BYTE(data_byte);
                            vector16 = state->cb2x2_rgb565[data_byte];
                            this_ptr[2] = vector16[0];
                            this_ptr[3] = vector16[1];
                            this_ptr[stride+2] = vector16[2];
                            this_ptr[stride+3] = vector16[3];

                            this_ptr += stride * 2;

                            GET_BYTE(data_byte);
                            vector16 = state->cb2x2_rgb565[data_byte];
                            this_ptr[0] = vector16[0];
                            this_ptr[1] = vector16[1];
                            this_ptr[stride+0] = vector16[2];
                            this_ptr[stride+1] = vector16[3];

                            GET_BYTE(data_byte);
                            vector16 = state->cb2x2_rgb565[data_byte];
                            this_ptr[2] = vector16[0];
                            this_ptr[3] = vector16[1];
                            this_ptr[stride+2] = vector16[2];
                            this_ptr[stride+3] = vector16[3];

                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    /* sanity check to see if the stream was fully consumed */
    if (status == ROQ_SUCCESS && index < size-2)
    {
        status = ROQ_BAD_VQ_STREAM;
    }

    return status;
}

#ifndef CONVERT_16BPP
static int32_t roq_unpack_vq_rgba(uint8_t *buf, int32_t size, uint32_t arg, 
    roq_state *state)
{
    int32_t status = ROQ_SUCCESS;
    int32_t mb_x, mb_y;
    int32_t block;     /* 8x8 blocks */
    int32_t subblock;  /* 4x4 blocks */
    int32_t stride = state->stride;
    int32_t i;

    /* frame and pixel management */
    uint32_t *this_frame;
    uint32_t *last_frame;

    int32_t line_offset;
    int32_t mb_offset;
    int32_t block_offset;
    int32_t subblock_offset;

    uint32_t *this_ptr;
    uint32_t *last_ptr;
    uint32_t *vector;

    /* bytestream management */
    int32_t index = 0;
    int32_t mode_set = 0;
    int32_t mode, mode_lo, mode_hi;
    int32_t mode_count = 0;

    /* vectors */
    int32_t mx, my;
    int32_t motion_x, motion_y;
    uint8_t data_byte;

    mx = (int8_t)(arg >> 8);
    my = (int8_t)arg;

    if (state->current_frame & 1)
    {
        this_frame = (uint32_t*)frame_contain[1];
        last_frame = (uint32_t*)frame_contain[0];
    }
    else
    {
        this_frame = (uint32_t*)frame_contain[0];
        last_frame = (uint32_t*)frame_contain[1];
    }
    /* special case for frame 1, which needs to begin with frame 0's data */
    if (state->current_frame == 1)
    {
        memcpy(frame_contain[1], frame_contain[0],
            state->texture_height * state->stride * sizeof(uint32_t));
    }

    for (mb_y = 0; mb_y < state->mb_height && status == ROQ_SUCCESS; mb_y++)
    {
        line_offset = mb_y * 16 * stride;
        for (mb_x = 0; mb_x < state->mb_width && status == ROQ_SUCCESS; mb_x++)
        {
            mb_offset = line_offset + mb_x * 16;
            for (block = 0; block < 4 && status == ROQ_SUCCESS; block++)
            {
                block_offset = mb_offset + (block / 2 * 8 * stride) + (block % 2 * 8);
                /* each 8x8 block gets a mode */
                GET_MODE();
                switch (mode)
                {
                case 0:  /* MOT: skip */
                    break;

                case 1:  /* FCC: motion compensation */
                    GET_BYTE(data_byte);
                    motion_x = 8 - (data_byte >>  4) - mx;
                    motion_y = 8 - (data_byte & 0xF) - my;
                    last_ptr = last_frame + block_offset + 
                        (motion_y * stride) + motion_x;
                    this_ptr = this_frame + block_offset;
                    for (i = 0; i < 8; i++)
                    {
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;
                        *this_ptr++ = *last_ptr++;

                        last_ptr += stride - 8;
                        this_ptr += stride - 8;
                    }
                    break;

                case 2:  /* SLD: upsample 4x4 vector */
                    GET_BYTE(data_byte);
                    vector = state->cb4x4_rgba[data_byte];
                    for (i = 0; i < 4*4; i++)
                    {
                        this_ptr = this_frame + block_offset +
                            (i / 4 * 2 * stride) + (i % 4 * 2);
                        this_ptr[0] = *vector;
                        this_ptr[1] = *vector;
                        this_ptr[stride+0] = *vector;
                        this_ptr[stride+1] = *vector;
                        vector++;
                    }
                    break;

                case 3:  /* CCC: subdivide into 4 subblocks */
                    for (subblock = 0; subblock < 4; subblock++)
                    {
                        subblock_offset = block_offset + (subblock / 2 * 4 * stride) + (subblock % 2 * 4);

                        GET_MODE();
                        switch (mode)
                        {
                        case 0:  /* MOT: skip */
                            break;

                        case 1:  /* FCC: motion compensation */
                            GET_BYTE(data_byte);
                            motion_x = 8 - (data_byte >>  4) - mx;
                            motion_y = 8 - (data_byte & 0xF) - my;
                            last_ptr = last_frame + subblock_offset + 
                                (motion_y * stride) + motion_x;
                            this_ptr = this_frame + subblock_offset;
                            for (i = 0; i < 4; i++)
                            {
                                *this_ptr++ = *last_ptr++;
                                *this_ptr++ = *last_ptr++;
                                *this_ptr++ = *last_ptr++;
                                *this_ptr++ = *last_ptr++;

                                last_ptr += stride - 4;
                                this_ptr += stride - 4;
                            }
                            break;

                        case 2:  /* SLD: use 4x4 vector from codebook */
                            GET_BYTE(data_byte);
                            vector = state->cb4x4_rgba[data_byte];
                            this_ptr = this_frame + subblock_offset;
                            for (i = 0; i < 4; i++)
                            {
                                *this_ptr++ = *vector++;
                                *this_ptr++ = *vector++;
                                *this_ptr++ = *vector++;
                                *this_ptr++ = *vector++;

                                this_ptr += stride - 4;
                            }
                            break;

                        case 3:  /* CCC: subdivide into 4 subblocks */
                            GET_BYTE(data_byte);
                            vector = state->cb2x2_rgba[data_byte];
                            this_ptr = this_frame + subblock_offset;
                            this_ptr[0] = vector[0];
                            this_ptr[1] = vector[1];
                            this_ptr[stride+0] = vector[2];
                            this_ptr[stride+1] = vector[3];

                            GET_BYTE(data_byte);
                            vector = state->cb2x2_rgba[data_byte];
                            this_ptr[2] = vector[0];
                            this_ptr[3] = vector[1];
                            this_ptr[stride+2] = vector[2];
                            this_ptr[stride+3] = vector[3];

                            this_ptr += stride * 2;

                            GET_BYTE(data_byte);
                            vector = state->cb2x2_rgba[data_byte];
                            this_ptr[0] = vector[0];
                            this_ptr[1] = vector[1];
                            this_ptr[stride+0] = vector[2];
                            this_ptr[stride+1] = vector[3];

                            GET_BYTE(data_byte);
                            vector = state->cb2x2_rgba[data_byte];
                            this_ptr[2] = vector[0];
                            this_ptr[3] = vector[1];
                            this_ptr[stride+2] = vector[2];
                            this_ptr[stride+3] = vector[3];

                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    /* sanity check to see if the stream was fully consumed */
    if (status == ROQ_SUCCESS && index < size-2)
    {
        status = ROQ_BAD_VQ_STREAM;
    }

    return status;
}
#endif

int32_t dreamroq_play(char *filename, int32_t colorspace, int32_t loop,
    roq_callbacks_t *cbs)
{
    FILE *f;
    size_t file_ret;
    int32_t chunk_id;
    uint32_t chunk_size;
    uint32_t chunk_arg;
    roq_state state;
    int32_t status;
    int32_t initialized = 0;
    uint8_t read_buffer[MAX_BUF_SIZE];
    int32_t i, snd_left, snd_right;

    f = fopen(filename, "rb");
    if (!f)
        return ROQ_FILE_OPEN_FAILURE;

    file_ret = fread(read_buffer, CHUNK_HEADER_SIZE, 1, f);
    if (file_ret != 1)
    {
        fclose(f);
        //printf("\nROQ_FILE_READ_FAILURE\n\n");
        return ROQ_FILE_READ_FAILURE;
    }
    chunk_id   = LE_16(&read_buffer[0]);
    chunk_size = LE_32(&read_buffer[2]);
    if (chunk_id != RoQ_SIGNATURE || chunk_size != 0xFFFFFFFF)
    {
        fclose(f);
        return ROQ_FILE_READ_FAILURE;
    }
    framerate = LE_16(&read_buffer[6]);
    //printf("RoQ file plays at %d frames/sec\n", framerate);
    
    /* Initialize Audio SQRT Look-Up Table */
    for(i = 0; i < 128; i++)
    {
        roq_audio.snd_sqr_array[i] = i * i;
        roq_audio.snd_sqr_array[i + 128] = -(i * i);
    }

    status = ROQ_SUCCESS;
    while (!feof(f) && status == ROQ_SUCCESS)
    {
        /* if client program defined a quit callback, check if it's time
         * to quit */
        if (cbs->quit_cb && cbs->quit_cb())
            break;

        file_ret = fread(read_buffer, CHUNK_HEADER_SIZE, 1, f);
        if (file_ret != 1)
        {
            /* if the read failed but the file is not EOF, there is a deeper
             * problem; don't entertain the idea of looping */
            if (!feof(f))
                break;
            else if (loop)
            {
                /* it shouldn't be necessary to close and re-open the file
                 * here; however, this works around a bug in KOS 1.2.0 in
                 * which seeking back doesn't clear the EOF flag */
                fclose(f);
                f = fopen(filename, "rb");
                if (!f)
                    status = ROQ_FILE_OPEN_FAILURE;
                else
                {
                    /* skip the signature header */
                    fseek(f, 8, SEEK_SET);
                    continue;
                }
            }
            else
                break;
        }

        chunk_id   = LE_16(&read_buffer[0]);
        chunk_size = LE_32(&read_buffer[2]);
        chunk_arg  = LE_16(&read_buffer[6]);

        if (chunk_size > MAX_BUF_SIZE)
        {
            fclose(f);
            return ROQ_CHUNK_TOO_LARGE;
        }

        file_ret = fread(read_buffer, chunk_size, 1, f);
        if (file_ret != 1)
        {
            status = ROQ_FILE_READ_FAILURE;
            break;
        }
            
        state.colorspace = colorspace;

        switch(chunk_id)
        {
        case RoQ_INFO:
            if (initialized)
                continue;

            state.alpha = chunk_arg;
            state.width = LE_16(&read_buffer[0]);
            state.height = LE_16(&read_buffer[2]);
            /* width and height each need to be divisible by 16 */
            if ((state.width & 0xF) || (state.height & 0xF))
            {
                status = ROQ_INVALID_PIC_SIZE;
                break;
            }
            
            state.mb_width = state.width / 16;
            state.mb_height = state.height / 16;
            state.mb_count = state.mb_width * state.mb_height;
            if (state.width < 8 || state.width > 1024)
                status = ROQ_INVALID_DIMENSION;
            else
            {
                state.stride = 8;
                while (state.stride < state.width)
                    state.stride <<= 1;
            }
            if (state.height < 8 || state.height > 1024)
                status = ROQ_INVALID_DIMENSION;
            else
            {
                state.texture_height = 8;
                while (state.texture_height < state.height)
                    state.texture_height <<= 1;
            }
            
            state.stride = state.width;
            state.texture_height = state.height;
            
			// printf("RoQ_INFO: dimensions = %dx%d, %dx%d; %d mbs, texture = %dx%d\n",  state.width, state.height, state.mb_width, state.mb_height, state.mb_count, state.stride, state.texture_height);
			// printf("Size of %d\n", state.texture_height * state.stride * sizeof(uint32_t));
            state.current_frame = 0;
            /* set this flag so that this code is not executed again when looping */
            initialized = 1;
            break;

        case RoQ_QUAD_CODEBOOK:
			#ifdef CONVERT_16BPP
				status = roq_unpack_quad_codebook_rgb565(read_buffer, chunk_size, chunk_arg, &state);
			#else
			if (colorspace == ROQ_RGB565)
                status = roq_unpack_quad_codebook_rgb565(read_buffer, chunk_size, chunk_arg, &state);
			else if (colorspace == ROQ_RGBA)
                status = roq_unpack_quad_codebook_rgba(read_buffer, chunk_size, 
                    chunk_arg, &state);
			#endif
            break;

        case RoQ_QUAD_VQ:
			#ifdef CONVERT_16BPP
			status = roq_unpack_vq_rgb565(read_buffer, chunk_size, chunk_arg, &state);
			#else
            if (colorspace == ROQ_RGB565)
                status = roq_unpack_vq_rgb565(read_buffer, chunk_size, 
                    chunk_arg, &state);
            else if (colorspace == ROQ_RGBA)
                status = roq_unpack_vq_rgba(read_buffer, chunk_size, 
                    chunk_arg, &state);
			#endif
            if (cbs->render_cb)
                status = cbs->render_cb(frame_contain[state.current_frame & 1], 
                    state.width, state.height, state.stride, state.texture_height,
                    colorspace);

            state.current_frame++;
            break;

        case RoQ_JPEG:
            break;

        case RoQ_SOUND_MONO:
            roq_audio.channels = 1;
            roq_audio.pcm_samples = chunk_size*2;
            snd_left = chunk_arg;
            for(i = 0; i < chunk_size; i++)
            {
                snd_left += roq_audio.snd_sqr_array[read_buffer[i]];
                roq_audio.pcm_sample[i * 2] = snd_left & 0xff;
                roq_audio.pcm_sample[i * 2 + 1] = (snd_left & 0xff00) >> 8;
            }
            if (cbs->audio_cb)
                status = cbs->audio_cb(roq_audio.pcm_sample, roq_audio.pcm_samples,
                                   roq_audio.channels);
            break;

        case RoQ_SOUND_STEREO:
            roq_audio.channels = 2;
            roq_audio.pcm_samples = chunk_size*2;
            snd_left = (chunk_arg & 0xFF00);
            snd_right = (chunk_arg & 0xFF) << 8;
            for(i = 0; i < chunk_size; i += 2)
            {
                snd_left  += roq_audio.snd_sqr_array[read_buffer[i]];
                snd_right += roq_audio.snd_sqr_array[read_buffer[i+1]];
                roq_audio.pcm_sample[i * 2] = snd_left & 0xff;
                roq_audio.pcm_sample[i * 2 + 1] = (snd_left & 0xff00) >> 8;
                roq_audio.pcm_sample[i * 2 + 2] =  snd_right & 0xff;
                roq_audio.pcm_sample[i * 2 + 3] = (snd_right & 0xff00) >> 8;
            }
            if (cbs->audio_cb)
                status = cbs->audio_cb( roq_audio.pcm_sample, roq_audio.pcm_samples,
                                   roq_audio.channels );
            break;

        case RoQ_PACKET:
            /* still unimplemented */
            break;

        default:
            break;
        }
    }

    if (f) fclose(f);

    if (cbs->finish_cb)
        cbs->finish_cb();

    return status;
}

