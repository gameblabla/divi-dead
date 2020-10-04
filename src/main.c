#ifdef DREAMCAST
#include <kos.h>
KOS_INIT_FLAGS(INIT_DEFAULT);
#endif

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <libgen.h>
#include <stdarg.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include <SDL/SDL_ttf.h>

#include "main.h"

#ifdef JOYSTICK_ENABLED
#define joy_commit_range 8192
static uint_fast8_t dpad_input[4] = {0, 0, 0, 0};
#endif

char game_directory[512];

void lang_postinit();

int toggle_info = 0;
int initialized = 0;
int menu_mode = 0;
int audio_initialized = 0;
int debug_global = 0;
int hide_debug = 0;
int must_resume_music = 0;

int audio_voice_enabled = 1;
int audio_music_enabled = 1;

int is_shift_jis = 0;

// Reading
SDL_mutex *mutex_reading = NULL;
void READING_START() { if (mutex_reading) SDL_mutexP(mutex_reading); }
void READING_END  () { if (mutex_reading) SDL_mutexV(mutex_reading); }

#ifdef DREAMCAST
#include <SDL_dreamcast.h>
#endif

//int volume_music  = 64;
//int volume_music  = 80;
int volume_music  = 92;
int volume_effect = 32;
int volume_voice  = 128;
int volume_movie  = 40;

SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};
SDL_Color red   = {0xFF, 0x00, 0x00, 0xFF};
SDL_Color grey  = {0x7F, 0x7F, 0x7F, 0xFF};
SDL_Color blue  = {0xA0, 0xA0, 0xFF, 0xFF};
SDL_Color green = {0x00, 0xFF, 0x00, 0xFF};
SDL_Color black = {0x00, 0x00, 0x00, 0xFF};

Mix_Music *music  = NULL;
Mix_Chunk *effect = NULL;
Mix_Chunk *voice_mix = NULL;
Mix_Chunk *click = NULL;
TTF_Font *font  = NULL; int font_height  = 0;
TTF_Font *font2 = NULL; int font2_height = 0;

SDL_Surface *screen = NULL, *screen_video = NULL, *interface = NULL;
SDL_PixelFormat screen_format;

int done = 0, game_end = 0, swap_buttons = 0;
char language[0x30] = {0};
char lang_texts[12][0x30] = {
	"ENGLISH.DL1",
	"START",
	"SAVE",
	"LOAD",
	"EXIT",
	"%.1f%% GALLERY",
	"SAVE IMAGE",
	"Start new game?",
	"OPTIONS",
	"voice",
	"music",
	"No data"
};

int font_size = FONT_DEFAULT_SIZE;

int CHECK_MENU_KEYS();
int OPTION_SAVE_IMAGE(int);
void GAME_MUSIC_STOP();
void GAME_MUSIC_PLAY(char*);
void vfs_init();

int image_cache_pos = 0;

// cell(112x74) margin(8x4)
CG_PAGE cg_pages[6] = {
	{"CGMODE_1", "CGMODE_A",   0, 0, 1 },
	{"CGMODE_2", "CGMODE_B",  25, 1, 1 },
	{"CGMODE_2", "CGMODE_C",  50, 1, 1 },
	{"CGMODE_2", "CGMODE_D",  75, 1, 1 },
	{"CGMODE_2", "CGMODE_E", 100, 1, 1 },
	{"CGMODE_3", "CGMODE_F", 125, 1, 0 },
};

struct save_struct_game save, save_temp;

struct save_gallery save_s;

char save_title[0x10];

// FLIST
char gallery[150][0x10];

int game_playing = 0;
int game_start = 0;

int finish_game = 0;

uint32_t keys = 0;

uint8_t  script[0x10000];
uint8_t *scrp = script;

OPTION script_options[0x100];
int script_options_count = 0;

MAP_OPTION map_options[0x100];
int map_options_count = 0;
char map_images[2][0x20];


////////////////////////////////////////////////////////////
// UTILITIES                                              //
////////////////////////////////////////////////////////////

uint8_t  mem1(uint8_t *ptr) { return ((int)ptr[0] << 0); }
uint16_t mem2(uint8_t *ptr) { return (ptr[0] << 0) | (ptr[1] << 8); }
uint32_t   mem4(uint8_t *ptr) { return (ptr[0] << 0) | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24); }

void text_at_base(SDL_Color c_front, SDL_Color c_back, int x, int y, char *buffer, int update) {
	SDL_Rect d;
	SDL_Surface *surface;
	
	if (!font2 || !font2_height) return;

	surface = TTF_RenderText_Shaded(font2, buffer, c_front, c_back);

	d.x = x;
	d.y = y;
	d.w = surface->w;
	d.h = surface->h;

	SDL_BlitSurface(surface, NULL, screen_video, &d);
	if (update) SDL_UpdateRect(screen_video, x, y, x + surface->w, y + surface->h);
	SDL_FreeSurface(surface);
}

void text_at(SDL_Color front, int x, int y, char *format, ...) {
	char buffer[0x1000];
	va_list ap;
	va_start(ap, format);
	vsprintf(buffer, format, ap);
	va_end(ap);
	text_at_base(front, black, x, y, buffer, 0);
}

