/*
	File:		Speech.h

	Contains:	Interfaces to Speech Manager

	Written by:	Tim Schaaff

	Copyright:	� 1991-1993 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

		 <4>	 4/23/93	MC		?
		 <2>	 3/10/93	JDR		moved error codes into Errors
		 <1>	 3/10/93	JDR		moved to {CIncludes}
------------------------------------
		 <2>	 1/14/93	TIM		Rename badPhonemeText error return to badInputText.
		 <5>	11/30/92	TIM		Renamed callback function typedefs to fix name-space conflicts
									between Speech Mgr and Quicktime Movies.h and to make them more
									Speech Mgr specific.
		 <4>	10/28/92	TIM		Put badPhonemeText error code back into API.
		 <3>	 9/15/92	TIM		Removed badStructLen, unimplMsg, badSelector, badParmVal error
									return. Added �voiceNotFound� error return. Added length param
									to GetVoiceDescription. Prefixed some constants with �k� to
									conform to Apple naming conventions.
		 <2>	 7/23/92	TIM		Switch resource and file type defines over to the �official�
									ones from Brian McGhie.
		 <1>	 7/6/92		TIM		Add support for Voice management routines
		 <0>	 6/24/92	TIM		Bring interfaces up to date with 1.0a7 spec
		<-1>	 4/21/92	TIM		Bring interfaces up to date with 1.0a5 spec
		<-2>	 2/13/92	TIM		Bring interfaces up to date with 1.0a3 spec
		<-3>	11/19/91	TIM		Pass voice Handle argument to ReadNthVoice and ReadNamedVoice by
									reference
		<-4>	11/14/91	TIM		Flesh out more of the API
		<-5>	11/11/91	TIM		Bring interfaces up to date with ERS
		<-6>	 11/6/91	TIM		first checked in

*/

#ifndef __SPEECHEQU__
	#define __SPEECHEQU__

#include <stdint.h>


#if 0
/*Speech Manager errors*/
#define noSynthFound = -240,
#define synthOpenFailed = -241,
#define synthNotReady = -242,
#define bufTooSmall = -243,
#define voiceNotFound = -244,
#define incompatibleVoice = -245,
#define badDictFormat = -246,
#define badInputText = -247



#define gestaltSpeechAttr 			'ttsc'	/* Gestalt Manager selector for Speech Attributes */

enum {
    gestaltSpeechMgrPresent = 0				/* Gestalt bit which indicates that Speech Manager exists */
};
#endif





#define kTextToSpeechSynthType		 'ttsc'	/* Text-to-Speech Synthesizer component type 	*/
#define kTextToSpeechVoiceType		 'ttvd'	/* Text-to-Speech Voice resource type 			*/
#define kTextToSpeechVoiceFileType	 'ttvf'	/* Text-to-Speech Voice file type 				*/
#define kTextToSpeechVoiceBundleType 'ttvb'	/* Text-to-Speech Voice Bundle file type		*/

enum {										/* constants for SpeakBuffer and TextDone callback controlFlags bits */
	kNoEndingProsody 	= 1,
	kNoSpeechInterrupt 	= 2,
	kPreflightThenPause	= 4
};

enum {										/* constants for StopSpeechAt and PauseSpeechAt */
	kImmediate		= 0,
	kEndOfWord		= 1,
	kEndOfSentence	= 2
};

#define soStatus				'stat'		/* GetSpeechInfo & SetSpeechInfo selectors */
#define soErrors				'erro'
#define soInputMode				'inpt'
#define soCharacterMode			'char'
#define soNumberMode			'nmbr'
#define soRate					'rate'
#define soPitchBase				'pbas'
#define soPitchMod				'pmod'
#define soVolume				'volm'
#define soSynthType				'vers'
#define soRecentSync			'sync'
#define soPhonemeSymbols		'phsy'
#define soCurrentVoice			'cvox'
#define soCommandDelimiter		'dlim'
#define soReset					'rset'
#define soCurrentA5				'myA5'
#define soRefCon				'refc'
#define soTextDoneCallBack		'tdcb'
#define soSpeechDoneCallBack	'sdcb'
#define soSyncCallBack			'sycb'
#define soErrorCallBack			'ercb'
#define soPhonemeCallBack		'phcb'
#define soWordCallBack			'wdcb'
#define soSynthExtension		'xtnd'
#define soSoundOutput			'sndo'

#define soNote					'note'



/* Speaking Mode Constants */

#define modeText		'TEXT'		/* input mode constants 				*/
#define modeTX			'TX'
#define modePhonemes	'PHON'
#define modePH			'PH'
#define modeNormal		'NORM'		/* character mode and number mode constants */
#define modeLiteral		'LTRL'


/* GetVoiceInfo selectors 				*/

#define soVoiceDescription	'info'	/* gets basic voice info 				*/
#define soVoiceFile			'fref'	/* gets voice file ref info 			*/


struct SpeechChannelRecord
{
	long data[1];
};

typedef struct SpeechChannelRecord SpeechChannelRecord;
typedef SpeechChannelRecord *SpeechChannel;





struct VoiceSpec
{
	uint32_t	creator;
	uint32_t	id;
};

typedef struct VoiceSpec VoiceSpec;

enum {kNeuter = 0, kMale, kFemale};	/* returned in gender field below 		*/




struct VoiceDescription {
	int32_t		length;
	VoiceSpec 	voice;
	int32_t		version;
	unsigned char	name[64];
	unsigned char	comment[256];
	short		gender;
	short		age;
	short		script;
	short		language;
	short		region;
	int32_t		reserved[4];
};

typedef struct VoiceDescription VoiceDescription;





struct SpeechStatusInfo {
	unsigned char	outputBusy;
	unsigned char	outputPaused;
	int32_t		inputBytesLeft;
	short		phonemeCode;
};

typedef struct SpeechStatusInfo SpeechStatusInfo;





struct SpeechErrorInfo {
	short	count;
	short	oldest;
	int32_t	oldPos;
	short	newest;
	int32_t	newPos;
};

typedef struct SpeechErrorInfo SpeechErrorInfo;





struct PhonemeInfo {
	short		opcode;
	unsigned char	phStr[16];
	unsigned char	exampleStr[32];
	short		hiliteStart;
	short		hiliteEnd;
};

typedef struct PhonemeInfo PhonemeInfo;





struct PhonemeDescriptor
{
	short		phonemeCount;
	PhonemeInfo	thePhonemes[1];
};

typedef struct PhonemeDescriptor PhonemeDescriptor;





struct SpeechXtndData
{
	uint32_t	synthCreator;
	unsigned char	synthData[2];
};

typedef struct SpeechXtndData SpeechXtndData;





struct DelimiterInfo
{
	unsigned char	startDelimiter[2];		/* defaults to�[[� 	*/
	unsigned char	endDelimiter[2];		/* defaults to �]]� 	*/
};
typedef struct DelimiterInfo DelimiterInfo;


#endif
