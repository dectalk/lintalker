#ifndef __SPEECHSAY__
	#include "MT4.h"
#endif

#ifndef __FSYNTH__
	#include "Fsynth.h"
#endif



//---------------------------
// Data.c
//---------------------------
extern		short			CosTbl[];
extern		short			BcoeffTbl[];
extern		short			CcoeffTbl[];

extern		int32_t			TopOctave[];
extern		unsigned long	PhonFlags2[];
extern		unsigned long	One_Over_X_Tbl[];
extern		short			CtrlBlockTypeTbl[];
extern		short			PhonTypeTbl[];
extern		short			LogToLin[];
extern		short			LogToLin1[];
extern		short			logOf2Tbl[];
extern		unsigned short	ExpOf2Tbl[];
extern		short			OctFreqTbl[];
extern		short			SineWave15[];
extern		char			SineWave[];
extern		char			NoiseWave[];
extern		char			BandNoise[];
extern		char			HPNoise[];
extern		short			phonPitchTbl[];
extern		short			Male_Loci_Tbl[];
extern		short			Female_Loci_Tbl[];
extern		short			Front_Loci_Tbl[];
extern		short			Mid_Loci_Tbl[];
extern		short			Back_Loci_Tbl[];
extern		short			NoiseIndexTbl[];
extern		short			Male_NoiseAmpTbl[];
extern		short			Female_NoiseAmpTbl[];
extern		short			Rank_FWD_Tbl[];
extern		short			Rank_BKWD_Tbl[];
extern		short			DefaultTargTbl[];
extern		short			MaxDurTbl[];
extern		short			MinDurTbl[];
extern		short			BurstDurTbl[];
extern		short			XlateToAllo[];
extern		short			MaleEnvTbl[];
extern		short			FemaleEnvTbl[];
extern		short			f1FreqTblM[];
extern		short			f2FreqTblM[];
extern		short			f3FreqTblM[];
extern		short			b1FreqTblM[];
extern		short			b2FreqTblM[];
extern		short			b3FreqTblM[];
extern		short			avVolTblM[];
extern		short			f1FreqTblF[];
extern		short			f2FreqTblF[];
extern		short			f3FreqTblF[];
extern		short			b1FreqTblF[];
extern		short			b2FreqTblF[];
extern		short			b3FreqTblF[];
extern		short			avVolTblF[];
extern		short			BoundryDur[];


/*	BackEnd.c	*/
void	e_Fill_Next_Frame (voiceVarPtr vv);


/*	Say.c	*/

extern void		InitSay (voiceVarPtr vv);
extern void		Start_The_Speech (voiceVarPtr vv);
extern void		SaveFrame (voiceVarPtr vv);
extern void		InsertSample (voiceVarPtr vv, voiceDataPtr tvPtr);

extern void		synth_ResetVoice (voiceVarPtr vv);
extern short	synth_NewVoice (voiceVarPtr vv, voiceDataPtr vDat, unsigned char *sample);
extern void 	synth_SetVolume (voiceVarPtr zz, short vol);
extern void		synth_FillNextSampBuffer (voiceVarPtr vv);


/* Forward declarations	*/

void	synth_Init (voiceVarPtr vv);
void	synth_Start_Talk (voiceVarPtr vv);
void	synth_SpeakPhon (voiceVarPtr vv);
void	synth_StartNewPhon (voiceVarPtr vv);
void	synth_AdjustPhons1 (voiceVarPtr vv);
void	synth_AdjustPhons2 (voiceVarPtr vv);

void	fSynth_GetVoiceParams (voiceVarPtr vv, void *info );
void	fSynth_SetVoiceParams (voiceVarPtr vv, void *info );

void	fSynth_SetSoundSample (voiceVarPtr vv, unsigned char **info );
void	fSynth_GetVoiceTables (voiceVarPtr vv, unsigned long **info );
void	fSynth_SetVoiceTables (voiceVarPtr vv, unsigned long **info );
void	fSynth_GetGlotGain (voiceVarPtr vv, short *info );
void	fSynth_SetGlotGain (voiceVarPtr vv, short *info );
void	fSynth_GetSampleGain (voiceVarPtr vv, short *info );
void	fSynth_SetSampleGain (voiceVarPtr vv, short *info );
void	fSynth_GetSamplePitch (voiceVarPtr vv, short *info );
void	fSynth_SetSamplePitch (voiceVarPtr vv, short *info );
void	fSynth_GetVoiceVars (voiceVarPtr vv, unsigned long *info );

void	SetFormAddr (voiceVarPtr vv);




//---------------------------
// Export
//---------------------------
	void	Init_FSynthFuncPtrs (moduleFuncPtr mfp);


void	Init_FSynthFuncPtrs (moduleFuncPtr mfp)
{
	mfp->synth_Init_FUNC				= (synth_Init_Ptr) &synth_Init;
	mfp->synth_FillNextSampBuffer_FUNC	= (synth_FillNextSampBuffer_Ptr) &synth_FillNextSampBuffer;
	mfp->synth_ResetVoice_FUNC			= (synth_ResetVoice_Ptr) &synth_ResetVoice;
	mfp->synth_NewVoice_FUNC			= (synth_NewVoice_Ptr) &synth_NewVoice;
	mfp->synth_SetVolume_FUNC			= (synth_SetVolume_Ptr) &synth_SetVolume;
	mfp->synth_Start_Talk_FUNC			= (synth_Start_Talk_Ptr) &synth_Start_Talk;
	mfp->synth_SpeakPhon_FUNC			= (synth_SpeakPhon_Ptr) &synth_SpeakPhon;
	mfp->synth_StartNewPhon_FUNC		= (synth_StartNewPhon_Ptr) &synth_StartNewPhon;
	mfp->synth_AdjustPhons1_FUNC		= (synth_AdjustPhons1_Ptr) &synth_AdjustPhons1;
	mfp->synth_AdjustPhons2_FUNC		= (synth_AdjustPhons2_Ptr) &synth_AdjustPhons2;

	mfp->fSynth_GetVoiceParams_FUNC	 	= (fSynth_GetVoiceParams_Ptr) &fSynth_GetVoiceParams;
	mfp->fSynth_SetVoiceParams_FUNC	 	= (fSynth_SetVoiceParams_Ptr) &fSynth_SetVoiceParams;
	mfp->fSynth_SetSoundSample_FUNC	 	= (fSynth_SetSoundSample_Ptr) &fSynth_SetSoundSample;
	mfp->fSynth_GetVoiceTables_FUNC	 	= (fSynth_GetVoiceTables_Ptr) &fSynth_GetVoiceTables;
	mfp->fSynth_SetVoiceTables_FUNC	 	= (fSynth_SetVoiceTables_Ptr) &fSynth_SetVoiceTables;

	mfp->fSynth_GetGlotGain_FUNC	 	= (fSynth_GetGlotGain_Ptr) &fSynth_GetGlotGain;
	mfp->fSynth_SetGlotGain_FUNC	 	= (fSynth_SetGlotGain_Ptr) &fSynth_SetGlotGain;
	mfp->fSynth_GetSampleGain_FUNC	 	= (fSynth_GetSampleGain_Ptr) &fSynth_GetSampleGain;
	mfp->fSynth_SetSampleGain_FUNC	 	= (fSynth_SetSampleGain_Ptr) &fSynth_SetSampleGain;
	mfp->fSynth_GetSamplePitch_FUNC	 	= (fSynth_GetSamplePitch_Ptr) &fSynth_GetSamplePitch;
	mfp->fSynth_SetSamplePitch_FUNC	 	= (fSynth_SetSamplePitch_Ptr) &fSynth_SetSamplePitch;
	mfp->fSynth_GetVoiceVars_FUNC	 	= (fSynth_GetVoiceVars_Ptr) &fSynth_GetVoiceVars;
}




Ptr		GetThePtr (Ptr basePtr, long *tblPtr)
{
	Ptr		dataPtr;
	long	offset;

	offset = *tblPtr;
	dataPtr = basePtr + offset;
	return (dataPtr);
}



void	SetFormAddr (voiceVarPtr vv)
{
	long			*tblPtr;
	Ptr				basePtr;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	tblPtr = zz->FormTables;
	basePtr = (Ptr)zz->FormTables;
	zz->a_EnvelopeListTbl 	= (short*)GetThePtr(basePtr, tblPtr++);
	zz->a_f1FreqTblM 		= (short*)GetThePtr(basePtr, tblPtr++);
	zz->a_f2FreqTblM 		= (short*)GetThePtr(basePtr, tblPtr++);
	zz->a_f3FreqTblM 		= (short*)GetThePtr(basePtr, tblPtr++);
	zz->a_b1FreqTblM 		= (short*)GetThePtr(basePtr, tblPtr++);
	zz->a_b2FreqTblM 		= (short*)GetThePtr(basePtr, tblPtr++);
	zz->a_b3FreqTblM 		= (short*)GetThePtr(basePtr, tblPtr++);
	zz->a_avVolTblM 		= (short*)GetThePtr(basePtr, tblPtr++);
}


void	SetFormTables (voiceVarPtr vv, unsigned long *tblPtr)
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	zz->a_EnvelopeListTbl 	= (short*) *tblPtr++;
	zz->a_f1FreqTblM 		= (short*) *tblPtr++;
	zz->a_f2FreqTblM 		= (short*) *tblPtr++;
	zz->a_f3FreqTblM 		= (short*) *tblPtr++;
	zz->a_b1FreqTblM 		= (short*) *tblPtr++;
	zz->a_b2FreqTblM 		= (short*) *tblPtr++;
	zz->a_b3FreqTblM 		= (short*) *tblPtr++;
	zz->a_avVolTblM 		= (short*) *tblPtr++;
}


void	GetFormTables (voiceVarPtr vv, unsigned long *tblPtr)
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	*tblPtr++ = (unsigned long) zz->a_EnvelopeListTbl;
	*tblPtr++ = (unsigned long) zz->a_f1FreqTblM;
	*tblPtr++ = (unsigned long) zz->a_f2FreqTblM;
	*tblPtr++ = (unsigned long) zz->a_f3FreqTblM;
	*tblPtr++ = (unsigned long) zz->a_b1FreqTblM;
	*tblPtr++ = (unsigned long) zz->a_b2FreqTblM;
	*tblPtr++ = (unsigned long) zz->a_b3FreqTblM;
	*tblPtr++ = (unsigned long) zz->a_avVolTblM;
}