void message_info(char *format, ...) {
	char buffer[0x1000];
	va_list ap;
	va_start(ap, format);
	vsprintf(buffer, format, ap);
	va_end(ap);
	text_at_base(white, black, 0, 0, buffer, 1);
}

int iround(double r) { return (int)r + ((r <= ((double)(int)r) + 0.5 ) ? 0 : 1); }

void PROGRAM_EXIT(int v) {
	SDL_Quit();
	exit(v);
}

void PROGRAM_EXIT_ERROR(char *format, ...) {
	char buffer[0x1000];
	va_list ap;
	va_start(ap, format);
	vsprintf(buffer, format, ap);
	fprintf(stderr, "%s\n", buffer);
	va_end(ap);
	
	message_info("FATAL ERROR: %s", buffer);
	
	SDL_Delay(5000);
	
	PROGRAM_EXIT(-1);
}

int process_exit(const SDL_Event *event) {
	if (event->type == SDL_QUIT) PROGRAM_EXIT(0);
	return 1;
}

void PROGRAM_DELAY(uint32_t milli) {
	int event = 0;
	uint32_t start = SDL_GetTicks();
	while (SDL_GetTicks() < start + milli) {
		if (done) PROGRAM_EXIT(0);
		SDL_PumpEvents();
		SDL_Delay(1);
		event++;
	}
	if (!event) SDL_PumpEvents();
}

void do_init() {
	SDL_Delay(100);
	GAME_MUSIC_STOP();
	vfs_init();
	GAME_MUSIC_PLAY(NULL);
}

int IMAGE_CACHE_MAX = IMAGE_CACHE_MAX_DEFAULT;

#ifdef EMULATE_PSP
	#undef SCREEN_WIDTH 
	#undef SCREEN_HEIGHT
	#define SCREEN_WIDTH  480
	#define SCREEN_HEIGHT 272

	#undef IMAGE_CACHE_MAX
	#define IMAGE_CACHE_MAX 12

	#undef font_size_corrected
	#define font_size_corrected 9
	#undef text_margin_top
	#define text_margin_top 1
#endif

IMAGE_CACHE image_cache[32];

#define JOYB_MAX 16

JOYB joybut[JOYB_MAX];
#define joy_keys_max 200
int joy_keys_pos = 0;
int joy_keys[joy_keys_max];
int joy_just_pressed = 0;

int joy_keys_get(int pos) {
	int n = joy_keys_pos - pos - 1;
	while (n < 0) n += joy_keys_max;
	return joy_keys[n];
}

void joy_push_keys(int v) {
	if (v > 0) keys |= v;
	if (v < 0) keys &= ~(-v);
	if (joy_keys_get(0) == v) return;
	joy_keys[joy_keys_pos++] = v;
	joy_keys_pos %= joy_keys_max;
	joy_just_pressed = 1;
}

void joy_push_keys_pd(int v) {
	joy_push_keys(+v);
	joy_push_keys(-v);
}

#ifdef DEBUG_INPUT_OPTIONAL
int debug_combination[6] = { P_D(K_UP), P_D(K_DOWN), P_D(K_LEFT), P_D(K_RIGHT), P_D(K_MODE), 0 };
#endif

int joy_check_combination_basic(int *ptr) {
	int *cur = ptr;
	int n, len = 0;
	while (*cur++ != 0) len++;
	for (cur = ptr, n = 0; *cur == joy_keys_get(len - n - 1); cur++, n++) if (*cur == 0) return 1;
	return 0;
}

int joy_check_combination_basic_just(int *ptr) {
	if (!joy_just_pressed) return 0;
	return joy_check_combination_basic(ptr);
}


void KEYS_CLEAR() {
	memset(joybut, 0, sizeof(joybut));
	keys = 0;
}

