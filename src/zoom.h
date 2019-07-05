#ifndef ZOOM_H
#define ZOOM_H

#include "shared.h"

extern void _zoomSurfaceSize(int width, int height, double zoomx, double zoomy, int *dstwidth, int *dstheight);
extern int SDL_ZoomSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst);
extern SDL_Surface *SDL_ZoomSurface(SDL_Surface *src, double zoomx, double zoomy);

#endif
