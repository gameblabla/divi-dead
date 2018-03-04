#include "video.h"
#include "hooks.h"

SDL_Surface *screen;

#define SCREEN_WIDTH  HOOK_VIDEO_WIDTH
#define SCREEN_HEIGHT HOOK_VIDEO_HEIGHT

void video_init() {
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	
	HOOK_INIT_VIDEO_PRE();
	
	screen = SDL_SetVideoMode(
		HOOK_VIDEO_WIDTH,
		HOOK_VIDEO_HEIGHT,
		HOOK_VIDEO_BPP,
		HOOK_VIDEO_FLAGS
	);
}

SDL_Surface *SDL_CompositeImage(SDL_Surface *color, SDL_Surface *alpha) {
	if (!alpha) return color;
	if (!color) return NULL;
	
	SDL_Surface *r = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, alpha->w, alpha->h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);
	
	if (!r) return NULL;

	SDL_BlitSurface(color, NULL, r, NULL);
	
	SDL_LockSurface(r); SDL_LockSurface(alpha);

	{	
		unsigned int *d = r->pixels;
		unsigned char *a = alpha->pixels;
		int n, count = r->w * r->h;
		SDL_Color *cols = alpha->format->palette->colors;
		for (n = 0; n < count; n++, d++, a++) *d = (*d & ~0xFF) | cols[*a].r;
	}
	
	SDL_UnlockSurface(alpha); SDL_UnlockSurface(r);
	
	return r;
}

typedef float MAT4[4][4], VEC4[4], VEC3[3];

void mult_VEC4_MAT4(VEC4 v, MAT4 m, VEC4 r) {
	int i, j;
	for (i = 0; i < 4; i++) for (j = 0, r[i] = 0.0f; j < 4; j++) r[i] += v[j] * m[j][i];
}

MAT4 palettes[] = {
	1.00, 0.00, 0.00, 0.00,
	0.00, 1.00, 0.00, 0.00,
	0.00, 0.00, 1.00, 0.00,
	0.00, 0.00, 0.00, 0.00,

	1.00, 0.00, 0.00, 0.00,
	0.00, 0.74, 0.00, 0.00,
	0.00, 0.00, 0.74, 0.00,
	0.00, 0.00, 0.00, 0.00,

	0.74, 0.00, 0.00, 0.00,
	0.00, 0.74, 0.00, 0.00,
	0.00, 0.00, 1.00, 0.00,
	0.00, 0.00, 0.00, 0.00,
};

int clamp(int v, int m, int M) { if (v < m) return m; if (v > M) return M; return v; }

#define fconv_to(v) clamp((v) * 255.0f, 0, 0xFF);

void game_pal_transform_color(SDL_Color* c, MAT4 t) {
	VEC4 v, vr;
	v[0] = ((float)c->r) / 255.0f;
	v[1] = ((float)c->g) / 255.0f;
	v[2] = ((float)c->b) / 255.0f;
	v[3] = 1.0f;
	
	mult_VEC4_MAT4(v, t, vr);
	
	c->r = fconv_to(vr[0]);
	c->g = fconv_to(vr[1]);
	c->b = fconv_to(vr[2]);
}

void game_pal_transform(SDL_Palette* pal, MAT4 t) {
	int n, count = pal->ncolors;
	for (n = 0; n < count; n++) {
		SDL_Color *c = &pal->colors[n];
		game_pal_transform_color(c, t);
	}
}

SDL_Surface *game_get_character_ex(int type, int alt, int face, int pal) {
	SDL_Surface *mask = NULL, *color = NULL, *image = NULL;
	char temp[0x30];
	sprintf(temp, "B%02d%s_%d", type, alt ? "A" : "", 0   ); mask  = resman_get_image(temp, 1);
	sprintf(temp, "B%02d%s_%d", type, alt ? "A" : "", face); color = resman_get_image(temp, 1);
	
	if (color) game_pal_transform(color->format->palette, palettes[pal]);
	
	image = SDL_CompositeImage(color, mask);
	
	SDL_FreeSurface(mask);
	SDL_FreeSurface(color);
	
	return image;
}

