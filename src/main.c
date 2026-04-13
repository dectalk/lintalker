/*
 * main.c – WinTalker command-line interface (Linux port)
 *
 * Usage:
 *   wintalker [OPTIONS] "text to speak"
 *
 * Options:
 *   -v N        voice index 0-16 (default 0 = Fred)
 *   -o FILE     output WAV file (default: output.wav)
 *   -l          list available voices and exit
 *
 * The program synthesises speech synchronously and writes a 22 050 Hz,
 * mono, 16-bit PCM WAV file.  English.lex must be in the current directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>

#ifndef __SPEECHSAY__
	#include "MT4.h"
#endif

#ifndef __SPEECHMACINTOSH__
	#include "Macintosh.h"
#endif


/* ------------------------------------------------------------------ */
/* Globals (shared with Linux.c / Engine internals)                     */
/* ------------------------------------------------------------------ */

shellVarPtr  gInstanceStorage = NULL;
uintptr_t    gComponentRefcon = 0;
short        gDoDT            = 0;
short        gSpeechIsDone    = 1;
char         msg[MSG_LEN + 1];


/* ------------------------------------------------------------------ */
/* Imports from Linux.c                                                 */
/* ------------------------------------------------------------------ */

extern long  _OpenSpeech      (void);
extern long  _CloseSpeech     (shellVarPtr svv);
extern long  _UseVoice        (shellVarPtr svv, short index);
extern long  _LoadVoicePack   (shellVarPtr svv, const char *dir);
extern long  _SpeakBuffer     (shellVarPtr svv, Ptr textBuf, long byteLen, long controlFlags);
extern short WriteWavFile     (shellVarPtr svv, const char *filename);


/* ------------------------------------------------------------------ */
/* Voice name table                                                     */
/* ------------------------------------------------------------------ */

static const char *voiceNames[] =
{
	"Fred",       /* 0  */
	"Kathy",      /* 1  */
	"Princess",   /* 2  */
	"Junior",     /* 3  */
	"Ralph",      /* 4  */
	"Whisper",    /* 5  */
	"Zarvox",     /* 6  */
	"Trinoids",   /* 7  */
	"Bubbles",    /* 8  */
	"Boing",      /* 9  */
	"Bells",      /* 10 */
	"Hysterical", /* 11 */
	"Deranged",   /* 12 */
	"GoodNews",   /* 13 */
	"BadNews",    /* 14 */
	"PipeOrgan",  /* 15 */
	"Cellos",     /* 16 */
};

#define NUM_VOICES ((int)(sizeof(voiceNames) / sizeof(voiceNames[0])))


/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */

int main (int argc, char *argv[])
{
	int         voiceIdx   = 0;
	const char *outFile    = "output.wav";
	const char *text       = NULL;
	const char *voicePackDir = NULL;   /* -vp DIR: load a voice pack instead */
	long        err;
	int         i;

	/* Parse arguments */
	for (i = 1; i < argc; i++)
		{
		if (strcmp (argv[i], "-l") == 0)
			{
			printf ("Available voices:\n");
			for (int v = 0; v < NUM_VOICES; v++)
				printf ("  %2d  %s\n", v, voiceNames[v]);
			return 0;
			}
		else if (strcmp (argv[i], "-v") == 0 && i + 1 < argc)
			{
			const char *varg = argv[++i];
			/* Accept numeric index or name (case-insensitive) */
			if (varg[0] >= '0' && varg[0] <= '9')
				{
				voiceIdx = atoi (varg);
				}
			else
				{
				voiceIdx = -1;
				for (int v = 0; v < NUM_VOICES; v++)
					{
					if (strcasecmp (varg, voiceNames[v]) == 0)
						{ voiceIdx = v; break; }
					}
				}
			if (voiceIdx < 0 || voiceIdx >= NUM_VOICES)
				{
				fprintf (stderr, "Unknown voice '%s'. Use -l to list voices.\n", varg);
				return 1;
				}
			}
		else if (strcmp (argv[i], "-vp") == 0 && i + 1 < argc)
			{
			voicePackDir = argv[++i];
			}
		else if (strcmp (argv[i], "-o") == 0 && i + 1 < argc)
			{
			outFile = argv[++i];
			}
		else if (argv[i][0] != '-')
			{
			text = argv[i];
			}
		else
			{
			fprintf (stderr, "Unknown option: %s\n", argv[i]);
			fprintf (stderr,
			         "Usage: wintalker [-v N] [-vp DIR] [-o FILE] [-l] \"text\"\n");
			return 1;
			}
		}

	if (!text)
		{
		fprintf (stderr,
		         "Usage: wintalker [-v NAME|N] [-vp DIR] [-o FILE] [-l] \"text to speak\"\n"
		         "  -v NAME  voice name or index 0-%d (default 0 = Fred)\n"
		         "  -vp DIR  load a MacinTalk 3 voice pack directory\n"
		         "  -o FILE  output WAV file  (default: output.wav)\n"
		         "  -l       list voices\n",
		         NUM_VOICES - 1);
		return 1;
		}

	/* Initialise speech engine */
	err = _OpenSpeech ();
	if (err != noErr)
		{
		fprintf (stderr, "Error opening speech engine: %ld\n", err);
		return 1;
		}

	/* Select voice */
	if (voicePackDir)
		{
		err = _LoadVoicePack (gInstanceStorage, voicePackDir);
		if (err != noErr)
			{
			fprintf (stderr, "Error loading voice pack '%s': %ld\n", voicePackDir, err);
			_CloseSpeech (gInstanceStorage);
			gInstanceStorage = NULL;
			return 1;
			}
		printf ("Speaking with voice pack '%s': %s\n", voicePackDir, text);
		}
	else
		{
		err = _UseVoice (gInstanceStorage, (short) voiceIdx);
		if (err != noErr)
			{
			fprintf (stderr, "Error selecting voice %d: %ld\n", voiceIdx, err);
			_CloseSpeech (gInstanceStorage);
			gInstanceStorage = NULL;
			return 1;
			}
		printf ("Speaking with voice '%s': %s\n", voiceNames[voiceIdx], text);
		}

	err = _SpeakBuffer (gInstanceStorage, (Ptr)text, (long)strlen (text), 0);
	if (err != noErr)
		{
		fprintf (stderr, "SpeakBuffer error: %ld\n", err);
		_CloseSpeech (gInstanceStorage);
		gInstanceStorage = NULL;
		return 1;
		}

	/* Write WAV */
	if (WriteWavFile (gInstanceStorage, outFile) != 0)
		{
		fprintf (stderr, "Failed to write WAV file '%s'\n", outFile);
		_CloseSpeech (gInstanceStorage);
		gInstanceStorage = NULL;
		return 1;
		}

	printf ("Written to %s  (%ld samples, %.2f s)\n",
	        outFile,
	        gInstanceStorage->outputLen,
	        (double) gInstanceStorage->outputLen / 22050.0);

	/* Clean up */
	_CloseSpeech (gInstanceStorage);
	gInstanceStorage = NULL;
	return 0;
}
