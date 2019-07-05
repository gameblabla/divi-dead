//#define GAME_STDOUT_REDIRECT

//#undef ENABLE_VIDEO_SMPEG
#define ENABLE_VIDEO_SMPEG

#define RINGBUF_AUDIO

//#define VIDEOMODE_FLAGS (SDL_SWSURFACE|SDL_DOUBLEBUF)
#define VIDEOMODE_FLAGS (SDL_SWSURFACE)
#define VIDEOMODE_BITS 32
//#define VIDEOMODE_BITS 16

//#define FBUFFER_FULL_UPDATE

#undef CHECK_FILESYSTEM	
#undef main

#define DISABLE_MIDI

//#define AUDIO_CHECK_MOD
#define PSP_REGISTRY_CONFIG

#undef SCREEN_WIDTH 
#undef SCREEN_HEIGHT
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272

#undef IMAGE_CACHE_MAX_DEFAULT
#define IMAGE_CACHE_MAX_DEFAULT 12
//#define IMAGE_CACHE_MAX_DEFAULT 15

#undef font_size_corrected
#define font_size_corrected 9
#undef text_margin_top
#define text_margin_top 1

/*
#undef ACORRECT_X
#undef ACORRECT_Y
#define ACORRECT_X(v) ((((v) * SCREEN_WIDTH)  / ORI_SCREEN_WIDTH) + 0)
#define ACORRECT_Y(v) ((((v) * SCREEN_HEIGHT) / ORI_SCREEN_HEIGHT) + 1)
*/

#undef SAVE_ROOT
#undef SNAP_ROOT
#undef ALTERNATE_LANG
#define SAVE_ROOT "ms0:/PSP/SAVES/DIVIDEAD"
#define SNAP_ROOT "ms0:/PSP/PHOTO/DIVIDEAD"
#define ALTERNATE_LANG "ms0:/PSP/SAVES/DIVIDEAD/LANG"
#define FILE_PREFIX ""

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <psppower.h>
#include <psputility.h>
#include <pspreg.h>
#include <pspmscm.h>
#include <pspdisplay.h>
#include <psptypes.h>
#include <pspiofilemgr.h>

//#include <stdio.h>
//#include <string.h>
//#include <sys/types.h>
//#include <sys/unistd.h>

int main2();

PSP_MODULE_INFO("DIVIDEAD", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

char gamepath[128] = {0};

void standby_begin() {
	GAME_MUSIC_STOP();
}

int inserted_callback_count = 0;

void standby_end() {
	if (inserted_callback_count++ == 0) return;
	must_resume_music = 1;
}

int ms_callback(int arg1, int arg2, void *arg) {
    if (arg2 == MS_CB_EVENT_INSERTED){
		standby_end();
    } else if (arg2 == MS_CB_EVENT_EJECTED){
		standby_begin();
    }
    return 0;
}

#ifdef PSP_REGISTRY_CONFIG
	int get_registry_value(const char *dir, const char *name, unsigned int *val) {
		int ret = 0;
		struct RegParam reg;
		REGHANDLE h;

		memset(&reg, 0, sizeof(reg));
		reg.regtype = 1;
		reg.namelen = strlen("/system");
		reg.unk2 = 1;
		reg.unk3 = 1;
		strcpy(reg.name, "/system");
		
		if (sceRegOpenRegistry(&reg, 2, &h) == 0) {
			REGHANDLE hd;
			if (!sceRegOpenCategory(h, dir, 2, &hd)) {
				REGHANDLE hk;
				unsigned int type, size;

				if (!sceRegGetKeyInfo(hd, name, &hk, &type, &size)) {
					if (!sceRegGetKeyValue(hd, hk, val, 4)) {
						ret = 1;
						sceRegFlushCategory(hd);
					}
				}
				sceRegCloseCategory(hd);
			}
			sceRegFlushRegistry(h);
			sceRegCloseRegistry(h);
		}

		return ret;
	}
#endif

int exit_callback(int arg1, int arg2, void *common) {
	done = 1;
	return 0;
}

int CallbackThread(SceSize args, void *argp) {
	sceKernelRegisterExitCallback(sceKernelCreateCallback("Exit Callback", exit_callback, NULL));
    MScmRegisterMSInsertEjectCallback(sceKernelCreateCallback("MSCB", ms_callback, NULL));
	sceKernelSleepThreadCB();
	return 0;
}

int SetupCallbacks(void) {
	int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0) sceKernelStartThread(thid, 0, 0);
	return thid;
}

