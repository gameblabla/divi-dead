#ifndef MAIN_H
#define MAIN_H

#include "shared.h"

#if defined(ENABLE_VIDEO_ROQ)
#define VIDEO_EXTENSION ".ROQ"
#define VIDEO_EXTENSION_LOWERCASE ".roq"
#elif defined(ENABLE_VIDEO_SMPEG)
#define VIDEO_EXTENSION ".MPG"
#define VIDEO_EXTENSION_LOWERCASE ".mpg"
#endif

extern SDL_mutex *mutex_reading;

extern void READING_START();
extern void READING_END  ();
extern void GAME_BUFFER_REPAINT(int effect);

extern void GAME_SCREEN_UPDATE_RECT(SDL_Surface *from, SDL_Rect *rect);
extern void GAME_SCREEN_UPDATE(SDL_Surface *from);
extern void GAME_UPDATE_DEBUG_INFO();

extern void KEYS_UPDATE();
extern void KEYS_CLEAR();

extern int joy_check_combination_basic_just(int *ptr);
extern int joy_check_combination_basic(int *ptr);

extern void joy_push_keys_pd(int v);
extern void joy_push_keys(int v);
extern int joy_keys_get(int pos);

extern void message_info(char *format, ...);

extern void PROGRAM_EXIT(int v);
extern void PROGRAM_DELAY(uint32_t milli);
extern void PROGRAM_EXIT_ERROR(char *format, ...);
extern int process_exit(const SDL_Event *event);
extern void text_at_base(SDL_Color c_front, SDL_Color c_back, int x, int y, char *buffer, int update);

extern void lang_postinit();

#define EFFECT_CREDITS

#define SJIS_SUPPORT

#define ZERO(s) memset(&s, 0, sizeof(s));


#define AUDIO_CHECK_MOD

//#define GAME_DEBUG
//#define CHECK_FILESYSTEM
//#define EMULATE_PSP

//#define ENABLE_VIDEO_SMPEG

//#define LANGUAGE_DEFAULT "SPANISH"
//#define LANGUAGE_DEFAULT "JAPANESE"
#define LANGUAGE_DEFAULT "ENGLISH"
#define LANGUAGE_DEFAULT_UNDERSCORE "english"
#define ENABLE_MUSIC 1

#define SAVE_ROOT "."
#define SNAP_ROOT "."
#define ALTERNATE_LANG "LANG"
#define ALTERNATE_LANG_UNDERSCORE "lang"

#define ORI_SCREEN_WIDTH  640
#define ORI_SCREEN_HEIGHT 480

#define IMAGE_CACHE_MAX_DEFAULT 15

#define text_margin_top 0

//#define KEY_REPEAT_DELAY1 200
//#define KEY_REPEAT_DELAY2 40

#define KEY_REPEAT_DELAY1 420
#define KEY_REPEAT_DELAY2 50

#define ASK_NO      0
#define ASK_YES     1
#define ASK_CANCEL -1

#define SAVE_FULL 1
//#define SAVE_FULL 0

#define font_size_corrected ACORRECT_Y(font_size)

#define ACORRECT_X(v) iround((((double)(v)) * (double)SCREEN_WIDTH)  / (double)ORI_SCREEN_WIDTH)
#define ACORRECT_Y(v) iround((((double)(v)) * (double)SCREEN_HEIGHT) / (double)ORI_SCREEN_HEIGHT)

#define P_D(k) +(k), -(k)

#define ASPECT_X ((double)SCREEN_WIDTH / (double)ORI_SCREEN_WIDTH)
#define ASPECT_Y ((double)SCREEN_HEIGHT / (double)ORI_SCREEN_HEIGHT)
#define SCREEN_MUST_SCALE ((SCREEN_WIDTH != ORI_SCREEN_WIDTH) || (SCREEN_HEIGHT != ORI_SCREEN_HEIGHT))


#define MEMORY_DEBUG()
#define MEMORY_DEBUG2()

#define MOVIE_START()
#define MOVIE_END()  


#define BIT(n) (1 << (n))

#define K_UP      BIT(0)
#define K_DOWN    BIT(1)
#define K_LEFT    BIT(2)
#define K_RIGHT   BIT(3)
#define K_A       BIT(4)
#define K_B       BIT(5)
#define K_L       BIT(6)
#define K_R       BIT(7)
#define K_MODE    BIT(8)
#define K_SPECIAL BIT(9)

extern int toggle_info;
extern int initialized;
extern int menu_mode;
extern int audio_initialized;
extern int debug_global;
extern int hide_debug;
extern int must_resume_music;

extern int audio_voice_enabled;
extern int audio_music_enabled;

extern int is_shift_jis;

extern int volume_music;
extern int volume_effect;
extern int volume_voice;
extern int volume_movie;

extern SDL_Color white ;
extern SDL_Color red;
extern SDL_Color grey;
extern SDL_Color blue;
extern SDL_Color green;
extern SDL_Color black;

extern Mix_Music *music;
extern Mix_Chunk *effect;
extern Mix_Chunk *voice_mix;
extern Mix_Chunk *click;
extern TTF_Font *font;
extern int font_height;
extern TTF_Font *font2;
extern int font2_height;

extern SDL_Surface *screen, *screen_video, *interface;
extern SDL_PixelFormat screen_format;

extern int done, game_end, swap_buttons;
extern char language[0x30];
extern char lang_texts[12][0x30];

extern int font_size;

extern CG_PAGE cg_pages[6];
extern int image_cache_pos;

extern char save_title[0x10];

// FLIST
extern char gallery[150][0x10];

extern int game_playing;
extern int game_start;

extern int finish_game;

extern uint32_t keys;

extern uint8_t  script[0x10000];
extern uint8_t *scrp;

extern OPTION script_options[0x100];
extern int script_options_count;

extern MAP_OPTION map_options[0x100];
extern int map_options_count;
extern char map_images[2][0x20];

extern uint8_t  mem1(uint8_t *ptr);
extern uint16_t mem2(uint8_t *ptr);
extern uint32_t   mem4(uint8_t *ptr);

extern int iround(double r);

struct save_struct_game {
	char script[0x20];
	char background_o[0x20];
	char background_i[0x20];
	char background_v[0x20];
	char character_1[0x20];
	char character_2[0x20];
	char unknown[0x20];
	char music[0x20];
	uint16_t cursor1;
	uint16_t cursor2;
	uint32_t script_pos;
	uint32_t script_menu_return;
	uint32_t flags[1000];
};

struct save_gallery {
	char names[10][0x19];
	uint16_t unk[110];
	uint16_t gallery[152];
	uint16_t unk2[1750];
};

extern struct save_struct_game save;
extern struct save_struct_game save_temp;
extern struct save_gallery save_s;

extern int debug_combination[6];

extern int IMAGE_CACHE_MAX;
extern IMAGE_CACHE image_cache[32];

extern char game_directory[512];

#endif