void	SetTblAddr (voiceVarPtr vv)
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	//zz->IntervalVoices 		= (voiceData*)GetThePtr(basePtr, tblPtr++);

	zz->CosTblPtr 			= (rShort*)&CosTbl;
	zz->BcoeffTblPtr 		= (rShort*)&BcoeffTbl;
	zz->CcoeffTblPtr 		= (rShort*)&CcoeffTbl;
	zz->SineWave15Ptr 		= (rShort*)&SineWave15;
	zz->NoiseWavePtr		= (rUSC*)&NoiseWave;
	zz->BandNoisePtr 		= (rUSC*)&BandNoise;
	zz->HPNoisePtr	 		= (rUSC*)&HPNoise;

	zz->One_Over_X_Tbl 		= (long*)&One_Over_X_Tbl;
	zz->TOPtr 				= (Fixed*)&TopOctave;
	zz->CtrlBlockTypeTbl 	= (short*)&CtrlBlockTypeTbl;
	zz->Male_Loci_Tbl 		= (short*)&Male_Loci_Tbl;
	zz->Female_Loci_Tbl 	= (short*)&Female_Loci_Tbl;
	zz->Front_Loci_Tbl 		= (short*)&Front_Loci_Tbl;
	zz->Mid_Loci_Tbl 		= (short*)&Mid_Loci_Tbl;
	zz->Back_Loci_Tbl 		= (short*)&Back_Loci_Tbl;
	zz->NoiseIndexTbl 		= (short*)&NoiseIndexTbl;
	zz->Male_NoiseAmpTbl 	= (short*)&Male_NoiseAmpTbl;
	zz->Female_NoiseAmpTbl 	= (short*)&Female_NoiseAmpTbl;
	zz->Rank_FWD_Tbl 		= (short*)&Rank_FWD_Tbl;
	zz->Rank_BKWD_Tbl 		= (short*)&Rank_BKWD_Tbl;
	zz->DefaultTargTbl 		= (short*)&DefaultTargTbl;
	zz->BurstDurTbl 		= (short*)&BurstDurTbl;
	zz->MaleEnvelopeListTbl = (short*)&MaleEnvTbl;
	zz->FemaleEnvelopeListTbl 	= (short*)&FemaleEnvTbl;
	zz->f1FreqTblM 			= (short*)&f1FreqTblM;
	zz->f2FreqTblM 			= (short*)&f2FreqTblM;
	zz->f3FreqTblM 			= (short*)&f3FreqTblM;
	zz->b1FreqTblM 			= (short*)&b1FreqTblM;
	zz->b2FreqTblM 			= (short*)&b2FreqTblM;
	zz->b3FreqTblM 			= (short*)&b3FreqTblM;
	zz->avVolTblM 			= (short*)&avVolTblM;
	zz->f1FreqTblF 			= (short*)&f1FreqTblF;
	zz->f2FreqTblF 			= (short*)&f2FreqTblF;
	zz->f3FreqTblF 			= (short*)&f3FreqTblF;
	zz->b1FreqTblF 			= (short*)&b1FreqTblF;
	zz->b2FreqTblF 			= (short*)&b2FreqTblF;
	zz->b3FreqTblF 			= (short*)&b3FreqTblF;
	zz->avVolTblF 			= (short*)&avVolTblF;
}




void	synth_Init (voiceVarPtr vv)
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	
	SetTblAddr (vv);
	zz->delayBuffer = (rShort*)vv->reverbBufPtr;
	zz->SampleWave = NULL;
	zz->FormTables = NULL;
	vv->synthTech = kFormantSynth;
	//NewVoice (vv, &zz->IntervalVoices[0]);	
}


void	Init_ControlBlocks (formantVarPtr zz)
{
	ControlBlock	*cb;
	short			i;

	zz->big_Bang = true;
	for (i = kF1; i < kNumOfBlocks; i++)
		{
		cb = (ControlBlock*) &(zz->controlBlockArray[i]);
		
		cb->curP_START_Targ = 0;
		cb->curTarget_TIME = 0;
		cb->curTarget_STEP = 0;
		cb->curTarget_OFFS = 0;
		cb->HEAD_offs = 0;
		cb->HEAD_step = 0;
		cb->TAIL_offs = 0;
		cb->TAIL_step = 0;
		cb->TAIL_START_time = 0;
		cb->onset_END_TIME = 0;
		cb->onset_VAL = 0;
		cb->nextP_START_Targ = 0;
		cb->prevP_END_Targ = 0;
		cb->curP_END_Targ = 0;
		cb->ptrToTargetList = NULL;
		cb->lastVal = 0;
		}

}




void	Insert_Closure_Release (voiceVarPtr vv)
{
	short		i, index;
	short		cur_Phon;
	long		cur_Flags;
	short		prev_Phon;
	short		next_Phon;
	short		src,dest;

	for (i = 0; i < vv->phonBuf_2_In_Index; i++)
		{
		/*----------*/
		/* cur		*/
		/*----------*/
		cur_Phon = (*(vv->funcList->e_GetPhon_FUNC)) (vv, i);
		cur_Flags = vv->phonFlags2[cur_Phon];

		/*----------*/
		/* prev		*/
		/*----------*/
		prev_Phon = (*(vv->funcList->e_GetPhon_FUNC)) (vv, i-1);

		/*----------*/
		/* next		*/
		/*----------*/
		next_Phon = (*(vv->funcList->e_GetPhon_FUNC)) (vv, i+1);

		if (next_Phon == _SIL_)
			{
			if ( 	(cur_Flags & kHasReleaseF) &&
					(vv->phonBuf_2_In_Index < kPhonBuf_Red_Zone) )
				{
				/*----------------------------------*/
				/* Make room for plosive release	*/
				/*----------------------------------*/
				for (index = vv->phonBuf_2_In_Index; index > i; index--)
					{
					src = index-1;
					dest = index;
					vv->phon_Buf_2[dest] = vv->phon_Buf_2[src];
					vv->phon_Ctrl_Buf_2[dest] = vv->phon_Ctrl_Buf_2[src];
					vv->dur_Buf[dest] = vv->dur_Buf[src];

					vv->user_Cmd_Buf2[dest] = vv->user_Cmd_Buf2[src];
					vv->user_Pitch_Buf2[dest] = vv->user_Pitch_Buf2[src];
					vv->user_Dur_Buf2[dest] = vv->user_Dur_Buf2[src];
					vv->user_Note_Buf2[dest] = vv->user_Note_Buf2[src];
					vv->user_Rate_Buf2[dest] = vv->user_Rate_Buf2[src];
					}
				
				/*----------------------*/
				/* Flag it as RELEASE	*/
				/*----------------------*/
				vv->phon_Ctrl_Buf_2[i+1] = vv->phon_Ctrl_Buf_2[i] | kPlosive_Release;
				
				i++;								/* point to RELEASE phon		*/
				vv->phonBuf_2_In_Index++;			/* account for added release	*/
				
				/*------------------------------------------*/
				/* Decide if voiced release is AX or IX		*/
				/*------------------------------------------*/
				if ( (vv->phonFlags2[prev_Phon] & kFrontF) ||
					(cur_Phon == _t_) || (cur_Phon == _d_) )
					{
					vv->phon_Buf_2[i] = _IX_;
					}
				else
					{
					vv->phon_Buf_2[i] = _AX_;
					}
				
				vv->dur_Buf[i] = 25 / kFrameTime;
				vv->user_Cmd_Buf2[i] = 0;
				vv->user_Pitch_Buf2[i] = vv->user_Pitch_Buf2[i-1];
				vv->user_Dur_Buf2[i] = kDur_One;
				vv->user_Note_Buf2[i] = 0;
				vv->user_Rate_Buf2[i] = 0;
				}
			}
		}
}





void	Insert_Burst (voiceVarPtr vv)
{
	short			burstClosureDur, burstReleaseDur, burstDur;
	short			i;
	ControlBlock	*cb;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
		
	/****************************************/
	/* Pre-burst closure if cur =  PLOSIVE	*/
	/****************************************/
	
	if (vv->cur_PhonFlags_CF & kPlosiveF)
		{
		burstDur = zz->BurstDurTbl[vv->cur_Phon_CF] / kFrameTime;
		
		/*----------------------------------*/
		/* No burst if next is nasal		*/
		/* or stop with similar position	*/
		/*----------------------------------*/
		if ( (vv->cur_PhonFlags_CF & kStopF) && !(vv->cur_PhonFlags_CF & kVoicedF) )
			{
			#if 0
			if (vv->next_PhonFlags_CF & kNasalF) 
				{
				burstDur >>= 1;						/* @@@@ */
				}
			#endif
	
			//else if ((vv->next_PhonFlags_CF & kStopF) && !(vv->next_PhonFlags_CF & kVoicedF) ) 
			if (vv->next_PhonFlags_CF & (kStopF+kNasalF)) 
				{
				if (vv->next_PhonCtrl_CF & kPrimOrEmphStress)
					burstDur = 0;						/* @@@@ */
				else
					burstDur >>= 1;						/* @@@@ */
				}
			}

		/*-----------------------------------*/
		/* Shorten burst if STOP -> PlosFric */
		/*				or					 */
		/*      if phon dur is short		 */
		/*-----------------------------------*/
		if (burstDur > 1)
			{
			if (	((vv->cur_PhonFlags_CF & kStopF) && (vv->next_PhonFlags_CF & kPlosFricF)) || 
					(vv->cur_Phon_Dur_CF < 50 / kFrameTime) ) 
				{
				//burstDur--;
				}
			}
		
		burstClosureDur = vv->cur_Phon_Dur_CF - burstDur;
		
		/*------------------------------*/
		/* Limit closure for affricate	*/
		/* to 80 ms						*/
		/*------------------------------*/
		if ( 	(vv->cur_PhonFlags_CF & kAffricateF) && 
				(burstClosureDur > 80 / kFrameTime) )
			{
			burstClosureDur = 80 / kFrameTime;
			}
		
		for (i = kAp2; i <= kAB; i++)
			{
			cb = (ControlBlock*) &(zz->controlBlockArray[i]);
			cb->onset_END_TIME = burstClosureDur;
			cb->onset_VAL = 0;
			}
		}
	
	
	/************************************/
	/* If voiocless stop -> SONORANT	*/
	/*  add unvoiced aspiration after	*/
	/*  burst.							*/
	/************************************/

	burstReleaseDur = 0;

	if ( (vv->prev_PhonFlags_CF & kStopF) && !(vv->prev_PhonFlags_CF & kVoicedF) &&
			(vv->cur_PhonFlags_CF & kSonorant1F) )
		{
		burstReleaseDur = 40 / kFrameTime;					/* burst release intrudes on sonorant	*/
		zz->controlBlockArray[kAV].onset_VAL = 0;			/* no voicing during release	*/

		if (zz->Rank_FWD_Tbl[vv->next_Phon_CF] == kFrontR)
			/*--------------------------*/
			/* cur = FRONT				*/
			/*--------------------------*/
			zz->controlBlockArray[kAF].onset_VAL = 48;		/* @@@ was 54, less aspiration before front	*/
		else
			/*--------------------------*/
			/* cur � FRONT				*/
			/*--------------------------*/
			zz->controlBlockArray[kAF].onset_VAL = 54;		/* @@@@ was 61	*/
		
		if ( !(vv->cur_PhonCtrl_CF & kVowelF) )
			/*--------------------------*/
			/* cur � VOWEL				*/
			/*--------------------------*/
			{
			burstReleaseDur = 25 / kFrameTime;				/* shorter release and...	*/
			zz->controlBlockArray[kAF].onset_VAL -= 3;		/* ...less aspiration if NOT vowel	*/
			}
		
		if ( (vv->cur_PhonCtrl_CF & kLiqGlideF) || (vv->cur_Phon_CF == _ER_) )
			/*--------------------------*/
			/* cur = LiqGlide or ER		*/
			/*--------------------------*/
			zz->controlBlockArray[kAF].onset_VAL += 3;
		
		if (vv->prev2_Phon_CF == _s_)
			{
			if ( !(vv->prev2_PhonCtrl_CF & kSyllableTypeField) )
				burstReleaseDur = 10 / kFrameTime;				/* very short release if non-word-initial s-cluster	*/
			}
		else
			{
			if ( !(vv->cur_PhonCtrl_CF & kVowelF) )
				burstReleaseDur += 20 / kFrameTime;
			}
		
		if (burstReleaseDur >= vv->cur_Phon_Dur_CF)
			burstReleaseDur = vv->cur_Phon_Dur_CF - 1;
		
		if ( burstReleaseDur > (vv->cur_Phon_Dur_CF >> 1) )
			{
			if ( (vv->cur_PhonFlags_CF & kVowelF) && (vv->cur_PhonCtrl_CF & kPrimOrEmphStress) )
				burstReleaseDur = vv->cur_Phon_Dur_CF >> 1;
			}
		
		if (vv->cur_PhonCtrl_CF & kPlosive_Release)
			{
			burstReleaseDur = vv->cur_Phon_Dur_CF;
			//zz->controlBlockArray[kAF].onset_VAL -= 20;			/* @@@@ was 3	*/
			zz->controlBlockArray[kAF].onset_VAL = 0;			/* @@@@ was 3	*/
			}
			
		zz->controlBlockArray[kAV].onset_END_TIME = burstReleaseDur;
		zz->controlBlockArray[kAF].onset_END_TIME = burstReleaseDur;
		zz->controlBlockArray[kBW1].onset_END_TIME = burstReleaseDur;
		zz->controlBlockArray[kBW2].onset_END_TIME = burstReleaseDur;
		
		zz->controlBlockArray[kBW1].onset_VAL = zz->controlBlockArray[kBW1].curP_START_Targ + 250;
		zz->controlBlockArray[kBW2].onset_VAL = zz->controlBlockArray[kBW2].curP_START_Targ + 70;
		}
		
		
	if ( (vv->cur_PhonFlags_CF & kStopF) && (vv->cur_PhonFlags_CF & kVoicedF) &&
			(vv->prev_PhonFlags_CF & kVoicedF) && !(vv->next_PhonFlags_CF & kVoicedF) &&
			(vv->cur_Phon_CF != _TX_))
		/*----------------------------------*/
		/* Voicing for voiced plosive		*/
		/*----------------------------------*/
		{
		zz->controlBlockArray[kAV].onset_END_TIME = vv->cur_Phon_Dur_CF - (10 / kFrameTime);
		zz->controlBlockArray[kBW1].onset_END_TIME = vv->cur_Phon_Dur_CF;
		zz->controlBlockArray[kBW2].onset_END_TIME = vv->cur_Phon_Dur_CF;
		zz->controlBlockArray[kBW3].onset_END_TIME = vv->cur_Phon_Dur_CF;

		zz->controlBlockArray[kAV].onset_VAL = 53;
		zz->controlBlockArray[kBW1].onset_VAL = 1000;
		zz->controlBlockArray[kBW2].onset_VAL = 1000;
		zz->controlBlockArray[kBW3].onset_VAL = 1200;
		}
}






