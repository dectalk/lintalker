/*
 * Linux.c – Linux platform layer for WinTalker.
 *
 * Replaces Windows95.c.  No GUI, no audio device: speech is synthesised
 * synchronously into a heap buffer that the caller can write to a WAV file.
 *
 * Key differences from the Windows original:
 *   – HeapAlloc / HeapFree  →  calloc / free
 *   – waveOut* callbacks    →  synchronous loop in _SpeakBuffer
 *   – MakeDictPtrsAbsolute  →  64-bit safe (DictDisk on-disk struct)
 *   – gComponentRefcon      →  uintptr_t (pointer-sized)
 */

#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "english_lex.h"

#ifndef __SPEECHEQU__
	#include "SpeechEqu.h"
#endif

#ifndef __SPEECHSAY__
	#include "MT4.h"
#endif

#ifndef __FSYNTH__
	#include "Fsynth.h"
#endif

#ifndef __SPEECHVERSIONS__
	#include "Versions.h"
#endif

#ifndef __SPEECHMACINTOSH__
	#include "Macintosh.h"
#endif


/* ------------------------------------------------------------------ */
/* Globals                                                              */
/* ------------------------------------------------------------------ */

	extern	shellVarPtr		gInstanceStorage;
	extern	uintptr_t		gComponentRefcon;
	extern	short			gDoDT;
	extern	short			gSpeechIsDone;
	extern	char 			msg[MSG_LEN+1];

/* Voice data tables (Data.c) */
	extern	struct voiceData	Fred_Voice;
	extern	struct voiceData	Kathy_Voice;
	extern	struct voiceData	Princess_Voice;
	extern	struct voiceData	Junior_Voice;
	extern	struct voiceData	Ralph_Voice;
	extern	struct voiceData	Whisper_Voice;
	extern	struct voiceData	Zarvox_Voice;
	extern	struct voiceData	Trinoids_Voice;
	extern	struct voiceData	Bubbles_Voice;
	extern	struct voiceData	Boing_Voice;
	extern	struct voiceData	Bells_Voice;
	extern	struct voiceData	Hysterical_Voice;
	extern	struct voiceData	Deranged_Voice;
	extern	struct voiceData	GoodNews_Voice;
	extern	struct voiceData	BadNews_Voice;
	extern	struct voiceData	PipeOrgan_Voice;
	extern	struct voiceData	Cellos_Voice;

/* Wave samples (Sounds.c) */
	extern	char	Bells_Sound[];
	extern	char	Boing_Sound[];
	extern	char	Bubbles_Sound[];
	extern	char	Cellos_Sound[];
	extern	char	Deranged_Sound[];
	extern	char	Hysterical_Sound[];
	extern	char	PipeOrgan_Sound[];
	extern	char	Rules[];
	extern	char	Symbols[];

	extern	void	Init_BEFuncPtrs  (moduleFuncPtr mfp);		/* Engine.c      */
	extern	void	Init_FEFuncPtrs  (moduleFuncPtr mfp);		/* FrontEnd.c    */
	extern	void	Init_FSynthFuncPtrs (moduleFuncPtr mfp);	/* formantSynth.c */

/* Engine.c */
	extern	short	e_SpeakBuffer (voiceVarPtr vv, Ptr textBuf, long byteLen, long controlFlags);

/* Say.c */
	extern	void	synth_FillNextSampBuffer (voiceVarPtr vv);

/* Exports */
	long	_OpenSpeech	(void);
	long	_CloseSpeech (shellVarPtr svv);
	long	_SpeakBuffer (shellVarPtr svv, Ptr textBuf, long byteLen, long controlFlags);
	long	_UseVoice    (shellVarPtr svv, short index);
	long	_LoadVoicePack (shellVarPtr svv, const char *dir);
	short	WriteWavFile (shellVarPtr svv, const char *filename);

/* Forward declarations */
	static void  MaybeDisposeSharedGlobals (BESharedPtr sharedPtr);
	static void  CloseSpeechChannel        (shellVarPtr svv);
	static void  Init_ShellFuncPtrs        (moduleFuncPtr mfp);
	static void  AccumulateSamples         (shellVarPtr svv, WAVEHDR *wh);
	static void  Rules_LittleEndian        (void);
	static void  MakeDictPtrsAbsolute      (const char *rawBuf, Dict *out);


/* Parsed Symbols dictionary (global static; populated once) */
static Dict  g_SymbolsDict;
static int   g_SymbolsReady = 0;


/* ------------------------------------------------------------------ */
/* Byte-swap helpers                                                    */
/* ------------------------------------------------------------------ */

/* Read a big-endian 32-bit value without modifying the source buffer */
static uint32_t Read_BE32 (const unsigned char *p)
{
	return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16)
	     | ((uint32_t)p[2] <<  8) |  (uint32_t)p[3];
}

