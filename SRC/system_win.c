//#undef EFFECT_CREDITS

#define RINGBUF_AUDIO

#define CHECK_FILESYSTEM

#define VIDEOMODE_FLAGS (SDL_SWSURFACE|SDL_DOUBLEBUF)
//#define VIDEOMODE_FLAGS (SDL_SWSURFACE|SDL_DOUBLEBUF|SDL_OPENGL|SDL_OPENGLBLIT)

//#define VIDEOMODE_FLAGS (SDL_SWSURFACE)
//#define VIDEOMODE_BITS 32
//#define VIDEOMODE_BITS 16
#define VIDEOMODE_BITS 32
//#define VIDEOMODE_16BITS_TEMP_HACK
#define FILE_PREFIX ""
//#define FBUFFER_FULL_UPDATE

#define ENABLE_VIDEO_SMPEG

#include <io.h>
void mkdir2(char *name, int mode) {
	mkdir(name);
}
#undef mkdir
#define mkdir mkdir2	

// DEBUG
void message(char *format, ...) {
	char buffer[0x1000];
	va_list ap;
	va_start(ap, format);
	vsprintf(buffer, format, ap);
	va_end(ap);
	#ifdef GAME_DEBUG
		printf("DEBUG_MSG: %s\n", buffer);
	#endif
}

//#include <windows.h>
int MessageBoxA(void*, char*, char*, int);
int GetLocaleInfoA(int, int, char*, int);
int GetSystemDefaultLCID();

#define MB_YESNO 4
#define MB_ICONQUESTION 32
#define MB_DEFBUTTON1 0
#define IDCANCEL 2
#define IDYES 6
#define IDNO 7

int ask(char *text, int _default) {
	int msgboxID = MessageBoxA(
		NULL,
		text,
		"Dividead",
		MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON1
	);
	
    switch (msgboxID) {
		default:
		case IDCANCEL: return ASK_CANCEL;
		case IDYES:    return ASK_YES;
		case IDNO:     return ASK_NO;
    }	
}

#define LOCALE_USER_DEFAULT	0x400
#define LOCALE_ILANGUAGE	1
#define LOCALE_RETURN_NUMBER 0
#define LOCALE_SABBREVLANGNAME 3
#define LOCALE_IDEFAULTLANGUAGE 9
#define LOCALE_IDEFAULTCOUNTRY	10
#define LOCALE_SENGLANGUAGE 0x1001


void preinit() {
	int LCID = GetSystemDefaultLCID();
	int selected = 1;

	switch (LCID & 0x1FF) {
		case 0x07: strcpy(language, "GERMAN"); break;
		case 0x09: strcpy(language, "ENGLISH"); break;
		case 0x0A: strcpy(language, "SPANISH"); break;
		case 0x0C: strcpy(language, "FRENCH"); break;
		case 0x10: strcpy(language, "ITALIAN"); break;
		case 0x11: strcpy(language, "JAPANESE"); break;
		default: selected = 0; break;
	}
	
	if (selected) {
		printf("WINDOWS: LANGUAGE DETECTED: %s\n", language);
	} else {
		printf("WINDOWS: LANGUAGE NOT DETECTED\n");
	}
}