void KEYS_UPDATE() {
	#ifdef JOYSTICK_ENABLED
	int32_t joy_axis[2] = {0, 0};
	#endif
	int n;
	uint32_t ctime = SDL_GetTicks();
	keys = 0;
	
	/*if (initialized == 0) {
		do_init();
	}*/
	
	if (done) PROGRAM_EXIT(0);
	
	joy_just_pressed = 0;
	
	SDL_Event event;
	while (SDL_PollEvent(&event))  {
		switch (event.type) {
			case SDL_QUIT: PROGRAM_EXIT(0);
			case SDL_KEYDOWN:
				//printf("KeyDown: %d\n", event.key.keysym.sym);
				/*switch (event.key.keysym.sym) {
					case SDLK_UP:     keys |= K_UP; break;
					case SDLK_DOWN:   keys |= K_DOWN; break;
					case SDLK_LEFT:   keys |= K_LEFT; break;
					case SDLK_RIGHT:  keys |= K_RIGHT; break;
					case SDLK_RETURN: keys |= K_A; break;
					case 8:
					case SDLK_ESCAPE: keys |= K_B; break;
					case SDLK_LCTRL: 
					case SDLK_LSHIFT: keys |= K_L; break;
					case SDLK_RCTRL: 
					case SDLK_RSHIFT: keys |= K_R; break;
					case SDLK_SPACE:  keys |= K_MODE; break;
					case SDLK_c:      keys |= K_SPECIAL; break;
					default: break;
				}*/
				switch (event.key.keysym.sym) {
					#ifndef JOYSTICK_ENABLED
					case SDLK_UP:     joy_push_keys(+K_UP); break;
					case SDLK_DOWN:   joy_push_keys(+K_DOWN); break;
					case SDLK_LEFT:   joy_push_keys(+K_LEFT); break;
					case SDLK_RIGHT:  joy_push_keys(+K_RIGHT); break;
					#else
					case SDLK_UP:     joy_push_keys(+K_UP);  dpad_input[0] = 1; break;
					case SDLK_DOWN:   joy_push_keys(+K_DOWN); dpad_input[3] = 1; break;
					case SDLK_LEFT:   joy_push_keys(+K_LEFT); dpad_input[1] = 1; break;
					case SDLK_RIGHT:  joy_push_keys(+K_RIGHT); dpad_input[2] = 1; break;
					#endif
					case BUTTON_A_DEFINE: joy_push_keys(+K_A); break;
					case BUTTON_B_DEFINE: joy_push_keys(+K_B); break;
					case BUTTON_C_DEFINE: joy_push_keys(+K_L); break;
					case BUTTON_D_DEFINE: joy_push_keys(+K_R); break;
					case BUTTON_MODE_DEFINE:  joy_push_keys(+K_MODE); break;
					case BUTTON_SPECIAL_DEFINE:      joy_push_keys(+K_SPECIAL); break;
					case BUTTON_EXIT_KEY: PROGRAM_EXIT(0); break;
					default: break;
				}			
			break;
			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
					#ifndef JOYSTICK_ENABLED
					case SDLK_UP:     joy_push_keys(-K_UP); break;
					case SDLK_DOWN:   joy_push_keys(-K_DOWN); break;
					case SDLK_LEFT:   joy_push_keys(-K_LEFT); break;
					case SDLK_RIGHT:  joy_push_keys(-K_RIGHT); break;
					#else
					case SDLK_UP:     joy_push_keys(-K_UP); dpad_input[0] = 0; break;
					case SDLK_DOWN:   joy_push_keys(-K_DOWN); dpad_input[3] = 0; break;
					case SDLK_LEFT:   joy_push_keys(-K_LEFT); dpad_input[1] = 0; break;
					case SDLK_RIGHT:  joy_push_keys(-K_RIGHT); dpad_input[2] = 0; break;
					#endif
					case BUTTON_A_DEFINE: joy_push_keys(-K_A); break;
					case BUTTON_B_DEFINE: joy_push_keys(-K_B); break;
					case BUTTON_C_DEFINE: joy_push_keys(-K_L); break;
					case BUTTON_D_DEFINE: joy_push_keys(-K_R); break;
					case BUTTON_MODE_DEFINE:  joy_push_keys(-K_MODE); break;
					case BUTTON_SPECIAL_DEFINE:      joy_push_keys(-K_SPECIAL); break;
					default: break;
				}			
			break;
			#if JOYSTICK_ENABLED
			case SDL_JOYAXISMOTION:
			switch (event.jaxis.axis)
			{
				case 0: /* X axis */
					joy_axis[0] = event.jaxis.value;
				break;
				case 1: /* Y axis */
					joy_axis[1] = event.jaxis.value;
				break;
			}
			break;
			case SDL_JOYBUTTONUP:
				//printf("SDL_JOYBUTTONUP\n");
				joybut[event.jbutton.button].pressed = 0;
				
				switch (event.jbutton.button) {
					case 0: joy_push_keys(-K_B); break;
					case 8: joy_push_keys(-K_UP); break;
					case 6: joy_push_keys(-K_DOWN); break;
					case 7: joy_push_keys(-K_LEFT); break;
					case 9: joy_push_keys(-K_RIGHT); break;
					case 11:
					case 1: joy_push_keys(-(!swap_buttons ? K_A : K_B)); break;
					case 2: joy_push_keys(-(!swap_buttons ? K_B : K_A)); break;
					case 3: joy_push_keys(-K_SPECIAL); break; break;
					case 12: case 13:
					case 4: joy_push_keys(-K_L); break;
					case 5: joy_push_keys(-K_R); break;
					case 10: joy_push_keys(-K_MODE); break;
					default: break;
				}					
			break;
			case SDL_JOYBUTTONDOWN:
				//printf("SDL_JOYBUTTONDOWN\n");
				joybut[event.jbutton.button].time = ctime;
				joybut[event.jbutton.button].delay = KEY_REPEAT_DELAY1;
				joybut[event.jbutton.button].pressed = 2;
				switch (event.jbutton.button) {
					case 0: joy_push_keys(+K_B); break;
					case 8: joy_push_keys(+K_UP); break;
					case 6: joy_push_keys(+K_DOWN); break;
					case 7: joy_push_keys(+K_LEFT); break;
					case 9: joy_push_keys(+K_RIGHT); break;
					case 11:
					case 1: joy_push_keys(+(!swap_buttons ? K_A : K_B)); break;
					case 2: joy_push_keys(+(!swap_buttons ? K_B : K_A)); break;
					case 3: joy_push_keys(+K_SPECIAL); break; break;
					case 12: case 13:
					case 4: joy_push_keys(+K_L); break;
					case 5: joy_push_keys(+K_R); break;
					case 10: joy_push_keys(+K_MODE); break;
					default: break;
/*
 0 Triangle
 1 Circle
 2 Cross
 3 Square
 4 Left trigger
 5 Right trigger
 6 Down
 7 Left
 8 Up
 9 Right
10 Select
11 Start
12 Home
13 Hold
*/					
				}					
			break;
			#endif
		}
	}
	
	#ifdef JOYSTICK_ENABLED
	if (joy_axis[0] > joy_commit_range) joy_push_keys(+K_RIGHT);
	else if (joy_axis[0] < -joy_commit_range) joy_push_keys(+K_LEFT);
	else if (dpad_input[1] == 0 && dpad_input[2] == 0)
	{
		joy_push_keys(-K_LEFT);
		joy_push_keys(-K_RIGHT);
	}
		
	if (joy_axis[1] > joy_commit_range) joy_push_keys(+K_UP);
	else if (joy_axis[1] < -joy_commit_range) joy_push_keys(+K_DOWN);
	else if (dpad_input[0] == 0 && dpad_input[3] == 0)
	{
		joy_push_keys(-K_UP);
		joy_push_keys(-K_DOWN);
	}
	#endif
	
	for (n = 0; n < JOYB_MAX; n++) {
		if (!joybut[n].pressed) continue;
		
		if ((joybut[n].pressed == 2) || ((ctime - joybut[n].time) >= joybut[n].delay)) {
			if (joybut[n].pressed == 2) {
				joybut[n].pressed = 1;
			} else{
				joybut[n].time = ctime;
				joybut[n].delay = KEY_REPEAT_DELAY2;
			}
			
			switch (n) {
				case 0: keys |= K_B; break;
				case 8: keys |= K_UP; break;
				case 6: keys |= K_DOWN; break;
				case 7: keys |= K_LEFT; break;
				case 9: keys |= K_RIGHT; break;
				case 11:
				case 1: keys |= !swap_buttons ? K_A : K_B; break;
				case 2: keys |= !swap_buttons ? K_B : K_A; break;
				case 12: case 13:
				case 4: keys |= K_L; break;
				case 5: keys |= K_R; break;
				case 10: keys |= K_MODE; break;
				default: break;
			}			
		}
	}
	
	if (must_resume_music) {
		must_resume_music = 0;
		GAME_MUSIC_PLAY(NULL);
	}
}

