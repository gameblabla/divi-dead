#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(BITTBOY) || defined(RS97) || defined(GCW0)
#define VIDEOMODE_FLAGS (SDL_HWSURFACE)
#define VIDEOMODE_BITS 16
#define SCREEN_BUFFER_BPP 32

#define FONT_DEFAULT_SIZE 18
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#if defined(BITTBOY)
#define BUTTON_A_DEFINE SDLK_LCTRL
#define BUTTON_B_DEFINE SDLK_LALT
#define BUTTON_C_DEFINE SDLK_RCTRL
#define BUTTON_D_DEFINE SDLK_RETURN
#define BUTTON_MODE_DEFINE SDLK_LSHIFT
#define BUTTON_SPECIAL_DEFINE SDLK_SPACE
#define BUTTON_EXIT_KEY SDLK_ESCAPE
#elif defined(RS97)
#define BUTTON_A_DEFINE SDLK_LCTRL
#define BUTTON_B_DEFINE SDLK_LALT
#define BUTTON_C_DEFINE SDLK_RETURN
#define BUTTON_D_DEFINE SDLK_TAB
#define BUTTON_MODE_DEFINE SDLK_LSHIFT
#define BUTTON_SPECIAL_DEFINE SDLK_SPACE
#define BUTTON_EXIT_KEY SDLK_END
#elif defined(GCW0)
#define BUTTON_A_DEFINE SDLK_LCTRL
#define BUTTON_B_DEFINE SDLK_LALT
#define BUTTON_C_DEFINE SDLK_RETURN
#define BUTTON_D_DEFINE SDLK_TAB
#define BUTTON_MODE_DEFINE SDLK_LSHIFT
#define BUTTON_SPECIAL_DEFINE SDLK_SPACE
#define BUTTON_EXIT_KEY SDLK_ESCAPE
#endif

#else
#define VIDEOMODE_FLAGS (SDL_SWSURFACE)
#define VIDEOMODE_BITS 32
#define SCREEN_BUFFER_BPP 32

#define FONT_DEFAULT_SIZE 15
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 480

#define BUTTON_A_DEFINE SDLK_LCTRL
#define BUTTON_B_DEFINE SDLK_LALT
#define BUTTON_C_DEFINE SDLK_TAB
#define BUTTON_D_DEFINE SDLK_BACKSPACE
#define BUTTON_MODE_DEFINE SDLK_LSHIFT
#define BUTTON_SPECIAL_DEFINE SDLK_SPACE
#define BUTTON_EXIT_KEY SDLK_ESCAPE
#endif

#endif
