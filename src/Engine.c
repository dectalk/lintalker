
#ifndef __SPEECHSAY__
	#include "MT4.h"
#endif

//#include <windows.h>


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

extern		char	MAGIC_CHAR_MAP[];
extern		short	MAGIC_OPCODE_MAP[];
extern		short	CHAR_ATTR_FLAGS[];
extern		char	KindTBL[];
extern		char	dashruletab[];
extern		char	atruletab[];
extern		char	lruletab[];
extern		char	mruletab[];
extern		char	zruletab[];
extern		char	percentruletab[];
extern		char	bruletab[];
extern		unsigned short	Opcode_To_ASCII[];
extern		short	Divisors[];
extern		short	SuffixType[];
extern		char	AndPhonStr[];
extern		char	PowerStr[];
extern		char	CommaPhonStr[];
extern		char	ControlPhonStr[];
extern		char	NonASCIIPhonStr[];
extern		char	SilencePhonStr[];
extern		char	OSTypeStr[];
extern		char	SuffixTab[];
extern		char	AppleSCII[];
extern		char	DollarPhonStr[];
extern		char	CentPhonStr[];
extern		short	Phon_to_Phon[];
extern		char	ClockPhonStr[];
extern		short	Allo_to_Phon[];
extern		char	OhPhonStr[];


/*--------------*/
/* FrontEnd.c		*/
/*--------------*/
extern	short 	e_StartParse ( voiceVarPtr vv, Ptr theStr, unsigned long byteLen, unsigned long controlFlags );

/*--------------*/
/* BackEnd.c		*/
/*--------------*/
extern 		short	Talk (voiceVarPtr vv);

/*--------------*/
/* fsynth.c		*/
/*--------------*/
extern 		short	Talk (voiceVarPtr vv);
extern		void 	InitDefaultGlobals (voiceVar *vv);
extern		void	Init_Rate_Params (voiceVarPtr vv);
extern		short	NewVoice (voiceVarPtr vv, void *vDat, unsigned char *sample);
extern		void	ResetVoice (voiceVarPtr vv);
extern		void	Start_Talk (voiceVarPtr vv);
extern		void	StartNew_PitchClause (voiceVarPtr vv);
extern		void 	SetVolume (voiceVarPtr vv, long vol);

extern		void	e_Fill_Next_Frame (voiceVarPtr vv);
extern		short 	e_HzToPitch (voiceVarPtr vv, short hz);
extern		short	e_MidiToPitch (short midiNote);
extern		short	e_LogToLin (voiceVarPtr vv, short logVal);
extern		short	e_GetPhon (voiceVarPtr vv, short index);
extern		long	e_GetPhonCtrl (voiceVarPtr vv, short index);


short	e_OpenSpeechChannel (voiceVarPtr vv);
void	e_StopSpeechAt (voiceVarPtr vv, unsigned long whereToPause);
void	e_PauseSpeechAt (voiceVarPtr vv, unsigned long whereToPause);
void	e_ContinueSpeech( voiceVarPtr vv );
short	e_SpeakBuffer( voiceVarPtr vv, Ptr textBuf, long byteLen, long controlFlags );
short	e_GetSpeechStatus (voiceVarPtr vv, SpeechStatusInfo *status);
void	e_GetSpeechRate (voiceVarPtr vv, long *info);
void	e_GetSpeechPitch (voiceVarPtr vv, long *info);
void	e_GetSpeechVolume (voiceVarPtr vv, long *info);
void	e_GetSpeechMod (voiceVarPtr vv, long *info);
void	e_SetSpeechRate (voiceVarPtr vv, long info);
void	e_SetSpeechPitch (voiceVarPtr vv, long info);
void	e_SetSpeechVolume (voiceVarPtr vv, long info);
void	e_SetSpeechMod (voiceVarPtr vv, long info);
void	e_ResetParams (voiceVarPtr vv);
short	e_UseVoice ( voiceVarPtr vv, void *tvPtr, unsigned char *sample );
void	e_ReinitVoice (voiceVarPtr vv);
void	e_SetTempo (voiceVarPtr vv, short tempo);


