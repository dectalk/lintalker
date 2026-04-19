#include "MT4.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* External functions from FrontEnd.c */
extern void GetNextCh(voiceVarPtr vv);

/*
 * DECtalk note N → internal pitch value stored in user_Note_Buf1 for EC_sing.
 *
 * DECtalk note N is MIDI note (N + 42).
 * Replicates e_MidiToPitch((N+42)<<8) from BackEnd.c inline so Singing.c
 * (a FrontEnd file) doesn't need to call the back-end directly.
 *
 * Returns 0 for note 0 (no pitch change).
 */
static short DtalkNoteToInternalPitch(int dtNote)
{
	short midiQ8;
	long  pitch;

	if (dtNote <= 0)
		return 0;

	midiQ8 = (short)((dtNote + 42) << 8);  /* DECtalk note → Q8.8 MIDI */
	if (midiQ8 < 0x1F59)                    /* kMIDI_50HZ — below 50 Hz, clamp */
		return 0;
	midiQ8 -= 0x1F59;
	pitch = (((long)midiQ8 * 0x1555L) + 0x8000L) >> 16;
	return (short)pitch;
}

/*
 * Map a lowercase DECtalk phoneme string to a wintalker opcode.
 * Returns (opcode + 1) so that 0 means "unknown" (since _IY_ == 0).
 */
static short MapDectalkPhoneme(const char *p)
{
	/* vowels */
	if (!strcmp(p,"iy")) return _IY_+1;
	if (!strcmp(p,"ih")) return _IH_+1;
	if (!strcmp(p,"eh")) return _EH_+1;
	if (!strcmp(p,"ae")) return _AE_+1;
	if (!strcmp(p,"aa")) return _AA_+1;
	if (!strcmp(p,"ah")) return _AH_+1;
	if (!strcmp(p,"ao")) return _AO_+1;
	if (!strcmp(p,"uh")) return _UH_+1;
	if (!strcmp(p,"ax")) return _AX_+1;
	if (!strcmp(p,"er")) return _ER_+1;
	if (!strcmp(p,"ey")) return _EY_+1;
	if (!strcmp(p,"ay")) return _AY_+1;
	if (!strcmp(p,"oy")) return _OY_+1;
	if (!strcmp(p,"aw")) return _AW_+1;
	if (!strcmp(p,"ow")) return _OW_+1;
	if (!strcmp(p,"uw")) return _UW_+1;
	if (!strcmp(p,"yu")) return _YU_+1;
	if (!strcmp(p,"ix")) return _IX_+1;
	/* r-coloured vowels */
	if (!strcmp(p,"ir")) return _IR_+1;
	if (!strcmp(p,"xr")) return _XR_+1;
	if (!strcmp(p,"ar")) return _AR_+1;
	if (!strcmp(p,"or")) return _OR_+1;
	if (!strcmp(p,"ur")) return _UR_+1;
	/* syllabic consonants */
	if (!strcmp(p,"el")) return _EL_+1;
	if (!strcmp(p,"en")) return _EN_+1;
	/* 2-char consonants */
	if (!strcmp(p,"rr")) return _RX_+1;  /* rhotic / syllabic-r */
	if (!strcmp(p,"hx")) return _h_+1;
	if (!strcmp(p,"nx")) return _n_+1;
	if (!strcmp(p,"dx")) return _DX_+1;
	if (!strcmp(p,"zh")) return _ZH_+1;
	if (!strcmp(p,"sh")) return _SH_+1;
	if (!strcmp(p,"th")) return _TH_+1;
	if (!strcmp(p,"dh")) return _DH_+1;
	if (!strcmp(p,"ch")) return _CH_+1;
	if (!strcmp(p,"jh")) return _JH_+1;
	if (!strcmp(p,"ng")) return _NG_+1;
	if (!strcmp(p,"wh")) return _w_+1;
	/* 1-char consonants */
	if (!strcmp(p,"b"))  return _b_+1;
	if (!strcmp(p,"d"))  return _d_+1;
	if (!strcmp(p,"f"))  return _f_+1;
	if (!strcmp(p,"g"))  return _g_+1;
	if (!strcmp(p,"h"))  return _h_+1;
	if (!strcmp(p,"k"))  return _k_+1;
	if (!strcmp(p,"l"))  return _l_+1;
	if (!strcmp(p,"m"))  return _m_+1;
	if (!strcmp(p,"n"))  return _n_+1;
	if (!strcmp(p,"p"))  return _p_+1;
	if (!strcmp(p,"r"))  return _r_+1;
	if (!strcmp(p,"s"))  return _s_+1;
	if (!strcmp(p,"t"))  return _t_+1;
	if (!strcmp(p,"v"))  return _v_+1;
	if (!strcmp(p,"w"))  return _w_+1;
	if (!strcmp(p,"y"))  return _y_+1;
	if (!strcmp(p,"z"))  return _z_+1;
	if (!strcmp(p,"q"))  return _QX_+1;
	/* silence */
	if (!strcmp(p,"_"))  return _SIL_+1;
	return 0;
}

