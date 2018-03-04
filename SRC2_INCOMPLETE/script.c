#include "audio.h"
#include "video.h"

char script_data[0x10000];
char *script_pc = script_data;
int script_running = 0;

int flags[1000];

#define SCRIPT_GET8() (*script_pc++)
int SCRIPT_GET16() { int v1 = SCRIPT_GET8(), v2 = SCRIPT_GET8(); return (v1 << 0) | (v2 << 8); }
int SCRIPT_GET32() { int v1 = SCRIPT_GET16(), v2 = SCRIPT_GET16(); return (v1 << 0) | (v2 << 16); }
char* SCRIPT_GETSZ() { char* r = script_pc; while (*r++); r++; return r; }
#define SCRIPT_JUMP(addr) script_pc = script_data + addr;

typedef struct { void (*callback)(int v[6]); char *name, *format; } OP;

#define OPCODES_COUNT (sizeof(opcodes)/sizeof(opcodes[0]))

#define OP_FUNC(name) void op_##name##_callback(int v[6])
#define OP_DEF(name,format) {op_##name##_callback,#name,format}
#define OP_NULL {NULL,NULL,NULL}

#define VS(n) ((char *)v[n])
#define VI(n) ((unsigned int)v[n])
#define Vc(n) ((char)v[n])

// Muestra un texto
OP_FUNC(TEXT) {
	printf("%s\n", VS(0));
}

// Salta a una dirección concreta del script
OP_FUNC(JUMP) {
	SCRIPT_JUMP(VI(0));
}

// Pone a un valor determinado un rango de flags
OP_FUNC(SET_RANGE) {
}

// Cambia el valor de un flag
OP_FUNC(SET) {
	switch (Vc(1)) {
		case '=': flags[VI(0)]  = VI(1); break;
		case '+': flags[VI(0)] += VI(1); break;
		case '-': flags[VI(0)] -= VI(1); break;
	}
}

// Vacía la lista de opciones
OP_FUNC(OPTION_RESET) {
}

// Añade una opción a la lista de opciones
OP_FUNC(OPTION_ADD) {
}

// Muestra la lista de opciones
OP_FUNC(OPTION_SHOW) {
}

// Vuelve al último OPTION_RESET guardado
OP_FUNC(OPTION_RESHOW) {
}

// Salta a una dirección concreta si la condición NO se cumple
OP_FUNC(JUMP_IF_NOT) {
}

// Espera una cantidad de milisegundos
OP_FUNC(WAIT) {
}

// Cambia de script
OP_FUNC(SCRIPT) {
}

// Repinta la pantalla con los nuevos cambios, usando un efecto
OP_FUNC(REPAINT) {
}

// Pinta una imagen usando transparencias encima de lo que hay
OP_FUNC(IMAGE_OVERLAY) {
}

// Termina el juego, y activa los omakes en la galería
OP_FUNC(GAME_END) {
}

// Pinta progresivamente la pantalla de negro
OP_FUNC(FADE_OUT_BLACK) {
}

// Pinta progresivamente la pantalla de planco
OP_FUNC(FADE_OUT_WHITE) {
}

// Comienza a reproducir la música
OP_FUNC(MUSIC_PLAY) {
}

// Para la música
OP_FUNC(MUSIC_STOP) {
}

// Reproduce una voz
OP_FUNC(VOICE_PLAY) {
}

// Establece un rectangulo de recorte
OP_FUNC(CLIP) {
}

// Reproduce un efecto de sonido
OP_FUNC(EFFECT_PLAY) {
}

// Para el efecto de sonido actual
OP_FUNC(EFFECT_STOP) {
}

// Establece las imágenes que se usarán para el mapa
OP_FUNC(MAP_IMAGES) {
}

// Vacía la lista de opciones para el mapa
OP_FUNC(MAP_OPTION_RESET) {
}

// Añade una opción para el mapa
OP_FUNC(MAP_OPTION_ADD) { }

// Muestra el mapa con la lista de opciones
OP_FUNC(MAP_OPTION_SHOW) { }

// Pinta el borde
OP_FUNC(FOREGROUND) {
}

// Pinta el fondo
OP_FUNC(BACKGROUND) {
}

// Repinta el interior
OP_FUNC(REPAINT_IN) {
}

// Pinta un personaje en el centro de la pantalla
OP_FUNC(CHARA1) {
}

// Pinta dos personajes centrados
OP_FUNC(CHARA2) {
}

// Pinta una animación
OP_FUNC(ANIMATION) {
}

// Pinta un scroll hacia abajo
OP_FUNC(SCROLL_DOWN) {
}

