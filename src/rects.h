#ifndef RECTS_H
#define RECTS_H

#include "shared.h"

extern int menu_button_pos, menu_button_type;

extern SDL_Rect interface_title_clip[3];
extern SDL_Rect interface_main_buttons_clip[4];
extern SDL_Rect interface_next_clip[11];
extern SDL_Rect interface_player_clip[3];

extern SDL_Surface *interface_title_images[3];
extern SDL_Surface *interface_main_buttons_images[4];

extern SDL_Rect clip_bg1, clip_bg2, clip_text, clip;

extern SDL_Rect interface_main_buttons_pos[2];
extern SDL_Rect interface_next_pos;

extern SDL_Rect menus_pos;
extern SDL_Rect menus_option;
extern SDL_Rect menu_gallery_base;

extern SDL_Rect text_pos;

extern SDL_Rect *update_rects[10];

#define interface_next_count (sizeof(interface_next_clip) / sizeof(interface_next_clip[0]))
#define update_rects_count (sizeof(update_rects) / sizeof(update_rects[0]))

extern SDL_Surface *interface_next_images[interface_next_count];

#endif