SDL_Surface *game_get_character(char* name) {
	int type = 1, face = 1, alt = 0, pal;
	char pal_c;
	
	if (name[0] != 'B') return;
	
	if (sscanf(name + 1, "%d_%d%c", &type, &face, &pal_c) != 3) {
		if (sscanf(name + 4, "_%d%c", &face, &pal_c) != 2) return NULL;
		alt = 1;
	}	
	
	pal = pal_c - 'A';
	
	return game_get_character_ex(type, alt, face, pal);
}

void draw_surface(SDL_Surface* s, int x, int y) {
	SDL_Rect r;
	r.x = x;
	r.y = y;
	SDL_BlitSurface(s, NULL, screen, &r);
}

void draw_character(char *name, int x, int y) {
	SDL_Surface* s = game_get_character(name);
	if (s == NULL) { printf("Can't draw character1 '%s'\n", name); return; }
	draw_surface(s, x, y);
	SDL_FreeSurface(s);
}

void video_character1_draw(char *name1) {
	draw_character(name1, 176, 8);
}

void video_character2_draw(char *name1, char *name2) {
	draw_character(name1, 80, 8);
	draw_character(name2, 282, 8);
}

void video_foreground_draw(char* name) {
	SDL_Surface* s = resman_get_image(name, 1);
	if (s == NULL) { printf("Can't draw foreground '%s'\n", name);return; }
	draw_surface(s, 0, 0);
	SDL_FreeSurface(s);
}

void video_background_draw(char* name) {
	SDL_Surface* s = resman_get_image(name, 1);
	if (s == NULL) { printf("Can't draw background '%s'\n", name);return; }
	draw_surface(s, 32, 8);
	SDL_FreeSurface(s);
}

void video_update_start() {
}

void video_update_add(SDL_Rect r) {
	SDL_UpdateRect(screen, r.x, r.y, r.w, r.h);
}

void video_update_end(int time) {
	SWAIT(time);
}

void video_transition_play(int effect) {
	SDL_Rect r;
	int n, m, y, y2, x, steps;
	switch (effect) {
		default:
			video_update_start();
			video_update_add(SRECT(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
			video_update_end(0);
		break;
		case 1:
			steps = SCREEN_HEIGHT / 8;
			for (n = 0, m = 0; n < steps; n++) {
				video_update_start();
				for (y = 0, y2 = n; y <= SCREEN_HEIGHT; y += 16, y2--) {
					if (y2 < 0) break;
					if (y2 >= 16) continue;
					video_update_add(SRECT(0, y - y2, SCREEN_WIDTH, 1));
				}
				video_update_end(1000 / steps);
			}
		break;
		case 2:
			steps = 16;
			for (m = 0; m < 16; m++) {
				video_update_start();
				for (n = 0; n < SCREEN_WIDTH; n += 16) video_update_add(SRECT(n + m, 0, 1, SCREEN_HEIGHT));
				video_update_end(100 / steps);
			}
		break;
		case 3:
			steps = SCREEN_HEIGHT / 4;
			for (n = 0; n < SCREEN_HEIGHT; n += 4) {
				video_update_start();
					video_update_add(SRECT(0, n                    , SCREEN_WIDTH, 2));
					video_update_add(SRECT(0, SCREEN_HEIGHT - 2 - n, SCREEN_WIDTH, 2));
				video_update_end(1000 / steps);
			}
		break;
		case 4:
			steps = 16;
			for (m = 0; m < 16; m++) {
				video_update_start();
				for (n = 0; n < SCREEN_HEIGHT; n += 16) video_update_add(SRECT(0, n + m, SCREEN_WIDTH, 1));
				video_update_end(100 / steps);
			}
		break;		
	}
}