void GAME_UPDATE_DEBUG_INFO() {
	if (!debug_global) return;
	if (!toggle_info) return;
	if (hide_debug) return;
	message_info("%s:%d | END_FLAGS(%d[%d%d%d%d%d%d%d%d], %d[%d%d%d%d%d%d%d%d%d])", save.script, scrp - script,
		save.flags[700], save.flags[701], save.flags[702], save.flags[703], save.flags[704], save.flags[705], save.flags[706], save.flags[707], save.flags[708],
		save.flags[800], save.flags[801], save.flags[802], save.flags[803], save.flags[804], 0, save.flags[806], save.flags[807], save.flags[808], save.flags[810]
	);
}

void GAME_SCREEN_UPDATE(SDL_Surface *from) {
	SDL_BlitSurface(from, NULL, screen_video, NULL);
	SDL_Flip(screen_video);
	GAME_UPDATE_DEBUG_INFO();
}

void GAME_SCREEN_UPDATE_RECT(SDL_Surface *from, SDL_Rect *rect) {
	if (rect == NULL) {
		GAME_SCREEN_UPDATE(from);
		return;
	}
	SDL_BlitSurface(from, rect, screen_video, rect);
	
	#ifndef FBUFFER_FULL_UPDATE
		SDL_UpdateRect(screen_video, rect->x, rect->y, rect->w, rect->h);
	#else
		SDL_Flip(screen_video);
	#endif
	
	//if (toggle_info) message_info("%s:%d", save.script, scrp - script);
}

//#define UPDATE_RECTS_OPTIMIZED
//#define UPDATE_METHOD_1

void GAME_SCREEN_UPDATE_RECTS(SDL_Surface *from, int numrects, SDL_Rect *rects) {
	int n;
	#ifdef UPDATE_METHOD_1
		SDL_BlitSurface(from, NULL, screen_video, NULL);
	#else
		for (n = 0; n < numrects; n++) SDL_BlitSurface(from, rects + n, screen_video, rects + n);
	#endif
	
	#ifndef FBUFFER_FULL_UPDATE
		SDL_UpdateRects(screen_video, numrects, rects);
	#else
		SDL_Flip(screen_video);
	#endif	
	
	//if (toggle_info) message_info("%s:%d", save.script, scrp - script);
}

SDL_Rect rs[3072 * 2 * 4];
int rc = 0;

#ifdef UPDATE_RECTS_OPTIMIZED
#define _UPDATE_RECT_START()
#define _UPDATE_RECT(r) rs[rc++] = r;
#define _UPDATE_RECT_END() GAME_SCREEN_UPDATE_RECTS(screen, rc, rs);
#else
#define _UPDATE_RECT_START()
#define _UPDATE_RECT(r) GAME_SCREEN_UPDATE_RECT(screen, &r);
#define _UPDATE_RECT_END()
#endif

