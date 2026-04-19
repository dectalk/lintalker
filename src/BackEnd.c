
#ifndef __SPEECHVERSIONS__
	#include "Versions.h"
#endif

#include <stdio.h>

#ifndef __SPEECHSAY__
	#include "MT4.h"
#endif




/*	Engine.c	*/

extern void	e_SetTempo (voiceVarPtr vv, short tempo);

/*	formantSynth.c	*/
void	synth_Start_Talk (voiceVarPtr vv);



/* Forward declarations	*/

short 	Talk (voiceVarPtr vv);
void	Init_Rate_Params (voiceVarPtr vv);
void	Start_Talk (voiceVarPtr vv);
void	StartNew_PitchClause (voiceVarPtr vv);
void 	SetVolume (voiceVarPtr vv, long vol);
void	ResetVoice (voiceVarPtr vv);
short	NewVoice (voiceVarPtr vv, void *vDat, unsigned char *sample);

void 	e_Fill_Next_Frame (voiceVarPtr vv);
short 	e_HzToPitch (voiceVarPtr vv, short hz);
short	e_MidiToPitch (short midiNote);
short	e_LogToLin (voiceVarPtr vv, short logVal);
short	e_GetPhon (voiceVarPtr vv, short index);
long	e_GetPhonCtrl (voiceVarPtr vv, short index);






short	e_GetPhon (voiceVarPtr vv, short index)
{
	short	ret;
	
	if ( (index >= 0) && (index < vv->phonBuf_2_In_Index) )
		{
		ret = vv->phon_Buf_2[index];
		}
	else
		{
		ret = _SIL_;
		}
	return (ret);
}



long	e_GetPhonCtrl (voiceVarPtr vv, short index)
{
	long	ret;
	
	if ( (index >= 0) && (index < vv->phonBuf_2_In_Index) )
		{
		ret = vv->phon_Ctrl_Buf_2[index];
		}
	else
		{
		ret = 0;
		}
	return (ret);
}







void SetVolume (voiceVarPtr vv, long vol)
{

	if (vol > 0x10000) 
		vv->user_Volume = 0x0100;					/* clip at top			*/
	else if (vol < 0) 
		vv->user_Volume = 0;						/* clip at bottom		*/
	else 
		vv->user_Volume = vol >> 8;					/* xxxx.XXxx -> 00XX	*/
	
	(*(vv->funcList->synth_SetVolume_FUNC)) (vv, vv->user_Volume);
}






short e_HzToPitch (voiceVarPtr vv, short hz)
{
#define ratioK 2621						/* 4096 x (256 � 400)	*/

	long		ratio;
	long		fk,note,freq;



/*----------------------------------*/
/*	Pitch value:					*/
/*		0o ff						*/
/*		o = octave					*/
/*		ff = 256 steps / oct		*/
/*		$0 - $67F = 50 - 4524 hz	*/
/*----------------------------------*/
	note = 0;
	if (hz > 0)
		{		
		if (hz < 100)
			{
			freq = hz << 3;
			fk = 0x0;
			}
		else if (hz < 200)
			{
			freq = hz << 2;
			fk = 0x100;
			}
		else if (hz < 400)
			{
			freq = hz << 1;
			fk = 0x200;
			}
		else if (hz < 800)
			{
			freq = hz;
			fk = 0x300;
			}
		else if (hz < 1600)
			{
			freq = hz >> 1;
			fk = 0x400;
			}
		else if (hz < 3200)
			{
			freq = hz >> 2;
			fk = 0x500;
			}
		else
			{
			freq = hz >> 3;
			fk = 0x600;
			}
		
		ratio = ((freq - 400) * ratioK) >> 11;

		note = vv->logOf2Tbl[ratio] + fk;
		//if (note > 0x67F) note = 0x67F;
		}
		
	return (note);
}



short	e_MidiToPitch (short midiNote)
{
	long	pitch;

	/*----------------------*/
	/* midiNote = xx.xx		*/
	/*----------------------*/
	if (midiNote < kMIDI_50HZ)
		midiNote = 0;
	else
		midiNote -= kMIDI_50HZ;
	pitch = ((midiNote * kOneTwelfth) + kPointFive) >> 16;
	return (pitch);
}


short	PitchToHz (voiceVarPtr vv, short pitch)
{
	short	freq;
	
	freq = ( vv->OctFreqTbl[(pitch & 0xF00) >> 8] * vv->ExpOf2Tbl[pitch & 0xFF] ) >> 15;
	return (freq);
}




short	e_LogToLin (voiceVarPtr vv, short logVal)
{
	short	linVal;

	if (logVal > 63)
		logVal = 63;
	
	linVal = *(vv->logToLinPtr + (logVal >> 1));
	
	return (linVal);
}


short	LogToLog (voiceVarPtr vv, short logVal)
{

	if (logVal > 63)
		logVal = 63;
	
	return (logVal >> 1);
}






void DoNote (voiceVarPtr vv)
{
	short		note;

	if ( ((note = vv->user_Note_Buf2[vv->cur_PhonBuf_Index_CF]) != 0) &&		/* if pitch is entered...	*/
		 !(vv->phon_Ctrl_Buf_2[vv->cur_PhonBuf_Index_CF] & kSilenceDuration) )	/* ...and it's NOT silence embedded cmd */
		{
		if (vv->phon_Ctrl_Buf_2[vv->cur_PhonBuf_Index_CF] & kSingingDuration)
			{
			if (note < 0)
				{
				/* note > 37: raw Hz glide — linearly interpolate from current
				 * pitch to target over the phoneme's duration (DECtalk
				 * PHONE_TARGETS_SPECIFIED behaviour).  portamentoStep != 0
				 * selects the existing linear-ramp path in Interpolate_Pitch. */
				short targetPitch = e_HzToPitch(vv, (short)-note);
				short curPitch    = (short)(vv->portamentoAccum >> 16);
				short frames      = vv->dur_Buf[vv->cur_PhonBuf_Index_CF];
				if (frames < 1) frames = 1;
				vv->VP_baselinePitch = targetPitch;
				vv->portamentoStep   = ((long)(targetPitch - curPitch) << 16) / frames;
				vv->newPortaTarget   = true;
				}
			else
				{
				/* note <= 37: constant musical note — IIR convergence.
				 * portamentoStep = 0 signals IIR mode to Interpolate_Pitch. */
				vv->VP_baselinePitch = note;
				vv->portamentoStep = 0;
				vv->newPortaTarget = true;
				}
			}
		else
			{
			/* EC_note: note low byte is semitone offset above voiceNaturalPitch	*/
			note = (note & 0xFF) << 8;					/* 00XX -> XX00	*/
			if (note != 0x7F00)
				{
				vv->VP_baselinePitch = vv->voiceNaturalPitch + ((note * 0x1555) >> 16);
				if (vv->VP_baselinePitch < 0)
					vv->VP_baselinePitch = 0;
				}
			}
		}
}








void DoCtrl (voiceVarPtr vv)
{
	short				ctrlType;
	long				ctrlData;
	/*long				pitch1, pitch2;	*/
	/*register TMinfoPtr	theTask;	*/

	while (vv->ctrlCount)								/* in case we need to do more than one ctrl	*/
		{
		ctrlType = vv->CMDQueue[vv->cmdBufCount].type;
		ctrlData = vv->CMDQueue[vv->cmdBufCount++].data;
		vv->ctrlCount--;								/* the number of ctrls for this phon	*/
	
		switch (ctrlType)
			{
			case C_absMod:
				if (ctrlData < 0)
					ctrlData = 0;
				else if (ctrlData > (200 << 16))
					ctrlData = (200 << 16);
				vv->VP_pitchRange = ctrlData / 100;
				break;
				
			case C_absPitch:
				ctrlData >>=  8;
				vv->voiceNaturalPitch = e_MidiToPitch (ctrlData);
				vv->VP_baselinePitch = vv->voiceNaturalPitch;
				break;

			case C_relPitch:
				vv->voiceNaturalPitch = e_MidiToPitch (((vv->VP_baselinePitch * 12) + kMIDI_50HZ) + (ctrlData >> 8));
				vv->VP_baselinePitch = vv->voiceNaturalPitch;
				break;

			case C_absVol:
				SetVolume (vv, ctrlData);
				break;

			case C_relVol:
				SetVolume (vv, (vv->user_Volume << 8) + ctrlData);
				break;
				
			case C_reset:
				(*(vv->funcList->e_ResetFE_FUNC)) (vv);
				ResetVoice (vv);
				break;

			case C_voice:
				//NewVoice (vv, &vv->zz->IntervalVoices[ctrlData-1]);
				break;

			default:
				break;
			}
		}

}








void	Store_F0_and_Time (voiceVarPtr vv, short pitch, short time, short flags)
{

/*--------------------------------------------------*/
/* time = time into current phon (# of frames)		*/
/* pitch_Time_Offset = adjustment from last entry	*/
/*--------------------------------------------------*/
	if ( (vv->pitch_Time_Offset + time) >= 0)
		{
		vv->pitch_Buf_Time[vv->pitchBuf_In_Index] = vv->pitch_Time_Offset + time;	/* make time relative from last entry	*/
		vv->pitch_Time_Offset = 0 - time;											/* We're this amount into the phon		*/
		}
	else
		{
		vv->pitch_Buf_Time[vv->pitchBuf_In_Index] = 0;								/* Do it right away (can't go back in time)	*/
		}
	
	vv->pitch_Buf_Freq[vv->pitchBuf_In_Index] = pitch;
	vv->pitch_Buf_Flags[vv->pitchBuf_In_Index] = flags;
	
	if (vv->pitchBuf_In_Index < kPhonBuf_Red_Zone)
		vv->pitchBuf_In_Index++;
}





void	Fill_Pitch_Buf (voiceVarPtr vv)
{
	short		i;
	short		cur_Phon;
	long		cur_Ctrl;
	long		cur_Flags;
	long		cur_Stress;
	short		cur_SyllableType;
	short		cur_Dur;
	short		prev_Phon;
	long		prev_Ctrl;
	short		next_Phon;
	long		next_Flags;
	short		pitchIsFallen;
	short		timeT, pitchT;
	short		stress_Counter;
	short		raiseAmt, fallAmt, curBaseline;
	short		raiseAmt1, fallAmt1;
	short		lastStress;

#define kStressUp 0
#define kStressDn 1
	
	
	pitchIsFallen = true;
	vv->pitchBuf_In_Index = 0;
	stress_Counter = 0;
	curBaseline = 0;
	vv->pitch_Time_Offset = 0;
	raiseAmt = 0;
	fallAmt = 0;
	raiseAmt1 = 0;
	fallAmt1 = 0;
	lastStress = kStressDn;
	
	for (i = 0; i < vv->phonBuf_2_In_Index; i++)
		{
		/*----------*/
		/* cur		*/
		/*----------*/
		cur_Phon = e_GetPhon (vv, i);
		cur_Ctrl = e_GetPhonCtrl (vv, i);
		cur_Flags = vv->phonFlags2[cur_Phon];
		cur_Stress = cur_Ctrl & kStressField;
		cur_SyllableType = cur_Ctrl & kSyllableTypeField;
		cur_Dur = vv->dur_Buf[i];

		/*----------*/
		/* prev		*/
		/*----------*/
		prev_Phon = e_GetPhon (vv, i-1);
		prev_Ctrl = e_GetPhonCtrl (vv, i-1);

		/*----------*/
		/* next		*/
		/*----------*/
		next_Phon = e_GetPhon (vv, i+1);
		next_Flags = vv->phonFlags2[next_Phon];
		
		
		if (cur_Flags & kVowelF)
			{
			/********************/
			/* PITCH RISE		*/
			/********************/
			if ((cur_Ctrl & kPitchRise) && pitchIsFallen)
				{
				raiseAmt = vv->VP_riseAmt;
				
				if (vv->end_Punctuation == _Quest_)
					raiseAmt = raiseAmt >> 1;				/* less intonation in question clause (50%)	*/
								
				if (cur_Ctrl & kPitchFall)					/* if Rise/Fall...	*/
					timeT = (-80) / kFrameTime;				/* ...start rise earlier	*/
				else
					timeT = 0;								/* else start RISE now	*/
				
				Store_F0_and_Time (vv, raiseAmt, timeT, kPitchRiseFall_Flg);
				
				curBaseline += raiseAmt;
				pitchIsFallen = false;						/* pitch in raised state	*/
				}
			
			/************************/
			/* PITCH RISE1 FALL1	*/
			/************************/
			if (cur_Ctrl & kPitchRise1)
				{
				raiseAmt1 = vv->VP_riseAmt1;
				
				if (vv->end_Punctuation == _Quest_)
					raiseAmt1 >>=  1;					/* less intonation in question clause (50%)	*/
								
				
				Store_F0_and_Time (vv, raiseAmt1, 0, kPitchRiseFall1_Flg);
				
				//curBaseline += raiseAmt1;
				}

			else if (cur_Ctrl & kPitchFall1)
				{
				fallAmt1 = vv->VP_fallAmt1;
				
				Store_F0_and_Time (vv, fallAmt1, 0, kPitchRiseFall1_Flg);
				//curBaseline += fallAmt1;
				}


			/********************/
			/* PRIMARY STRESS	*/
			/********************/
			if (cur_Stress & kPrimOrEmphStress)
				{
				if (cur_Stress == kEmphaticStress)
					{
					pitchT = kHZ_28; 					/* emphatic stress = 28 hz	*/
					}
				else
					pitchT = kHZ_14;					/* primary stress = 7 hz	*/
					
				switch (stress_Counter)
					{
					case 0: 	pitchT += kHZ_10; 	break;		/*  1st stress = 20 hz	*/
					case 1: 	pitchT += kHZ_9; 	break;		/*  2nd stress = 9 hz	*/
					case 2: 	pitchT += kHZ_6; 	break;		/*  3rd stress = 6 hz	*/
					case 3: 	pitchT += kHZ_4; 	break;		/*  4th stress = 4 hz	*/
					}
					
				if (vv->end_Punctuation == _Quest_)
					pitchT = pitchT >> 1;				/* less stress for question clause	*/
				
				if ( (cur_Ctrl & kPitchFall) || (cur_SyllableType & kTerm_End) )
					{
					timeT = (-60) / kFrameTime;			/* early start for last syllable	*/
					}
				
				else if (cur_Stress == kEmphaticStress)
					timeT = 0;							/* emph stress starts at phon start	*/
				
				else
					timeT = cur_Dur >> 2;				/* prim stress starts 25% past phon start	*/


				//if (cur_Ctrl & kPitchFall1)
				//	pitchT = 0 - pitchT;
				/*----------------------------------*/
				/* scale to STRESS GAIN				*/
				/*----------------------------------*/
				pitchT = (vv->VP_stressGain * pitchT) >> 16;
				
			#if 0
				if (lastStress == kStressUp)
					{
					lastStress = kStressDn;
					pitchT = 0 - pitchT;
					}
				else
					lastStress = kStressUp;
			#endif
			
				if ( (cur_SyllableType & kTerm_End) && (cur_Stress != kEmphaticStress) )
					{
					pitchT = 0 - kHZ_4;
					//pitchT = 0;
					}
				
				Store_F0_and_Time (vv, pitchT, timeT, kPitchStress_Flg);
				
				stress_Counter++;
				}
				
			/********************/
			/* PITCH FALL		*/
			/********************/
			//if ( (cur_Ctrl & kPitchFall) && (!pitchIsFallen) )
			if ( cur_Ctrl & kPitchFall )
				{
			//#if 0
				timeT = cur_Dur - (160 / kFrameTime);
				
				if ( timeT < (25 / kFrameTime) )
					timeT = 25 / kFrameTime;
			//#endif
				
				//timeT = 25 / kFrameTime;					/* @@@@@	*/
				//timeT = cur_Dur >> 1;
				//timeT = 0;

				if (cur_SyllableType & kTerm_End)
					{
					switch (vv->end_Punctuation)
						{
						case _Comma_:	fallAmt = 0 - kHZ_12;	break;
						case _Period_:	fallAmt = 0 - kHZ_20; 	break;
						case _Quest_:	fallAmt = 0 - kHZ_7; 	break;
						case _Exclam_:	fallAmt = 0 - kHZ_20; 	break;
						}
					}

				else if (cur_SyllableType & kVerb_End)
					{
					fallAmt = 0;
					}

				else 
					fallAmt = vv->VP_fallAmt;
					
				/*----------------------------------*/
				/* scale to ASSERTIVENESS			*/
				/* remove RAISE bias				*/
				/*----------------------------------*/
				fallAmt = ((vv->VP_assertiveness * fallAmt) >> 16) - raiseAmt;
				
				Store_F0_and_Time (vv, fallAmt, timeT, kPitchRiseFall_Flg);
				curBaseline += fallAmt;
				pitchIsFallen = true;					/* pitch in raised state	*/
				}

			/************************/
			/* RAISE TYPE BOUNDRY	*/
			/*   comma or quest		*/
			/************************/
			if ( (cur_SyllableType & kTerm_End) && 
				( (vv->end_Punctuation == _Comma_) || (vv->end_Punctuation == _Quest_) ) )
				{
				//timeT = cur_Dur - (50 / kFrameTime);
				timeT = 0;
				
				if (vv->end_Punctuation == _Quest_)
					{
					Store_F0_and_Time (vv, kHZ_18, timeT, kPitchBoundry_Flg);			/* 1st ramp	*/
					Store_F0_and_Time (vv, kHZ_25, cur_Dur, kPitchBoundry_Flg);			/* 2nd ramp	*/
					}
				else
					{
					Store_F0_and_Time (vv, kHZ_7, timeT, kPitchBoundry_Flg);			/* 1st ramp	*/
					Store_F0_and_Time (vv, kHZ_10, cur_Dur, kPitchBoundry_Flg);			/* 2nd ramp	*/
					}
				}


			#if 0
			/************************************/
			/* Unstressed term VOWEL			*/
			/************************************/
			if ( 	(cur_SyllableType & kTerm_End) && 
					(!(cur_Stress & kPrimOrEmphStress) || !(cur_Ctrl & kPitchFall)) )
				{
				if ( (vv->end_Punctuation == _Period_) || (vv->end_Punctuation == _Exclam_) )
					{
					pitchT = (vv->VP_assertiveness * (0 - kHZ_8)) >> 16;
					Store_F0_and_Time (vv, pitchT, cur_Dur >> 2, kPitchBoundry_Flg);
					}
				else
					{
					timeT = cur_Dur - (80 / kFrameTime);
		
					if (vv->end_Punctuation == _Quest_)
						{
						Store_F0_and_Time (vv, kHZ_18, timeT, kPitchBoundry_Flg);
						Store_F0_and_Time (vv, kHZ_25, cur_Dur, kPitchBoundry_Flg);
						}
		
					if (vv->end_Punctuation == _Comma_)
						{
						Store_F0_and_Time (vv, kHZ_7, timeT + (20 / kFrameTime), kPitchBoundry_Flg);
						Store_F0_and_Time (vv, kHZ_10, cur_Dur, kPitchBoundry_Flg);
						}
					}
				}
			#endif
				
			}		/* end of cur = VOWEL	*/
		
		
		if ( (prev_Ctrl & kSilenceTypeField) >> kSilenceTypeShift )
			{
			Store_F0_and_Time (vv, 0 - curBaseline, 0, kPhraseReset);
			//Store_F0_and_Time (vv, 0 - curBaseline, 0, kPitchBoundry_Flg);
			curBaseline = 0;
			}
		
		#if 0
		if (cur_Phon == _SIL_)
			{
			//if (curBaseline != 0)
				{
				/*----------------------------------*/
				/* Get pitch back to zero in SIL	*/
				/*----------------------------------*/
				//Store_F0_and_Time (vv, 0 - curBaseline, 0, 0);
				//Store_F0_and_Time (vv, 0 - curBaseline, 0, kPhraseReset);
				//curBaseline = 0;
				}
			
			if (i > 0)
				stress_Counter = 1;								/* avoid BIG 1st stress	*/
			
			if (prev_Ctrl & kTerm_End)
				{
				Store_F0_and_Time (vv, 0, 0, kResetDecline);	/* reset the decline ramp	*/
				curBaseline = 0;
				stress_Counter = 0;
				}
			}
		#endif
		
		vv->pitch_Time_Offset += cur_Dur;
		}
}



