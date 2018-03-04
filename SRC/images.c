//#define DEBUG_IMAGES

////////////////////////////////////////////////////////////
// IMAGES                                                 //
////////////////////////////////////////////////////////////

SDL_Surface *SDL_CompositeImage(SDL_Surface *color, SDL_Surface *alpha) {
	SDL_PixelFormat pf;

	if (!alpha) return color;
	if (!color) return NULL;
	
	{
		pf.palette = NULL; pf.BitsPerPixel = 32; pf.BytesPerPixel = 4;
		pf.Rmask = 0xFF000000; pf.Gmask = 0x00FF0000; pf.Bmask = 0x0000FF00; pf.Amask = 0x000000FF;
		pf.Rshift = 24; pf.Gshift = 16; pf.Bshift = 8; pf.Ashift = 0;
		pf.Rloss = pf.Gloss = pf.Bloss = pf.Aloss = 0;
		pf.colorkey = 0; pf.alpha = 0xFF;
	}
	
	//SDL_Surface *r = SDL_ConvertSurface(color, &pf, SDL_SRCALPHA | SDL_SWSURFACE);
	
	SDL_Surface *r = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, alpha->w, alpha->h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	
	if (!r) PROGRAM_EXIT_ERROR("Can't composite image\n");

	SDL_BlitSurface(color, NULL, r, NULL);
	
	//SDL_FreeSurface(color);
	
	SDL_LockSurface(r); SDL_LockSurface(alpha);

	{	
		uint *d = r->pixels;
		ubyte *a = alpha->pixels;
		int n, count = r->w * r->h;
		//for (n = 0; n < count; n++, d++, a++) *d = (*d & ~0xFF) | *a;
		SDL_Color *cols = alpha->format->palette->colors;
		for (n = 0; n < count; n++, d++, a++) *d = (*d & ~0xFF) | cols[*a].r;
	}
	
	SDL_UnlockSurface(alpha); SDL_UnlockSurface(r);
	
	//SDL_FreeSurface(alpha);
	return r;
}

#define INTRINSIC_CHARACTERS

#ifdef INTRINSIC_CHARACTERS
//char *ichars_data = NULL;
SDL_RWops *ichars[256] = {NULL};

void ICHAR_process(int dummy_size) {
	int count;
	int n;
	if (!ichars_data) PROGRAM_EXIT_ERROR("Can't process ichar");
	count = *((int *)ichars_data);
	//printf("ICHAR_process... (%d)\n", count);
	for (n = 0; n < count; n++) {
		int *off = (int*)(ichars_data + 4 + 8 * n);
		if (off[0] && off[1]) {
			ichars[n] = SDL_RWFromMem(ichars_data + off[0], off[1]);
			/*
			printf("%d: %08X,%08X", n, off[0], off[1]);
			printf("(%08X,%08X,%d)", ichars_data, ichars_data + off[0], off[0]);
			printf("(%c%c%c%c)", ichars_data[off[0]], ichars_data[off[0] + 1], ichars_data[off[0] + 2], ichars_data[off[0] + 3]);
			printf("\n");
			*/
		} else {
			ichars[n] = NULL;
		}
	}
}

void ICHAR_load_process() {
	ICHAR_process(size_ichars_data);
}

/*void ICHAR_load_process() {
	int size;
	SDL_RWops *f = SDL_RWFromFile(FILE_PREFIX "ICMP.DAT", "rb");
	printf("ICHAR_load_process...");
	if (!f) { printf("(Not found)\n"); return; }
	ichars_data = malloc(size = SDL_RWseek(f, 0, SEEK_END));
	if (!ichars_data) PROGRAM_EXIT_ERROR("Can't load ichar");
	SDL_RWseek(f, 0, SEEK_SET);
	SDL_RWread(f, ichars_data, 1, size);
	SDL_RWclose(f);
	
	printf("(%d)\n", size);
	
	ICHAR_process();
}*/

SDL_Surface *CompositeCharacterGetColor(int type, int alt) {
	SDL_Surface *r;
	//sprintf(temp, "ICMP/B%02d_C.png", temp);
	//return IMG_Load(temp);
	int idx = (type - 1) * 4 + alt * 2 + 0;
	printf("Try Load Color: %d\n", idx);
	if (!ichars[idx]) {
		printf("Failed\n");
		return NULL;
	}
	SDL_RWseek(ichars[idx], 0, SEEK_SET);
	r = IMG_Load_RW(ichars[idx], 0);
	printf("Ok\n");
	return r;
}