void GAME_BUFFER_REPAINT(int effect) {
	int n, m, y, steps;
	
	//printf("GAME_BUFFER_REPAINT(%d)\n", effect);
	
	//effect = -10;
	
	switch (effect) {
		default:
		case -1:
		case 0: GAME_SCREEN_UPDATE(screen); break; // Directo
		
		case 1: // ??
		case -10: { // Persiana arriba-abajo
			int y2;
			m = 0;
			steps = SCREEN_HEIGHT * 2 / 16;
			for (n = 0; n < steps; n++) {
				_UPDATE_RECT_START();
				for (y = 0, y2 = n; y <= SCREEN_HEIGHT; y += 16, y2--) {
					if (y2 < 0) break;
					if (y2 >= 16) continue;
					SDL_Rect r = { 0, y - y2, SCREEN_WIDTH, 1 };
					_UPDATE_RECT(r);
				}
				_UPDATE_RECT_END();
				PROGRAM_DELAY(1000 / steps);
			}
		} break;

		case 3: { // Persiana arriba-abajo|abajo-arriba
			steps = SCREEN_HEIGHT / 4;
			for (n = 0; n < SCREEN_HEIGHT; n += 4) {
				SDL_Rect r1 = { 0, n                    , SCREEN_WIDTH, 2 };
				SDL_Rect r2 = { 0, SCREEN_HEIGHT - 2 - n, SCREEN_WIDTH, 2 };
				_UPDATE_RECT_START();
					_UPDATE_RECT(r1);
					_UPDATE_RECT(r2);
				_UPDATE_RECT_END();
				PROGRAM_DELAY(1000 / steps);
			}
		} break;

	#ifndef NULLDC_HACK
		case -2: // Barrido personaje
		case  2: // Barrido horizontal
			steps = 16;
			for (m = 0; m < 16; m++) {
				_UPDATE_RECT_START();
				for (n = 0; n < SCREEN_WIDTH; n += 16) {
					SDL_Rect r = { n + m, 0, 1, SCREEN_HEIGHT };
					_UPDATE_RECT(r);
				}
				_UPDATE_RECT_END();
				PROGRAM_DELAY(100 / steps);
			}
		break;
	#else
		case -2: // Barrido personaje
		case  2: // Barrido horizontal	
	#endif
		
		case -3: // Barrido título
		case 4: // Barrido vertical
			steps = 16;
			for (m = 0; m < 16; m++) {
				_UPDATE_RECT_START();
				for (n = 0; n < SCREEN_HEIGHT; n += 16) {
					SDL_Rect r = { 0, n + m, SCREEN_WIDTH, 1 };
					_UPDATE_RECT(r);
				}
				_UPDATE_RECT_END();
				PROGRAM_DELAY(100 / steps);
			}
		break;
	}
	
	GAME_SCREEN_UPDATE(screen);
	
#ifdef GAME_DEBUG
	message("GAME_BUFFER_REPAINT() | %d", effect);
#endif
}

void GAME_SCRIPT_JUMP(int addr);

void prepare_rects() {
	int n;
	for (n = 0; n < update_rects_count; n++) {
		SDL_Rect *rect = update_rects[n];
		rect->x = ACORRECT_X(rect->x);
		rect->y = ACORRECT_Y(rect->y);
		rect->w = ACORRECT_X(rect->w);
		rect->h = ACORRECT_Y(rect->h);
	}
}
void prepare_interface_image(SDL_Rect *rect, SDL_Surface **dest) {
	SDL_PixelFormat *pf = interface->format;
	SDL_Surface *temp = SDL_CreateRGBSurface(interface->flags, rect->w, rect->h, pf->BitsPerPixel, pf->Rmask, pf->Gmask, pf->Bmask, pf->Amask);
	SDL_BlitSurface(interface, rect, temp, NULL);
	SDL_SetColorKey(temp, SDL_SRCCOLORKEY, SDL_MapRGB(temp->format, 0x00, 0xFF, 0x00));
	*dest = SDL_ZoomSurface(temp, ASPECT_X, ASPECT_Y);
	SDL_FreeSurface(temp);
}

void SDL_Audio_Init()
{
	SDL_InitSubSystem(SDL_INIT_AUDIO);
	if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 1024) < 0) {
		printf("Can't initialize audio\n");
	} else {
		audio_initialized = 1;
	}
	
	if (audio_initialized) {
		if (!(click = Mix_LoadWAV_RW(STREAM_UNCOMPRESS_MEM(click_wav, size_click_wav), 1))) {
			PROGRAM_EXIT_ERROR("Can't locate 'CLICK_WAV'");
		} else {
			printf("Loaded 'CLICK_WAV'\n");
		}
	}
}