// Pinta un scroll hacia arriba
OP_FUNC(SCROLL_UP) {
}

// Establece un título de guardado
OP_FUNC(SAVE_TITLE) { }

OP opcodes[] = {
	OP_DEF(TEXT,             "T"    ),
	OP_DEF(OPTION_ADD,       "PT"   ),
	OP_DEF(JUMP,             "P"    ),
	OP_DEF(SET_RANGE,        "FF2"  ),
	OP_DEF(SET,              "Fc2"  ), OP_NULL,
	OP_DEF(OPTION_RESET,     ""     ),
	OP_DEF(OPTION_SHOW,      ""     ), OP_NULL, OP_NULL,
	OP_DEF(OPTION_RESHOW,    ""     ), OP_NULL, OP_NULL, OP_NULL, OP_NULL, OP_NULL,
	OP_DEF(JUMP_IF_NOT,      "Fc2P" ),
	OP_DEF(WAIT,             "2"    ), OP_NULL, OP_NULL,
	OP_DEF(REPAINT,          "2"    ),	
	OP_DEF(IMAGE_OVERLAY,    "S"    ), OP_NULL,
	OP_DEF(SCRIPT,           "S"    ),
	OP_DEF(GAME_END,         ""     ), OP_NULL, OP_NULL, OP_NULL, OP_NULL,
	OP_DEF(FADE_OUT_BLACK,   ""     ),
	OP_DEF(FADE_OUT_WHITE,   ""     ), OP_NULL, OP_NULL, OP_NULL, OP_NULL, OP_NULL, OP_NULL,
	OP_DEF(MUSIC_PLAY,       "S"    ), OP_NULL,
	OP_DEF(MUSIC_STOP,       ""     ), OP_NULL, OP_NULL,
	OP_DEF(VOICE_PLAY,       "S"    ), OP_NULL, OP_NULL, OP_NULL, OP_NULL,
	OP_DEF(CLIP,             "2222" ), OP_NULL, OP_NULL, OP_NULL, OP_NULL,
	OP_DEF(EFFECT_PLAY,      "S"    ),
	OP_DEF(EFFECT_STOP,      ""     ),
	OP_DEF(MAP_IMAGES,       "SS"   ),
	OP_DEF(MAP_OPTION_RESET, ""     ), OP_NULL, OP_NULL, OP_NULL, OP_NULL, OP_NULL, OP_NULL, OP_NULL,
	OP_DEF(MAP_OPTION_ADD,   "P2222"),
	OP_DEF(MAP_OPTION_SHOW,  ""     ), OP_NULL, OP_NULL, OP_NULL, OP_NULL,
	OP_DEF(FOREGROUND,       "S"    ),
	OP_DEF(BACKGROUND,       "s"    ), OP_NULL, OP_NULL,
	OP_DEF(REPAINT_IN,       "2"    ),
	OP_DEF(CHARA1,           "S"    ),
	OP_DEF(CHARA2,           "SS"   ),
	OP_DEF(ANIMATION,        ""     ),
	OP_DEF(SCROLL_DOWN,      ""     ),
	OP_DEF(SCROLL_UP,        ""     ),
	OP_DEF(SAVE_TITLE,       "T"    ),
};

