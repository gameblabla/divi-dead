#define DEBUG_SCRIPT
//#define DEBUG_SCRIPT_FLAGS

////////////////////////////////////////////////////////////
// SCRIPT                                                 //
////////////////////////////////////////////////////////////

ubyte  SCRIPT_GET8 () { ubyte  v = mem1(scrp); scrp += 1; return v; }
ushort SCRIPT_GET16() { ushort v = mem2(scrp); scrp += 2; return v; }
uint   SCRIPT_GET32() { uint   v = mem4(scrp); scrp += 4; return v; }
char  *STRING_GETSZ() { char *r = (char *)scrp; scrp += strlen((char *)scrp) + 1; return r; }
int    RANGE_COLLAPSE(int v, int m, int M) { if (v < m) return m; if (v > M) return M; return v; }

void GAME_SCRIPT_JUMP(int addr) {
	scrp = script + addr;
}

int filter_animation(char c) {
#ifdef FILTER_HACK
	//if (c == 'C') return 1;
	//if (c == 'B') return 1;
	//if (c == 'D') return 1;
	switch (c) {
		//case 'C': return 1;
		case 'B': case 'D': return 1;
		default: return 0;
	}
#endif

	return 0;
}

int GAME_SCRIPT_LOAD(char *name) {
	SDL_RWops *f;
	char temp[0x40];
	int len;
	
	scrp = script;
	
	sprintf(temp, "%s.AB", name);

	if (!(f = VFS_LOAD(temp))) {
		printf("Can't load script ('%s')", temp);
		//PROGRAM_EXIT_ERROR("Can't load script ('%s')", temp);
		return -1;
	}
	
	strcpy(save.script, name);

	len = SDL_RWseek(f, 0, SEEK_END);
	SDL_RWseek(f, 0, SEEK_SET);
	SDL_RWread(f, script, 1, len);
	
	printf("SCRIPT_LOAD: %s(%d)\n", temp, len);
		
	SDL_RWclose(f);
	
	script[sizeof(script) - 3] = 0xFF;
	script[sizeof(script) - 2] = 0xFF;
	script[sizeof(script) - 1] = 0xFF;
	
	return 0;
}

/*
struct {
	char script[0x20];
	char background_o[0x20];
	char background_i[0x20];
	char background_v[0x20];
	char character_1[0x20];
	char character_2[0x20];
	char unknown[0x20];
	char music[0x20];
	uint unk1;
	uint script_pos;
	uint script_menu_return;
	uint flags[1000];
} save = {0}, save_temp = {0};
*/

// flags:
//    000-050 +51
//    100-150 +51
//    500-510 +11
//    700-710 +11
//    800-810 +11
// --------------
//            135

void GAME_SAVE_FULL(SDL_RWops *f) {
	VAR_SAVE_L(&save, sizeof(save));
}

void GAME_SAVE_MIN(SDL_RWops *f) {
	int n, p = 0;
	unsigned short vars[135];
	unsigned char magic = 0xFF;
	
	printf("GAME_SAVE_MIN:0\n");
	
	for (n =   0; n <=  50; n++) vars[p++] = save.flags[n];
	for (n = 100; n <= 150; n++) vars[p++] = save.flags[n];
	for (n = 500; n <= 510; n++) vars[p++] = save.flags[n];
	for (n = 700; n <= 710; n++) vars[p++] = save.flags[n];
	for (n = 800; n <= 810; n++) vars[p++] = save.flags[n];
	
	//printf("vars: %d\n", p);
	
	printf("GAME_SAVE_MIN:1\n");
	
	VAR_SAVE_L(&magic, 1);
	VAR_SAVE_L(save.script, 0x10);
	VAR_SAVE_L(save.background_o, 0x10);
	VAR_SAVE_L(save.background_i, 0x10);
	VAR_SAVE_L(save.background_v, 0x10);
	VAR_SAVE_L(save.character_1, 0x10);
	VAR_SAVE_L(save.character_2, 0x10);
	VAR_SAVE_L(save.music, 0x10);
	VAR_SAVE_L(&save.script_pos, 4);
	VAR_SAVE_L(&save.script_menu_return, 4);
	VAR_SAVE_L(vars, sizeof(vars));
	
	printf("GAME_SAVE_MIN:2\n");
}