void	Calc_Ramp_Steps (voiceVarPtr vv)
{
	short		i;
	long		cur_Ctrl;
	short		cur_SyllableType;
	short		cur_Dur;
	short		accum;
	short		mode, rampIndex;

#define kRampMode	0
#define kSusMode	1


	rampIndex = 0;
	mode = kRampMode;
	accum = 1;
	
	for (i = 0; i < vv->phonBuf_2_In_Index; i++)
		{
		/*----------*/
		/* cur		*/
		/*----------*/
		cur_Ctrl = e_GetPhonCtrl (vv, i);
		cur_SyllableType = cur_Ctrl & kSyllableTypeField;
		cur_Dur = vv->dur_Buf[i];
	
		if (mode == kRampMode)
			{
			if ( (cur_Ctrl & kSilenceTypeField) || (cur_SyllableType & kTerm_End) )
				{
				vv->rampSteps[rampIndex] = (vv->baselineFall_START - vv->baselineFall_END << 16) / accum;
				if (cur_Ctrl & kSilenceTypeField)
					{
					//vv->rampSteps[rampIndex] >>= 1;
					}
				else
					{
					if ( (vv->end_Punctuation == _Comma_) || (vv->end_Punctuation == _Quest_) )
						vv->rampSteps[rampIndex] >>= 1;
					}
				if (rampIndex < kMaxRamps)
					rampIndex++;
				//mode = kSusMode;
				accum = 1;
				}
			else
				accum += cur_Dur;
			}
		else
			{
			if (cur_Ctrl & kSilenceTypeField)
				{
				mode = kRampMode;
				accum = 1;
				i++;					// skip over silence phon
				}
			}
		}
	
	vv->curRamp = 0;
	vv->down_Ramp_Step = vv->rampSteps[0];
}







void	StartNewPhon (voiceVarPtr vv)
{

	if ( (e_GetPhonCtrl (vv, vv->cur_PhonBuf_Index_CF)) & kWord_Start )
		{
		vv->nLastWordStart = vv->lastWordStart;
		vv->lastWordStart = vv->cur_PhonBuf_Index_CF;					/* remember last word start position - for Continue	*/

		vv->pFilter_Out1_Save2 = vv->pFilter_Out1_Save1;
		vv->pFilter_Out2_Save2 = vv->pFilter_Out2_Save1;
		vv->down_Ramp_Offset_Save2 = vv->down_Ramp_Offset_Save1;
		vv->fallRise_Offset_Save2 = vv->fallRise_Offset_Save1;
		vv->fallRise1_Offset_Save2 = vv->fallRise1_Offset_Save1;
		vv->stress_Target_Save2 = vv->stress_Target_Save1;
		vv->punct_Offset_Save2 = vv->punct_Offset_Save1;

		vv->next_PitchBuf_Time_Save2 = vv->next_PitchBuf_Time_Save1;	
		vv->phon_Index_Targ_Save2 = vv->phon_Index_Targ_Save1;
		vv->phon_Index_CP_Save2 = vv->phon_Index_CP_Save1;
		vv->pitchBuf_Out_Index_Save2 = vv->pitchBuf_Out_Index_Save1;
		vv->time_IntoPhon_CP_Save2 = vv->time_IntoPhon_CP_Save1;
		vv->cur_Phon_Dur_CC_Save2 = vv->cur_Phon_Dur_CC_Save1;
		vv->cur_PhonDur_CP_Save2 = vv->cur_PhonDur_CP_Save1;
		vv->time_IntoPhon_Targ_Save2 = vv->time_IntoPhon_Targ_Save1;
		vv->cur_PitchBuf_Time_Save2 = vv->cur_PitchBuf_Time_Save1;
		vv->cmdBufCount_Save2 = vv->cmdBufCount_Save1;
		vv->next_PitchBuf_Time_Save1 = vv->next_PitchBuf_Time;
		vv->phon_Index_Targ_Save1 = vv->phon_Index_Targ;
		vv->phon_Index_CP_Save1 = vv->phon_Index_CP;
		vv->pitchBuf_Out_Index_Save1 = vv->pitchBuf_Out_Index;
		vv->time_IntoPhon_CP_Save1 = vv->time_IntoPhon_CP;
		vv->cur_Phon_Dur_CC_Save1 = vv->cur_Phon_Dur_CC;
		vv->cur_PhonDur_CP_Save1 = vv->cur_PhonDur_CP;
		vv->time_IntoPhon_Targ_Save1 = vv->time_IntoPhon_Targ;
		vv->cur_PitchBuf_Time_Save1 = vv->cur_PitchBuf_Time;

		vv->pFilter_Out1_Save1 = vv->pFilter_Out1;
		vv->pFilter_Out2_Save1 = vv->pFilter_Out2;
		vv->down_Ramp_Offset_Save1 = vv->down_Ramp_Offset;
		vv->fallRise_Offset_Save1 = vv->fallRise_Offset;
		vv->fallRise1_Offset_Save1 = vv->fallRise1_Offset;
		vv->stress_Target_Save1 = vv->stress_Target;
		vv->punct_Offset_Save1 = vv->punct_Offset;
		vv->cmdBufCount_Save1 = vv->cmdBufCount;
		vv->VP_baselinePitch_Save1 = vv->VP_baselinePitch;
		}
		
	if ( (vv->ctrlCount = vv->user_Cmd_Buf2[vv->cur_PhonBuf_Index_CF]) > 0)
		DoCtrl (vv);

	if (vv->sync_On_Marker &&
	    !(vv->phon_Ctrl_Buf_2[vv->cur_PhonBuf_Index_CF] & kSingingDuration))
		{
		if (vv->phon_Ctrl_Buf_2[vv->cur_PhonBuf_Index_CF] & kSampleMarker)
			{
			vv->frameMarker = vv->markerBuf[vv->markerIndex++];
			if (vv->markerIndex == vv->lastMarkerIndex)
				vv->markerIndex = 0;
			}
		}

	else
		DoNote (vv);
	

	vv->dur_Done_in_Phon_CF = 0;
	vv->cur_Phon_Dur_CF = vv->dur_Buf[vv->cur_PhonBuf_Index_CF];
	
	if (vv->cur_PhonBuf_Index_CF == 0)
		/*--------------------------*/
		/* Start of new sentence	*/
		/*--------------------------*/
		{
		vv->prev_Phon_CF = _SIL_;
		vv->prev_PhonCtrl_CF = 0;
		vv->prev2_Phon_CF = _SIL_;
		vv->prev2_PhonCtrl_CF = 0;
		}
	else
		{
		/*--------------------------*/
		/* prev2					*/
		/*--------------------------*/
		vv->prev2_Phon_CF = vv->prev_Phon_CF;
		vv->prev2_PhonCtrl_CF = vv->prev_PhonCtrl_CF;
		/*--------------------------*/
		/* prev						*/
		/*--------------------------*/
		vv->prev_Phon_CF = vv->cur_Phon_CF;
		vv->prev_PhonCtrl_CF = vv->cur_PhonCtrl_CF;
		}
		
	vv->prev_PhonFlags_CF = vv->phonFlags2[vv->prev_Phon_CF];

	/*--------------------------*/
	/* cur						*/
	/*--------------------------*/
	vv->cur_Phon_CF = e_GetPhon (vv, vv->cur_PhonBuf_Index_CF);
	vv->cur_PhonCtrl_CF = e_GetPhonCtrl (vv, vv->cur_PhonBuf_Index_CF);
	vv->cur_PhonFlags_CF = vv->phonFlags2[vv->cur_Phon_CF];

	/*--------------------------*/
	/* next						*/
	/*--------------------------*/
	vv->next_Phon_CF = e_GetPhon (vv, vv->cur_PhonBuf_Index_CF +1);
	vv->next_PhonCtrl_CF = e_GetPhonCtrl (vv, vv->cur_PhonBuf_Index_CF +1);
	vv->next_PhonFlags_CF = vv->phonFlags2[vv->next_Phon_CF];
}









void	Phon_Boundry_Pitch (voiceVarPtr vv)
{

	short		cur_Phon, next_Phon;
	long		cur_Flags, next_Flags;
	long		cur_Ctrl, next_Ctrl;
	
	if (vv->time_IntoPhon_CP >= vv->cur_PhonDur_CP)
		{
		vv->time_IntoPhon_CP -= vv->cur_PhonDur_CP;
		vv->phon_Index_CP++;
		vv->cur_PhonDur_CP = vv->dur_Buf[vv->phon_Index_CP];
		
		cur_Phon = e_GetPhon (vv, vv->phon_Index_CP);
		cur_Flags = vv->phonFlags2[cur_Phon];
		cur_Ctrl = e_GetPhonCtrl (vv, vv->phon_Index_CP+1);;

		next_Phon = e_GetPhon (vv, vv->phon_Index_CP+1);
		next_Flags = vv->phonFlags2[next_Phon];
		next_Ctrl = e_GetPhonCtrl (vv, vv->phon_Index_CP+1);;
		
		if (vv->pitch_Boundry == 0)
			{
			vv->pitch_Boundry = kNeverHappens;
			}
			
		if (vv->pitch_Boundry > 0)
			vv->pitch_Boundry = 0;			/* ramp back up	*/
		
		vv->pbHold = kNeverHappens;
		vv->pbLowGain = false;
		
		if (	(cur_Flags & kVowel1F) &&
				!(next_Ctrl & kMid_Syllable_In_Word)	&&		/* next is NOT mid or last vowel in word	*/
				( (cur_Ctrl & kSyllableTypeField) >= kWord_End) &&
				(next_Phon != _YU_) )
			{
			if (cur_Flags & kVowelF)
				/*--------------------------*/
				/* cur = vowel				*/
				/*--------------------------*/
				{
				if ( (cur_Phon == next_Phon) && (next_Ctrl & kPrimOrEmphStress) ) 
					{
					vv->pbHold = vv->cur_PhonDur_CP;
					}
				else if ( (cur_Ctrl & kSyllableTypeField) >= kPrep_End )
					{
					vv->pbHold = vv->cur_PhonDur_CP;
					vv->pbLowGain = true;
					}
				}
			else
				/*--------------------------*/
				/* cur � vowel				*/
				/*--------------------------*/
				{
				if (	!(cur_Flags & kStopF) &&
						(cur_Phon != _DX_) &&
						(next_Ctrl & kPrimOrEmphStress) )
					{
					vv->pbHold = vv->cur_PhonDur_CP;
					}
				}
			}
		
		if (next_Flags & kGStopF)
			vv->pbHold = vv->cur_PhonDur_CP;
			
		if (cur_Flags & kGStopF)
			{
			vv->pbHold = vv->cur_PhonDur_CP;
			goto Exit;
			}

		}
	if (	(vv->time_IntoPhon_CP == 50 / kFrameTime) ||
			(vv->time_IntoPhon_CP == vv->cur_PhonDur_CP -1) )
		{
		vv->pitch_Boundry = vv->pbHold;
		vv->low_Gain_CP = vv->pbLowGain;
		}

Exit:
	return;
}













