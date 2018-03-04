char* CREDIT_READ() {
	char *r;
	int len;
	SDL_RWops *f = STREAM_UNCOMPRESS_MEM(roll_txt, size_roll_txt);
	len = SDL_RWseek(f, 0, SEEK_END);
	SDL_RWseek(f, 0, SEEK_SET);
	r = malloc(len + 1);
	SDL_RWread(f, r, 1, len);
	r[len] = 0;
	SDL_RWclose(f);
	return r;
}

void SDL_DrawSurface(SDL_Surface* s, int x, int y) {
	SDL_Rect r = {x, y};
	SDL_BlitSurface(s, NULL, screen, &r);
}

SDL_Surface *CREDIT_SF_RESIZE(SDL_Surface *in) {
	SDL_Surface *out;
	
	#define CREDIT_PROP ((double)screen->w / 640)
	
	out = SDL_ZoomSurface(in, CREDIT_PROP, CREDIT_PROP);
	
	SDL_FreeSurface(in);
	return out;
}

void CREDIT_SHOW(int can_exit) {
	TTF_Font* font_h;
	TTF_Font* font_t;
	char *line;
	char *credit;
	char *rcredit;
	//char temp[0x80];
	int n;
	int fps;
	int nlines = 0;
	SDL_Surface *extra[2];
	SDL_Surface *lines[256] = { NULL };
	SDL_Color c_t = {0xFF, 0xFF, 0xFF, 0xFF};
	SDL_Color c_h = {0xD0, 0xD0, 0xD0, 0xFF};
	
	#ifdef DREAMCAST
	fps = 20;
	#else
	//fps = 50;
	fps = 25;
	#endif
	
	/*
	SDL_Surface *roll[CREDIT_IMAGES];
	// Load rolls
	for (n = 0; n < CREDIT_IMAGES; n++) {
		sprintf(temp, "RES/END/ROLL%d.png", n);
		roll[n] = IMG_Load(temp);
	}
	*/
	
	hide_debug++;
	
	credit = CREDIT_READ();
	
	extra[0] = CREDIT_SF_RESIZE(IMG_Load_RW(SDL_RWFromConstMem(end0_png, size_end0_png), 1));
	extra[1] = CREDIT_SF_RESIZE(IMG_Load_RW(SDL_RWFromConstMem(end1_png, size_end1_png), 1));
	
	font_h = TTF_OpenFontRW(SDL_RWFromMem(font_ttf, size_font_ttf), 1, (int)(font_size_corrected * 1.4));
	font_t = TTF_OpenFontRW(SDL_RWFromMem(font_ttf, size_font_ttf), 1, (int)(font_size_corrected * 1.1));
	
	for (rcredit = credit; (line = strtok(rcredit, "\n")) != NULL; rcredit = NULL) {
		printf("%d:%s\n", nlines, line);
		switch (line[0]) {
			case '.': // Void
				lines[nlines++] = SDL_CreateRGBSurface(SDL_SWSURFACE, 1, (38 * screen->h) / 480, 32, 0, 0, 0, 0);
			break;
			case '@': // Title
				lines[nlines++] = TTF_RenderText_Shaded(font_h, line + 1, c_h, black);
				lines[nlines++] = SDL_CreateRGBSurface(SDL_SWSURFACE, 1, 2, 32, 0, 0, 0, 0);
			break;
			default: // Text
				lines[nlines++] = TTF_RenderText_Shaded(font_t, line, c_t, black);
			break;
			case '#': // Image
				sscanf(line, "#%d", &n);
				//sprintf(temp, "RES/END/ROLL%d.png", n);
				//lines[nlines++] = CREDIT_SF_RESIZE(IMG_Load(temp));
				{
					SDL_RWops *f = NULL;
					switch (n) {
						case 0: f = STREAM_UNCOMPRESS_MEM(roll0_png, size_roll0_png); break;
						case 1: f = STREAM_UNCOMPRESS_MEM(roll1_png, size_roll1_png); break;
						case 2: f = STREAM_UNCOMPRESS_MEM(roll2_png, size_roll2_png); break;
						default: continue;
					}
					lines[nlines++] = CREDIT_SF_RESIZE(IMG_Load_RW(f, 1));
				}
			break;
			case '*': nlines++; break;			
		}
	}
	
	int sy = screen->h;
	//int sy = -2000;

	while (1) {
		int start = SDL_GetTicks();
		KEYS_UPDATE();
		
		if (can_exit && ((keys & K_A) || (keys & K_B))) goto _cleanup;
		
		int y = sy;
		int ey = 0;
		SDL_FillRect(screen, NULL, 0);
		for (n = 0; n < nlines; n++) {
			if (lines[n]) {
				SDL_DrawSurface(lines[n], screen->w / 2 - lines[n]->w / 2, y);
				y += lines[n]->h;
			} else {
				ey = y;
			}
		}
		
		#ifdef DREAMCAST
			sy -= 4;
		#else
			sy -= 2;
			//sy -= 3;
		#endif

		#ifdef EFFECT_CREDITS
			SDL_DrawSurface(extra[0], 0, 0);
			SDL_DrawSurface(extra[1], 0, screen->h - extra[1]->h);
		#endif
		
		GAME_BUFFER_REPAINT(0);
		
		while (SDL_GetTicks() < start + (1000 / fps)) PROGRAM_DELAY(1);
		
		PROGRAM_DELAY(0);

		if (ey == 0) ey = y;
		if ((ey < (screen->h / 2) - (y - ey) / 2)) break;
	}
	
	SDL_Delay(2000);
	GAME_BUFFER_FADEOUT_EX(4);
	SDL_Delay(400);

_cleanup:

	hide_debug--;

	// Release
	free(credit);
	TTF_CloseFont(font_h);
	TTF_CloseFont(font_t);
	SDL_FreeSurface(extra[0]);
	SDL_FreeSurface(extra[1]);
	for (n = 0; n < nlines; n++) SDL_FreeSurface(lines[n]);
}