int GAME_SAVE(int n) {
	SDL_RWops *f;
	char name[0x100];
	char date[80];
	time_t now;
	struct tm *tmPtr;
	
	printf("GAME_SAVE: %d\n", n);
	
	now = time(NULL);
	tmPtr = localtime(&now);
	strftime(date, 12, "%d/%m %H:%M", tmPtr);
	sprintf(save_s.names[n], " %s %s", date, save_title);

	printf("GAME_SAVE():0\n");
	
	sprintf(name, SAVE_ROOT "/DATA/DATA%d.DAT", n);
	
	printf("GAME_SAVE():1\n");
	
#ifdef DREAMCAST
	f = save_buffer_open();
	printf("GAME_SAVE():DC\n");
#else
	mkdir(SAVE_ROOT "/DATA", 0777);
	if (!(f = SDL_RWFromFile(name, "wb"))) {
		printf("Can't save\n");
		return 0;
	}
#endif
	printf("GAME_SAVE():2\n");
	
	if (SAVE_FULL) {
		GAME_SAVE_FULL(f);
	} else {
		GAME_SAVE_MIN(f);
	}
	
	printf("GAME_SAVE():3\n");

#ifdef DREAMCAST
	printf("GAME_SAVE():3b\n");
	dc_save_file_save(1, n, save_buffer, SDL_RWtell(f));
#endif

	printf("GAME_SAVE():4\n");
	
	SDL_RWclose(f);
	
	//sprintf(save_s.names[n], "%s %s", save_title, date);
	SYS_SAVE();
	
	printf("GAME_SAVE():8\n");
	
	return 1;
}

void GAME_LOAD_FULL(SDL_RWops *f) {
	VAR_LOAD_L(&save, sizeof(save));
}

void GAME_LOAD_MIN(SDL_RWops *f) {
	int n, p = 0;
	unsigned short vars[135];
	unsigned char magic = 0xFF;
	
	VAR_LOAD_L(&magic, 1);
	VAR_LOAD_L(save.script, 0x10);
	VAR_LOAD_L(save.background_o, 0x10);
	VAR_LOAD_L(save.background_i, 0x10);
	VAR_LOAD_L(save.background_v, 0x10);
	VAR_LOAD_L(save.character_1, 0x10);
	VAR_LOAD_L(save.character_2, 0x10);
	VAR_LOAD_L(save.music, 0x10);
	VAR_LOAD_L(&save.script_pos, 4);
	VAR_LOAD_L(&save.script_menu_return, 4);
	VAR_LOAD_L(vars, sizeof(vars));

	for (n =   0; n <=  50; n++) save.flags[n] = vars[p++];
	for (n = 100; n <= 150; n++) save.flags[n] = vars[p++];
	for (n = 500; n <= 510; n++) save.flags[n] = vars[p++];
	for (n = 700; n <= 710; n++) save.flags[n] = vars[p++];
	for (n = 800; n <= 810; n++) save.flags[n] = vars[p++];
}

int GAME_LOAD(int n) {
	unsigned char magic = 0xFF;
	char name[0x80];
	SDL_RWops *f;

	printf("GAME_LOAD: %d\n", n);
	
	sprintf(name, SAVE_ROOT "/DATA/DATA%d.DAT", n);

#ifdef DREAMCAST
	printf("GAME_LOAD():Dc\n");
	if (dc_save_file_load(1, n, save_buffer, &save_buffer_size) != 0) {
		return 0;
	}
	f = save_buffer_open_load();
#else
	if (!(f = SDL_RWFromFile(name, "rb"))) {
		printf("Can't load\n");
		return 0;
	}
#endif

	printf("GAME_LOAD():1\n");
	
	VAR_LOAD_L(&magic, 1);
	SDL_RWseek(f, 0, SEEK_SET);	
	
	printf("GAME_LOAD():2\n");
	
	(magic != 0xFF) ? GAME_LOAD_FULL(f) : GAME_LOAD_MIN(f);
	
	printf("GAME_LOAD():3\n");

	SDL_RWclose(f);
	
	printf("GAME_LOAD():4\n");
	
	GAME_SCRIPT_LOAD(save.script);
	GAME_SCRIPT_JUMP(save.script_pos);
	
	printf("GAME_LOAD():5\n");

	//SDL_Flip(screen);
	GAME_BUFFER_REPAINT(0);
	
	printf("GAME_LOAD():6\n");
	
	strcpy(save_title, save_s.names[n] + 13);
	
	printf("GAME_LOAD():7\n");
	
	return 1;
}

