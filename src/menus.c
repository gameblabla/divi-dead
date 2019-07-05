#include "menus.h"

SDL_Surface *snapshot_first = NULL;
SDL_Surface *snapshot_last = NULL;

void fadeOutTick(SDL_Surface *screen, int step) {
	/*
	int v[4];
	int len = screen->w * screen->h;
	uint32_t *ptr = screen->pixels;
	if (0) {
		processSurface(screen, NULL, fadeOutPixel, 0x10);
	} else{
		SDL_LockSurface(screen);
		for (; len; len--, ptr++) {
			PROCESS_V(0); PROCESS_V(1); PROCESS_V(2); PROCESS_V(3);
			*ptr = PROCESS_I(0) | PROCESS_I(1) | PROCESS_I(2) | PROCESS_I(3);
		}
		SDL_UnlockSurface(screen);
	}
	*/
	
	if (step == 0x100) {
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
	}
}

int GAME_MENU_SHOW_EX(OPTION_GAME_MENU *menu, int menu_level) {
	SDL_Rect rect_start = menus_pos;
	SDL_Rect pos;
	SDL_Rect spos;
	int retval = 0;
	int tw, th;
	int zpos = 0;
	int update = 1;
	int n;
	int lines = 0;
	int lines_h = font_height + menus_option.h;
	int height;
	SDL_Surface *snapshot;
	
	printf("GAME_MENU_SHOW_EX:0\n");
	
	KEYS_CLEAR();
	
	rect_start.w = SCREEN_WIDTH;
	rect_start.h = SCREEN_HEIGHT;

	rect_start.x += menu_level * menus_pos.w;
	rect_start.y += menu_level * menus_pos.h;
	
	printf("GAME_MENU_SHOW_EX:1\n");
	
	snapshot = SDL_CreateRGBSurface(SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, screen_format.BitsPerPixel, screen_format.Rmask, screen_format.Gmask, screen_format.Bmask, screen_format.Amask);
	SDL_BlitSurface(screen, NULL, snapshot, NULL);
	if (menu_level == 0) snapshot_first = snapshot;
	snapshot_last = snapshot;
	
	//printf("GAME_MENU_SHOW_EX:2\n");
	
	while (menu[lines].text) lines++;
	
	height = lines * lines_h;
	
	SDL_BlitSurface(snapshot, NULL, screen, NULL);

	fadeOutTick(screen, 0x10);
	
	//printf("GAME_MENU_SHOW_EX:3\n");
	
	SDL_BlitSurface(interface_main_buttons_images[menu_button_type * 2 + menu_button_pos], NULL, screen, &interface_main_buttons_pos[menu_button_pos]);
	
	//printf("GAME_MENU_SHOW_EX:4\n");
	
	while (1) {
		KEYS_UPDATE();
		
		if (keys & K_B) {
			//printf("GAME_MENU_SHOW_EX:K_B (cancel)\n");
			break;
		}
		
		if (keys & K_MODE) {
			//printf("GAME_MENU_SHOW_EX:mode\n");
			menu_mode = ((keys & K_MODE) != 0);
		}
		
		if (joy_check_combination_basic_just(debug_combination)) {
			message_info("DEBUG_MODE +ON");
			debug_global = 1;
			toggle_info = 1;
		}

		//if ((keys & K_A) || (keys & K_MODE)) {
		if ((keys & K_A)) {
			//printf("GAME_MENU_SHOW_EX: Key A\n");
			menu_mode = ((keys & K_MODE) != 0);
			//printf("GAME_MENU_SHOW_EX: menu mode\n");
			if (!menu[zpos].callback) break;
			//printf("GAME_MENU_SHOW_EX: Valid callbackA\n");
			if ((retval = menu[zpos].callback(zpos)) != 0) {
				//printf("GAME_MENU_SHOW_EX: Return != 0 break\n");
				break;
			}
			//printf("GAME_MENU_SHOW_EX: End callback\n");
			update = 1;
		}
		
		if (keys & K_UP) {
			//printf("GAME_MENU_SHOW_EX:up\n");
			zpos--;
			if (zpos < 0) zpos += lines;
			update = 1;
		}
		if (keys & K_DOWN) {
			//printf("GAME_MENU_SHOW_EX:down\n");
			zpos++;
			if (zpos >= lines) zpos -= lines;
			update = 1;
		}
		
		if (update) {
			int theight = height;
			pos = rect_start;
			
			//printf("GAME_MENU_SHOW_EX:UPDATE:0\n");

			SDL_BlitSurface(interface_title_images[0], NULL, screen, &pos); pos.y += interface_title_images[0]->h;
			
			//printf("GAME_MENU_SHOW_EX:UPDATE:1\n");
			
			spos = pos;
			spos.y += 4;
			
			//printf("GAME_MENU_SHOW_EX:UPDATE:2\n");
			
			while (theight > 0) {
				SDL_BlitSurface(interface_title_images[1], NULL, screen, &pos); pos.y += interface_title_images[1]->h;
				theight -= interface_title_images[1]->h;
			}
			
			//printf("GAME_MENU_SHOW_EX:UPDATE:3\n");
			
			SDL_BlitSurface(interface_title_images[2], NULL, screen, &pos); pos.y += interface_title_images[2]->h;
			
			//printf("GAME_MENU_SHOW_EX:UPDATE:4\n");
			
			for (n = 0; n < lines; n++) {
				SDL_Rect cpos = spos;
				char *text = menu[n].text;
				
				if (text == NULL) {
					text = "void";
				} else {
					//text[0x18] = 0;
				}
				
				if (strlen(text) > 0x20) text = "too long";
				if (strlen(text) == 0) text = "Zero Length";

				/*
				printf("GAME_MENU_SHOW_EX:UPDATE:LINE:%d\n", n);
				printf("GAME_MENU_SHOW_EX:UPDATE:TEXT:%s\n", text);
				printf("GAME_MENU_SHOW_EX:UPDATE:LINE:... (0)\n");
				*/
				
				TextSizeEncoding(font, text, &tw, &th);
				cpos.x += interface_title_images[0]->w / 2 - tw / 2;
				cpos.y += lines_h / 2 - th / 2;
				
				//printf("GAME_MENU_SHOW_EX:UPDATE:LINE:... (1)\n");

				if (zpos == n) {
					SDL_Rect rect;
					rect.x = pos.x + menus_option.x;
					rect.y = cpos.y - menus_option.h / 2 - text_margin_top;
					rect.h = lines_h - menus_option.h / 2;
					rect.w = menus_option.w;
					SDL_FillRect(screen, &rect, 0xFFFFFFFF);
				}
				
				//printf("GAME_MENU_SHOW_EX:UPDATE:LINE:... (2)\n");
				
				GAME_TEXT_PRINT2_EX(text, (zpos == n) ? black : white, (zpos != n) ? black : white, cpos, 1);
				spos.y += lines_h;
				
				//printf("GAME_MENU_SHOW_EX:UPDATE:LINE:... (3)\n");
			}
			
			//printf("GAME_MENU_SHOW_EX:UPDATE:5\n");
		
			//SDL_Flip(screen);
			GAME_BUFFER_REPAINT(0);
			update = 0;
		}
		PROGRAM_DELAY(6);
	}
	
	//printf("GAME_MENU_SHOW_EX:end1\n");
	
	SDL_BlitSurface(snapshot, NULL, screen, NULL);
	SDL_FreeSurface(snapshot); snapshot = NULL;	
	
	//printf("GAME_MENU_SHOW_EX:end2\n");
	
	//SDL_Flip(screen);
	GAME_BUFFER_REPAINT(0);
	
	//printf("GAME_MENU_SHOW_EX:end3\n");
	
	KEYS_CLEAR();
	
	return retval;
}