/*void message_info(char *format, ...) {
	char buffer[0x1000];
	va_list ap;
	va_start(ap, format);
	vsprintf(buffer, format, ap);
	pspDebugScreenSetXY(0, 0);
	pspDebugScreenPrintf("%s", buffer);
	va_end(ap);
}*/

void message(char *format, ...) {
	char buffer[0x1000];
	va_list ap;
	va_start(ap, format);
	vsprintf(buffer, format, ap);
	#ifdef GAME_DEBUG
		pspDebugScreenSetXY(0, 0);
		pspDebugScreenPrintf("%s\n", buffer);
	#endif
	va_end(ap);
}

#ifdef GAME_STDOUT_REDIRECT
	static void cleanup_output(void) {
		fclose(stdout);
		fclose(stderr);
	}
#endif

void mkdir(char *str, int mode) {
	sceIoMkdir(str, 0777);
}

void setFreq(int a, int b, int c) {
	scePowerSetClockFrequency(a, b, c);
	//scePowerSetCpuClockFrequency(a);
	//scePowerSetBusClockFrequency(c); 	
}

void main_init() {
	char buf[512]; buf[0] = 0;
	int iso = 0;
	#ifdef PSP_REGISTRY_CONFIG
		uint lang;
		uint but;
		
		if (get_registry_value("/CONFIG/SYSTEM/XMB", "language", &lang)) {
			//printf("Language : %d\n", lang);
			switch (lang) {
				default:
				case 0: strcpy(language, "JAPANESE"); break;
				case 1: strcpy(language, "ENGLISH"); break;
				case 2: strcpy(language, "FRENCH"); break;
				case 3: strcpy(language, "SPANISH"); break;
				case 4: strcpy(language, "GERMAN"); break;
				case 5: strcpy(language, "ITALIAN"); break;
			}
		}
		
		if (get_registry_value("/CONFIG/SYSTEM/XMB", "button_assign", &but)) {
			swap_buttons = (but != 0);
		}
	#endif

	mkdir("ms0:/PSP", 0777);
	
	mkdir("ms0:/PSP/SAVES", 0777);
	mkdir("ms0:/PSP/SAVES/DIVIDEAD", 0777);
	mkdir("ms0:/PSP/SAVES/DIVIDEAD/LANG", 0777);
	
	mkdir("ms0:/PSP/PHOTO", 0777);
	mkdir("ms0:/PSP/PHOTO/DIVIDEAD", 0777);
	
	getcwd(buf, sizeof(buf));

	if (sceIoChdir("disc0:/PSP_GAME/USRDIR/") >= 0) {
		printf("Changed to 'disc0:/PSP_GAME/USRDIR/'\n");
		if (sceIoGetstat("SG.DL1", NULL) >= 0) {
			printf("File 'SG.DL1' exists (WORKING FROM UMD/ISO)\n");
			iso = 1;
		}
	}
	
	getcwd(gamepath, sizeof(gamepath));
	
	if (!iso) sceIoChdir(buf);
}

#undef MOVIE_START
#undef MOVIE_END
#define MOVIE_START() setFreq(333, 333, 166);
#define MOVIE_END()   setFreq(166, 66, 111);

int main(void) {
	//setFreq(166, 66, 111);
	//setFreq(333, 333, 166);

	atexit(sceKernelExitGame);

	#ifdef GAME_STDOUT_REDIRECT
		freopen("ms0:/PSP/SAVES/DIVIDEAD/stdout.txt", "w", stdout);
		freopen("ms0:/PSP/SAVES/DIVIDEAD/stderr.txt", "w", stderr);
		//setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
		setbuf(stdout, NULL);
		setbuf(stderr, NULL);
		atexit(cleanup_output);
	#endif

	pspDebugScreenInit();
	
	SetupCallbacks();
	
	main_init();
	main2(NULL, 0);

	return 0;
}