short 	Adjust_Colored_Target (voiceVarPtr vv, short index, short entryCount)
{
	short			cur_Phon, next_Phon, prev_Phon;
	long			cur_Flags, next_Flags, prev_Flags, cur_PhonCtrl;
	short			adjust;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	cur_Phon = (*(vv->funcList->e_GetPhon_FUNC)) (vv, index);
	next_Phon = (*(vv->funcList->e_GetPhon_FUNC)) (vv, index+1);
	prev_Phon = (*(vv->funcList->e_GetPhon_FUNC)) (vv, index-1);
	
	cur_Flags = vv->phonFlags2[cur_Phon];
	next_Flags = vv->phonFlags2[next_Phon];
	prev_Flags = vv->phonFlags2[prev_Phon];
	adjust = 0;

	if (zz->cur_ControlBlk_Index == kF3)
		/************************/
		/*		F3				*/
		/************************/
		{
		if ( (cur_Flags & kVowel1F) && (cur_Phon != _ER_) &&
				((prev_Flags & kLiqGlide2F) || (next_Flags & kLiqGlide2F)) )	/* w, r, RX	*/
			{
			adjust = -150;
			}
		}

	else if (zz->cur_ControlBlk_Index == kF2)
		/************************/
		/*		F2				*/
		/************************/
		{
		cur_PhonCtrl = vv->phon_Ctrl_Buf_2[index];
		
		/*------------------------------*/
		/* L-colored vowels				*/
		/*------------------------------*/
		if (next_Phon == _LX_)
			{
			if (cur_Flags & kFrontF)
				/*------------------------------*/
				/* FRONT -> lx					*/
				/*------------------------------*/
				{
				adjust = -150;				
				}
				
			else if ( ((cur_Phon == _AY_) || (cur_Phon == _OY_)) && (entryCount > 0) )
				/*------------------------------*/
				/* 'y' of ay oy -> LX			*/
				/*------------------------------*/
				{
				adjust = -250;
				}
			}
			
		if ( ((prev_Phon == _LX_) || (prev_Phon == _l_) || (prev_Phon == _w_)) && (cur_Flags & kFrontF) )
			/*------------------------------*/
			/* w,l lx -> FRONT				*/
			/*------------------------------*/
			{
			adjust = -150;	
			}
			
		if ( (cur_Phon == _UW_) && (prev_Flags & kAlveolarF) )
			/*------------------------------*/
			/* alveolar -> _UW_				*/
			/*------------------------------*/
			{
			adjust = 200;	
			}

		if ( (entryCount > 0) && ((cur_Phon == _UW_) || (cur_Phon == _YU_)) && (next_Flags & kAlveolarF) )
			/*----------------------------------*/
			/* 'w' of _UW_, _YU_ -> alveolar	*/
			/*----------------------------------*/
			{
			adjust += 200;	
			}
			
		//#if 0
		if (cur_PhonCtrl & kStressField)
			/*------------------------------------------*/
			/* STRESSED phons at clause boundries are 	*/
			/* articulated more clearly (less coloring)	*/
			/*------------------------------------------*/
			{
			//if ( (cur_PhonCtrl & kSyllableTypeField) >= kPrep_End )		/* @@@@	*/
				adjust = adjust >> 1;						/* 50%	*/
			}
		else
		//#endif
			/*--------------------------------------*/
			/* UNSTRESSED phons have more coloring	*/
			/*--------------------------------------*/
			{
			adjust += adjust >> 1;							/* 150%	*/
			//adjust = adjust >> 1;							/* 50%	*/
				
			if ( (entryCount > 0) && (cur_Phon == _YU_) )
				adjust = 400;								/* @@@@ */
			}
		
		if (adjust > 400)
			adjust = 400;
		else if (adjust < (-400))
			adjust = (-400);
		}
		
		return (adjust);
}




short	GetTarget (voiceVarPtr vv, short index)
{
	ControlBlock	*cb;
	short	cur_ControlBlk_Type;
	short	cur_phon;
	long	cur_Flags;
	long	cur_PhonCtrl;
	short	next_phon;
	long	next_Flags;
	short	prev_phon;
	long	prev_Flags;
	short	target_Val;
	short	*targetPtr;
	short	rank;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;	
	cb = (ControlBlock*) &(zz->controlBlockArray[zz->cur_ControlBlk_Index]);
	cur_ControlBlk_Type = zz->CtrlBlockTypeTbl[zz->cur_ControlBlk_Index];

	cur_phon = (*(vv->funcList->e_GetPhon_FUNC)) (vv, index);
	cur_Flags = vv->phonFlags2[cur_phon];
	cur_PhonCtrl = vv->phon_Ctrl_Buf_2[index];
	next_phon = (*(vv->funcList->e_GetPhon_FUNC)) (vv, index+1);
	next_Flags = vv->phonFlags2[next_phon];
	prev_phon = (*(vv->funcList->e_GetPhon_FUNC)) (vv, index-1);
	prev_Flags = vv->phonFlags2[prev_phon];
	
	target_Val = -1;

	/*--------------------------*/
	/* F1 - BW3					*/
	/*--------------------------*/
	if ( (cur_ControlBlk_Type == kFreqType) || (cur_ControlBlk_Type == kBWType) )
		{
		targetPtr = zz->voice_Formants[zz->cur_ControlBlk_Index];
		target_Val = targetPtr[cur_phon];
		if (target_Val < kNoValue)
			goto ReturnWithTarget;
		if (target_Val < 0)
			{
			if (target_Val == kNoValue)
				/*--------------------------*/
				/* use n+1 phon's target	*/
				/*--------------------------*/
				{
				target_Val = targetPtr[next_phon];
				if (target_Val == kNoValue)
					/*--------------------------*/
					/* use n+2 phon's target	*/
					/*--------------------------*/
					{
					target_Val = targetPtr[(*(vv->funcList->e_GetPhon_FUNC)) (vv, index+2)];
					if (target_Val == kNoValue)
						/*--------------------------*/
						/* use n-1 phon's target	*/
						/*--------------------------*/
						{
						target_Val = targetPtr[prev_phon];
						if ( (target_Val < 0) && (target_Val != kNoValue) )
							{
							/*--------------------------*/
							/* Get last target in list	*/
							/*--------------------------*/
							target_Val = zz->EnvelopeListTbl[(target_Val & 0x7FFF) +2];
							}
							
						if (target_Val == kNoValue)
							{
							target_Val = zz->DefaultTargTbl[zz->cur_ControlBlk_Index];
							}
						}
					}
				}
			if (target_Val < kNoValue)
				{
				target_Val &= 0x7FFF;							/* strip flag bit	*/
				target_Val = zz->EnvelopeListTbl[target_Val];	/* get 1st entry	*/
				}
				
			if (zz->cur_ControlBlk_Index == kF1)
				{
				if ( (cur_Flags & kPlosFricF) && !(cur_Flags & kObstF) && (prev_Flags & kVowelF) )
					/*------------------------------*/
					/* vowel -> FRIC				*/
					/*------------------------------*/
					target_Val += 40;
				}

			if ( ((cur_phon == _n_) || (cur_phon == _EN_)) &&  (zz->cur_ControlBlk_Index == kBW2))	/* @@@@	*/
				{
				if ( (zz->Rank_FWD_Tbl[next_phon]) != kFrontR )
					target_Val += 60;
				
			if ( ((cur_phon == _n_) || (cur_phon == _EN_) || (cur_phon == _NG_)) 
					&&  (zz->cur_ControlBlk_Index == kBW3) )
					{
					if ( (next_Flags & kYGlideStartF) || (prev_Flags & kYGlideEndF) )
						{
						target_Val = kMaxBandWidth;
						}
					}
				}
			
			}
		}

	/*--------------------------*/
	/* FNZ						*/
	/*--------------------------*/
	else if (cur_ControlBlk_Type == kFNZType)
		{
		if (cur_Flags & kNasalF)
			target_Val = zz->nasalTargFreq;
		else
			target_Val = zz->nasalBaseFreq;
		}

	/*--------------------------*/
	/* AV, AF					*/
	/*--------------------------*/
	else if (cur_ControlBlk_Type == kSourceAmpType)
		{
		if (zz->cur_ControlBlk_Index == kAV)
			/*------*/
			/* AV	*/
			/*------*/
			{
			target_Val = zz->voice_av_Tbl[cur_phon];
			
			if (cur_PhonCtrl & kPlosive_Release)
				{
			if (prev_Flags & kNasalF)
				target_Val -= 6;					/* lower nasal release	*/
			else
				target_Val -= 20;					/* plosive -> release is even lower	*/
				}
			
			if ( (cur_Flags & kStopF) && !(prev_Flags & kVoicedF) )
				target_Val = 0;					/* no voicing in stop if  unvoiced -> STOP	*/
			
			if ( (cur_phon == _h_)  && (prev_Flags & kVoicedF) && !(cur_PhonCtrl & kPrimOrEmphStress) )
				target_Val = 54;				/* voice H if proceeded by voiced phon	*/
			}
			
		else if (cur_phon == _h_)				/* H is the ONLY aspirated phon	*/
			/*------*/
			/* AF	*/
			/*------*/
			{
			if (zz->Rank_FWD_Tbl[next_phon] == kFrontR)
				target_Val = 58;			/* weaker aspiration if followed by front	*/
			else
				target_Val = 62;

			if ( !(cur_PhonCtrl & kStressField) )
				target_Val -= 1;					/* unstressed is quieter	*/
			}
			
		else
			target_Val = 0;
		
		}

	/*--------------------------*/
	/* A2 - AB					*/
	/*--------------------------*/
	else if (cur_ControlBlk_Type == kResonAmpType)
		{
		target_Val = zz->NoiseIndexTbl[cur_phon];
		
		if (target_Val == kNoValue)
			{
			target_Val = 0;
			}
		else
			{
			if (next_phon == _SIL_)
				rank = zz->Rank_BKWD_Tbl[prev_phon];
			else
				rank = zz->Rank_FWD_Tbl[next_phon];
			
			if (rank == kRoundR)
				rank = kBackR;
			
			target_Val += (zz->cur_ControlBlk_Index - kAp2) + (rank * 6);
			target_Val = zz->voice_NoiseAmp_Tbl[target_Val];
			
			if ( (vv->phon_Ctrl_Buf_2[index+1] & kPlosive_Release) && (target_Val >= 4) )
				target_Val -= 4;					/* lower amp if next is plosive release	*/
			

			//else if ( (cur_Flags & kPlosFricF) && !(cur_Flags & kPlosiveF) && (target_Val >= 20) )
			//	target_Val -= 4;					/* lower amp if cur is fricative @@@@	*/
			
			}
		}
ReturnWithTarget:
	return (target_Val);
}