int GAME_MENU_SHOW(OPTION_GAME_MENU *menu) {
	GAME_MENU_SHOW_EX(menu, 0);
	return 0;
}

int save_load_action = 0;

int SAVE_LOAD_OPTION(int n) {
	if (save_load_action) {
		printf("save: %d\n", n);
		GAME_SAVE(n);
	} else {
		printf("load: %d\n", n);
		if (GAME_LOAD(n)) {
			game_playing = 0;
			game_start = 1;
			// Restart script
			if (menu_mode) scrp = script;
		}
	}
	
	//PROGRAM_DELAY(22);
	
	return 1;
}

OPTION_GAME_MENU main_menu_save_load[11] = {
	{ save_s.names[0], SAVE_LOAD_OPTION},
	{ save_s.names[1], SAVE_LOAD_OPTION},
	{ save_s.names[2], SAVE_LOAD_OPTION},
	{ save_s.names[3], SAVE_LOAD_OPTION},
	{ save_s.names[4], SAVE_LOAD_OPTION},
	{ save_s.names[5], SAVE_LOAD_OPTION},
	{ save_s.names[6], SAVE_LOAD_OPTION},
	{ save_s.names[7], SAVE_LOAD_OPTION},
	{ save_s.names[8], SAVE_LOAD_OPTION},
	{ save_s.names[9], SAVE_LOAD_OPTION},
	{ NULL           , NULL },
};