//---------------------------
// Export
//---------------------------
void	Init_BEFuncPtrs (moduleFuncPtr mfp);


void	Init_BEFuncPtrs (moduleFuncPtr mfp)
{
	mfp->e_OpenSpeechChannel_FUNC 	= (e_OpenSpeechChannel_Ptr) &e_OpenSpeechChannel;
	mfp->e_SpeakBuffer_FUNC 		= (e_SpeakBuffer_Ptr) &e_SpeakBuffer;
	mfp->e_PauseSpeechAt_FUNC 		= (e_PauseSpeechAt_Ptr) &e_PauseSpeechAt;
	mfp->e_StopSpeechAt_FUNC 		= (e_StopSpeechAt_Ptr) &e_StopSpeechAt;
	mfp->e_ContinueSpeech_FUNC 		= (e_ContinueSpeech_Ptr) &e_ContinueSpeech;
	mfp->e_GetSpeechStatus_FUNC 	= (e_GetSpeechStatus_Ptr) &e_GetSpeechStatus;
	mfp->e_GetSpeechRate_FUNC 		= (e_GetSpeechRate_Ptr) &e_GetSpeechRate;
	mfp->e_GetSpeechPitch_FUNC 		= (e_GetSpeechPitch_Ptr) &e_GetSpeechPitch;
	mfp->e_GetSpeechVolume_FUNC 	= (e_GetSpeechVolume_Ptr) &e_GetSpeechVolume;
	mfp->e_GetSpeechMod_FUNC 		= (e_GetSpeechMod_Ptr) &e_GetSpeechMod;
	mfp->e_SetSpeechRate_FUNC 		= (e_SetSpeechRate_Ptr) &e_SetSpeechRate;
	mfp->e_SetSpeechPitch_FUNC 		= (e_SetSpeechPitch_Ptr) &e_SetSpeechPitch;
	mfp->e_SetSpeechVolume_FUNC 	= (e_SetSpeechVolume_Ptr) &e_SetSpeechVolume;
	mfp->e_SetSpeechMod_FUNC 		= (e_SetSpeechMod_Ptr) &e_SetSpeechMod;
	mfp->e_ResetParams_FUNC 		= (e_ResetParams_Ptr) &e_ResetParams;
	mfp->e_UseVoice_FUNC 			= (e_UseVoice_Ptr) &e_UseVoice;
	mfp->e_ReinitVoice_FUNC 		= (e_ReinitVoice_Ptr) &e_ReinitVoice;
	mfp->e_SetTempo_FUNC	 		= (e_SetTempo_Ptr) &e_SetTempo;

	mfp->e_Fill_Next_Frame_FUNC	 	= (e_Fill_Next_Frame_Ptr) &e_Fill_Next_Frame;
	mfp->e_HzToPitch_FUNC	 		= (e_HzToPitch_Ptr) &e_HzToPitch;
	mfp->e_MidiToPitch_FUNC	 		= (e_MidiToPitch_Ptr) &e_MidiToPitch;
	mfp->e_LogToLin_FUNC	 		= (e_LogToLin_Ptr) &e_LogToLin;
	mfp->e_GetPhon_FUNC	 			= (e_GetPhon_Ptr) &e_GetPhon;
	mfp->e_GetPhonCtrl_FUNC	 		= (e_GetPhonCtrl_Ptr) &e_GetPhonCtrl;
}




