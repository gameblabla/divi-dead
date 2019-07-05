////////////////////////////////////////////////////////////
// INPUT                                                  //
////////////////////////////////////////////////////////////

#include "input.h"

void GAME_INPUT_WAIT(char *text) {
	int tick = 0;
	int tick_start = 4;
	
	int marked_as_conflictive = 0;
	
	GAME_UPDATE_DEBUG_INFO();
	
	while (game_playing) {
		PROGRAM_DELAY(12);
		KEYS_UPDATE();
		GAME_SOUND_GC_COLLECT();
		
		if (keys & K_SPECIAL) {
			if (debug_global) {
				if (!marked_as_conflictive) {
					char path[0x100];
					FILE *f;
					sprintf(path, SAVE_ROOT "/DATA/conflictive.txt");
					message_info("MARKED AS CONFLICTIVE               ");
					marked_as_conflictive = 1;
					if ((f = fopen(path, "a+b")) != NULL) {
						fprintf(f, "%s:%s\n", save.script, text);
						fclose(f);
						printf("CONFLICTIVE:%s:%s\n", save.script, text);
					} else {
						printf("Can't open conflictive.txt for appending\n");
					}
				}
			}
			KEYS_CLEAR();
		}
		
		//if (keys & K_B) keys |= K_L;
		if (keys & K_B) keys |= K_A;

		if (CHECK_MENU_KEYS()) continue;		
		
		keys &= ~(K_LEFT | K_RIGHT | K_UP | K_DOWN | K_MODE);
		
		if (keys) break;
		if (tick > tick_start) {
			SDL_BlitSurface(interface_next_images[((tick - tick_start) / 10) % interface_next_count], NULL, screen, &interface_next_pos);
			GAME_SCREEN_UPDATE_RECT(screen, &interface_next_pos);
		}
		tick++;
	}
	
	GAME_TEXT_CLEAR();
	GAME_TEXT_UPDATE();
	
	GAME_VOICE_STOP();
}

void GAME_INPUT_OPTIONS_DRAW(int sel) {
	int n, x, y;
	GAME_TEXT_CLEAR();
	for (n = x = 0; x < 2; x++) {
		for (y = 0; y < 4; y++, n++) {
			SDL_Rect clip;
			if (n >= script_options_count) continue;
			clip = clip_text;
			clip.w /= 2;
			clip.x += clip.w * x;
			if (n == sel) {
				SDL_Rect rect;
				rect.x = GAME_TEXT_LINE_X();
				rect.y = GAME_TEXT_LINE_Y(y) - text_pos.h / 2;
				rect.w = (clip_text.w - text_pos.x) / 2;
				rect.h = font_height + text_pos.h;
				rect.x += rect.w * x;
				SDL_FillRect(screen, &rect, 0xFFFFFFFF);
			}
			SDL_Rect clip2 = clip;
			clip2.w += 80;
			GAME_TEXT_PRINT_LINE(script_options[n].text, (n == sel) ? black : white, (n == sel) ? white : black, clip2, y);
		}
	}
	GAME_TEXT_UPDATE();
}

void GAME_INPUT_OPTIONS() {
	int n = 0, update = 1;
	
	printf("%08X\n", save.cursor1);
	
	printf("map_options_count:%d\n", script_options_count);
	
	n = (save.cursor1 & 0x7fff) % script_options_count;
	if (n < 0) n = 0;
	
	GAME_UPDATE_DEBUG_INFO();
	
	while (game_playing) {
		KEYS_UPDATE();
		GAME_SOUND_GC_COLLECT();
		
		//if (keys & K_B) keys |= K_L;
		if (CHECK_MENU_KEYS()) continue;
		
		// up
		if ((keys & K_UP)) {
			n--; if (n < 0) n = script_options_count - 1;
			update = 1;
		}
		// down
		else if ((keys & K_DOWN)) {
			n++;
			if (n >= script_options_count) n = 0;
			update = 1;
		}
		// horizontal
		else if ((keys & K_LEFT) || (keys & K_RIGHT)) {
			if (n - 4 >= 0) {
				n -= 4;
				update = 1;
			} else if (n + 4 < script_options_count) {
				n += 4;
				update = 1;
			}
		}
		// accept
		else if (keys & K_A) {
			printf("OPTION|JUMP: 0x%04X\n", script_options[n].ptr);
			GAME_SCRIPT_JUMP(script_options[n].ptr);
			break;
		}
		
		// we should update?
		if (update) {
			//printf("OPTION: %d\n", n);
			update = 0;
			save.cursor1 = n | 0x8000;
			GAME_INPUT_OPTIONS_DRAW(n);
		}
		
		PROGRAM_DELAY(5);
	}
	
	if (game_playing) save.cursor1 &= 0x7fff;
	
	printf("%08X\n", save.cursor1);
	
	GAME_TEXT_CLEAR();
	GAME_TEXT_UPDATE();
	
	GAME_VOICE_STOP();
}

void GAME_INPUT_MAP_OPTIONS() {
	int n, update = 1;
	int refresh_images = 1;
	
	GAME_UPDATE_DEBUG_INFO();
	
	n = save.cursor2 % map_options_count;
	
	SDL_Surface *off = NULL;
	SDL_Surface *on = NULL;
	
	while (game_playing) {
		KEYS_UPDATE();
		GAME_SOUND_GC_COLLECT();
		
		if (refresh_images) {
			off = GAME_IMAGE_GET(map_images[0]);
			on  = GAME_IMAGE_GET(map_images[1]);
			refresh_images = 0;
		}
		
		//if (keys & K_B) keys |= K_L;
		if (CHECK_MENU_KEYS()) {
			refresh_images = 1;
			continue;
		}
		
		if ((keys & K_UP)) {
			n--;
			if (n < 0) n += map_options_count;
			update = 1;
		}
		if ((keys & K_DOWN)) {
			n++;
			if (n >= map_options_count) n -= map_options_count;
			update = 1;
		}
		
		if (keys & K_A) {
			printf("MAP_OPTION|JUMP: 0x%04X\n", map_options[n].ptr);
			GAME_SCRIPT_JUMP(map_options[n].ptr);
			break;
		}		
		
		if (update) {
			MAP_OPTION *copt = &map_options[n];
			SDL_Rect from;
			SDL_Rect to;

			save.cursor2 = n;

			from.x = ACORRECT_X(copt->x1);
			from.y = ACORRECT_Y(copt->y1);
			from.w = ACORRECT_X(copt->x2 - copt->x1);
			from.h = ACORRECT_Y(copt->y2 - copt->y1);
			
			to = from;
			to.x += clip_bg2.x;
			to.y += clip_bg2.y;
			
			update = 0;
			SDL_BlitSurface(off, NULL, screen, &clip_bg2);
			SDL_BlitSurface(on, &from, screen, &to);
			
			GAME_BUFFER_REPAINT(0);
		}
		
		PROGRAM_DELAY(5);
	}
	
	GAME_TEXT_CLEAR();
	GAME_TEXT_UPDATE();
	
	GAME_VOICE_STOP();
	//if (on) SDL_FreeSurface(on);
	//if (off) SDL_FreeSurface(off);
}
