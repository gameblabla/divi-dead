#include "hooks.h"

#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

void WIN_HOOK_INIT_VIDEO_PRE() {
	putenv("SDL_VIDEO_WINDOW_POS");
	putenv("SDL_VIDEO_CENTERED=1");
	
	// Native window caption
	SDL_WM_SetCaption("Dividead", NULL);

	{ // Native icon
		HINSTANCE handle = GetModuleHandle(NULL);
		HICON icon = LoadIcon(handle, "icon");
		SDL_SysWMinfo wminfo;
		SDL_VERSION(&wminfo.version);
		SDL_GetWMInfo(&wminfo);
		HWND hwnd = (HANDLE)wminfo.window;
		SetClassLong(hwnd, GCL_HICON, (LONG)icon);	
	}
	
	// Native cursor
	{
		SDL_Cursor *cursor = SDL_GetCursor();
		*(int *)cursor->wm_cursor = (int)LoadCursor(NULL, IDC_ARROW);
		SDL_SetCursor(cursor);	
	}	
}
