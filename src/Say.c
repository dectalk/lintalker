#ifndef __SPEECHSAY__
	#include "MT4.h"
#endif

#ifndef __FSYNTH__
	#include "Fsynth.h"
#endif


// BackEnd.c

void	e_Fill_Next_Frame (voiceVarPtr vv);


extern	void	SetFormAddr (voiceVarPtr vv);


void	InitSay (voiceVarPtr vv);
void	Start_The_Speech (voiceVarPtr vv);
void	SayFrame (voiceVarPtr vv);
void	InitVoice (voiceVarPtr vv, voiceDataPtr vd);
void	SaveFrame (voiceVarPtr vv);
void	InsertSample (voiceVarPtr vv, voiceDataPtr tvPtr);

void	synth_ResetVoice (voiceVarPtr vv);
short	synth_NewVoice (voiceVarPtr vv, void *vDat, unsigned char *sample);
void 	synth_SetVolume (voiceVarPtr zz, short vol);
void	synth_FillNextSampBuffer (voiceVarPtr vv);





#if 0
void	SavePCM (voiceVarPtr vv)
{
	unsigned char *fName = "\ptalk.wave";
	short			fID;
	long			len;
	OSErr			rslt;

	rslt = Create(fName, 0, 'SFX!', 'FSSD');
	rslt = FSOpen(fName, 0, &fID);
	len = vv->waveIndex-1;
	rslt = FSWrite(fID, &len, (Ptr)vv->waveP);
	rslt = FSClose(fID);
}
#endif




void SetNextBuffer (voiceVarPtr vv)
{

	if (vv->nextSampBuf == kBuf2)
		{
		vv->sampleBuffer = vv->sampleBuffer1;
		vv->nextSampBuf = kBuf1;					/* use 'sampleBuffer1' next time	*/
		}
	else
		{
		vv->sampleBuffer = vv->sampleBuffer2;
		vv->nextSampBuf = kBuf2;					/* use 'sampleBuffer2' next time	*/
		}
}




void	FillSampBuf (voiceVarPtr vv)
{
	
	vv->waveIndex = 0;
	for (vv->curSndFrame = 0; vv->curSndFrame < kSampBufGroup; vv->curSndFrame++)
		{
		if (vv->speakState == kSpeakLastFrame)
			{
			SayFrame (vv);
			(*(vv->funcList->_i_Last_Snd_Buffer_FUNC)) (vv);
			break;
			}
		else
			{
			SayFrame (vv);
			e_Fill_Next_Frame (vv);
			}
		}
}






void	synth_FillNextSampBuffer (voiceVarPtr vv)
{
	FillSampBuf (vv);					/* fill a sound buf with speech	*/
	(*(vv->funcList->_i_Cur_Sample_Buffer_FUNC)) (vv, vv->sampleBuffer, vv->waveIndex);
	//SetNextBuffer (vv);					/* point to next free buffer	*/
}





void	Calc_Pole_Coefficients (formantVarPtr zz, rShort *Acoeff, rShort *Bcoeff, rShort *Ccoeff, short pitch, short bandWidth)
{
	short			bwIndex;
	rShort			cosVal;

	if (bandWidth > kMaxBandWidth)
		bandWidth = kMaxBandWidth;

	if (bandWidth < zz->voiceMinBW)
		bandWidth = zz->voiceMinBW;

	if (pitch < 256)
		pitch = 256;
	
	bwIndex = (bandWidth - 50)/5;
	*Ccoeff = *(zz->CcoeffTblPtr + bwIndex);
	cosVal = *(zz->CosTblPtr + (pitch - 256));
	*Bcoeff = mMul2(*(zz->BcoeffTblPtr + bwIndex), cosVal, kPrecision-1);		/* implied 2x	*/
	*Acoeff = kOnePtOh - *Bcoeff - *Ccoeff;
}


void	Calc_Zero_Coefficients (formantVarPtr zz, rShort *Acoeff, rShort *Bcoeff, rShort *Ccoeff, short pitch, short bandWidth)
{
	short			bwIndex;
	rShort			cosVal;

	if (bandWidth > kMaxBandWidth)
		bandWidth = kMaxBandWidth;
	
	bwIndex = (bandWidth - 50)/5;
	*Ccoeff = *(zz->CcoeffTblPtr + bwIndex);
	cosVal = *(zz->CosTblPtr + (pitch - 256));
	*Bcoeff = mMul2(*(zz->BcoeffTblPtr + bwIndex), cosVal, kPrecision-1);		/* implied 2x	*/

	*Bcoeff = 0 - (*Bcoeff);
	*Ccoeff = 0 - (*Ccoeff);

	*Acoeff = kOnePtOh + (*Bcoeff) + (*Ccoeff);
}



void	InitFixedFormants (formantVarPtr zz, voiceVarPtr vv)
{


	Calc_Pole_Coefficients (zz, &zz->Acoeff4,&zz->Bcoeff4,&zz->Ccoeff4, zz->voice_F4_Freq, zz->voice_F4_BW);

	Calc_Pole_Coefficients (zz, &zz->Acoeff4p,&zz->Bcoeff4p,&zz->Ccoeff4p, zz->f4_Par, zz->bw4_Par);
	zz->Acoeff4p = mMul2(zz->Acoeff4p, kNoiseGain, kPrecision);

	Calc_Pole_Coefficients (zz, &zz->Acoeff5,&zz->Bcoeff5,&zz->Ccoeff5, zz->f5_Par, zz->bw5_Par);
	zz->Acoeff5 = mMul2(zz->Acoeff5, kNoiseGain, kPrecision);

	Calc_Pole_Coefficients (zz, &zz->Acoeff6,&zz->Bcoeff6,&zz->Ccoeff6, zz->f6_Par, zz->bw6_Par);
	zz->Acoeff6 = mMul2(zz->Acoeff6, kNoiseGain, kPrecision);

	Calc_Pole_Coefficients (zz, &zz->AcoeffNP,&zz->BcoeffNP,&zz->CcoeffNP, zz->fNP, zz->bNP);
	

}



void	InitSay (voiceVarPtr vv)
{
	short	i, scale;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	vv->nextSampBuf = kBuf1;
	vv->sampleBuffer = vv->sampleBuffer1;
	
	vv->waveIndex = 0;
	zz->glotIndex = 0;
	zz->glotIndex1 = 0;
	zz->sndIndex = 0;
	zz->sndIndex1 = 0;
	zz->noiseIndex = 0;

	InitFixedFormants (zz, vv);

	zz->Na1 = 0;
	zz->Nb1 = 0;
	zz->Na2 = 0;
	zz->Nb2 = 0;
	zz->Na3 = 0;
	zz->Nb3 = 0;
	zz->Na4 = 0;
	zz->Nb4 = 0;
	zz->Na5 = 0;
	zz->Nb5 = 0;
	zz->Na6 = 0;
	zz->Nb6 = 0;
	zz->Na2a = 0;
	zz->Nb2a = 0;
	zz->Na3a = 0;
	zz->Nb3a = 0;
	zz->Na4a = 0;
	zz->Nb4a = 0;
	zz->NaNP = 0;
	zz->NbNP = 0;
	zz->NaNZ = 0;
	zz->NbNZ = 0;

	zz->lastnSamp = 0;
			
	zz->lastAmp = 0;
	zz->lastSample = 0;
	zz->lastRevbSample = 0;
	
	//---------------------------------
	// Init reverb
	//---------------------------------
	scale = zz->reverbDelay;
	zz->tapBuffer[0] = (kTap4 * zz->reverbDelay) >> 16;
	zz->tapBuffer[1] = (kTap5 * zz->reverbDelay) >> 16;
	zz->tapBuffer[2] = (kTap6 * zz->reverbDelay) >> 16;
	zz->tapBuffer[3] = (kTap8 * zz->reverbDelay) >> 16;
	zz->maxRvbDelay = (kMaxTap * zz->reverbDelay) >> 16;
	zz->delay_Index = 0;
	for (i = 0; i < kMaxTap; i++)			// clear tank
		zz->delayBuffer[i] = 0;

	//---------------------------------
	// Flag if we're using reverb
	//---------------------------------
	if ((zz->reverbDepth > 0) && vv->cpuIsFast)
		zz->addReverb = true;
	else
		zz->addReverb = false;

}