int MAIN_MENU_GAME_START(int p) {
	if (game_playing && ask(lang_texts[7], ASK_YES) != ASK_YES) return 0;
	
	memset(&save, 0, sizeof(save));
	
	game_playing = 0;
	game_start = 1;
	GAME_SCRIPT_LOAD("AASTART");
	
	//GAME_SCRIPT_LOAD("charatest");
	
	//GAME_SCRIPT_LOAD("F777");
	//GAME_SCRIPT_LOAD("F999_2");
	//GAME_SCRIPT_LOAD("F0_102");
	return 1;
}

int MAIN_MENU_GAME_LOAD(int p) {
	int r;
	menu_button_pos = 0;
	save_load_action = 0;
	printf("MAIN_MENU_GAME_LOAD:0\n");
	r = GAME_MENU_SHOW_EX(main_menu_save_load, 1);	
	printf("MAIN_MENU_GAME_LOAD:1\n");
	return r;
}

int MAIN_MENU_GAME_SAVE(int p) {
	int r;
	save_load_action = 1;
	menu_button_pos = 0;
	printf("MAIN_MENU_GAME_SAVE:0\n");
	r = GAME_MENU_SHOW_EX(main_menu_save_load, 1);
	printf("MAIN_MENU_GAME_SAVE:1\n");
	return r;
}

int MAIN_MENU_GAME_EXIT(int p) {
	printf("exit\n");
	Mix_FadeOutMusic(1000);
	GAME_BUFFER_FADEOUT_EX(0x18);
	GAME_MUSIC_STOP();
	PROGRAM_EXIT(0);
	return 0;
}

int OPTION_DUMMY(int p) {
	return 0;
}

void SDL_DrawRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color, int border) {
	SDL_Rect rect;
	
	rect = *dstrect;
	rect.h = border;
	SDL_FillRect(dst, &rect, color);
	rect.y += dstrect->h - border;
	SDL_FillRect(dst, &rect, color);

	rect = *dstrect;
	rect.w = border;
	SDL_FillRect(dst, &rect, color);
	rect.x += dstrect->w - border;
	SDL_FillRect(dst, &rect, color);
}

OPTION_GAME_MENU extra_menu_gallery[2] = {
	{ lang_texts[6], OPTION_SAVE_IMAGE },
	{ NULL            , NULL },
};

void OPTION_GALLERY_SHOW(char *name) {
	char rname[128];
	
	sprintf(rname, "%s.BMP", name);

	FSLI *f;
	
	f = VFS_FIND(rname);

	if (f == NULL) return;

	printf("Showing '%s'\n", name);

	GAME_BACKGROUND_I(rname);
	GAME_BUFFER_REPAINT(0);
	
	while (1) {
		KEYS_UPDATE();
		
		if (keys & K_R) {
			GAME_MENU_SHOW_EX(extra_menu_gallery, 1);
			continue;
		}
		
		if (keys) break;
		PROGRAM_DELAY(6);
	}
}