short	Get_FIRST_Target (voiceVarPtr vv, short index)
{
	short		targ;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;	
	targ = GetTarget (vv, index);
	
	if (targ < kNoValue)
		/*----------------------------------*/
		/* Get target from FIRST entry		*/
		/* in 'EnvelopeListTbl'				*/
		/*----------------------------------*/
		{
		targ &= 0x7FFF;					/* strip flag bit	*/
		targ = zz->EnvelopeListTbl[targ];
		
		if (zz->CtrlBlockTypeTbl[zz->cur_ControlBlk_Index] == kFreqType)
			targ += Adjust_Colored_Target (vv, index, 0);
		}
		
	return (targ);
}




short	Get_LAST_Target (voiceVarPtr vv, short index)
{
	short		targ;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;	
	targ = GetTarget (vv, index);
	
	if (targ < kNoValue)
		/*----------------------------------*/
		/* Get target from LAST entry		*/
		/* in 'EnvelopeListTbl'				*/
		/*----------------------------------*/
		{
		targ = zz->EnvelopeListTbl[(targ & 0x7FFF) +2];
		
		if (zz->CtrlBlockTypeTbl[zz->cur_ControlBlk_Index] == kFreqType)
			targ += Adjust_Colored_Target (vv, index, 1);
		}
		
	return (targ);
}





void	Get_Locus (voiceVarPtr vv, short i_Consonant, short i_Vowel, short bType)
{
	long		con_Flags, v1_Flags;
	short		consonant_Phon;
	short		vowel1_Phon;
	short		consonant_Rank;
	short		vowel_Rank;
	short		v1_Target;
	short		loci_Tbl_Index;
	short		locus_Freq;
	short		locus_Pcnt;
	short		target_Offset;
	short		f2_y_Colored;
	formantVarPtr 	zz;


	zz = (formantVarPtr)vv->synthVars;	
	if ( (zz->cur_ControlBlk_Index >= kF1) && (zz->cur_ControlBlk_Index <= kF3) )
		{
		/*----------------------*/
		/* f1 - f3				*/
		/*----------------------*/

		consonant_Phon = (*(vv->funcList->e_GetPhon_FUNC)) (vv, i_Consonant);
		vowel1_Phon = (*(vv->funcList->e_GetPhon_FUNC)) (vv, i_Vowel);
		
		if (bType == C_V_type)
			{
			vowel_Rank = zz->Rank_FWD_Tbl[vowel1_Phon];
			consonant_Rank = zz->Rank_BKWD_Tbl[consonant_Phon];
			}
		else   /* V_C_type	*/
			{
			vowel_Rank = zz->Rank_BKWD_Tbl[vowel1_Phon];
			consonant_Rank = zz->Rank_FWD_Tbl[consonant_Phon];
			}
	
		if ( (consonant_Rank == kConsonantR) && (vowel_Rank != kConsonantR) )
			{
			/*------------------------------*/
			/* C -> V or V -> C				*/
			/*------------------------------*/
			v1_Flags = vv->phonFlags2[vowel1_Phon];
			con_Flags = vv->phonFlags2[consonant_Phon];
				
			if (v1_Flags & kYGlideStartF)
				f2_y_Colored = true;
			else
				f2_y_Colored = false;
			
			if (bType == C_V_type)
				{
				v1_Target = Get_FIRST_Target (vv, i_Vowel);		/* C ->> V	*/
				}
			else
				{
				v1_Target = Get_LAST_Target (vv, i_Vowel);		/* V <<- C	*/
				}
			
			switch (vowel_Rank)
				{
				case kFrontR:
					loci_Tbl_Index = zz->Front_Loci_Tbl[consonant_Phon];
					break;

				case kMiddleR:
					loci_Tbl_Index = zz->Mid_Loci_Tbl[consonant_Phon];
					break;

				case kBackR:
				case kRoundR:
					loci_Tbl_Index = zz->Back_Loci_Tbl[consonant_Phon];
					break;
				}
			
			if (loci_Tbl_Index != kNoValue)
				{
				loci_Tbl_Index = loci_Tbl_Index >> 1;					/* @@@@@ change table	*/
				loci_Tbl_Index += (zz->cur_ControlBlk_Index - kF1) * 3;
				
				locus_Freq = *(zz->voice_Locus_Tbl + loci_Tbl_Index++);
				locus_Pcnt = *(zz->voice_Locus_Tbl + loci_Tbl_Index++);
				locus_Freq += zz->locusOffset;
				zz->trans_TIME = *(zz->voice_Locus_Tbl + loci_Tbl_Index) / kFrameTime;
				
				//if ( !(con_Flags & kNasalF) && (zz->voice_Num == kMaleTbls) && !(f2_y_Colored) )
				if ( !(con_Flags & kNasalF) && !(f2_y_Colored) )
					zz->trans_TIME = zz->trans_TIME - (zz->trans_TIME >> 2);
				
				if ( (vowel_Rank == kRoundR) && (zz->cur_ControlBlk_Index != kF1) &&
						(con_Flags & (kDentalF + kPalatalF)) )
					{
					locus_Pcnt = (locus_Pcnt >> 1) + 50;
					}
				
				if (f2_y_Colored && (zz->cur_ControlBlk_Index == kF2) )
					{
					locus_Pcnt = (25 - (locus_Pcnt >> 2)) + locus_Pcnt;
					//zz->trans_TIME = (zz->trans_TIME >> 1) + (10/kFrameTime);
					}
				
				target_Offset = (locus_Pcnt * (v1_Target - locus_Freq)) / 100;
				zz->trans_LEVEL = locus_Freq + target_Offset;
				}
			}
		}
}






