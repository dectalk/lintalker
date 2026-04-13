#ifndef __STDIO__
#include <stdio.h>
#endif

#include <windows.h>


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




	extern		shellVarPtr		gInstanceStorage;
	extern		unsigned long	gComponentRefcon;
	extern		short			gDoDT;
	extern		short			gSpeechIsDone;
	extern		char 			msg[MSG_LEN+1];
	

//---------------------
// Data.c
//---------------------
	extern		struct	voiceData	Fred_Voice;
	extern		struct	voiceData	Kathy_Voice;
	extern		struct	voiceData	Princess_Voice;
	extern		struct	voiceData	Junior_Voice;
	extern		struct	voiceData	Ralph_Voice;
	extern		struct	voiceData	Whisper_Voice;
	extern		struct	voiceData	Zarvox_Voice;
	extern		struct	voiceData	Trinoids_Voice;
	extern		struct	voiceData	Bubbles_Voice;

	extern		struct	voiceData	Boing_Voice;
	extern		struct	voiceData	Bells_Voice;
	extern		struct	voiceData	Hysterical_Voice;
	extern		struct	voiceData	Deranged_Voice;
	extern		struct	voiceData	GoodNews_Voice;
	extern		struct	voiceData	BadNews_Voice;
	extern		struct	voiceData	PipeOrgan_Voice;
	extern		struct	voiceData	Cellos_Voice;

//---------------------
// Sounds.c
//---------------------
	extern		char	Bells_Sound[];
	extern		char	Boing_Sound[];
	extern		char	Bubbles_Sound[];
	extern		char	Cellos_Sound[];
	extern		char	Deranged_Sound[];
	extern		char	Hysterical_Sound[];
	extern		char	PipeOrgan_Sound[];
	extern		char	Rules[];
	extern		char	Symbols[];

	extern		void	Init_BEFuncPtrs (moduleFuncPtr mfp);		// Engine.c
	extern		void	Init_FEFuncPtrs (moduleFuncPtr mfp);		// FrontEnd.c
	extern		void	Init_FSynthFuncPtrs (moduleFuncPtr mfp);	// formantSynth.c


//------------------------
// Engine.c
//------------------------
extern	short	e_SpeakBuffer ( voiceVarPtr vv, Ptr textBuf, long byteLen, long controlFlags );

//------------------------
// Say.c
//------------------------
extern	void	synth_FillNextSampBuffer (voiceVarPtr vv);

//------------------------
// Exports
//------------------------
	long 		_OpenSpeech		(HWND hWnd);
	long 		_CloseSpeech	(shellVarPtr svv);
	void   		SM_CallBack (shellVarPtr svv, LPWAVEHDR lpwh);
	long 		_UseVoice ( shellVarPtr svv, short index );
	long 		_SpeakBuffer (shellVarPtr svv, Ptr textBuf, long byteLen, long controlFlags); 

//------------------------
// Forward
//------------------------
	void		MaybeDisposeSharedGlobals (BESharedPtr sharedPtr);
	void		CloseSpeechChannel (shellVarPtr svv);
	void		Init_ShellFuncPtrs (moduleFuncPtr mfp);
	void		SendBufCmd (shellVarPtr svv, LPWAVEHDR lpwh);
	short		InitSound16 (shellVarPtr svv);
	void		Rules_LittleEndian ();
	void		MakeDictPtrsAbsolute ( char *dictPtr );
	short		WriteMMWave (WavFileInfoPtr wInfo);