int OPTION_DEBUG_RESTART_ROOM(int g) {
	if (!game_playing) {
		fadeOutTick(screen, 0x100);
		GAME_BUFFER_REPAINT(0);
		message_info("Game not started                    ");
		PROGRAM_DELAY(2000);
		return 0;
	}

	scrp = script;
	game_playing = 0;
	game_start = 1;
	save.script_pos = 0;

	KEYS_CLEAR();
	
	return 1;
}

int check_name_aa(int idx) {
	char *name = files[idx %= files_count].name;
	int len = strlen(name);
	if (stricmp(name + len - 3, ".AB") == 0) {
		return 1;
	}
	return 0;
}

int check_name_aa_find(int idx, int dir, int count) {
	int n;
	if (dir == 0) {
		for (n = idx - 1; n >= 0; n--) {
			if (!check_name_aa(n)) continue;
			idx = n;
			if (--count <= 0) break;
		}	
	} else {
		for (n = idx + 1; n < files_count; n++) {
			if (!check_name_aa(n)) continue;
			idx = n;
			if (--count <= 0) break;
		}	
	}
	
	return idx;
}

int OPTION_JUMP_ROOM(int g) {
	int update = 1;
	int idx = 0, n;
	char name[0x20] = {};
	
	if (!game_playing) {
		fadeOutTick(screen, 0x100);
		GAME_BUFFER_REPAINT(0);
		message_info("Game not started                    ");
		PROGRAM_DELAY(2000);
		return 0;
	}	
	
	//
	fadeOutTick(screen, 0x100);
	idx = check_name_aa_find(-1, 1, 1);
	
	sprintf(name, "%s.AB", save.script);
	for (n = 0; n < files_count; n++) {
		//printf("%s, %s\n", name, files[idx].name);
		if (stricmp(name, files[n].name) == 0) {
			idx = n;
			break;
		}
	}
	
	GAME_BUFFER_REPAINT(0);	
	
	KEYS_CLEAR();
	
	while (1) {
		KEYS_UPDATE();
		if (keys & K_B) return 0;
		
		if (keys & K_A) {
			strcpy(name, files[idx].name);
			for (n = 0; n < 0x20; n++) if (name[n] == '.') name[n] = 0;
			break;
		}
		
		if (keys & K_UP) {
			idx = check_name_aa_find(idx, 0, 1);
			update = 1;
		}

		if (keys & K_LEFT) {
			idx = check_name_aa_find(idx, 0, 10);
			update = 1;
		}

		if (keys & K_DOWN) {
			idx = check_name_aa_find(idx, 1, 1);
			update = 1;
		}
		
		if (keys & K_RIGHT) {
			idx = check_name_aa_find(idx, 1, 10);
			update = 1;
		}
		
		if (update) {
			text_at(white, 50, 50, "%03d: %s            ", idx, files[idx].name);
			update = 0;
			SDL_Flip(screen_video); GAME_UPDATE_DEBUG_INFO();
		}
	}
	
	GAME_SCRIPT_LOAD(name);

	game_playing = 0;
	game_start = 1;
	save.script_pos = 0;
	
	save.background_o[0] = 0;
	save.background_i[0] = 0;
	save.background_v[0] = 0;
	save.character_1[0] = 0;
	save.character_2[0] = 0;
	save.unknown[0] = 0;
	save.music[0] = 0;

	KEYS_CLEAR();

	return 1;
}