void	Interpolate_Pitch (voiceVarPtr vv)
{
	short		collect;
	short		cur_Phon, next_Phon;
	long		cur_Ctrl;
	short		cur_Flags, next_Flags;
	short		pbIndex;
	short		vibrato;
	short		phon_Pitch_Target;


	collect = true;
	do
		{
		if (	(vv->cur_PitchBuf_Time >= vv->next_PitchBuf_Time) &&
				(vv->pitchBuf_Out_Index < vv->pitchBuf_In_Index) )
			{
			vv->cur_PitchBuf_Pitch = vv->pitch_Buf_Freq[vv->pitchBuf_Out_Index];
			vv->cur_PitchBuf_Flags = vv->pitch_Buf_Flags[vv->pitchBuf_Out_Index];
			
			vv->cur_PitchBuf_Time -= vv->next_PitchBuf_Time;
			vv->pitchBuf_Out_Index++;
			
			vv->next_PitchBuf_Time = vv->pitch_Buf_Time[vv->pitchBuf_Out_Index];
			
			if (vv->cur_PitchBuf_Flags & kResetDecline)		/* reset the decline ramp	*/
				vv->down_Ramp_Offset = 0;
				
			else if (vv->cur_PitchBuf_Flags & kPhraseReset)		/* reset the decline ramp	*/
				{
				vv->down_Ramp_Offset = (vv->baselineFall_START - vv->baselineFall_END) << 14;

				if (vv->curRamp < kMaxRamps)
					vv->curRamp++;
				vv->down_Ramp_Step = vv->rampSteps[vv->curRamp];
				
				#if 0
				if (vv->baseLine_Offset <= vv->baseline_End_Offset)
					{
					//vv->down_Ramp_Offset = 0;
					vv->down_Ramp_Offset = vv->VP_baselineFall << 15;
					}
				#endif
					
				//vv->down_Ramp_Offset = vv->down_Ramp_Offset >> 1;
				//vv->fallRise_Offset = 0;
				//vv->fallRise_Offset = vv->fallRise_Offset >> 1;
				
				}
				
			else if (vv->cur_PitchBuf_Flags & kPitchRiseFall_Flg)
				{
				vv->fallRise_Offset += vv->cur_PitchBuf_Pitch;
				if (vv->cur_PitchBuf_Pitch < 0)
					/*--------------------------*/
					/* Pitch Fall				*/
					/*--------------------------*/
					{
					if (vv->stress_Target > 0)
						vv->stress_Target = 0;
					
					//if (vv->cur_PitchBuf_Pitch != vv->VP_fallAmt1)
					//	vv->down_Ramp_Offset = (vv->baseline_Start_Offset - vv->baseline_End_Offset) << 16;
					}
				else
					/*--------------------------*/
					/* Pitch Rise				*/
					/*--------------------------*/
					{
					if (vv->stress_Target < 0)
						vv->stress_Target = 0;
					}
				}
			else if (vv->cur_PitchBuf_Flags & kPitchRiseFall1_Flg)
				/*--------------------------*/
				/* Rise / Fall 1			*/
				/*--------------------------*/
				{
				vv->fallRise1_Offset += vv->cur_PitchBuf_Pitch;
				}
			else if (vv->cur_PitchBuf_Flags & kPitchStress_Flg)
				/*--------------------------*/
				/* Stress Ramp				*/
				/*--------------------------*/
				{
				vv->stress_Target = vv->cur_PitchBuf_Pitch;
				vv->stress_Active_Time = vv->stress_Duration;
				}
			else
				{
				vv->punct_Offset = vv->cur_PitchBuf_Pitch << 1;
				
				//vv->stress_Target = vv->cur_PitchBuf_Pitch;				/* @@@@ x 2	*/
				//vv->stress_Active_Time = vv->stress_Duration;
				}
			}
		else
			collect = false;
		
		}
	while (collect);
	

		
	if (!vv->singing)
		{
		/*--------------------------*/
		/* Down Ramp				*/
		/*--------------------------*/
		vv->baseLine_Offset = vv->baseline_Start_Offset - (vv->down_Ramp_Offset >> 16) + vv->user_Pitch_Buf2[vv->phon_Index_Targ];
		//vv->baseLine_Offset = vv->baseline_Start_Offset - (vv->down_Ramp_Offset >> 16);
		
		if (vv->baseLine_Offset > vv->baseline_End_Offset)
			vv->down_Ramp_Offset += vv->down_Ramp_Step;
		
		/*--------------------------*/
		/* Stress Active			*/
		/*--------------------------*/
		vv->stress_Active_Time--;
		if (vv->stress_Active_Time < 0)
			vv->stress_Target = 0;
		
	
	
		if ( 	(vv->time_IntoPhon_Targ > (vv->cur_Phon_Dur_CC + vv->phon_Dur_Delay)) &&
				(vv->phon_Index_Targ < vv->phonBuf_2_In_Index) )
			/*--------------------------*/
			/* Pitch for phon			*/
			/*--------------------------*/
			{
			vv->time_IntoPhon_Targ -= vv->cur_Phon_Dur_CC;
			
			vv->phon_Index_Targ++;
			vv->cur_Phon_Dur_CC = vv->dur_Buf[vv->phon_Index_Targ];
			vv->phon_Dur_Delay = 0;
	
			cur_Phon = e_GetPhon (vv, vv->phon_Index_Targ);
			cur_Ctrl = e_GetPhonCtrl (vv, vv->phon_Index_Targ);
			cur_Flags = vv->phonFlags2[cur_Phon];
			next_Phon = e_GetPhon (vv, vv->phon_Index_Targ+1);
			next_Flags = vv->phonFlags2[next_Phon];
	
			vv->phon_Pitch_Offset = vv->phonPitchTbl[cur_Phon];
			
			//if ( !(cur_Ctrl & kStressField) )		/* @@@@ NOTE 50% reduction always */
				vv->phon_Pitch_Offset >>= 1;			/* 50% if NOT stressed	*/
			
			if ( !(next_Flags & kVoicedF) )
				vv->phon_Dur_Delay = 25 / kFrameTime;		/* if next is NOT voiced	*/
			
			if (cur_Flags & kVoicedF)
				/*--------------------------*/
				/* cur is voiced			*/
				/*--------------------------*/
				{
				vv->phon_Pitch_Offset_1 = vv->phon_Pitch_Offset << 1;
				vv->uvPhon_Pitch_Targ = 0;
				}
			else
				/*--------------------------*/
				/* cur is unvoiced			*/
				/*--------------------------*/
				{
				vv->uvPhon_Pitch_Targ = vv->phon_Pitch_Offset << kStepSizeRes;
				vv->phon_Pitch_Offset_1 = 0;
				
				if (cur_Flags & kStopF)
					vv->phon_Dur_Delay = 30 / kFrameTime;
				else
					vv->phon_Dur_Delay = 0;
				}
			}
	
	//vv->uvPhon_Pitch_Targ = 0;				// +++++
			
		Phon_Boundry_Pitch (vv);
	//#if 0
		phon_Pitch_Target = ((vv->stress_Target +
							vv->fallRise_Offset +
							//vv->fallRise1_Offset +
							vv->punct_Offset +
							vv->baseLine_Offset) * vv->VP_intonation) >> 16;
	
		phon_Pitch_Target = (phon_Pitch_Target + vv->phon_Pitch_Offset_1) << kStepSizeRes;
		
		
	//#endif
	
		//phon_Pitch_Target = (vv->fallRise_Offset) << kStepSizeRes;				// +++++
		//phon_Pitch_Target = 0;				// +++++
		
		/*--------------------------*/
		/* Filter to target			*/
		/*--------------------------*/
		if (vv->newSentence)
			{
			vv->pFilter_Out1 = vv->pFilter_Out2 = vv->VP_baselinePitch;			/* start filter storage at 1st pitch	*/
			vv->newSentence = false;
			}


	//vv->pFilter_In_Gain = (50 * 48) + 4800;
	//vv->pFilter_FB_Gain = k100percent - vv->pFilter_In_Gain;

		vv->pFilter_Out1 = 	((vv->pFilter_In_Gain * phon_Pitch_Target) +
							(vv->pFilter_FB_Gain * vv->pFilter_Out1)) >> 16;
	
		vv->pFilter_Out2 =	((vv->pFilter_In_Gain * (vv->pFilter_Out1 + vv->uvPhon_Pitch_Targ)) +
							(vv->pFilter_FB_Gain * vv->pFilter_Out2)) >> 16;
	
		vv->basePitch_Offset = vv->pFilter_Out2 >> kStepSizeRes;
		//vv->basePitch_Offset = (vv->pFilter_Out2 + (vv->phon_Pitch_Offset_1 << 0)) >> kStepSizeRes;
		//vv->basePitch_Offset = phon_Pitch_Target >> kStepSizeRes;
	
	
	//#if 0
		/*----------------------------------*/
		/* Phon pitch decay back to base	*/
		/*----------------------------------*/
		vv->phon_Pitch_Offset_1 = (vv->phon_Pitch_Offset_1 * 98 * pct) >> 16;
	
		/*------------------------------*/
		/* phon boundry envelope		*/
		/*------------------------------*/
		pbIndex = vv->time_IntoPhon_CP - vv->pitch_Boundry;
		if (pbIndex < 0)
			pbIndex = 0 - pbIndex;			/* ABS	*/
		
		if (pbIndex <= (45 / kFrameTime))
			{
			if (vv->low_Gain_CP)
				vv->basePitch_Offset += (pbIndex * (10 / (45 / kFrameTime))) - 10;
			else
				vv->basePitch_Offset += (pbIndex * (80 / (45 / kFrameTime))) - 80;
			}
	//#endif
	
		vv->controlF0 = ((vv->basePitch_Offset * vv->VP_pitchRange) >> 16) + vv->VP_baselinePitch;


		#if 0
		vv->vibrato_Phase1 = (kVibFreq1 + vv->vibrato_Phase1) & 0xFFFFFF;
		vibrato = ((unsigned char)(*(vv->SineWavePtr + (vv->vibrato_Phase1 >> 16)))) - 128;
		vv->vibrato_Phase2 = (kVibFreq2 + vv->vibrato_Phase2) & 0xFFFFFF;
		vibrato += ((unsigned char)(*(vv->SineWavePtr + (vv->vibrato_Phase2 >> 16)))) - 128;
		vibrato >>= 4;						/* �3 hz	*/
		vv->controlF0 += vibrato;				// +++++
		#endif

		vv->vibrato_Phase1 = (vv->vibratoFreq + vv->vibrato_Phase1) & 0xFFFFFF;
		vibrato = ((unsigned char)(*(vv->SineWavePtr + (vv->vibrato_Phase1 >> 16)))) - 128;
		if ( vv->speech_Rate >= 100)
			vv->controlF0 += ((vibrato * vv->vibratoDepth1) >> 16);
		else
			vv->controlF0 += ((vibrato * vv->vibratoDepth2) >> 16);
		}
	else
		{
		if (vv->newSentence)
			{
			vv->portamentoAccum = vv->VP_baselinePitch << 16;
			vv->newSentence = false;
			vv->newPortaTarget = false;
			}

		else if (vv->newPortaTarget)
			{
			if (vv->portamentoStep > 0)
				{
				vv->portamentoAccum += vv->portamentoStep;
				if ( (vv->portamentoAccum >> 16) >=  vv->VP_baselinePitch )
					{
					vv->portamentoAccum = vv->VP_baselinePitch << 16;
					vv->newPortaTarget = false;
					}
				}
			else if (vv->portamentoStep < 0)
				{
				vv->portamentoAccum += vv->portamentoStep;
				if ( (vv->portamentoAccum >> 16) <  vv->VP_baselinePitch)
					{
					vv->portamentoAccum = vv->VP_baselinePitch << 16;
					vv->newPortaTarget = false;
					}
				}
			else if (vv->singing)
				{
				/* IIR convergence for EC_sing: f0 += (target - f0) >> 2        */
				/* Matches DECtalk set_user_target. Converges within ~1 unit in  */
				/* ~16 frames for large intervals; near-instant for small ones.  */
				long target = (long)vv->VP_baselinePitch << 16;
				long diff   = target - vv->portamentoAccum;
				vv->portamentoAccum += diff >> 2;
				if (diff > -0x10000L && diff < 0x10000L)
					{
					vv->portamentoAccum = target;
					vv->newPortaTarget  = false;
					}
				}
			else
				{
				vv->portamentoAccum = vv->VP_baselinePitch << 16;
				vv->newPortaTarget = false;
				}
			}
		
		vv->controlF0 = vv->portamentoAccum >> 16;

		vv->vibrato_Phase1 = (vv->vibratoFreq + vv->vibrato_Phase1) & 0xFFFFFF;
		vibrato = ((unsigned char)(*(vv->SineWavePtr + (vv->vibrato_Phase1 >> 16)))) - 128;
		if (vv->cur_PhonCtrl_CF & kLowVibrato)
			vv->controlF0 += ((vibrato * vv->vibratoDepth2) >> 16);
		else
			vv->controlF0 += ((vibrato * vv->vibratoDepth1) >> 16);
		}

	if (vv->controlF0 < 0)
		vv->controlF0 = 0;
	
	vv->cur_PitchBuf_Time++;
	vv->time_IntoPhon_Targ++;
	vv->time_IntoPhon_CP++;
}






void	StartNew_PitchClause (voiceVarPtr vv)
{

	vv->baseline_Start_Offset = vv->baselineFall_START;
	vv->baseline_End_Offset = vv->baselineFall_END;

	vv->down_Ramp_Offset = 0;
	vv->down_Ramp_Offset_Save1 = 0;
	vv->down_Ramp_Offset_Save2 = 0;
	
	if (vv->start_of_Paragraph_Flag)
		{
		vv->baseline_Start_Offset += kHZ_12;
		vv->baseline_End_Offset += kHZ_7;
		vv->start_of_Paragraph_Flag = false;
		}
		
	vv->next_PitchBuf_Time = vv->pitch_Buf_Time[0];	
	vv->phon_Index_Targ = -1;
	vv->phon_Index_CP = -1;
	vv->pitchBuf_Out_Index = 0;
	vv->time_IntoPhon_CP = 0;
	vv->cur_Phon_Dur_CC = 0;
	vv->cur_PhonDur_CP = 0;

	vv->next_PitchBuf_Time_Save1 = vv->next_PitchBuf_Time;	
	vv->phon_Index_Targ_Save1 = vv->phon_Index_Targ;
	vv->phon_Index_CP_Save1 = vv->phon_Index_CP;
	vv->pitchBuf_Out_Index_Save1 = 0;
	vv->time_IntoPhon_CP_Save1 = 0;
	vv->cur_Phon_Dur_CC_Save1 = 0;
	vv->cur_PhonDur_CP_Save1 = 0;
	
	vv->next_PitchBuf_Time_Save2 = vv->next_PitchBuf_Time_Save1;	
	vv->phon_Index_Targ_Save2 = vv->phon_Index_Targ_Save1;
	vv->phon_Index_CP_Save2 = vv->phon_Index_CP_Save1;
	vv->pitchBuf_Out_Index_Save2 = 0;
	vv->time_IntoPhon_CP_Save2 = 0;
	vv->cur_Phon_Dur_CC_Save2 = 0;
	vv->cur_PhonDur_CP_Save2 = 0;
		
	vv->phon_Dur_Delay = 0;
	vv->uvPhon_Pitch_Targ = 0;
	vv->phon_Pitch_Offset_1 = 0;

	vv->fallRise_Offset = 0;
	vv->fallRise1_Offset = 0;
	vv->fallRise_Offset_Save1 = 0;
	vv->fallRise_Offset_Save2 = 0;
	vv->fallRise1_Offset_Save1 = 0;
	vv->fallRise1_Offset_Save2 = 0;
	vv->stress_Target = 0;
	vv->stress_Target_Save1 = 0;
	vv->stress_Target_Save2 = 0;
	vv->punct_Offset = 0;
	vv->punct_Offset_Save1 = 0;
	vv->punct_Offset_Save2 = 0;

	vv->VP_baselinePitch_Save1 = vv->VP_baselinePitch;
	vv->VP_baselinePitch_Save2 = vv->VP_baselinePitch;
	
	vv->time_IntoPhon_Targ = vv->pitch_Clause_StartTime;
	vv->cur_PitchBuf_Time = vv->time_IntoPhon_Targ >> 1;

	vv->time_IntoPhon_Targ_Save1 = vv->time_IntoPhon_Targ;
	vv->cur_PitchBuf_Time_Save1 = vv->cur_PitchBuf_Time;

	vv->time_IntoPhon_Targ_Save2 = vv->time_IntoPhon_Targ_Save1;
	vv->cur_PitchBuf_Time_Save2 = vv->cur_PitchBuf_Time_Save1;
}