long _OpenSpeech (HWND hWnd)
{

typedef	short	(*_i_Init_FuncPtr) (moduleFuncPtr mfp);

	short			error;
	BESharedPtr		sharedPtr;
	shellVarPtr		svv;
	OSErr			err;
	//_i_Init_FuncPtr	_i_Init_Func;
	FILE			*file;		/* Input File Stream */
	long			fileLen;
	fpos_t			tempZ;

	//------------------------------------------------
	// Check to see if we're the 1st instance
	// If we are, get memory for shared globals
	//------------------------------------------------
	error = kNoError;
	sharedPtr = ((BESharedPtr) gComponentRefcon);

	if (!sharedPtr)									// will be NULL if there is no shared storage yet (we're the 1st instance)
		{											// we need to allocate one for all instances to work with
		//------------------------------------------------
		// Get shared memory for all channels
		//------------------------------------------------
		sharedPtr = (BESharedPtr) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BEShared) );
		
		if (!sharedPtr)												// not enough RAM for shared globals
			{
			error = kOutOfMemory;
			goto FAILURE;											// gotta bomb out right away
			}
		gComponentRefcon = (long) sharedPtr;
		sharedPtr->instanceCount = 0;								// no channels open yet

		sharedPtr->Dict = NULL;
		sharedPtr->funcList = NULL;

		//------------------------------------------------
		// Is the CPU fast enough?
		//------------------------------------------------
		sharedPtr->gestaltCPU = 6;
		
		Rules_LittleEndian ();

		//------------------------------------------------
		// Get Exceptions Dictionary
		//------------------------------------------------		
			if ((file = fopen("English.lex", "rb")) != NULL)
				{
				err = fseek ( file, 0, SEEK_END );
				err = fgetpos ( file, (fpos_t*)&tempZ );
				fileLen = (long)tempZ;
				rewind ( file );
				
				sharedPtr->Dict = (char*) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, fileLen);
				
				if (sharedPtr->Dict)
					{
					err = fread ((Ptr)sharedPtr->Dict, sizeof(char), fileLen, file);
					err = fclose (file);
					MakeDictPtrsAbsolute (sharedPtr->Dict);
					}
				else
					{
					err = fclose (file);
					goto FAILURE1;
					}
				}
			else
				goto FAILURE1;

		MakeDictPtrsAbsolute ((char*)&Symbols);
		
		//--------------------------------------------------
		// Init the function pointers
		//--------------------------------------------------
		sharedPtr->funcList = (moduleFuncPtr) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(moduleFunc));

		if (!sharedPtr->funcList)												// not enough RAM for shared globals
			{
			error = kOutOfMemory;
			goto FAILURE;											// gotta bomb out right away
			}

		Init_ShellFuncPtrs (sharedPtr->funcList);
		
		Init_FEFuncPtrs (sharedPtr->funcList);							// init front-end func pointers
		Init_BEFuncPtrs (sharedPtr->funcList);							// init back-end func pointers

		}

	//------------------------------------------------
	// Get global channel memory for shell
	//------------------------------------------------
	svv = (shellVarPtr) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(shellVar));
	if (!svv)
		{
		error = kOutOfMemory;
		goto FAILURE;
		}

	svv->sPtr = (Ptr)sharedPtr;
	svv->soundBufPtr = NULL;						// no wave buffer yet
	svv->reverbBufPtr = NULL;						// no wave reverb yet
	svv->formantGlobals = NULL;
	svv->tokBufferPtr = (FETokenPtr) NULL;
	svv->ChannelGlobals = (voiceVarPtr) NULL;
	svv->hWnd = hWnd;
	svv->wh[0] = svv->wh[1] = NULL;
	svv->pSaveBuf = NULL;
	svv->cSaveLen = 0;

	svv->funcList = sharedPtr->funcList;

	gInstanceStorage = (shellVarPtr) svv;
	
	//------------------------------------------------
	// Get global channel memory for engine
	//------------------------------------------------
	svv->ChannelGlobals = (voiceVarPtr) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(voiceVar));
	if (!svv->ChannelGlobals)
		{
		error = kOutOfMemory;
		goto FAILURE2;
		}
	
	svv->ChannelGlobals->shellV 		= (Ptr) svv;
	svv->ChannelGlobals->funcList 		= svv->funcList;
	svv->ChannelGlobals->Dict 			= (DictPtr) sharedPtr->Dict;
	svv->ChannelGlobals->Symbols 		= (DictPtr) &Symbols;
	svv->ChannelGlobals->Rules 			= (char*)&Rules;
	svv->ChannelGlobals->cpuIsFast 		= true;
	svv->ChannelGlobals->hWnd			= svv->hWnd;

	//------------------------------------------------
	// Get sound channel and wave buffers
	//------------------------------------------------
	error = InitSound16 (svv);
	if (error == kNoError)
		{
		//------------------------------------------------
		// Use 16-bit sound
		//------------------------------------------------
		svv->ChannelGlobals->bit16_Sound = true;
		svv->ChannelGlobals->sampleBuffer1 = (unsigned char*)svv->wh[0]->lpData;
		svv->ChannelGlobals->sampleBuffer2 = (unsigned char*)svv->wh[1]->lpData;
		}
	else
		{
		error = kNoSoundHardware;
		goto FAILURE2;
		}
	svv->ChannelGlobals->reverbBufPtr 	= svv->reverbBufPtr;


	//------------------------------------------
	// Get memory for FE tokens
	//------------------------------------------
	svv->tokBufferPtr = (FETokenPtr) HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(FEToken) * kTokMax);
	
	if (!svv->tokBufferPtr)
		{
		error = kOutOfMemory;						// and return to caller
		goto FAILURE2;
		}
	svv->ChannelGlobals->tokBuffer = svv->tokBufferPtr;						// store ptr to var


	//------------------------------------------------
	// Initilize the engine
	//------------------------------------------------
	error = (*(svv->funcList->e_OpenSpeechChannel_FUNC)) (svv->ChannelGlobals);
	if (error != kNoError)
		{
		goto FAILURE2;
		}

	sharedPtr->instanceCount += 1;					// one more instance is opened
	return (noErr);


