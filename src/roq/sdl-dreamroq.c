/*
 * Dreamroq by Mike Melanson
 *
 * This is a simple, sample program that helps test the Dreamroq library.
 */

#include <stdio.h>
#include <SDL/SDL.h>
#include <portaudio.h>

#include "dreamroqlib.h"
#include "shared.h"

#define SOUND_FREQUENCY 22050
#define SOUND_SAMPLES_SIZE 1024

extern int32_t framerate;
static PaStream *apu_stream;
static int32_t text_mult = 2;
static int32_t create_surface = 0;

static SDL_Surface* video_sdl_surface;

static int32_t quit_cb()
{
	SDL_Event event;
	Uint8 *keystate = SDL_GetKeyState(NULL);
    SDL_PollEvent(&event);
    if (keystate[BUTTON_A_DEFINE] || keystate[BUTTON_B_DEFINE] || keystate[BUTTON_EXIT_KEY] || keystate[BUTTON_D_DEFINE])
    {
		return 1;
	}
	
    return 0;
}

static int32_t render_cb(uint16_t buf[], int32_t width, int32_t height, int32_t stride, int32_t texture_height, int32_t colorspace)
{
	if (width > screen->w || height > screen->h)
	{
		if (create_surface == 0)
		{
			video_sdl_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, SCREEN_BUFFER_BPP, 0, 0, 0, 0);
			create_surface = 1;
		}
		
		SDL_LockSurface(video_sdl_surface);
		memcpy(video_sdl_surface->pixels, buf, (width*height)*text_mult);
		SDL_UnlockSurface(video_sdl_surface);
		SDL_SoftStretch(video_sdl_surface, NULL, screen_video, NULL);
		SDL_Flip(screen_video);
	}
	else
	{
		SDL_LockSurface(screen_video);
		memcpy(screen_video->pixels, buf, (width*height)*text_mult);
		SDL_UnlockSurface(screen_video);
		SDL_Flip(screen_video);
	}

    return ROQ_SUCCESS;
}

static int32_t data_size = 0;
static int32_t audio_output_initialized = 0;

static int32_t audio_cb(uint8_t *snd, int32_t samples, int32_t channels)
{
    int32_t byte_rate;
    int32_t i;

    Pa_WriteStream( apu_stream, snd, (samples/4));

    return ROQ_SUCCESS;
}

static int finish_cb()
{
    return ROQ_SUCCESS;
}

static void ROQ_Init_Sound()
{
	Pa_Initialize();
	
	PaStreamParameters outputParameters;
	outputParameters.device = Pa_GetDefaultOutputDevice();
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paInt16;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	
	Pa_OpenStream( &apu_stream, NULL, &outputParameters, SOUND_FREQUENCY, SOUND_SAMPLES_SIZE, paNoFlag, NULL, NULL);
	Pa_StartStream( apu_stream );
}


uint_fast8_t MOVIE_PLAY(char *name, int skip)
{
    int32_t status;
    int32_t err;
    roq_callbacks_t cbs;
    int32_t bitdepth_check;
    
    create_surface = 0;
    
	ROQ_Init_Sound();

    cbs.render_cb = render_cb;
    cbs.audio_cb = audio_cb;
    cbs.quit_cb = quit_cb;
    cbs.finish_cb = finish_cb;
    
    if (VIDEOMODE_BITS == 32)
    {
		bitdepth_check = ROQ_RGBA;
		text_mult = 4;
	}
	else
	{
		bitdepth_check = ROQ_RGB565;
		text_mult = 2;
	}

	status = dreamroq_play(name, bitdepth_check, 0, &cbs);
	
	Pa_StopStream(apu_stream);
	Pa_CloseStream(apu_stream);
	Pa_Terminate();
	
	if (video_sdl_surface != NULL && create_surface == 1)
	{
		SDL_FreeSurface(video_sdl_surface);
	}
    
    return status;
}