int OPTION_DEBUG_EDIT_FLAGS(int g) {
	#define OP_DB_COLUMNS 5
	int x, y;
	int index_count = 0;
	int indexes[100];
	int cur = -1, next = 0;
	int y_max;
	int x_lc_max;
	
	if (!game_playing) {
		fadeOutTick(screen, 0x100);
		GAME_BUFFER_REPAINT(0);
		message_info("Game not started                    ");
		PROGRAM_DELAY(2000);
		return 0;
	}	

	toggle_info = 1;
	
	#define OP_DB_PRINT_OP_KV(c, x, y, k, v) text_at(c, screen_video->w * (x) / OP_DB_COLUMNS, (y) * font2_height, "%03d: %4d ", k, v);
	#define OP_DB_PRINT_OP(c, x, y, k) { OP_DB_PRINT_OP_KV(c, x, y, k, save.flags[k]); }
	#define OP_DB_PRINT_N(c, n) if ((n) >= 0 && (n) < index_count) { OP_DB_PRINT_OP(c, ((n) % OP_DB_COLUMNS), ((n) / OP_DB_COLUMNS + 2), indexes[n]); }
	#define OP_DB_PRINT_N_U(c, n) { OP_DB_PRINT_N(c, n); SDL_Flip(screen_video); }
	#define OP_DB_PRINT_VARS { int n; for (n = 0; n < index_count; n++) OP_DB_PRINT_N(white, n); SDL_Flip(screen_video); GAME_UPDATE_DEBUG_INFO(); }

	#define OP_DB_ADD_FLAGS(start, count) { int e, p; for (e = index_count, p = start; index_count < e + (count); index_count++, p++) { indexes[index_count] = p; } }
	#define OP_DB_VAR(n) save.flags[indexes[n]]
	
	//fadeOutTick(screen, 0x40);
	fadeOutTick(screen, 0x100);
	
	GAME_BUFFER_REPAINT(0);
	
	OP_DB_ADD_FLAGS(0, 36);
	OP_DB_ADD_FLAGS(50, 1);
	OP_DB_ADD_FLAGS(112, 21);
	OP_DB_ADD_FLAGS(501, 2);
	OP_DB_ADD_FLAGS(700, 10);
	OP_DB_ADD_FLAGS(800, 9);
	OP_DB_ADD_FLAGS(810, 1);
	
	OP_DB_PRINT_VARS;
	
	x = 0; y = 0;
	y_max = index_count / OP_DB_COLUMNS + ((index_count % OP_DB_COLUMNS) == 0);
	x_lc_max = index_count % OP_DB_COLUMNS;
	
	while (1) {
		KEYS_UPDATE();
		if (keys & K_B) break;
		
		if (keys & K_LEFT ) x--;
		if (keys & K_RIGHT) x++;
		if (keys & K_UP   ) y--;
		if (keys & K_DOWN && cur + OP_DB_COLUMNS < index_count) y++;
		
		if (x < 0) x = 0;
		if (x >= OP_DB_COLUMNS) x = OP_DB_COLUMNS - 1;
		if (y < 0) y = 0;
		if (y > y_max) y = y_max;
		if (y == y_max && x >= x_lc_max) x = x_lc_max - 1;
		
		next = y * OP_DB_COLUMNS + x;
		
		do {
			if (next != cur) {
				if (next < 0) next = index_count + next;
				if (next >= index_count) next = next - index_count;
				OP_DB_PRINT_N_U(white, cur);
				cur = next;
				OP_DB_PRINT_N_U(red, cur);
			}
			
			if (keys & K_A) {
				int update = 1;
				int back = OP_DB_VAR(cur);
				KEYS_CLEAR();
				while (1) {
					KEYS_UPDATE();
					
					if (keys & K_B    ) { OP_DB_VAR(cur) = back; break; }
					if (keys & K_A    ) { GAME_UPDATE_DEBUG_INFO(); break; }
					if (keys & K_DOWN ) { update = 1; OP_DB_VAR(cur)--; }
					if (keys & K_UP   ) { update = 1; OP_DB_VAR(cur)++; }
					if (keys & K_LEFT ) { update = 1; OP_DB_VAR(cur) -= 10; }
					if (keys & K_RIGHT) { update = 1; OP_DB_VAR(cur) += 10; }
					
					if (update) {
						if (((int)OP_DB_VAR(cur)) < 0) OP_DB_VAR(cur) = 0;
						if (OP_DB_VAR(cur) > 1000) OP_DB_VAR(cur) = 1000;
						OP_DB_PRINT_N_U(green, cur);
						update = 0;
					}
				}
				KEYS_CLEAR();
				
				cur = -1;
				continue;
			}
			
			break;
		} while (1);
	}
	
	KEYS_CLEAR();
	
	return 1;
}