SDL_Surface *CompositeCharacterGetMask(int type, int alt) {
	SDL_Surface *r;
	//char temp[50];
	//sprintf(temp, "ICMP/B%02d_M.png", temp);
	//return IMG_Load(temp);
	int idx = (type - 1) * 4 + alt * 2 + 1;
	printf("Try Load Mask: %d\n", idx);
	if (!ichars[idx]) {
		printf("Failed\n");
		return NULL;
	}
	SDL_RWseek(ichars[idx], 0, SEEK_SET);
	
	r = IMG_Load_RW(ichars[idx], 0);
	
	printf("Ok\n");
	
	return r;
}

void colorTransformSaturate(float *c) {
	if (*c < 0.0f) *c = 0.0f;
	if (*c > 1.0f) *c = 1.0f;
}

void colorTransform(int type, float *r, float *g, float *b, float *a) {
	switch (type) {
		case 1:
			*r = *r * 0.90f + 0.0f;
			*g = *g * 0.65f - 0.1f;
			*b = *b * 0.65f - 0.1f;
		break;
		case 2:
			*b = *b * 0.90f + 0.0f;
			*g = *g * 0.65f - 0.1f;
			*r = *r * 0.65f - 0.1f;
		break;
	}
	
	colorTransformSaturate(r);
	colorTransformSaturate(g);
	colorTransformSaturate(b);
	colorTransformSaturate(a);
}

void colorTransformC(int type, SDL_Color* c) {
	float r, g, b, a;
	r = ((float)c->r) / 255.0f;
	g = ((float)c->g) / 255.0f;
	b = ((float)c->b) / 255.0f;
	//a = ((float)c->a) / 255.0f;
	colorTransform(type, &r, &g, &b, &a);
	c->r = (int)(r * 255.0f);
	c->g = (int)(g * 255.0f);
	c->b = (int)(b * 255.0f);
	//c->a = (int)(*a * 255.0f);
}

int SDL_GetPixel(SDL_Surface *s, int x, int y) {
	if (x < 0 || y < 0) return -1;
	if (x >= s->w || y >= s->h) return -1;
	return ((char *)s->pixels)[y * s->pitch + x];
}

SDL_Surface *CompositeCharacter(int type, int alt, int face, int palette) {
	SDL_Surface *c = CompositeCharacterGetColor(type, alt);
	SDL_Surface *m = CompositeCharacterGetMask(type, alt);
	SDL_Surface *r = NULL;

	// Show character image in VMU
	printf("CompositeCharacter(%d, %d, %d, %d)\n", type, alt, face, palette);
	#ifdef DREAMCAST
		printf("DREAMCAST\n");
		dc_draw_vmu_icon(vmu_chars + 192 * (type - 1), size_vmu_chars / 192);
	#endif
	
	if (!c || !m) {
		if (c) SDL_FreeSurface(c);
		if (m) SDL_FreeSurface(m);
		printf("Not loaded images (%d,%d)\n", c, m);
		return NULL;
	} else {
		printf("Loaded images");
	}
	
	//if (!c) PROGRAM_EXIT_ERROR("Can't composite character (color)\n");
	//if (!m) PROGRAM_EXIT_ERROR("Can't composite character (mask)\n");

	if (face > 1) {
		int cx, cy, sc, xm = 0x7FFFFFFF, xM = 0, ym = 0x7FFFFFFF, yM = 0;
		int x = 107, y = 114, w = 61, h = 43;
		SDL_Rect face_s, face_d;

		sc = SDL_GetPixel(c, 0, 0);
		for (cx = 0; cx < c->w; cx++) {
			if (sc != SDL_GetPixel(c, cx, c->h - 1)) {
				if (cx < xm) xm = cx;
				if (cx > xM) xM = cx;
			}
		}
		
		x = xm; w = xM - xm + 1;

		for (cy = 0; cy < c->h; cy++) {
			if (sc != SDL_GetPixel(c, c->w - 1, cy)) {
				if (cy < ym) ym = cy;
				if (cy > yM) yM = cy;
			}
		}
		
		y = ym; h = yM - ym + 1;

		face_d.x = x;
		face_d.y = y;
		face_d.w = w;
		face_d.h = h;
		
		face_s.x = w * (face - 2);
		face_s.y = c->h - h - 1;
		face_s.w = w;
		face_s.h = h;
		
		SDL_BlitSurface(c, &face_s, c, &face_d);
	}

	if (c && c->format && c->format->palette) {	
		SDL_Palette *pal = c->format->palette;
		int n; for (n = 0; n < pal->ncolors; n++) colorTransformC(palette, &pal->colors[n]);		
	}
	
	r = SDL_CompositeImage(c, m);
	
	if (c) SDL_FreeSurface(c);
	if (m) SDL_FreeSurface(m);
	
	return r;
}