void	SayFrame (voiceVarPtr vv)
{
	formantVarPtr	zz;
	FramePtr		framePtr;

	//---------------------------
	// Cascade excitation
	//---------------------------
	rShort		vPulse, vPulse1;				// glottal source #1 and #2
	rShort		asperation;						// Aspiration source
	rShort		breath;							// Breath source
	rShort		totalBreathGain;				// Breath gain
	rLong		nGain;							// nasal damping
	short		cycleIndex;						// index into buzz wave
	long		sampleIndex;					// index into wavesample
	rShort		sourceC;						// Cascaded source (Buzz + Wavesample + Breath + Aspiration)
	rShort		sourceP;						// Parallell source
	rShort		SampV;							/* voiced bank output					*/
	
	//---------------------------
	// Parallel excitation
	//---------------------------
	rShort		nPulse;							// Noise source for parallel bank					
	rShort		SampAB;							/* parallel feed-through output			*/
	rShort		Samp2;							/* parallel formant #2 output			*/
	rShort		Samp3;							/* parallel formant #3 output			*/
	rShort		Samp4;							/* parallel formant #4 output			*/
	rShort		Samp5,Samp6;					/* parallel formant #5 and #6 output	*/
	rShort		Samp;							/* sum of parallell banks				*/

	//---------------------------
	// Output
	//---------------------------
	rShort		nSamp;							// Sum of cascade and parallel banks
	rUSShort	wByte;							// Output sample byte
	
	//---------------------------
	// Reverb temps
	//---------------------------
	short		tCntr;							// reverb tap loop counter
	short		reflect_Index;					// index into delay tank
	rLong		accumL;							// reverb reflection accumulator
	
	//---------------------------
	// Misc temps
	//---------------------------
	short		sampCtr;						// Loop counter
	rShort		tSamp;							// temp sample for emphasis
	short		curF0Pitch;						// curent F0
	short		tempP;							// temp index
	short		noNasal, vowel;					// temp flags
	rShort		Acoeff2q, Acoeff3q, Acoeff4q, Acoeff5q, Acoeff6q;	// parallel coefficients with scaled gain
	short		ampCtr;							// Av interpolator
	short		ampBank;

	//---------------------------
	// Local copy of synth globals
	//---------------------------
	short			local_bit16_Sound;				// true = 16 bit sound output
	short			local_sync_On_Marker;
	long			local_VP_pitchRange;
	short			local_VP_baselinePitch;
	long			local_cur_PhonFlags_CF;
	unsigned char	*local_sampleBuffer;
	long			local_waveIndex;

	zz = (formantVarPtr)vv->synthVars;
	local_bit16_Sound = vv->bit16_Sound;
	local_sync_On_Marker = vv->sync_On_Marker;
	local_VP_pitchRange = vv->VP_pitchRange;
	local_VP_baselinePitch = vv->VP_baselinePitch;
	local_cur_PhonFlags_CF = vv->cur_PhonFlags_CF;
	local_sampleBuffer = vv->sampleBuffer;
	local_waveIndex = vv->waveIndex;
	if (!local_bit16_Sound)
		local_sampleBuffer += local_waveIndex;
	else
		//(unsigned short*)local_sampleBuffer += local_waveIndex;
		local_sampleBuffer += (local_waveIndex << 1);				// for CW
	
	//---------------------------------
	// Use previous Frame
	//---------------------------------
	if (zz->curFrameBuf == kFrame1)
		framePtr = &zz->frameBuf2;
	else
		framePtr = &zz->frameBuf1;

	//---------------------------------
	// Reset if output is silent
	//---------------------------------
	if ( (zz->curAmp == 0) && (zz->Af == 0) )
		{
		zz->glotIndex = 0;							/* reset voiced source phase pointer	*/
		zz->glotIndex1 = 0;							/* reset voiced source phase pointer	*/
		zz->Na1 = 0;
		zz->Nb1 = 0;
		zz->Na2 = 0;
		zz->Nb2 = 0;
		zz->Na3 = 0;
		zz->Nb3 = 0;
		zz->Na4 = 0;
		zz->Nb4 = 0;
		zz->NaNP = 0;
		zz->NbNP = 0;
		zz->NaNZ = 0;
		zz->NbNZ = 0;
		zz->lastAmp	= 0;
		}

	/*------------------------------------------*/
	/* Calculate new filter coefficients		*/
	/*------------------------------------------*/
	Calc_Pole_Coefficients (zz, &zz->Acoeff1,&zz->Bcoeff1,&zz->Ccoeff1, framePtr->f1 + zz->voiceF1Gain, framePtr->bw1);
	Calc_Pole_Coefficients (zz, &zz->Acoeff2,&zz->Bcoeff2,&zz->Ccoeff2, framePtr->f2 + zz->voiceF2Gain, framePtr->bw2);
	Calc_Pole_Coefficients (zz, &zz->Acoeff3,&zz->Bcoeff3,&zz->Ccoeff3, framePtr->f3 + zz->voiceF3Gain, framePtr->bw3);

	//---------------------------------
	// Calc nasal if pole � zero
	//---------------------------------
	if (framePtr->FNZ != zz->fNP)
		{
		noNasal = false;
		Calc_Zero_Coefficients (zz, &zz->AcoeffNZ,&zz->BcoeffNZ,&zz->CcoeffNZ, framePtr->FNZ + zz->nasalAmt, zz->bNP);
		nGain = mRatio(zz->AcoeffNP, zz->AcoeffNZ, 16);
		}
	else
		noNasal = true;


	//---------------------------------
	// Calculate new Av, Af and ab
	//---------------------------------
	ampBank = false;
	#if FLOAT_SYNTH_MT3
		zz->Av = (((rShort)framePtr->Av) / 32) *  zz->speechVolume;
		zz->Af = (((rShort)framePtr->Af) / 8) *  zz->speechVolume;
		zz->ab = (((rShort)framePtr->AB) / 32) *  zz->speechVolume;
	#else
		zz->Av = framePtr->Av * zz->speechVolume;				/* 0000 0000 000x xxxx  --> 000x xxxx 0000 0000		*/
		zz->Af = (framePtr->Af * zz->speechVolume) << 2;
		zz->ab = framePtr->AB * zz->speechVolume;
	#endif
	
	if ( (zz->Af > 0) || (zz->ab > 0) )
		ampBank = true;
		
	//---------------------------------
	// Calculate new a2
	//---------------------------------
	if (framePtr->a2)
		{
		#if FLOAT_SYNTH_MT3
			zz->amp2 = ((rShort)framePtr->a2) / 32;
		#else
			zz->amp2 = framePtr->a2 << kPrecision - 5;
		#endif
		Acoeff2q = mMul2(zz->Acoeff2, zz->amp2, kPrecision);
		ampBank = true;
		}
	else
		{
		zz->amp2 = 0;
		zz->Nb2a = 0;
		zz->Na2a = 0;
		}
		
	//---------------------------------
	// Calculate new a3
	//---------------------------------
	if (framePtr->a3)
		{
		#if FLOAT_SYNTH_MT3
			zz->amp3 = ((rShort)framePtr->a3) / 32;
		#else
			zz->amp3 = framePtr->a3 << kPrecision - 5;
		#endif
		Acoeff3q = mMul2(zz->Acoeff3, zz->amp3, kPrecision);
		ampBank = true;
		}
	else
		{
		zz->amp3 = 0;
		zz->Nb3a = 0;
		zz->Na3a = 0;
		}
		
	//---------------------------------
	// Calculate new a4
	//---------------------------------
	//if ( (framePtr->a4) && (zz->Af == 0) )
	if (framePtr->a4)
		{
		#if FLOAT_SYNTH_MT3
			zz->amp4 = ((rShort)framePtr->a4) / 32;
		#else
			zz->amp4 = framePtr->a4 << kPrecision - 5;
		#endif
		Acoeff4q = mMul2(zz->Acoeff4p, zz->amp4, kPrecision);
		ampBank = true;
		}
	else
		{
		zz->amp4 = 0;
		zz->Nb4a = 0;
		zz->Na4a = 0;
		}
		
	//---------------------------------
	// Calculate new a5
	//---------------------------------
	if (framePtr->a5)
		{
		#if FLOAT_SYNTH_MT3
			zz->amp5 = ((rShort)framePtr->a5) / 32;
		#else
			zz->amp5 = framePtr->a5 << kPrecision - 5;
		#endif
		Acoeff5q = mMul2(zz->Acoeff5, zz->amp5, kPrecision);
		ampBank = true;
		}
	else
		{
		zz->amp5 = 0;
		zz->Nb5 = 0;
		zz->Na5 = 0;
		}
		
	//---------------------------------
	// Calculate new a6
	//---------------------------------
	if (framePtr->a6)
		{
		#if FLOAT_SYNTH_MT3
			zz->amp6 = ((rShort)framePtr->a6) / 32;
		#else
			zz->amp6 = framePtr->a6 << kPrecision - 5;
		#endif
		Acoeff6q = mMul2(zz->Acoeff6, zz->amp6, kPrecision);
		ampBank = true;
		}
	else
		{
		zz->amp6 = 0;
		zz->Nb6 = 0;
		zz->Na6 = 0;
		}

	SampAB = 0;
	Samp2 = 0;
	Samp3 = 0;
	Samp4 = 0;
	Samp5 = 0;
	Samp6 = 0;


	/*----------------------------------------------*/
	/* Calculate new voice source phase increments	*/
	/*----------------------------------------------*/
	curF0Pitch = framePtr->f0;
	
	//---------------------------------
	// Wavesample
	//---------------------------------
	if (zz->glotType == kUseSnd)
		{
		//-------------------------------------
		// Buzz oscillator freq
		//-------------------------------------
		zz->glotInc = (*(zz->TOPtr + ((curF0Pitch + zz->VP_offsetPitch) & 0xFF))) >> (3 - ((curF0Pitch + zz->VP_offsetPitch) >> 8));

		//-------------------------------------
		// Using MARKERS
		//-------------------------------------
		if (local_sync_On_Marker)
			{
			//-------------------------------------
			// 1st oscillator always plays every sample
			//-------------------------------------
			zz->sampleInc = 1 << 14;
			if (zz->voiceChorus != 0)
				{
				//-------------------------------------
				// Offset 2nd oscillator for chorus
				//-------------------------------------
				tempP = 0x1CD + zz->voiceChorus;
				zz->sampleInc1 = (*(zz->TOPtr + (tempP & 0xFF))) >> (5 - (tempP >> 8));
				zz->sampleInc1 = zz->sampleInc1 >> 2;
				}
			if (framePtr->marker != kNoMarker)
				{
				//-------------------------------------
				// Set sample start at new marker
				//-------------------------------------
				zz->sndIndex = framePtr->marker << 14;
				zz->sndIndex1 = zz->sndIndex;
				}
			}
		else
			{
			if (local_VP_pitchRange == 0)
				curF0Pitch = local_VP_baselinePitch;
			zz->sampleInc = (*(zz->TOPtr + (curF0Pitch & 0xFF))) >> (5 - (curF0Pitch >> 8));
			zz->sampleInc = zz->sampleInc >> 2;

			if (zz->voiceChorus != 0)
				{
				curF0Pitch += zz->voiceChorus;
				if (curF0Pitch < 0)
					curF0Pitch = 0;
				zz->sampleInc1 = (*(zz->TOPtr + (curF0Pitch & 0xFF))) >> (5 - (curF0Pitch >> 8));
				zz->sampleInc1 = zz->sampleInc1 >> 2;
				}
			}
		}
	else
		{
		//---------------------------------
		// Buzz
		//---------------------------------
		zz->glotInc = (*(zz->TOPtr + (curF0Pitch & 0xFF))) >> (3 - (curF0Pitch >> 8));
		if (zz->voiceChorus != 0)
			{
			curF0Pitch += zz->voiceChorus;
			if (curF0Pitch < 0)
				curF0Pitch = 0;
			zz->glotInc1 = (*(zz->TOPtr + (curF0Pitch & 0xFF))) >> (3 - (curF0Pitch >> 8));
			}
		}
	
	totalBreathGain = mMul2(zz->breathGain, zz->Av, kPrecision);
	
	/*if (framePtr->phon_Edge && (local_cur_PhonFlags_CF & kVowelF) )	*/
	if (framePtr->phon_Edge && (local_cur_PhonFlags_CF & kSonorantF) )
		vowel = true;
	else
		vowel = false;

	//---------------------------------
	// 3ms (32 steps) Av ramp
	// from last to new target
	//---------------------------------
	#if FLOAT_SYNTH_MT3
		zz->ampStep	= (zz->Av - zz->lastAmp) / 32;			/* Tc = 3ms (32 steps to target)	*/
	#else
		zz->ampStep	= ((zz->Av << kAmpStepRes) - zz->lastAmp) >> 3;			/* Tc = 3ms (32 steps to target)	*/
	#endif
	zz->curAmp_Full	= zz->lastAmp;
	zz->lastAmp	= mScale(zz->Av, kAmpStepRes);
	ampCtr = 0;


	/********************************/
	/* Sample calculations			*/
	/********************************/

	for (sampCtr = (kSampFrameLen / 2)-1; sampCtr >= 0; --sampCtr)
		{

		if (ampCtr < 8)
			{
			zz->curAmp_Full = zz->curAmp_Full + zz->ampStep;		// step ramp
			#if FLOAT_SYNTH_MT3
				zz->curAmp = zz->curAmp_Full;
			#else
				zz->curAmp = zz->curAmp_Full >> kAmpStepRes;		// extract integer part
			#endif
			ampCtr++;
			}
		else
			{
			zz->curAmp = zz->Av;
			}

		//----------------------------------------
		// If any excitation...
		//----------------------------------------
		if ( (zz->curAmp > 0) || (ampBank) )
			{
			//----------------------------------------
			// Next NOISE sample
			//----------------------------------------
			zz->noiseIndex = (zz->noiseIndex + 1) & kNoiseLen-1;			// wrap-around
			
			if (zz->curAmp > 0)
				{
				//----------------------------------------
				// GLOTTAL EXCITATION
				//----------------------------------------
				if (zz->glotType == kUseSnd)
					{
					/*------------------------------------------*/
					/* Use wavesample source					*/
					/*------------------------------------------*/
					zz->sndIndex = zz->sampleInc  + zz->sndIndex;
					sampleIndex = zz->sndIndex >> 14;
					if (zz->sync_On_Vowel && vowel)
						{
						//----------------------------------------
						// Reset wavesample at VOWEL
						//----------------------------------------
						zz->sndIndex = 0;
						sampleIndex = 0;
						vowel = false;
						}
					else if (sampleIndex >= zz->sampleLength)
						{
						//----------------------------------------
						// LOOP wavesample if at end
						//----------------------------------------
						sampleIndex = (sampleIndex - zz->sampleLength) + zz->loopPoint;
						zz->sndIndex = sampleIndex << 14;
						}
					vPulse = ((unsigned char)(*(zz->SampleWave + sampleIndex))) - 128;
					#if FLOAT_SYNTH_MT3
						sourceC = vPulse * 12;
					#else
						sourceC = (vPulse - (vPulse >> 2)) << 4;			// 0.75vPulse * 16
					#endif
					
					/*------------------------------------------*/
					/* Chorous									*/
					/*------------------------------------------*/
					if (zz->voiceChorus != 0)
						{
						zz->sndIndex1 = zz->sampleInc1  + zz->sndIndex1;
						sampleIndex = zz->sndIndex1 >> 14;
						if (zz->sync_On_Vowel && vowel)
							{
							zz->sndIndex1 = 0;
							sampleIndex = 0;
							vowel = false;
							}
						else if (sampleIndex >= zz->sampleLength)
							{
							sampleIndex = (sampleIndex - zz->sampleLength) + zz->loopPoint;
							zz->sndIndex1 = sampleIndex << 14;
							}
						vPulse = ((unsigned char)(*(zz->SampleWave + sampleIndex))) - 128;
						#if FLOAT_SYNTH_MT3
							vPulse *= 12;
						#else
							vPulse = (vPulse - (vPulse >> 2)) << 4;
						#endif
						sourceC = mDiv((sourceC + vPulse), 2, 1);
						}
					//----------------------------------------
					// Get wavesample and scale to gain
					//----------------------------------------
					sourceC = mMul2(sourceC, zz->wavesampleGain, kPrecision);
					sourceC = mMul2(sourceC, zz->curAmp, kPrecision);
					}
				else
					sourceC = 0;
					
				/*------------------------------------------*/
				/* Get next glottal source sample			*/
				/*------------------------------------------*/
				zz->glotIndex = (zz->glotInc  + zz->glotIndex) & 0xFFFFFF;
				cycleIndex = zz->glotIndex >> 16;
				vPulse = zz->voiceWaveform[cycleIndex];
				
				/*------------------------------------------*/
				/* Chorous									*/
				/*------------------------------------------*/
				if ( (zz->voiceChorus != 0) && (zz->glotType != kUseSnd) )
					{
					zz->glotIndex1 = (zz->glotInc1  + zz->glotIndex1) & 0xFFFFFF;
					cycleIndex = zz->glotIndex1 >> 16;
	
					vPulse1 = zz->voiceWaveform1[cycleIndex];
					vPulse = mDiv((vPulse + vPulse1), 2, 1);
					}
				if (zz->glotType != kUseSnd)
					sourceC = mMul2(vPulse, zz->curAmp, kPrecision) + sourceC;
				}
			else
				{
				//----------------------------------------
				// There's NO glottal excitation
				//----------------------------------------
				sourceC = 0;
				zz->lastnSamp = 0;
				zz->glotIndex = 0;							/* start at begining of waveform next time	*/
				zz->glotIndex1 = 0;							/* start at begining of waveform next time	*/
				zz->lastAmp	= 0;
				vPulse = 0;
				}
	
			/*------------------------------------------*/
			/* Cascade filter branch					*/
			/*------------------------------------------*/
			if ( (zz->curAmp > 0) || (zz->Af > 0) )
				{
				//----------------------------------------
				// Add ASPIRATION
				//----------------------------------------
				#if FLOAT_SYNTH_MT3
					asperation = *(zz->BandNoisePtr + zz->noiseIndex);
				#else
					asperation = (*(zz->BandNoisePtr + zz->noiseIndex)) - 128;
				#endif
				sourceC += mMul2(asperation, zz->Af, kPrecision);
				
				//----------------------------------------
				// Add BREATH
				//----------------------------------------
				if ( (totalBreathGain > 0) && (cycleIndex > zz->breathCycle))
					{
					#if FLOAT_SYNTH_MT3
						breath = *(zz->breathWave + zz->noiseIndex);
					#else
						breath = (*(zz->breathWave + zz->noiseIndex)) - 128;
					#endif
					sourceC += mMul2(breath, totalBreathGain, kPrecision-2);
					}
	
				if (noNasal)
					{
					SampV = sourceC;			/* Skip calculation since nasal pole/zero cancel each other out	*/
					//zz->NaNZ = 0;
					//zz->NbNZ = 0;
					//zz->NaNP = 0;
					//zz->NbNP = 0;
					}
				else
					{
					/*------------------------------------------*/
					/* Nasal zero antiresonator					*/
					/*------------------------------------------*/
					SampV = sourceC + (mUnScale( ((zz->BcoeffNZ * zz->NaNZ) + (zz->CcoeffNZ * zz->NbNZ)), kPrecision));
					zz->NbNZ = zz->NaNZ;
					zz->NaNZ = sourceC;
					SampV = mMul2(nGain, SampV, 16);
		
					/*------------------------------------------*/
					/* Nasal pole resonator						*/
					/*------------------------------------------*/
					SampV = SampV + (mUnScale( ((zz->BcoeffNP * zz->NaNP) + (zz->CcoeffNP * zz->NbNP)), kPrecision));;
					zz->NbNP = zz->NaNP;
					zz->NaNP = SampV;
					}
				
				SampV = mUnScale( ((zz->Acoeff1 * SampV) + (zz->Bcoeff1 * zz->Na1) + (zz->Ccoeff1 * zz->Nb1)), kPrecision);
				zz->Nb1 = zz->Na1;
				zz->Na1 = SampV;
				
				SampV = mUnScale( ((zz->Acoeff2 * SampV) + (zz->Bcoeff2 * zz->Na2) + (zz->Ccoeff2 * zz->Nb2)), kPrecision);
				zz->Nb2 = zz->Na2;
				zz->Na2 = SampV;
				
				SampV = mUnScale( ((zz->Acoeff3 * SampV) + (zz->Bcoeff3 * zz->Na3) + (zz->Ccoeff3 * zz->Nb3)), kPrecision);
				zz->Nb3 = zz->Na3;
				zz->Na3 = SampV;
	
				SampV = mUnScale( ((zz->Acoeff4 * SampV) + (zz->Bcoeff4 * zz->Na4) + (zz->Ccoeff4 * zz->Nb4)), kPrecision);
				zz->Nb4 = zz->Na4;
				zz->Na4 = SampV;
				}
			else
				{
				SampV = 0;
				}
		
			/*------------------------------------------*/
			/* Parallel filter branch					*/
			/*------------------------------------------*/			
			#if FLOAT_SYNTH_MT3
				nPulse = *(zz->NoiseWavePtr + zz->noiseIndex);
			#else
				nPulse = (*(zz->NoiseWavePtr + zz->noiseIndex)) - 128;
			#endif
			sourceP = mMul2(nPulse, zz->voiceNoiseGain, kPrecision);
	
			if (zz->ab > 0)
				{
				SampAB = mMul2(sourceP, zz->ab, (kPrecision-1));
				}
				
			if (zz->amp2 > 0)
				{
				Samp2 = mUnScale( ((Acoeff2q * sourceP) + (zz->Bcoeff2 * zz->Na2a) + (zz->Ccoeff2 * zz->Nb2a)), kPrecision);
				zz->Nb2a = zz->Na2a;
				zz->Na2a = Samp2;
				}

			if (zz->amp3 > 0)
				{
				Samp3 = mUnScale( ((Acoeff3q * sourceP) + (zz->Bcoeff3 * zz->Na3a) + (zz->Ccoeff3 * zz->Nb3a)), kPrecision);
				zz->Nb3a = zz->Na3a;
				zz->Na3a = Samp3;
				}

			if (zz->amp4 > 0)
				{
				Samp4 = mUnScale( ((Acoeff4q * sourceP) + (zz->Bcoeff4p * zz->Na4a) + (zz->Ccoeff4p * zz->Nb4a)), kPrecision);
				zz->Nb4a = zz->Na4a;
				zz->Na4a = Samp4;
				}

			if (zz->amp5 > 0)
				{
				Samp5 = mUnScale( ((Acoeff5q * sourceP) + (zz->Bcoeff5 * zz->Na5) + (zz->Ccoeff5 * zz->Nb5)), kPrecision);
				zz->Nb5 = zz->Na5;
				zz->Na5 = Samp5;
				}

			if (zz->amp6 > 0)
				{
				Samp6 = mUnScale( ((Acoeff6q * sourceP) + (zz->Bcoeff6 * zz->Na6) + (zz->Ccoeff6 * zz->Nb6)), kPrecision);
				zz->Nb6 = zz->Na6;
				zz->Na6 = Samp6;
				}
	
			/*--------------------------------------------------*/
			/* Sum all the Parallel filter branches together	*/
			/*--------------------------------------------------*/
			Samp = SampAB - Samp3 + Samp4 - Samp5 + Samp6 - Samp2;
			
			
			/*--------------------------------------------------*/
			/* Add the Parallel and Cascade branches together.	*/
			/*--------------------------------------------------*/
			nSamp = SampV + Samp;
			//nSamp = sourceP;

			/*--------------------------------------------------*/
			/* Some voiced radiation loss due to impedance 		*/
			/*  mismatch from mouth to outside air.				*/
			/*--------------------------------------------------*/
			if (zz->hfEmph)
				{
				#if FLOAT_SYNTH_MT3
					nSamp *= 1.25;
					tSamp = nSamp  - (zz->lastSample * 0.75);
					zz->lastSample = nSamp;
					nSamp = tSamp + (nSamp * 0.5);
				#else
					nSamp += (nSamp >> 2);
					tSamp = nSamp  - (zz->lastSample - (zz->lastSample >> 2));
					zz->lastSample = nSamp;
					nSamp = tSamp + (nSamp >> 1);
				#endif
				}


			/*--------------------------------------------------*/
			/* Add reverb										*/
			/*--------------------------------------------------*/
			if (zz->addReverb)
				{
				zz->delayBuffer[zz->delay_Index] = nSamp;
				accumL = 0;
				for (tCntr = 3; tCntr >= 0; tCntr--)
					{
					reflect_Index = zz->delay_Index - zz->tapBuffer[tCntr];
					if (reflect_Index < 0)							/* wrap-around	*/
						reflect_Index += zz->maxRvbDelay;
					
					accumL += zz->delayBuffer[reflect_Index];
					}


				#if FLOAT_SYNTH_MT3
					nSamp += (((accumL  + zz->lastRevbSample) / 8) * zz->reverbDepth);									/* filter hi-freqs in reflections	*/
				#else
					nSamp += (((short)((accumL  + zz->lastRevbSample) >> 3) * zz->reverbDepth) >> kPrecision);			/* filter hi-freqs in reflections	*/
				#endif
				zz->lastRevbSample = accumL;

				zz->delay_Index++;
				if (zz->delay_Index >= zz->maxRvbDelay)
					zz->delay_Index = 0;
				}


			/*--------------------------------------------------*/
			/* Clip at max										*/
			/*--------------------------------------------------*/
			if (nSamp > 8191)
				nSamp = 8191;
			else if (nSamp < (-8191))
				nSamp = (-8191);			

			
			/*--------------------------------------------------*/
			/* Output final sample and interpolated sample		*/
			/*--------------------------------------------------*/

#if POWERPC_NATIVE_MT3
#else
			if (!local_bit16_Sound)
				{
				/*------------------------------------------*/
				/* 14-bit -> 8-bit							*/
				/* 11khz -> 22khz linear interpolation		*/
				/*------------------------------------------*/

				wByte = ((((nSamp - zz->lastnSamp) >> 1) + zz->lastnSamp) >> 6) + 128;
				*local_sampleBuffer++ = wByte;
		
				wByte = (nSamp >> 6) + 128;
				*local_sampleBuffer++ = wByte;
				}
			else
#endif
				{
				/*------------------------------------------*/
				/* 14-bit -> 16-bit							*/
				/* 11khz -> 22khz linear interpolation		*/
				/*------------------------------------------*/
				#if	FLOAT_SYNTH_MT3
					wByte = (((nSamp - zz->lastnSamp) / 2) + zz->lastnSamp) * 4;
				#else
					wByte = (((nSamp - zz->lastnSamp) >> 1) + zz->lastnSamp) << 2;
				#endif
				//*((unsigned short*)local_sampleBuffer)++ = wByte;
				*((unsigned short*)local_sampleBuffer) = wByte;				// for CW  error
				local_sampleBuffer += 2;									// for CW  error
		
				#if	FLOAT_SYNTH_MT3
					wByte = nSamp * 4;
				#else
					wByte = nSamp << 2;
				#endif
				//*((unsigned short*)local_sampleBuffer)++ = wByte;
				*((unsigned short*)local_sampleBuffer) = wByte;				// for CW  error
				local_sampleBuffer += 2;									// for CW  error
				}
			local_waveIndex += 2;

			/*--------------------------------------------------*/
			/* Prepare for the next sample's interpolation		*/
			/*--------------------------------------------------*/
			zz->lastnSamp = nSamp;
			}

		else											/* output is zero, don't bother with calculations	*/
			{
			zz->lastnSamp = 0;
			zz->glotIndex = 0;
			zz->glotIndex1 = 0;							/* start at begining of waveform next time	*/
			zz->lastAmp	= 0;
	
			if (!local_bit16_Sound)
				{
				//*((unsigned short*)local_sampleBuffer)++ = 0x8080;
				*((unsigned short*)local_sampleBuffer) = 0x8080;			// for CW  error
				local_sampleBuffer += 2;									// for CW  error
				}
			else
				{
				//*((uint32_t*)local_sampleBuffer)++ = 0;
				*((uint32_t*)local_sampleBuffer) = 0;						// for CW  error
				local_sampleBuffer += 4;									// for CW  error
				}
			local_waveIndex += 2;
			}
		}
	vv->waveIndex = local_waveIndex;
}