FAILURE1:
	error = kNoDataTables;
	goto FAILURE;
	

FAILURE2:
	CloseSpeechChannel (svv);
FAILURE:
	MaybeDisposeSharedGlobals (sharedPtr);
	return ( (long) error );
}




long _CloseSpeech ( shellVarPtr svv)
{

	BESharedPtr sharedPtr;

	if (svv) 
		{
		sharedPtr = (BESharedPtr)svv->sPtr;
		CloseSpeechChannel (svv);
		sharedPtr->instanceCount -= 1;							// one more instance is closed
		MaybeDisposeSharedGlobals (sharedPtr); 			// get rid of shared globals if we are the only instance
		}

	return (noErr);
}




long _SpeakBuffer (shellVarPtr svv, Ptr textBuf, long byteLen, long controlFlags) 
{
	short				error;
	//long				errorC;
	//short				i;
	//MMRESULT			rc;

	
	if (svv->ChannelGlobals->Busy)
		{
		error = synthNotReady;
		goto NOSPEECH;
		}

	svv->inSpeechDT = false;
	svv->doAnotherBuffer = false;
	svv->lastSndBuffer = false;
	svv->firstBuffer = true;
	svv->doneWithSpeech = false;
	svv->stopSpeech = false;
	svv->soundCB_Count = 0;
	svv->cb_Count = 0;
	
	error = e_SpeakBuffer (svv->ChannelGlobals, textBuf, byteLen, controlFlags);
	
	if (error == kNoError)
		{
		waveOutPause (svv->hwo);
		waveOutUnprepareHeader (svv->hwo, svv->wh[0], sizeof(WAVEHDR));           
		waveOutUnprepareHeader (svv->hwo, svv->wh[1], sizeof(WAVEHDR));           
		
		svv->ChannelGlobals->sampleBuffer = (unsigned char*)svv->wh[0]->lpData;
		synth_FillNextSampBuffer (svv->ChannelGlobals);
		SendBufCmd (svv, svv->wh[0]);							// send 1st buffer to Snd Mgr
		
		svv->ChannelGlobals->sampleBuffer = (unsigned char*)svv->wh[1]->lpData;
		synth_FillNextSampBuffer (svv->ChannelGlobals);
		SendBufCmd (svv, svv->wh[1]);							// send 2nd buffer to Snd Mgr
		
		waveOutRestart (svv->hwo);
		}
	else if (error == kNothingToSpeak)			// nothing audible...
		{
		svv->ChannelGlobals->Busy = false;
		gSpeechIsDone = true;						// ...so we're done.
		error = kNoError;						// no reason to make this an error.
		}

NOSPEECH:	
	return ((long) error);
}