void script_run() {
	while (script_running) {
		OP op; int v[6];
		int n, vl;

		int op_id = SCRIPT_GET16();
		
		if (op_id < OPCODES_COUNT) {
			op = opcodes[op_id];
			vl = strlen(op.format);
			for (n = 0; n < vl; n++) {
				switch (op.format[n]) {
					case 'P': v[n] = SCRIPT_GET32(); break;
					case '2': v[n] = SCRIPT_GET16(); break;
					case 'F': v[n] = clamp(SCRIPT_GET16(), 0, 999); break;
					case 'c': v[n] = SCRIPT_GET8(); break;
					case 'T': v[n] = (int)SCRIPT_GETSZ(); break;
					case 'S': case 's': v[n] = (int)SCRIPT_GETSZ(); break;
				}
			}
			
			if (op.callback) {
				op.callback(v);
				continue;
			}
		}
		
		printf("\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("Unknown OP 0x%02X\n", op_id);
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("Invalid OP. Probably a new version of the script? Starting room again.\n");
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		
		SCRIPT_JUMP(0);
	}
		
	/*
	switch (op) {	
		case OP_GAME_END: {
			int n;
			game_playing = 0;
			game_end = 1;
			for (n = 0; n < 10; n++) save_s.gallery[n + 140] = 1;
			SYS_SAVE();
			CREDIT_SHOW(0);
		} break;
		
		case OP_SCRIPT_CHANGE: { char *s = STRING_GETSZ_N(); GAME_SCRIPT_LOAD(s); } break;
		
		// ---------------
		//  INPUT         
		// ---------------
		
		case OP_TEXT: GAME_SAVE_POSITION_2(); {
			char *s = STRING_GETSZ_N();
			if (strlen(s)) {
				GAME_TEXT_SNAP();
				GAME_TEXT_PRINT(s, white, clip_text);
				GAME_TEXT_UPDATE();
				GAME_INPUT_WAIT(s);
			}
		} break;

		case OP_TITLE: { char *s = STRING_GETSZ_N(); strcpy(save_title, s); } break;
		
		case OP_OPTION_RESET: GAME_SAVE_POSITION_2(); { // OPTION_RESET
			if (!(save.cursor1 & 0x8000)) {
				printf("back:%08X, cursor1=0\n", save.cursor1);
				save.cursor1 = 0;
			}
			script_options_count = 0;
			save.script_menu_return = scrp - 2 - script;
		} break;
		
		case OP_OPTION: {
			int ptr = SCRIPT_GET32();
			char *s = STRING_GETSZ_N();
			script_options[script_options_count].text = s;
			script_options[script_options_count].ptr = ptr;
			script_options_count++;
		} break;			
		case OP_OPTION_RESHOW: { scrp = script + save.script_menu_return; save.cursor1 |= 0x8000; } break;
		case OP_OPTION_SHOW: { GAME_TEXT_SNAP(); GAME_INPUT_OPTIONS(); } break; // OPTION_SHOW
		case 0x37: GAME_SAVE_POSITION_2(); { // MAP_IMAGES
			char *s1 = STRING_GETSZ_N();
			char *s2 = STRING_GETSZ_N();
			strcpy(map_images[0], s1);
			strcpy(map_images[1], s2);
		} break;
		case 0x38: { map_options_count = 0; } break; // MAP_OPTION_RESET
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
		} break;

		case 0x41: { GAME_INPUT_MAP_OPTIONS(); } break; // MAP_OPTION_SHOW
		case OP_WAIT: { int t = SCRIPT_GET16(); PROGRAM_DELAY(t * 10); } break; // WAIT
		
		
		// ---------------
		//  SOUND RELATED 
		// ---------------
		
		case OP_MUSIC_PLAY: { char *s = STRING_GETSZ_N(); GAME_MUSIC_PLAY(s); } break;
		case OP_MUSIC_STOP: { GAME_MUSIC_STOP(); } break;
		case OP_VOICE_PLAY: { char *s = STRING_GETSZ_N(); GAME_VOICE_PLAY(s); } break;
		case OP_SOUND_PLAY: { char *s = STRING_GETSZ_N(); GAME_SOUND_PLAY(s); } break;
		case OP_SOUND_STOP: { GAME_SOUND_STOP(); } break;
			
		// ---------------
		//  IMAGE RELATED
		// ---------------

		case 0x46: { char *s = STRING_GETSZ_N(); GAME_BACKGROUND_O(s); } break; // BACKGROUND
		case 0x47: { char *s = STRING_GETSZ_N(); GAME_BACKGROUND_I(s); } break; // BACKGROUND_INNER
		case 0x16: { char *s = STRING_GETSZ_N(); GAME_BACKGROUND_I_MASK(s); } break; // IMAGE_MASK
		case 0x4B: { char *s = STRING_GETSZ_N(); GAME_CHARA(s); } break; // CHARACTER
		case 0x4C: { char *s1 = STRING_GETSZ_N(); char *s2 = STRING_GETSZ_N(); GAME_CHARA2(s1, s2); } break; // TWO CHARACTERS

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
		
		case 0x30: { clip.x = SCRIPT_GET16(); clip.y = SCRIPT_GET16(); clip.w = SCRIPT_GET16(); clip.h = SCRIPT_GET16(); } break; // CLIP
		case 0x1E: { GAME_BUFFER_FADEOUT(); PROGRAM_DELAY(250); } break; // FADE_OUT
		case 0x1F: { GAME_BUFFER_FADEOUT2(); PROGRAM_DELAY(250); } break; // FADE_OUT_WHITE
		case 0x4A: { int t = SCRIPT_GET16(); GAME_BUFFER_REPAINT(t); } break; // BUFFER_REPAINT_INNER
		case 0x14: { int t = SCRIPT_GET16(); GAME_BUFFER_REPAINT(t); } break; // BUFFER_REPAINT

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
	*/
}