void	Start_The_Speech (voiceVarPtr vv)
{
	/*--------------------------*/
	/* fill buf 1 with speech	*/
	/*--------------------------*/
	FillSampBuf (vv);
	(*(vv->funcList->_i_First_Sample_Buffer_FUNC)) (vv, vv->sampleBuffer, vv->waveIndex);
	
	/*--------------------------*/
	/* point buf 2...			*/
	/*--------------------------*/
	SetNextBuffer (vv);
	
	/*--------------------------*/
	/* ...fill it with speech	*/
	/*--------------------------*/
	FillSampBuf (vv);
	(*(vv->funcList->_i_Cur_Sample_Buffer_FUNC)) (vv, vv->sampleBuffer, vv->waveIndex);
	
	/*--------------------------*/
	/* point back to buf 1		*/
	/*--------------------------*/
	SetNextBuffer (vv);
}






void synth_SetVolume (voiceVarPtr vv, short vol)
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	#if FLOAT_SYNTH_MT3
		zz->speechVolume = ((rShort)vol) / 256;
	#else
		zz->speechVolume = vol;
	#endif
	zz->voiceNoiseGain = mMul2(zz->setNoiseGain, zz->speechVolume, 8);
}



void	SaveFrame (voiceVarPtr vv)
{
	FramePtr		frameBuf;
	short			curF1, curF2, curF3;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	if (zz->curFrameBuf == kFrame1)
		frameBuf = &zz->frameBuf1;
	else
		frameBuf = &zz->frameBuf2;

	curF1 = zz->controlData[kF1];
	curF2 = zz->controlData[kF2];
	curF3 = zz->controlData[kF3];
	
	while ( (curF2 - curF1) < 200 )
		curF1 -= 10;

	while ( (curF3 - curF2) < 600 )
		curF3 += 10;

	frameBuf->f1 = (*(vv->funcList->e_HzToPitch_FUNC)) (vv, curF1);
	frameBuf->f2 = (*(vv->funcList->e_HzToPitch_FUNC)) (vv, curF2);
	frameBuf->f3 = (*(vv->funcList->e_HzToPitch_FUNC)) (vv, curF3);

	frameBuf->bw1 = (zz->controlData[kBW1] * zz->voiceBWgain1) >> 16;
	frameBuf->bw2 = (zz->controlData[kBW2] * zz->voiceBWgain2) >> 16;
	frameBuf->bw3 = (zz->controlData[kBW3] * zz->voiceBWgain3) >> 16;

	frameBuf->FNZ = (*(vv->funcList->e_HzToPitch_FUNC)) (vv, zz->controlData[kFNZ]);

	if (zz->controlData[kAp2] < 0) zz->controlData[kAp2] = 0;					/* @@@@@	*/
	if (zz->controlData[kAp3] < 0) zz->controlData[kAp3] = 0;					/* @@@@@	*/
	if (zz->controlData[kAp4] < 0) zz->controlData[kAp4] = 0;					/* @@@@@	*/
	if (zz->controlData[kAp5] < 0) zz->controlData[kAp5] = 0;					/* @@@@@	*/
	if (zz->controlData[kAp6] < 0) zz->controlData[kAp6] = 0;					/* @@@@@	*/
	if (zz->controlData[kAB] < 0) zz->controlData[kAB] = 0;					/* @@@@@	*/
	if (zz->controlData[kAV] < 0) zz->controlData[kAV] = 0;					/* @@@@@	*/
	if (zz->controlData[kAF] < 0) zz->controlData[kAF] = 0;					/* @@@@@	*/
	
	frameBuf->Av = (*(vv->funcList->e_LogToLin_FUNC)) (vv, zz->controlData[kAV]);

	frameBuf->Af = (*(vv->funcList->e_LogToLin_FUNC)) (vv, zz->controlData[kAF]);

	frameBuf->a2 = (*(vv->funcList->e_LogToLin_FUNC)) (vv, zz->controlData[kAp2]);
	frameBuf->a3 = (*(vv->funcList->e_LogToLin_FUNC)) (vv, zz->controlData[kAp3]);
	frameBuf->a4 = (*(vv->funcList->e_LogToLin_FUNC)) (vv, zz->controlData[kAp4]);
	frameBuf->a5 = (*(vv->funcList->e_LogToLin_FUNC)) (vv, zz->controlData[kAp5]);
	frameBuf->a6 = (*(vv->funcList->e_LogToLin_FUNC)) (vv, zz->controlData[kAp6]);
	frameBuf->AB = (*(vv->funcList->e_LogToLin_FUNC)) (vv, zz->controlData[kAB]);
	
	frameBuf->f0 = vv->controlF0;

	frameBuf->phon_Edge = vv->starting_New_Phon;
	frameBuf->marker = vv->frameMarker;
	vv->frameMarker = kNoMarker;
	
}