static void MyStuffBECommand(unsigned char *buf, unsigned long cmd, unsigned long val)
{
	BECommandPtr cb;

	cb = &((BECommandPtr)&buf[1])[buf[0]];
	cb[0] = BE_ECmd;
	cb[1] = 6;
	cb[2] = (uint16_t)((uint32_t)cmd >> 16);
	cb[3] = (uint16_t)((uint32_t)cmd & 0xFFFF);
	cb[4] = (uint16_t)((uint32_t)val >> 16);
	cb[5] = (uint16_t)((uint32_t)val & 0xFFFF);
	buf[0] += 6;
}

/*
 * Read one DECtalk phoneme from the input stream using longest-match
 * (try 2-char first, then 1-char).  Handles the '_' silence symbol.
 * Advances vv->Ch past the matched characters.
 * Returns the wintalker opcode.
 */
static short ReadOneDtalkPhoneme(voiceVarPtr vv)
{
	char  two[3], one[2];
	short op;

	if (vv->Ch == '_')
		{ GetNextCh(vv); return _SIL_; }

	if (!isalpha((unsigned char)vv->Ch))
		return _SIL_;

	/* try 2-char match */
	if (isalpha((unsigned char)vv->NextCh))
		{
		two[0] = (char)tolower((unsigned char)vv->Ch);
		two[1] = (char)tolower((unsigned char)vv->NextCh);
		two[2] = '\0';
		op = MapDectalkPhoneme(two);
		if (op != 0)
			{
			GetNextCh(vv);          /* consume 1st char */
			GetNextCh(vv);          /* consume 2nd char */
			return (short)(op - 1);
			}
		}

	/* 1-char match */
	one[0] = (char)tolower((unsigned char)vv->Ch);
	one[1] = '\0';
	op = MapDectalkPhoneme(one);
	GetNextCh(vv);
	return (op != 0) ? (short)(op - 1) : _SIL_;
}


/*
 * ParseSinging – called by FrontEnd whenever:
 *   (a) vv->Ch == '[' and !vv->AtCmdBegin  (new phoneme/command block)
 *   (b) vv->pendingSingPhoneme != 0         (state left from previous call)
 *
 * pendingSingPhoneme encoding:
 *   0      : not in singing mode
 *  -1      : inside a [...] phoneme block; no phoneme held
 *   N > 0  : phoneme (N-1) is waiting to be emitted; still inside block
 *
 * pendingSingNote (repurposed): 1 = next phoneme is first of its block,
 *   0 = subsequent phoneme within block.
 *
 * One token is yielded per call; the caller loops back via the trigger.
 *
 * DECtalk control commands  [:cmd ...]  are silently skipped (kNullTok).
 *
 * Word boundary placement:
 *   The engine's coarticulation rules use _Word_ to mark word-initial position
 *   (sets kWord_Start / kWord_Initial_Consonant on the phoneme that follows).
 *   _Word_ must come BEFORE the phoneme it marks, not after.  Within a singing
 *   block all phonemes belong to one phrase, so only the FIRST phoneme of each
 *   block gets the _Word_ prefix; all others are emitted bare so coarticulation
 *   treats them as word-medial.
 */
