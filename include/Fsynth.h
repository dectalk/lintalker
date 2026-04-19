#ifndef __FSYNTH__
	#define __FSYNTH__

#include <stdint.h>



#define	kUseHarm		0
#define	kUseSnd			1
#define	kUseSyncSnd		2

#define	kMaleTbls	0
#define	kFemaleTbls	1


#define kMaxBandWidth 1225			/* 1280 - 50 - 5	*/
#define kPrecision 13				/* 13 bit fraction in Fixed format (8192)	*/
#define kNoiseLen 2048
#define kSizeOf1xTbl 100
#define kNoValue (-1)


/*------------------*/
/* for Get_Locus	*/
/*------------------*/
#define C_V_type 0		/* consonant to vowel boundry	*/
#define V_C_type 1		/* vowel to consonant boundry	*/


/*------------------*/
/* Control Types	*/
/*------------------*/

#define	kFreqType		0
#define	kBWType 		1
#define	kFNZType 		2
#define	kSourceAmpType 	3
#define	kResonAmpType 	4
#define kNoNasal_Freq	350
#define kNasal_Freq		500


#if FLOAT_SYNTH_MT3
	#define kNoiseGain (0.4)
#else
	#define kNoiseGain 3200				/* kNoiseGain / kPrecision = 3200 / 8192 = 0.4	*/
#endif


#if FLOAT_SYNTH_MT3
	#define kOnePtOh 1.0
#else
	#define kOnePtOh 0x2000				/* 1.0 in fixed format	*/
#endif




struct ControlBlock
{
	short	curP_START_Targ;	/* 0		*/
	short	curTarget_TIME;		/* 2		*/
	short	curTarget_STEP;		/* 4		*/
	short	curTarget_OFFS;		/* 6		*/
	short	HEAD_offs;			/* 8	(shrinks)	*/
	short	HEAD_step;			/* 10		*/
	short	TAIL_offs;			/* 12	(grows)	*/
	short	TAIL_step;			/* 14		*/
	short	TAIL_START_time;	/* 16		*/
	short	onset_END_TIME;		/* 18		*/
	short	onset_VAL;			/* 20		*/
	short	nextP_START_Targ;	/* 22		*/
	short	prevP_END_Targ;		/* 24		*/
	short	curP_END_Targ;		/* 26		*/
	short	*ptrToTargetList;	/* 28	*/
	short	lastVal;
};
typedef struct ControlBlock ControlBlock;
typedef ControlBlock *ControlBlockPtr;



struct Frame
{
	short	Av;
	short	Af;
	short	f0;
	short	f1;
	short	f2;
	short	f3;
	short	a2;
	short	a3;
	short	a4;
	short	a5;
	short	a6;
	short	FNZ;
	short	AB;
	short	bw1;
	short	bw2;
	short	bw3;
	short	phon_Edge;
	long	marker;
};
typedef struct Frame Frame;
typedef Frame *FramePtr;





#pragma pack (push, 1)	// BYTE aligned
struct voiceData
{
	short	pitch;			/* 50 - 500hz	*/
	short	pitchRange;		/* 0 - 100	*/
	short	stressGain;		/* 0 - 100	*/
	short	rate;
	short	voice;
	short	vGain;			/* 0 - 100	*/
	short	aGain;			/* 0 - 100	*/
	short	aCycle;			/* 0 - 255	*/
	
	short	f4_Freq;
	short	f4_BW;
	short	f4p_Freq;
	short	f4p_BW;
	short	f5p_Freq;
	short	f5p_BW;
	short	f6p_Freq;
	short	f6p_BW;

	short	nasal_Base;
	short	nasal_targ;
	short	nasal_BW;
	short	locus;
	short	bwGain1;
	short	bwGain2;
	short	bwGain3;
	short	f1_Offset;
	short	f2_Offset;
	short	f3_Offset;
	short	chorus;
	short	nGain;
	short	sPitch;			/* MIDI		*/
	short	sGain;			/* 0 - 100	*/
	short	AsperW;			/* 0 - 2	*/
	short	voiceVers;
	