static void	SetTblAddr (voiceVarPtr vv)
{
	vv->phonPitchTbl 		= (short*)&phonPitchTbl;
	vv->phonTypeTbl 		= (short*)&PhonTypeTbl;
	vv->logToLinPtr 		= (short*)&LogToLin;
	vv->logOf2Tbl 			= (short*)&logOf2Tbl;
	vv->SineWavePtr 		= (unsigned char*)&SineWave;
	vv->XlateToAllo 		= (short*)&XlateToAllo;
	vv->phonFlags2 			= (long*)&PhonFlags2;
	vv->BoundryDurTbl 		= (short*)&BoundryDur;
	vv->ExpOf2Tbl	 		= (unsigned short*)&ExpOf2Tbl;
	vv->OctFreqTbl	 		= (unsigned short*)&OctFreqTbl;
	vv->maxDurTbl 			= (short*)&MaxDurTbl;
	vv->minDurTbl 			= (short*)&MinDurTbl;

	vv->MagicCharMap 		= (short*)&MAGIC_CHAR_MAP;
	vv->MagicOpcodeMap 		= (short*)&MAGIC_OPCODE_MAP;
	vv->CharAttr 			= (TokenAttr*)&CHAR_ATTR_FLAGS;
	vv->kind 				= (unsigned char*)&KindTBL;
	vv->dashruletab 		= (unsigned char*)&dashruletab;
	vv->atruletab 			= (unsigned char*)&atruletab;
	vv->lruletab 			= (unsigned char*)&lruletab;
	vv->mruletab 			= (unsigned char*)&mruletab;
	vv->zruletab 			= (unsigned char*)&zruletab;
	vv->percentruletab 		= (unsigned char*)&percentruletab;
	vv->bruletab 			= (unsigned char*)&bruletab;
	vv->Opcode_To_ASCII		= (unsigned short*)&Opcode_To_ASCII;
	vv->AndPhonStr			= (unsigned char*)&AndPhonStr;
	vv->PowerStr			= (unsigned char*)&PowerStr;
	vv->CommaPhonStr		= (unsigned char*)&CommaPhonStr;
	vv->ControlPhonStr		= (unsigned char*)&ControlPhonStr;
	vv->NonASCIIPhonStr		= (unsigned char*)&NonASCIIPhonStr;
	vv->SilencePhonStr		= (unsigned char*)&SilencePhonStr;
	vv->OSTypeStr			= (unsigned char*)&OSTypeStr;
	vv->divisorsPtr			= (unsigned short*)&Divisors;
	vv->SuffixTab			= (unsigned char*)&SuffixTab;
	vv->SuffixType			= (short*)&SuffixType;
	vv->AppleSCII			= (unsigned char*)&AppleSCII;
	vv->DollarPhonStr		= (unsigned char*)&DollarPhonStr;
	vv->CentPhonStr			= (unsigned char*)&CentPhonStr;
	vv->ClockPhonStr		= (unsigned char*)&ClockPhonStr;
	vv->OhPhonStr			= (unsigned char*)&OhPhonStr;
	vv->Allo_to_Phon		= (short*)&Allo_to_Phon;
	vv->Phon_to_Phon		= (short*)&Phon_to_Phon;
}	





void	InitTheGlobals (voiceVarPtr vv)
{
	SetTblAddr (vv);
	
	vv->starting_New_Phon = false;
	
	e_SetTempo (vv, 120);				/* default tempo = 120 bpm	*/
}






/* =========================================================================================*/
/* 																							*/
/*  NAME: 	e_OpenSpeechChannel																*/
/* 																							*/
/*  FUNCTION:																				*/
/*			Initilize channel data.															*/
/* 																							*/
/*  INPUTS:																					*/
/*			vv	- Pointer to channel globals												*/
/* 																							*/
/*  OUTPUTS:																				*/
/*			Return "kNoError" if startup is successful.										*/
/* 																							*/
/*  DESCRIPTION:																			*/
/*			1. Get channel global memory.													*/
/*			2. Init channel global memory.													*/
/* 																							*/
/* 																							*/
/*  NOTES:																					*/
/* 																							*/
/* 																							*/
/*  DATE			VERSION		CHANGE FLAG		WHO		WHY									*/
/*  ----------------------------------------------------------------------------------------*/
/*  2/25/93		1.00							MC		Original code						*/
/* 																							*/
/* =========================================================================================*/
short	e_OpenSpeechChannel (voiceVarPtr vv)
{
	short			error;

	error = kNoError;
	/*------------------------------------------------	*/
	/* Get global memory for this channel	*/
	/*------------------------------------------------	*/
	

	vv->hash		= (short*) vv->Rules;
	vv->rule		= (unsigned char*) (vv->Rules + (26*2));
	vv->Busy = false;								/* we're not speaking yet	*/
	
	/*------------------------------------------------	*/
	/* Initilize Speech Channel	*/
	/*------------------------------------------------	*/
	InitTheGlobals(vv);
	

	(*(vv->funcList->e_InitFE_FUNC)) (vv);

FAILURE:
	return (error);
}