void sdl_init() {
	SDL_Init(SDL_INIT_VIDEO);
	mutex_reading = SDL_CreateMutex();
	SDL_ShowCursor(0);

	SDL_WM_SetCaption("Divi Dead SDL", NULL);
	if (!(screen_video = SDL_SetVideoMode(
#if defined(DYNAMIC_RESOLUTION)
	0,	0,
#else
	SCREEN_WIDTH, SCREEN_HEIGHT,
#endif
	VIDEOMODE_BITS, VIDEOMODE_FLAGS))) PROGRAM_EXIT_ERROR("Can't initialize graphic mode");


#ifdef ADJUST_FONT_SIZE_DYNAMIC
	if (screen_video->w < 321)
	{
		font_size = 16;
	}
	else if (screen_video->w < 641)
	{
		font_size = 17;
	}
	else if (screen_video->w < 969)
	{
		font_size = 18;
	}
	else
	{
		font_size = 19;
	}
#endif

	screen_format = *screen_video->format;

	if (!(screen = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BUFFER_BPP, 0, 0, 0, 0))) PROGRAM_EXIT_ERROR("Can't initialize frame buffer");
	{
		SDL_Rect r;
		SDL_Surface *s;
		printf("Loading Splash...");
		s = IMG_Load_RW(STREAM_UNCOMPRESS_MEM(splash_jpg, size_splash_jpg), 1);
		if (s == NULL) {
			printf("Error: '%s'\n", SDL_GetError());
		} else {
			printf("OK\n");
		}
		
		//SDL_FillRect(screen, NULL, 0xFFFFFFFF);
		r.x = screen->w / 2 - s->w / 2;
		r.y = screen->h / 2 - s->h / 2;
		r.w = s->w;
		r.h = s->h;
		SDL_BlitSurface(s, NULL, screen, &r);
		SDL_FreeSurface(s);
		SDL_BlitSurface(screen, NULL, screen_video, NULL);
		SDL_Flip(screen_video);
	}
	
	if (TTF_Init() < 0) PROGRAM_EXIT_ERROR("Can't initialize TTF");
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);

#ifndef ENABLE_VIDEO_ROQ
	SDL_Audio_Init();
#endif
	
	if (!(font = TTF_OpenFontRW(SDL_RWFromMem(font_ttf, size_font_ttf), 1, font_height = font_size_corrected))) {
		PROGRAM_EXIT_ERROR("Invalid FONT_TTF (II)");
	} else {
		printf("Loaded FONT_TTF\n");
	}
	
	if (!(font2 = TTF_OpenFontRW(SDL_RWFromMem(font2_ttf, size_font2_ttf), 1, font2_height = 13))) {
		PROGRAM_EXIT_ERROR("Invalid FONT2_TTF (II)");
	} else {
		printf("Loaded FONT2_TTF\n");
	}

	SDL_JoystickOpen(0);
	SDL_SetEventFilter(process_exit);
	SDL_EnableKeyRepeat(KEY_REPEAT_DELAY1, KEY_REPEAT_DELAY2);
	prepare_rects();
}

void lang_init() {
	FILE *f;
	char temp[512], temp2[512];
	int n = 0, m, l;
	
	if (!strlen(language)) strcpy(language, LANGUAGE_DEFAULT);
	
#ifdef GAME_HOME_DIRECTORY
	snprintf(temp, sizeof(temp), "%s/%s%s/%s.TXT", game_directory, FILE_PREFIX, ALTERNATE_LANG, language);
	snprintf(temp2, sizeof(temp2), "%s/%s%s/%s.txt", game_directory, FILE_PREFIX, ALTERNATE_LANG, language);
#else
	sprintf(temp, "%s%s/%s.TXT", FILE_PREFIX, ALTERNATE_LANG, language);
	sprintf(temp2, "%s%s/%s.txt", FILE_PREFIX, ALTERNATE_LANG, language);
#endif

	if (_file_exists(temp)) 
	{
		if ((f = fopen(temp, "rb")) == NULL) {
			printf("Can't open '%s'\n", temp);
			return;
		}
	}
	else if (_file_exists(temp2)) 
	{
		if ((f = fopen(temp2, "rb")) == NULL) {
			printf("Can't open '%s'\n", temp);
			return;
		}
	}
	else
	{
		printf("File '%s' doesn't exists\n", temp);
		sprintf(temp, "%sLANG/%s.TXT", FILE_PREFIX, language);
		return;	
	}

	while (!feof(f)) {
		fgets(lang_texts[n], 0x30, f);
		for (m = 0, l = strlen(lang_texts[n]); m < l; m++) {
			switch (lang_texts[n][m]) {
				case '\n': case '\r':
					lang_texts[n][m] = 0;
					m = l;
				break;
			}
		}
		n++;
	}
	fclose(f);
}