	short	riseAmt;
	short	fallAmt;
	short	riseAmt1;
	short	fallAmt1;
	int32_t	assertiveness;
	short	baselineFall;
	short	quickness;
	short	pitchCmdStep;
	short	durCmdStep;
	int32_t	down_Ramp_Step;
	short	stressDurTime;

	short	tempo;

	short	waveType;
	short	vWave[48];
	short	vWave1[48];
	short	sndID;
	short	vowelSync;
	int32_t	loopPoint;
	short	customForm;

	short	nasalAmt;
	short	vibratoDepth1;
	short	vibratoDepth2;
	short	vibratoFreq;
	short	intonation;
	short	portamento;
	short	emphVoice;
	short	rvbDelay;
	short	rvbDepth;
	short	rvbWetDry;		// UNUSED

	int32_t	free1;
	int32_t	free2;
	int32_t	free3;
	int32_t	free4;
	int32_t	free5;
	int32_t	free6;
	int32_t	free7;
	int32_t	free8;
	
};
#pragma pack (pop)

typedef struct voiceData voiceData;
typedef voiceData *voiceDataPtr;




#define kF1 	0
#define kF2 	1
#define kF3 	2
#define kBW1	3
#define kBW2 	4
#define kBW3 	5
#define kFNZ 	6
#define kAV 	7
#define kAF 	8
#define kAp2 	9
#define kAp3 	10
#define kAp4 	11
#define kAp5 	12
#define kAp6 	13
#define kAB 	14

#define kNumOfBlocks kAB+1



#if		FLOAT_SYNTH_MT3
	typedef double 			rUSC;
	typedef double			rShort;
	typedef double			rUSShort;
	typedef double 			rLong;
#else
	typedef unsigned char 	rUSC;
	typedef short			rShort;
	typedef unsigned short	rUSShort;
	typedef long 			rLong;
#endif


//---------------------------
// 'nextSampBuf' constants
//---------------------------
	#define kBuf1 0
	#define kBuf2 1
	
//---------------------------
// 'curFrameBuf' constants
//---------------------------
	#define kFrame1 0
	#define kFrame2 1

//---------------------------
// Av interpolator
//---------------------------
#define	kAmpStepRes 16						/* Step size 00XX.xxxx	*/


//---------------------------
// Reverb constants
//---------------------------
#define kTapFactor	1
#define kNumOfTaps	8
#define kTap1		(404 * kTapFactor)
#define kTap2		(1058 * kTapFactor)
#define kTap3		(1362 * kTapFactor)
#define kTap4		(2318 * kTapFactor)
#define kTap5		(2909 * kTapFactor)
#define kTap6		(3723 * kTapFactor)
#define kTap7		(4030 * kTapFactor)
#define kTap8		(4096 * kTapFactor)
#define kMaxTap		kTap8


#if		FLOAT_SYNTH_MT3
	#define mMul2(x,y,s)	(x * y)
	#define mRatio(x,y,s)	(x / y)
	#define mScale(x,s)		(x)
	#define mUnScale(x,s)	(x)
	#define mDiv(x,y,s)		(x / y)
#else
	#define mMul2(x,y,s)	((x * y) >> s)
	#define mRatio(x,y,s)	((x << s) / y)
	#define mScale(x,s)		(x << s)
	#define mUnScale(x,s)	(x >> s)
	#define mDiv(x,y,s)		(x >> s)
#endif



