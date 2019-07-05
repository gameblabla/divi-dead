#include <ctype.h>
#include "system_unix.h"

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

int ask(char *text, int _default) {
	return ASK_YES;
}

int stricmp(char *a, char *b) {
	while (1) {
		char ac, bc; ac = *a; bc = *b;
		if (ac == bc && !ac) return 0;
		if (ac >= 'a' && ac <= 'z') ac -= ('a' - 'A');
		if (bc >= 'a' && bc <= 'z') bc -= ('a' - 'A');
		if (!ac || (ac < bc)) return 1;
		if (!bc || (ac > bc)) return -1;
		a++; b++;
	}
	return 0;
}

void preinit() {
	char loc[3];
	int selected = 1;
	char *env = getenv("LANG");
	if (env && env[0] && env[1]) {
		loc[0] = toupper(env[0]);
		loc[1] = toupper(env[1]);
		loc[2] = 0;
		if (0 == 0) { }
		else if (strcmp(loc, "EN") == 0) strcpy(language, "ENGLISH");
		else if (strcmp(loc, "ES") == 0) strcpy(language, "SPANISH");
		else if (strcmp(loc, "JA") == 0) strcpy(language, "JAPANESE");
		else if (strcmp(loc, "DE") == 0) strcpy(language, "GERMAN");
		else if (strcmp(loc, "IT") == 0) strcpy(language, "ITALIAN");
		else if (strcmp(loc, "FR") == 0) strcpy(language, "FRENCH");
		else selected = 0;
	} else {
		selected = 0;
	}
	
	if (!selected) {
		printf("UNIX: NOT DETECTED LANG ENVIROMENT\n");
	} else {
		printf("UNIX: DETECTED LANG ENVIROMENT : %s\n", language);
	}
}