void	Mod_Duration (voiceVarPtr vv)
{
#define k100pct_Dur 128

	short	cur_Phon;
	long	cur_PhonCtrl;
	long	cur_PhonFlags;
	long	cur_SyllableType;
	short	cur_VowelFlag;
	long	cur_Stress;
	short	prev_Phon;
	long	prev_PhonCtrl;
	long	prev_PhonFlags;
	short	next_Phon;
	long	next_PhonCtrl;
	long	next_PhonFlags;
	short	next2_Phon;
	long	next2_PhonCtrl;
	long	next2_PhonFlags;
	short	i;
	short	maxDur, minDur;
	short	fixed_Duration;
	short	percent_Duration;
	short	eFlag, theObstr, vocFlag;
	short	dur_Hold;
	long	obstrFlags, num_1;
	short	total_Dur, vowel_Index, note_Dur, firstPass;
	
	total_Dur = 0;
	vowel_Index = 0;
	note_Dur = 0;
	firstPass = true;
	short	next_NoteDur, temp, dur_Adjust;
	short	tempS;

	prev_Phon = _SIL_;
	prev_PhonCtrl = 0;
	vv->markerIndex = 0;
	eFlag = false;
	firstPass = true;
	total_Dur = 0;
	vv->dur_Buf[0] = 1;			/* initial SIL = 5ms	*/
	for (i = 1; i < vv->phonBuf_2_In_Index; i++)
		{
		/*------------------*/
		/* cur				*/
		/*------------------*/
		cur_Phon = e_GetPhon (vv, i);
		cur_PhonCtrl = e_GetPhonCtrl (vv, i);
		cur_SyllableType = cur_PhonCtrl & kSyllableTypeField;
		cur_Stress = cur_PhonCtrl & kStressField;
		cur_PhonFlags = vv->phonFlags2[cur_Phon];
		if (cur_PhonFlags & kVowelF)
			cur_VowelFlag = true;
		else
			cur_VowelFlag = false;

		/*------------------*/
		/* prev				*/
		/*------------------*/
		prev_Phon = e_GetPhon (vv, i-1);
		prev_PhonCtrl = e_GetPhonCtrl (vv, i-1);
		prev_PhonFlags = vv->phonFlags2[prev_Phon];

		/*------------------*/
		/* next				*/
		/*------------------*/
		next_Phon = e_GetPhon (vv, i+1);
		next_PhonCtrl = e_GetPhonCtrl (vv, i+1);
		next_PhonFlags = vv->phonFlags2[next_Phon];

		/*------------------*/
		/* 2 phons ahead	*/
		/*------------------*/
		next2_Phon = e_GetPhon (vv, i+2);
		next2_PhonCtrl = e_GetPhonCtrl (vv, i+2);;
		next2_PhonFlags = vv->phonFlags2[next2_Phon];


		percent_Duration = k100pct_Dur;								/* Start with 100%	*/
		fixed_Duration = 0;
		maxDur = vv->maxDurTbl[cur_Phon];
		minDur = vv->minDurTbl[cur_Phon];

		/********************************************/
		/*											*/
		/*	#1 - Pause Insertion					*/
		/*											*/
		/********************************************/
		
		if (cur_Phon == _SIL_)
			{
			tempS = (cur_PhonCtrl & kSilenceTypeField) >> kSilenceTypeShift;
			if (tempS)
				{
				/*--------------------------------------*/
				/* Boundry silence						*/
				/*--------------------------------------*/
				dur_Hold = vv->BoundryDurTbl[tempS];
				}
			else
				/*--------------------------------------*/
				/* Non-boundry silence					*/
				/*--------------------------------------*/
				{
				dur_Hold = 200;
				}
				
			/*--------------------------------------*/
			/* Scale for speaking rate				*/
			/*--------------------------------------*/
			dur_Hold = (dur_Hold * vv->rate_Ratio) >> 16;
			
			/*--------------------------------------*/
			/* Silence embedded command				*/
			/*--------------------------------------*/
			if ( !(vv->singing) && (cur_PhonCtrl & kSilenceDuration) )
				{
				dur_Hold = vv->user_Note_Buf2[i];
				}
			
			/*--------------------------------------*/
			/* Min silence = 20ms					*/
			/*--------------------------------------*/
			if (dur_Hold < 10)
				dur_Hold = 10;
			
			goto Set_The_Dur;
			
			}
			
		/********************************************/
		/*											*/
		/*	#2 - Clause-final Lengthening			*/
		/*											*/
		/********************************************/
		if (cur_SyllableType & kTerm_End)
			{
			/*--------------------------*/
			/* cur = termiation			*/
			/*--------------------------*/
			
			if (cur_PhonFlags & kStopF)
				/*----------------------------------*/
				/* Don't lengthen stops				*/
				/*----------------------------------*/
				fixed_Duration = 0;
			
			else if ( (cur_PhonFlags & kVoicedF) && (cur_PhonFlags & kFric) )
				/*----------------------------------*/
				/* Lengthen voiced fricatives a bit	*/
				/*----------------------------------*/
				fixed_Duration = 20;
			
			else if ( (cur_PhonFlags & kVocLiq) &&
					(next_PhonFlags & kPlosFricF) && !(next_PhonFlags & kVoicedF) )
				/*----------------------------------*/
				/* RX or LX -> voiceless PlosFric	*/
				/*----------------------------------*/
				fixed_Duration = 15;
				
			else
				/*----------------------------------*/
				/* All others						*/
				/*----------------------------------*/
				fixed_Duration = 40;
			
			if (next_PhonFlags & kSonorantF)
				/*----------------------------------*/
				/* Shorten if next is sonorant		*/
				/*----------------------------------*/
				fixed_Duration -= 20;

			if ( (vv->phonBuf_2_In_Index < 10) && (cur_Stress) && (cur_VowelFlag) )
				/*----------------------------------*/
				/* Less than 10 phons in sentence	*/
				/* cur = stressed vowel				*/
				/*----------------------------------*/
				{
				fixed_Duration += (10 - vv->phonBuf_2_In_Index) * 5;
				}
			}



		if (cur_VowelFlag)
			/*=-=-=-=-=-=-=-=-=-=-=-*/
			/* cur = Vowel			*/
			/*=-=-=-=-=-=-=-=-=-=-=-*/
			{
			/************************************************/
			/*												*/
			/*	#3  Non-phrase-final shortening of vowels	*/
			/*												*/
			/************************************************/
				if (cur_SyllableType < kVerb_End) 
					{
					percent_Duration = (percent_Duration * 60 * pct) >> 16;
					}
			


			/************************************************/
			/*												*/
			/*	#4  Non-word final shortening of vowels		*/
			/*												*/
			/************************************************/
			
			if ( !(cur_Stress & kPrimOrEmphStress) && !(cur_PhonCtrl & kMore_Than_One_Syllable_In_Word) )
				/*----------------------------------------------*/
				/* NOT primary or emph stressed and 			*/
				/* only 1 vowel in word							*/
				/*----------------------------------------------*/
				{
				if (cur_Stress & kSecondaryStress)
					percent_Duration = (percent_Duration * 85 * pct) >> 16;
				else
					percent_Duration = (percent_Duration * 55 * pct) >> 16;
				}
			else if ( (cur_PhonCtrl & kMore_Than_One_Syllable_In_Word) && (cur_SyllableType < kWord_End) && !(cur_Stress & kPrimOrEmphStress) )
				/*----------------------------------------------*/
				/* more than 1 vowel in word					*/
				/* NOT at a word boundry						*/
				/*----------------------------------------------*/
				{
				if ( (cur_PhonCtrl & kSyllableOrderField) <= kFirst_Syllable_In_Word)
					percent_Duration = (percent_Duration * 85 * pct) >> 16;
				else
					percent_Duration = (percent_Duration * 80 * pct) >> 16;
				}
			
			/************************************************/
			/*												*/
			/*	#5  Polysyllabic shortening					*/
			/*												*/
			/************************************************/
			if (cur_PhonCtrl & kMore_Than_One_Syllable_In_Word)
				percent_Duration = (percent_Duration * 80 * pct) >> 16;
			}


		/************************************************/
		/*												*/
		/*	#6  Non-word-initial consonant shortening	*/
		/*												*/
		/************************************************/
		if ( !(cur_VowelFlag) && !(cur_PhonCtrl & kWord_Initial_Consonant) )
			/*----------------------------------*/
			/* Not word-initial AND a consonant	*/
			/*----------------------------------*/
			{
			if ( (cur_PhonFlags & kFric) && (cur_SyllableType & kWord_End) )
					
				/*------------------------------------------*/
				/* Except for fricitives at end of word.	*/
				/* Make them longer							*/
				/*------------------------------------------*/
				fixed_Duration += 20;
			else
				percent_Duration = (percent_Duration * 85 * pct) >> 16;
			}

		/************************************************/
		/*												*/
		/*	#7  Unstressed shortening					*/
		/*												*/
		/************************************************/
		if ( !(cur_Stress & kPrimOrEmphStress) )
			/*----------------------------------*/
			/* Not primary or emph stressed		*/
			/*----------------------------------*/
			{
			if ( !(cur_PhonFlags & kPlosFricF) && !(cur_PhonFlags & kGStopF) )		/* don't compress plosivies or fricatives	*/
				{
				minDur = minDur - (minDur >> 2);									/* .75 more compressible	*/
				}
			
			if (cur_VowelFlag)
				{
				/*----------------------------------*/
				/* Unstressed Vowel					*/
				/*----------------------------------*/
				if ( (cur_PhonCtrl & kSyllableOrderField) == kMid_Syllable_In_Word )
					percent_Duration = (percent_Duration * 55 * pct) >> 16;		/* syllabic word medial	*/
				else
					percent_Duration = (percent_Duration * 70 * pct) >> 16;		/* all syllabic others	*/
				
				}
			else
				/*----------------------------------*/
				/* Unstressed Consonant				*/
				/*----------------------------------*/
				{
				if ( (cur_Phon >= _w_) && (cur_Phon <= _l_) )
					percent_Duration = (percent_Duration * 60 * pct) >> 16;		/* liquid or glide	*/
				else
					percent_Duration = (percent_Duration * 70 * pct) >> 16;		/* all other consonants	*/
				}
			
			}


		/************************************************/
		/*												*/
		/*	#8  Lengthening for emphasis				*/
		/*												*/
		/************************************************/
		if ( (cur_PhonCtrl & kWord_Initial_Consonant) || 
				( cur_VowelFlag && (cur_Stress != kEmphaticStress)) )
			{
			eFlag = false;									/* start of a new word OR non-emph vowel	*/
			}
			
		if ( cur_Stress == kEmphaticStress )
			{
			eFlag = true;									/* continue lengthening until above condition is met	*/
			}
			
		if ( eFlag )
			{
			if (cur_VowelFlag)
				fixed_Duration += 60;						/* lengthen emph vowels			*/
			else
				fixed_Duration += 20;						/* lengthen emph consonants 	*/
			}




		/************************************************/
		/*												*/
		/*	#9  Postvocalic context of vowels			*/
		/*												*/
		/************************************************/
		vocFlag = false;
		theObstr = _SIL_;
		num_1 = k100percent;
		
		if ( (cur_VowelFlag) || 
			( ((cur_PhonFlags & kVocLiq) || (cur_PhonFlags & kNasalF)) &&
			!(cur_PhonCtrl & kStressedWInitial) &&
			(next_PhonFlags & kPlosFricF) ) )
			/*------------------------------------------------------------------*/
			/*	VOWEL or														*/
			/*  RX,LX or NASAL -> unstressed PlosFric (don't cross word bound)	*/
			/*------------------------------------------------------------------*/
			{
			if ( !(next_PhonFlags & kVowelF) && !(next_PhonCtrl & kStressedWInitial) )
				/*----------------------------------------------------------*/
				/* next = unstressed consonant (don't cross word bound)		*/
				/*----------------------------------------------------------*/
				{
				theObstr = next_Phon;					/* assume n+1 is the obstruent (n or n-1 is the vowel)	*/
				
				if ( ((next_PhonFlags & kVocLiq) || (next_PhonFlags & kNasalF)) &&
					!(next2_PhonCtrl & kStressedWInitial) &&
					(next2_PhonFlags & kPlosFricF) )
					{
					/*--------------------------------------------------------------------------*/
					/*  next = RX,LX or NASAL -> unstressed PlosFric (don't cross word bound)	*/
					/*--------------------------------------------------------------------------*/
					vocFlag = true;
					theObstr = next2_Phon;			/* n+2 is the obstruent (cur is the vowel)	*/
					}
					
				if (theObstr != _SIL_)
					{
					obstrFlags = vv->phonFlags2[theObstr];
					if ( !(obstrFlags & kVoicedF) )
						/*----------------------------------*/
						/* Obstruent is voiceless			*/
						/*----------------------------------*/
						{
						fixed_Duration = fixed_Duration - (fixed_Duration >> 1);		/* .75	*/
						num_1 = k1pct * 80;
						
						if (obstrFlags & (kStopF+kAffricateF))
							num_1 = k1pct * 55;				/* before a voiceless stop or affricate	*/
						}
						
					else if (obstrFlags & kPlosFricF)
						/*----------------------------------*/
						/* Obstruent is voiced PlosFric		*/
						/*----------------------------------*/
						{
						num_1 = k1pct * 120;				/* before a voiced fric or plosive	*/
						if ( !(obstrFlags & kStopF) && (theObstr != _DX_) && (cur_PhonFlags & kPrimOrEmphStress) )
							{
							fixed_Duration += 25;			/* before a voiced fric	*/
							}
						}
						
					else if (obstrFlags & kNasalF)
						/*----------------------------------*/
						/* Obstruent is nasal				*/
						/*----------------------------------*/
						{
						num_1 = k1pct * 85;
						}
					}
				}
			/*--------------------------------------------------*/
			/* Move closer to 100% if vowel is non-phrase final	*/
			/*--------------------------------------------------*/
			if ( (cur_SyllableType < kTerm_End) || (vocFlag) )
				num_1 = (num_1 >> 1) + kOneHalf;
			
			percent_Duration = (percent_Duration * num_1) >> 16;
				
			}

		/************************************************/
		/*												*/
		/*	#10  Shortening or lengthening in clusters	*/
		/*												*/
		/************************************************/
		
		if (cur_VowelFlag)
			{
			if (next_PhonFlags & kVowelF)
				/*--------------------------*/
				/*	VOWEL -> vowel			*/
				/*--------------------------*/
				{
				fixed_Duration += 30;
				}
				
			if ( ((cur_PhonCtrl & kSyllableOrderField) == kFirst_Syllable_In_Word) &&
				(cur_PhonCtrl & kPrimOrEmphStress) && !(prev_PhonCtrl & kWord_Initial_Consonant) )
				/*------------------------------*/
				/*	Stressed word-initial VOWEL	*/
				/*------------------------------*/
				{
				fixed_Duration += 25;
				}
			
			if (next_Phon == _LX_)
				{
				/*------------------------------*/
				/*	VOWEL -> LX					*/
				/*------------------------------*/
				fixed_Duration -= 20;
				}
			}

		else if (cur_PhonFlags & kConsonantF)
			{
			if ( (next_PhonFlags & kConsonantF) && (cur_SyllableType < kTerm_End) )
				/*------------------------------------------*/
				/* CONSONANT -> non-phrase-final consonant	*/
				/*------------------------------------------*/
				{
				num_1 = k1pct * 55;
				if ( (cur_PhonFlags & kNasalF) && (next_PhonCtrl & kWord_Initial_Consonant) )
					/*------------------------------------------*/
					/* NASAL -> word-initial-consonant			*/
					/*------------------------------------------*/
					{
					num_1 = k1pct * 150;
					}
				minDur = minDur - (minDur >> 2);
				
				if ( (cur_Phon == _s_) || (cur_Phon == _TH_) )
					{
					if (next_PhonFlags & kStopF)
						num_1 = k1pct * 50;
						
					if (next_Phon == _SH_)
						{
						dur_Hold = 12;
						goto Set_The_Dur;
						}
					}
				percent_Duration = (percent_Duration * num_1) >> 16;
				}
				
			//if ( (prev_PhonFlags & kConsonantF) && ((prev_PhonCtrl & kSyllableTypeField) < kTerm_End) )
			if ( (prev_PhonFlags & kConsonantF)  )							/* @@@@@	*/
				/*------------------------------------------*/
				/* non-phrase-final consonant -> CONSONANT	*/
				/*------------------------------------------*/
				{
				num_1 = k1pct * 55;
				minDur = minDur - (minDur >> 2);
				
				if (cur_PhonFlags & kStopF)
					{
					if (prev_Phon == _s_)
						num_1 = k1pct * 60;
					else if ( (prev_PhonFlags & kNasalF) && !cur_Stress )
						{
						num_1 = k1pct * 10;
						}
					}
					
				percent_Duration = (percent_Duration * num_1) >> 16;
				}
			
			}

		/************************************************/
		/*												*/
		/*	#11 Lengthening due to plosive aspiration	*/
		/*												*/
		/************************************************/
		
		if ( (cur_PhonFlags & kSonorantF) && !(prev_PhonFlags & kVoicedF) && (prev_PhonFlags & kStopF) )
			/*------------------------------------------*/
			/* voiceless stop -> SONORANT				*/
			/*------------------------------------------*/
			{
			fixed_Duration += 20;
			}

		/************************************************/
		/*												*/
		/*	#12 Lengthening due to glide				*/
		/*												*/
		/************************************************/
		
		if ( (cur_PhonFlags & kVowel1F) && (prev_PhonFlags & kSonorConsonF) && !(prev_PhonFlags & kNasalF) )
			/*------------------------------------------*/
			/* LiqGlide -> VOWEL						*/
			/*------------------------------------------*/
			{
			if (fixed_Duration == 0)
				fixed_Duration = 20;
			}

		/************************************************/
		/*												*/
		/*	Lengthen short phrases						*/
		/*												*/
		/************************************************/
		if ( (vv->phonBuf_2_In_Index < 10) && (minDur != maxDur) )
			/*----------------------------------*/
			/* Less than 10 phons in sentence	*/
			/*----------------------------------*/
			{
			fixed_Duration += (5 - (vv->phonBuf_2_In_Index >> 1)) * kFrameTime;
			}

		
		/********************************************/
		/* Check for rate change					*/
		/********************************************/
		temp = vv->user_Rate_Buf2[i];
		if (temp != 0)
			{
			if (vv->singing)
				{
				vv->tempo = temp;
				e_SetTempo (vv, vv->tempo);
				}
			else
				{
				vv->speech_Rate = temp;
				Init_Rate_Params (vv);
				}
			}

		if (cur_PhonCtrl & kSingingDuration)
		{
			dur_Hold = vv->user_Dur_Buf2[i];
			dur_Hold /= kFrameTime;
			total_Dur = 0;
			if (cur_VowelFlag)
				vowel_Index = i;
			goto Duration_Done;
		}

		dur_Hold = ((percent_Duration * (maxDur - minDur)) >> 7) + minDur;		/* [scale  x (max-min)] + min	*/
		
		if ( (vv->speech_Rate != kNormal_Speech_Rate) && (dur_Hold != 0) )
			{
			dur_Hold = (dur_Hold * vv->rate_Ratio_LowGain) >> 16;
			fixed_Duration = (fixed_Duration * vv->rate_Ratio) >> 16;
			}
			
		dur_Hold += fixed_Duration;
		
Set_The_Dur:
		dur_Hold = (dur_Hold * vv->user_Dur_Buf2[i]) >> kDurStepRes;
		dur_Hold /= kFrameTime;
		
Duration_Done:
		if ( (cur_Phon != _SIL_) && (dur_Hold < 8 / kFrameTime) )
			dur_Hold = 8 / kFrameTime;

		vv->dur_Buf[i] = dur_Hold;

		if (vv->sync_On_Marker && !(cur_PhonCtrl & kSingingDuration))
			{

			if ( (cur_PhonCtrl & kSyllable_Start) && (firstPass) )
					vv->phon_Ctrl_Buf_2[i] |= kSampleMarker;

			//if ( (cur_PhonCtrl & kWord_Start) || (cur_PhonCtrl & kTerm_Bound) )
			//if ( (cur_PhonCtrl & kSyllable_Start) || (cur_PhonCtrl & kTerm_Bound) )
			if ( (cur_PhonFlags & kVowelF) || (cur_PhonCtrl & kTerm_Bound) )
				{
				if ( !(cur_PhonCtrl & kTerm_Bound) &&  !(firstPass) )
					{
					if ((cur_PhonFlags & kSonorantF) ||  firstPass)
						vv->phon_Ctrl_Buf_2[i] |= kSampleMarker;
					else
						vv->phon_Ctrl_Buf_2[i+1] |= kSampleMarker;
					}
				if ( !firstPass )
					{
					note_Dur = (vv->markerBuf[vv->markerIndex+1] - vv->markerBuf[vv->markerIndex]) / (kSampFrameLen >> 1);
					dur_Adjust = note_Dur - total_Dur;
					if (next_Phon == _SIL_)
						vv->dur_Buf[vowel_Index] += (dur_Adjust-20);
					else
						vv->dur_Buf[vowel_Index] += (dur_Adjust-10);
					if (vv->dur_Buf[vowel_Index] < 4)
						vv->dur_Buf[vowel_Index] = 4;			/* @@@@	*/
					total_Dur = 0;
					vv->markerIndex++;
					if (vv->markerIndex == vv->lastMarkerIndex)
						vv->markerIndex = 0;
					}
				firstPass = false;
				}
			if ( cur_VowelFlag )
				vowel_Index = i;
			total_Dur += dur_Hold;
			}

		else if (vv->singing && !(cur_PhonCtrl & kSingingDuration))
			{
			next_NoteDur = (vv->user_Note_Buf2[i] & kNoteDur) >> kNoteDurShift;
			if ( (next_NoteDur != 0) || (cur_PhonCtrl & kTerm_Bound) )
				{
				if (!firstPass && !(vv->phon_Ctrl_Buf_2[vowel_Index] & kSingingDuration))
					{
					dur_Adjust = note_Dur - total_Dur;
					vv->dur_Buf[vowel_Index] += dur_Adjust;
					if (vv->dur_Buf[vowel_Index] < 4)
						vv->dur_Buf[vowel_Index] = 4;			/* @@@@	*/
					if (vv->dur_Buf[vowel_Index] > 100)			/* @@@@	*/
						vv->phon_Ctrl_Buf_2[vowel_Index] |= kLowVibrato;	/* less vibrato depth on sustained vowel	*/
					}
				firstPass = false;
				vowel_Index = i;
				note_Dur = vv->Note_Times[next_NoteDur];
				total_Dur = 0;
				}
			total_Dur += dur_Hold;
			if (cur_VowelFlag)
				vowel_Index = i;
			}
		
		}

}




short	Count_StressVowels_Till_Boundry (voiceVarPtr vv, short boundry, short cur_Index)
{
	short	i, count;
	short	cur_Phon;
	long	cur_PhonCtrl;
	long	cur_PhonFlags;
	
	count = 0;
	for (i = cur_Index; i < vv->phonBuf_2_In_Index; i++)
		{
		cur_Phon = vv->phon_Buf_2[i];
		cur_PhonCtrl = vv->phon_Ctrl_Buf_2[i];
		cur_PhonFlags = vv->phonFlags2[cur_Phon];
		if (i != cur_Index)
			{
			if ( (cur_PhonCtrl & kPrimOrEmphStress) && (cur_PhonFlags & kVowelF) )
				count++;
			}
		
		if ( (cur_PhonCtrl & kSyllableTypeField) >= boundry )
			break;
		//if (cur_Phon == _SIL_)
		//	break;
		}
	return (count);
}



short	Any_StressVowels_Remain (voiceVarPtr vv, short cur_Index)
{
	short	i, count;
	short	cur_Phon;
	long	cur_PhonCtrl;
	long	cur_PhonFlags;
	
	count = 0;
	for (i = cur_Index+1; i < vv->phonBuf_2_In_Index; i++)
		{
		cur_Phon = vv->phon_Buf_2[i];
		cur_PhonCtrl = vv->phon_Ctrl_Buf_2[i];
		cur_PhonFlags = vv->phonFlags2[cur_Phon];
		
		if ( (cur_PhonCtrl & kBoundryTypeField) == kWord_Start )
			break;

		if ( (cur_PhonCtrl & kPrimOrEmphStress) && (cur_PhonFlags & kVowelF) )
			count++;
		
		}
	return (count);
}


short	Count_Vowels_Till_Boundry (voiceVarPtr vv, short boundry, short cur_Index)
{
	short	i, count;
	short	cur_Phon;
	long	cur_PhonCtrl;
	long	cur_PhonFlags;
	
	count = 0;
	for (i = cur_Index; i < vv->phonBuf_2_In_Index; i++)
		{
		cur_Phon = vv->phon_Buf_2[i];
		cur_PhonCtrl = vv->phon_Ctrl_Buf_2[i];
		cur_PhonFlags = vv->phonFlags2[cur_Phon];
		if (i != cur_Index)
			{
			if ( cur_PhonFlags & kVowelF )
				count++;
			}
		
		if ( (cur_PhonCtrl & kSyllableTypeField) >= boundry )
			break;
		//if (cur_Phon == _SIL_)
		//	break;
		}
	return (count);
}