struct formantVar
{
//*******************************
// FORMANT CONTROL
//*******************************
	short		*EnvelopeListTbl;		/* ptr to 'EnvelopeListTbl'$$	*/
	short		*f1FreqTblM;			/* ptr to 'f1FreqTblM'	*/
	short		*f2FreqTblM;			/* ptr to 'f2FreqTblM'	*/
	short		*f3FreqTblM;			/* ptr to 'f3FreqTblM'	*/
	short		*b1FreqTblM;			/* ptr to 'b1FreqTblM'	*/
	short		*b2FreqTblM;			/* ptr to 'b2FreqTblM'	*/
	short		*b3FreqTblM;			/* ptr to 'b3FreqTblM'	*/
	short		*avVolTblM;				/* ptr to 'avVolTblM'	*/
	short		*f1FreqTblF;			/* ptr to 'f1FreqTblF'	*/
	short		*f2FreqTblF;			/* ptr to 'f2FreqTblF'	*/
	short		*f3FreqTblF;			/* ptr to 'f3FreqTblF'	*/
	short		*b1FreqTblF;			/* ptr to 'b1FreqTblF'	*/
	short		*b2FreqTblF;			/* ptr to 'b2FreqTblF'	*/
	short		*b3FreqTblF;			/* ptr to 'b3FreqTblF'	*/
	short		*avVolTblF;				/* ptr to 'avVolTblF'	*/
	short		*CtrlBlockTypeTbl;		/* ptr to 'CtrlBlockTypeTbl'$$	*/
	short		*DefaultTargTbl;		/* ptr to 'DefaultTargTbl'$$	*/
	short		*Rank_FWD_Tbl;			/* ptr to 'Rank_FWD_Tbl'$$	*/
	short		*Rank_BKWD_Tbl;			/* ptr to 'Rank_BKWD_Tbl'$$	*/
	short		*NoiseIndexTbl;			/* ptr to 'NoiseIndexTbl'$$	*/
	short		*Front_Loci_Tbl;		/* ptr to 'Front_Loci_Tbl'$$	*/
	short		*Mid_Loci_Tbl;			/* ptr to 'Mid_Loci_Tbl'$$	*/
	short		*Back_Loci_Tbl;			/* ptr to 'Back_Loci_Tbl'$$	*/
	short		*Male_NoiseAmpTbl;		/* ptr to 'Male_NoiseAmpTbl'$$	*/
	short		*Female_NoiseAmpTbl;	/* ptr to 'Female_NoiseAmpTbl'$$	*/
	short		*Male_Loci_Tbl;			/* ptr to 'Male_Loci_Tbl'$$	*/
	short		*Female_Loci_Tbl;		/* ptr to 'Female_Loci_Tbl'$$	*/
	short		*BurstDurTbl;			/* ptr to 'BurstDurTbl'$$	*/
	long		*One_Over_X_Tbl;		/* ptr to 'One_Over_X_Tbl'$$	*/
	short		*MaleEnvelopeListTbl;	/* ptr to 'EnvelopeListTbl'	*/
	short		*FemaleEnvelopeListTbl;	/* ptr to 'EnvelopeListTbl'	*/

	short		*a_EnvelopeListTbl;		/* ptr to 'EnvelopeListTbl'	*/
	short		*a_f1FreqTblM;			/* ptr to 'f1FreqTblM'	*/
	short		*a_f2FreqTblM;			/* ptr to 'f2FreqTblM'	*/
	short		*a_f3FreqTblM;			/* ptr to 'f3FreqTblM'	*/
	short		*a_b1FreqTblM;			/* ptr to 'b1FreqTblM'	*/
	short		*a_b2FreqTblM;			/* ptr to 'b2FreqTblM'	*/
	short		*a_b3FreqTblM;			/* ptr to 'b3FreqTblM'	*/
	short		*a_avVolTblM;			/* ptr to 'avVolTblM'	*/

	short		*voice_Formants[6];				/* f1 - bw3	*/
	short		*voice_av_Tbl;
	short		*voice_NoiseAmp_Tbl;
	short		*voice_Locus_Tbl;

	short		locusOffset;

	short		cur_Phon_MaxDur_CF;
	long		cur_Phon_PctOfMaxDur_CF;
	long		cur_Phon_PctOfMaxDur1_CF;	/* gravitate to 1.0	*/
	long		cur_Phon_PctOfMaxDur2_CF;

	short		trans_TIME;
	short		trans_LEVEL;
	short		trans_TIME_f2;
	short		trans_LEVEL_f2;

	short		f2_HEAD_offset;			/* grows	*/
	short		f2_HEAD_step;
	short		f2_TAIL_START_time;
	short		f2_TAIL_offset;			/* shrinks	*/
	short		f2_TAIL_step;