void InvDFT (formantVarPtr zz, voiceDataPtr vd)
{
	short	i,j, sIndex;
	rShort	amp, amp1;
	short	sIndex1;
	short	*hPtr;
	short	*hPtr1;
	rLong	max, max1, max2, hold;
	rLong	voiceWaveGain;


	//------------------------------------------
	// Calculate harmonic gain
	//------------------------------------------
	voiceWaveGain = vd->vGain;							// this is done in 2 steps so the compiler uses LONG arithmetic
	voiceWaveGain = mRatio(voiceWaveGain,200,16);

	//------------------------------------------
	// Clear waves
	//------------------------------------------
	for (j = 0; j < 256; j++)
		{
		zz->voiceWaveform[j] = 0;
		zz->voiceWaveform1[j] = 0;
		}
		
	//------------------------------------------
	// Get pointers to harmonic data
	//------------------------------------------
	hPtr = (short*)&vd->vWave;
	hPtr1 = (short*)&vd->vWave1;
	
	//------------------------------------------
	// Step through harmonics 0-48
	//------------------------------------------
	for (i = 0; i < 48; i++)
		{
		//------------------------------------------
		// Scale the harmonic's value
		//------------------------------------------
		amp =	mMul2(hPtr[i],voiceWaveGain,16);
		amp1 =	mMul2(hPtr1[i],voiceWaveGain,16);
		
		//------------------------------------------
		// Start at sin(0)
		//------------------------------------------
		sIndex = 0;
		sIndex1 = 0;
		//------------------------------------------
		// Step through entire buzz cycle (256 bytes)
		//------------------------------------------
		for (j = 0; j < 256; j++)
			{
			//------------------------------------------
			// Add sine of 'i' freq, 'amp' amplitude
			//------------------------------------------
			zz->voiceWaveform[j] =  zz->voiceWaveform[j] + mMul2(amp,zz->SineWave15Ptr[sIndex],16);		/* 16 = 14 (sine) + 2	*/
			zz->voiceWaveform1[j] =  zz->voiceWaveform1[j] + mMul2(amp1,zz->SineWave15Ptr[sIndex],16);		/* 16 = 14 (sine) + 2	*/
			
			//------------------------------------------
			// Calculate next sine index
			//------------------------------------------
			sIndex = sIndex + i;
			if (sIndex > 255)
				sIndex = sIndex - 256;					// wrap-around
			sIndex1 = sIndex1 + i;
			if (sIndex1 > 255)
				sIndex1 = sIndex1 - 256;				// wrap-around
			}
		}
	
	//------------------------------------------
	// Find result waveform's peak
	//------------------------------------------
	max = 0;											// abs peak of buzz 1
	max1 = 0;											// abs peak of buzz 2
	for (j = 0; j < 256; j++)
		{
		hold = zz->voiceWaveform[j];
		if (hold < 0)
			hold = 0 - hold;
		if (hold > max)
			max = hold;

		hold = zz->voiceWaveform1[j];
		if (hold < 0)
			hold = 0 - hold;
		if (hold > max1)
			max1 = hold;
		}

	//------------------------------------------
	// Scale buzz #2 to match buzz #1
	//------------------------------------------
	if (max1 > 0)
		{
		max2 = mRatio(max,max1,16);
		for (j = 0; j < 256; j++)
			{
			zz->voiceWaveform1[j] = mMul2(zz->voiceWaveform1[j], max2, 16);
			}
		}
}