void	Pitch_RaiseAndFall (voiceVarPtr vv)
{
enum { kFallen, kRaised, kStart, kFinished};

	short	cur_Phon;
	long	cur_PhonCtrl;
	long	cur_PhonFlags;
	short	pState;
	short	index, i;
	short	wdIndex, firstWord, lastWord, lastState;
	long	wdType[64];
	short	action, stressCount;
	
	
	pState = kStart;
	lastState = kStart;
	wdIndex = 0;
	stressCount = 1;
	
	for (index = 0; index < vv->phonBuf_2_In_Index; index++)
		{
		/*----------*/
		/* cur		*/
		/*----------*/
		cur_Phon = vv->phon_Buf_2[index];
		cur_PhonCtrl = vv->phon_Ctrl_Buf_2[index];
		cur_PhonFlags = vv->phonFlags2[cur_Phon];

		if ( (pState == kRaised) && ((cur_PhonCtrl & kBoundryTypeField) == kWord_Start) )
			{
			if (cur_PhonCtrl & kContent_Word)
				wdType[wdIndex] = kPitchRise1;
			else
				wdType[wdIndex] = kPitchFall1;
			if (wdIndex < 63)
				wdIndex++;
			stressCount = 0;
			lastWord = index;
			if ( (lastState == kStart) && (pState == kRaised) )
				{
				lastState = kRaised;
				firstWord = index;
				}
			}

		if (cur_PhonFlags & kVowelF)
			{
			if (pState == kStart)
				{
				if ( (Count_Vowels_Till_Boundry (vv, kTerm_End, index)) == 0 )
					/*----------------------------------------------*/
					/* There's no more vowels in sentence.			*/
					/* FALL on this vowel and exit					*/
					/*----------------------------------------------*/
					{
					vv->phon_Ctrl_Buf_2[index] |= kPitchFall;
					pState = kFinished;
					break;
					}
			
				else if ( (Count_StressVowels_Till_Boundry (vv, kTerm_End, index)) == 0 )
					/*----------------------------------------------*/
					/* There's no more stress vowels in sentence.	*/
					/*----------------------------------------------*/
					{
					vv->phon_Ctrl_Buf_2[index] |= kPitchFall;
					pState = kFinished;
					//vv->phon_Ctrl_Buf_2[index] |= kPitchRise;
					//pState = kRaised;
					}
			
				else if (cur_PhonCtrl & kIsStressed)
					/*--------------------------*/
					/* RISE on first stressed	*/
					/* vowel in sentence.		*/
					/*--------------------------*/
					{
					vv->phon_Ctrl_Buf_2[index] |= kPitchRise;
					pState = kRaised;
					}
				}
			
			else if (pState == kRaised) 
				{
				if (cur_PhonCtrl & kPrimOrEmphStress)
					stressCount++;
					
				if ( (Count_Vowels_Till_Boundry (vv, kTerm_End, index)) == 0 )
					{
					/*----------------------------------*/
					/* FALL on last vowel in sentence	*/
					/*----------------------------------*/
					//if ( (vv->end_Punctuation == _Period_) || (vv->end_Punctuation == _Exclam_) )
						vv->phon_Ctrl_Buf_2[index] |= kPitchFall;
					pState = kFallen;
					break;
					}


				else if ( (cur_PhonCtrl & kPrimOrEmphStress) && ((Count_StressVowels_Till_Boundry (vv, kTerm_End, index)) == 0) )
					{
					/*--------------------------*/
					/* FALL on last stressed	*/
					/* vowel in sentence		*/
					/*--------------------------*/
					vv->phon_Ctrl_Buf_2[index] |= kPitchFall;
					pState = kFallen;
					break;
					}
				}
			}
		}
	
	wdIndex--;
	wdIndex--;
	if ( (wdIndex > 1) && (pState != kFinished) )
		{
		pState = kFallen;
		for (i = 0; i < wdIndex; i++)
			/*----------------------------------*/
			/* Remove consecutive RISE or FALLs	*/
			/*----------------------------------*/
			{
			if (pState == kFallen)
				{
				wdType[i] = kPitchRise1;
				pState = kRaised;
				}
			else
				{
				wdType[i] = kPitchFall1;
				pState = kFallen;
				}
			}
		
		if (pState == kRaised)
			{
			wdType[i] = kPitchFall1;
			wdIndex++;
			}
			
		action = false;
		i = 0;
		for (index = firstWord; index < lastWord; index++)
			{
			/*----------*/
			/* cur		*/
			/*----------*/
			cur_Phon = vv->phon_Buf_2[index];
			cur_PhonFlags = vv->phonFlags2[cur_Phon];
			cur_PhonCtrl = vv->phon_Ctrl_Buf_2[index];
			
			if ((cur_PhonCtrl & kBoundryTypeField) == kWord_Start)
				action = true;
				
			//if ( (cur_PhonFlags & kVowelF) && (cur_PhonCtrl & kPrimOrEmphStress) && action )
			if ( (cur_PhonFlags & kVowelF) && action )
				{
				if ( !(Any_StressVowels_Remain (vv, index)) )
					{
					action = false;
					if (i < wdIndex)
						vv->phon_Ctrl_Buf_2[index] |= wdType[i];
					i++;
					}
				}
			}
		
		}
}





#if 0
void	Pitch_RaiseAndFall (voiceVarPtr vv)
{
enum { kFallen, kRaised, kStart, kFinished};

	short	cur_Phon;
	long	cur_PhonCtrl;
	long	cur_PhonFlags;
	short	pState;
	short	index, i;
	short	wdIndex, firstWord, lastWord, lastState;
	long	wdType[64];
	short	action, stressCount;
	
	
	pState = kStart;
	lastState = kStart;
	wdIndex = 0;
	stressCount = 1;
	
	for (index = 0; index < vv->phonBuf_2_In_Index; index++)
		{
		/*----------*/
		/* cur		*/
		/*----------*/
		cur_Phon = vv->phon_Buf_2[index];
		cur_PhonCtrl = vv->phon_Ctrl_Buf_2[index];
		cur_PhonFlags = vv->phonFlags2[cur_Phon];

		if ( (pState == kRaised) && ((cur_PhonCtrl & kBoundryTypeField) == kWord_Start) )
			{
			//if (stressCount < 1)
			//	wdIndex--;							/* Last word had no stresses, skip it	*/
			if (cur_PhonCtrl & kContent_Word)
				wdType[wdIndex] = kPitchRise1;
			else
				wdType[wdIndex] = kPitchFall1;
			wdIndex++;
			stressCount = 0;
			lastWord = index;
			if ( (lastState == kStart) && (pState == kRaised) )
				{
				lastState = kRaised;
				firstWord = index;
				}
			}

		if (cur_PhonFlags & kVowelF)
			{
			if (pState == kStart)
				{
				if ( (Count_Vowels_Till_Boundry (vv, kTerm_End, index)) == 0 )
					/*----------------------------------------------*/
					/* There's no more vowels in sentence.			*/
					/* FALL on this vowel and exit					*/
					/*----------------------------------------------*/
					{
					vv->phon_Ctrl_Buf_2[index] |= kPitchFall;
					pState = kFinished;
					break;
					}
				else if ( (Count_StressVowels_Till_Boundry (vv, kTerm_End, index)) == 0 )
					/*----------------------------------------------*/
					/* There's no more stress vowels in sentence.	*/
					/*----------------------------------------------*/
					{
					vv->phon_Ctrl_Buf_2[index] |= kPitchFall;
					pState = kFinished;
					//vv->phon_Ctrl_Buf_2[index] |= kPitchRise;
					//pState = kRaised;
					}
				else if (cur_PhonCtrl & kIsStressed)
					/*--------------------------*/
					/* RISE on first stressed	*/
					/* vowel in sentence.		*/
					/*--------------------------*/
					{
					vv->phon_Ctrl_Buf_2[index] |= kPitchRise;
					pState = kRaised;
					}
				}
			
			else if (pState == kRaised) 
				{
				if (cur_PhonCtrl & kPrimOrEmphStress)
					stressCount++;
					
				if ( (Count_Vowels_Till_Boundry (vv, kTerm_End, index)) == 0 )
					{
					/*----------------------------------*/
					/* FALL on last vowel in sentence	*/
					/*----------------------------------*/
					//if ( (vv->end_Punctuation == _Period_) || (vv->end_Punctuation == _Exclam_) )
						vv->phon_Ctrl_Buf_2[index] |= kPitchFall;
					pState = kFallen;
					break;
					}


				else if ( (cur_PhonCtrl & kPrimOrEmphStress) && ((Count_StressVowels_Till_Boundry (vv, kTerm_End, index)) == 0) )
					{
					/*--------------------------*/
					/* FALL on last stressed	*/
					/* vowel in sentence		*/
					/*--------------------------*/
					vv->phon_Ctrl_Buf_2[index] |= kPitchFall;
					pState = kFallen;
					break;
					}
				}
			}
		}
	
	wdIndex--;
	if ( (wdIndex > 1) && (pState != kFinished) )
		{
		pState = kRaised;
		for (i = 0; i < wdIndex; i++)
			/*----------------------------------*/
			/* Remove consecutive RISE or FALLs	*/
			/*----------------------------------*/
			{
			if (pState == kRaised)
				{
				if (wdType[i] == kPitchRise1)
					wdType[i] = 0;
				else
					pState = kFallen;
				}
			else
				{
				if (wdType[i] == kPitchFall1)
					wdType[i] = 0;
				else
					pState = kRaised;
				}
			}
		
		action = false;
		i = 0;
		for (index = firstWord; index < lastWord; index++)
			{
			/*----------*/
			/* cur		*/
			/*----------*/
			cur_Phon = vv->phon_Buf_2[index];
			cur_PhonFlags = vv->phonFlags2[cur_Phon];
			cur_PhonCtrl = vv->phon_Ctrl_Buf_2[index];
			
			if ((cur_PhonCtrl & kBoundryTypeField) == kWord_Start)
				action = true;
				
			//if ( (cur_PhonFlags & kVowelF) && (cur_PhonCtrl & kPrimOrEmphStress) && action )
			if ( (cur_PhonFlags & kVowelF) && action )
				{
				action = false;
				vv->phon_Ctrl_Buf_2[index] |= wdType[i];
				i++;
				}
			}
		
		}
}

#endif