SDL_Surface *CompositeCharacterName(char *name) {
	int matched = 0, type = 1, face = 1, palette = 0, alt = 0; char palette_c;
	
	printf("Trying CompositeCharacterName('%s'): ", name);
	if (!ichars_data) { printf("Not loaded ichar_data\n"); return NULL; }
	if (toupper(name[0]) != 'B') { printf("Doesn't match 'B'\n"); return NULL; }

	//name = "b03_2b.bmP";
	//name = "b10A_1a";
	
	/*
	type = 10;
	face = 2;
	palette_c = 'C';
	*/

	if ((matched = sscanf(name + 1, "%d_%d%c", &type, &face, &palette_c) != 3)) {
		if ((matched = sscanf(name + 4, "_%d%c", &face, &palette_c) != 2)) {
			printf("Doesn't match params (matched:%d)\n", matched); return NULL;
		} else {
			alt = 1;
		}
	} else {
		alt = 0;
	}
	
	palette = toupper(palette_c) - 'A';
	printf("Correct: type:%d, face:%d, palette:%d\n", type, face, palette);
	
	return CompositeCharacter(type, alt, face, palette);
}
SDL_Surface *character_img = NULL;
#endif

#define fadeOutPixel_c(c)  if (*c >= param) *c -= param; else *c = 0;
#define fadeOut2Pixel_c(c) if (*c >= 0x100 - param) *c += param; else *c = 0xFF;

void fadeOutPixel(int param, ubyte *r, ubyte *g, ubyte *b, ubyte *a) {
	fadeOutPixel_c(r); fadeOutPixel_c(g); fadeOutPixel_c(b); fadeOutPixel_c(a);
}

void fadeOut2Pixel(int param, ubyte *r, ubyte *g, ubyte *b, ubyte *a) {
	fadeOut2Pixel_c(r); fadeOut2Pixel_c(g); fadeOut2Pixel_c(b); fadeOut2Pixel_c(a);
}

#define processSurface_iterate() { \
	SDL_LockSurface(s); \
	cur = s->pixels; \
	end = s->pixels + s->pitch * s->h; \
	for (;cur < end; cur++) { \
		SDL_GetRGBA(*cur, pf, &r, &g, &b, &a); \
		func(param, &r, &g, &b, &a); \
		*cur = SDL_MapRGBA(pf, r, g, b, a); \
	} \
	SDL_UnlockSurface(s); \
}

void processSurface(SDL_Surface *s, SDL_Rect *rect, void (*func)(int, ubyte*, ubyte*, ubyte*, ubyte*), int param) {
	SDL_PixelFormat *pf = s->format;
	ubyte r, g, b, a;
	switch (pf->BytesPerPixel) {
		case 3: case 1: return; // Not supported (8 bits nor 24 bits)
		case 2: { ushort *cur, *end; processSurface_iterate(); } break; // 16 bits
		case 4: { uint *cur, *end; processSurface_iterate(); } break; // 32 bits
	}
}

#define PROCESS_V(idx) v[idx] = (*ptr >> (idx * 8)) & 0xFF; v[idx] -= step; if (v[idx] < 0) v[idx] = 0;
#define PROCESS_V2(idx) v[idx] = (*ptr >> (idx * 8)) & 0xFF; v[idx] += step; if (v[idx] > 0xFF) v[idx] = 0xFF;
#define PROCESS_I(idx) (v[idx] << (idx * 8))

void GAME_BUFFER_FADEOUT_EX(int step) {
	int v[4];
	int n, m, l = SCREEN_WIDTH * SCREEN_HEIGHT;
	int *ptr;
/*
	#ifdef DREAMCAST
		return;
	#endif
*/

	m = (0x100 / step) + 1;
	while (m--) {
		if (screen->format->BytesPerPixel == 4) {
		//if (1) {
			SDL_LockSurface(screen);
			ptr = screen->pixels;
			
			for (n = 0; n < l; n++, ptr++) {
				PROCESS_V(0); PROCESS_V(1); PROCESS_V(2); PROCESS_V(3);
				*ptr = PROCESS_I(0) | PROCESS_I(1) | PROCESS_I(2) | PROCESS_I(3);
			}
			SDL_UnlockSurface(screen);			
		} else {
			processSurface(screen, NULL, fadeOutPixel, step);
		}
		GAME_BUFFER_REPAINT(-1);
	}

	//PROGRAM_DELAY(500);
}

