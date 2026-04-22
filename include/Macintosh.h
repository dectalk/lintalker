#ifndef __SPEECHMACINTOSH__
	#define __SPEECHMACINTOSH__


#include "linux_compat.h"
#include "mt4.h"


#define notSupported  -248
#define cpuTooSlow  -249

#define kSample_22K 0x56EE8BA3			/* Fixed sample rate of 22254.5454 Hz 	*/
#define kSample_22050 22050 << 16		/* Fixed sample rate for PPC 	*/


#define	kusPerTick 16639			/* number of us per tick count */
#define MSG_LEN                   128



/*----------------------------------------------*/
/* This will display a message in the debugger	*/
/*----------------------------------------------*/
#if DEBUG
	#define DebugMessage(s)			DebugStr ((ConstStr255Param)s)
#else
	#define DebugMessage(s)			((void)	0)
#endif



/*----------------------------------------------*/
/* Private 'SpeechInfo' selectors				*/
/*----------------------------------------------*/

#define soSetSoundBuf		'0001'
#define soGetSndBufEnd		'0002'
#define soControlCallBack	'0003'
#define soReinitVoice		'0004'
#define soTempo				'0005'
#define soPhonCtrlCallBack	'0006'

#define soVoiceParams	'vdat'
#define soSoundSample	'snds'
#define soVoiceTables	'vtbl'
#define soGlotGain		'glot'
#define soSampGain		'sndg'
#define soSampPitch		'sndp'
#define soVoiceVars		'vvar'


#define kLastMessage	200



struct BEShared								/* vars allocated once for all instances */
{
	short			instanceCount;			/* total number of open channels */
	const char		*DictRaw;				/* pointer to English.lex data (may be read-only flash) */
	int				dictRawOwned;			/* 1 if DictRaw was malloc'd and must be freed */
	Dict			DictMem;				/* parsed in-memory English dictionary */
	moduleFuncPtr	funcList;
	int32_t			gestaltCPU;
};
typedef struct BEShared BEShared;
typedef BEShared *BESharedPtr;



struct shellVar
{
	/*----------------------------------*/
	/* Shell Channel Vars				*/
	/*----------------------------------*/
	Ptr				sPtr;				/* Pointer to shared globals */
	Ptr				soundBufPtr;
	Ptr				reverbBufPtr;
	Ptr				formantGlobals;
	FETokenPtr		tokBufferPtr;
	short			inSpeechDT;			/* Deferred Task lock-out */
	short			doAnotherBuffer;
	short			lastSndBuffer;
	short			doneWithSpeech;
	unsigned char	*curSampleBuffer;	/* ptr to the current sample buffer */
	long			curSampleLen;
	unsigned char	*firstSampleBuffer;	/* ptr to the 1st sample buffer (double buffer) */
	long			firstSampleLen;
	short			firstBuffer;		/* boolean */

	voiceVarPtr		ChannelGlobals;
	moduleFuncPtr	funcList;

	unsigned long	recentSync;			/* last 'sync' command encountered by the Back-End */

	short			stopSpeech;
	short			soundCB_Count;

	short			cb_Count;			/* debugging */
	short			useSysHeap;
	short			*pSaveBuf;
	long			cSaveLen;

	/*----------------------------------*/
	/* Linux audio output               */
	/*----------------------------------*/
	void			*hWnd;				/* unused on Linux */
	WAVEHDR			*wh[2];				/* double-buffer headers */

	/* Accumulated PCM output (built up by SpeakBuffer) */
	short			*outputBuf;			/* heap-allocated sample buffer */
	long			outputLen;			/* samples written so far */
	long			outputCapacity;		/* allocated capacity in samples */

	/* Bridge callback for embedded streaming and lipsync */
	void			(*stream_cb)(const int16_t *samples, int count, int phoneme_id, void *userdata);
	void			*stream_userdata;
};
typedef struct shellVar shellVar;
typedef shellVar *shellVarPtr;
typedef shellVarPtr *shellVarHandle;




struct WavFileInfo
{
	short		*bufPtr;
	long		len;			/* number of samples */
	long		sampleRate;		/* hertz */
	short		numOfChan;		/* mono = 1, stereo = 2 */
	char		*fileNamePtr;
	long		wFormatTag;
};
typedef struct WavFileInfo WavFileInfo;
typedef WavFileInfo *WavFileInfoPtr;




#endif