void	Fill_Phon_Buf_2 (voiceVarPtr vv)
{
	short	cur_Phon;
	long	cur_PhonCtrl;
	long	cur_PhonFlags;
	short	prev_Phon, prev2_Phon, prev3_Phon;
	long	prev_PhonCtrl;
	long	prev_PhonFlags, prev2_PhonFlags, prev3_PhonFlags;
	short	next_Phon, next2_Phon, next3_Phon;
	long	next_PhonCtrl, next2_PhonCtrl;
	long	next_PhonFlags;
	short	last_stored_phon;
	long	last_PhonFlags;
	short	delFwd;
	short	target_phon;
	short	insertGlot;
	short	user_Dur, user_Pitch, user_Cmd, user_Note, user_Rate;
	short	last_User_Pitch, cur_Syll;
	

	vv->phonBuf_2_In_Index = 0;
	last_stored_phon = _SIL_;
	last_User_Pitch = 0;
	
	for (vv->phonBuf_1_Out_Index = 0; vv->phonBuf_1_Out_Index < vv->phonBuf_1_In_Index; vv->phonBuf_1_Out_Index++)
		{
		/*--------------*/
		/* cur			*/
		/*--------------*/
		cur_Phon = vv->phon_Buf_1[vv->phonBuf_1_Out_Index];
		cur_PhonCtrl = vv->phon_Ctrl_Buf_1[vv->phonBuf_1_Out_Index];
		cur_PhonFlags = vv->phonFlags2[cur_Phon];
		cur_Syll = cur_PhonCtrl & kSyllableOrderField;
		
		/*--------------*/
		/* next			*/
		/*--------------*/
		if (vv->phonBuf_1_Out_Index < vv->phonBuf_1_In_Index -1)
			{
			next_Phon = vv->phon_Buf_1[vv->phonBuf_1_Out_Index +1];
			next_PhonCtrl = vv->phon_Ctrl_Buf_1[vv->phonBuf_1_Out_Index +1];
			}
		else
			{
			next_Phon = _SIL_;
			next_PhonCtrl = 0;
			}
		next_PhonFlags = vv->phonFlags2[next_Phon];

		/*--------------*/
		/* 2 ahead		*/
		/*--------------*/
		if (vv->phonBuf_1_Out_Index < (vv->phonBuf_1_In_Index -2))
			{
			next2_Phon = vv->phon_Buf_1[vv->phonBuf_1_Out_Index +2];
			next2_PhonCtrl = vv->phon_Ctrl_Buf_1[vv->phonBuf_1_Out_Index +2];
			}
		else
			{
			next2_Phon = _SIL_;
			next2_PhonCtrl = 0;
			}

		/*--------------*/
		/* 3 ahead		*/
		/*--------------*/
		if (vv->phonBuf_1_Out_Index < (vv->phonBuf_1_In_Index -3))
			{
			next3_Phon = vv->phon_Buf_1[vv->phonBuf_1_Out_Index +3];
			}
		else
			{
			next3_Phon = _SIL_;
			}

		/*--------------*/
		/* prev			*/
		/*--------------*/
		if (vv->phonBuf_1_Out_Index > 0)
			{
			prev_Phon = vv->phon_Buf_1[vv->phonBuf_1_Out_Index -1];
			prev_PhonCtrl = vv->phon_Ctrl_Buf_1[vv->phonBuf_1_Out_Index -1];
			}
		else
			{
			prev_Phon = _SIL_;
			prev_PhonCtrl = 0;
			}
		prev_PhonFlags = vv->phonFlags2[prev_Phon];

		/*--------------*/
		/* 2 back		*/
		/*--------------*/
		if (vv->phonBuf_1_Out_Index > 1)
			{
			prev2_Phon = vv->phon_Buf_1[vv->phonBuf_1_Out_Index -2];
			}
		else
			{
			prev2_Phon = _SIL_;
			}
		prev2_PhonFlags = vv->phonFlags2[prev2_Phon];
		
		/*--------------*/
		/* 3 back		*/
		/*--------------*/
		if (vv->phonBuf_1_Out_Index > 2)
			{
			prev3_Phon = vv->phon_Buf_1[vv->phonBuf_1_Out_Index -3];
			}
		else
			{
			prev3_Phon = _SIL_;
			}
		prev3_PhonFlags = vv->phonFlags2[prev3_Phon];
		
		/*--------------*/
		/*last stored	*/
		/*--------------*/
		if (vv->phonBuf_1_Out_Index == 0)
			last_stored_phon = _SIL_;
		else
			last_stored_phon = vv->phon_Buf_2[vv->phonBuf_2_In_Index-1];
		last_PhonFlags = vv->phonFlags2[last_stored_phon];
		
		user_Cmd = vv->user_Cmd_Buf1[vv->phonBuf_1_Out_Index];
		user_Pitch = vv->user_Pitch_Buf1[vv->phonBuf_1_Out_Index];
		user_Dur = vv->user_Dur_Buf1[vv->phonBuf_1_Out_Index];
		user_Note = vv->user_Note_Buf1[vv->phonBuf_1_Out_Index];
		user_Rate = vv->user_Rate_Buf1[vv->phonBuf_1_Out_Index];

		target_phon = cur_Phon;
		delFwd = false;
		insertGlot = false;

		/*--------------------------*/
		/* EN Rules					*/
		/*--------------------------*/
		if ( (vv->synthTech == kFormantSynth) && (cur_Phon == _n_) && (prev_Phon == _IX_) )
			{
			if ( (prev2_PhonFlags & kPlosFricF) && (prev2_Phon != _b_) && (prev2_Phon != _g_) )
				{
				if ( !((prev2_Phon == _d_) && (prev3_PhonFlags & kVowelF)) )
					{
					vv->phon_Buf_2[vv->phonBuf_2_In_Index -1] = _EN_;
					delFwd = true;
					}
				}
			}

		/*--------------------------*/
		/* EL Rules					*/
		/*--------------------------*/
		if ( (vv->synthTech == kFormantSynth) && (cur_Phon == _l_) && !(cur_PhonCtrl & (kPrimOrEmphStress + kWord_Initial_Consonant)) )
			{
			if ( (prev_Phon == _AX_) || (prev_Phon == _UH_) )
				{
				vv->phon_Buf_2[vv->phonBuf_2_In_Index -1] = _EL_;
				delFwd = true;
				goto STUFF_BUFF;
				}
			}

		//if ( !(cur_PhonCtrl & (kWord_Initial_Consonant)) && (prev_PhonFlags & kVowel1F) )
		if ( !(cur_PhonCtrl & (kPrimOrEmphStress + kWord_Initial_Consonant)) && (prev_PhonFlags & kVowel1F) )
			/*------------------------------------------------------*/
			/* NOT primary stressed (start of stressed syllable)	*/
			/* NOT Word-initial consonant							*/
			/* prev is vowel										*/
			/*------------------------------------------------------*/
			{
			if (cur_Phon == _l_)
				target_phon = _LX_;
			else if (cur_Phon == _r_)
				{
				target_phon = _RX_;
				switch (prev_Phon)
					{
					case _UW_:
					case _UH_:
						vv->phon_Buf_2[vv->phonBuf_2_In_Index -1] = _UR_;
						delFwd = true;
						break;

					case _AO_:
					case _OW_:
						vv->phon_Buf_2[vv->phonBuf_2_In_Index -1] = _OR_;
						delFwd = true;
						break;

					case _AA_:
						vv->phon_Buf_2[vv->phonBuf_2_In_Index -1] = _AR_;
						delFwd = true;
						break;

					case _AH_:
					case _AX_:
						vv->phon_Buf_2[vv->phonBuf_2_In_Index -1] = _ER_;
						delFwd = true;
						break;
						
					case _IH_:
					case _IY_:
						vv->phon_Buf_2[vv->phonBuf_2_In_Index -1] = _IR_;
						delFwd = true;
						break;
						
					case _AE_:
					case _EH_:
					case _EY_:
						if (vv->synthTech == kFormantSynth)
							{
							vv->phon_Buf_2[vv->phonBuf_2_In_Index -1] = _XR_;
							delFwd = true;
							}
						break;
					}
				}
			}



		/*----------------------------------*/
		/* yUW -> YU rule					*/
		/* "you, you'll, you'd" etc.		*/
		/* - NOT R-colored e.g. "uranium"	*/
		/*----------------------------------*/
		if ( 	
				(vv->synthTech == kFormantSynth) &&
				(prev_PhonCtrl & (kWord_Initial_Consonant)) &&
				(prev_Phon == _y_) &&									/* YOU... */
				(cur_Phon == _UW_) &&
				(next_Phon != _r_) &&									/* exclude R-colored */
				( (cur_PhonCtrl & kSyllableTypeField) >= kWord_End )	/* last syllable before word boundry */
				)
			{
			vv->phon_Buf_2[vv->phonBuf_2_In_Index -1] = _YU_;
			vv->phon_Ctrl_Buf_2[vv->phonBuf_2_In_Index -1] = cur_PhonCtrl;
			delFwd = true;
			}


		/*--------------------------*/
		/* DHAH -> DHIY rule		*/
		/* "thuh man" - "thee ant"	*/
		/*--------------------------*/
		if ( 	(next_PhonFlags & kVowelF) && (cur_Phon == _AH_) && 
				(cur_PhonCtrl & kSyllableTypeField) && (prev_Phon == _DH_) &&
				(prev_PhonCtrl & kWord_Initial_Consonant) &&
				(next_PhonCtrl & kPrimOrEmphStress) )
			{
			target_phon = _IY_;
			}


		#if 0
		/*--------------------------*/
		/* fER -> fOR rule			*/
		/* "fer me" - "for all"		*/
		/*--------------------------*/

		if (	(cur_Phon == _f_) && (next_Phon == _ER_)	&& 
				!(next_PhonCtrl & kMore_Than_One_Syllable_In_Word) )
			{
			if ( (vv->phonFlags2[next2_Phon] & kVowelF) || (next2_Phon == _SIL_) )
				{
				vv->phon_Buf_1[vv->phonBuf_1_Out_Index+1] = _OR_;
				next_Phon = _OR_;
				next_PhonFlags = vv->phonFlags2[next_Phon];
				}
			}
	


		/*--------------------------*/
		/* fAHr -> fAOr rule		*/
		/* "fer me" - "for all"		*/
		/*--------------------------*/

		else if (	(cur_Phon == _f_) && (next_Phon == _AH_) && (next2_Phon == _r_) && 
				!(next_PhonCtrl & kMore_Than_One_Syllable_In_Word) )
			{
			if ( (vv->phonFlags2[next3_Phon] & kVowelF) || (next3_Phon == _SIL_) )
				{
				vv->phon_Buf_1[vv->phonBuf_1_Out_Index+1] = _AO_;		/* this will become _OR_ */
				next_Phon = _AO_;
				next_PhonFlags = vv->phonFlags2[next_Phon];
				}
			}
		#endif

		/*--------------------------*/
		/* EHnd -> AEnd rule		*/
		/*--------------------------*/

		if (	(cur_Phon == _SIL_) && (next_Phon == _EH_) &&
				 (next2_Phon == _n_) && (next3_Phon == _d_) &&
				 (next_PhonCtrl & kPrimOrEmphStress) )
			{
			vv->phon_Buf_1[vv->phonBuf_1_Out_Index+1] = _AE_;
			next_Phon = _AE_;
			next_PhonFlags = vv->phonFlags2[next_Phon];
			}


		/*--------------------------*/
		/* Glottal rule				*/
		/*--------------------------*/
		if ( (vv->synthTech == kFormantSynth) && (cur_PhonFlags & kVowelF) && (next_PhonFlags & kVowelF) &&	
				(next_PhonCtrl & kPrimOrEmphStress) && (cur_PhonCtrl & kWord_End) )
			{
			insertGlot = true;
			}
		

		/*----------------------------------*/
		/* Dental->affricate y-slur rule	*/
		/*----------------------------------*/
		if ( 	((next_Phon == _YU_) || (next_Phon == _y_)) && 
				!(next_PhonCtrl & kPrimOrEmphStress) )
			/*----------------------------------*/
			/* next = _YU_ or _y_				*/
			/* next NOT stressed				*/
			/*----------------------------------*/
			{
		#if 0	
			if (cur_Phon == _t_)
				{
				/*--------------------------*/
				/* ty -> CHy  (voiceless)	*/
				/* "at your" -> "atch your"	*/
				/*--------------------------*/
				target_phon = _CH_;
				goto STUFF_BUFF;
				}
		#endif
			if (cur_Phon == _d_)
				{
				/*--------------------------*/
				/* dy -> JHy	(voiced)	*/
				/* "had you" -> "hadj you"	*/
				/*--------------------------*/
				target_phon = _JH_;
				goto STUFF_BUFF;
				}
			}

		
		/*------------------*/
		/* t rules			*/
		/*------------------*/
		if (cur_Phon == _t_)
			{
			/*--------------------------*/
			/* tUH -> tUW				*/
			/* "tuh him" - "too all"	*/
			/*--------------------------*/
			if ( 	(next_Phon == _UW_) && 
					((next_PhonCtrl & kSyllableTypeField) >= kWord_End ) &&
					!(cur_PhonCtrl & kPrimOrEmphStress) &&
					((next2_Phon == _SIL_) || (vv->phonFlags2[next2_Phon] & kVowelF)) ) 
				{
				vv->phon_Buf_1[vv->phonBuf_1_Out_Index +1] = _UW_;
				}

			/*--------------------------*/
			/* Glottalize the 't' if	*/
			/* followed by consonant	*/
			/*--------------------------*/
			else
				{ 
				/*--------------------------*/
				/* "at last" -> " ae'last "	*/
				/* "at the" -> " ae'the "	*/
				/*--------------------------*/
				if ( (next_Phon == _l_) || (next_Phon == _DH_) )
					{
SUB_T_GLOT:			if (last_PhonFlags & kSonorantF)
						target_phon = _TX_;
					else
						target_phon = _d_;
					goto STUFF_BUFF;
					}
				/*--------------------------*/
				/* "at me" -> " ae'me "		*/
				/* "at you" -> " ae'you "	*/
				/*--------------------------*/
				else if ( (cur_PhonCtrl & kSyllableTypeField) >= kWord_End )
					{
					if ( 	((next_PhonFlags & kSonorConsonF) && (next_Phon != _EN_)) || 	/* not EN at end of word */
							(next_Phon == _h_) )
						goto SUB_T_GLOT;
					}
				else if ( (next_Phon == _EN_) || ((next_Phon == _IX_) && (next2_Phon == _n_)) )
					{
					goto SUB_T_GLOT;
					}
				}
			}



		/*----------------------*/
		/* Dental flap DX rules	*/
		/*----------------------*/
		if ( (cur_Phon == _d_) || (cur_Phon == _t_) )
			{
			/*------------------------------*/
			/* Don't flap if syllabic 'n'	*/
			/*   will follow:				*/
			/*   "highten", "pardon"		*/
			/*------------------------------*/
			if ( (next_Phon == _IX_) && (next2_Phon == _n_) )
				{
				if (cur_Phon == _t_)
					goto SKIP_FLAP;
				else if ( !(prev_PhonFlags & kVowelF) )			// flap (vowel)-d-IX-n ("coincidence")
					goto SKIP_FLAP;
				}
			
			
			if ( 	(next_PhonFlags & kVowelF) &&
					(last_PhonFlags & kSonorantF) &&
					!(last_PhonFlags & kNasalF) )
				{
				//if ( (cur_PhonCtrl & kSyllableTypeField) >= kWord_End )
				if ( next_PhonCtrl & kWord_Start )
					/*------------------------------*/
					/* Always flap word term:		*/
					/*		"buT i am..."			*/
					/*------------------------------*/
					{
					target_phon = _DX_;
					}
				else if ( !(cur_PhonCtrl & kPrimOrEmphStress) )
					{
					if (cur_PhonCtrl & kWord_Initial_Consonant)
						/*------------------------------*/
						/* Flap word initial:			*/
						/*		"maybe Tonight"			*/
						/*		"maybe Demolish"		*/
						/*------------------------------*/
						{
						if ( (next_Phon == _AX_) || (next_Phon == _IX_) || (next_Phon == _UH_) ) 
							target_phon = _DX_;
						}
	
					/*+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*/
					/* T dental flap rules			*/
					/*+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*/
					else if (cur_Phon == _t_)
						{
						if (next_Phon == _OW_)
							/*----------------------------------------------*/
							/* Flap before _OW_ (only allowable dipthong):	*/
							/*		"mosquiTo"								*/
							/*		"mosquiTo reads"						*/
							/* 		"blowTorch" - no flap					*/
							/*		"alto" - no flap						*/
							/*----------------------------------------------*/
							{
							if ( (vv->phon_Ctrl_Buf_2[vv->phonBuf_2_In_Index-1] & kStressField) && 
								 ( (next2_Phon != _r_) || (next2_PhonCtrl & kWord_Initial_Consonant) ) )
								 {
								 target_phon = _DX_;
								 }
							}
							
						//else if (   ((next_Phon == _AH_) || (next_Phon == _AX_)) &&
						//			(next2_Phon == _r_) && ((next_PhonCtrl & kSyllableOrderField) == kLast_Syllable_In_Word) )
						else if (   ((next_Phon == _AH_) || (next_Phon == _AX_)) &&
									(next2_Phon == _r_)  && !(next_PhonCtrl & kPrimaryStress) )
							/*----------------------------------*/
							/* Flap ER at last syllable only:	*/
							/* AHr -> ER						*/
							/*		"centimeter"				*/
							/*----------------------------------*/
							{
							if (!(cur_PhonCtrl & kWord_Initial_Consonant)  && !(next_PhonCtrl & kPrimaryStress) )
								target_phon = _DX_;
							}
	
						else if (next_Phon == _ER_)
							/*----------------------------------*/
							/* Flap ER at last syllable only:	*/
							/*		"centimeter"				*/
							/*----------------------------------*/
							{
							//if ((cur_Syll == kLast_Syllable_In_Word) || ((next_PhonCtrl & kSyllableTypeField) >= kWord_End) )
							if (!(cur_PhonCtrl & kWord_Initial_Consonant))
								target_phon = _DX_;
							}
							
						else if ( ((next_Phon == _AX_) || (next_Phon == _IY_) || (next_Phon == _IX_) || (next_Phon == _EL_) ) 
								  && ((next2_Phon != _r_) || (next2_PhonCtrl & kWord_Initial_Consonant)) && !(next_PhonCtrl & kPrimaryStress) )
							{
							target_phon = _DX_;
							}
						}

					/*+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*/
					/* D dental flap rules			*/
					/*+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=*/
					else
						{
						if (next_Phon == _OW_)
							/*----------------------------------------------*/
							/* Flap before _OW_ (only allowable dipthong):	*/
							/*		"tornaDo"								*/
							/*----------------------------------------------*/
							{
							if (vv->phon_Ctrl_Buf_2[vv->phonBuf_2_In_Index-1] & kStressField)
								 {
								 target_phon = _DX_;
								 }
							}
		
						else if ( (next_Phon == _AX_) || (next_Phon == _IY_) || (next_Phon == _IX_) || (next_Phon == _EL_) ||
								  (next_Phon == _ER_) || (next_Phon == _IH_) || (next_Phon == _AH_) || (next_Phon == _AA_) )
							{
							target_phon = _DX_;
							}
						}
					}
				}
			}

SKIP_FLAP:


		/*----------------------*/
		/* DH rules				*/
		/*----------------------*/
		if ( (vv->synthTech == kFormantSynth) && (cur_Phon == _DH_) && !(cur_PhonCtrl & kPrimaryStress) )
			{
			switch (last_stored_phon)
				{
				/*----------------------------------*/
				/* "Vowelize" the DH				*/
				/* "at the store" -> "at duh store" */
				/*----------------------------------*/
				case _t_:
				case _TX_:
				case _d_:
					target_phon = _DD_;
					break;
				/*----------------------------------*/
				/* Slur the 'n' over 				*/
				/* "in the box" -> "in'uh box" 		*/
				/*----------------------------------*/
				case _n_:
					target_phon = _n_;
					break;
				}
			}

STUFF_BUFF:
		if (!delFwd)
			{
			vv->phon_Buf_2[vv->phonBuf_2_In_Index] = target_phon;
			vv->phon_Ctrl_Buf_2[vv->phonBuf_2_In_Index] = cur_PhonCtrl;

			vv->user_Cmd_Buf2[vv->phonBuf_2_In_Index] = user_Cmd;
			vv->user_Pitch_Buf2[vv->phonBuf_2_In_Index] = user_Pitch + last_User_Pitch;
			vv->user_Dur_Buf2[vv->phonBuf_2_In_Index] = user_Dur;
			vv->user_Note_Buf2[vv->phonBuf_2_In_Index] = user_Note;
			vv->user_Rate_Buf2[vv->phonBuf_2_In_Index] = user_Rate;
			
			if (vv->phonBuf_2_In_Index < kPhonBuf_Red_Zone)
				vv->phonBuf_2_In_Index++;
			
			if (insertGlot)
				{
				vv->phon_Buf_2[vv->phonBuf_2_In_Index] = _QX_;
				vv->phon_Ctrl_Buf_2[vv->phonBuf_2_In_Index] = 0;
				vv->user_Cmd_Buf2[vv->phonBuf_2_In_Index] = 0;
				vv->user_Pitch_Buf2[vv->phonBuf_2_In_Index] = vv->user_Pitch_Buf2[vv->phonBuf_2_In_Index-1];
				vv->user_Dur_Buf2[vv->phonBuf_2_In_Index] = kDur_One;
				vv->user_Note_Buf2[vv->phonBuf_2_In_Index] = 0;
				vv->user_Rate_Buf2[vv->phonBuf_2_In_Index] = 0;
				
				if (vv->phonBuf_2_In_Index < kPhonBuf_Red_Zone)
					vv->phonBuf_2_In_Index++;
				}
			}
		else
			{
			vv->user_Cmd_Buf2[vv->phonBuf_2_In_Index-1] += user_Cmd;
			vv->user_Pitch_Buf2[vv->phonBuf_2_In_Index-1] += user_Pitch;
			if (user_Dur != kDur_One)
				vv->user_Dur_Buf2[vv->phonBuf_2_In_Index-1] = user_Dur;
			if (user_Rate != 0)
				vv->user_Rate_Buf2[vv->phonBuf_2_In_Index-1] = user_Rate;
			/*vv->user_Note_Buf2[vv->phonBuf_2_In_Index-1] += user_Note;	*/
			
			if (cur_PhonCtrl & kSyllable_Start)
				{
				vv->phon_Ctrl_Buf_1[vv->phonBuf_1_Out_Index +1] |= kSyllable_Start;
				}
			}
			
		last_User_Pitch += user_Pitch;
		}

}












short	If_Consonant_Cluster (short Consonant_1st, short Consonant_2nd)
{
	short		ret;

	ret = false;
	switch (Consonant_1st)
		{
			case _f_:	switch (Consonant_2nd)
							{
							case _r_:
							case _l_:	ret = true; break;
							}
						break;
			case _v_:	switch (Consonant_2nd)
							{
							case _r_:
							case _l_:	ret = true; break;
							}
						break;
			case _TH_:	switch (Consonant_2nd)
							{
							case _r_:
							case _w_:	ret = true; break;
							}
						break;
			case _s_:	switch (Consonant_2nd)
							{
							case _w_:
							case _l_:
							case _p_:
							case _t_:
							case _k_:
							case _m_:
							case _n_:
							case _f_:	ret = true; break;
							}
						break;
			case _SH_:	switch (Consonant_2nd)
							{
							case _w_:
							case _l_:
							case _p_:
							case _t_:
							case _r_:
							case _m_:
							case _n_:	ret = true; break;
							}
						break;
			case _p_:	switch (Consonant_2nd)
							{
							case _r_:
							case _l_:	ret = true; break;
							}
						break;
			case _b_:	switch (Consonant_2nd)
							{
							case _r_:
							case _l_:	ret = true; break;
							}
						break;
			case _t_:	switch (Consonant_2nd)
							{
							case _r_:
							case _w_:	ret = true; break;
							}
						break;
			case _d_:	switch (Consonant_2nd)
							{
							case _r_:
							case _w_:	ret = true; break;
							}
						break;
			case _k_:	switch (Consonant_2nd)
							{
							case _r_:
							case _l_:
							case _w_:	ret = true; break;
							}
						break;
			case _g_:	switch (Consonant_2nd)
							{
							case _r_:
							case _l_:
							case _w_:	ret = true; break;
							}
						break;
		}
	return (ret);
}


short	Find_Next_Word_Bound (voiceVarPtr vv, short index)
{
	short	i;
	
	for (i = index+1; i < vv->phonBuf_1_In_Index; i++)
		{
		if ( vv->phon_Ctrl_Buf_1[i] & (kBoundryTypeField | kWord_Start) )
			break;
		}
	return (i);
}



void	MarkSyllableStart (voiceVarPtr vv)
{
	short	index;
	short	cur_Phon;
	long	cur_Ctrl;
	long	cur_PhonFlags;
	short	dist, syllable_index;
	short	phon_1st, phon_2nd;
	long	syllOrder;


	syllable_index = 0;
	for (index = 0; index < vv->phonBuf_1_In_Index; )
		{
		while (vv->phon_Buf_1[index] == _SIL_)
			{
			syllable_index++;
			index++;
			if (index >= vv->phonBuf_1_In_Index)
				goto SYLL_DONE;
			}
		cur_Phon = vv->phon_Buf_1[index];
		cur_Ctrl = vv->phon_Ctrl_Buf_1[index];
		cur_PhonFlags = vv->phonFlags2[cur_Phon];
		if (cur_PhonFlags & kVowelF)
			{
			vv->phon_Ctrl_Buf_1[syllable_index] |= kSyllable_Start;
			syllOrder = cur_Ctrl & kSyllableOrderField;
			if ( (syllOrder == kOneOrNo_Syllable_InWord) || (syllOrder == kLast_Syllable_In_Word) )
				{
				index = Find_Next_Word_Bound (vv, index);
				syllable_index = index;
				}
			else
				/*----------------------------------------------*/
				/* It's either the 1st or mid vowel in word.	*/
				/* Scan forward for consonants.					*/
				/*----------------------------------------------*/
				{
				dist = (-1);
				do
					{
					index++;
					cur_PhonFlags = vv->phonFlags2[vv->phon_Buf_1[index]];
					dist++;											/* count number of consonants	*/
					}
				while ( !(cur_PhonFlags & kVowelF) );
				
				if (dist == 0)
					{
					syllable_index = index;
					}

				else if (dist == 1)
					{
					index--;										/* start next syllable on consonant	*/
					syllable_index = index;
					}

				else if (dist == 2)
					{
					phon_2nd = vv->phon_Buf_1[index-1];
					phon_1st = vv->phon_Buf_1[index-2];
					if ( If_Consonant_Cluster (phon_1st, phon_2nd) )
						index -= 2;									/* start next syllable on cluster	*/
					else
						index--;									/* start next syllable on 2nd consonant	*/
					syllable_index = index;
					}

				else if (dist == 3)
					{
					phon_2nd = vv->phon_Buf_1[index-1];				/* 3rd consonant	*/
					phon_1st = vv->phon_Buf_1[index-2];				/* 2nd consonant	*/
					if ( If_Consonant_Cluster (phon_1st, phon_2nd) )
						{
						if (vv->phon_Buf_1[index-3] == _s_)
							index -= 3;								/* start next syllable on s-cluster	*/
						else
							index -= 2;								/* start next syllable on cluster	*/
						}
					else
						index--;									/* start next syllable on 3rd consonant	*/
					syllable_index = index;
					}

				else
					{
					phon_2nd = vv->phon_Buf_1[index-dist];			/* 1st consonant	*/
					phon_1st = vv->phon_Buf_1[index-dist+1];		/* 2nd consonant	*/
					if ( If_Consonant_Cluster (phon_1st, phon_2nd) )
						index -= (dist - 2);						/* start next syllable after cluster	*/
					else
						index -= (dist >> 1);						/* start next syllable somewhere in the middle	*/
					syllable_index = index;
					}
				}
			}
		else
			index++;
		
		}
SYLL_DONE:
	return;
}