void GAME_BUFFER_FADEOUT() {
	GAME_BUFFER_FADEOUT_EX(8);
}

void GAME_BUFFER_FADEOUT2_EX(int step) {
	int v[4];
	int n, m, l = SCREEN_WIDTH * SCREEN_HEIGHT;
	int *ptr;
/*
	#ifdef DREAMCAST
		return;
	#endif
*/
	m = (0x100 / step) + 1;
	while (m--) {
		if (screen->format->BytesPerPixel == 4) {
			SDL_LockSurface(screen);
			ptr = screen->pixels;
			
			for (n = 0; n < l; n++, ptr++) {
				PROCESS_V2(0); PROCESS_V2(1); PROCESS_V2(2); PROCESS_V2(3);
				*ptr = PROCESS_I(0) | PROCESS_I(1) | PROCESS_I(2) | PROCESS_I(3);
			}
			SDL_UnlockSurface(screen);			
		} else {
			processSurface(screen, NULL, fadeOut2Pixel, step);
		}
		GAME_BUFFER_REPAINT(-1);
	}

	//PROGRAM_DELAY(500);
}

void GAME_BUFFER_FADEOUT2() {
	GAME_BUFFER_FADEOUT2_EX(7);
}

int SDL_RWsize(SDL_RWops *f) {
	int len;
	int bpos = SDL_RWtell(f);
	len = SDL_RWseek(f, 0, SEEK_END);
	SDL_RWseek(f, bpos, SEEK_SET);
	return len;
}

void reduceMemory() {
	PROGRAM_EXIT_ERROR("Can't load image (Out of memory?)");
}

