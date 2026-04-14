/*
 * Web.c – Emscripten platform entry point for WinTalker.
 *
 * Defines the engine globals (normally in Wavinout.c / src/main.c,
 * both excluded here) and exposes a minimal EMSCRIPTEN_KEEPALIVE API
 * that the JavaScript frontend calls directly.
 *
 * Synthesis is synchronous: wt_speak() blocks until all samples are
 * accumulated in gInstanceStorage->outputBuf, then returns.  The JS
 * side reads the buffer and plays it via the Web Audio API.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <emscripten.h>

#include "linux_compat.h"
#include "SpeechEqu.h"
#include "MT4.h"
#include "Fsynth.h"
#include "Versions.h"
#include "Macintosh.h"

/* ------------------------------------------------------------------ */
/* Engine globals – defined here (Wavinout.c / src/main.c excluded)   */
/* ------------------------------------------------------------------ */

shellVarPtr  gInstanceStorage = NULL;
uintptr_t    gComponentRefcon = 0;
short        gDoDT            = 0;
short        gSpeechIsDone    = 0;
char         msg[MSG_LEN+1]   = {0};

/* ------------------------------------------------------------------ */
/* Platform layer (Linux.c)                                            */
/* ------------------------------------------------------------------ */

extern long _OpenSpeech  (void);
extern long _CloseSpeech (shellVarPtr svv);
extern long _SpeakBuffer (shellVarPtr svv, Ptr textBuf, long byteLen, long controlFlags);
extern long _UseVoice    (shellVarPtr svv, short index);

/* ------------------------------------------------------------------ */
/* Exported API                                                        */
/* ------------------------------------------------------------------ */

/* Initialise the speech engine.  Call once on page load.
 * Returns 0 on success, non-zero on error. */
EMSCRIPTEN_KEEPALIVE
int wt_init (void)
{
    return (int) _OpenSpeech ();
}

/* Select a built-in voice (0 = Fred … 16 = Cellos).
 * Returns 0 on success. */
EMSCRIPTEN_KEEPALIVE
int wt_set_voice (int index)
{
    if (!gInstanceStorage) return -1;
    return (int) _UseVoice (gInstanceStorage, (short) index);
}

/* Synthesise text.  Blocks until complete; audio is in the PCM buffer.
 * Returns 0 on success. */
EMSCRIPTEN_KEEPALIVE
int wt_speak (const char *text)
{
    if (!gInstanceStorage || !text) return -1;
    return (int) _SpeakBuffer (gInstanceStorage, (Ptr) text, (long) strlen (text), 0);
}

/* Pointer to the synthesised PCM buffer (signed 16-bit, 22050 Hz mono).
 * Valid until the next wt_speak() call. */
EMSCRIPTEN_KEEPALIVE
short *wt_get_buffer (void)
{
    if (!gInstanceStorage) return NULL;
    return gInstanceStorage->outputBuf;
}

/* Number of samples in the PCM buffer. */
EMSCRIPTEN_KEEPALIVE
int wt_get_buffer_len (void)
{
    if (!gInstanceStorage) return 0;
    return (int) gInstanceStorage->outputLen;
}

/* Shut down the engine. */
EMSCRIPTEN_KEEPALIVE
void wt_close (void)
{
    if (gInstanceStorage)
        _CloseSpeech (gInstanceStorage);
}