void	Place_Stress_In_Consonant (voiceVarPtr vv)
{
	short	index;
	short	cur_Phon;
	long	cur_Ctrl;
	long	cur_PhonFlags;
	short	dist;
	short	phon_1st, phon_2nd;
	
	for (index = vv->scanIndex+1; index < vv->phonBuf_1_In_Index; index++)
		{
		/*----------------------------------------	*/
		/* Loop fwd till a stress is found.	*/
		/* Exit at vowel or word boundry.	*/
		/*----------------------------------------	*/
		cur_Phon = vv->phon_Buf_1[index];
		cur_Ctrl = vv->phon_Ctrl_Buf_1[index];
		cur_PhonFlags = vv->phonFlags2[cur_Phon];
		if (cur_Ctrl & kIsStressed)
			{
			dist = index - vv->scanIndex;
			if (dist > 3)
				break;					/* we look at only 3 proceeding consonants	*/
			if (dist > 1)
				/*------------------------------------	*/
				/* Stress is two or three phons away	*/
				/*------------------------------------	*/
				{
				/*------------------------------------	*/
				/* If it's a valid consonant PAIR,	*/
				/*  stress it.	*/
				/*------------------------------------	*/
				phon_2nd = vv->phon_Buf_1[index-1];
				phon_1st = vv->phon_Buf_1[index-2];
				if (!(If_Consonant_Cluster (phon_1st, phon_2nd)))
					break;									/* not a consonant pair, EXIT	*/
				
				/*------------------------------------	*/
				/* If it's a valid consonant TRIO,	*/
				/*  stress it.	*/
				/*------------------------------------	*/
				if (dist == 3)
					{
					if (vv->phon_Buf_1[index-3] != _s_)
						break;								/* not a consonant trio, EXIT	*/
					}
				}
				
			if (cur_Ctrl & kPrimaryStress)
				{
				if (vv->is_Compound_Noun)
					vv->phon_Ctrl_Buf_1[vv->scanIndex] |= kSecondaryStress;
				else
					vv->phon_Ctrl_Buf_1[vv->scanIndex] |= kPrimaryStress;
				}
				
			else if (cur_Ctrl & kSecondaryStress)
				{
				if (!vv->is_Compound_Noun)
					vv->phon_Ctrl_Buf_1[vv->scanIndex] |= kSecondaryStress;
				}
				
			else if (cur_Ctrl & kEmphaticStress)
				{
				vv->phon_Ctrl_Buf_1[vv->scanIndex] |= kEmphaticStress;
				}
			break;
			}
		/*----------------------------------------	*/
		/* Exit at vowel or word boundry.	*/
		/*----------------------------------------	*/
		if ( (cur_PhonFlags & kVowelF) || (cur_Ctrl & kBoundryTypeField) )
			break;
		}
	
}



void	MarkSyllable (voiceVarPtr vv)
{
	short	index;
	short	cur_Phon;
	long	cur_Bound;
	long	cur_PhonFlags;
	long	order;
	long	cur_SyllableType;
	
	
	/*------------------------------------------------------------------------------*/
	/* Scan backwards in PhonBuf_1 till word boundry and look for any other vowels.	*/
	/* Set 'order' to kLast_Syllable_In_Word if there are any.						*/
	/*------------------------------------------------------------------------------*/
	order = 0;
	for (index = vv->scanIndex-1; index > 0; index--)
		{
		cur_Phon = vv->phon_Buf_1[index];
		cur_PhonFlags = vv->phonFlags2[cur_Phon];
		cur_SyllableType = vv->phon_Ctrl_Buf_1[index] & kSyllableTypeField;
		if (cur_SyllableType >= kWord_End)
			break;
		
		if (cur_PhonFlags & kVowelF)
			{
			order = kLast_Syllable_In_Word;		/* there's at least one proceeding vowel	*/
			break;
			}
		}

	/*----------------------------------------------------------------------------------*/
	/* Scan forward in PhonBuf_1 till word boundry and look for any other vowels		*/
	/* If there's a fwd vowel but no bkwd vowel: 	'order' = kFirst_Syllable_In_Word	*/
	/* If there's a fwd vowel and a bkwd vowel: 	'order' = kMid_Syllable_In_Word		*/
	/* If there's no fwd vowel but a bkwd vowel: 	'order' = kLast_Syllable_In_Word	*/
	/* If there's no fwd vowel and no bkwd vowel: 	'order' = 0							*/
	/*----------------------------------------------------------------------------------*/
	for (index = vv->scanIndex+1; index < vv->phonBuf_1_In_Index; index++)
		{
		cur_Phon = vv->phon_Buf_1[index];
		cur_Bound = vv->phon_Ctrl_Buf_1[index] & kBoundryTypeField;
		cur_PhonFlags = vv->phonFlags2[cur_Phon];
		if (cur_Bound)
			{
			vv->phon_Ctrl_Buf_1[vv->scanIndex] |= order;
			break;
			}
		if (cur_PhonFlags & kVowelF)
			{
			if (order == kLast_Syllable_In_Word)
				order = kMid_Syllable_In_Word;
			else
				if (order == 0)
					order = kFirst_Syllable_In_Word;
			}
		}
}





/************************************************/
/* Mark phons in last syllable before boundry	*/
/*  with boundry type flag						*/
/************************************************/

void	MarkBoundry (voiceVarPtr vv)
{
	short	index;
	short	cur_Phon;
	long	cur_PhonFlags;
	long	cur_Bound;
	long	boundType;
	
	
	for (index = vv->scanIndex+1; index < vv->phonBuf_1_In_Index; index++)
		{
		cur_Phon = vv->phon_Buf_1[index];
		cur_PhonFlags = vv->phonFlags2[cur_Phon];
		cur_Bound = vv->phon_Ctrl_Buf_1[index] & kBoundryTypeField;
		if (cur_Bound)
			{
			boundType = 0;
			if (cur_Bound & kTerm_Bound)	boundType |= (kTerm_End + kWord_End);
			if (cur_Bound & kPrep_Start)	boundType |= (kPrep_End + kWord_End);
			if (cur_Bound & kVerb_Start)	boundType |= (kVerb_End + kWord_End);
  			if (cur_Bound & kWord_Start)	boundType |= kWord_End;
			vv->phon_Ctrl_Buf_1[vv->scanIndex] |= boundType;
			}

		if (cur_PhonFlags & kVowelF)
			break;
		}
}







void	Flag_PhonBuf_1 (voiceVarPtr vv)
{
	short	cur_Phon;
	long	cur_PhonFlags;	
	long	cur_Ctrl;	

	vv->is_Compound_Noun = false;
	
	for (vv->scanIndex = 0; vv->scanIndex < vv->phonBuf_1_In_Index; vv->scanIndex++)
		{
		cur_Phon = vv->phon_Buf_1[vv->scanIndex];
		cur_PhonFlags = vv->phonFlags2[cur_Phon];
		cur_Ctrl = vv->phon_Ctrl_Buf_1[vv->scanIndex];

		if (cur_Ctrl & kCompoundNoun)
			vv->is_Compound_Noun = true;
		else if (cur_Ctrl & kBoundryTypeField)
			vv->is_Compound_Noun = false;

		if (cur_PhonFlags & kVowelF)
			{
			/*--------------------------*/
			/* Phon is a VOWEL			*/
			/*--------------------------*/
			MarkSyllable (vv);
			}
		else
			/*--------------------------*/
			/* Phon is a CONSONANT		*/
			/*--------------------------*/
			{
			//Place_Stress_In_Consonant (vv);
			}

		MarkBoundry (vv);
		}
		
	MarkSyllableStart (vv);
}







static void Store_Phon_In_PhonBuf_1 (voiceVarPtr vv, short phon)
{
	short	cur_PhonType;
	short	storeIt;

	storeIt = true;
	cur_PhonType = vv->phonTypeTbl[phon];
	if (cur_PhonType & kPhonemeType)
		{
		
#if 0
		/*------------------------------*/
		/* EL and EN rules				*/
		/*								*/
		/* Kludgey, but it needs to	be	*/
		/* done before the Allo rules!	*/
		/*------------------------------*/
		if ( (vv->phonBuf_1_In_Index > 0) &&
			 !(vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index-1] & kPrimOrEmphStress) &&
			 !(vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index-1] & kWord_Start) )
			{
			prevPhon = vv->phon_Buf_1[vv->phonBuf_1_In_Index-1];
			
			if ( (phon == _n_) && ((prevPhon == _EH_) || ((prevPhon == _IX_) && (vv->phon_Buf_1[vv->phonBuf_1_In_Index-2] == _t_) )) )
				{
				vv->phon_Buf_1[vv->phonBuf_1_In_Index-1] = _EN_;
				storeIt = false;
				}
			if ( (phon == _l_) && (prevPhon == _AX_) && (vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index+1] & kWord_Start))
				{
				vv->phon_Buf_1[vv->phonBuf_1_In_Index-1] = _EL_;
				storeIt = false;
				}
			}
#endif

		if (storeIt)
			{
			vv->phon_Buf_1[vv->phonBuf_1_In_Index] = phon;
	
			if (vv->phonBuf_1_In_Index < kPhonBuf_Red_Zone)
				vv->phonBuf_1_In_Index++;
	
			/*-------------------------	*/
			/* init next buffers	*/
			/*-------------------------	*/
			vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] = 0;
			vv->user_Pitch_Buf1[vv->phonBuf_1_In_Index] = 0;
			vv->user_Note_Buf1[vv->phonBuf_1_In_Index] = 0;
			vv->user_Cmd_Buf1[vv->phonBuf_1_In_Index] = 0;
			vv->user_Dur_Buf1[vv->phonBuf_1_In_Index] = kDur_One;
			vv->user_Rate_Buf1[vv->phonBuf_1_In_Index] = 0;
			}
		}
		

}








void	QueueCommand (voiceVarPtr vv, short cmd, long data)
{
	if (vv->cmdBufCount < kCQsize)							/* make sure we have room in the queue for another ctrl	*/
		{
		vv->CMDQueue[vv->cmdBufCount].type = cmd;
		vv->CMDQueue[vv->cmdBufCount++].data = data;
		vv->user_Cmd_Buf1[vv->phonBuf_1_In_Index]++;		/* number of commands at this position	*/
		}
}




void	Parse_Embedded_Command ( voiceVarPtr vv, BECommandPtr bCmdPtr)
{
	uint32_t	embedType;
	int32_t		embedData;

	embedType = ((uint32_t)bCmdPtr[2] << 16) | (uint32_t)bCmdPtr[3];
	embedData = ((int32_t)bCmdPtr[4] << 16) | (uint32_t)bCmdPtr[5];

	switch (embedType)
		{
		case EC_pbas:			/* baseline pitch (absolute)	*/
			QueueCommand (vv, C_absPitch, embedData);
			break;
			
		case EC_pbar:			/* baseline pitch (relative)	*/
			QueueCommand (vv, C_relPitch, embedData);
			break;
			
		case EC_pmod:			/* pitch modulation (absolute)	*/
			QueueCommand (vv, C_absMod, embedData);
			break;
			
		case EC_pmor:			/* pitch modulation (relative)	*/
			QueueCommand (vv, C_relMod, embedData >> 16);
			break;
			
		case EC_rate:			/* speaking rate (absolute)	*/
			vv->lastRate = embedData >> 16;										/* integer part for RATE	*/
			if (vv->lastRate < kMinRate)
				vv->lastRate = kMinRate;
			vv->user_Rate_Buf1[vv->phonBuf_1_In_Index] = vv->lastRate;
			break;
			
		case EC_ratr:			/* speaking rate (relative)	*/
			vv->lastRate += (embedData >> 16);									/* integer part for RATE	*/
			if (vv->lastRate < kMinRate)
				vv->lastRate = kMinRate;
			vv->user_Rate_Buf1[vv->phonBuf_1_In_Index] = vv->lastRate;
			break;
			
		case EC_volm:			/* speaking volume 	(absolute)	*/
			QueueCommand (vv, C_absVol, embedData);
			break;
			
		case EC_volr:			/* speaking volume 		(relative)	*/
			QueueCommand (vv, C_relVol, embedData);
			break;
			
		case EC_slnc:			/* insert silence into speech	*/
			embedData >>= 16;
			if (embedData > 0)
				{
				vv->user_Note_Buf1[vv->phonBuf_1_In_Index] = embedData;				/* note duration	*/
				vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kSilenceDuration;
				Store_Phon_In_PhonBuf_1 (vv, _SIL_);
				}
			break;
			
		case EC_svox:			/* current voice	*/
			QueueCommand (vv, C_voice, embedData);
			break;
			
		case EC_sync:			/* make synchronization callback	*/
			QueueCommand (vv, C_sync, embedData);
			break;
			
		case EC_word:			/* make word callback	*/
			QueueCommand (vv, C_word, embedData);
			break;
			
		case EC_rset:			/* make reset callback	*/
			QueueCommand (vv, C_reset, embedData);
			break;
			
		case EC_note:			/* issue NOTE command	*/
			vv->user_Note_Buf1[vv->phonBuf_1_In_Index] = (embedData >> 16) & kNotePitch;		/* note pitch	*/
			vv->user_Note_Buf1[vv->phonBuf_1_In_Index] |= embedData  & kNoteDur;				/* note duration	*/
			vv->singing = true;
			break;

		case EC_sing:			/* issue SING command	*/
			vv->user_Note_Buf1[vv->phonBuf_1_In_Index] = (embedData >> 16);		/* note pitch	*/
			vv->user_Dur_Buf1[vv->phonBuf_1_In_Index] = (embedData & 0xFFFF);		/* note duration	*/
			vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kSingingDuration;
			vv->singing = true;
			break;

		case EC_tempo:			/* issue TEMPO command	*/
			//vv->user_Tempo_Buf1[vv->phonBuf_1_In_Index] = embedData >> 16;						/* integer part for TEMPO	*/
			break;

		case EC_marker:			/* issue MARKER command	*/
			if (vv->markerIndex < kMaxMarkers-1)
				{
				vv->markerBuf[vv->markerIndex] = embedData;										/* the marker		*/
				vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kSampleMarker;
				vv->sync_On_Marker = true;
				vv->singing = true;
				vv->markerIndex++;
				}
			else
				{
				vv->sync_On_Marker = false;
				vv->singing = false;
				}
			break;
		}
}