void	e_StopSpeechAt (voiceVarPtr vv, unsigned long whereToPause) 
{
	#pragma unused ( whereToPause )

	vv->outputPaused = true;
	(*(vv->funcList->e_AbortParse_FUNC)) (vv);						/* kill current FE parse	*/
	vv->speechState = kStopped;
}


void	e_PauseSpeechAt (voiceVarPtr vv, unsigned long whereToPause) 
{
	#pragma unused ( whereToPause )

	vv->outputPaused = true;
	vv->speechState = kPaused;
	vv->Busy = false;
	
	vv->lastWordStart 				= vv->nLastWordStart;
	vv->pFilter_Out1_Save1 			= vv->pFilter_Out1_Save2;
	vv->pFilter_Out2_Save1			= vv->pFilter_Out2_Save2;
	vv->down_Ramp_Offset_Save1		= vv->down_Ramp_Offset_Save2;
	vv->fallRise_Offset_Save1		= vv->fallRise_Offset_Save2;
	vv->fallRise1_Offset_Save1		= vv->fallRise1_Offset_Save2;
	vv->stress_Target_Save1			= vv->stress_Target_Save2;
	vv->punct_Offset_Save1			= vv->punct_Offset_Save2;
	
	vv->next_PitchBuf_Time_Save1	= vv->next_PitchBuf_Time_Save2;	
	vv->phon_Index_Targ_Save1		= vv->phon_Index_Targ_Save2;
	vv->phon_Index_CP_Save1			= vv->phon_Index_CP_Save2;
	vv->pitchBuf_Out_Index_Save1	= vv->pitchBuf_Out_Index_Save2;
	vv->time_IntoPhon_CP_Save1		= vv->time_IntoPhon_CP_Save2;
	vv->cur_Phon_Dur_CC_Save1		= vv->cur_Phon_Dur_CC_Save2;
	vv->cur_PhonDur_CP_Save1		= vv->cur_PhonDur_CP_Save2;
	vv->time_IntoPhon_Targ_Save1	= vv->time_IntoPhon_Targ_Save2;
	vv->cur_PitchBuf_Time_Save1		= vv->cur_PitchBuf_Time_Save2;
	vv->cmdBufCount_Save1 			= vv->cmdBufCount_Save2;
	vv->songIndex_Save1 			= vv->songIndex_Save2;
	vv->VP_baselinePitch_Save1 		= vv->VP_baselinePitch_Save2;
}







/* ==========================================================================================	*/
/* 	*/
/*  NAME: 	e_ContinueSpeech	*/
/* 	*/
/*  FUNCTION:	*/
/*			Resume after a HaltSpeechAt.	*/
/* 	*/
/*  INPUTS:	*/
/*			locals - instance local storage	*/
/* 	*/
/*  OUTPUTS:	*/
/*			outputPaused:		Set to FALSE 	*/
/* 	*/
/* 	*/
/*  DESCRIPTION:	*/
/* 	*/
/* 	*/
/*  NOTES:	*/
/* 	*/
/* 	*/
/*  DATE			VERSION		CHANGE FLAG		WHO		WHY	*/
/*  -----------------------------------------------------------------------------------------	*/
/*  2/25/93		1.00							MC		Original code	*/
/* 	*/
/* ==========================================================================================	*/

