#include "zoom.h"

#define VALUE_LIMIT	0.001
typedef struct tColorRGBA { Uint8 v[4]; } tColorRGBA;

void _zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight) {
    if (zoomx < VALUE_LIMIT) zoomx = VALUE_LIMIT;
    if (zoomy < VALUE_LIMIT) zoomy = VALUE_LIMIT;
    if ((*dstwidth  = (int)((double)width  * zoomx)) < 1) *dstwidth  = 1;
    if ((*dstheight = (int)((double)height * zoomy)) < 1) *dstheight = 1;
}

int SDL_ZoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst) {
	int retval = -1;
    int x, y, sx, sy, *sax = NULL, *say = NULL, *csax, *csay, csx, csy, ex, ey, t1, t2, sstep;
    tColorRGBA *c00, *c01, *c10, *c11;
    tColorRGBA *sp, *csp, *dp;
    int dgap;

	sx = (int)(0x10000 * (src->w - 1) / dst->w);
	sy = (int)(0x10000 * (src->h - 1) / dst->h);

    if ((sax = (int *)malloc((dst->w + 1) * sizeof(Uint32))) == NULL) goto _cleanup;
    if ((say = (int *)malloc((dst->h + 1) * sizeof(Uint32))) == NULL) goto _cleanup;

    sp = csp = (tColorRGBA *) src->pixels;
    dp = (tColorRGBA *) dst->pixels;

    csx = 0;
    csax = sax;
    for (x = 0; x <= dst->w; x++) {
		*csax = csx;
		csax++;
		csx &= 0xffff;
		csx += sx;
    }
	
    csy = 0;
    csay = say;
    for (y = 0; y <= dst->h; y++) {
		*csay = csy;
		csay++;
		csy &= 0xffff;
		csy += sy;
    }

    dgap = dst->pitch - (dst->w << 2);

	csay = say;
	for (y = 0; y < dst->h; y++) {
	    c01 = c00 = csp; c01++;
	    c11 = c10 = (tColorRGBA *) ((Uint8 *) csp + src->pitch); c11++;
	    csax = sax;
		
	    for (x = 0; x < dst->w; x++) {
			ex = (*csax & 0xffff); ey = (*csay & 0xffff);
			

			//#define G(f, i) ((f->v[i] * f->v[3]) / 0xFF)
			#define G(f, i) (f->v[i])
			
		
			#define PN(S, F1, F2, i) S = ((((G(F2, i) - G(F1, i)) * ex) >> 16) + G(F1, i)) & 0xFF;

			#define P1(i) PN(t1, c00, c01, i)
			#define P2(i) PN(t2, c10, c11, i)
			#define PW(i) dp->v[i] = (((t2 - t1) * ey) >> 16) + t1;
			
			#define P12W(i) P1(i) P2(i) PW(i)
			
			P12W(0) P12W(1) P12W(2) P12W(3)
						
			csax++;
			sstep = (*csax >> 16);
			c00 += sstep; c01 += sstep; c10 += sstep; c11 += sstep;
			dp++;
	    }

	    csay++;
	    csp = (tColorRGBA *) ((Uint8 *) csp + (*csay >> 16) * src->pitch);
	    dp = (tColorRGBA *) ((Uint8 *) dp + dgap);
	}
	
	retval = 0;

_cleanup:
    if (sax) free(sax);
    if (say) free(say);

    return retval;
}

SDL_Surface *SDL_ZoomSurface(SDL_Surface *src, double zoomx, double zoomy) {
    SDL_Surface *rsrc;
    SDL_Surface *rdst;
    int dstwidth, dstheight;

    if (!(rsrc = src)) return NULL;

    if (rsrc->format->BitsPerPixel != 32) {
		rsrc = SDL_CreateRGBSurface(SDL_SWSURFACE, src->w, src->h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
		SDL_BlitSurface(src, NULL, rsrc, NULL);
	}
	
	_zoomSurfaceSize(rsrc->w, rsrc->h, zoomx, zoomy, &dstwidth, &dstheight);
	rdst = SDL_CreateRGBSurface(SDL_SWSURFACE, dstwidth, dstheight, 32, rsrc->format->Rmask, rsrc->format->Gmask, rsrc->format->Bmask, rsrc->format->Amask);

    SDL_LockSurface(rsrc);
	SDL_LockSurface(rdst);
	{
		SDL_ZoomSurfaceRGBA(rsrc, rdst);
		SDL_SetAlpha(rdst, SDL_SRCALPHA, 255);		
	}
	SDL_UnlockSurface(rdst);
    SDL_UnlockSurface(rsrc);
	
	if (rsrc != src) SDL_FreeSurface(rsrc);

    return rdst;
}