long _UseVoice ( shellVarPtr svv, short index )
{
typedef	short	(*_i_Init_FuncPtr) (moduleFuncPtr mfp);
	OSErr			result;
	short			error;
	voiceDataPtr	tvPtr;
	//_i_Init_FuncPtr	_i_Init_Func;
	BESharedPtr 	sharedPtr;
	unsigned char	*sample;


	result = noErr;
	sharedPtr = (BESharedPtr)svv->sPtr;

	switch (index)
		{
		case 0:
			tvPtr = (voiceDataPtr) &Fred_Voice;
			sample = NULL;
		break;

		case 1:
			tvPtr = (voiceDataPtr) &Kathy_Voice;
			sample = NULL;
		break;

		case 2:
			tvPtr = (voiceDataPtr) &Princess_Voice;
			sample = NULL;
		break;

		case 3:
			tvPtr = (voiceDataPtr) &Junior_Voice;
			sample = NULL;
		break;

		case 4:
			tvPtr = (voiceDataPtr) &Ralph_Voice;
			sample = NULL;
		break;

		case 5:
			tvPtr = (voiceDataPtr) &Whisper_Voice;
			sample = NULL;
		break;

		case 6:
			tvPtr = (voiceDataPtr) &Zarvox_Voice;
			sample = NULL;
		break;

		case 7:
			tvPtr = (voiceDataPtr) &Trinoids_Voice;
			sample = NULL;
		break;
		
		case 8:
			tvPtr = (voiceDataPtr) &Bubbles_Voice;
			sample = (unsigned char*)&Bubbles_Sound;
		break;
		
		case 9:
			tvPtr = (voiceDataPtr) &Boing_Voice;
			sample = (unsigned char*)&Boing_Sound;
		break;
		
		case 10:
			tvPtr = (voiceDataPtr) &Bells_Voice;
			sample = (unsigned char*)&Bells_Sound;
		break;
		
		case 11:
			tvPtr = (voiceDataPtr) &Hysterical_Voice;
			sample = (unsigned char*)&Hysterical_Sound;
		break;
		
		case 12:
			tvPtr = (voiceDataPtr) &Deranged_Voice;
			sample = (unsigned char*)&Deranged_Sound;
		break;
		
		case 13:
			tvPtr = (voiceDataPtr) &GoodNews_Voice;
			sample = NULL;
		break;
		
		case 14:
			tvPtr = (voiceDataPtr) &BadNews_Voice;
			sample = NULL;
		break;
		
		case 15:
			tvPtr = (voiceDataPtr) &PipeOrgan_Voice;
			sample = (unsigned char*)&PipeOrgan_Sound;
		break;
		
		case 16:
			tvPtr = (voiceDataPtr) &Cellos_Voice;
			sample = (unsigned char*)&Cellos_Sound;
		break;

		default:
			tvPtr = (voiceDataPtr) &Fred_Voice;
			sample = NULL;
		break;
		}

	//------------------------------------------------
	// Formant synth voice
	//------------------------------------------------
	if (svv->formantGlobals == NULL)
		{
		svv->formantGlobals = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(formantVar));		// Get Formant global memory

		if (!svv->formantGlobals)
			{
			error = kOutOfMemory;
			goto VOICE_FAILURE;
			}
		svv->ChannelGlobals->synthVars = (void*)svv->formantGlobals;
		}
	Init_FSynthFuncPtrs (sharedPtr->funcList);						// init Formant Synth func pointers

	error = (*(svv->funcList->e_UseVoice_FUNC)) (svv->ChannelGlobals, tvPtr, sample);
	

	return ( (long)error );

//------------------------------------------------
// Voice failures
//------------------------------------------------
VOICE_FAILURE:
	return ( (long)error );
}






static void	CloseSpeechChannel (shellVarPtr svv)
{

	//---------------------------------------------
	// Get rid of the SoundChannel.
	//---------------------------------------------
	if (svv->wh[0])
		{
		waveOutUnprepareHeader (svv->hwo, svv->wh[0], sizeof(WAVEHDR));
		HeapFree(GetProcessHeap(), 0, svv->wh[0]);
		}           
	if (svv->wh[1])
		{
		waveOutUnprepareHeader (svv->hwo, svv->wh[1], sizeof(WAVEHDR));           
		HeapFree(GetProcessHeap(), 0, svv->wh[1]);
		} 
		          
	(void) waveOutClose (svv->hwo);

	//---------------------------------------------
	// Dispose of memory allocated for this channel.
	//---------------------------------------------
	if (svv->soundBufPtr != NULL)
		HeapFree(GetProcessHeap(), 0, svv->soundBufPtr);
		
	if (svv->tokBufferPtr != NULL)
		HeapFree(GetProcessHeap(), 0, svv->tokBufferPtr);

	if (svv->reverbBufPtr != NULL)
		HeapFree(GetProcessHeap(), 0, svv->reverbBufPtr);
		
	if (svv->formantGlobals != NULL)
		HeapFree(GetProcessHeap(), 0, svv->formantGlobals);
		
	if (svv->ChannelGlobals != NULL)
		{
		HeapFree(GetProcessHeap(), 0, svv->ChannelGlobals);
		}

	HeapFree(GetProcessHeap(), 0, svv);
}