int OPTION_GALLERY(int g) {
	int x = 0, y = 0, page = 0;
	int nx, ny, np;
	int update = 0, update_page = 1;
	CG_PAGE *cpage = &cg_pages[page];
	SDL_Surface *img = NULL;
	SDL_Surface *img_e = NULL;
	SDL_Surface *bg_temp = NULL;
	SDL_PixelFormat *pf = &screen_format;
	
	SDL_Rect rect_base = menu_gallery_base;
	SDL_Rect rect;
	
	GAME_BACKGROUND_O("WAKU_A1");
	
	bg_temp = SDL_CreateRGBSurface(screen->flags, clip_bg2.w, clip_bg2.h, pf->BitsPerPixel, pf->Rmask, pf->Gmask, pf->Bmask, pf->Amask);
	
	save_temp = save;
	
	while (1) {
		KEYS_UPDATE();
		
		if (keys & K_B) break;
		
		if (keys & K_R) {
			GAME_MENU_SHOW_EX(extra_menu_gallery, 1);
			continue;
		}		
		
		if ((keys & K_UP   ) && y > 0) { y--; update = 1; }
		if ((keys & K_DOWN ) && y < 4) { y++; update = 1; }
		if ((keys & K_LEFT )) {
			if (x > 0) {
				x--; update = 1;
			} else if (y > 0) {
				//x = 4; y--; update = 1;
			}
		}
		if ((keys & K_RIGHT)) {
			if (x < 4) {
				x++; update = 1;
			} else if (y < 4) {
				//x = 0; y++; update = 1;
			}
		}
		
		if (keys & K_A) {
			if (x == 0 && y == 0 && cpage->up) {
				page--;
				update_page = 1;
			} else if (x == 4 && y == 4 && cpage->down) {
				page++;
				update_page = 1;
			} else {
				np = cpage->from + y * 5 + x;
				printf("Image: %d...", np);
				if ((np >= 0 && np < 150) && save_s.gallery[np]) {
					int n, len;
					char name[128];
					printf("Viewing\n");
					strcpy(name, gallery[np]);
					for (n = 0; n < 128; n++) if (name[n] == '.') { name[n] = 0; break; }
					len = strlen(name);
					
					switch (name[0]) {
						case 'O': // OMAKE
						case 'I': // Images
							OPTION_GALLERY_SHOW(name);
							name[len + 1] = 0;
							for (n = 0; n <= 5; n++) {
								name[len] = 'A' + n;
								OPTION_GALLERY_SHOW(name);
							}
						break;
						case 'H': // Places
							for (n = 0; n <= 5; n++) {
								name[len - 1] = '0' + n;
								OPTION_GALLERY_SHOW(name);
							}
						break;
						default: OPTION_GALLERY_SHOW(name);
					}

					update = 1;
				} else {
					printf("Disabled\n");
				}
			}
		}

		if (update_page) {
			cpage = &cg_pages[page];
			update_page = 0;
			update = 1;
			img   = GAME_IMAGE_GET(cpage->i_d);
			img_e = GAME_IMAGE_GET(cpage->i_e);
			
			SDL_BlitSurface(img, NULL, bg_temp, NULL);
			
			np = cpage->from;
			for (ny = 0; ny < 5; ny++) {
				for (nx = 0; nx < 5; nx++, np++) {
					if ((np >= 0 && np < 150) && save_s.gallery[np]) {
						//printf("Enable Image: %d\n", np);
						rect = rect_base;
						rect.x += rect_base.w * nx;
						rect.y += rect_base.h * ny;
						SDL_BlitSurface(img_e, &rect, bg_temp, &rect);
					}
				}
			}
		}
		
		if (update) {
			update = 0;
			SDL_BlitSurface(bg_temp, NULL, screen, &clip_bg2);

			rect = rect_base;
			rect.x += clip_bg2.x + (rect_base.w * x);
			rect.y += clip_bg2.y + (rect_base.h * y);
			
			SDL_DrawRect(screen, &rect, SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00), 2);
			GAME_BUFFER_REPAINT(0);
		}
		
		PROGRAM_DELAY(6);
	}
	
	save = save_temp;
	
	SDL_FreeSurface(bg_temp);
	
	return 1;
}