	short		big_Bang;

	short			cur_ControlBlk_Index;
	ControlBlock	controlBlockArray[kNumOfBlocks];
	short			controlData[kNumOfBlocks];

	short		diphEntryArray[100];		/* pair entries of: -num of steps-, -step size-	*/
	short		*next_DiphEntry;		/* ptr to next free loc	*/
	


//*******************************
// FORMANT ENGINE
//*******************************
	//---------------------------
	// INPUT: Frame buffers
	//---------------------------
	Frame			frameBuf1;
	Frame			frameBuf2;
	short			curFrameBuf;		// Set to kFrame1 or kFrame2

	//---------------------------
	// Buzz data
	//---------------------------
	short			glotType;				// Set to kUseHarm, kUseSnd or kUseSyncSnd
	rShort			voiceWaveform[256];		// voiced waveform #1	
	rShort			voiceWaveform1[256];	// voiced waveform #2
	rShort			*SineWave15Ptr;			// Used to synthesize buzz wave in InvDFT
	
	
	//---------------------------
	// Buzz excitation
	//---------------------------
	Fixed			glotInc;			/* wave index #1 (sets pitch)	*/
	Fixed			glotInc1;			/* wave index #2 (sets pitch)	*/
	Fixed			glotIndex;			/* accum for voiced waveform #1	*/
	Fixed			glotIndex1;			/* accum for voiced waveform #2	*/
			
	//---------------------------
	// Noise excitation
	//---------------------------
	short			noiseIndex;			/* index into noise waveform	*/
	rShort			setNoiseGain;
	
	//---------------------------
	// Wavesample excitation
	//---------------------------
	unsigned char	*SampleWave;		/* Ptr to wavesample				*/
	long			sampleLength;		/* Length of wavesample				*/
	Fixed			sampleInc;			/* Wavesample index #1 (sets pitch)	*/
	Fixed			sampleInc1;			/* Wavesample index #2 (sets pitch)	*/
	unsigned long	sndIndex;			/* accumulator for wavesample	#1	*/
	unsigned long	sndIndex1;			/* accumulator for wavesample	#2	*/
	long			loopPoint;			// Loop from here to end of sample
	short			sync_On_Vowel;		// Restart sample at every vowel
	short			VP_offsetPitch;		// F0 offset for wavesample pitch
	
	//---------------------------
	// Av interpolator
	//---------------------------
	rLong			ampStep;			/* Step size (fixed point)			*/
	rLong			lastAmp;			/* Av of last frame (fixed point)	*/
	rLong			curAmp_Full;		/* Current amp (fixed point)		*/
	rShort			curAmp;				/* Current amp (integer)			*/

	//---------------------------
	// Glottal excitation gain
	//---------------------------
	rShort			Af;					// Aspiration
	rShort			Av;					// Voicing
	rShort			wavesampleGain;		// Wavesample

	//---------------------------
	// Breath parameters
	//---------------------------
	rShort			breathGain;			// Gain
	short			breathCycle;		// Duty cycle
	rUSC			*breathWave;		// Waveform
	
	//---------------------------
	// Voice parameters
	//---------------------------
	short			voiceF1Gain;		// F1 offset
	short			voiceF2Gain;		// F2 offset
	short			voiceF3Gain;		// F3 offset
	long			voiceBWgain1;		// B1 gain (fixed point)
	long			voiceBWgain2;		// B2 gain (fixed point)
	long			voiceBWgain3;		// B3 gain (fixed point)
	short			voiceMinBW;			// Min BW clip value
	short			voice_F4_Freq;		// Cascaded F4
	short			voice_F4_BW;		//    "
	short			voiceChorus;		// Chorus flag
	rShort			voiceNoiseGain;		// Parallel bank noise gain
	short			f4_Par;				// Parallel F4
	short			bw4_Par;			//    "
	short			f5_Par;				// Parallel F5
	short			bw5_Par;			//    "
	short			f6_Par;				// Parallel F6
	short			bw6_Par;			//    "
	short			fNP;				// Nasal pole freq
	short			bNP;				// Nasal pole bw
	short			nasalBaseFreq;		// Non-nasalization freq
	short			nasalTargFreq;		// Nasalization freq
	short			nasalAmt;			// Nasal zero offset	
	short			hfEmph;				// High-freq emphasize flag
	short			voice_Num;			// Male or female
	long			*FormTables;		/* voice formant tables	*/
	