void 	Head_Rules (voiceVarPtr vv)
{
	ControlBlock	*cb;
	short			cur_ControlBlk_Type;
	short			ampT;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	cb = (ControlBlock*) &(zz->controlBlockArray[zz->cur_ControlBlk_Index]);
	cur_ControlBlk_Type = zz->CtrlBlockTypeTbl[zz->cur_ControlBlk_Index];

	if (cur_ControlBlk_Type == kFreqType)
		/****************/
		/* f1 - f3		*/
		/****************/
		{
		if (vv->cur_PhonFlags_CF & kSonorant1F)
			/*----------------------*/
			/* cur = SONORANT		*/
			/*----------------------*/
			{
			if ( !(vv->cur_PhonFlags_CF & kLiqGlideF) )
				/*----------------------*/
				/* cur � LiqGlide		*/
				/*----------------------*/
				{
				zz->trans_TIME = 45 / kFrameTime;
				
				if (vv->prev_PhonFlags_CF & kLiqGlideF)
					/*-----------------------------------*/
					/* liqglide -> NON-LIQGLIDE SONORANT */
					/*-----------------------------------*/
					{
					zz->trans_LEVEL = (cb->prevP_END_Targ + zz->trans_LEVEL) >> 1; 	 /* move closer to liq-glide end target	*/
					
					if ( (vv->prev_Phon_CF == _l_) && (zz->cur_ControlBlk_Index == kF1) )
						/*--------------------------*/
						/* prev = L					*/
						/*--------------------------*/
						{
						zz->trans_LEVEL += 80;					/* !!!!! freq	*/
						}
						
					else if ( (vv->prev_Phon_CF == _r_) && (zz->cur_ControlBlk_Index != kF1) )
						/*--------------------------*/
						/* prev = R					*/
						/*--------------------------*/
						{
						zz->trans_TIME = 70 / kFrameTime;
						}
					}
				else
					/*--------------------------*/
					/* prev � LiqGlide			*/
					/*--------------------------*/
					{
					if (vv->cur_Phon_CF == _h_)
						/*--------------------------*/
						/* cur = H					*/
						/*--------------------------*/
						{
						zz->trans_LEVEL = (cb->prevP_END_Targ + zz->trans_LEVEL) >> 1;
						}
					}
				}
			else
				/*--------------------------*/
				/* cur = LiqGlide			*/
				/*--------------------------*/
				{
				if ( !(vv->prev_PhonFlags_CF & kLiqGlideF) )
					/*--------------------------*/
					/* prev � LiqGlide			*/
					/*--------------------------*/
					{
					zz->trans_LEVEL = (cb->prevP_END_Targ + zz->trans_LEVEL) >> 1;
					}
				else
					{
					zz->trans_LEVEL = (cb->prevP_END_Targ + zz->trans_LEVEL) >> 1;
					}
				zz->trans_TIME = 32 / kFrameTime;
				}
			}
			
		if (vv->cur_Phon_CF == _SIL_)
			/*--------------------------*/
			/* cur = SIL				*/
			/*--------------------------*/
			{
			zz->trans_LEVEL = cb->prevP_END_Targ;			/* prev end target...	*/
			zz->trans_TIME = vv->cur_Phon_Dur_CF;			/* ...for the entire duration	*/
			}
		else
			{
			/*--------------------------*/
			/*	 C  <-  V				*/
			/*	n-1     n				*/
			/*--------------------------*/
			Get_Locus (	vv, vv->cur_PhonBuf_Index_CF-1,		/* i_Consonant = n-1	*/
						vv->cur_PhonBuf_Index_CF,			/* i_Vowel = n			*/
						C_V_type);							/* C -> V				*/

			/*--------------------------*/
			/*	 V  <-  C				*/
			/*	n-1     n				*/
			/*--------------------------*/
			Get_Locus (	vv, vv->cur_PhonBuf_Index_CF,		/* i_Consonant = n	*/
						vv->cur_PhonBuf_Index_CF-1,			/* i_Vowel = n-1	*/
						V_C_type);							/* V -> C			*/
			
			if ( (vv->prev_PhonFlags_CF & kStopF) && !(vv->prev_PhonFlags_CF & kVoicedF) &&
				(zz->cur_ControlBlk_Index == kF1) )
				/*-----------------------------	*/
				/* prev = voiceless stop	*/
				/*-----------------------------	*/
				{
				zz->trans_LEVEL += 100;					/* f1 + 100	*/
				}
			
			if (vv->cur_PhonFlags_CF & kPlosFricF)
				/*-----------------------------	*/
				/* cur = PlosFric	*/
				/*-----------------------------	*/
				{
				if (zz->cur_ControlBlk_Index == kF1)
					zz->trans_TIME = 20 / kFrameTime;		/* f1 to targ faster	*/
				else
					zz->trans_TIME = 30 / kFrameTime;
					
				if (vv->cur_PhonFlags_CF & kStopF)
					{
					zz->trans_TIME = vv->cur_Phon_Dur_CF;
					}
				}
				
			if (vv->cur_PhonFlags_CF & kNasalF)
				/*--------------------------*/
				/* cur = Nasal				*/
				/*--------------------------*/
				{
				if (zz->cur_ControlBlk_Index == kF1)
					zz->trans_TIME = 0;						/* f1 steps to targ right away	*/
				else
					zz->trans_TIME = vv->cur_Phon_Dur_CF;	/* f2-f3 slow ramp to targ	*/
				
				if ( ((vv->cur_Phon_CF == _n_) || (vv->cur_Phon_CF == _EN_)) &&
					(zz->Rank_BKWD_Tbl[vv->prev_Phon_CF] == kFrontR) )
					/*--------------------------*/
					/* front -> N, EN			*/
					/*--------------------------*/
					{
					if (zz->cur_ControlBlk_Index == kF2)
						{
						
						if (vv->prev_PhonFlags_CF & kYGlideEndF)
							zz->trans_LEVEL -= 200;					/* f2 - 200	*/
						else
							zz->trans_LEVEL -= 100;					/* f2 - 100	*/
						}
						
					else if (zz->cur_ControlBlk_Index == kF3)
						{
						zz->trans_LEVEL -= 100;						/* f3 - 100	*/
						}
					}
					
				else if ( (vv->cur_Phon_CF == _m_) && (zz->cur_ControlBlk_Index == kF2) &&
					(vv->prev_PhonFlags_CF & kYGlideEndF) )
					/*--------------------------*/
					/* y -> M					*/
					/*--------------------------*/
					{
					zz->trans_LEVEL -= 150;							/* f2 - 150	*/
					}
				}
			}
		
		if ( !(vv->cur_PhonFlags_CF & kPlosFricF) && (zz->Rank_BKWD_Tbl[vv->prev_Phon_CF] != kConsonantR) &&
				(zz->trans_TIME > 0) )
			{
			zz->trans_TIME = 1 + ((zz->cur_Phon_PctOfMaxDur1_CF * zz->trans_TIME) >> 16);
			}
		}
		

	else if (cur_ControlBlk_Type == kFNZType)
		/****************/
		/* fnz			*/
		/****************/
		{
		if ( (vv->prev_PhonFlags_CF & kNasalF) && !(vv->cur_PhonFlags_CF & kNasalF) )
			/*--------------------------*/
			/* nasal -> XX				*/
			/*--------------------------*/
			{
			zz->trans_LEVEL = zz->nasalBaseFreq + ((zz->nasalTargFreq - zz->nasalBaseFreq) >> 1);
			
			if (vv->cur_PhonFlags_CF & kLiqGlideF)
				zz->trans_TIME = 80 / kFrameTime;
			else
				zz->trans_TIME = 80 / kFrameTime;
			}
			
		if (vv->cur_PhonFlags_CF & kNasalF)
			zz->trans_LEVEL = zz->nasalTargFreq;
		}
		
		
	else if (cur_ControlBlk_Type == kBWType)
		/****************/
		/* bw1 - bw3	*/
		/****************/
		{		
		if ( (vv->cur_PhonFlags_CF & kVoicedF) )
			/*--------------------------*/
			/* cur = Voiced				*/
			/*--------------------------*/
			{
			if ( !(vv->prev_PhonFlags_CF & kVoicedF) && (zz->cur_ControlBlk_Index == kBW1) )
				/*--------------------------*/
				/* prev � Voiced   BW1		*/
				/*--------------------------*/
				{
				zz->trans_TIME = 50 / kFrameTime;
				zz->trans_LEVEL = ((zz->controlBlockArray[kF1].curP_START_Targ) >> 3) + cb->curP_START_Targ;
				}
			else
				zz->trans_TIME = 40 / kFrameTime;				/* voiced -> VOICED	*/
			}
			
		else
			/*--------------------------*/
			/* cur � Voiced				*/
			/*--------------------------*/
			zz->trans_TIME = 20 / kFrameTime;
		
		if (vv->prev_Phon_CF == _SIL_)
			/*--------------------------*/
			/* prev = _SIL_				*/
			/*--------------------------*/
			{
			zz->trans_LEVEL = ((kBW3 - cur_ControlBlk_Type)  * 50) + cb->curP_START_Targ;
			zz->trans_TIME = 50 / kFrameTime;
			}
		else if (vv->cur_Phon_CF == _SIL_)
			/*--------------------------*/
			/* cur = _SIL_				*/
			/*--------------------------*/
			{
			zz->trans_LEVEL = ((kBW3 - cur_ControlBlk_Type)  * 50) + cb->prevP_END_Targ;
			
			if ( !(vv->phonFlags2[vv->prev2_Phon_CF] & kVoicedF) &&
					(vv->prev_PhonCtrl_CF & kPlosive_Release) && (zz->cur_ControlBlk_Index == kBW1) )
				/*--------------------------*/
				/* prev = Plosive_Release	*/
				/* BW1						*/
				/*--------------------------*/
				{
				zz->trans_LEVEL = 250;
				}
			zz->trans_TIME = 50 / kFrameTime;
			}
			
		if (vv->prev_PhonFlags_CF & kNasalF)
			{
			/*--------------------------*/
			/* prev = Nasal				*/
			/*--------------------------*/
			zz->trans_LEVEL = cb->curP_START_Targ;			/* step to target	*/
			
			if (zz->cur_ControlBlk_Index == kBW2)
				{
				if ( ((vv->prev_Phon_CF == _n_) || (vv->prev_Phon_CF == _EN_)) &&
						(zz->Rank_FWD_Tbl[vv->cur_Phon_CF] != kFrontR) )
					{
					zz->trans_LEVEL += 60;
					zz->trans_TIME = 60 / kFrameTime;
					}
				}

			else if (zz->cur_ControlBlk_Index == kBW1)
				{
				zz->trans_LEVEL += 70;
				zz->trans_TIME = 100 / kFrameTime;
				}
			}
			
		if (vv->cur_PhonFlags_CF & kNasalF)
			/*--------------------------*/
			/* cur = Nasal				*/
			/*--------------------------*/
			{
			zz->trans_TIME = 0;
			}
		}
		
		
	else if ( (cur_ControlBlk_Type == kResonAmpType) || (cur_ControlBlk_Type == kSourceAmpType) )
		/************************/
		/* a2 - ab, av, af		*/
		/************************/
		{
		ampT = cb->curP_START_Targ - 10;
		
		if ( (zz->trans_LEVEL < ampT) || (vv->prev_PhonFlags_CF & kStopF) || ( vv->prev_Phon_CF == _JH_) )
			{
			zz->trans_LEVEL = ampT;
			
			if ( !(vv->cur_PhonFlags_CF & kPlosFricF) )
				zz->trans_TIME = 20 / kFrameTime;
			
			if (zz->cur_ControlBlk_Index == kAV)
				/*--------------------------*/
				/* AV						*/
				/*--------------------------*/
				{
				if ( (vv->prev_Phon_CF == _SIL_) && (vv->cur_PhonFlags_CF & kVoicedF) )
					/*--------------------------*/
					/* sil -> VOICED			*/
					/*--------------------------*/
					{
					zz->trans_LEVEL -= 8;
					zz->trans_TIME = 45 / kFrameTime;
					}

				if (vv->prev_PhonFlags_CF & kPlosFricF)
					/*--------------------------*/
					/* prev = PlosFric			*/
					/*--------------------------*/
					{
					zz->trans_LEVEL = ampT + 6;					/* @@@@ if av = 0????	*/
					}

				if (vv->prev_PhonFlags_CF & kStopF)
					/*--------------------------*/
					/* prev = stop				*/
					/*--------------------------*/
					{
					zz->trans_LEVEL = cb->curP_START_Targ - 5;
					/*zz->trans_LEVEL = 30;						/* @@@@	*/
					}
				}
			}
				
		if ( (vv->cur_PhonFlags_CF & kVoicedF) && (vv->prev_PhonFlags_CF & kNasalF) )
			/*--------------------------*/
			/* nasal -> VOICED			*/
			/*--------------------------*/
			{
			zz->trans_TIME = 0;
			}

		if ( (vv->prev_PhonFlags_CF & kVoicedF) && (vv->cur_PhonFlags_CF & kNasalF) &&
				(zz->cur_ControlBlk_Index == kAV) )
			/*--------------------------*/
			/* voiced -> NASAL			*/
			/* AV						*/
			/*--------------------------*/
			{
			zz->trans_TIME = 0;				/* abrupt change	*/
			}
			
		ampT = cb->prevP_END_Targ - 10;
		
		if (zz->trans_LEVEL < ampT)
			{
			zz->trans_LEVEL = ampT - 3;
			
			if (vv->cur_Phon_CF == _SIL_)
				zz->trans_TIME = 70 / kFrameTime;

			/*if (zz->cur_ControlBlk_Index == kAV)	*/
			/*	zz->trans_TIME = 0;	*/
			}
			
		if ( (zz->cur_ControlBlk_Index == kAp3) && (vv->cur_PhonFlags_CF & kAffricateF) )
			{
			zz->trans_TIME = vv->cur_Phon_Dur_CF -2;
			zz->trans_LEVEL = cb->curP_START_Targ - 30;
			}
		
		
		if ( 	(zz->cur_ControlBlk_Index == kAV) && 
				(vv->cur_PhonFlags_CF & kPlosiveF) )
			{
			zz->trans_TIME = 10 / kFrameTime;				/* Ramp Av to zero for closure	*/
			}
			
		if (zz->cur_ControlBlk_Index == kAF)
			{
			if ( 	(vv->cur_Phon_CF == _SIL_) ||
					(vv->cur_Phon_CF == _f_) ||
					(vv->cur_Phon_CF == _TH_) ||
					(vv->cur_Phon_CF == _s_) ||
					(vv->cur_Phon_CF == _SH_) )
				{
				if ( (vv->prev_PhonFlags_CF & kVoicedF) && !(vv->prev_PhonFlags_CF & kPlosFricF) )
					{
					if (vv->cur_Phon_CF == _SIL_)
						{
						zz->trans_TIME = 80 / kFrameTime;
						zz->trans_LEVEL = 52;
						}
					else
						{
						zz->trans_TIME = 45 / kFrameTime;
						zz->trans_LEVEL = 48;
						}
					}
				}
			}
		}
	
	if (zz->trans_TIME > vv->cur_Phon_Dur_CF)
		zz->trans_TIME = vv->cur_Phon_Dur_CF;

	if (zz->trans_TIME > 130 / kFrameTime)
		zz->trans_TIME = 130 / kFrameTime;

	if (zz->trans_TIME < 0)
		zz->trans_TIME = 0;
}