static void	MaybeDisposeSharedGlobals (BESharedPtr sharedPtr)
{
	if (sharedPtr != NULL)										// we think we have a handle to some shared globals
		{
		if (sharedPtr->instanceCount == 0)						// but no more active instances left
			{
			//-----------------------------------
			// Frontend
			//-----------------------------------
			if (sharedPtr->Dict != NULL)
				{
				HeapFree (GetProcessHeap(), 0, sharedPtr->Dict);
				}

			//-----------------------------------
			// Segment Func pointers
			//-----------------------------------
			if (sharedPtr->funcList != NULL)
				{
				HeapFree (GetProcessHeap(), 0, sharedPtr->funcList);
				}

			gComponentRefcon = 0;
			HeapFree (GetProcessHeap(), 0, sharedPtr);		// and get rid of the shared globals storage
			}
		}
}





static	void	Rules_LittleEndian ()
{
	short			i, index;
	unsigned char	*hashPtr;
	unsigned char	ch1, ch2;
	
	
	hashPtr = (unsigned char*)&Rules;
	for (i = 0; i < 26; i++)
		{
		index = i << 1;
		ch1 = hashPtr[index];
		ch2 = hashPtr[index+1];
		
		hashPtr[index] = ch2;
		hashPtr[index+1] = ch1;
		}

}





static void  SendBufCmd (shellVarPtr svv, LPWAVEHDR lpwh)
{
	MMRESULT		rc;
	//long			i;
	//LPWAVEHDR		lpwh;
	
	
	svv->soundCB_Count++;						// another pending sound callback

	lpwh->dwBufferLength 	= svv->curSampleLen << 1;
	
	rc = waveOutPrepareHeader (svv->hwo, lpwh, sizeof(WAVEHDR));       

	// write buffers to the queue                            

	if (rc == MMSYSERR_NOERROR)                                  
		rc = waveOutWrite (svv->hwo, lpwh, sizeof(WAVEHDR));       
	
	if (rc != MMSYSERR_NOERROR)                                  
		{
		waveOutGetErrorText (rc, msg, MSG_LEN),                        
		MessageBox (svv->hWnd, msg, NULL, MB_OK);
		}
}









void   SM_CallBack (shellVarPtr svv, LPWAVEHDR lpwh)
{
	
	
	svv->soundCB_Count--;						// used another pending sound callback
	
	if (!svv->stopSpeech)
		{
		//if ( !svv->doneWithSpeech )			// last buffer not yet sent
		//	{
			
		//	}
		
		if (!svv->lastSndBuffer)
			{
			svv->ChannelGlobals->sampleBuffer = (unsigned char*)lpwh->lpData;
			synth_FillNextSampBuffer (svv->ChannelGlobals);
			SendBufCmd (svv, lpwh);
			}
		else
			{
			//if (!svv->doneWithSpeech)
			//	{
			//	svv->ChannelGlobals->sampleBuffer = (unsigned char*)lpwh->lpData;
			//	synth_FillNextSampBuffer (svv->ChannelGlobals);
			//	SendBufCmd (svv, lpwh, true);
			//	svv->doneWithSpeech = true;
			//	}
			//else
				{
				svv->ChannelGlobals->Busy = false;
				gSpeechIsDone = true;
				}
			}
		}
}