void vfs_init() {
	//if (initialized != 0) return;

	char temp[512];
	
	//initialized = 1;
	
	VFS_RESET();
	
#ifdef GAME_HOME_DIRECTORY
	snprintf(temp, sizeof(temp), "%s/%s%s/%s", game_directory, FILE_PREFIX, ALTERNATE_LANG, lang_texts[0]);
#else
	sprintf(temp, "%s%s/%s", FILE_PREFIX, ALTERNATE_LANG, lang_texts[0]);
#endif
	if (!_file_exists(temp)) 
	{
#ifdef GAME_HOME_DIRECTORY
		snprintf(temp, sizeof(temp), "%s/%s%s/%s", game_directory, FILE_PREFIX, ALTERNATE_LANG_UNDERSCORE, "english.dl1");
		if (!_file_exists(temp)) 
		{
			printf("File '%s' doesn't exists\n", temp);
			sprintf(temp, "%sLANG/%s", FILE_PREFIX, lang_texts[0]);
		}
#else
		printf("File '%s' doesn't exists\n", temp);
		sprintf(temp, "%sLANG/%s", FILE_PREFIX, lang_texts[0]);
#endif
	}
	VFS_MOUNT(temp);
	
#ifdef GAME_HOME_DIRECTORY
	char sg_path[512], wv_path[512];
	
	snprintf(sg_path, sizeof(sg_path), "%s/%s", game_directory, "SG.DL1");
	snprintf(wv_path, sizeof(wv_path), "%s/%s", game_directory, "WV.DL1");
	
	if (!VFS_MOUNT(sg_path))
	{
		snprintf(sg_path, sizeof(sg_path), "%s/%s", game_directory, "sg.dl1");
		if (!VFS_MOUNT(sg_path))
		{
			PROGRAM_EXIT_ERROR("Can't locate 'SG.DL1'");
		}
	}
	
	if (!VFS_MOUNT(wv_path))
	{
		snprintf(wv_path, sizeof(wv_path), "%s/%s", game_directory, "wv.dl1");
		if (!VFS_MOUNT(wv_path))
		{
			PROGRAM_EXIT_ERROR("Can't locate 'WV.DL1'");
		}
	}
#else
	if (!VFS_MOUNT(FILE_PREFIX "SG.DL1")) PROGRAM_EXIT_ERROR("Can't locate 'SG.DL1'");
	if (!VFS_MOUNT(FILE_PREFIX "WV.DL1")) {
		printf("Can't locate 'WV.DL1'\n");
	}
#endif
}

void lang_postinit() {
	GAME_MUSIC_STOP();
	lang_init();
	vfs_init();
	
	if (strcmp("JAPANESE", language) == 0) {
		char *font_data;
		int size;
#ifdef GAME_HOME_DIRECTORY
		char temp[512];
		snprintf(temp, sizeof(temp), "%s/jap.ttf", game_directory);
		SDL_RWops *f = SDL_RWFromFile(temp, "rb");
#else
		SDL_RWops *f = SDL_RWFromFile("jap.ttf", "rb");
#endif
		if (f) {
			size = SDL_RWseek(f, 0, SEEK_END);
			SDL_RWseek(f, 0, SEEK_SET);
			if (font_data = malloc(size)) {
				SDL_RWread(f, font_data, 1, size);
				if (font) TTF_CloseFont(font);
				font = TTF_OpenFontRW(SDL_RWFromMem(font_data, size), 1, font_height = font_size_corrected + 3);
				SDL_RWclose(f);
				is_shift_jis = 1;
				if (IMAGE_CACHE_MAX > 7) IMAGE_CACHE_MAX = 7;
			} else {
				printf("Can't allocate japanese font\n");
			}
		}
	} else {
		if (font) TTF_CloseFont(font);
		font = TTF_OpenFontRW(SDL_RWFromMem(font_ttf, size_font_ttf), 1, font_height = font_size_corrected);
		is_shift_jis = 0;
	}
	must_resume_music = 1;
}

void game_init() {
	int n;
	
	vfs_init();
	
	SYS_LOAD(); FLIST_LOAD();
	
	interface = GAME_IMAGE_GET_EX3("WAKU_P", NULL, 0, 0);

	SDL_SetColorKey(interface, 0, 0);
	
	for (n = 0; n < 4; n++) prepare_interface_image(&interface_main_buttons_clip[n], interface_main_buttons_images + n);
	for (n = 0; n < 3; n++) prepare_interface_image(&interface_title_clip[n], interface_title_images + n);
	for (n = 0; n < interface_next_count; n++) {	
		prepare_interface_image(&interface_next_clip[n], interface_next_images + n);
	}
}

