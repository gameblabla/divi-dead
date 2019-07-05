#ifndef SYSTEM_UNIX
#define SYSTEM_UNIX

#include "shared.h"

#define FILE_PREFIX ""

extern void message(char *format, ...);
extern int ask(char *text, int _default);
extern int stricmp(char *a, char *b);
extern void preinit();


#endif