BOOL OpenSndOutput (shellVarPtr svv)
{
   WAVEOUTCAPS  	woc;                                               
	WAVEFORMATEX 	wfx;
	UINT         	nDevId;
	MMRESULT     	rc;                                                
	UINT         	nMaxDevices = waveOutGetNumDevs();                  
	
	svv->hwo = NULL;
		                                                                
	for (nDevId = 0; nDevId < nMaxDevices; nDevId++)                         
	{                                                               
	   rc = waveOutGetDevCaps (nDevId, &woc, sizeof(woc));
	   if (rc == MMSYSERR_NOERROR)                                  
	   {
          // attempt 22.05 kHz mono if device is capable

          if (woc.dwFormats & WAVE_FORMAT_2M16)
          	{
              wfx.nChannels      = 1;      // mono
              wfx.nSamplesPerSec = 22050;  // 22.05 kHz (22.05 * 1000)
          	}
          else
          { 
          	//break;  
              wfx.nChannels      = woc.wChannels;  // use DevCaps # channels
              wfx.nSamplesPerSec = 22050;  // 22.05 kHz (22.05 * 1000)
          }

          wfx.wFormatTag      = WAVE_FORMAT_PCM;                          
      	 wfx.wBitsPerSample  = 16;                                        
      	 wfx.nBlockAlign     = wfx.nChannels * wfx.wBitsPerSample / 8;   
      	 wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;     
      	 wfx.cbSize          = 0;                                        

	       rc = waveOutOpen (&svv->hwo, nDevId, &wfx, (DWORD)svv->hWnd, 0,                   
	                        CALLBACK_WINDOW);
	                                                  
	       if (rc == MMSYSERR_NOERROR)
          {
              DWORD dwVol;

              // set volume level to at least 80%

              rc = waveOutGetVolume (svv->hwo, &dwVol);

              if (rc == MMSYSERR_NOERROR)
                  if ( LOWORD(dwVol) < 0xCCCC ||
                     ( wfx.nChannels == 2 && HIWORD(dwVol) < 0xCCCC )  )
                      rc = waveOutSetVolume( svv->hwo, (DWORD)MAKELONG( 0xCCCC, 0xCCCC ) );
          }
	       
          if (rc != MMSYSERR_NOERROR)
	       {                                                        
	           //waveOutGetErrorText (rc, msg, MSG_LEN),                    
	           MessageBox (svv->hWnd, "Error #1", NULL, MB_OK);
	           return(FALSE);
	       }
	       
	       break;                                                        
	   }                                                            
	}                                                               
	
   // device not found, error condition
   //..................................

	if (svv->hwo == NULL)
	    return(FALSE);

   return(TRUE);
}



static short	InitSound16 (shellVarPtr svv)
{
	short		error;
	//long		result;

	error = kNoError;										// start with no errors

	//------------------------------------------
	// Get memory for SndMgr double s
	//------------------------------------------
	svv->soundBufPtr = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, 4 * kSampBufLen);

	if (!svv->soundBufPtr)
		{
		error = kOutOfMemory;						// and return to caller
		goto FAILURE1;
		}
	
	

	//------------------------------------------
	// Get memory for reverb delay buffer
	//------------------------------------------
	svv->reverbBufPtr = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, kMaxTap * sizeof(rShort));
	if (!svv->reverbBufPtr)
		{
		error = kOutOfMemory;						// and return to caller
		goto FAILURE2;
		}

	//------------------------------------------
	// Open a SndMgr Sound Channel
	//------------------------------------------
	if (OpenSndOutput (svv))
		{
		//------------------------------------------
   		// Allocate WAVEHDR buffer block
		//------------------------------------------
		svv->wh[0] = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WAVEHDR) );
		svv->wh[1] = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WAVEHDR) );
		svv->wh[0]->lpData = (char*) svv->soundBufPtr;
		svv->wh[1]->lpData = (char*) svv->soundBufPtr + (kSampBufLen << 1);
		}
	else
		{
		error = kNoSoundHardware;
		goto FAILURE3;
		}  

	return (error);


FAILURE3:
	HeapFree(GetProcessHeap(), 0, svv->reverbBufPtr);
	svv->reverbBufPtr = NULL;
FAILURE2:
	HeapFree(GetProcessHeap(), 0, svv->soundBufPtr);
	svv->soundBufPtr = NULL;
FAILURE1:
	return (error);
}




static	void	Long_LittleEndian (unsigned char *locPtr)
{
	unsigned char	ch1, ch2, ch3, ch4;
	
	
	ch1 = locPtr[0];
	ch2 = locPtr[1];
	ch3 = locPtr[2];
	ch4 = locPtr[3];
	
	locPtr[0] = ch4;
	locPtr[1] = ch3;
	locPtr[2] = ch2;
	locPtr[3] = ch1;
}