void	Tail_Rules (voiceVarPtr vv)
{
	ControlBlock	*cb;
	short			cur_ControlBlk_Type;
	short			ampT;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	cb = (ControlBlock*) &(zz->controlBlockArray[zz->cur_ControlBlk_Index]);
	cur_ControlBlk_Type = zz->CtrlBlockTypeTbl[zz->cur_ControlBlk_Index];

	if (cur_ControlBlk_Type == kFreqType)
		/****************/
		/* f1 - f3		*/
		/****************/
		{
		if (vv->cur_PhonFlags_CF & kSonorant1F)
			/*--------------------------*/
			/* cur = SONORANT			*/
			/*--------------------------*/
			{
			zz->trans_TIME = 45 / kFrameTime;
			if ( !(vv->cur_PhonFlags_CF & kLiqGlideF) )
				/*--------------------------*/
				/* cur � LiqGlide			*/
				/*--------------------------*/
				{
				if (vv->next_PhonFlags_CF & kLiqGlideF)
					/*--------------------------*/
					/* next = LiqGlide			*/
					/*--------------------------*/
					{
					/*zz->trans_LEVEL = (cb->nextP_START_Targ + zz->trans_LEVEL) >> 1;	*/
					
					if (zz->cur_ControlBlk_Index == kF3)
						zz->trans_TIME = 60 / kFrameTime;
					
					if ( (vv->next_Phon_CF == _l_) && (zz->cur_ControlBlk_Index == kF1) )
						/*--------------------------*/
						/* next = L					*/
						/*--------------------------*/
						{
						zz->trans_LEVEL += 80;					/* !!!!! freq	*/
						}
						
					}
				else
					/*--------------------------*/
					/* next � LiqGlide			*/
					/*--------------------------*/
					{
					if (vv->next_Phon_CF == _h_)
						/*--------------------------*/
						/* cur = H					*/
						/*--------------------------*/
						{
						zz->trans_LEVEL = (cb->curP_END_Targ + zz->trans_LEVEL) >> 1;
						}
					}
				}
			else
				/*--------------------------*/
				/* cur = LiqGlide			*/
				/*--------------------------*/
				{
				if ( !(vv->next_PhonFlags_CF & kLiqGlideF) )
					/*--------------------------*/
					/* next � LiqGlide			*/
					/*--------------------------*/
					{
					zz->trans_LEVEL = (cb->curP_END_Targ + zz->trans_LEVEL) >> 1;
					zz->trans_TIME = 20 / kFrameTime;
					}
				else
					{
					zz->trans_LEVEL = (cb->curP_END_Targ + zz->trans_LEVEL) >> 1;
					zz->trans_TIME = 40 / kFrameTime;
					}
				}
			}
			
		if (vv->next_Phon_CF == _SIL_)
			/*--------------------------*/
			/* next = SIL				*/
			/*--------------------------*/
			{
			zz->trans_TIME = 0;
			}
		else
			{
			/*--------------------------*/
			/*	V  ->  C 				*/
			/*	n     n+1				*/
			/*--------------------------*/
			Get_Locus (	vv, vv->cur_PhonBuf_Index_CF+1,		/* i_Consonant = n+1	*/
						vv->cur_PhonBuf_Index_CF,			/* i_Vowel = n			*/
						V_C_type);							/* V1 -> C -- V2		*/

			/*--------------------------*/
			/*	C  ->  V				*/
			/*	n     n+1				*/
			/*--------------------------*/
			Get_Locus (	vv, vv->cur_PhonBuf_Index_CF,		/* i_Consonant = n		*/
						vv->cur_PhonBuf_Index_CF+1,			/* i_Vowel = n+1		*/
						C_V_type);							/* V2 -- C -> V1		*/
			
			
			if (vv->cur_PhonFlags_CF & kPlosFricF)
				{
				if (zz->cur_ControlBlk_Index == kF1)
					zz->trans_TIME = 20 / kFrameTime;
				else
					zz->trans_TIME = 30 / kFrameTime;

				if (vv->cur_PhonFlags_CF & kStopF)
					{
					zz->trans_TIME = vv->cur_Phon_Dur_CF;
				
					if ( !(vv->cur_PhonFlags_CF & kVoicedF) && (zz->cur_ControlBlk_Index == kF1) )
						/*--------------------------*/
						/* cur = voiceless stop		*/
						/*--------------------------*/
						{
						zz->trans_LEVEL += 100;					/* f1 + 100	*/
						}
					}
				}
				
			
				
			if (vv->cur_PhonFlags_CF & kNasalF)
				/*--------------------------*/
				/* cur = Nasal				*/
				/*--------------------------*/
				{
				if (zz->cur_ControlBlk_Index == kF1)
					zz->trans_TIME = 0;
				else
					zz->trans_TIME = vv->cur_Phon_Dur_CF;
				
				if ( ((vv->cur_Phon_CF == _n_) || (vv->cur_Phon_CF == _EN_)) &&
					(zz->Rank_FWD_Tbl[vv->next_Phon_CF] == kFrontR) )
					/*--------------------------*/
					/* N, EN -> Front 			*/
					/*--------------------------*/
					{
					if (zz->cur_ControlBlk_Index == kF2)
						{
						zz->trans_LEVEL -= 100;						/* f2 - 100	*/
						if (vv->next_PhonFlags_CF & kYGlideStartF)
							zz->trans_LEVEL -= 100;					/* f2 - 100	*/
						}
					else if (zz->cur_ControlBlk_Index == kF3)
						{
						zz->trans_LEVEL -= 100;						/* f3 - 100	*/
						}
					}
				else if ( (vv->cur_Phon_CF == _m_) && (zz->cur_ControlBlk_Index == kF2) &&
					(vv->next_PhonFlags_CF & kYGlideStartF) )
					/*--------------------------*/
					/*  M -> Front				*/
					/*--------------------------*/
					{
					zz->trans_LEVEL -= 150;							/* f2 - 150	*/
					}
				}
			}
		
		if ( !(vv->cur_PhonFlags_CF & kPlosFricF) && (zz->Rank_FWD_Tbl[vv->next_Phon_CF] != kConsonantR) &&
				(zz->trans_TIME > 0) )
			{
			zz->trans_TIME = 1 + ((zz->cur_Phon_PctOfMaxDur2_CF * zz->trans_TIME) >> 16);
			}
		}
		

	else if (cur_ControlBlk_Type == kFNZType)
		/****************/
		/* fnz			*/
		/****************/
		{
		if ( (vv->next_PhonFlags_CF & kNasalF) && !(vv->cur_PhonFlags_CF & kNasalF) )
			/*--------------------------*/
			/*  XX -> nasal				*/
			/*--------------------------*/
			{
			zz->trans_LEVEL = zz->nasalTargFreq;
			zz->trans_TIME = 80 / kFrameTime;
			}
		}
		
		
	else if (cur_ControlBlk_Type == kBWType)
		/****************/
		/* bw1 - bw3	*/
		/****************/
		{		
		if ( (vv->cur_PhonFlags_CF & kVoicedF) )
			/*--------------------------*/
			/* cur = Voiced				*/
			/*--------------------------*/
			{
			zz->trans_TIME = 40 / kFrameTime;
			
			if ( !(vv->next_PhonFlags_CF & kVoicedF) && (zz->cur_ControlBlk_Index == kBW1) )
				/*--------------------------*/
				/* next � Voiced   BW1		*/
				/*--------------------------*/
				{
				zz->trans_TIME = 50 / kFrameTime;
				zz->trans_LEVEL = ((zz->controlBlockArray[kF1].curP_START_Targ) >> 3) + cb->curP_END_Targ;
				}
			}
		else
			/*--------------------------*/
			/* cur � Voiced				*/
			/*--------------------------*/
			zz->trans_TIME = 20 / kFrameTime;
		

		if (vv->next_Phon_CF == _SIL_)
			/*--------------------------*/
			/* next = _SIL_				*/
			/*--------------------------*/
			{
			zz->trans_LEVEL = ((kBW3 - cur_ControlBlk_Type)  * 50) + cb->curP_END_Targ;
			zz->trans_TIME = 50 / kFrameTime;
			}
			
		else if (vv->cur_Phon_CF == _SIL_)
			/*--------------------------*/
			/* cur = _SIL_				*/
			/*--------------------------*/
			{
			zz->trans_LEVEL = ((kBW3 - cur_ControlBlk_Type)  * 50) + cb->nextP_START_Targ;
			zz->trans_TIME = 50 / kFrameTime;
			}
			
		if (vv->next_PhonFlags_CF & kNasalF)
			{
			/*--------------------------*/
			/* next = Nasal				*/
			/*--------------------------*/
			zz->trans_LEVEL = cb->curP_END_Targ;
			
			if (zz->cur_ControlBlk_Index == kBW2)
				{
				if ( ((vv->next_Phon_CF == _n_) || (vv->next_Phon_CF == _EN_)) &&
						(zz->Rank_FWD_Tbl[vv->cur_Phon_CF] != kFrontR) )
					{
					zz->trans_LEVEL += 60;
					zz->trans_TIME = 60 / kFrameTime;
					}
				}

			else if (zz->cur_ControlBlk_Index == kBW1)
				{
				zz->trans_LEVEL += 100;
				zz->trans_TIME = 100 / kFrameTime;
				}
			}
			
		if (vv->cur_PhonFlags_CF & kNasalF)
			/*--------------------------*/
			/* cur = Nasal				*/
			/*--------------------------*/
			{
			zz->trans_TIME = 0;
			}
		}
		
		
	else if ( (cur_ControlBlk_Type == kResonAmpType) || (cur_ControlBlk_Type == kSourceAmpType) )
		/********************/
		/* a2 - ab, av, af	*/
		/********************/
		{
		ampT = cb->nextP_START_Targ - 10;
		
		if (zz->trans_LEVEL < ampT)
			{
			zz->trans_LEVEL = ampT;
			
			if ( vv->cur_Phon_CF == _SIL_ )
				zz->trans_TIME = 70 / kFrameTime;
			}
			
		if ( (zz->cur_ControlBlk_Index == kAV) && (zz->trans_LEVEL < cb->nextP_START_Targ) )
			/*--------------------------*/
			/* AV						*/
			/*--------------------------*/
			{
			if ( 	(vv->cur_Phon_CF != _v_) &&
					(vv->cur_Phon_CF != _DH_) &&
					(vv->cur_Phon_CF != _JH_) &&
					(vv->cur_Phon_CF != _ZH_) &&
					(vv->cur_Phon_CF != _z_) )
				{
				zz->trans_TIME = 0;
				
				if (vv->cur_PhonFlags_CF & (kStopF+kAffricateF))
					{
				/*--------------------------*/
				/* cur = Stop or Affricate	*/
				/*--------------------------*/
					if (vv->cur_PhonFlags_CF & kVoicedF)
						/*--------------------------*/
						/* cur = Voiced				*/
						/*--------------------------*/
						{
						zz->trans_LEVEL = cb->curP_END_Targ - 3;
						zz->trans_TIME = 45 / kFrameTime;
						}
					else
						zz->trans_TIME = 0;
					
					goto Done;
					}
				}
			}
				
		if ( (vv->cur_PhonFlags_CF & kVoicedF) && (vv->next_PhonFlags_CF & kNasalF) )
			/*--------------------------*/
			/* VOICED -> nasal			*/
			/*--------------------------*/
			{
			zz->trans_TIME = 0;
			}

		if (vv->cur_PhonFlags_CF & kNasalF)
			{
				if ( (vv->next_PhonFlags_CF & kVoicedF) &&
					!(vv->cur_PhonFlags_CF & kPlosFricF) && !(vv->next_PhonCtrl_CF & kPlosive_Release) )
				{
				zz->trans_TIME = 0;
				}
			else
				{
				zz->trans_TIME = 40 / kFrameTime;
				}
			}
			
		ampT = cb->curP_END_Targ - 10;
		
		if (vv->cur_PhonFlags_CF & kPlosiveF)
			{
			zz->trans_TIME = 15 / kFrameTime;
			
			if ( (vv->cur_PhonFlags_CF  & kStopF) || (vv->cur_Phon_CF == _DX_) ||
				(vv->cur_Phon_CF == _QX_) || (vv->cur_Phon_CF == _DD_) )
				{
				ampT = cb->curP_END_Targ;
				}
			}
		
		if (zz->trans_LEVEL < ampT)
			{
			zz->trans_LEVEL = ampT - 3;
			zz->trans_TIME = 20 / kFrameTime;
			}
		
		if (zz->cur_ControlBlk_Index == kAV)
			/*--------------------------*/
			/* Av						*/
			/*--------------------------*/
			{
			if ( (zz->trans_LEVEL < ampT) || ((ampT > 0 ) && (vv->next_PhonCtrl_CF & kPlosive_Release)) )
				{
				zz->trans_LEVEL = ampT + 3;
				if ( (vv->next_Phon_CF == _SIL_) || (vv->next_PhonCtrl_CF & kPlosive_Release) )
					{
					zz->trans_TIME = 75 / kFrameTime;
					}
				}
			}
		
		if (vv->next_Phon_CF >= _p_) 							/* @@@@@	*/
			{
			if ( !(vv->cur_PhonFlags_CF & kNasalF) || (zz->cur_ControlBlk_Index != kAV) )
				zz->trans_TIME = 0;
			}
		
		/*#if 0	*/
		if (zz->cur_ControlBlk_Index == kAF)
			{
			if ( 	(vv->cur_Phon_CF == _f_) ||
					(vv->cur_Phon_CF == _TH_) ||
					(vv->cur_Phon_CF == _s_) ||
					(vv->cur_Phon_CF == _SH_) )
				{
				if ( (vv->next_PhonFlags_CF & kVoicedF) && !(vv->next_PhonFlags_CF & kPlosFricF) )
					{
					zz->trans_TIME = 40 / kFrameTime;
					zz->trans_LEVEL = 52;
					}
				}
			
			if ( (vv->cur_PhonFlags_CF & kVowelF) && (vv->next_Phon_CF == _SIL_) )
				{
				zz->trans_TIME = 130 / kFrameTime;
				zz->trans_LEVEL = 52;
				}
			}
		/*#endif	*/
		
		}

Done:
	if (zz->trans_TIME > vv->cur_Phon_Dur_CF)
		zz->trans_TIME = vv->cur_Phon_Dur_CF;

	if (zz->trans_TIME > 130 / kFrameTime)
		zz->trans_TIME = 130 / kFrameTime;
	
	cb->TAIL_START_time = vv->cur_Phon_Dur_CF - zz->trans_TIME;

	if (zz->trans_TIME < 0)
		zz->trans_TIME = 0;
}