/* Read a big-endian 16-bit value without modifying the source buffer */
static uint16_t Read_BE16 (const unsigned char *p)
{
	return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

/* In-place byte-swap helpers (still needed for Rules[] which lives in RAM) */
static void Long_LittleEndian (unsigned char *p)
{
	unsigned char c0 = p[0], c1 = p[1], c2 = p[2], c3 = p[3];
	p[0] = c3; p[1] = c2; p[2] = c1; p[3] = c0;
}

static void Short_LittleEndian (unsigned char *p)
{
	unsigned char c0 = p[0], c1 = p[1];
	p[0] = c1; p[1] = c0;
}


/* ------------------------------------------------------------------ */
/* Rules_LittleEndian – byte-swap the 16-bit hash entries in Rules[]   */
/* ------------------------------------------------------------------ */

static void Rules_LittleEndian (void)
{
	short         i, idx;
	unsigned char *hashPtr = (unsigned char *)&Rules;
	unsigned char  ch1, ch2;

	for (i = 0; i < 26; i++)
		{
		idx  = i << 1;
		ch1  = hashPtr[idx];
		ch2  = hashPtr[idx + 1];
		hashPtr[idx]     = ch2;
		hashPtr[idx + 1] = ch1;
		}
}


/* ------------------------------------------------------------------ */
/* MakeDictPtrsAbsolute                                                 */
/*                                                                      */
/* Parses the 32-bit Mac big-endian binary dictionary stored in rawBuf  */
/* and fills the in-memory Dict struct *out with proper 64-bit pointers.*/
/* ------------------------------------------------------------------ */

static void MakeDictPtrsAbsolute (const char *rawBuf, Dict *out)
{
	long            i, j;
	const DictDisk *d;
	const uint32_t *diskIdx;
	uint32_t        words_off, index_off;

	if (!rawBuf || !out)
		return;

	d = (const DictDisk *)rawBuf;

	/* Read all header fields, byte-swapping into local variables (rawBuf is read-only) */
	out->nextDict  = NULL;
	out->version   = Read_BE32 ((const unsigned char *)&d->version);
	out->type      = Read_BE32 ((const unsigned char *)&d->type);
	out->wordCount = Read_BE32 ((const unsigned char *)&d->wordCount);
	out->flags     = Read_BE32 ((const unsigned char *)&d->flags);
	words_off      = Read_BE32 ((const unsigned char *)&d->words_off);
	index_off      = Read_BE32 ((const unsigned char *)&d->index_off);

	for (i = 0; i < HASH_ENTRIES; i++)
		out->hash[i] = Read_BE32 ((const unsigned char *)&d->hash[i]);

	for (i = 0; i < kPOS_Slots; i++)
		for (j = 0; j < 4; j++)
			out->POScodes[i][j] = Read_BE16 ((const unsigned char *)&d->POScodes[i][j]);

	/* Resolve word-data pointer (offset from start of buffer) */
	out->words = (unsigned char *)rawBuf + words_off;

	/* Expand each 32-bit big-endian index entry to a native pointer */
	diskIdx    = (const uint32_t *)((uintptr_t)rawBuf + index_off);
	out->index = (IndexEntry *)calloc (out->wordCount, sizeof (IndexEntry));

	if (out->index)
		{
		for (i = 0; i < (long)out->wordCount; i++)
			out->index[i] = (IndexEntry)((uintptr_t)rawBuf + Read_BE32 ((const unsigned char *)&diskIdx[i]));
		}
}


/* ------------------------------------------------------------------ */
/* _OpenSpeech                                                          */
/* ------------------------------------------------------------------ */

long _OpenSpeech (void)
{
	short		 error;
	BESharedPtr	 sharedPtr;
	shellVarPtr	 svv;
	OSErr		 err;

	error      = kNoError;
	sharedPtr  = (BESharedPtr) gComponentRefcon;

	if (!sharedPtr)
		{
		/* First instance: allocate shared globals */
		sharedPtr = (BESharedPtr) calloc (1, sizeof (BEShared));
		if (!sharedPtr)
			{ error = kOutOfMemory; goto FAILURE; }

		gComponentRefcon        = (uintptr_t) sharedPtr;
		sharedPtr->instanceCount = 0;
		sharedPtr->DictRaw       = NULL;
		sharedPtr->funcList      = NULL;

		sharedPtr->gestaltCPU    = 6;
		sharedPtr->dictRawOwned  = 0;

		Rules_LittleEndian ();

#ifndef NO_FILESYSTEM
		/* Load English.lex from disk, relative to the binary */
		{
			char    exePath[4096];
			char    lexPath[4096];
			ssize_t exeLen;
			FILE   *f;
			long    fsize;
			char   *buf;

			exeLen = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
			if (exeLen > 0)
			{
				char *slash;
				exePath[exeLen] = '\0';
				slash = strrchr(exePath, '/');
				if (slash) slash[1] = '\0';
				snprintf(lexPath, sizeof(lexPath), "%sEnglish.lex", exePath);
			}
			else
			{
				snprintf(lexPath, sizeof(lexPath), "English.lex");
			}

			f = fopen(lexPath, "rb");
			if (f)
			{
				fseek(f, 0, SEEK_END);
				fsize = ftell(f);
				rewind(f);
				buf = (char *)malloc((size_t)fsize);
				if (buf && fread(buf, 1, (size_t)fsize, f) == (size_t)fsize)
				{
					sharedPtr->DictRaw      = buf;
					sharedPtr->dictRawOwned = 1;
				}
				else
				{
					free(buf);
					fprintf(stderr, "warning: failed to read '%s', using built-in dictionary\n", lexPath);
					sharedPtr->DictRaw = (char *) english_lex_data;
				}
				fclose(f);
			}
			else
			{
				fprintf(stderr, "warning: '%s' not found, using built-in dictionary\n", lexPath);
				sharedPtr->DictRaw = (char *) english_lex_data;
			}
		}
#else
		/* Embedded dictionary (NO_FILESYSTEM build) */
		sharedPtr->DictRaw = (char *) english_lex_data;
#endif
		MakeDictPtrsAbsolute (sharedPtr->DictRaw, &sharedPtr->DictMem);

		/* Parse the built-in Symbols dictionary once */
		if (!g_SymbolsReady)
			{
			MakeDictPtrsAbsolute ((char *)&Symbols, &g_SymbolsDict);
			g_SymbolsReady = 1;
			}

		/* Init function-pointer tables */
		sharedPtr->funcList = (moduleFuncPtr) calloc (1, sizeof (moduleFunc));
		if (!sharedPtr->funcList)
			{ error = kOutOfMemory; goto FAILURE; }

		Init_ShellFuncPtrs (sharedPtr->funcList);
		Init_FEFuncPtrs    (sharedPtr->funcList);
		Init_BEFuncPtrs    (sharedPtr->funcList);
		}

	/* Allocate per-channel shell globals */
	svv = (shellVarPtr) calloc (1, sizeof (shellVar));
	if (!svv)
		{ error = kOutOfMemory; goto FAILURE; }

	svv->sPtr         = (Ptr) sharedPtr;
	svv->soundBufPtr  = NULL;
	svv->reverbBufPtr = NULL;
	svv->formantGlobals = NULL;
	svv->tokBufferPtr = NULL;
	svv->ChannelGlobals = NULL;
	svv->hWnd         = NULL;
	svv->wh[0] = svv->wh[1] = NULL;
	svv->pSaveBuf     = NULL;
	svv->cSaveLen     = 0;
	svv->outputBuf    = NULL;
	svv->outputLen    = 0;
	svv->outputCapacity = 0;

	svv->funcList = sharedPtr->funcList;
	gInstanceStorage = svv;

	/* Allocate engine channel globals */
	svv->ChannelGlobals = (voiceVarPtr) calloc (1, sizeof (voiceVar));
	if (!svv->ChannelGlobals)
		{ error = kOutOfMemory; goto FAILURE2; }

	svv->ChannelGlobals->shellV    = (Ptr) svv;
	svv->ChannelGlobals->funcList  = svv->funcList;
	svv->ChannelGlobals->Dict      = &sharedPtr->DictMem;
	svv->ChannelGlobals->Symbols   = &g_SymbolsDict;
	svv->ChannelGlobals->Rules     = (char *)&Rules;
	svv->ChannelGlobals->cpuIsFast = 1;
	svv->ChannelGlobals->hWnd      = NULL;

	/* Allocate double-buffer audio scratch space */
	svv->soundBufPtr = calloc (1, 4 * kSampBufLen);
	if (!svv->soundBufPtr)
		{ error = kOutOfMemory; goto FAILURE2; }

	svv->reverbBufPtr = calloc (1, kMaxTap * sizeof (rShort));
	if (!svv->reverbBufPtr)
		{ error = kOutOfMemory; goto FAILURE2; }

	/* Set up WAVEHDR double-buffer headers */
	svv->wh[0] = (WAVEHDR *) calloc (1, sizeof (WAVEHDR));
	svv->wh[1] = (WAVEHDR *) calloc (1, sizeof (WAVEHDR));
	if (!svv->wh[0] || !svv->wh[1])
		{ error = kOutOfMemory; goto FAILURE2; }

	svv->wh[0]->lpData = (char *) svv->soundBufPtr;
	svv->wh[1]->lpData = (char *) svv->soundBufPtr + (kSampBufLen << 1);

	svv->ChannelGlobals->bit16_Sound  = 1;
	svv->ChannelGlobals->sampleBuffer1 = (unsigned char *)svv->wh[0]->lpData;
	svv->ChannelGlobals->sampleBuffer2 = (unsigned char *)svv->wh[1]->lpData;
	svv->ChannelGlobals->reverbBufPtr  = svv->reverbBufPtr;

	/* FE token buffer */
	svv->tokBufferPtr = (FETokenPtr) calloc (kTokMax, sizeof (FEToken));
	if (!svv->tokBufferPtr)
		{ error = kOutOfMemory; goto FAILURE2; }
	svv->ChannelGlobals->tokBuffer = svv->tokBufferPtr;

	/* Open the speech engine channel */
	err = (*(svv->funcList->e_OpenSpeechChannel_FUNC)) (svv->ChannelGlobals);
	if (err != kNoError)
		{ error = err; goto FAILURE2; }

	sharedPtr->instanceCount += 1;
	return (long) noErr;

FAILURE1:
	error = kNoDataTables;
	goto FAILURE;

FAILURE2:
	CloseSpeechChannel (svv);
FAILURE:
	MaybeDisposeSharedGlobals (sharedPtr);
	return (long) error;
}


/* ------------------------------------------------------------------ */
/* _CloseSpeech                                                         */
/* ------------------------------------------------------------------ */

long _CloseSpeech (shellVarPtr svv)
{
	BESharedPtr sharedPtr;

	if (svv)
		{
		sharedPtr = (BESharedPtr) svv->sPtr;
		CloseSpeechChannel (svv);
		sharedPtr->instanceCount -= 1;
		MaybeDisposeSharedGlobals (sharedPtr);
		}

	return (long) noErr;
}


/* ------------------------------------------------------------------ */
/* _SpeakBuffer – synchronous: fills samples until lastSndBuffer set   */
/* ------------------------------------------------------------------ */

long _SpeakBuffer (shellVarPtr svv, Ptr textBuf, long byteLen, long controlFlags)
{
	short error;

	if (svv->ChannelGlobals->Busy)
		return (long) synthNotReady;

	svv->inSpeechDT      = 0;
	svv->doAnotherBuffer = 0;
	svv->lastSndBuffer   = 0;
	svv->firstBuffer     = 1;
	svv->doneWithSpeech  = 0;
	svv->stopSpeech      = 0;
	svv->soundCB_Count   = 0;
	svv->cb_Count        = 0;

	/* Reset accumulated output for this utterance */
	svv->outputLen = 0;

	error = e_SpeakBuffer (svv->ChannelGlobals, textBuf, byteLen, controlFlags);

	if (error == kNoError)
		{
		/* Fill first buffer */
		svv->ChannelGlobals->sampleBuffer = (unsigned char *)svv->wh[0]->lpData;
		synth_FillNextSampBuffer (svv->ChannelGlobals);
		AccumulateSamples (svv, svv->wh[0]);

		/* Fill second buffer (unless already done) */
		if (!svv->lastSndBuffer)
			{
			svv->ChannelGlobals->sampleBuffer = (unsigned char *)svv->wh[1]->lpData;
			synth_FillNextSampBuffer (svv->ChannelGlobals);
			AccumulateSamples (svv, svv->wh[1]);
			}

		/* Keep alternating until the engine signals last buffer */
		while (!svv->lastSndBuffer)
			{
			svv->ChannelGlobals->sampleBuffer = (unsigned char *)svv->wh[0]->lpData;
			synth_FillNextSampBuffer (svv->ChannelGlobals);
			AccumulateSamples (svv, svv->wh[0]);

			if (!svv->lastSndBuffer)
				{
				svv->ChannelGlobals->sampleBuffer = (unsigned char *)svv->wh[1]->lpData;
				synth_FillNextSampBuffer (svv->ChannelGlobals);
				AccumulateSamples (svv, svv->wh[1]);
				}
			}

		svv->ChannelGlobals->Busy = 0;
		gSpeechIsDone = 1;
		}
	else if (error == kNothingToSpeak)
		{
		svv->ChannelGlobals->Busy = 0;
		gSpeechIsDone = 1;
		error = kNoError;
		}

	return (long) error;
}


/* ------------------------------------------------------------------ */
/* Voice pack loader                                                    */
/* ------------------------------------------------------------------ */

/* Runtime state for a loaded voice pack (lives for the process lifetime) */
static voiceData      g_LoadedVoice;
static unsigned char *g_LoadedPCMWave = NULL;

/* Read a big-endian int16 from a byte buffer */
static int16_t vp_rd16 (const uint8_t *p)
{
	return (int16_t)((uint16_t)p[0] << 8 | (uint16_t)p[1]);
}

/* Read a big-endian int32 from a byte buffer */
static int32_t vp_rd32 (const uint8_t *p)
{
	return (int32_t)(  (uint32_t)p[0] << 24
	                 | (uint32_t)p[1] << 16
	                 | (uint32_t)p[2] <<  8
	                 | (uint32_t)p[3]);
}

/* Write a value as little-endian uint32 into a byte buffer */
static void vp_wr_le32 (uint8_t *p, uint32_t v)
{
	p[0] = (uint8_t)(v);
	p[1] = (uint8_t)(v >>  8);
	p[2] = (uint8_t)(v >> 16);
	p[3] = (uint8_t)(v >> 24);
}

/* Read an entire file into a malloc'd buffer; caller must free it. */
static uint8_t *vp_read_file (const char *path, size_t *out_len)
{
	FILE   *f  = fopen(path, "rb");
	uint8_t *buf;
	long    sz;

	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	sz = ftell(f);
	rewind(f);
	if (sz <= 0) { fclose(f); return NULL; }
	buf = (uint8_t *)malloc((size_t)sz);
	if (!buf)    { fclose(f); return NULL; }
	if (fread(buf, 1, (size_t)sz, f) != (size_t)sz)
		{ free(buf); fclose(f); return NULL; }
	fclose(f);
	*out_len = (size_t)sz;
	return buf;
}

/*
 * parse_voice_description
 *
 * Parses a MacinTalk 3 VoiceDescription binary (big-endian) into a voiceData
 * struct.  The file layout is:
 *   [0..361]  Base VoiceDescription header (362 bytes), creator "mtk3"
 *   [362..]   VOICE_DATA fields (428 bytes full, 352 bytes compact)
 *
 * Returns 0 on success, -1 if the file is too small or has wrong creator.
 */
static int parse_voice_description (const uint8_t *data, size_t len, voiceData *vd)
{
	const size_t  VOICE_DATA_OFF = 362;
	const uint8_t *d;
	size_t         avail;
	int            i;

	if (len < VOICE_DATA_OFF + 94)           /* need at least through waveType */
		return -1;
	if (memcmp(data + 4, "mtk3", 4) != 0)
		return -1;

	memset(vd, 0, sizeof(voiceData));
	d     = data + VOICE_DATA_OFF;
	avail = len  - VOICE_DATA_OFF;

#define RD16(off)  ((size_t)(off)+2 <= avail ? vp_rd16(d+(off)) : 0)
#define RD32(off)  ((size_t)(off)+4 <= avail ? vp_rd32(d+(off)) : 0)

	vd->pitch         = RD16(0);
	vd->pitchRange    = RD16(2);
	vd->stressGain    = RD16(4);
	vd->rate          = RD16(6);
	vd->voice         = RD16(8);    /* voice_type_sel: 0=male tables, 1=female */
	vd->vGain         = RD16(10);
	vd->aGain         = RD16(12);
	vd->aCycle        = RD16(14);
	vd->f4_Freq       = RD16(16);
	vd->f4_BW         = RD16(18);
	vd->f4p_Freq      = RD16(20);
	vd->f4p_BW        = RD16(22);
	vd->f5p_Freq      = RD16(24);
	vd->f5p_BW        = RD16(26);
	vd->f6p_Freq      = RD16(28);
	vd->f6p_BW        = RD16(30);
	vd->nasal_Base    = RD16(32);
	vd->nasal_targ    = RD16(34);
	vd->nasal_BW      = RD16(36);
	vd->locus         = RD16(38);
	vd->bwGain1       = RD16(40);
	vd->bwGain2       = RD16(42);
	vd->bwGain3       = RD16(44);
	vd->f1_Offset     = RD16(46);
	vd->f2_Offset     = RD16(48);
	vd->f3_Offset     = RD16(50);
	vd->chorus        = RD16(52);
	vd->nGain         = RD16(54);
	vd->sPitch        = RD16(56);
	vd->sGain         = RD16(58);
	vd->AsperW        = RD16(60);
	vd->voiceVers     = RD16(62);
	vd->riseAmt       = RD16(64);
	vd->fallAmt       = RD16(66);
	vd->riseAmt1      = RD16(68);
	vd->fallAmt1      = RD16(70);
	vd->assertiveness = RD32(72);
	vd->baselineFall  = RD16(76);
	vd->quickness     = RD16(78);
	vd->pitchCmdStep  = RD16(80);
	vd->durCmdStep    = RD16(82);
	vd->down_Ramp_Step = RD32(84);
	vd->stressDurTime = RD16(88);
	vd->tempo         = RD16(90);
	vd->waveType      = RD16(92);

	/* vWave and vWave1: 48 big-endian int16 each */
	for (i = 0; i < 48; i++)
	{
		vd->vWave[i]  = RD16(94  + i * 2);
		vd->vWave1[i] = RD16(190 + i * 2);
	}

	vd->sndID        = RD16(286);
	vd->vowelSync    = RD16(288);
	vd->loopPoint    = RD32(290);
	vd->customForm   = 0;           /* WinTalker has no custom formant tables */
	vd->nasalAmt     = RD16(296);
	vd->vibratoDepth1 = RD16(298);
	vd->vibratoDepth2 = RD16(300);
	vd->vibratoFreq  = RD16(302);
	vd->intonation   = RD16(304);
	vd->portamento   = RD16(306);
	vd->emphVoice    = RD16(308);
	vd->rvbDelay     = RD16(310);
	vd->rvbDepth     = RD16(312);
	vd->rvbWetDry    = RD16(314);
	vd->free1        = RD32(316);
	vd->free2        = RD32(320);
	vd->free3        = RD32(324);
	vd->free4        = RD32(328);
	vd->free5        = RD32(332);
	vd->free6        = RD32(336);
	vd->free7        = RD32(340);
	vd->free8        = RD32(344);

	/* notes[0] = count, notes[1..N] = note values (each a BE int16) */
	{
		short nc = RD16(348);
		if (nc < 0 || nc >= 40) nc = 0;
		vd->notes[0] = nc;
		for (i = 0; i < nc && i < 39; i++)
			vd->notes[i + 1] = RD16(350 + i * 2);
	}

#undef RD16
#undef RD32

	return 0;
}

/*
 * load_pcmwave
 *
 * Loads a MacinTalk 3 PCMWave binary file.  The on-disk format is big-endian
 * but the engine reads the header as native int32_t* (little-endian on x86).
 * This function converts the header fields in-place before returning the buffer.
 */
static unsigned char *load_pcmwave (const char *path, size_t *out_len)
{
	size_t   sz;
	uint8_t *buf = vp_read_file(path, &sz);
	uint32_t sample_len, num_markers, i;

	if (!buf) return NULL;
	if (sz < 8) { free(buf); return NULL; }

	/* Parse big-endian header */
	sample_len  = (uint32_t)buf[0]<<24|(uint32_t)buf[1]<<16|(uint32_t)buf[2]<<8|buf[3];
	num_markers = (uint32_t)buf[4]<<24|(uint32_t)buf[5]<<16|(uint32_t)buf[6]<<8|buf[7];

	if (num_markers > 64) num_markers = 0;      /* sanity clamp */
	if (8 + num_markers * 4 + sample_len > sz)
		{ free(buf); return NULL; }             /* truncated file */

	/* Write header fields back as little-endian */
	vp_wr_le32(buf + 0, sample_len);
	vp_wr_le32(buf + 4, num_markers);
	for (i = 0; i < num_markers; i++)
	{
		uint8_t *mp = buf + 8 + i * 4;
		uint32_t m  = (uint32_t)mp[0]<<24|(uint32_t)mp[1]<<16|(uint32_t)mp[2]<<8|mp[3];
		vp_wr_le32(mp, m);
	}

	*out_len = sz;
	return buf;
}

/*
 * _LoadVoicePack
 *
 * Loads a MacinTalk 3 voice pack from a directory path.  The directory must
 * contain a "VoiceDescription" file and optionally a "PCMWave" file.
 *
 * Example: _LoadVoicePack(svv, "voices/Albert/machine gun")
 */
long _LoadVoicePack (shellVarPtr svv, const char *dir)
{
	char           path[4096];
	uint8_t       *vdbuf;
	size_t         sz;
	short          error;
	voiceDataPtr   tvPtr;
	BESharedPtr    sharedPtr;

	/* --- Load and parse VoiceDescription --- */
	snprintf(path, sizeof(path), "%s/VoiceDescription", dir);
	vdbuf = vp_read_file(path, &sz);
	if (!vdbuf)
	{
		fprintf(stderr, "voice pack: cannot read '%s'\n", path);
		return -1;
	}

	if (parse_voice_description(vdbuf, sz, &g_LoadedVoice) != 0)
	{
		fprintf(stderr, "voice pack: '%s' is not a valid MacinTalk 3 VoiceDescription\n", path);
		free(vdbuf);
		return -1;
	}
	free(vdbuf);

	/* --- Load PCMWave if needed --- */
	if (g_LoadedPCMWave)
	{
		free(g_LoadedPCMWave);
		g_LoadedPCMWave = NULL;
	}

	if (g_LoadedVoice.waveType == kUseSnd || g_LoadedVoice.waveType == kUseSyncSnd)
	{
		snprintf(path, sizeof(path), "%s/PCMWave", dir);
		g_LoadedPCMWave = load_pcmwave(path, &sz);
		if (!g_LoadedPCMWave)
		{
			fprintf(stderr,
			        "voice pack: waveType=%d but cannot read PCMWave at '%s'; "
			        "falling back to harmonic synthesis\n",
			        (int)g_LoadedVoice.waveType, path);
			g_LoadedVoice.waveType = kUseHarm;
		}
	}

	/* --- Initialise formant globals if not already done --- */
	sharedPtr = (BESharedPtr) svv->sPtr;

	if (svv->formantGlobals == NULL)
	{
		svv->formantGlobals = calloc(1, sizeof(formantVar));
		if (!svv->formantGlobals)
			return (long) kOutOfMemory;
		svv->ChannelGlobals->synthVars = svv->formantGlobals;
	}

	Init_FSynthFuncPtrs(sharedPtr->funcList);

	tvPtr = &g_LoadedVoice;
	error = (*(svv->funcList->e_UseVoice_FUNC))(svv->ChannelGlobals, tvPtr, g_LoadedPCMWave);
	return (long) error;
}


/* ------------------------------------------------------------------ */
/* _UseVoice                                                            */
/* ------------------------------------------------------------------ */

long _UseVoice (shellVarPtr svv, short index)
{
	short        error;
	voiceDataPtr tvPtr;
	unsigned char *sample;
	BESharedPtr   sharedPtr;

	error     = kNoError;
	sharedPtr = (BESharedPtr) svv->sPtr;

	switch (index)
		{
		case  0: tvPtr = &Fred_Voice;       sample = NULL;                           break;
		case  1: tvPtr = &Kathy_Voice;      sample = NULL;                           break;
		case  2: tvPtr = &Princess_Voice;   sample = NULL;                           break;
		case  3: tvPtr = &Junior_Voice;     sample = NULL;                           break;
		case  4: tvPtr = &Ralph_Voice;      sample = NULL;                           break;
		case  5: tvPtr = &Whisper_Voice;    sample = NULL;                           break;
		case  6: tvPtr = &Zarvox_Voice;     sample = NULL;                           break;
		case  7: tvPtr = &Trinoids_Voice;   sample = NULL;                           break;
		case  8: tvPtr = &Bubbles_Voice;    sample = (unsigned char *)&Bubbles_Sound;  break;
		case  9: tvPtr = &Boing_Voice;      sample = (unsigned char *)&Boing_Sound;    break;
		case 10: tvPtr = &Bells_Voice;      sample = (unsigned char *)&Bells_Sound;    break;
		case 11: tvPtr = &Hysterical_Voice; sample = (unsigned char *)&Hysterical_Sound; break;
		case 12: tvPtr = &Deranged_Voice;   sample = (unsigned char *)&Deranged_Sound;  break;
		case 13: tvPtr = &GoodNews_Voice;   sample = NULL;                           break;
		case 14: tvPtr = &BadNews_Voice;    sample = NULL;                           break;
		case 15: tvPtr = &PipeOrgan_Voice;  sample = (unsigned char *)&PipeOrgan_Sound; break;
		case 16: tvPtr = &Cellos_Voice;     sample = (unsigned char *)&Cellos_Sound;    break;
		default: tvPtr = &Fred_Voice;       sample = NULL;                           break;
		}

	if (svv->formantGlobals == NULL)
		{
		svv->formantGlobals = calloc (1, sizeof (formantVar));
		if (!svv->formantGlobals)
			{ error = kOutOfMemory; goto VOICE_FAILURE; }
		svv->ChannelGlobals->synthVars = svv->formantGlobals;
		}

	Init_FSynthFuncPtrs (sharedPtr->funcList);

	error = (*(svv->funcList->e_UseVoice_FUNC)) (svv->ChannelGlobals, tvPtr, sample);

VOICE_FAILURE:
	return (long) error;
}


/* ------------------------------------------------------------------ */
/* AccumulateSamples – append one buffer-fill to svv->outputBuf         */
/* ------------------------------------------------------------------ */

static void AccumulateSamples (shellVarPtr svv, WAVEHDR *wh)
{
	long needed;
	long newCap;
	short *newBuf;

	long sampleCount = svv->curSampleLen;	/* 16-bit samples in this fill */
	if (sampleCount <= 0)
		return;

	needed = svv->outputLen + sampleCount;
	if (needed > svv->outputCapacity)
		{
		newCap = svv->outputCapacity ? svv->outputCapacity * 2 : 65536L;
		if (newCap < needed)
			newCap = needed;
		newBuf = (short *) realloc (svv->outputBuf, (size_t)(newCap * sizeof (short)));
		if (!newBuf)
			return;		/* out of memory – skip this chunk */
		svv->outputBuf      = newBuf;
		svv->outputCapacity = newCap;
		}

	memcpy (svv->outputBuf + svv->outputLen,
	        wh->lpData,
	        (size_t)(sampleCount * sizeof (short)));
	svv->outputLen += sampleCount;
}


/* ------------------------------------------------------------------ */
/* WriteWavFile – write accumulated PCM as a standard RIFF/WAV file    */
/* ------------------------------------------------------------------ */

short WriteWavFile (shellVarPtr svv, const char *filename)
{
	FILE    *fp;
	uint32_t dataBytes, riffSize, fmtSize;
	uint16_t u16;
	uint32_t u32;

	if (!svv->outputBuf || svv->outputLen <= 0)
		{
		fprintf (stderr, "WriteWavFile: no audio data to write\n");
		return 1;
		}

	fp = fopen (filename, "wb");
	if (!fp)
		{
		fprintf (stderr, "WriteWavFile: cannot open '%s'\n", filename);
		return 1;
		}

	dataBytes = (uint32_t)(svv->outputLen * sizeof (short));
	fmtSize   = 16;						/* PCM fmt chunk is always 16 bytes */
	riffSize  = 4 + 8 + fmtSize + 8 + dataBytes;

	/* RIFF header */
	fwrite ("RIFF", 1, 4, fp);
	u32 = riffSize; fwrite (&u32, 4, 1, fp);
	fwrite ("WAVE", 1, 4, fp);

	/* fmt chunk */
	fwrite ("fmt ", 1, 4, fp);
	u32 = fmtSize; fwrite (&u32, 4, 1, fp);

	u16 = 1;        fwrite (&u16, 2, 1, fp);	/* PCM format */
	u16 = 1;        fwrite (&u16, 2, 1, fp);	/* mono */
	u32 = 22050;    fwrite (&u32, 4, 1, fp);	/* sample rate */
	u32 = 22050*2;  fwrite (&u32, 4, 1, fp);	/* byte rate */
	u16 = 2;        fwrite (&u16, 2, 1, fp);	/* block align */
	u16 = 16;       fwrite (&u16, 2, 1, fp);	/* bits per sample */

	/* data chunk */
	fwrite ("data", 1, 4, fp);
	u32 = dataBytes; fwrite (&u32, 4, 1, fp);
	fwrite (svv->outputBuf, sizeof (short), (size_t)svv->outputLen, fp);

	fclose (fp);
	return 0;
}


/* ------------------------------------------------------------------ */
/* Shell function-pointer callbacks (called by engine)                  */
/* ------------------------------------------------------------------ */

static void _i_Last_Snd_Buffer (voiceVarPtr vv)
{
	shellVarPtr svv = (shellVarPtr) vv->shellV;
	svv->lastSndBuffer = 1;
}

static void _i_Cur_Sample_Buffer (voiceVarPtr vv, unsigned char *sampleBuffer, long sampleLen)
{
	shellVarPtr svv = (shellVarPtr) vv->shellV;
	svv->curSampleBuffer = sampleBuffer;
	svv->curSampleLen    = sampleLen;
}

static void _i_First_Sample_Buffer (voiceVarPtr vv, unsigned char *sampleBuffer, long sampleLen)
{
	shellVarPtr svv = (shellVarPtr) vv->shellV;
	svv->firstSampleBuffer = sampleBuffer;
	svv->firstSampleLen    = sampleLen;
	svv->curSampleLen      = sampleLen;
}

static void Init_ShellFuncPtrs (moduleFuncPtr mfp)
{
	mfp->_i_Last_Snd_Buffer_FUNC      = (_i_Last_Snd_Buffer_Ptr)      &_i_Last_Snd_Buffer;
	mfp->_i_Cur_Sample_Buffer_FUNC    = (_i_Cur_Sample_Buffer_Ptr)    &_i_Cur_Sample_Buffer;
	mfp->_i_First_Sample_Buffer_FUNC  = (_i_First_Sample_Buffer_Ptr)  &_i_First_Sample_Buffer;
}


/* ------------------------------------------------------------------ */
/* CloseSpeechChannel / MaybeDisposeSharedGlobals                       */
/* ------------------------------------------------------------------ */

static void CloseSpeechChannel (shellVarPtr svv)
{
	if (svv->wh[0]) { free (svv->wh[0]); svv->wh[0] = NULL; }
	if (svv->wh[1]) { free (svv->wh[1]); svv->wh[1] = NULL; }

	if (svv->soundBufPtr)    { free (svv->soundBufPtr);    svv->soundBufPtr    = NULL; }
	if (svv->tokBufferPtr)   { free (svv->tokBufferPtr);   svv->tokBufferPtr   = NULL; }
	if (svv->reverbBufPtr)   { free (svv->reverbBufPtr);   svv->reverbBufPtr   = NULL; }
	if (svv->formantGlobals) { free (svv->formantGlobals); svv->formantGlobals = NULL; }
	if (svv->ChannelGlobals) { free (svv->ChannelGlobals); svv->ChannelGlobals = NULL; }
	if (svv->outputBuf)      { free (svv->outputBuf);      svv->outputBuf      = NULL; }

	free (svv);
}

static void MaybeDisposeSharedGlobals (BESharedPtr sharedPtr)
{
	if (!sharedPtr)
		return;
	if (sharedPtr->instanceCount != 0)
		return;

	/* Free the index array allocated by MakeDictPtrsAbsolute */
	if (sharedPtr->DictMem.index)
		{
		free (sharedPtr->DictMem.index);
		sharedPtr->DictMem.index = NULL;
		}

	/* Free DictRaw only if it was malloc'd from disk */
	if (sharedPtr->dictRawOwned && sharedPtr->DictRaw)
		{
		free ((char *)sharedPtr->DictRaw);
		sharedPtr->DictRaw = NULL;
		}

	if (sharedPtr->funcList)
		free (sharedPtr->funcList);

	gComponentRefcon = 0;
	free (sharedPtr);
}