	/*------------------------------*/
	/* Filter Bank tables			*/
	/*------------------------------*/
	rShort			*CosTblPtr;						/* ptr to 'CosTbl'		*/
	rShort			*BcoeffTblPtr;					/* ptr to 'BcoeffTbl'	*/
	rShort			*CcoeffTblPtr;					/* ptr to 'CcoeffTbl'	*/
	Fixed			*TOPtr;							/* ptr to 'TopOctave'	*/
	rUSC			*NoiseWavePtr;					/* ptr to 'NoiseWave'	white noise	*/
	rUSC			*BandNoisePtr;					/* ptr to 'BandNoise'	band-pass filtered @1000hz	*/
	rUSC			*HPNoisePtr;					/* ptr to 'HPNoise'		hi-pass-pass filtered @1000hz	*/

	//---------------------------
	// Filter coefficients
	//---------------------------
	rShort			Acoeff1,Bcoeff1,Ccoeff1;		/* cascade F1			*/
	rShort			Acoeff2,Bcoeff2,Ccoeff2;		/* parallel/cascade F2	*/
	rShort			Acoeff3,Bcoeff3,Ccoeff3;		/* parallel/cascade F3	*/
	rShort			Acoeff4,Bcoeff4,Ccoeff4;		/* cascade F4			*/
	rShort			Acoeff4p,Bcoeff4p,Ccoeff4p;		/* parallel F4			*/
	rShort			Acoeff5,Bcoeff5,Ccoeff5;		/* parallel F5			*/
	rShort			Acoeff6,Bcoeff6,Ccoeff6;		/* parallel F6			*/
	rShort			AcoeffNZ,BcoeffNZ,CcoeffNZ;		/* nasal zero			*/
	rShort			AcoeffNP,BcoeffNP,CcoeffNP;		/* nasal pole			*/
	
	//---------------------------
	// IIR delay taps
	//---------------------------
	rShort			Na1,Nb1;						/* cascade F1	*/
	rShort			Na2,Nb2;						/* cascade F2	*/
	rShort			Na3,Nb3;						/* cascade F3	*/
	rShort			Na4,Nb4;						/* cascade F4	*/
	rShort			Na5,Nb5;						/* parallel F5	*/
	rShort			Na6,Nb6;						/* parallel F6	*/
	rShort			Na2a,Nb2a;						/* parallel F2	*/
	rShort			Na3a,Nb3a;						/* parallel F3	*/
	rShort			Na4a,Nb4a;						/* parallel F4	*/
	rShort			NaNZ,NbNZ;						/* nasal zero	*/
	rShort			NaNP,NbNP;						/* nasal pole	*/

	//---------------------------
	// parallel bank input gains
	//---------------------------
	rShort			amp2,amp3;
	rShort			amp4;
	rShort			amp5,amp6;
	rShort			ab;
		
	//---------------------------
	// Reverb
	//---------------------------
	short			tapBuffer[kNumOfTaps];			// Tap indexes
	rShort			*delayBuffer;					// Delay tank
	short			maxRvbDelay;					// Tank size
	short			delay_Index;					// current index into reverb tank
	rShort			reverbDepth;					// Dry/wet mix
	long			reverbDelay;					// scales taps
	short			addReverb;						// reverb on/off flag

	//---------------------------
	// Misc
	//---------------------------
	rShort			lastSample;						// emphasis differential
	rLong			lastRevbSample;					// lowpass averaging for reverb
	rShort			lastnSamp;						// 22 KHZ interpolation
	voiceData		*IntervalVoices;
	voiceData		vd;
	short			Is16BitSound;					// 8/16 bit output flag
	rShort			speechVolume;
};
typedef struct formantVar formantVar;
typedef formantVar *formantVarPtr;


#endif