// speech_Rate
// sync_On_Marker
// voiceNaturalPitch
// VP_pitchRange
// VP_stressGain
// VP_riseAmt
// VP_fallAmt
// VP_riseAmt1
// VP_fallAmt1
// VP_assertiveness
// VP_baselineFall
// down_Ramp_Step
// VP_quickness
// pitchCmdStep
// durCmdStep
// stressDurTime
// vibratoDepth1
// vibratoDepth2
// vibratoFreq
// VP_intonation
// portamento
// tempo

void	InitVoice (voiceVarPtr vv, voiceDataPtr vd)
{
	short		temp_Pitch;
	rLong		tempLong;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	vv->speech_Rate = vd->rate;
	zz->glotType = vd->waveType;
	vv->sync_On_Marker = false;
	
	vv->voiceNaturalPitch = (*(vv->funcList->e_HzToPitch_FUNC)) (vv, vd->pitch);
	
	if ( (vd->waveType == kUseSnd) || (vd->waveType == kUseSyncSnd) )
		{
		temp_Pitch = vv->voiceNaturalPitch;
		vv->voiceNaturalPitch = (*(vv->funcList->e_MidiToPitch_FUNC)) (vd->sPitch << 8);
		zz->VP_offsetPitch = temp_Pitch - vv->voiceNaturalPitch;
	
		if (vd->waveType == kUseSyncSnd)
			{
			vv->sync_On_Marker = true;
			zz->glotType = kUseSnd;
			}
		else
			{
			vv->sync_On_Marker = false;
			}
		
		tempLong = vd->sGain;
		zz->wavesampleGain = mRatio(tempLong,100,kPrecision);
		}
		
	zz->voice_Num = vd->voice;
	
	vv->VP_pitchRange = vd->pitchRange;
	vv->VP_pitchRange = (vv->VP_pitchRange << 16) / 100;
	
	vv->VP_stressGain = vd->stressGain;
	vv->VP_stressGain = (vv->VP_stressGain << 16) / 100;

	tempLong = vd->aGain;
	tempLong = mRatio(tempLong,100,kPrecision);
	#if FLOAT_SYNTH_MT3
		zz->breathGain = mMul2(tempLong * 4,kNoiseGain,kPrecision);
	#else
		zz->breathGain = mMul2(tempLong,kNoiseGain,kPrecision);
	#endif

	zz->breathCycle = vd->aCycle;

	if (vd->f4_Freq > 800)									// $$$$
		zz->voice_F4_Freq = (*(vv->funcList->e_HzToPitch_FUNC)) (vv, vd->f4_Freq);
	else
		zz->voice_F4_Freq = vd->f4_Freq;
	zz->voice_F4_BW = vd->f4_BW;

	zz->f4_Par = (*(vv->funcList->e_HzToPitch_FUNC)) (vv, vd->f4p_Freq);
	zz->bw4_Par = vd->f4p_BW;
	
	zz->f5_Par = (*(vv->funcList->e_HzToPitch_FUNC)) (vv, vd->f5p_Freq);
	zz->bw5_Par = vd->f5p_BW;
	
	//zz->f6_Par = (*(vv->funcList->e_HzToPitch_FUNC)) (vv, vd->f6p_Freq);
	zz->f6_Par = (*(vv->funcList->e_HzToPitch_FUNC)) (vv, 4700);		// $$$$
	zz->bw6_Par = vd->f6p_BW;

	zz->nasalBaseFreq = vd->nasal_Base;
	zz->nasalTargFreq = vd->nasal_targ;
	zz->fNP = (*(vv->funcList->e_HzToPitch_FUNC)) (vv, zz->nasalBaseFreq);
	zz->bNP = vd->nasal_BW;

	InitFixedFormants (zz, vv);

	zz->voiceBWgain1 = vd->bwGain1;
	zz->voiceBWgain1 = (zz->voiceBWgain1 << 16) / 100;
	zz->voiceBWgain2 = vd->bwGain2;
	zz->voiceBWgain2 = (zz->voiceBWgain2 << 16) / 100;
	zz->voiceBWgain3 = vd->bwGain3;
	zz->voiceBWgain3 = (zz->voiceBWgain3 << 16) / 100;

	zz->voiceF1Gain = vd->f1_Offset;
	zz->voiceF2Gain = vd->f2_Offset;
	zz->voiceF3Gain = vd->f3_Offset;
	
	zz->voiceChorus = vd->chorus;
	
	tempLong = vd->nGain;
	zz->voiceNoiseGain = mRatio(tempLong,100,kPrecision);
	
	if (vv->bit16_Sound)
		{
		#if FLOAT_SYNTH_MT3
			zz->voiceNoiseGain *= 0.75;
		#else
			zz->voiceNoiseGain = mMul2(zz->voiceNoiseGain,0xCCCC,16);
		#endif
		}
	zz->setNoiseGain = zz->voiceNoiseGain;
	
	zz->sync_On_Vowel = vd->vowelSync;
	zz->loopPoint = vd->loopPoint;

	if (vd->customForm)
		{
		zz->voice_Formants[kF1] = zz->a_f1FreqTblM;
		zz->voice_Formants[kF2] = zz->a_f2FreqTblM;
		zz->voice_Formants[kF3] = zz->a_f3FreqTblM;
		zz->voice_Formants[kBW1] = zz->a_b1FreqTblM;
		zz->voice_Formants[kBW2] = zz->a_b2FreqTblM;
		zz->voice_Formants[kBW3] = zz->a_b3FreqTblM;
		zz->voice_av_Tbl = zz->a_avVolTblM;
		zz->EnvelopeListTbl = zz->a_EnvelopeListTbl;
		}
	else
		{
		if (zz->voice_Num == kMaleTbls)
			{
			zz->EnvelopeListTbl = zz->MaleEnvelopeListTbl;
			zz->voice_Formants[kF1] = zz->f1FreqTblM;
			zz->voice_Formants[kF2] = zz->f2FreqTblM;
			zz->voice_Formants[kF3] = zz->f3FreqTblM;
			zz->voice_Formants[kBW1] = zz->b1FreqTblM;
			zz->voice_Formants[kBW2] = zz->b2FreqTblM;
			zz->voice_Formants[kBW3] = zz->b3FreqTblM;
			zz->voice_av_Tbl = zz->avVolTblM;
			}
		else
			{
			zz->EnvelopeListTbl = zz->FemaleEnvelopeListTbl;
			zz->voice_Formants[kF1] = zz->f1FreqTblF;
			zz->voice_Formants[kF2] = zz->f2FreqTblF;
			zz->voice_Formants[kF3] = zz->f3FreqTblF;
			zz->voice_Formants[kBW1] = zz->b1FreqTblF;
			zz->voice_Formants[kBW2] = zz->b2FreqTblF;
			zz->voice_Formants[kBW3] = zz->b3FreqTblF;
			zz->voice_av_Tbl = zz->avVolTblF;
			}
		}
	if (zz->voice_Num == kMaleTbls)
		{
		zz->voice_NoiseAmp_Tbl = zz->Male_NoiseAmpTbl;
		zz->voice_Locus_Tbl = zz->Male_Loci_Tbl;
		zz->voiceMinBW = 50;
		}
	else
		{
		zz->voice_NoiseAmp_Tbl = zz->Female_NoiseAmpTbl;
		zz->voice_Locus_Tbl = zz->Female_Loci_Tbl;
		zz->voiceMinBW = 50;
		}

	if (vd->AsperW == 0)
		zz->breathWave = zz->BandNoisePtr;
	else if (vd->AsperW == 1)
		zz->breathWave = zz->NoiseWavePtr;
	else
		zz->breathWave = zz->HPNoisePtr;

	vv->VP_riseAmt = vd->riseAmt;
	vv->VP_fallAmt = vd->fallAmt;	
	vv->VP_riseAmt1 = vd->riseAmt1;
	vv->VP_fallAmt1 = vd->fallAmt1;

	vv->VP_assertiveness = vd->assertiveness;

	vv->VP_baselineFall = vd->baselineFall;
	vv->down_Ramp_Step = vd->down_Ramp_Step;
	vv->VP_quickness = vd->quickness;

	vv->pitchCmdStep = vd->pitchCmdStep;
	vv->durCmdStep = vd->durCmdStep;
	vv->stressDurTime = vd->stressDurTime >> 1;
	
	zz->locusOffset = vd->locus;

	zz->nasalAmt = vd->nasalAmt;
	vv->vibratoDepth1 = vd->vibratoDepth1;
	vv->vibratoDepth1 = (vv->vibratoDepth1 << 16) / 1000;
	
	vv->vibratoDepth2 = vd->vibratoDepth2;
	vv->vibratoDepth2 = (vv->vibratoDepth2 << 16) / 1000;
	
	vv->vibratoFreq = vd->vibratoFreq;
	vv->vibratoFreq = (vv->vibratoFreq << 16) / 10;
	vv->vibratoFreq = (vv->vibratoFreq * 256) / 200;		/* 256 samples/cycle,  200hz = 5ms sample time	*/
	
	vv->VP_intonation = vd->intonation;
	vv->VP_intonation = (vv->VP_intonation << 16) / 100;
	
	vv->portamento = vd->portamento / kFrameTime;
	if (vv->portamento == 0)
		vv->portamento = 1;

	if (vd->emphVoice > 0)
		zz->hfEmph = true;
	else
		zz->hfEmph = false;

	zz->reverbDepth = vd->rvbDepth;
	zz->reverbDepth = mRatio(zz->reverbDepth,100,kPrecision);

	zz->reverbDelay = vd->rvbDelay;
	if (zz->reverbDelay > 100)
		zz->reverbDelay = 100;		/* clip at 100% max	*/
	if (zz->reverbDelay < 10)
		zz->reverbDelay = 10;		/* clip at 10% min	*/
	zz->reverbDelay = (zz->reverbDelay << 16) / 100;

	vv->tempo = vd->tempo;
	
}