int OPTION_SAVE_IMAGE(int n) {
	char name[0x100];
	sprintf(name, SNAP_ROOT "/dividead_%d.bmp", (int)time(NULL));
	SDL_Surface *temp = SDL_CreateRGBSurface(SDL_SWSURFACE, clip_bg2.w, clip_bg2.h, screen_format.BitsPerPixel, screen_format.Rmask, screen_format.Gmask, screen_format.Bmask, screen_format.Amask);
	SDL_BlitSurface(snapshot_last, &clip_bg2, temp, &clip_bg1);
	SDL_SaveBMP(temp, name);
	SDL_FreeSurface(temp);
	return 1;
}

int OPTION_CREDITS(int n) {
	//bg_temp = SDL_CreateRGBSurface(screen->flags, clip_bg2.w, clip_bg2.h, pf->BitsPerPixel, pf->Rmask, pf->Gmask, pf->Bmask, pf->Amask);
	CREDIT_SHOW(1);
	return 1;
}

int MAIN_MENU_GAME_OPTIONS_VOICE(int p);
int MAIN_MENU_GAME_OPTIONS_MUSIC(int p);

char zzoptions[2][0x30];

OPTION_GAME_MENU main_menu_options[3] = {
	{ zzoptions[0], MAIN_MENU_GAME_OPTIONS_VOICE},
	{ zzoptions[1], MAIN_MENU_GAME_OPTIONS_MUSIC},
	{ NULL           , NULL },
};

void updateOptionsTexts(int updated) {
	//lang_texts[8]
	sprintf(zzoptions[0], "%s %s", lang_texts[9], audio_voice_enabled ? "on" : "off");
	sprintf(zzoptions[1], "%s %s", lang_texts[10], audio_music_enabled ? "on" : "off");
	
	if (updated) {
		if (audio_music_enabled) {
			GAME_MUSIC_PLAY(NULL);
		} else {
			GAME_MUSIC_STOP();
		}
	}
}

int MAIN_MENU_GAME_OPTIONS_VOICE(int p) {
	audio_voice_enabled = !audio_voice_enabled;
	updateOptionsTexts(0);
	return 0;
}

int MAIN_MENU_GAME_OPTIONS_MUSIC(int p) {
	audio_music_enabled = !audio_music_enabled;
	updateOptionsTexts(1);
	return 0;
}

int MAIN_MENU_GAME_OPTIONS(int p) {
	updateOptionsTexts(0);
	GAME_MENU_SHOW_EX(main_menu_options, 1);
	return 0;
}

int OPTION_SELECT_LANG(int p) {
	switch (p) {
		case 0: strcpy(language, "JAPANESE"); break;
		case 1: strcpy(language, "ENGLISH"); break;
		case 2: strcpy(language, "GERMAN"); break;
		case 3: strcpy(language, "FRENCH"); break;
		case 4: strcpy(language, "SPANISH"); break;
		case 5: strcpy(language, "ITALIAN"); break;
	}
	lang_postinit();

	return 0;
}

OPTION_GAME_MENU main_menu_langs[7] = {
	{ "JAPANESE" , OPTION_SELECT_LANG },
	{ "ENGLISH"  , OPTION_SELECT_LANG },
	{ "GERMAN"   , OPTION_SELECT_LANG },
	{ "FRENCH"   , OPTION_SELECT_LANG },
	{ "SPANISH"  , OPTION_SELECT_LANG },
	{ "ITALIAN"  , OPTION_SELECT_LANG }, 
	{ NULL       , NULL },
};

int MAIN_MENU_GAME_LANG(int p) {
	return GAME_MENU_SHOW_EX(main_menu_langs, 1);
}