static	void	Short_LittleEndian (unsigned char *locPtr)
{
	unsigned char	ch1, ch2;
	
	ch1 = locPtr[0];
	ch2 = locPtr[1];
	
	locPtr[0] = ch2;
	locPtr[1] = ch1;
}




static void	MakeDictPtrsAbsolute ( char *dictPtr )
{
	DictPtr 		d;
	long 			i, j;
	IndexEntry		*index;
	long			temp;
	

	if (dictPtr) 
		{
		d = (DictPtr) dictPtr;
		
		//------------------------------------
		// Convert Header to Little Endian
		//------------------------------------
		Long_LittleEndian ((unsigned char*)&d->version);
		Long_LittleEndian ((unsigned char*)&d->type);
		
		Long_LittleEndian ((unsigned char*)&d->wordCount);
		for (i = 0; i < HASH_ENTRIES; i++)
			Long_LittleEndian ((unsigned char*)&d->hash[i]);

		for (i = 0; i < kPOS_Slots; i++)
			{
			for (j = 0; j < 4; j++)
				{
				Short_LittleEndian ((unsigned char*)&d->POScodes[i][j]);
				}
			}
			
		Long_LittleEndian ((unsigned char*)&d->words);
		Long_LittleEndian ((unsigned char*)&d->index);
		Long_LittleEndian ((unsigned char*)&d->flags);

		//---------------------------------------------------------------
		// Convert Word Ptrs from relative addrs to absolute addrs
		//---------------------------------------------------------------
		d->words += (unsigned long) d;
		d->index = (IndexEntry*)((unsigned long)d->index + (unsigned long) d);
	
		index = d->index;

		for (i = d->wordCount - 1; i >= 0; --i)
			{
			Long_LittleEndian ((unsigned char*)index);
			temp = (long) *index;
			temp += (unsigned long) d;
			*index = (IndexEntry)temp;
			index++;
			//*index++ += (unsigned long) d;		// convert index ptrs from relative addrs to absolute addrs
			}
		}
}









static void	_i_Last_Snd_Buffer (voiceVarPtr vv)
{
	shellVarPtr	svv;
	
	
	svv = (shellVarPtr) vv->shellV;							// recover shell channel globals
	svv->lastSndBuffer = true;
}


static void	_i_Cur_Sample_Buffer (voiceVarPtr vv, unsigned char	*sampleBuffer, long sampleLen)
{
	shellVarPtr	svv;
	
	
	svv = (shellVarPtr) vv->shellV;							// recover shell channel globals
	svv->curSampleBuffer = sampleBuffer;
	svv->curSampleLen = sampleLen;
}


static void	_i_First_Sample_Buffer (voiceVarPtr vv, unsigned char	*sampleBuffer, long sampleLen)
{
	shellVarPtr	svv;
	
	
	svv = (shellVarPtr) vv->shellV;							// recover shell channel globals
	svv->firstSampleBuffer = sampleBuffer;
	svv->firstSampleLen = sampleLen;
	svv->curSampleLen = sampleLen;
}








static void	Init_ShellFuncPtrs (moduleFuncPtr mfp)
	{
	mfp->_i_Last_Snd_Buffer_FUNC 		= (_i_Last_Snd_Buffer_Ptr) &_i_Last_Snd_Buffer;
	mfp->_i_Cur_Sample_Buffer_FUNC 		= (_i_Cur_Sample_Buffer_Ptr) &_i_Cur_Sample_Buffer;
	mfp->_i_First_Sample_Buffer_FUNC 	= (_i_First_Sample_Buffer_Ptr) &_i_First_Sample_Buffer;
	}






//------------------------------------------
// Standard types
//------------------------------------------
#define RIFF_TYPE_WAVE		mmioFOURCC ('W','A','V','E')
#define RIFF_TYPE_FORMAT	mmioFOURCC ('f','m','t',' ')
#define RIFF_TYPE_DATA		mmioFOURCC ('d','a','t','a')



