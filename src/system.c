////////////////////////////////////////////////////////////
// SYSTEM                                                 //
////////////////////////////////////////////////////////////

//#define DEBUG_FLIST

#include "system.h"

char save_buffer[0x1800];
int save_buffer_size;

SDL_RWops *save_buffer_open() {
	return SDL_RWFromMem(save_buffer, sizeof(save_buffer));
}

SDL_RWops *save_buffer_open_load() {
	return SDL_RWFromConstMem(save_buffer, save_buffer_size);
}

void SYS_SAVE_FULL(SDL_RWops *f) {
	VAR_SAVE_L(&save_s, sizeof(save_s));
}

void SYS_SAVE_MIN(SDL_RWops *f) {
	int n, m, tp, i;
	unsigned char gallerybf[19];
	unsigned char magic = 0xFF;
	unsigned char fvar;
	
	for(i=0;i<sizeof(gallerybf);i++)
	{
		gallerybf[i] = 0;
	}
	
	for (tp = 0, n = 0; n < 150; n += 8, tp++) {
		fvar = 0;
		for (m = 0; m < 8; m++) if (save_s.gallery[n + m]) fvar |= (1 << m);
		gallerybf[tp] = fvar;
	}
	
	VAR_SAVE_L(&magic, 1);
	VAR_SAVE_L(save_s.names, sizeof(save_s.names));
	VAR_SAVE_L(gallerybf, 19);
}

int SYS_SAVE() {
	char sys_path[512];
	//int retval;
	SDL_RWops *f;
	
#ifdef DREAMCAST
	f = save_buffer_open();
#else

#ifdef HOME_DIRECTORY
	char save_path[512];
	snprintf(save_path, sizeof(save_path), "%s/%s", getenv("HOME"), SAVE_DIRECTORY_NAME);
	mkdir(save_path, 0755);
	
	snprintf(save_path, sizeof(save_path), "%s/%s/snaps", getenv("HOME"), SAVE_DIRECTORY_NAME);
	mkdir(save_path, 0755);
	
	snprintf(save_path, sizeof(save_path), "%s/%s/saves", getenv("HOME"), SAVE_DIRECTORY_NAME);
	mkdir(save_path, 0755);
	
	snprintf(sys_path, sizeof(sys_path), "%s/%s/SYS.DAT", getenv("HOME"), SAVE_DIRECTORY_NAME);
#else
	mkdir(SAVE_ROOT "/DATA", 0755);
	snprintf(sys_path, sizeof(sys_path), SAVE_ROOT "/DATA/SYS.DAT");
#endif
	
	if (!(f = SDL_RWFromFile(sys_path, "wb"))) {
		printf("Can't write system\n");
		return 0;
	}	
#endif

	SAVE_FULL ? SYS_SAVE_FULL(f) : SYS_SAVE_MIN(f);
	
#ifdef DREAMCAST
	dc_save_file_save(0, 0, save_buffer, SDL_RWtell(f));
#endif	
	
	SDL_RWclose(f);
	
	return 1;
}

void SYS_LOAD_FULL(SDL_RWops *f) {
	VAR_LOAD_L(&save_s, sizeof(save_s))
}

void SYS_LOAD_MIN(SDL_RWops *f) {
	int n, m, tp, i;
	unsigned char gallerybf[19];
	unsigned char magic = 0xFF;
	unsigned char fvar;

	for(i=0;i<sizeof(gallerybf);i++)
	{
		gallerybf[i] = 0;
	}
	
	VAR_LOAD_L(&magic, 1);
	VAR_LOAD_L(save_s.names, sizeof(save_s.names));
	VAR_LOAD_L(gallerybf, 19);

	// Compress gallery
	for (tp = 0, n = 0; n < 150; n += 8, tp++) {
		fvar = gallerybf[tp];
		for (m = 0; m < 8; m++) {
			save_s.gallery[n + m] = (fvar & 1);
			fvar >>= 1;
		}
	}
}

int SYS_LOAD() {
	//int retval;
	SDL_RWops *f;
	unsigned char magic;
	char sys_path[256];
	char sys_path_underscore[256];
#ifdef DREAMCAST
	dc_save_file_load(0, 0, save_buffer, &save_buffer_size);
	f = save_buffer_open_load();
#else

#ifdef HOME_DIRECTORY
	snprintf(sys_path, sizeof(sys_path), "%s/%s/SYS.DAT", getenv("HOME"), SAVE_DIRECTORY_NAME);
	snprintf(sys_path_underscore, sizeof(sys_path_underscore), "%s/%s/sys.dat", getenv("HOME"), SAVE_DIRECTORY_NAME);
#else
	snprintf(sys_path, sizeof(sys_path), SAVE_ROOT "/DATA/SYS.DAT");
	snprintf(sys_path_underscore, sizeof(sys_path_underscore), SAVE_ROOT "/data/sys.dat");
#endif

	if (!(f = SDL_RWFromFile(sys_path, "rb"))) 
	{
		if (!(f = SDL_RWFromFile(sys_path_underscore, "rb"))) 
		{
			int n;
			for (n = 0; n < 10; n++) strcpy(save_s.names[n], lang_texts[11]);
			printf("Can't load system\n");
			SYS_SAVE();
			return 0;
		}
	}
#endif
	
	VAR_LOAD_L(&magic, 1);
	SDL_RWseek(f, 0, SEEK_SET);
	
	(magic != 0xFF) ? SYS_LOAD_FULL(f) : SYS_LOAD_MIN(f);
	
	SDL_RWclose(f);	
	
#ifdef DREAMCAST
	dc_sys_hack();
#endif
	
	return 1;
}

int FLIST_LOAD() {
	SDL_RWops *f;
	int n, m;

	printf("Loading FLIST...");	
	
	if (!(f = VFS_LOAD("FLIST"))) {
		printf("Can't load flist\n");
		return 0;
	}
	
	SDL_RWread(f, &gallery, 1, sizeof(gallery));
	SDL_RWclose(f);
	
	sprintf(gallery[149], "%s", "OMAKE_0  ");
	
	for (n = 0; n < 150; n++) {
		for (m = 0; m < 0x10; m++) {
			if (gallery[n][m] == ' ') {
				gallery[n][m] = 0;
				break;
			}
		}
	}
	
	printf("Ok\n");	
	
	return 1;
}

int FLIST_CHECK(char *name) {
	char temp[0x30]; char *tempp = temp; 
	int n; strcpy(temp, name);
	while (*tempp) { if (*tempp == '.') *tempp = 0; tempp++; }

	#ifdef DEBUG_FLIST
		printf("FLIST_CHECK : ");
	#endif
	for (n = 0; n < 150; n++) {
		if (stricmp(gallery[n], temp) != 0) continue;
		// FOUND!
		
		if (!save_s.gallery[n]) {
			save_s.gallery[n] = 1;
			SYS_SAVE();
			#ifdef DEBUG_FLIST
				printf("Found (added)\n");
			#endif
		} else {
			#ifdef DEBUG_FLIST
				printf("Found (already exists)\n");
			#endif
		}
		
		return 1;
	}
	
	#ifdef DEBUG_FLIST
		printf("Not found\n");
	#endif
	
	return 0;
}
