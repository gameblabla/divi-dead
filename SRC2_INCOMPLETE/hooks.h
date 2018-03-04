#ifndef __HOOKS_H
#define __HOOKS_H
	#define HOOK_INIT_PRE();
	#define HOOK_INIT_SOUND_PRE()
	#define HOOK_INIT_VIDEO_PRE() 
	
	#define HOOK_VIDEO_WIDTH  640
	#define HOOK_VIDEO_HEIGHT 480
	#define HOOK_VIDEO_BPP    32
	#define HOOK_VIDEO_FLAGS  SDL_DOUBLEBUF
	
	#define HOOK_AUDIO_HAS      1
	#define HOOK_AUDIO_FREQ     22050
	#define HOOK_AUDIO_FORMAT   AUDIO_S8
	#define HOOK_AUDIO_CHANNELS 1
	#define HOOK_AUDIO_CSIZE    1024
	
	#define HOOK_RESMAN_SIZE    (16 * 1024 * 1024)
	
	#if defined(PAT_WIN)
		#include "PAT/WIN/hooks.h"
		
	#elif defined(PAT_PSP)
		#include "PAT/PSP/hooks.h"
		
	#elif defined(PAT_NDS)
		#include "PAT/SDC/hooks.h"
		
	#elif defined(PAT_SDC)
		#include "PAT/NDS/hooks.h"
		
	#elif defined(PAT_MAC)
		#include "PAT/MAC/hooks.h"
		
	#elif defined(PAT_LIN)
		#include "PAT/LIN/hooks.h"
		
	#else
		#error "Unsupported platform"
		
	#endif
	
#endif