short	Scale_Prcnt_to_PhonDur (formantVarPtr zz, short percent)
{
	long	tempL;
	
	tempL = (percent * zz->cur_Phon_PctOfMaxDur_CF) >> 8;		/* 8 bit fraction	*/
	tempL = ((zz->cur_Phon_MaxDur_CF * tempL) / 100) >> 8;
	if (tempL <= 0)
		tempL = 1;
	return (tempL);
}






/*--------------------------*/
/*							*/
/*			 t1	  t2		*/
/*		     |    |			*/
/* 	p1 ------|	  |			*/
/*		     -	  |			*/
/*            -	  |			*/
/*             -  |			*/
/*			    - |			*/
/*			   p2 -------	*/
/*--------------------------*/

void 	Get_Diphthongs (voiceVarPtr vv, short index)
{
	ControlBlock	*cb;
	short			cur_ControlBlk_Type;
	short			p1_Val, p2_Val, t1_Val, t2_Val;
	long			artic_Factor, tempL;
	short			rampTime, step_Size;
	formantVarPtr 	zz;


	zz = (formantVarPtr)vv->synthVars;
	cb = (ControlBlock*) &(zz->controlBlockArray[zz->cur_ControlBlk_Index]);
	cur_ControlBlk_Type = zz->CtrlBlockTypeTbl[zz->cur_ControlBlk_Index];
	
	artic_Factor = k1pct * 10;						/* coarticulation amount	*/
	cb->ptrToTargetList = zz->next_DiphEntry;		/* remember starting point in fifo	*/


	/*----------------------------------*/
	/* Get envelope values from table	*/
	/*----------------------------------*/
	p1_Val = zz->EnvelopeListTbl[index++];
	t1_Val = zz->EnvelopeListTbl[index++];
	p2_Val = zz->EnvelopeListTbl[index++];
	t2_Val = zz->EnvelopeListTbl[index];

	t1_Val = Scale_Prcnt_to_PhonDur (zz, t1_Val);
	t2_Val = Scale_Prcnt_to_PhonDur (zz, t2_Val);

	if (cur_ControlBlk_Type == kFreqType)
		/*--------------------------*/
		/* f1 - f3					*/
		/*--------------------------*/
		{
		/*----------------------------------*/
		/* p1 coarticulates with PREV phon	*/
		/*----------------------------------*/
		if (cb->prevP_END_Targ > 0)
			p1_Val += ((cb->prevP_END_Targ - p1_Val) * artic_Factor) >> 16;
		p1_Val += Adjust_Colored_Target (vv, vv->cur_PhonBuf_Index_CF, 0);
	
		/*----------------------------------*/
		/* p2 coarticulates with NEXT phon	*/
		/*----------------------------------*/
		if (cb->nextP_START_Targ > 0)
			p2_Val += ((cb->nextP_START_Targ - p2_Val) * artic_Factor) >> 16;
		p2_Val += Adjust_Colored_Target (vv, vv->cur_PhonBuf_Index_CF, 1);
		}


	rampTime = t2_Val - t1_Val;					/* # of frames in ramp	*/

	tempL = (p2_Val - p1_Val) << kStepSizeRes;
	if (rampTime < kSizeOf1xTbl)
		/*------------------*/
		/* the FAST way		*/
		/*------------------*/
		{
		step_Size = (zz->One_Over_X_Tbl[rampTime] * tempL) >> 16;
		}
	else
		/*------------------*/
		/* the SLOW way		*/
		/*------------------*/
		{
		step_Size = tempL / rampTime;
		}
		
	cb->curP_START_Targ = p1_Val;				/* Starting TARGET	*/
	cb->curTarget_TIME = t1_Val;				/* 1st END TIME	*/
	cb->curTarget_STEP = 0;						/* 1st STEP SIZE	*/

	*zz->next_DiphEntry = t2_Val;				/* -> ramp END TIME	*/
	zz->next_DiphEntry++;
	*zz->next_DiphEntry = step_Size;			/* -> ramp STEP SIZE	*/
	zz->next_DiphEntry++;

	*zz->next_DiphEntry = vv->cur_Phon_Dur_CF;	/* -> phon END TIME	*/
	zz->next_DiphEntry++;
	*zz->next_DiphEntry = 0;					/* -> end STEP SIZE	*/
	zz->next_DiphEntry++;

	cb->curP_END_Targ = p2_Val;					/* END target value	*/

}
		




void	Fill_Phon_Targets (voiceVarPtr vv)
{
	ControlBlock	*cb;
	short			i;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	if (vv->cur_PhonBuf_Index_CF == 0)
		/*--------------------------*/
		/* Start of new sentence	*/
		/*--------------------------*/
		{
		if (zz->big_Bang)
			/*--------------------------*/
			/* Start of buffer			*/
			/*--------------------------*/
			{
			zz->big_Bang = false;
			for (zz->cur_ControlBlk_Index = 0; zz->cur_ControlBlk_Index < kNumOfBlocks; zz->cur_ControlBlk_Index++)
				{
				cb = (ControlBlock*) &(zz->controlBlockArray[zz->cur_ControlBlk_Index]);
				cb->curP_END_Targ = Get_FIRST_Target (vv, vv->cur_PhonBuf_Index_CF);
				}
			}
		}
	zz->cur_Phon_MaxDur_CF = vv->maxDurTbl[vv->cur_Phon_CF] / kFrameTime;
	if ( !(vv->cur_PhonFlags_CF & kPlosFricF) && (vv->cur_Phon_CF != _SIL_) )
		{
		zz->cur_Phon_PctOfMaxDur_CF = ((long)vv->cur_Phon_Dur_CF << 16) / zz->cur_Phon_MaxDur_CF;
		zz->cur_Phon_PctOfMaxDur1_CF = (zz->cur_Phon_PctOfMaxDur_CF >> 1) + kOneHalf;
		zz->cur_Phon_PctOfMaxDur2_CF = zz->cur_Phon_PctOfMaxDur1_CF - (10 * pct);
		}
	
	zz->next_DiphEntry = (short*) &zz->diphEntryArray[0];		/* reset pointer	*/

	for (i = kF1; i < kNumOfBlocks; i++)
		{
		cb = (ControlBlock*) &(zz->controlBlockArray[i]);
		cb->onset_END_TIME = 0;
		}

}






void	Init_Ctrls_for_New_Phon (voiceVarPtr vv)
{
	ControlBlock	*cb;
	short			cur_ControlBlk_Type;
	long			tempL;
	short			tempS;
	formantVarPtr 	zz;


	zz = (formantVarPtr)vv->synthVars;
	Fill_Phon_Targets (vv);

	for (zz->cur_ControlBlk_Index = kF1; zz->cur_ControlBlk_Index < kNumOfBlocks; zz->cur_ControlBlk_Index++)
		{
		cb = (ControlBlock*) &(zz->controlBlockArray[zz->cur_ControlBlk_Index]);
		cur_ControlBlk_Type = zz->CtrlBlockTypeTbl[zz->cur_ControlBlk_Index];
		
		cb->prevP_END_Targ = cb->curP_END_Targ;
		cb->nextP_START_Targ = Get_FIRST_Target (vv, vv->cur_PhonBuf_Index_CF +1);	/* NEXT phon's target	*/
		cb->curTarget_OFFS = 0;
		cb->curP_START_Targ = GetTarget (vv, vv->cur_PhonBuf_Index_CF);		/* CUR phon's target	*/
		
		
		if (cb->curP_START_Targ < kNoValue)
			/*--------------------------*/
			/* diphthong				*/
			/*--------------------------*/
			{
			Get_Diphthongs (vv, cb->curP_START_Targ & 0x7FFF);					/* strip flag bit	*/
			}
			
		else
		
			/*--------------------------*/
			/* NOT a diphthong			*/
			/*--------------------------*/
			{
			cb->curTarget_STEP = 0;
			cb->curTarget_TIME = vv->cur_Phon_Dur_CF;

			if (cur_ControlBlk_Type == kFreqType)
				/*----------------------------------------------*/
				/* f1 - f3										*/
				/*												*/
				/* Coarticulate curP_START_Targ 				*/
				/*  with prevP_END_Targ and nextP_START_Targ	*/
				/*----------------------------------------------*/
				{
				tempL = k1pct * 10;
				if (vv->phon_Ctrl_Buf_2[vv->cur_PhonBuf_Index_CF] & kIsStressed)
					{
					if (zz->cur_ControlBlk_Index == kF2)
						tempL = k1pct * 25;
					else
						tempL = k1pct * 15;
					}
				
				cb->curP_START_Targ += ((( (cb->prevP_END_Targ + cb->nextP_START_Targ) >> 1) - cb->curP_START_Targ) * tempL) >> 16;
				}
			
			cb->curP_END_Targ = cb->curP_START_Targ;		/* end = start, since not a diphthong	*/
			}

		if (cur_ControlBlk_Type == kFreqType)
			/*----------------------------------*/
			/* f1 - f3							*/
			/*									*/
			/* Coarticulate nextP_START_Targ 	*/
			/*  with curP_END_Targ				*/
			/*----------------------------------*/
			{
			tempL = k1pct * 10;
			cb->nextP_START_Targ += ( (cb->curP_END_Targ - cb->nextP_START_Targ) * tempL) >> 16;
			}
		
		/********************************/
		/*								*/
		/* HEAD Envelope				*/
		/*								*/
		/********************************/
		
		/*--------------------------*/
		/* Set defaults				*/
		/*--------------------------*/
		zz->trans_LEVEL = (cb->prevP_END_Targ + cb->curP_START_Targ) >> 1;		/* mid point between prev and cur	*/
		zz->trans_TIME = 32 / kFrameTime;											/* 32 ms transition time to level	*/
		
		Head_Rules (vv);
			//zz->trans_TIME <<= 1;
		
		cb->HEAD_offs = 0;
		/*cb->HEAD_step = 0;	*/
		/*if (zz->trans_TIME <= 0) zz->trans_TIME = 2;				/* @@@@@ ADDED	*/
		
		if (zz->trans_TIME > 0)
			{
			cb->HEAD_offs = (zz->trans_LEVEL - cb->curP_START_Targ) << kStepSizeRes;
				
			if (cb->HEAD_offs != 0)
				{
				tempL = (zz->One_Over_X_Tbl[zz->trans_TIME] * cb->HEAD_offs) >> 16;
				cb->HEAD_step = tempL;
				cb->HEAD_offs = tempL * zz->trans_TIME;
				}
			}
		
		
		/********************************/
		/*								*/
		/* TAIL Envelope				*/
		/*								*/
		/********************************/
		
		zz->trans_LEVEL = (cb->curP_END_Targ + cb->nextP_START_Targ) >> 1;
		zz->trans_TIME = 25 / kFrameTime;
		
		Tail_Rules (vv);
			//zz->trans_TIME <<= 1;
		
		cb->TAIL_offs = 0;
		cb->TAIL_step = 0;
					
		if (zz->trans_TIME > 0)
			{
			tempS = (zz->trans_LEVEL - cb->curP_END_Targ) << kStepSizeRes;
			if (tempS != 0)
				{
				cb->TAIL_step = (zz->One_Over_X_Tbl[zz->trans_TIME] * tempS) >> 16;
				}
			}
		
		}
		
	Insert_Burst (vv);
}