void	e_ContinueSpeech( voiceVarPtr vv ) 
{
	if (vv->outputPaused && (vv->speechState == kPaused) )
		{
		vv->outputPaused = false;
		vv->speechState = kNormal;
		vv->Busy = true;
		
		vv->cur_PhonBuf_Index_CF = vv->nLastWordStart;

		vv->pFilter_Out1 		= vv->pFilter_Out1_Save2;
		vv->pFilter_Out2		= vv->pFilter_Out2_Save2;
		vv->down_Ramp_Offset	= vv->down_Ramp_Offset_Save2;
		vv->fallRise_Offset		= vv->fallRise_Offset_Save2;
		vv->fallRise1_Offset	= vv->fallRise1_Offset_Save2;
		vv->stress_Target		= vv->stress_Target_Save2;
		vv->punct_Offset		= vv->punct_Offset_Save2;
		
		vv->next_PitchBuf_Time	= vv->next_PitchBuf_Time_Save2;	
		vv->phon_Index_Targ		= vv->phon_Index_Targ_Save2;
		vv->phon_Index_CP		= vv->phon_Index_CP_Save2;
		vv->pitchBuf_Out_Index	= vv->pitchBuf_Out_Index_Save2;
		vv->time_IntoPhon_CP	= vv->time_IntoPhon_CP_Save2;
		vv->cur_Phon_Dur_CC		= vv->cur_Phon_Dur_CC_Save2;
		vv->cur_PhonDur_CP		= vv->cur_PhonDur_CP_Save2;
		vv->time_IntoPhon_Targ	= vv->time_IntoPhon_Targ_Save2;
		vv->cur_PitchBuf_Time	= vv->cur_PitchBuf_Time_Save2;
		vv->cmdBufCount 		= vv->cmdBufCount_Save2;
		vv->songIndex 			= vv->songIndex_Save2;
		vv->VP_baselinePitch 	= vv->VP_baselinePitch_Save2;

		Start_Talk(vv);
		}
}






/* ==========================================================================================	*/
/* 	*/
/*  NAME: 	e_SpeakBuffer	*/
/* 	*/
/*  FUNCTION:	*/
/*			Check if request code is supported.	*/
/* 	*/
/*  INPUTS:	*/
/*			locals - instance local storage	*/
/*			1st param	- request code	*/
/* 	*/
/*  OUTPUTS:	*/
/*			Return "true" if request is supported.	*/
/* 	*/
/*  DESCRIPTION:	*/
/* 	*/
/* 	*/
/*  NOTES:	*/
/* 	*/
/* 	*/
/*  DATE			VERSION		CHANGE FLAG		WHO		WHY	*/
/*  -----------------------------------------------------------------------------------------	*/
/*  2/25/93		1.00							MC		Original code	*/
/* 	*/
/* ==========================================================================================	*/

short	e_SpeakBuffer( voiceVarPtr vv, Ptr textBuf, long byteLen, long controlFlags ) 
{
	short		error;

	error = kNoError;
	if (!(controlFlags & kNoSpeechInterrupt))	/* stop any active speech before speaking new buffer	*/
		{
		if (vv->Busy)							/* back-end is still working	*/
			e_StopSpeechAt (vv, kImmediate);
		}
	else if (vv->Busy)
		{
		error = synthNotReady;
		goto NOSPEECH;
		}

	if ( (!textBuf) || (*textBuf == 0) )		/* NULL text buffer ptr	*/
		{
		error = kNothingToSpeak;
		goto NOSPEECH;
		}

	vv->Busy = true;						/* set status to speaking from here on	*/

	error = e_StartParse (vv, textBuf, byteLen, controlFlags);


	if (!error)
		{
		error = Talk (vv);
		if (error)
			goto NOSPEECH;
	
		if (controlFlags & kPreflightThenPause)
			{
			//e_PauseSpeechAt (vv, 0); 
			}
		}

	return (error);

NOSPEECH:
	vv->Busy = false;
	return (error);
}










short	e_GetSpeechStatus (voiceVarPtr vv, SpeechStatusInfo *status)
{
	short		error = kNoError;
	
	if (!status)								/* we were passed a NULL ptr	*/
		error = kParamError;
	else
		{
		if (vv->speechState != kStopped)			/* not stopped, poll back-end for real busy & phoneme status	*/
			{
			status->outputBusy = vv->Busy; 				/* return copy of our current status	*/
			/*if (vv->Busy)	*/
			/*	status->phonemeCode = vv->currentPhon;	*/
			/*else	*/
				status->phonemeCode = 0;					/* speech is not being generated	*/
			}
		else									/* we are stopped, we'll fill in the record ourselves	*/
			{
			status->outputBusy 		= false;
			status->phonemeCode 	= 0;
			}

		/* In any event, fill out the fields we know about.	*/

		status->outputPaused 	= (vv->speechState == kPaused);
		status->inputBytesLeft 	= vv->StrEOF - vv->StrPos; 	/* number of bytes left to process	*/
		}
	
	return (error);
}