int main(int argc, char* argv[]) 
{
	uint_fast8_t result_movie = 0;
#ifdef GAME_HOME_DIRECTORY
	if (argc < 2)
	{
		printf("Requires a path to a DL1 file\n");
		return 0;
	}
	snprintf(game_directory, sizeof(game_directory), "%s", dirname(argv[1]));
#endif
	
	MEMORY_DEBUG();
	
	ZERO(save);
	ZERO(save_temp);
	ZERO(save_s);
	ZERO(save_title);
	ZERO(gallery);
	ZERO(script);
	ZERO(script_options);
	ZERO(map_options);
	ZERO(image_cache);
	ZERO(joybut);
	ZERO(joy_keys);
	
	// menus.c
	ZERO(zzoptions);
	
	preinit();
	preinit();

	/*if (0) {
		lang_init();
		sdl_init();
		CREDIT_SHOW(1);
		return 0;
	}*/
	
	printf("+--------------------------------------------------------------------------\n");
	printf("| DIVI DEAD implementation @ soywiz (C) 2008 - RC3 build " __DATE__ "                \n");
	printf("+--------------------------------------------------------------------------\n");

	lang_init();
	sdl_init();
	
	printf("LANGUAGE: %s\n", language);
	#ifdef SJIS_SUPPORT
	if (strcmp("JAPANESE", language) == 0) {
		char *font_data;
		int size;
#ifdef GAME_HOME_DIRECTORY
		char temp[512];
		snprintf(temp, sizeof(temp), "%s/jap.ttf", game_directory);
		SDL_RWops *f = SDL_RWFromFile(temp, "rb");
#else
		SDL_RWops *f = SDL_RWFromFile("jap.ttf", "rb");
#endif
		if (f) {
			size = SDL_RWseek(f, 0, SEEK_END);
			SDL_RWseek(f, 0, SEEK_SET);
			if (font_data = malloc(size)) {
				SDL_RWread(f, font_data, 1, size);
				TTF_CloseFont(font);
				font = TTF_OpenFontRW(SDL_RWFromMem(font_data, size), 1, font_height = font_size_corrected + 3);
				SDL_RWclose(f);
				is_shift_jis = 1;
				if (IMAGE_CACHE_MAX > 7) IMAGE_CACHE_MAX = 7;
			} else {
				printf("Can't allocate japanese font\n");
			}
		}
	}
	#endif
	
	#ifdef DREAMCAST
		SDL_DC_MapKey(0, SDL_DC_R,     SDLK_LSHIFT); 
		SDL_DC_MapKey(0, SDL_DC_L,     SDLK_RSHIFT); 
		SDL_DC_MapKey(0, SDL_DC_A,     SDLK_RETURN);
		SDL_DC_MapKey(0, SDL_DC_X,     SDLK_RETURN);
		SDL_DC_MapKey(0, SDL_DC_Y,     SDLK_ESCAPE);
		SDL_DC_MapKey(0, SDL_DC_B,     SDLK_ESCAPE);
		SDL_DC_MapKey(0, SDL_DC_START, SDLK_SPACE);
		SDL_DC_MapKey(0, SDL_DC_UP,    SDLK_UP);
		SDL_DC_MapKey(0, SDL_DC_DOWN,  SDLK_DOWN);
		SDL_DC_MapKey(0, SDL_DC_LEFT,  SDLK_LEFT);
		SDL_DC_MapKey(0, SDL_DC_RIGHT, SDLK_RIGHT);
		
		SDL_JoystickEventState(SDL_ENABLE);
		SDL_DC_EmulateMouse(0);
		SDL_JoystickOpen(0);
	#elif defined(JOYSTICK_ENABLED)
		SDL_JoystickEventState(SDL_ENABLE);
		SDL_JoystickOpen(0);
	#endif
	
	#ifdef INTRINSIC_CHARACTERS
		ICHAR_load_process();
	#endif
	
	game_init();

	printf("MOVIE_START();\n");
	MOVIE_START();
#ifdef GAME_HOME_DIRECTORY
	char cs_robo_path[512], opening_path[512];
	snprintf(cs_robo_path, sizeof(cs_robo_path), "%s/%s", game_directory, "CS_ROGO"VIDEO_EXTENSION);
	
	result_movie = MOVIE_PLAY(cs_robo_path, 1);
	if (!result_movie)
	{
		snprintf(cs_robo_path, sizeof(cs_robo_path), "%s/%s", game_directory, "cs_rogo"VIDEO_EXTENSION_LOWERCASE);
		result_movie = MOVIE_PLAY(cs_robo_path, 1);
	}
	
	snprintf(opening_path, sizeof(opening_path), "%s/%s", game_directory, "OPEN"VIDEO_EXTENSION);
	result_movie = MOVIE_PLAY(opening_path, 1);
	if (!result_movie)
	{
		snprintf(opening_path, sizeof(opening_path), "%s/%s", game_directory, "open"VIDEO_EXTENSION_LOWERCASE);
		result_movie = MOVIE_PLAY(opening_path, 1);
	}
#else
	MOVIE_PLAY(FILE_PREFIX "CS_ROGO.MPG", 1);
	MOVIE_PLAY(FILE_PREFIX "OPEN.MPG", 1);
#endif
	MOVIE_END();
	printf("MOVIE_END();\n");
	
#ifdef ENABLE_VIDEO_ROQ
	SDL_Audio_Init();
#endif

	KEYS_CLEAR();
	
	game_end = 1;
	
	MEMORY_DEBUG();
	
	while (1) {
		if (keys & K_A) keys |= K_L;
		if (keys & K_B) keys |= K_L;
		if (CHECK_MENU_KEYS()) { KEYS_UPDATE(); continue; }
		if (game_start) {
			SDL_FillRect(screen, NULL, 0);
			GAME_BUFFER_REPAINT(0);
			
			printf("GAME START\n");
			game_start = 0;
			GAME_SCRIPT_PROCESS();
		}
		
		if (game_end) {
			printf("Start menu...\n");
			game_end = 0;
			GAME_MUSIC_PLAY("opening");
			GAME_BACKGROUND_O("WAKU_A1");
			GAME_BACKGROUND_I("TITLE");
			GAME_BUFFER_REPAINT(-10);
			memset(&save, 0, sizeof(save));
		}
		
		PROGRAM_DELAY(6);
		KEYS_UPDATE();
	}
		
	SDL_Quit();
	
	return 0;
}