void	Interpolate_Formants (voiceVarPtr vv)
{
	ControlBlock	*cb;
	short			i, offset;
	short			val;
	//long			val1;
	formantVarPtr 	zz;


	zz = (formantVarPtr)vv->synthVars;
	/****************************/
	/*   F1 - FNZ				*/
	/****************************/
	for (i = kF1; i <= kFNZ; i++)
		{
		cb = (ControlBlock*) &(zz->controlBlockArray[i]);
		
		if (vv->dur_Done_in_Phon_CF > cb->curTarget_TIME)
			/*------------------------------------------*/
			/* Get diphthong values from target list	*/
			/*------------------------------------------*/
			{
			cb->curTarget_TIME = *(cb->ptrToTargetList++);
			cb->curTarget_STEP = *(cb->ptrToTargetList++);
			cb->curP_START_Targ += (cb->curTarget_OFFS >> kStepSizeRes);
			cb->curTarget_OFFS = 0;
			}
		
		/*------------------------------------------*/
		/* Add TARGET step to target offset			*/
		/*------------------------------------------*/
		cb->curTarget_OFFS += cb->curTarget_STEP;
		
		/*------------------------------------------*/
		/* Add HEAD offset to target offset			*/
		/*------------------------------------------*/
		offset = cb->curTarget_OFFS + cb->HEAD_offs;
		
		if (cb->HEAD_offs != 0)
			cb->HEAD_offs -= cb->HEAD_step;				/* shrink the HEAD offset	*/
		
		if (vv->dur_Done_in_Phon_CF >= cb->TAIL_START_time)
			/*------------------------------------------*/
			/* Add TAIL offset to target offset			*/
			/*------------------------------------------*/
			{
			offset += cb->TAIL_offs;
			cb->TAIL_offs += cb->TAIL_step;				/* grow the TAIL offset	*/
			}
		
		val = cb->curP_START_Targ + (offset >> kStepSizeRes);


		#if 0
		if (i <= kBW3)
			{
			if (cb->lastVal == 0)
				cb->lastVal = val;
			val = 	((24000 * val) +
					((k100percent - 24000) * cb->lastVal)) >> 16;
			cb->lastVal = val;
			}
		#endif
		
		#if 0
		val1 = (val + cb->lastVal) >> 1;
		cb->lastVal = val1;
		
		//if (zz->leadGain)
		if (i <= kF3)
			{
			//val1 = (((long)(val - cb->lastVal)) * zz->leadGain) >> 16;
			val1 = (((long)(val - cb->lastVal)) * 0x10000) >> 16;
			cb->lastVal = val;
			val += (short) val1;
			}
		#endif


		
		zz->controlData[i] = val;
		
		/*--------------------------*/
		/* Constant ONSET value		*/
		/*--------------------------*/
		if (cb->onset_END_TIME > 0)
			{
			if (vv->dur_Done_in_Phon_CF < cb->onset_END_TIME)
				zz->controlData[i] = cb->onset_VAL;
			}
			
		}

	/****************************/
	/*   AV - AB				*/
	/****************************/
	for (i = kAV; i <= kAB; i++)
		{
		cb = (ControlBlock*) &(zz->controlBlockArray[i]);
		
		/*--------------------------------------*/
		/* Add HEAD offset to target offset		*/
		/*--------------------------------------*/
		offset = cb->curP_START_Targ + (cb->HEAD_offs >> kStepSizeRes);
		
		if (cb->HEAD_offs != 0)
			cb->HEAD_offs -= cb->HEAD_step;				/* shrink the HEAD offset	*/
		
		if (vv->dur_Done_in_Phon_CF >= cb->TAIL_START_time)
			/*--------------------------------------*/
			/* Add TAIL offset to target offset		*/
			/*--------------------------------------*/
			{
			offset += (cb->TAIL_offs >> kStepSizeRes);
			cb->TAIL_offs += cb->TAIL_step;				/* grow the TAIL offset	*/
			}

		zz->controlData[i] = offset;
		
		/*--------------------------*/
		/* Constant ONSET value		*/
		/*--------------------------*/
		if (cb->onset_END_TIME > 0)
			{
			if (vv->dur_Done_in_Phon_CF < cb->onset_END_TIME)
				{
				zz->controlData[i] = cb->onset_VAL;
				}
			else if ( (i >= kAp2) && (vv->dur_Done_in_Phon_CF == (cb->onset_END_TIME +1)) &&
					(zz->controlData[i] > 10) )
				/*----------------------*/
				/* A2 - AB				*/
				/*----------------------*/
				{
				zz->controlData[i] -= 10;				/* lower by 10db after onset	*/
				}
			}
		}
}





void	synth_AdjustPhons2 (voiceVarPtr vv)
{
	Insert_Closure_Release (vv);
}


void	synth_AdjustPhons1 (voiceVarPtr vv)
{
}



void	synth_StartNewPhon (voiceVarPtr vv)
{
		Init_Ctrls_for_New_Phon (vv);
}


void	synth_SpeakPhon (voiceVarPtr vv)
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	Interpolate_Formants (vv);
	SaveFrame (vv);
	
	if (zz->curFrameBuf == kBuf1)
		zz->curFrameBuf = kBuf2;
	else
		zz->curFrameBuf = kBuf1;
		
	vv->dur_Done_in_Phon_CF++;					// we've done one more Frame...
}


void	synth_Start_Talk (voiceVarPtr vv)
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	Init_ControlBlocks (zz);
	zz->curFrameBuf = kFrame1;
	e_Fill_Next_Frame (vv);				/* fill 1st frame	*/
	InitSay (vv);
	//Start_The_Speech (vv);
}




void	CopyVoice (voiceDataPtr src, voiceDataPtr dst)
{
	short			i;

	dst->voice = src->voice;
	dst->pitch = src->pitch;
	dst->pitchRange = src->pitchRange;
	dst->stressGain = src->stressGain;
	dst->rate = src->rate;
	dst->vGain = src->vGain;
	dst->aGain = src->aGain;
	dst->aCycle = src->aCycle;
	dst->f4_Freq = src->f4_Freq;
	dst->f4_BW = src->f4_BW;
	dst->f4p_Freq = src->f4p_Freq;
	dst->f4p_BW = src->f4p_BW;
	dst->f5p_Freq = src->f5p_Freq;
	dst->f5p_BW = src->f5p_BW;
	dst->f6p_Freq = src->f6p_Freq;
	dst->f6p_BW = src->f6p_BW;
	dst->nasal_Base = src->nasal_Base;
	dst->nasal_targ = src->nasal_targ;
	dst->nasal_BW = src->nasal_BW;
	dst->locus = src->locus;
	dst->bwGain1 = src->bwGain1;
	dst->bwGain2 = src->bwGain2;
	dst->bwGain3 = src->bwGain3;
	dst->f1_Offset = src->f1_Offset;
	dst->f2_Offset = src->f2_Offset;
	dst->f3_Offset = src->f3_Offset;
	dst->chorus = src->chorus;
	dst->nGain = src->nGain;
	dst->sPitch = src->sPitch;
	dst->sGain = src->sGain;
	dst->AsperW = src->AsperW;
	dst->voiceVers = src->voiceVers;
	dst->riseAmt = src->riseAmt;
	dst->fallAmt = src->fallAmt;
	dst->riseAmt1 = src->riseAmt1;
	dst->fallAmt1 = src->fallAmt1;
	dst->assertiveness = src->assertiveness;
	dst->baselineFall = src->baselineFall;
	dst->quickness = src->quickness;
	dst->pitchCmdStep = src->pitchCmdStep;
	dst->durCmdStep = src->durCmdStep;
	dst->down_Ramp_Step = src->down_Ramp_Step;
	dst->stressDurTime = src->stressDurTime;
	dst->tempo = src->tempo;
	dst->waveType = src->waveType;
	dst->sndID = src->sndID;
	dst->vowelSync = src->vowelSync;
	dst->loopPoint = src->loopPoint;
	
	for (i = 0; i < 48; i++)
		{
		dst->vWave[i] = src->vWave[i];
		dst->vWave1[i] = src->vWave1[i];
		}

	dst->customForm = src->customForm;
	
	dst->nasalAmt = src->nasalAmt;
	dst->vibratoDepth1 = src->vibratoDepth1;
	dst->vibratoDepth2 = src->vibratoDepth2;
	dst->vibratoFreq = src->vibratoFreq;
	dst->intonation = src->intonation;
	dst->portamento = src->portamento;
	dst->emphVoice = src->emphVoice;
	dst->rvbDelay = src->rvbDelay;
	dst->rvbDepth = src->rvbDepth;
	dst->rvbWetDry = src->rvbWetDry;

}



	void	fSynth_GetVoiceParams (voiceVarPtr vv, void *info )
{
	voiceDataPtr	vd_target;
	short			i;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	vd_target = *(voiceDataPtr*)info;	
	CopyVoice (&zz->vd, vd_target);
	
}


	void	fSynth_SetVoiceParams (voiceVarPtr vv, void *info )
{
	voiceDataPtr	vd_target;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	vd_target = *(voiceDataPtr*)info;
	CopyVoice (vd_target, &zz->vd);

}


	void	fSynth_SetSoundSample (voiceVarPtr vv, unsigned char **info )
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	zz->SampleWave = *info;
	InsertSample (vv, &zz->vd);
}

	void	fSynth_GetVoiceTables (voiceVarPtr vv, unsigned long **info )
{
	unsigned long	*vt_target;
	
	vt_target = *info;
	GetFormTables (vv, vt_target);
}

	void	fSynth_SetVoiceTables (voiceVarPtr vv, unsigned long **info )
{
	unsigned long	*vt_target;
	
	vt_target = *info;
	SetFormTables (vv, vt_target);
}



	void	fSynth_GetGlotGain (voiceVarPtr vv, short *info )
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	*info = zz->vd.vGain;
}

	void	fSynth_SetGlotGain (voiceVarPtr vv, short *info )
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	zz->vd.vGain = *info;
}


	void	fSynth_GetSampleGain (voiceVarPtr vv, short *info )
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	*info = zz->vd.sGain;
}

	void	fSynth_SetSampleGain (voiceVarPtr vv, short *info )
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	zz->vd.sGain = *info;
}


	void	fSynth_GetSamplePitch (voiceVarPtr vv, short *info )
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	*info = zz->vd.sPitch;
}

	void	fSynth_SetSamplePitch (voiceVarPtr vv, short *info )
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	zz->vd.sPitch = *info;
}


	void	fSynth_GetVoiceVars (voiceVarPtr vv, unsigned long *info )
{
	
	*info = (unsigned long) vv;
}