void GAME_SAVE_POSITION_2() {
	save.script_pos = scrp - script - 2;
}

void GAME_SCRIPT_PROCESS() {
	game_playing = 1;
	printf("GAME_SCRIPT_PROCESS\n");
	
	GAME_MUSIC_PLAY(NULL);
	GAME_BACKGROUND_O(NULL);
	GAME_BACKGROUND_I(NULL);
	GAME_BACKGROUND_I_MASK(NULL);
	GAME_CHARA2(NULL, NULL);
	GAME_BUFFER_REPAINT(0);
	
	while (game_playing) {
		ushort op = SCRIPT_GET16();
		#ifdef DEBUG_SCRIPT
			printf("OP: %02X at 0x%06X : ", op, (scrp - script - 2));
		#endif
		
		GAME_SOUND_GC_COLLECT();
		
		switch (op) {
			// ---------------
			//  FLOW RELATED  
			// ---------------
			
			case 0x02: { // JUMP
				int addr = SCRIPT_GET32();
				#ifdef DEBUG_SCRIPT
					printf("JUMP(0x%06X)\n", addr);
				#endif
				GAME_SCRIPT_JUMP(addr);
			} break;
			case 0x10: { // JUMP_IF
				int flag  = RANGE_COLLAPSE(SCRIPT_GET16(),  0, 999);
				char  op  = SCRIPT_GET8();
				int value = SCRIPT_GET16();
				int addr  = SCRIPT_GET32();
				#ifdef DEBUG_SCRIPT
					printf("JUMP_IF: F[%d]{%d} %c %d -> 0x%06X\n", flag, save.flags[flag], op, value, addr);
				#endif
				#ifdef DEBUG_SCRIPT_FLAGS
					printf("JUMP_IF: F[%d]{%d} %c %d -> 0x%06X\n", flag, save.flags[flag], op, value, addr);
				#endif
				switch (op) {
					case '=': if (!(save.flags[flag] == value)) GAME_SCRIPT_JUMP(addr); break; // ==
					case '}': if (!(save.flags[flag] >  value)) GAME_SCRIPT_JUMP(addr); break; // >, >= ??
					case '{': if (!(save.flags[flag] <  value)) GAME_SCRIPT_JUMP(addr); break; // <, <= ??
					default:
						printf("JUMP_IF: unknown operator '%c'\n", op);
						PROGRAM_EXIT(-1);
					break;
				}
			} break;
			case 0x03: { // FLAG_SET_RANGE
				int l, h, v;
				l = RANGE_COLLAPSE(SCRIPT_GET16(), 0, 999);
				h = RANGE_COLLAPSE(SCRIPT_GET16(), 0, 999);
				v = SCRIPT_GET16();
				#ifdef DEBUG_SCRIPT
					printf("FLAG_SET_RANGE(%d-%d=%d)\n", l, h, v);
				#endif
				#ifdef DEBUG_SCRIPT_FLAGS
					printf("FLAG_SET_RANGE(%d-%d=%d)\n", l, h, v);
				#endif
				for (; l < h; l++) save.flags[l] = v;
			} break;
			case 0x4: { // FLAG_SET
				int flag  = RANGE_COLLAPSE(SCRIPT_GET16(), 0, 999);
				char  op  = SCRIPT_GET8();
				int value = SCRIPT_GET16();
				#ifdef DEBUG_SCRIPT
					printf("FLAG_SET: F[%d]{%d} %c %d\n", flag, save.flags[flag], op, value);
				#endif
				#ifdef DEBUG_SCRIPT_FLAGS
					printf("FLAG_SET: F[%d]{%d} %c %d\n", flag, save.flags[flag], op, value);
				#endif
				switch (op) {
					case '=': save.flags[flag]  = value; break; // =
					case '+': save.flags[flag] += value; break; // + ??
					case '-': save.flags[flag] -= value; break; // - ?? (solo una vez -0??)
					default:
						printf("SET: unknown operator '%c'\n", op);
						PROGRAM_EXIT(-1);
					break;
				}
			} break;
			case 0x18: { // SCRIPT_CHANGE
				char *s = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("SCRIPT_CHANGE('%s')\n", s);
				#endif
				GAME_SCRIPT_LOAD(s);
			} break;
			case 0x19: { // GAME_END
				#ifdef DEBUG_SCRIPT
					printf("GAME_END\n");
				#endif
				game_playing = 0;
				game_end = 1;
				{
					int n;
					for (n = 0; n < 10; n++) save_s.gallery[n + 140] = 1;
				}
				SYS_SAVE();
				CREDIT_SHOW(0);
			} break;
			
			// ---------------
			//  INPUT         
			// ---------------
			
			case 0x00: GAME_SAVE_POSITION_2(); { // TEXT
				char *s = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("TEXT('%s')\n", s);
				#endif
				if (strlen(s)) {
					GAME_TEXT_SNAP();
					GAME_TEXT_PRINT(s, white, clip_text);
					GAME_TEXT_UPDATE();
					GAME_INPUT_WAIT(s);
				}
			} break;

			case 0x50: { // SAVE_TITLE
				char *s = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("SAVE_TITLE('%s')\n",s);
				#endif
				strcpy(save_title, s);
			} break;
			
			case 0x06: GAME_SAVE_POSITION_2(); { // OPTION_RESET
				#ifdef DEBUG_SCRIPT
					printf("OPTION_RESET()\n");
				#endif
				if (!(save.cursor1 & 0x8000)) {
					printf("back:%08X, cursor1=0\n", save.cursor1);
					save.cursor1 = 0;
				}
				script_options_count = 0;
				save.script_menu_return = scrp - 2 - script;
			} break;
			
			case 0x01: { // OPTION
				int ptr = SCRIPT_GET32();
				char *s = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("OPTION(0x%06X, '%s')\n", ptr, s);
				#endif
				script_options[script_options_count].text = s;
				script_options[script_options_count].ptr = ptr;
				script_options_count++;
			} break;			
			case 0x0A: { // OPTION_RESHOW?
				#ifdef DEBUG_SCRIPT
					printf("OPTION_RESHOW()\n");
					// Deberia guardar la posicion del ultimo OPTION_RESET y reestablecerla (para que los saves puedan cargarse bien)
				#endif
				scrp = script + save.script_menu_return;
				save.cursor1 |= 0x8000;
			} break;
			case 0x07: { // OPTION_SHOW
				#ifdef DEBUG_SCRIPT
					printf("OPTION_SHOW()\n");
				#endif
				GAME_TEXT_SNAP();
				GAME_INPUT_OPTIONS();
			} break;
			
			case 0x37: GAME_SAVE_POSITION_2(); { // MAP_IMAGES
				char *s1 = STRING_GETSZ();
				char *s2 = STRING_GETSZ();
				strcpy(map_images[0], s1);
				strcpy(map_images[1], s2);
				#ifdef DEBUG_SCRIPT
					printf("MAP_IMAGES('%s', '%s')\n", map_images[0], map_images[1]);
				#endif
			} break;
			case 0x38: { // MAP_OPTION_RESET
				#ifdef DEBUG_SCRIPT
					printf("MAP_OPTION_RESET()\n");
				#endif
				map_options_count = 0;
			} break;
			case 0x40: { // MAP_OPTION
				uint ptr = SCRIPT_GET32();
				uint x1  = SCRIPT_GET16();
				uint y1  = SCRIPT_GET16();
				uint x2  = SCRIPT_GET16();
				uint y2  = SCRIPT_GET16();
				map_options[map_options_count].ptr = ptr;
				map_options[map_options_count].x1 = x1;
				map_options[map_options_count].y1 = y1;
				map_options[map_options_count].x2 = x2;
				map_options[map_options_count].y2 = y2;
				map_options_count++;
				#ifdef DEBUG_SCRIPT
					printf("MAP_OPTION(0x%06X, %d, %d, %d, %d)\n", ptr, x1, y1, x2, y2);
				#endif
			} break;
			case 0x41: { // MAP_OPTION_SHOW
				#ifdef DEBUG_SCRIPT
					printf("MAP_OPTION_SHOW()\n");
				#endif
				GAME_INPUT_MAP_OPTIONS();
			} break;			
			
			case 0x11: { // WAIT
				int t = SCRIPT_GET16();
				#ifdef DEBUG_SCRIPT
					printf("WAIT(%d.%d)\n", t / 100, t % 100);
				#endif
				PROGRAM_DELAY(t * 10);
			} break;
			
			// ---------------
			//  SOUND RELATED 
			// ---------------
			
			case 0x26: { // MUSIC_PLAY
				char *s = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("MUSIC_PLAY('%s')\n", s);
				#endif
				GAME_MUSIC_PLAY(s);
			} break;
			case 0x28: { // MUSIC_STOP
				#ifdef DEBUG_SCRIPT
					printf("MUSIC_STOP()\n");
				#endif
				GAME_MUSIC_STOP();
			} break;
			case 0x2B: { // VOICE_PLAY
				char *s = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("VOICE_PLAY('%s')\n", s);
				#endif
				GAME_VOICE_PLAY(s);
			} break;
			case 0x35: { // SOUND
				char *s = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("SOUND('%s')\n", s);
				#endif
				GAME_SOUND_PLAY(s);
			} break;
			case 0x36: { // SOUND_STOP
				#ifdef DEBUG_SCRIPT
					printf("SOUND_STOP()\n");
				#endif
				GAME_SOUND_STOP();
			} break;			
			// ---------------
			//  IMAGE RELATED
			// ---------------

			case 0x46: { // BACKGROUND
				char *s = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("BACKGROUND('%s')\n", s);
				#endif
				GAME_BACKGROUND_O(s);
			} break;
			case 0x47: { // BACKGROUND_INNER
				char *s = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("BACKGROUND_INNER('%s')\n", s);
				#endif
				GAME_BACKGROUND_I(s);
			} break;
			case 0x16: { // IMAGE_MASK
				char *s = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("IMAGE_MASK('%s')\n", s);
				#endif
				GAME_BACKGROUND_I_MASK(s);
			} break;
			case 0x4B: { // CHARACTER
				char *s = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("CHARA('%s')\n", s);
				#endif
				GAME_CHARA(s);
			} break;
			case 0x4C: { // TWO CHARACTERS
				char *s1 = STRING_GETSZ();
				char *s2 = STRING_GETSZ();
				#ifdef DEBUG_SCRIPT
					printf("CHARA2('%s', '%s')\n", s1, s2);
				#endif
				GAME_CHARA2(s1, s2);
			} break;

			// ----------------------
			//  IMAGE/EFFECT RELATED
			// ----------------------

			case 0x4D: { // ANIMATION ABCDEF
				char cc;
				char temp[0x40]; int len;
				strcpy(temp, save.background_i);
				len = strlen(temp);
				char *c = temp + len - 1;
				
				for (cc = 'A'; cc <= 'F'; cc++) {
					printf("P(%c)", cc);
					if (filter_animation(cc)) continue;
					*c = cc; GAME_IMAGE_GET(temp);
				}
				for (cc = 'A'; cc <= 'F'; cc++) { 
					printf("S(%c)", cc);
					if (filter_animation(cc)) continue;
					*c = cc; GAME_BACKGROUND_I(temp);
				}
				
				#ifdef DEBUG_SCRIPT
					printf("ANIMATION()\n");
				#endif
			} break;
			case 0x4F:   // SCROLL_UP (add 'B')
			case 0x4E: { // SCROLL_DOWN (add 'A')
				#ifdef DEBUG_SCRIPT
					if (op == 0x4E) {
						printf("SCROLL_DOWN()\n");
					} else {
						printf("SCROLL_UP()\n");
					}
				#endif

				int y; char temp[0x40];
				SDL_Rect move1, move2;
				
				sprintf(temp, "%s%c", save.background_i, (op == 0x4E) ? 'A' : 'B');
				SDL_PixelFormat *pf = screen->format;
				SDL_Surface *s1 = SDL_CreateRGBSurface(SDL_SWSURFACE, clip_bg2.w, clip_bg2.h, pf->BitsPerPixel, pf->Rmask, pf->Gmask, pf->Bmask, pf->Amask);
				SDL_Surface *s2 = GAME_IMAGE_GET(temp);
				
				SDL_BlitSurface(screen, &clip_bg2, s1, NULL);
				
				SDL_SetClipRect(screen, &clip_bg2);
				
				for (y = 0; y < s1->h;
				#ifdef DREAMCAST
				y += 4
				#else
				y++
				#endif
				) {
					move2 = clip_bg2; 
					move1 = clip_bg2;
					if (op == 0x4E) { // SCROLL_DOW
						move2.y += clip_bg2.h - y;
						move1.y -= y;
					} else { // SCROLL_UP
						move2.y += y - clip_bg2.h;
						move1.y += y;					
					}

					SDL_BlitSurface(s1, NULL, screen, &move1);
					SDL_BlitSurface(s2, NULL, screen, &move2);
					
					GAME_SCREEN_UPDATE_RECT(screen, &clip_bg2);
					PROGRAM_DELAY(1);
				}
				
				SDL_SetClipRect(screen, &clip_bg1);
				//SDL_SetClipRect(screen, NULL);
				
				SDL_FreeSurface(s1);
			} break;			
			// ----------------
			//  EFFECT RELATED
			// ----------------
			
			case 0x30: { // CLIP
				clip.x = SCRIPT_GET16();
				clip.y = SCRIPT_GET16();
				clip.w = SCRIPT_GET16();
				clip.h = SCRIPT_GET16();
				#ifdef DEBUG_SCRIPT
					printf("CLIP(%d, %d, %d, %d)\n", clip.x, clip.y, clip.w, clip.h);
				#endif
			} break;

			case 0x1E: { // FADE_OUT
				#ifdef DEBUG_SCRIPT
					printf("FADE_OUT()\n");
				#endif
				GAME_BUFFER_FADEOUT();
				PROGRAM_DELAY(250);
			} break;
			case 0x1F: { // FADE_OUT_WHITE
				#ifdef DEBUG_SCRIPT
					printf("FADE_OUT_WHITE()\n");
				#endif
				GAME_BUFFER_FADEOUT2();
				PROGRAM_DELAY(250);
			} break;			
			case 0x4A: { // BUFFER_REPAINT_INNER
				int t = SCRIPT_GET16();
				#ifdef DEBUG_SCRIPT
					printf("BUFFER_REPAINT_INNER(%d)\n", t);
				#endif
				GAME_BUFFER_REPAINT(t);
			} break;
			case 0x14: { // BUFFER_REPAINT
				int t = SCRIPT_GET16();
				#ifdef DEBUG_SCRIPT
					printf("BUFFER_REPAINT(%d)\n", t);
				#endif
				GAME_BUFFER_REPAINT(t);
			} break;

			default:
				printf("\n");
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				printf("Unknown OP 0x%02X\n", op);
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				printf("Invalid OP. Probably a new version of the script? Starting room again.\n");
				printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				scrp = script;
				//return;
				//PROGRAM_EXIT(-10);
			break;
		}
	}
	
	printf("Stopped...\n");
	
	game_playing = 0;
}