SDL_Surface *GAME_IMAGE_GET_EX3(char *_name1, char *_name2, int usecache, int scale) {
	int n;
	int hasTransparent = 0;
	int checkTransparent = (screen_format.BitsPerPixel == 16);
	int mustCleanAlphaFix = 0;
	SDL_RWops *f;
	SDL_Surface *surface = NULL, *surface_o = NULL, *surface_o1 = NULL, *surface_o2 = NULL;
	char name[0x80] = {0}, name1[0x40] = {0}, name2[0x40] = {0};
	if (_name1) sprintf(name1, strrchr(_name1, '.') ? "%s" : "%s.BMP", _name1);
	if (_name2) sprintf(name2, strrchr(_name2, '.') ? "%s" : "%s.BMP", _name2);
	sprintf(name, "%s#%s", name1, name2);
	
	#ifdef DEBUG_IMAGES
		printf("I: '%s'", name);
	#endif
	
	MEMORY_DEBUG();
	
	#ifdef INTRINSIC_CHARACTERS
		{
			surface_o1 = CompositeCharacterName(_name1);
			if (surface_o1) {
				printf("[0]");
				usecache = 0;
				if (character_img) { SDL_FreeSurface(character_img); character_img = NULL; }
				character_img = surface_o1;
				printf("[1]");
			}
		}
	#endif
	
	if (usecache) {
		for (n = 0; n < IMAGE_CACHE_MAX; n++) {
			// We have file in cache
			if ((stricmp(image_cache[n].name, name) == 0) && image_cache[n].surface) {
				#ifdef DEBUG_IMAGES
					printf("| (ALREADY CACHED)");
				#endif
				surface = image_cache[n].surface;
				image_cache[n].used++;
				break;
			}
		}
	}

	if (!surface) {
		if (!surface_o1) {
			if (_name1) {
				#ifdef DEBUG_IMAGES
					printf("| L('%s')...", name1);
				#endif
				if ((f = VFS_LOAD(name1)) != NULL) {
					while (1) {
						SDL_RWseek(f, 0, SEEK_SET);
						if ((surface_o1 = IMG_LoadBMP_RW(f)) != NULL) break;
						reduceMemory();
					}
					
					SDL_RWclose(f);
				} else {
					#ifdef DEBUG_IMAGES
						printf("| (Can't load)");
					#endif
				}
			}

			if (_name2) {
				#ifdef DEBUG_IMAGES
					printf("| L('%s')...", name2);
				#endif
				if ((f = VFS_LOAD(name2)) != NULL) {
					while (1) {
						SDL_RWseek(f, 0, SEEK_SET);
						if ((surface_o2 = IMG_LoadBMP_RW(f)) != NULL) break;
						reduceMemory();
					}
					SDL_RWclose(f);
				} else {
					#ifdef DEBUG_IMAGES
						printf("| (Can't load)");
					#endif
				}
			}
		}
		
		if (!surface_o1 && !surface_o2) {
			#ifdef DEBUG_IMAGES
				printf("\n");
			#endif
			return NULL;
		}
		
		if (surface_o1 && surface_o2) {
			while (1) {
				if ((surface_o = SDL_CompositeImage(surface_o1, surface_o2)) != NULL) break;
				reduceMemory();
			}
			SDL_FreeSurface(surface_o1);
			SDL_FreeSurface(surface_o2);
		} else {
			if (surface_o1) surface_o = surface_o1;
			if (surface_o2) surface_o = surface_o2;
			
			if (surface_o->format->BitsPerPixel != 8) {
				if (toupper(name1[0]) == 'I' && name1[1] == '_' && name1[2] == '1' && name1[3] == '7') {
					SDL_SetColorKey(surface_o, SDL_SRCCOLORKEY, SDL_MapRGB(surface_o->format, 0x53, 0xFF, 0x00));
				} else {
					SDL_SetColorKey(surface_o, SDL_SRCCOLORKEY, SDL_MapRGB(surface_o->format, 0x00, 0xFF, 0x00));
				}
			}
			
			mustCleanAlphaFix = 1;
			if (toupper(name[0]) == 'B') mustCleanAlphaFix = 0;
		}

		if (SCREEN_MUST_SCALE && scale) {
			while (1) {
				if ((surface = SDL_ZoomSurface(surface_o, ASPECT_X, ASPECT_Y)) != NULL) break;
				reduceMemory();
			}
			if (character_img == surface_o) character_img = surface;
			SDL_FreeSurface(surface_o);
			
			if (mustCleanAlphaFix) {
				int n, l = surface->w * surface->h;
				int *ptr;
				SDL_LockSurface(surface);
				ptr = surface->pixels;
				
				if (checkTransparent) {
					for (n = 0; n < l; n++, ptr++) if (((*ptr >> 24) & 0xFF) != 0xFF) { *ptr = 0; hasTransparent = 1; }
				} else {
					for (n = 0; n < l; n++, ptr++) if (((*ptr >> 24) & 0xFF) != 0xFF) *ptr = 0;
				}
				SDL_UnlockSurface(surface);			
			} else {				
			}
		} else {
			surface = surface_o;

			if (checkTransparent) {
				int n, l = surface->w * surface->h;
				int *ptr;
				SDL_LockSurface(surface);
				ptr = surface->pixels;
				for (n = 0; n < l; n++, ptr++) if (((*ptr >> 24) & 0xFF) != 0xFF) { hasTransparent = 1; break; }
				SDL_UnlockSurface(surface);			
			}
		}

		if (usecache) {
			int npos = 0;
			int min = 0xFFFFFFF, max = 0;
			
			for (n = 0; n < IMAGE_CACHE_MAX; n++) {
				int cused = image_cache[n].used;
				if (cused < min) {
					npos = n;
					min = cused;
				}
				if (cused > max) max = cused;
			}
			
			#ifdef DEBUG_IMAGES
				printf("\n");
				for (n = 0; n < IMAGE_CACHE_MAX; n++) {
					printf("%d,", image_cache[n].used);
				}
				
				printf(" (%d) | %d", npos, IMAGE_CACHE_MAX);
			#endif
			
			if (image_cache[npos].surface) SDL_FreeSurface(image_cache[npos].surface);
			
			if (checkTransparent && !hasTransparent) {
				SDL_Surface *surface2 = surface;
				//printf("AAAAAAAAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaaaa****************************AAA\n");
				surface = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h, screen_format.BitsPerPixel, screen_format.Rmask, screen_format.Gmask, screen_format.Bmask, screen_format.Amask);
				SDL_BlitSurface(surface2, NULL, surface, NULL);
				SDL_FreeSurface(surface2);
			}
			
			strcpy(image_cache[npos].name, name);
			image_cache[npos].surface = surface;
			image_cache[npos].used = max + 1;
			#ifdef DEBUG_IMAGES
				printf("| CACHED");
			#endif			
		} else {
			#ifdef DEBUG_IMAGES
				printf("| NO CACHED");
			#endif
		}
	}
	
	#ifdef DEBUG_IMAGES
		printf("\n");
	#endif
	
	return surface;
}

