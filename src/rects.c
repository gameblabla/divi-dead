////////////////////////////////////////////////////////////
// RECTS                                                  //
////////////////////////////////////////////////////////////

#include "rects.h"

int menu_button_pos = 0, menu_button_type = 0;

////////////////////////////////////////////////////////////
// INTERFACE CLIPS                                        //
////////////////////////////////////////////////////////////

SDL_Rect interface_title_clip[3] = {
	{ 0,  0, 240, 44 },
	{ 0, 42, 240,  8 },
	{ 0, 52, 240, 28 },
};

SDL_Rect interface_main_buttons_clip[4] = {
	{ 64 * 0, 80, 64, 64 },
	{ 64 * 1, 80, 64, 64 },
	{ 64 * 2, 80, 64, 64 },
	{ 64 * 3, 80, 64, 64 },
};

SDL_Rect interface_next_clip[9] = {
	{ 18 * 0, 144, 18, 18 },
	{ 18 * 1, 144, 18, 18 },
	{ 18 * 2, 144, 18, 18 },
	{ 18 * 3, 144, 18, 18 },
	{ 18 * 4, 144, 18, 18 },
	{ 18 * 5, 144, 18, 18 },
	{ 18 * 6, 144, 18, 18 },
	{ 18 * 7, 144, 18, 18 },
	{ 18 * 8, 144, 18, 18 },
	{ 18 * 8, 144, 18, 18 },
	{ 18 * 8, 144, 18, 18 },
};

SDL_Rect interface_player_clip[3] = {
	{ 0, 162 + 80 * 0, 152, 80 },
	{ 0, 162 + 80 * 1, 152, 80 },
	{ 0, 162 + 80 * 2, 152, 80 },
};


SDL_Surface *interface_title_images[3] = {0};
SDL_Surface *interface_main_buttons_images[4] = {0};
SDL_Surface *interface_next_images[interface_next_count] = {0};

////////////////////////////////////////////////////////////
// GLOBAL CLIPS                                           //
////////////////////////////////////////////////////////////

SDL_Rect clip_bg1  = {   0,   0, 640, 480 };
SDL_Rect clip_bg2  = {  32,   8, 576, 376 };
SDL_Rect clip_text = {  96, 392, 448,  80 };
SDL_Rect clip;

////////////////////////////////////////////////////////////
// POSITIONS                                              //
////////////////////////////////////////////////////////////

SDL_Rect interface_main_buttons_pos[2] = {
	{        0 + 16, 480 - 64 - 16, 64, 64 },
	{ 640 - 64 - 16, 480 - 64 - 16, 64, 64 },
};

SDL_Rect interface_next_pos = { 513, 447, 18, 18 };

SDL_Rect menus_pos = { 48, 16, 64, 16 };
SDL_Rect menus_option = { 12, 0, 217, 6 };
SDL_Rect menu_gallery_base = { 8, 4, 112, 74 };

SDL_Rect text_pos = { 10, 6, 0, 2 + text_margin_top };

SDL_Rect *update_rects[10] = {
	&interface_main_buttons_pos[0],
	&interface_main_buttons_pos[1],

	&interface_next_pos,

	&clip_bg1,
	&clip_bg2,
	&clip_text,
	
	&menus_pos,
	&menus_option,
	&menu_gallery_base,
	&text_pos,
};

