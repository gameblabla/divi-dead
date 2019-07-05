#ifndef MENUS_H
#define MENUS_H

#include "shared.h"
#include "structures.h"

extern SDL_Surface *snapshot_first;
extern SDL_Surface *snapshot_last;

void fadeOutTick(SDL_Surface *screen, int step);
extern int GAME_MENU_SHOW_EX(OPTION_GAME_MENU *menu, int menu_level);
extern int GAME_MENU_SHOW(OPTION_GAME_MENU *menu);

extern int save_load_action;
extern int SAVE_LOAD_OPTION(int n);
extern OPTION_GAME_MENU main_menu_save_load[11];
extern int MAIN_MENU_GAME_START(int p);
extern int MAIN_MENU_GAME_LOAD(int p);
extern int MAIN_MENU_GAME_SAVE(int p);
extern int MAIN_MENU_GAME_EXIT(int p);
extern int OPTION_DUMMY(int p);
extern void SDL_DrawRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color, int border);

extern OPTION_GAME_MENU extra_menu_gallery[2];

extern void OPTION_GALLERY_SHOW(char *name);
extern int OPTION_DEBUG_RESTART_ROOM(int g);

extern int check_name_aa(int idx);
extern int check_name_aa_find(int idx, int dir, int count);
extern int OPTION_JUMP_ROOM(int g);
extern int OPTION_DEBUG_EDIT_FLAGS(int g);
extern int OPTION_GALLERY(int g);
extern int OPTION_SAVE_IMAGE(int n);
extern int OPTION_CREDITS(int n);

extern int MAIN_MENU_GAME_OPTIONS_VOICE(int p);
extern int MAIN_MENU_GAME_OPTIONS_MUSIC(int p);

extern char zzoptions[2][0x30];

extern OPTION_GAME_MENU main_menu_options[3];

extern void updateOptionsTexts(int updated);

extern int MAIN_MENU_GAME_OPTIONS_VOICE(int p);
extern int MAIN_MENU_GAME_OPTIONS_MUSIC(int p);
extern int MAIN_MENU_GAME_OPTIONS(int p);

extern int OPTION_SELECT_LANG(int p);

extern OPTION_GAME_MENU main_menu_langs[7];

extern int MAIN_MENU_GAME_LANG(int p);

extern OPTION_GAME_MENU main_menu_ingame[6];
extern OPTION_GAME_MENU main_menu_title[6];

extern char percent_option[0x30];

extern OPTION_GAME_MENU extra_menu_title_nodebug[3];
extern OPTION_GAME_MENU extra_menu_title_debug[7];

extern void MAIN_MENU_SHOW();
extern void EXTRA_MENU_SHOW();
extern int CHECK_MENU_KEYS_EX();
extern int CHECK_MENU_KEYS();

#endif
