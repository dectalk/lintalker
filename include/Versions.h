/*
	File:		Versions.h

	Contains:	Version and Resource Type & ID constants for the 
				Speech Manager and MacInTalk 2

	Written by:	Tim Schaaff

	Copyright:	© 1992 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

		 <4>	 2/17/93	TIM		#1066800,<TIM>: Bump ye olde kMacinTalk version to 1.0b4
		 <3>	  2/4/93	TIM		Bump the version.
		 <2>	 1/21/93	TIM		#0: Bump version.
		<10>	 12/8/92	TIM		Clean up some more resource id values and bump version.
		 <9>	11/16/92	TIM		Set all resource ids to the proper range for system resources.
		 <8>	10/21/92	TIM		Add type and id constants for Speech Mgr 'vers' resource. Zapped
									version defines for Speech Mgr version.
		 <7>	 9/15/92	TIM		Add defines for Gala Tea default voice spec. Bumped version to
									1.0a2.
		 <6>	 8/12/92	TIM		Bumped Speech Mgr & MacInTalk versions to alpha 1.
		 <5>	  8/3/92	TIM		Moved kBootIconID here. Clean up code so JDR doesn't thrash me
									so much during code review.
		 <4>	 7/23/92	TIM		Switch resource and file type defines over to the “official”
									ones from Brian McGhie.
		 <3>	 7/21/92	TIM		Modified Cyclone version info and consolidated Rez type & ID
									defines.
		 <2>	 7/17/92	TIM		Bumped version.
		 <1>	 6/29/92	TIM		first created
	To Do:
*/

#ifndef __SPEECHVERSIONS__
	#define __SPEECHVERSIONS__



/* Version constant defines which are compatible with Rez	*/

#define kDevelopment_V	0x20
#define kAlpha_V		0x40
#define kBeta_V			0x60
#define kFinal_V		0x80
#define kRelease_V		0x80

/*——————————————————————————————————————————————————————————————*/
/*		Version Constants for the MacInTalk 3 Engine			*/
/*——————————————————————————————————————————————————————————————*/

#define SpeechEngineInterfaceRev	1	/* Rev of SPI to MacInTalk3 */
#define SpeechEngineCodeRev			1	/* Rev of this implementation */

#define kMT3MajorVersion 		1							/* 8-bits */
#define kMT3MinorVersion 		0x40						/* 4-bits */
#define kMT3BugFixVersion 		0							/* 4-bits */
#define kMT3DevelopmentStage 	kDevelopment_V				/* 8-bits */
#define kMT3NonRelVersion		7							/* 8-bits */

#define	kMT3VersionStr			"1.4d7"
#define	kMT3CopyrightStr		"1.4d7, © Apple Computer, Inc. 1994"

#define kMacInTalkVersion	 ((kMT3MajorVersion		<< 24) 		\
							| (kMT3MinorVersion 	<< 16) 		\
							| (kMT3BugFixVersion 	<< 16) 		\
							| (kMT3DevelopmentStage << 8)  		\
							| (kMT3NonRelVersion))



/*——————————————————————————————————————————————————————————————*/
/*		Version Constants for the MacInTalk 3 Voices			*/
/*——————————————————————————————————————————————————————————————*/

#define kVoicesMajorVersion 		1							/* 8-bits */
#define kVoicesMinorVersion 		0x40						/* 4-bits */
#define kVoicesBugFixVersion 		0							/* 4-bits */
#define kVoicesDevelopmentStage 	kDevelopment_V					/* 8-bits */
#define kVoicesNonRelVersion		7							/* 8-bits */

#define	kVoicesVersionStr			"1.4d7"
#define	kVoicesCopyrightStr			"1.4d7, © Apple Computer, Inc. 1994"


#define	kVoiceVer 		0x0104
#define	kMinVoiceVer	kVoiceVer

								
/*——————————————————————————————————————————————————————————————*/
/*				Resource Type and ID Constants					*/
/*——————————————————————————————————————————————————————————————*/


#define kTextToSpeechEngineType		'ttsc'		/* component type for speech engines */

#define kVoiceFileType				'ttvf'		/* voice file type */
#define kVoiceBundleType			'ttvb'		/* voice bundle file type */

#define kVoiceDescription			'ttvd'		/* voice description resource type */

#define kMacInTalkCreator			'mtk3'
#define kDefaultVoiceID				1



#define kShellNameStringID			1
#define kShellInfoStringID			2

/*——————————————————————————————————————————————————————————————*/
/*				"ttvi" data and code resources					*/
/*——————————————————————————————————————————————————————————————*/
#define kDataTables_TYPE			'ttvi'		/* resource type for data */

#define kRule_Rez_TYPE				kDataTables_TYPE
#define	kRule_Rez_ID				5			/* resource ID of pronunciation rules */

#define DICT_REZ_TYPE				kDataTables_TYPE
#define DICT_REZ_ID					6			/* resource ID of exceptions dictionary */
#define SYMBOL_REZ_ID				7			/* resource ID of symbols dictionary */


/*——————————————————————————————————————————————————————————————*/
/*				"ttss" phoneme symbols resource				*/
/*——————————————————————————————————————————————————————————————*/
#define PHONEME_SYMBOLS_TYPE		'ttss'		/* resource type of the phoneme symbols definitions */
#define PHONEME_SYMBOLS_ID			0			/* resource ID of the phoneme symbols definitions */

/*——————————————————————————————————————————————————————————————*/
/*				"ttvi" default voice resource					*/
/*——————————————————————————————————————————————————————————————*/
#define kVoicePrefsType				'ttsp'		/* default voice resource type */
#define kVoicePrefsID				1			/* default voice resource ID */

/*——————————————————————————————————————————————————————————————*/
/*							Icon IDs 							*/
/*——————————————————————————————————————————————————————————————*/
#define	kExtIconID					-4064
#define	kDocIconID					-4063


#endif