short	Collect_FE_Tokens ( voiceVarPtr vv )
{

#define	kNoIndex (-1)

	short			cur_Phon;
	short			cur_PhonType;
	long			cur_PhonFlags;
	short			lastStress_1_Index;
	short			lastStress_2_Index;
	short			last_Word_Index;
	short			lastVowel_Index;
	short			word_Initial;
	short			stress_Counter;			/* count primary or emphatic stresses	*/
	unsigned char	*tokStr;
	short			gotSentence, index, wordCount;
	long			boffs;
	short			len;
	short			wordStress_1_Index;
	short			wordStress_2_Index;
	short			wordVowel_Index;
	short			wordWasEmph, wordWasDeemph;
	short			tempIndex, curPOS, isContentWord;

	vv->cmdBufCount = 0;						/* start WRITING commands into top of queue	*/
	vv->end_Punctuation = 0;
	stress_Counter = 0;
	word_Initial = true;
	vv->is_Compound_Noun = false;
	vv->start_of_Paragraph_Flag = false;
	lastStress_1_Index = kNoIndex;
	lastStress_2_Index = kNoIndex;
	lastVowel_Index = kNoIndex;
	wordStress_1_Index = kNoIndex;
	wordStress_2_Index = kNoIndex;
	wordVowel_Index = kNoIndex;
	wordWasEmph = false;
	wordWasDeemph = false;
	vv->markerIndex = 0;
	gotSentence = false;
	wordCount = 0;
	vv->lastRate = vv->speech_Rate;
	isContentWord = false;
	

	vv->phon_Buf_1[0] = _SIL_;
	vv->phon_Ctrl_Buf_1[0] = 0;
	vv->user_Pitch_Buf1[0] = 0;
	vv->user_Note_Buf1[0] = 0;
	vv->user_Cmd_Buf1[0] = 0;
	vv->user_Dur_Buf1[0] = kDur_One;
	vv->user_Rate_Buf1[0] = 0;

	vv->phon_Ctrl_Buf_1[1] = 0;
	vv->user_Pitch_Buf1[1] = 0;
	vv->user_Note_Buf1[1] = 0;
	vv->user_Cmd_Buf1[1] = 0;
	vv->user_Dur_Buf1[1] = kDur_One;
	vv->user_Rate_Buf1[1] = 0;

	vv->phonBuf_1_In_Index = 1;
	last_Word_Index = vv->phonBuf_1_In_Index;
	
	while (!gotSentence)
		{
		if (vv->phonBuf_1_In_Index >= kPhonBuf_Yellow_Zone)
			{
			/*------------------------------*/
			/* Try to end gracefully		*/
			/*  at a word boundry.			*/
			/*------------------------------*/
			vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kTerm_Bound;
				vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= (kBND_Pause << kSilenceTypeShift);
			vv->end_Punctuation = _Comma_;
			Store_Phon_In_PhonBuf_1 (vv, _SIL_);
			gotSentence = true;
			break;
			}
		vv->opTok = (*(vv->funcList->e_ParseNextWord_FUNC)) (vv);	/* scan the next "word" from the input text	*/
		
		if ( (vv->opTok->hasAlt) && (vv->opTok->altChoice == 1) )
			{
			vv->opCount = vv->opTok->phonHold[0];						/* number of opcodes in our token	*/
			tokStr = &vv->opTok->phonHold[1];							/* ptr to the first opcode			*/
			}
		else
			{
			vv->opCount = vv->opTok->phonStr[0];						/* number of opcodes in our token	*/
			tokStr = &vv->opTok->phonStr[1];							/* ptr to the first opcode			*/
			}

		if (vv->opTok->tokType == kEOFTok)
			{
			if (!vv->end_Punctuation)									/* No term punctuation in sentence!	*/
				{
				vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kTerm_Bound;
				vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= (kBND_Decl << kSilenceTypeShift);
				vv->end_Punctuation = _Period_;
				Store_Phon_In_PhonBuf_1 (vv, _SIL_);
				}
			vv->FEinputDone = true;
			break;
			}

		/*------------------------------*/
		/* Add minor phrase boundry		*/
		/*------------------------------*/
		if ( (vv->opTok->add_BND) & (!vv->singing) )
			{
			vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= (vv->opTok->phrasingBND << kSilenceTypeShift);
			if ( (vv->opTok->phrasingBND >= kBND_Paren_L) && (vv->opTok->phrasingBND != kBND_Sep6) )
				{
				vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kVerb_Start;
				Store_Phon_In_PhonBuf_1 (vv, _SIL_);
				}
			word_Initial = true;
			vv->is_Compound_Noun = false;
				
			}
			
		/*------------------------------*/
		/* Process embedded command		*/
		/*------------------------------*/
		if (vv->opTok->tokType == kECommandTok)
			{
			Parse_Embedded_Command (vv, (BECommandPtr)tokStr);
			wordCount++;
			}
		else
			{
			while (vv->opCount)
				{
				if (vv->phonBuf_1_In_Index >= kPhonBuf_Red_Zone)
					{
					/*------------------------------*/
					/* Buffer is full.				*/
					/* End abruptly now!			*/
					/*------------------------------*/
					vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kTerm_Bound;
					vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= (kBND_Pause << kSilenceTypeShift);
					vv->end_Punctuation = _Comma_;
					Store_Phon_In_PhonBuf_1 (vv, _SIL_);
					gotSentence = true;
					break;
					}
	
				cur_Phon = (short) (*tokStr++);					/* here's the next command opcode (extend from byte to BECommand)	*/
				cur_PhonType = vv->phonTypeTbl[cur_Phon];
				cur_PhonFlags = vv->phonFlags2[cur_Phon];
				
				if (cur_PhonType & kControlType)
					/*--------------------------*/
					/* Phon is a CONTROL		*/
					/*--------------------------*/
					{
					switch (cur_Phon)
						{
						case _Stress1_:
							if ( (vv->is_Compound_Noun ) || !(isContentWord) )
							//if ( vv->is_Compound_Noun )
								{
								vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kSecondaryStress;
								lastStress_2_Index = vv->phonBuf_1_In_Index;
								if (wordStress_2_Index == kNoIndex)						/* remember 1st stress only	*/
									wordStress_2_Index = vv->phonBuf_1_In_Index;
								}
							else
								{
								vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kPrimaryStress;
								lastStress_1_Index = vv->phonBuf_1_In_Index;
								if (wordStress_1_Index == kNoIndex)						/* remember 1st stress only	*/
									wordStress_1_Index = vv->phonBuf_1_In_Index;
								stress_Counter++;
								}
							break;
							
						case _Stress2_:
							if (!vv->is_Compound_Noun)
								{
								vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kSecondaryStress;
								if (wordStress_2_Index == kNoIndex)						/* remember 1st stress only	*/
									wordStress_2_Index = vv->phonBuf_1_In_Index;
								}
							lastStress_2_Index = vv->phonBuf_1_In_Index;
							break;
							
						case _EmphStress_:
							vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kEmphaticStress;
							stress_Counter++;
							break;
		
						case _Word_:
							/*--------------------------------------------------*/
							/* If last word was Emph, insert emph stress		*/
							/*  at last 1 or 2 stress or at last vowel in word	*/
							/*--------------------------------------------------*/
							if (wordWasEmph)
								{
								wordWasEmph = false;
								tempIndex = kNoIndex;
								
								if (wordStress_1_Index != kNoIndex)
									tempIndex = wordStress_1_Index;
									
								else if (wordStress_2_Index != kNoIndex)
									tempIndex = wordStress_2_Index;

								else if (wordVowel_Index != kNoIndex)
									tempIndex = wordVowel_Index;
									
								if (tempIndex != kNoIndex)
									{
									vv->phon_Ctrl_Buf_1[tempIndex] &= ~kStressField;
									vv->phon_Ctrl_Buf_1[tempIndex] |= kEmphaticStress;
									}
								}
							
							vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kWord_Start;
							word_Initial = true;
							vv->is_Compound_Noun = false;
							last_Word_Index = vv->phonBuf_1_In_Index;
							wordCount++;

							if (vv->WordCB)									// make word callbacks
								{
								if (vv->opTok->tokType == kLiteralTok)
									{
									boffs = vv->opTok->bufOffset + vv->opTok->tokPos - 1;
									len   = 1;
									}
								else if (vv->opTok->tokType == kRawPhonemeTok)
									{
									boffs = vv->opTok->bufOffset + 1;
									len   = vv->opTok->phonStr[0];
									}
								else
									{
									boffs = vv->opTok->bufOffset;
									len   = vv->opTok->tokStr[0];
									}
								QueueCommand (vv, C_word, (len << 24) + boffs);
								}
								
							/*--------------------------------------------------*/
							/* Remember word prominence (if any)				*/
							/*--------------------------------------------------*/
							wordWasEmph = false;
							wordWasDeemph = false;							/* Assume NEW word has no prominence			*/
							wordStress_1_Index = kNoIndex;
							wordStress_2_Index = kNoIndex;					/* Starting NEW word so reset word stress ptr	*/
							wordVowel_Index =  kNoIndex;
							if (vv->opTok->tokEmphasis == kEmphasizeWord)
								{
								wordWasEmph = true;
								}
							else if (vv->opTok->tokEmphasis == kDeemphasizeWord)
								wordWasDeemph = true;

							curPOS = vv->opTok->POSchoice;
							if ( 	(curPOS == kNoun) || (curPOS == kVerb) || (curPOS == kAdj) || (curPOS == kAdv) ||
									(curPOS == kInterr) || (curPOS == kInterj) || (curPOS == kVPart) || (curPOS == kQuant) ||
									(curPOS == kIPron) || (curPOS == kRPron) ) 
							//if ( (curPOS == kNoun) )
								{
								vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kContent_Word;
								isContentWord = true;
								}
							else
								isContentWord = false;
			
							break;

						case _Prep_:
							vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kPrep_Start;
							break;
							
						case _Verb_:
							vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kVerb_Start;
							break;
	
						case _Comma_:
						case _Period_:
						case _Quest_:
						case _Exclam_:
							vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kTerm_Bound;
							vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= (vv->opTok->phrasingBND << kSilenceTypeShift);
							vv->end_Punctuation = cur_Phon;
							Store_Phon_In_PhonBuf_1 (vv, _SIL_);
							while (vv->opCount > 1)
								{
								tokStr++, vv->opCount--;				/* skip SIL from Frontend (if one is there)	*/
								}
							word_Initial = true;
							vv->is_Compound_Noun = false;
							gotSentence = true;							/* Got all tokens for one sentence - break-out	*/
							break;
		
						case _Para_:
							vv->start_of_Paragraph_Flag = true;
							break;
		
						case _dInc_:
							vv->user_Dur_Buf1[vv->phonBuf_1_In_Index] = (vv->user_Dur_Buf1[vv->phonBuf_1_In_Index] * vv->durCmdStep) >> kDurStepRes;
							break;
		
						case _dDec_:
							vv->user_Dur_Buf1[vv->phonBuf_1_In_Index] = (vv->user_Dur_Buf1[vv->phonBuf_1_In_Index] << kDurStepRes) / vv->durCmdStep;
							break;
		
						case _pRise_:
							vv->user_Pitch_Buf1[vv->phonBuf_1_In_Index] += vv->pitchCmdStep;
							break;
		
						case _pFall_:
							vv->user_Pitch_Buf1[vv->phonBuf_1_In_Index] -= vv->pitchCmdStep;
							break;
		
						case _Comp_:
							vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kCompoundNoun;
							vv->is_Compound_Noun = true;
							break;
		
						}
					}
					
				else
					/*--------------------------*/
					/* Phon is NOT a CONTROL	*/
					/*--------------------------*/
					{
					if (cur_PhonFlags & kVowelF)
						{
						/*--------------------------*/
						/* Phon is VOWEL			*/
						/*--------------------------*/
						word_Initial = false;
						wordVowel_Index = vv->phonBuf_1_In_Index;
						lastVowel_Index = vv->phonBuf_1_In_Index;
						}
					else
						/*--------------------------*/
						/* Phon is NOT a VOWEL		*/
						/*--------------------------*/
						{
						if (word_Initial)
							vv->phon_Ctrl_Buf_1[vv->phonBuf_1_In_Index] |= kWord_Initial_Consonant;
						}
					
					Store_Phon_In_PhonBuf_1 (vv, cur_Phon);
					}
					
				vv->opCount--;								/* one fewer opcodes in token	*/
				}
			}
		}

	/*--------------------------------------------------*/
	/* If last word was Emph, insert emph stress		*/
	/*  at last 1 or 2 stress or at last vowel in word	*/
	/*--------------------------------------------------*/
	if (wordWasEmph)
		{
		tempIndex = kNoIndex;
		
		if (wordStress_1_Index != kNoIndex)
			tempIndex = wordStress_1_Index;
			
		else if (wordStress_2_Index != kNoIndex)
			tempIndex = wordStress_2_Index;

		else if (wordVowel_Index != kNoIndex)
			tempIndex = wordVowel_Index;
			
		if (tempIndex != kNoIndex)
			{
			vv->phon_Ctrl_Buf_1[tempIndex] &= ~kStressField;
			vv->phon_Ctrl_Buf_1[tempIndex] |= kEmphaticStress;
			}
		}
		
	
	if (wordCount)
		{
		
		if (stress_Counter == 0)
			/*----------------------------------------------*/
			/* If there are no primary or emphatic stresses	*/
			/* in sentence, add one at end.					*/
			/*----------------------------------------------*/
			{
			if (lastStress_2_Index == kNoIndex)
				/*--------------------------------------*/
				/* No secondary stresses in sentence.	*/
				/*--------------------------------------*/
				{
				for (index = last_Word_Index; index < vv->phonBuf_1_In_Index; index++)
					/*--------------------------------------*/
					/* Find 1st vowel of last word.			*/
					/*--------------------------------------*/
					{
					cur_Phon = vv->phon_Buf_1[index];
					if (vv->phonFlags2[cur_Phon] & kVowelF)
						{
						vv->phon_Ctrl_Buf_1[index] |= kPrimaryStress;
						lastStress_1_Index = index;
						break;
						}
					}
				}
			else
				/*--------------------------------------------------*/
				/* Change last secondary stress to primary stress	*/
				/*--------------------------------------------------*/
				{
				vv->phon_Ctrl_Buf_1[lastStress_2_Index] &= ~kStressField;
				vv->phon_Ctrl_Buf_1[lastStress_2_Index] |= kPrimaryStress;
				lastStress_1_Index = lastStress_2_Index;
				}
			}
			
		if (vv->end_Punctuation == _Exclam_)
			{
			/*--------------------------------------------------*/
			/* Change last stress or vowel to emphatic stress	*/
			/*--------------------------------------------------*/
			tempIndex = kNoIndex;
			if (lastStress_1_Index != kNoIndex)
				tempIndex = lastStress_1_Index;
				
			else if (lastStress_2_Index != kNoIndex)
				tempIndex = lastStress_2_Index;
				
			else if (lastVowel_Index != kNoIndex)
				tempIndex = lastVowel_Index;
				
			if (tempIndex != kNoIndex)
				{
				vv->phon_Ctrl_Buf_1[tempIndex] &= ~kStressField;
				vv->phon_Ctrl_Buf_1[tempIndex] |= kEmphaticStress;
				}
			}
		
		Flag_PhonBuf_1 (vv);
		}
	return (wordCount);
}







void	ParseSentence (voiceVarPtr vv)
{
	vv->cmdBufCount = 0;						/* start READING commands from top of queue	*/
	vv->cmdBufCount_Save1 = 0;
	vv->newSentence = true;
	Fill_Phon_Buf_2 (vv);
	(*(vv->funcList->synth_AdjustPhons1_FUNC)) (vv);
	Pitch_RaiseAndFall (vv);
	Mod_Duration (vv);
	(*(vv->funcList->synth_AdjustPhons2_FUNC)) (vv);
	Calc_Ramp_Steps (vv);
	Fill_Pitch_Buf (vv);
	StartNew_PitchClause (vv);

	vv->markerIndex = 0;
	vv->frameMarker = kNoMarker;
}






void	e_Fill_Next_Frame (voiceVarPtr vv)
{
	short	wordCount;
	
	if (vv->speakState == kSpeakNewPhon)
		{
		StartNewPhon (vv);
		(*(vv->funcList->synth_StartNewPhon_FUNC)) (vv);
		vv->speakState = kSpeakPhon;
		vv->starting_New_Phon = true;
		}
	if (vv->speakState == kSpeakPhon)
		{
		Interpolate_Pitch (vv);
		(*(vv->funcList->synth_SpeakPhon_FUNC)) (vv);
		vv->starting_New_Phon = false;
		
		if (vv->dur_Done_in_Phon_CF >= vv->cur_Phon_Dur_CF)
			{
			vv->cur_PhonBuf_Index_CF++;
			if (vv->cur_PhonBuf_Index_CF < vv->phonBuf_2_In_Index)
				{
				vv->speakState = kSpeakNewPhon;					/* start new phon next time	*/
				}
			else
				{
				wordCount = 0;
				while ( !vv->FEinputDone && !(wordCount = Collect_FE_Tokens (vv)) );
				if (wordCount)
					{
					ParseSentence (vv);
					vv->cur_PhonBuf_Index_CF = 0;
					vv->speakState = kSpeakNewPhon;				/* start new phon next time	*/
					}
				else
					{
					vv->speakState = kSpeakLastFrame;
					}
				}
			}
		}
}







void	Start_Talk (voiceVarPtr vv)
{
	vv->speakState = kSpeakNewPhon;
	vv->dur_Done_in_Phon_CF = 0;
	vv->cur_Phon_Dur_CF = 0;
	synth_Start_Talk (vv);

	//(*(vv->funcList->synth_Start_Talk_FUNC)) (vv);
}








short	Talk (voiceVarPtr vv)
{
	short	i;
	short	error;
	short	wordCount;

	error = kNoError;
	for (i = kPhonBufSize-1; i >= 0; --i)
		{
		vv->phon_Buf_1[i] = _SIL_;
		vv->phon_Buf_2[i] = _SIL_;
		vv->phon_Ctrl_Buf_1[i] = 0;
		vv->phon_Ctrl_Buf_2[i] = 0;
		}

	vv->vibrato_Phase1 = 0;
	vv->vibrato_Phase2 = 0;
	vv->lastWordStart = 0;
	vv->FEinputDone = false;
	vv->outputPaused = false;
	vv->VP_baselinePitch = vv->voiceNaturalPitch;

	while ( !vv->FEinputDone && !(wordCount = Collect_FE_Tokens (vv)) );
	if (wordCount )
		{
		ParseSentence (vv);				/* get 1st sentence	*/
		vv->cur_PhonBuf_Index_CF = 0;
		Start_Talk (vv);
		}
	else
		error = kNothingToSpeak;
		
	return (error);
	
}




void	Init_Rate_Params (voiceVarPtr vv)
{

	if (vv->speech_Rate < kMinRate)
		vv->speech_Rate = kMinRate;
	
	/*----------------------------------------------*/
	/* Set 'rate_Ratio':							*/
	/*												*/
	/* set_Speech_Rate: 	120		180		360		*/
	/*						---		---		---		*/
	/* rate_Ratio			1.5		 1		0.5		*/
	/*----------------------------------------------*/
	vv->rate_Ratio = (kNormal_Speech_Rate << 16) / vv->speech_Rate;

	/*----------------------------------------------*/
	/* Set 'rate_Ratio_LowGain':					*/
	/*												*/
	/* set_Speech_Rate: 	120		180		360		*/
	/*						---		---		---		*/
	/* rate_Ratio			1.25	 1		0.63	*/
	/*----------------------------------------------*/
	vv->rate_Ratio_LowGain =  (kNormal_Speech_Rate << 16) / ((((vv->speech_Rate - kNormal_Speech_Rate) * (k1pct * 60)) >> 16) + kNormal_Speech_Rate);

	vv->stress_Duration = (vv->rate_Ratio * vv->stressDurTime) >> 16;	
}




void	Init_Pitch_Params (voiceVarPtr vv)
{
	vv->VP_baselinePitch = vv->voiceNaturalPitch;

	vv->baselineFall_START = kHZ_7 + (vv->VP_baselineFall );
	vv->baselineFall_END = kHZ_7 - (vv->VP_baselineFall);
	
	vv->pFilter_Out1 = vv->baselineFall_START << kStepSizeRes;
	vv->pFilter_Out2 = vv->pFilter_Out1;
	
	vv->pFilter_Out1_Save1 = vv->pFilter_Out1;
	vv->pFilter_Out2_Save1 = vv->pFilter_Out2;
	vv->pFilter_Out1_Save2 = vv->pFilter_Out1;
	vv->pFilter_Out2_Save2 = vv->pFilter_Out2;
	
	vv->pFilter_In_Gain = vv->VP_quickness;
	vv->pFilter_FB_Gain = k100percent - vv->VP_quickness;

	vv->pitch_Clause_StartTime = 10 / kFrameTime;
	
	vv->pitch_Boundry = kNeverHappens;
	vv->low_Gain_CP = false;
}



void	ResetVoice (voiceVarPtr vv)
{
	(*(vv->funcList->synth_ResetVoice_FUNC)) (vv);

	vv->pendingSingPhoneme = 0;
	vv->pendingSingDuration = 0;
	vv->pendingSingNote = 0;

	vv->singing = false;

	vv->user_Volume = 256;				/* Volume = 100%	*/
	(*(vv->funcList->synth_SetVolume_FUNC)) (vv, vv->user_Volume);
	
	Init_Rate_Params (vv);
	Init_Pitch_Params (vv);
}


short	NewVoice (voiceVarPtr vv, void *vDat, unsigned char *sample)
{
	short			error;

	error = (*(vv->funcList->synth_NewVoice_FUNC)) (vv, vDat, sample);

	if (error == kNoError)
		ResetVoice (vv);
		
	return (error);
}