void	e_GetSpeechRate (voiceVarPtr vv, long *info)
{

	if (vv->singing)
		*info = vv->tempo << 16;				/* 0xxx.0000	*/
	else
		*info = vv->speech_Rate << 16;				/* 0xxx.0000	*/
}


void	e_GetSpeechPitch (voiceVarPtr vv, long *info)
{
	*info = ((vv->VP_baselinePitch * 12) + kMIDI_50HZ) << 8;			/* 00nn.ff00	*/
}


void	e_GetSpeechVolume (voiceVarPtr vv, long *info)
{

	*info = vv->user_Volume << 8;					/* 0000.xx00	*/
}


void	e_GetSpeechMod (voiceVarPtr vv, long *info)
{
	*info = vv->VP_pitchRange * 50;					/* 0xxx.0000	*/
}






void	e_SetSpeechRate (voiceVarPtr vv, long info)
{

	if (vv->singing)
		{
		vv->tempo = info >> 16;				/* extract integer part (xXXX.xxxx -> 0XXX);	*/
		e_SetTempo (vv, vv->tempo);
		}
	else
		{
		vv->speech_Rate = info >> 16;				/* extract integer part (xXXX.xxxx -> 0XXX);	*/
		Init_Rate_Params (vv);
		}
}


void	e_SetSpeechPitch (voiceVarPtr vv, long info)
{
	long		param;

	param = info >> 8;							/* 00XX.ffff -> XX.ff	*/
	if (param < kMIDI_50HZ)
		param = 0;
	else
		param -= kMIDI_50HZ;
	vv->voiceNaturalPitch = ((param * kOneTwelfth) + kPointFive) >> 16;
	vv->VP_baselinePitch = vv->voiceNaturalPitch;
}


void	e_SetSpeechVolume (voiceVarPtr vv, long info)
{
	SetVolume (vv, info);
}


void	e_SetSpeechMod (voiceVarPtr vv, long info)
{
	if (info < 0)
		info = 0;
	else if (info > (100 << 16))
		info = 100 << 16;
	vv->VP_pitchRange = info / 50;
}


void	e_ResetParams (voiceVarPtr vv)
{
	(*(vv->funcList->e_ResetFE_FUNC)) (vv);
	ResetVoice (vv);
}




short	e_UseVoice ( voiceVarPtr vv, void *tvPtr, unsigned char *sample ) 
{
	short		error = kNoError;

	(*(vv->funcList->synth_Init_FUNC)) (vv);
	
	error = NewVoice (vv, tvPtr, sample);

	return (error);
}



void	e_ReinitVoice (voiceVarPtr vv)
{
	ResetVoice (vv);
}





/*------------------*/
/*	1	= 16th		*/
/*	2	= .16th		*/
/*	3	= 8th		*/
/*	4	= .8th		*/
/*	5	= 4th		*/
/*	6	= .4th		*/
/*	7	= 2nd		*/
/*	8	= .2nd		*/
/*	9	= 1st		*/
/*	10	= .1st		*/
/*	11	= ??		*/
/*	12	= .??		*/
/*------------------*/
void	e_SetTempo (voiceVarPtr vv, short tempo)
{
	#define kBPM (((60/4) * 1000) / kFrameTime)		/* milliseconds	(4 = 16th note)	*/
	short	i,j;
	short	note_16th;
	
	
	if (tempo < 20)
		tempo = 20;
	else if (tempo > 240)
		tempo = 240;

	vv->tempo = tempo;
	
	note_16th = kBPM / tempo;
	
	vv->Note_Times[0] = note_16th;		/* NOT USED	*/
	j = note_16th;						/* @@@@ 16th note	*/
	for (i = 1; i < 12; i++)
		{
		vv->Note_Times[i++] = j;
		vv->Note_Times[i] = j + (j >> 1);	/* dotted note	*/
		j <<= 1;
		}
}


