SDL_Surface *GAME_IMAGE_GET_EX2(char *_name1, char *_name2, int usecache) {
	return GAME_IMAGE_GET_EX3(_name1, _name2, usecache, 1);
}

SDL_Surface *GAME_IMAGE_GET_EX(char *name, int usecache) {
	return GAME_IMAGE_GET_EX2(name, NULL, usecache);
}

SDL_Surface *GAME_IMAGE_GET(char *name) {
	return GAME_IMAGE_GET_EX(name, 1);
}

void GAME_BACKGROUND(char *name, SDL_Rect clip, int colorKey) {
	SDL_Surface *surface;
	if ((surface = GAME_IMAGE_GET(name)) != NULL) {
		if (colorKey) SDL_SetColorKey(surface, SDL_SRCCOLORKEY, SDL_MapRGB(surface->format, 0x00, 0xFF, 0x00));
		SDL_BlitSurface(surface, NULL, screen, &clip);
		//SDL_FreeSurface(surface);
	}
	
	#ifdef DREAMCAST
		printf("CLEAR VMU IMAGE\n");
		dc_draw_vmu_icon(NULL, 0);
	#endif	
}

void GAME_BACKGROUND_O(char *name) {
	if (name) {
		strcpy(save.background_o, name);
		strcpy(save.background_i, " ");
		strcpy(save.background_v, " ");
		strcpy(save.character_1 , " ");
		strcpy(save.character_2 , " ");
	}

	// Tipo de interfaz
	switch (save.background_o[5]) {
		default: case 'A': case 'a': menu_button_type = 0; break;
		case 'B': case 'C': case 'b': case 'c': menu_button_type = 1; break;
	}
	
	GAME_BACKGROUND(save.background_o, clip_bg1, 0);
}

void GAME_BACKGROUND_I(char *name) {
	if (name) {
		strcpy(save.background_i, name);
		strcpy(save.background_v, " ");
		strcpy(save.character_1, " ");
		strcpy(save.character_2, " ");
	}
	FLIST_CHECK(save.background_i);
	GAME_BACKGROUND(save.background_i, clip_bg2, 0);
}

void GAME_BACKGROUND_I_MASK(char *name) {
	if (name) {
		strcpy(save.background_v, name);
	}
	GAME_BACKGROUND(save.background_v, clip_bg2, 1);
}

void GAME_CHARA_EX(char *_name, SDL_Rect clip) {
	SDL_Surface *surface = NULL;
	char name1[0x40], name2[0x40]; char* under;

	strcpy(name1, _name);
	strcpy(name2, _name);
	if ((under = strrchr(name2, '_')) != '\0') { under[0] = '_'; under[1] = '0'; under[2] = 0; }
	
	if ((surface = GAME_IMAGE_GET_EX2(name1, name2, 1)) == NULL) return;
	clip.x += clip.w / 2 - surface->w / 2;
	clip.y += clip.h - surface->h;
	SDL_BlitSurface(surface, NULL, screen, &clip);
	//printf("  CHAR_CLIP(%d, %d)\n", clip.x, clip.y);
	
	#ifdef INTRINSIC_CHARACTERS
		if (character_img) {
			SDL_FreeSurface(character_img);
			character_img = NULL;
		}
	#endif
}

void GAME_CHARA(char *name) {
	if (name) {
		strcpy(save.character_1, name);
		strcpy(save.character_2, " ");
	}
	GAME_CHARA_EX(save.character_1, clip_bg2);
	GAME_BUFFER_REPAINT(-2);
}

void GAME_CHARA2(char *name1, char *name2) {
	if (name1) strcpy(save.character_1, name1);
	if (name2) strcpy(save.character_2, name2);
	if (name2) {
		SDL_Rect clip = clip_bg2;
		clip.w /= 2;
		GAME_CHARA_EX(save.character_1, clip);
		clip.x += clip.w;
		GAME_CHARA_EX(save.character_2, clip);
		GAME_BUFFER_REPAINT(-2);
	} else {
		GAME_CHARA(NULL);
	}
}