void ParseSinging(voiceVarPtr vv, FETokenPtr tok)
{
	short opcode;
	int   duration, note;

	/*----------------------------------------------------------*/
	/* (1) Drain a held phoneme from the previous call           */
	/*----------------------------------------------------------*/
	if (vv->pendingSingPhoneme > 0)
		{
		tok->tokType = kRawPhonemeTok;
		if (vv->pendingSingNote)
			{
			/* first phoneme of block – prefix with _Word_ */
			tok->phonStr[0] = 2;
			tok->phonStr[1] = (Byte)_Word_;
			tok->phonStr[2] = (Byte)(vv->pendingSingPhoneme - 1);
			vv->pendingSingNote = 0;
			}
		else
			{
			/* word-medial phoneme – no boundary */
			tok->phonStr[0] = 1;
			tok->phonStr[1] = (Byte)(vv->pendingSingPhoneme - 1);
			}
		vv->pendingSingPhoneme = -1;    /* stay in block */
		return;
		}

	/*----------------------------------------------------------*/
	/* (2) Start of a new '[' block                              */
	/*----------------------------------------------------------*/
	if (vv->pendingSingPhoneme == 0)
		{
		if (vv->Ch == '[')
			GetNextCh(vv);

		/* DECtalk control command  [:...]  – skip silently */
		if (vv->Ch == ':')
			{
			while (vv->Ch != ']' && vv->Ch != kEOFCh)
				GetNextCh(vv);
			if (vv->Ch == ']')
				GetNextCh(vv);
			tok->tokType = kNullTok;
			return;
			}

		/* Entering a phoneme block: mark first phoneme */
		vv->pendingSingPhoneme = -1;
		vv->pendingSingNote    = 1;
		}

	/*----------------------------------------------------------*/
	/* (3) Parse one phoneme from the active block               */
	/*     (pendingSingPhoneme == -1 here)                       */
	/*----------------------------------------------------------*/
	while (vv->Ch != ']' && vv->Ch != kEOFCh)
		{
		/* skip whitespace between phonemes */
		if (isspace((unsigned char)vv->Ch))
			{ GetNextCh(vv); continue; }

		/* skip any stray non-phoneme chars (shouldn't appear) */
		if (!isalpha((unsigned char)vv->Ch) && vv->Ch != '_')
			{ GetNextCh(vv); continue; }

		opcode   = ReadOneDtalkPhoneme(vv);
		duration = 0;
		note     = 0;

		/* optional  <duration[,note]>  */
		if (vv->Ch == '<')
			{
			GetNextCh(vv);
			while (isdigit((unsigned char)vv->Ch))
				{ duration = duration * 10 + (vv->Ch - '0'); GetNextCh(vv); }
			if (vv->Ch == ',')
				{
				GetNextCh(vv);
				while (isdigit((unsigned char)vv->Ch))
					{ note = note * 10 + (vv->Ch - '0'); GetNextCh(vv); }
				}
			while (vv->Ch != '>' && vv->Ch != ']' && vv->Ch != kEOFCh)
				GetNextCh(vv);
			if (vv->Ch == '>')
				GetNextCh(vv);
			}

		if (duration > 0 || note > 0)
			{
			/* Emit EC_sing; stash phoneme for next call.
			 * note <= 37: musical note  → convert to internal pitch (IIR lock in DoNote)
			 * note >  37: raw Hz target → store as negative short  (linear glide in DoNote) */
			short pitch = (note > 37) ? -(short)note : DtalkNoteToInternalPitch(note);
			tok->tokType    = kECommandTok;
			tok->phonStr[0] = 0;
			MyStuffBECommand(tok->phonStr, EC_sing,
				(unsigned long)(((unsigned long)(unsigned short)pitch << 16) |
				                (unsigned long)(unsigned short)duration));
			vv->pendingSingPhoneme = (short)(opcode + 1);  /* N > 0 */
			return;
			}
		else
			{
			/* Plain phoneme – no duration/pitch override, stay in block */
			tok->tokType = kRawPhonemeTok;
			if (vv->pendingSingNote)
				{
				tok->phonStr[0] = 2;
				tok->phonStr[1] = (Byte)_Word_;
				tok->phonStr[2] = (Byte)opcode;
				vv->pendingSingNote = 0;
				}
			else
				{
				tok->phonStr[0] = 1;
				tok->phonStr[1] = (Byte)opcode;
				}
			/* pendingSingPhoneme stays -1 */
			return;
			}
		}

	/* End of block */
	if (vv->Ch == ']')
		GetNextCh(vv);
	vv->pendingSingPhoneme = 0;
	tok->tokType = kNullTok;
}
