/*
 * GodotPlatform.c – GDExtension platform layer for WinTalker/Lintalker.
 *
 * Defines the engine globals (Linux.c / main.c declares them extern) and
 * implements the functions that don't exist in Linux.c but are needed by
 * the GDExtension:
 *
 *   _SetCustomVoice     – install a caller-supplied voiceData struct
 *   _InstallLipsync     – attach the per-frame lipsync accumulator
 *   _GetLipsyncFrames   – retrieve the accumulated lipsync frame buffer
 *   _ClearLipsync       – reset and free the lipsync buffer
 *
 * Linux.c is compiled with NO_FILESYSTEM so it always uses the built-in
 * english_lex_data dictionary.
 */

#include <stdlib.h>
#include <string.h>

#include "../../../include/SpeechEqu.h"
#include "../../../include/MT4.h"
#include "../../../include/Fsynth.h"
#include "../../../include/Versions.h"
#include "../../../include/Macintosh.h"

/* ------------------------------------------------------------------ */
/* Engine globals (extern'd by Linux.c; defined once here)             */
/* ------------------------------------------------------------------ */

shellVarPtr  gInstanceStorage = NULL;
uintptr_t    gComponentRefcon = 0;
short        gDoDT            = 0;
short        gSpeechIsDone    = 0;
char         msg[MSG_LEN + 1] = {0};

/* ------------------------------------------------------------------ */
/* External symbols from Linux.c                                       */
/* ------------------------------------------------------------------ */

extern void Init_FSynthFuncPtrs (moduleFuncPtr mfp);

/* ------------------------------------------------------------------ */
/* Lipsync frame layout                                                 */
/*                                                                      */
/* Each frame is 4 floats (16 bytes):                                  */
/*   [0] phoneme       – current phoneme opcode (cast to float)        */
/*   [1] prev_phoneme  – previous phoneme opcode                       */
/*   [2] next_phoneme  – next phoneme opcode (coarticulation look-fwd) */
/*   [3] blend         – position within phoneme 0.0 – 1.0             */
/* ------------------------------------------------------------------ */

#define LIPSYNC_FLOATS_PER_FRAME 4

typedef struct
{
    float *buf;     /* heap-allocated float buffer */
    int    count;   /* frames accumulated so far   */
    int    cap;     /* frames capacity             */
} LipsyncAccum;

/* Per-instance lipsync accumulator (one per shellVarPtr).
 * We keep it in a static slot since the extension is single-instance.
 * If multi-voice support is ever needed, embed it in the shellVar. */
static LipsyncAccum g_lipsync = { NULL, 0, 0 };

/* ------------------------------------------------------------------ */
/* Lipsync callback – called by the engine once per audio frame        */
/* ------------------------------------------------------------------ */

static void lipsync_frame_cb (voiceVarPtr vv, void *userdata)
{
    LipsyncAccum *acc = (LipsyncAccum *) userdata;
    float blend;
    float *slot;

    /* Grow buffer if needed (double strategy, initial 4096 frames ≈ 20s) */
    if (acc->count >= acc->cap)
        {
        int   newcap = acc->cap ? acc->cap * 2 : 4096;
        float *newbuf = (float *) realloc (acc->buf,
                            (size_t) newcap * LIPSYNC_FLOATS_PER_FRAME * sizeof (float));
        if (!newbuf) return;   /* drop frame on OOM rather than crash */
        acc->buf = newbuf;
        acc->cap = newcap;
        }

    blend = (vv->cur_Phon_Dur_CF > 0)
          ? (float) vv->dur_Done_in_Phon_CF / (float) vv->cur_Phon_Dur_CF
          : 1.0f;
    if (blend > 1.0f) blend = 1.0f;

    slot    = acc->buf + (size_t) acc->count * LIPSYNC_FLOATS_PER_FRAME;
    slot[0] = (float) vv->cur_Phon_CF;
    slot[1] = (float) vv->prev_Phon_CF;
    slot[2] = (float) vv->next_Phon_CF;
    slot[3] = blend;

    acc->count++;
}

/* ------------------------------------------------------------------ */
/* _InstallLipsync                                                      */
/*                                                                      */
/* Attaches the lipsync accumulator to the engine channel and clears   */
/* any previously accumulated frames.  Call before _SpeakBuffer.       */
/* ------------------------------------------------------------------ */

void _InstallLipsync (shellVarPtr svv)
{
    if (!svv || !svv->ChannelGlobals) return;
    g_lipsync.count = 0;   /* reset without freeing — reuse buffer */
    svv->ChannelGlobals->lipsync_cb       = lipsync_frame_cb;
    svv->ChannelGlobals->lipsync_userdata = &g_lipsync;
}

/* ------------------------------------------------------------------ */
/* _GetLipsyncFrames                                                    */
/*                                                                      */
/* Returns a pointer to the accumulated float buffer and the number of */
/* frames in *out_count.  Valid until the next _InstallLipsync call.   */
/* ------------------------------------------------------------------ */

const float *_GetLipsyncFrames (int *out_count)
{
    if (out_count) *out_count = g_lipsync.count;
    return g_lipsync.buf;
}

/* ------------------------------------------------------------------ */
/* _ClearLipsync – free the buffer entirely                            */
/* ------------------------------------------------------------------ */

void _ClearLipsync (shellVarPtr svv)
{
    if (svv && svv->ChannelGlobals)
        {
        svv->ChannelGlobals->lipsync_cb       = NULL;
        svv->ChannelGlobals->lipsync_userdata = NULL;
        }
    free (g_lipsync.buf);
    g_lipsync.buf   = NULL;
    g_lipsync.count = 0;
    g_lipsync.cap   = 0;
}

/* ------------------------------------------------------------------ */
/* _SetCustomVoice                                                      */
/* ------------------------------------------------------------------ */

long _SetCustomVoice (shellVarPtr svv, voiceData *vd, unsigned char *pcmwave)
{
    BESharedPtr sharedPtr;
    short       error;

    if (!svv || !vd)
        return -1L;

    sharedPtr = (BESharedPtr) svv->sPtr;

    if (svv->formantGlobals == NULL)
        {
        svv->formantGlobals = calloc (1, sizeof (formantVar));
        if (!svv->formantGlobals)
            return (long) kOutOfMemory;
        svv->ChannelGlobals->synthVars = svv->formantGlobals;
        }

    Init_FSynthFuncPtrs (sharedPtr->funcList);

    error = (*(svv->funcList->e_UseVoice_FUNC)) (svv->ChannelGlobals, vd, pcmwave);
    return (long) error;
}