OPTION_GAME_MENU main_menu_ingame[6] = {
	{ lang_texts[1], MAIN_MENU_GAME_START },
	{ lang_texts[2], MAIN_MENU_GAME_SAVE },
	{ lang_texts[3], MAIN_MENU_GAME_LOAD },
	{ lang_texts[8], MAIN_MENU_GAME_OPTIONS },
	{ "LANGUAGE",    MAIN_MENU_GAME_LANG },    // LANGUAGE
#ifndef DREAMCAST
	{ lang_texts[4], MAIN_MENU_GAME_EXIT },
#endif
	{ NULL     , NULL },
};

OPTION_GAME_MENU main_menu_title[6] = {
	{ lang_texts[1], MAIN_MENU_GAME_START },   // START
	{ lang_texts[3], MAIN_MENU_GAME_LOAD },    // LOAD
	{ lang_texts[8], MAIN_MENU_GAME_OPTIONS }, // OPTIONS
	{ "LANGUAGE",    MAIN_MENU_GAME_LANG },    // LANGUAGE
#ifndef DREAMCAST
	{ lang_texts[4], MAIN_MENU_GAME_EXIT },    // EXIT
#endif
	{ NULL     , NULL },
};


char percent_option[0x30] = {0};
//debug_global
OPTION_GAME_MENU extra_menu_title_nodebug[3] = {
	{ percent_option  , OPTION_GALLERY },      // PERCENT
	{ lang_texts[6]   , OPTION_SAVE_IMAGE },   // SCREENSHOT
	{ NULL            , NULL },
};

OPTION_GAME_MENU extra_menu_title_debug[7] = {
	{ percent_option  , OPTION_GALLERY },            // PERCENT
	{ lang_texts[6]   , OPTION_SAVE_IMAGE },         // SCREENSHOT
	{ "Edit Flags"    , OPTION_DEBUG_EDIT_FLAGS },   // DEBUG: EDIT FLAGS
	{ "Restart Room"  , OPTION_DEBUG_RESTART_ROOM }, // DEBUG: RESTART ROOM
	{ "Jump Room"     , OPTION_JUMP_ROOM },          // DEBUG: JUMP ROOM
	{ "View Credits"  , OPTION_CREDITS},             // DEBUG: VIEW CREDITS
	{ NULL            , NULL },
};

//OPTION_GAME_MENU *extra_menu_title = extra_menu_title_nodebug;

void MAIN_MENU_SHOW() {
	menu_button_pos = 0;
	GAME_MENU_SHOW(game_playing ? main_menu_ingame : main_menu_title);	
}

void EXTRA_MENU_SHOW() {
	int n, count = 0;
	for (n = 0; n < 150; n++) {
		if (save_s.gallery[n]) count++;
	}
	sprintf(percent_option, lang_texts[5], (((double)count) / 150) * 100);
	menu_button_pos = 1;
	GAME_MENU_SHOW(debug_global ? extra_menu_title_debug : extra_menu_title_nodebug);
}

int CHECK_MENU_KEYS_EX() {
	if (keys & K_MODE) {
		toggle_info = !toggle_info;
		GAME_SCREEN_UPDATE(screen);
	}

	if ((keys & K_L) || (keys & K_R)) {
		message("CHECK_MENU_KEYS :: 1");
		if (audio_initialized) { Mix_Pause(1); Mix_Pause(2); }
		//Mix_PauseMusic();
		GAME_SOUND_CLICK();
		message("CHECK_MENU_KEYS :: 2");
		if (keys & K_L) MAIN_MENU_SHOW(); else EXTRA_MENU_SHOW();
		message("CHECK_MENU_KEYS :: 3");
		if (!game_start) {
			if (audio_initialized) { Mix_Resume(1); Mix_Resume(2); }
			//Mix_ResumeMusic();
		}
		message("CHECK_MENU_KEYS :: 4");
		return 1;
	}
	
	return 0;
}

int CHECK_MENU_KEYS() {
	int retval;
	#ifdef DREAMCAST
		dc_draw_vmu_icon_block = 1;
	#endif
	retval = CHECK_MENU_KEYS_EX();
	#ifdef DREAMCAST
		dc_draw_vmu_icon_block = 0;
	#endif
	return retval;
}