short WriteMMWave (WavFileInfoPtr wInfo)
{
	long		dwNumSamples, dwSize;
	long		dwNumSamplesPerSec, dwNumAvgBytesPerSec;
	short		wChannels, wBlockAlign, wBitsPerSample, wFormatTag;
	FOURCC		ckid, fccType;
	WAVEFORMATEX			wfex;

	FILE		*pFile			= NULL;
	short		errCode			= 0;		// no error

	//-----------------------------------------
	// Set parameters of wave file
	// NOTE: assumes 16-bit samples
	//-----------------------------------------
	dwNumSamples		= wInfo->len;
	dwNumSamplesPerSec	= wInfo->sampleRate;
	wChannels			= wInfo->numOfChan;
	dwNumAvgBytesPerSec	= sizeof(short) * dwNumSamplesPerSec * wChannels;
	wBlockAlign			= sizeof(short) * wChannels;
	wBitsPerSample		= sizeof(short) * 8;
	wFormatTag			= WAVE_FORMAT_PCM;


	//-----------------------------------------
	// Open output file
	//-----------------------------------------
	if ((pFile = fopen (wInfo->fileNamePtr, "w+b")) == NULL)
		{
		goto WAV_ERROR;
		}

	//--------------------------------------------------------------
	// Example of 30 sample, stereo file:
	//
	//'RIFF'
	//	ckSizeA					4		156
	//	'WAVE'
	//
	//'fmt '
	//	ckSizeB					4		16
	//		wFormatTag			2		1
	//		wChannels			2		2
	//		dwNumSamplesPerSec	4		22050
	//		dwNumAvgBytesPerSec	4		88200
	//		wBlockAlign			2		4
	//		cbSize				4
	//	wBitsPerSample			2		16
	//
	//'data'
	//	ckSizeC							120
	//
	//	30 stereo samples
	//--------------------------------------------------------------

	ckid = FOURCC_RIFF;
	dwSize =	(3 * sizeof (FOURCC)) +			// 'WAVE' + 'fmt ' + 'data'
				(2 * sizeof (DWORD)) +			// ckSizeB + ckSizeC
				sizeof (WAVEFORMATEX) + 
				(dwNumSamples * sizeof(short) * wChannels);

	fccType = RIFF_TYPE_WAVE;
	if (	(fwrite (&ckid, sizeof (FOURCC), 1, pFile) != 1) ||
			(fwrite (&dwSize, sizeof (DWORD), 1, pFile) != 1) ||
			(fwrite (&fccType, sizeof (FOURCC), 1, pFile) != 1))
		goto WAV_ERROR;

	//------------------------------
	// Write 'fmt ' subchunk
	//------------------------------
	memset (&wfex, 0, sizeof(wfex));
	ckid = RIFF_TYPE_FORMAT;
	dwSize = sizeof (wfex);


	if (	(fwrite (&ckid, sizeof (FOURCC), 1, pFile) != 1) ||
			(fwrite (&dwSize, sizeof (DWORD), 1, pFile) != 1))
		goto WAV_ERROR;
	
	wfex.wFormatTag			= wFormatTag;
	wfex.nChannels			= wChannels;
	wfex.nSamplesPerSec		= dwNumSamplesPerSec;
	wfex.nAvgBytesPerSec	= dwNumAvgBytesPerSec;
	wfex.nBlockAlign		= wBlockAlign;
	wfex.wBitsPerSample		= wBitsPerSample;
	wfex.cbSize				= 0;

	if (fwrite (&wfex, sizeof(wfex), 1, pFile) != 1)
		goto WAV_ERROR;

	//------------------------------
	// Write 'data' subchunk
	//------------------------------
	ckid = RIFF_TYPE_DATA;
	dwSize = dwNumSamples * sizeof(short) * wChannels;
	if (	(fwrite (&ckid, sizeof (FOURCC), 1, pFile) != 1) ||
			(fwrite (&dwSize, sizeof (DWORD), 1, pFile) != 1) )
		goto WAV_ERROR;


	if (fwrite (wInfo->bufPtr, sizeof (char), dwSize, pFile) != (size_t)dwSize)
		goto WAV_ERROR;

	//------------------------------------------------------
	// We are done -- files are closed below.
	//------------------------------------------------------
	goto EXIT_FUNCTION;


WAV_ERROR:
	errCode = 1;


EXIT_FUNCTION:
	//------------------------------------------------------
	// Close file if open
	//------------------------------------------------------
	if (pFile)
		fclose (pFile);

	return (errCode);
}