#define main main2

int ask(char *text, int _default) {
	return _default;
}

void preinit() {
}

/*
int ask(char *text, int _default) {
	int retval = _default;
	int status;
	pspUtilityMsgDialogParams dialog;
	memset(&dialog, 0, sizeof(dialog));
	//dialog.mode = pspUtilityMsgDialogMode.PSP_UTILITY_MSGDIALOG_MODE_TEXT;
	//dialog.options = PSP_UTILITY_MSGDIALOG_OPTION_TEXT | PSP_UTILITY_MSGDIALOG_OPTION_YESNO_BUTTONS;
	strcpy(dialog.message, text);
	
	if (sceUtilityMsgDialogInitStart(&dialog) == 0) {
		while ((status = sceUtilityMsgDialogGetStatus()) != 4) {
			if (status == 3) sceUtilityMsgDialogShutdownStart();
			sceUtilityMsgDialogUpdate(1);
		}
		
		switch (dialog.base.result) {
			default:
			case 1: retval = ASK_CANCEL; break;
			case 2: retval = ASK_YES; break;
			case 3: retval = ASK_NO; break;
		}
		//switch (dialog.buttonPressed) { }
	}
	
	return retval;
}
*/

/*
int pmf_init(SceInt32 nPackets) {
	SceMpegAvcMode m_MpegAvcMode;
	
	m_RingbufferPackets = nPackets;

	sceMpegInit();
	m_RingbufferSize = sceMpegRingbufferQueryMemSize(m_RingbufferPackets);
	m_MpegMemSize = sceMpegQueryMemSize(0);
	m_RingbufferData = malloc(m_RingbufferSize);
	m_MpegMemData = malloc(m_MpegMemSize);
	sceMpegRingbufferConstruct(&m_Ringbuffer, m_RingbufferPackets, m_RingbufferData, m_RingbufferSize, &RingbufferCallback, &m_FileHandle);
	sceMpegCreate(&m_Mpeg, m_MpegMemData, m_MpegMemSize, &m_Ringbuffer, BUFFER_WIDTH, 0, 0);

	m_MpegAvcMode.iUnk0 = -1;
	m_MpegAvcMode.iUnk1 = 3;
	sceMpegAvcDecodeMode(&m_Mpeg, &m_MpegAvcMode);

	return 0;
}

void pmf_shutdown() {
	free(m_pEsBufferAtrac);
	sceMpegFreeAvcEsBuf(&m_Mpeg, m_pEsBufferAVC);
	sceMpegUnRegistStream(&m_Mpeg, m_MpegStreamAVC);
	sceMpegUnRegistStream(&m_Mpeg, m_MpegStreamAtrac);
	sceIoClose(m_FileHandle);
	sceMpegDelete(&m_Mpeg);
	sceMpegRingbufferDestruct(&m_Ringbuffer);
	sceMpegFinish();
	free(m_RingbufferData);
	free(m_MpegMemData);
}

void pmf_play() {
	int retVal, fail = 0;
	ReaderThreadData* TDR;
	DecoderThreadData* TDD;

	InitReader();
	InitVideo();
	InitAudio();
	InitDecoder();

	TDR = &Reader;
	TDD = &Decoder;

	sceKernelStartThread(Reader.m_ThreadID,  sizeof(void*), &TDR);
	sceKernelStartThread(Audio.m_ThreadID,   sizeof(void*), &TDD);
	sceKernelStartThread(Video.m_ThreadID,   sizeof(void*), &TDD);
	sceKernelStartThread(Decoder.m_ThreadID, sizeof(void*), &TDD);

	sceKernelWaitThreadEnd(Decoder.m_ThreadID, 0);
	sceKernelWaitThreadEnd(Video.m_ThreadID, 0);
	sceKernelWaitThreadEnd(Audio.m_ThreadID, 0);
	sceKernelWaitThreadEnd(Reader.m_ThreadID, 0);

	ShutdownDecoder();
	ShutdownAudio();
	ShutdownVideo();
	ShutdownReader();
}
*/