void	InsertSample (voiceVarPtr vv, voiceDataPtr tvPtr)
{
	int32_t		*sHeader;
	int32_t		i;
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;
	if (zz->SampleWave != NULL)
		{
		sHeader = (int32_t*) zz->SampleWave;
		zz->sampleLength = *sHeader;

		if (tvPtr->waveType == kUseSyncSnd)
			{
			i = sHeader[1];									/* Number of markers	*/
			for (i = 0; i < sHeader[1]; i++)
				{
				vv->markerBuf[i] = sHeader[i+2];
				}
			vv->lastMarkerIndex = i-1;
			zz->SampleWave = (unsigned char*) &sHeader[i+2];	/* skip len & num of markers & markers	*/
			}
		else
			{
			zz->SampleWave = zz->SampleWave + 4;			/* skip len param	*/
			}
		}

}






void	synth_ResetVoice (voiceVarPtr vv)
{
	formantVarPtr 	zz;

	zz = (formantVarPtr)vv->synthVars;

	InitVoice (vv, &zz->vd);

	InvDFT (zz, &zz->vd);
}





short	synth_NewVoice (voiceVarPtr vv, void *voice, unsigned char *sample)
{
	short			i;
	short			*theNotes;
	formantVarPtr 	zz;
	short			error;
	voiceDataPtr	vDat;


	zz = (formantVarPtr)vv->synthVars;
	vDat = (voiceDataPtr)voice;
	error = kNoError;

	/*----------------------------------------------*/
	/* Load voice parameters						*/
	/*----------------------------------------------*/
		
	if ( (vDat->waveType == kUseSnd) || (vDat->waveType == kUseSyncSnd) )
		{
		zz->SampleWave = sample;
		if (zz->SampleWave == NULL)
			{
			error = kNoWaveSample;
			goto FAILURE;
			}
		InsertSample (vv, vDat);
		}

	if (vDat->customForm > 0)
		{
		//zz->FormTables = (long*) (*(vv->funcList->_i_GetFormTables_FUNC)) (vv, vDat->customForm);
		zz->FormTables = NULL;
		if (zz->FormTables == NULL)
			{
			error = kNoFormTables;
			goto FAILURE;
			}
		SetFormAddr (vv);
		}

	zz->vd.voice = vDat->voice;
	zz->vd.pitch = vDat->pitch;
	zz->vd.pitchRange = vDat->pitchRange;
	zz->vd.stressGain = vDat->stressGain;
	zz->vd.rate = vDat->rate;
	zz->vd.vGain = vDat->vGain;
	zz->vd.aGain = vDat->aGain;
	zz->vd.aCycle = vDat->aCycle;
	zz->vd.f4_Freq = vDat->f4_Freq;
	zz->vd.f4_BW = vDat->f4_BW;
	zz->vd.f4p_Freq = vDat->f4p_Freq;
	zz->vd.f4p_BW = vDat->f4p_BW;
	zz->vd.f5p_Freq = vDat->f5p_Freq;
	zz->vd.f5p_BW = vDat->f5p_BW;
	zz->vd.f6p_Freq = vDat->f6p_Freq;
	zz->vd.f6p_BW = vDat->f6p_BW;
	zz->vd.nasal_Base = vDat->nasal_Base;
	zz->vd.nasal_targ = vDat->nasal_targ;
	zz->vd.nasal_BW = vDat->nasal_BW;
	zz->vd.locus = vDat->locus;
	zz->vd.bwGain1 = vDat->bwGain1;
	zz->vd.bwGain2 = vDat->bwGain2;
	zz->vd.bwGain3 = vDat->bwGain3;
	zz->vd.f1_Offset = vDat->f1_Offset;
	zz->vd.f2_Offset = vDat->f2_Offset;
	zz->vd.f3_Offset = vDat->f3_Offset;
	zz->vd.chorus = vDat->chorus;
	zz->vd.nGain = vDat->nGain;
	zz->vd.sPitch = vDat->sPitch;
	zz->vd.sGain = vDat->sGain;
	zz->vd.AsperW = vDat->AsperW;
	zz->vd.voiceVers = vDat->voiceVers;
	vv->voiceVers = vDat->voiceVers;

	zz->vd.riseAmt = vDat->riseAmt;
	zz->vd.fallAmt = vDat->fallAmt;
	zz->vd.riseAmt1 = vDat->riseAmt1;
	zz->vd.fallAmt1 = vDat->fallAmt1;
	zz->vd.assertiveness = vDat->assertiveness;
	zz->vd.baselineFall = vDat->baselineFall;
	zz->vd.quickness = vDat->quickness;
	zz->vd.pitchCmdStep = vDat->pitchCmdStep;
	zz->vd.durCmdStep = vDat->durCmdStep;
	zz->vd.down_Ramp_Step = vDat->down_Ramp_Step;
	zz->vd.stressDurTime = vDat->stressDurTime;

	zz->vd.tempo = vDat->tempo;


	zz->vd.waveType = vDat->waveType;
	zz->vd.sndID = vDat->sndID;
	zz->vd.vowelSync = vDat->vowelSync;
	zz->vd.loopPoint = vDat->loopPoint;

	for (i = 0; i < 48; i++)
		{
		zz->vd.vWave[i] = vDat->vWave[i];
		zz->vd.vWave1[i] = vDat->vWave1[i];
		}

	zz->vd.customForm = vDat->customForm;
	
	zz->vd.nasalAmt = vDat->nasalAmt;
	zz->vd.vibratoDepth1 = vDat->vibratoDepth1;
	zz->vd.vibratoDepth2 = vDat->vibratoDepth2;
	zz->vd.vibratoFreq = vDat->vibratoFreq;
	zz->vd.intonation = vDat->intonation;
	zz->vd.portamento = vDat->portamento;

	zz->vd.emphVoice = vDat->emphVoice;
	zz->vd.rvbDelay = vDat->rvbDelay;
	zz->vd.rvbDepth = vDat->rvbDepth;

	return (error);
	

FAILURE:	
	return (error);
}



