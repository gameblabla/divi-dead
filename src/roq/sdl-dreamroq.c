/*
 * Dreamroq by Mike Melanson
 *
 * This is a simple, sample program that helps test the Dreamroq library.
 */

#include <stdio.h>
#include <SDL/SDL.h>

#include "dreamroqlib.h"
#include "shared.h"

#define SOUND_FREQUENCY 22050
#define SOUND_SAMPLES_SIZE 1024

extern int32_t framerate;

#if defined(OSS_SOUND)
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/soundcard.h>
static int32_t oss_audio_fd = -1;
#elif defined(ALSA_SOUND)
#include <alsa/asoundlib.h>
static snd_pcm_t *handle;
#else
#include <portaudio.h>
static PaStream *apu_stream;
#endif


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
	if (width != screen->w || height != screen->h)
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

#if defined(OSS_SOUND)
	write(oss_audio_fd, snd, samples );
#elif defined(ALSA_SOUND)
	uint32_t ret, len;
	len = samples;
	ret = snd_pcm_writei(handle, snd, len);
	while(ret != len) 
	{
		if (ret < 0) 
		{
			snd_pcm_prepare( handle );
		}
		else 
		{
			len -= ret;
		}
		ret = snd_pcm_writei(handle, snd, len);
	}
#else
    Pa_WriteStream( apu_stream, snd, (samples/4));
#endif

    return ROQ_SUCCESS;
}

static int finish_cb()
{
    return ROQ_SUCCESS;
}

static void ROQ_Init_Sound()
{
#if defined(OSS_SOUND)
	uint32_t channels = 2;
	uint32_t format = AFMT_S16_LE;
	uint32_t tmp = SOUND_FREQUENCY;
	int32_t err_ret;
	
	oss_audio_fd = open("/dev/dsp", O_WRONLY );
	if (oss_audio_fd < 0)
	{
		printf("Couldn't open /dev/dsp.\n");
		return;
	}
	err_ret = ioctl(oss_audio_fd, SNDCTL_DSP_SPEED,&tmp);
	if (err_ret == -1)
	{
		printf("Could not set sound frequency\n");
		return;
	}
	err_ret = ioctl(oss_audio_fd, SNDCTL_DSP_CHANNELS, &channels);
	if (err_ret == -1)
	{
		printf("Could not set channels\n");
		return;
	}
	err_ret = ioctl(oss_audio_fd, SNDCTL_DSP_SETFMT, &format);
	if (err_ret == -1)
	{
		printf("Could not set sound format\n");
		return;
	}
#elif defined(ALSA_SOUND)
	snd_pcm_hw_params_t *params;
	uint32_t val;
	uint32_t ret;
	int32_t dir = -1;
	snd_pcm_uframes_t frames;
	
	/* Open PCM device for playback. */
	int32_t rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:0,0,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0);
		
	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:1,0,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
		rc = snd_pcm_open(&handle, "plughw:1,0", SND_PCM_STREAM_PLAYBACK, 0);

	if (rc < 0)
	{
		fprintf(stderr, "unable to open PCM device: %s\n", snd_strerror(rc));
		return;
	}
	
	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	rc = snd_pcm_hw_params_any(handle, params);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_any %s\n", snd_strerror(rc));
		return;
	}

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_access %s\n", snd_strerror(rc));
		return;
	}

	/* Signed 16-bit little-endian format */
	rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_format %s\n", snd_strerror(rc));
		return;
	}

	/* Two channels (stereo) */
	rc = snd_pcm_hw_params_set_channels(handle, params, 2);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_channels %s\n", snd_strerror(rc));
		return;
	}
	
	val = SOUND_FREQUENCY;
	rc=snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_rate_near %s\n", snd_strerror(rc));
		return;
	}

	/* Set period size to settings.aica.BufferSize frames. */
	frames = SOUND_SAMPLES_SIZE;
	rc = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_buffer_size_near %s\n", snd_strerror(rc));
		return;
	}
	frames *= 4;
	rc = snd_pcm_hw_params_set_buffer_size_near(handle, params, &frames);
	if (rc < 0)
	{
		fprintf(stderr, "Error:snd_pcm_hw_params_set_buffer_size_near %s\n", snd_strerror(rc));
		return;
	}

	/* Write the parameters to the driver */
	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0)
	{
		fprintf(stderr, "Unable to set hw parameters: %s\n", snd_strerror(rc));
		return;
	}

#else
	Pa_Initialize();
	
	PaStreamParameters outputParameters;
	outputParameters.device = Pa_GetDefaultOutputDevice();
	outputParameters.channelCount = 2;
	outputParameters.sampleFormat = paInt16;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	
	Pa_OpenStream( &apu_stream, NULL, &outputParameters, SOUND_FREQUENCY, SOUND_SAMPLES_SIZE, paNoFlag, NULL, NULL);
	Pa_StartStream( apu_stream );
#endif
}


uint_fast8_t MOVIE_PLAY(char *name, int skip)
{
    int32_t status;
    int32_t err;
    roq_callbacks_t cbs;
    int32_t bitdepth_check;
    
    create_surface = 0;
    
    if (!_file_exists(name))
    {
		return 0;
	}
    
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
	
#if defined(OSS_SOUND)
	if (oss_audio_fd >= 0)
	{
		close(oss_audio_fd);
		oss_audio_fd = -1;
	}
#elif defined(ALSA_SOUND)
	if (handle)
	{
		snd_pcm_drain(handle);
		snd_pcm_close(handle);
	}
#else
	Pa_StopStream(apu_stream);
	Pa_CloseStream(apu_stream);
	Pa_Terminate();
#endif
	

	if (video_sdl_surface != NULL && create_surface == 1)
	{
		SDL_FreeSurface(video_sdl_surface);
	}
    
    return status;
}
