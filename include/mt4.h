#ifndef __SPEECHSAY__
	#define __SPEECHSAY__

#include "MacTypes.h"
#include "linux_compat.h"

#ifndef __SPEECHEQU__
	#include "SpeechEqu.h"
#endif


#define POWERPC_NATIVE_MT3	1				// Set TRUE for PPC Build
//#define FLOAT_SYNTH_MT3		1			// DON'T SET THIS! NOT WORKING.




/*--------------------------*/
/* POS codes				*/
/*--------------------------*/

#define kNoun		0
#define kVerb		1
#define kAdj		2
#define kPrep		3
#define kVaux		4
#define kRVaux		5
#define kInterj		6
#define kConj		7
#define kCConj		8
#define kInterr		9
#define kDet		10
#define kAdv		11
#define kInf		12
#define kGen		13
#define kRelPro		14
#define kPPron		15
#define kIPron		16
#define kRPron		17
#define kDPron		18
#define kArt		19
#define kQuant		20
#define kNeg		21
#define kSadv		22
#define kContr		23
#define kVPart		24
#define kSubjPron	25
#define kObjPron	26

#define kMaxPOS		32
#define kPOSmask	(kMaxPOS-1)
#define kUndefPOS	(-1)
#define kAbriv		kMaxPOS+kNoun

#define kLastPOS	kAbriv


/*--------------------------*/
/* Composite POS codes		*/
/*--------------------------*/
#define kHas_Noun		(1<<0)
#define kHas_Verb		(1<<1)
#define kHas_Adj		(1<<2)
#define kHas_Prep		(1<<3)
#define kHas_Vaux		(1<<4)
#define kHas_RVaux		(1<<5)
#define kHas_Interj		(1<<6)
#define kHas_Conj		(1<<7)
#define kHas_CConj		(1<<8)
#define kHas_Interr		(1<<9)
#define kHas_Det		(1<<10)
#define kHas_Adv		(1<<11)
#define kHas_Inf		(1<<12)
#define kHas_Gen		(1<<13)
#define kHas_RelPro		(1<<14)
#define kHas_PPron		(1<<15)
#define kHas_IPron		(1<<16)
#define kHas_RPron		(1<<17)
#define kHas_DPron		(1<<18)
#define kHas_Art		(1<<19)
#define kHas_Quant		(1<<20)
#define kHas_Neg		(1<<21)
#define kHas_Sadv		(1<<22)
#define kHas_Contr		(1<<23)
#define kHas_VPart		(1<<24)
#define kHas_SubjPron	(1<<25)
#define kHas_ObjPron	(1<<26)


/*--------------------------*/
/* Prosodic boundary types	*/
/*--------------------------*/
#define kBND_Pause		1	// comma
#define kBND_Decl		2	// declarative
#define kBND_Quest		3	// yes-no question
#define kBND_Emph		4	// exclamation
#define kBND_Paren_L	5	// left paren
#define kBND_Paren_R	6	// right paren

#define kBND_Sep1		7	// boundary strenth 1 (punctuation)
#define kBND_Sep2		8	// boundary strenth 2 (conjunction)
#define kBND_Sep3		9	// boundary strenth 3 (subject/predicate)
#define kBND_Sep4		10	// boundary strenth 4 (subordinating conjunction)
#define kBND_Sep5		11	// boundary strenth 5 (relative clause)
#define kBND_Sep6		12	// boundary strenth 6 (preposition)
#define kBND_Sep7		13	// boundary strenth 7 (quotative tag)


#define kBND_None		0


/*--------------------------*/
/* ERROR codes				*/
/*--------------------------*/

#define	kNoError			0
#define	kNoDataTables		-1
#define	kOutOfMemory		-2
#define	kNoSoundHardware	-3
#define	kNoWaveSample		-4
#define	kParamError			-5
#define	kUnknownEmbeddedCmd	-6
#define	kNoFormTables		-7
#define	synthNotReady		-8
#define	bufTooSmall			-9

#define	kNothingToSpeak		(-107)




#define	kMaxVoice 9			/* last internal voice	*/



/* Text-to-phon errors	*/

#define	kNoPerror 0
#define	kPbufFull 1				/* phon buffer is full	*/
#define	kPphonEnd 2				/* end of input	*/
#define	kNoPhons 3				/* no phons in word	*/
#define kPbufHold 4				/* no word in FE buffer	*/
#define kGotFullSen 5			/* parsed a full sentence (GetNextSentence)	*/




/* Phon Controls	*/

#define  EndCtrl 0x8000			/* end of phons	*/
#define  HoldCtrl 0x8001		/* wait for next word	*/
#define  NopCtrl 0x8002			/* continue	*/
#define  WordCtrl 0x8003		/* word boundry	*/
#define  EmbedCmd 0x8004		/* embedded command	*/
#define  FirstByte 0x8005

#define kCQsize 32					/* Command queue size  MUST BE A POWER OF 2!!!	*/
#define	kMaxMarkers 32
/* Flow Control  Opcodes	*/
#define BE_EOF				((BECommand) 0x8000)	/* End of commands	*/
#define BE_ECmd				((BECommand) 0x8004)	/* Start of embedded command	*/

/*	Embedded command format currently defined as follows:
	
		BE_ECmd opcode		(1 word)
		Cmd Word Length		(1 word) 	(includes BE_ECmd, Length field, Selector, & Argument data)
		Cmd Selector		(2 words) 	(32-bit unsigned long)
		Cmd Arguments		(varies)	(parameters required by the particular command)
		
*/

/* Embedded Command Selectors	*/
#define EC_pbas			((unsigned long) 'pbas')		/* change baseline pitch 		(absolute)	*/
#define EC_pbar			((unsigned long) 'pbar')		/* change baseline pitch 		(relative)	*/
#define EC_pmod			((unsigned long) 'pmod')		/* change pitch modulation 		(absolute)	*/
#define EC_pmor			((unsigned long) 'pmor')		/* change pitch modulation 		(relative)	*/
#define EC_rate			((unsigned long) 'rate')		/* change speaking rate 		(absolute)	*/
#define EC_ratr			((unsigned long) 'ratr')		/* change speaking rate 		(relative)	*/
#define EC_volm			((unsigned long) 'volm')		/* change speaking volume 		(absolute)	*/
#define EC_volr			((unsigned long) 'volr')		/* change speaking volume 		(relative)	*/
#define EC_slnc			((unsigned long) 'slnc')		/* insert silence into speech	*/
#define EC_svox			((unsigned long) 'svox')		/* change current voice	*/
#define EC_sync			((unsigned long) 'sync')		/* make synchronization callback	*/
#define EC_word			((unsigned long) 'word')		/* word callback marker info	*/
#define EC_rset			((unsigned long) 'rset')		/* reset command	*/

#define EC_note			((unsigned long) 'note')		/* sing the note	*/
#define EC_tempo		((unsigned long) 'tmpo')		/* sing the note	*/
#define EC_marker		((unsigned long) 'mark')		/* sing the note	*/
#define EC_sing			((unsigned long) 'sing')		/* DECtalk singing [ph<dur,note>] */

typedef unsigned short BECommand, *BECommandPtr, **BECommandHandle;	/* command to the Back-End	*/



/*----------------------------------*/
/* Embedded and Control Commands	*/
/*----------------------------------*/

#define  C_Cmd 0
#define  C_absPitch 1			/*  abs pitch control	*/
#define  C_relPitch 2			/*  rel pitch control	*/
#define  C_absRate 3			/*  abs rate control	*/
#define  C_relRate 4			/*  rel rate control	*/
#define  C_absMod 5				/*  abs mod control	*/
#define  C_relMod 6				/*  rel mod control	*/
#define  C_absVol 7				/*  abs vol control	*/
#define  C_relVol 8				/*  rel vol control	*/
#define  C_silence 9			/*  silence control	*/
#define  C_voice 10				/*  voice control	*/
#define  C_sync 11				/*  sync CB control	*/
#define  C_word 12				/*  word CB control	*/
#define  C_reset 13				/*  reset to default voice params	*/
#define  C_note 14				/*  note control	*/



#define kMaxWordSize 40
#define	kNormalPitch 323			/* 120 hz	*/


#define	kHZ_1	(326-kNormalPitch)
#define	kHZ_2	(329-kNormalPitch)
#define	kHZ_3	(332-kNormalPitch)
#define	kHZ_4	(335-kNormalPitch)
#define	kHZ_5	(338-kNormalPitch)
#define	kHZ_6	(341-kNormalPitch)
#define	kHZ_7	(344-kNormalPitch)
#define	kHZ_8	(347-kNormalPitch)
#define	kHZ_9	(350-kNormalPitch)
#define	kHZ_10	(352-kNormalPitch)
#define	kHZ_11	(355-kNormalPitch)
#define	kHZ_12	(358-kNormalPitch)
#define	kHZ_13	(361-kNormalPitch)
#define	kHZ_14	(364-kNormalPitch)
#define	kHZ_15	(366-kNormalPitch)
#define	kHZ_16	(369-kNormalPitch)
#define	kHZ_17	(372-kNormalPitch)
#define	kHZ_18	(374-kNormalPitch)
#define	kHZ_19	(377-kNormalPitch)
#define	kHZ_20	(380-kNormalPitch)
#define	kHZ_21	(382-kNormalPitch)
#define	kHZ_22	(385-kNormalPitch)
#define	kHZ_23	(388-kNormalPitch)
#define	kHZ_24	(390-kNormalPitch)
#define	kHZ_25	(393-kNormalPitch)
#define	kHZ_26	(395-kNormalPitch)
#define	kHZ_27	(398-kNormalPitch)
#define	kHZ_28	(400-kNormalPitch)
#define	kHZ_29	(403-kNormalPitch)
#define	kHZ_30	(405-kNormalPitch)
#define	kHZ_31	(408-kNormalPitch)
#define	kHZ_32	(410-kNormalPitch)
#define	kHZ_33	(413-kNormalPitch)
#define	kHZ_34	(415-kNormalPitch)
#define	kHZ_35	(417-kNormalPitch)
#define	kHZ_36	(420-kNormalPitch)
#define	kHZ_37	(422-kNormalPitch)
#define	kHZ_38	(424-kNormalPitch)
#define	kHZ_39	(427-kNormalPitch)
#define	kHZ_40	(429-kNormalPitch)
#define	kHZ_41	(431-kNormalPitch)
#define	kHZ_42	(434-kNormalPitch)
#define	kHZ_43	(436-kNormalPitch)
#define	kHZ_44	(438-kNormalPitch)
#define	kHZ_45	(440-kNormalPitch)
#define	kHZ_46	(443-kNormalPitch)
#define	kHZ_47	(445-kNormalPitch)
#define	kHZ_48	(447-kNormalPitch)
#define	kHZ_49	(449-kNormalPitch)
#define	kHZ_50	(451-kNormalPitch)
#define	kHZ_51	(454-kNormalPitch)
#define	kHZ_52	(456-kNormalPitch)
#define	kHZ_53	(458-kNormalPitch)
#define	kHZ_54	(460-kNormalPitch)
#define	kHZ_55	(462-kNormalPitch)
#define	kHZ_56	(464-kNormalPitch)
#define	kHZ_57	(466-kNormalPitch)
#define	kHZ_58	(468-kNormalPitch)
#define	kHZ_59	(471-kNormalPitch)
#define	kHZ_60	(473-kNormalPitch)


/* #define	kDownRampStep 15360		.078 hz / frame	*/

enum
	{	
	_IY_,	_IH_,	_EH_,	_AE_,	_AA_,	_AH_,	_AO_,	_UH_,	_AX_,	_ER_,
	_EY_,	_AY_,	_OY_,	_AW_,	_OW_,	_UW_,	_YU_,	_IR_,	_XR_,	_AR_,
	_OR_,	_UR_,	_IX_,	_SIL_,	_RX_,	_LX_,	_EL_,	_EN_,	_w_,	_y_,
	_r_,	_l_,	_h_,	_m_,	_n_,	_NG_,	_f_,	_v_,	_TH_,	_DH_,
	_s_,	_z_,	_SH_,	_ZH_,	_p_,	_b_,	_t_,	_d_,	_k_,	_g_,
	_CH_,	_JH_,	_TX_,	_DX_,	_QX_,	_DD_,
	_Stress1_,
	_Stress2_,
	_EmphStress_,
	_pRise_,
	_pFall_,
	_dInc_,
	_dDec_,
	_Syll_,
	_Word_,
	_Prep_,
	_Verb_,
	_Comma_,
	_Period_,
	_Quest_,
	_Exclam_,
	_Comp_,
	_Para_,
	_EmphWord_,
	_FuncWord_
	};
typedef short phons;


#define kNoEmphasis 		0
#define kEmphasizeWord		1
#define kDeemphasizeWord 	2

#define kNumOfPhons _Para_+1


/*------------------------------*/
/* Filter Bank Constants		*/
/*------------------------------*/

#define kFrameTime 5					/* ms per frame	*/
#define kSampFrameLen 112				/* 5.032 ms	*/
#define kSampBufGroup (1000/kFrameTime)	/* 100 ms	*/

#define kFrameTime_us 5033			/* us per frame (13A8.AE04)			*/
#define kGroupTime 100653			/* kFrameTime_us x kSampBufGroup	*/

#define kSampBufLen ((kSampBufGroup * kSampFrameLen)+kSampFrameLen)
#define kSampleBufferSize (kSampBufGroup * kSampFrameLen)
				
#define kNoMarker (-1)

#define kMIDI_50HZ 0x1F59			/* $1F.59 = 31.3498		*/
#define kOneTwelfth 0x1555			/* (1/12) * 65536		*/
#define kPointFive 0x8000			/* 0.5					*/

/*------------------*/
/* Vibrato			*/
/*------------------*/

#define kVibFreq1 134217 			/* 1.6 hz at 5ms sample frame (200hz)		*/
#define kVibFreq2 310378 			/* 3.7 hz at 5ms sample frame (200hz) 		*/

#define kMaxRamps 16


/****************************/
/* Phon_Ctrl_Buf flags		*/
/****************************/

/* XXXL FRMD SSSS BBBB kpCs ssoo ifrv tttt	*/
/* r = pitch rise					*/
/* f = pitch fall					*/
/* X = unused						*/
/* s = stress type					*/
/* o = vowel order					*/
/* i = word initial consonant		*/
/* p = plosive release				*/
/* t = syllable type				*/
/* B = boundry type					*/
/* k = compound noun				*/
/* v = low vibrato depth			*/
/* S = Silence type					*/
/* D = silence Duration				*/
/* M = sample Marker				*/
/* C = Content word					*/
/* R = pitch rise 1					*/
/* F = pitch fall 1					*/
/* L = syLlable start				*/


#define kSyllable_Start		0x10000000

#define kSilenceTypeField 	0x00F00000			/* mask	*/
#define kSilenceTypeShift	20					/* number of bits to align	*/

#define kSilenceDuration	0x01000000
#define kSampleMarker		0x02000000
#define kSingingDuration	0x40000000


#define kCompoundNoun 		0x8000

#define kBoundryTypeField 	0xF0000				/* mask	*/
#define kWord_Start 		0x10000
#define kPrep_Start 		0x20000
#define kVerb_Start 		0x40000
#define kTerm_Bound 		0x80000


#define kSyllableTypeField 	0x0F				/* mask	*/
#define kWord_End 			0x0001
#define kPrep_End 			0x0002
#define kVerb_End 			0x0004
#define kTerm_End 			0x0008

#define kSyllableOrderField				0x0300	/* mask	*/
#define kFirst_Syllable_In_Word			0x0100
#define kMid_Syllable_In_Word			0x0200
#define kLast_Syllable_In_Word			0x0300
#define kMore_Than_One_Syllable_In_Word	0x0300		/* either bit is set	*/
#define kOneOrNo_Syllable_InWord		0x0000		/* niether bits are set	*/

#define kStressField		0x1C00			/* mask	*/
#define kPrimaryStress		0x0400
#define kSecondaryStress	0x0800
#define kEmphaticStress		0x1000
#define kIsStressed			0x1C00
#define kPrimOrEmphStress	0x1400

#define kWord_Initial_Consonant 0x0080		/* up to 1st vowel in word	*/
#define kStressedWInitial kIsStressed+kWord_Initial_Consonant
#define kPlosive_Release	0x4000			/* added phon at end of nasal or plosive before _SIL_	*/

#define kContent_Word		0x2000			/* This is a Content word	*/

#define kPitchRise	0x0020
#define kPitchFall	0x0040
#define kPitchRise1	0x04000000
#define kPitchFall1	0x08000000

#define	kLowVibrato	0x10					/* low depth on sustained notes during singing	*/


/****************************/
/* Note Fields				*/
/****************************/
#define kNotePitch		0x00FF
#define kNoteDur		0x0F00
#define kNoteDurShift	8				/* 0x00 -> 000x	*/


/****************************/
/* Phon types				*/
/****************************/

#define kPhonemeType  0x0001
#define kControlType  0x0002
#define kStressType  0x0004
#define kBoundryType  0x0008
#define kWordBoundType  0x0010
#define kTerminatorType  0x0020
#define	kPDType 0x0040


/****************************/
/* PhonFlags2				*/
/****************************/

#define kVowelF			(1<<0)
#define kConsonantF		(1<<1)
#define kVoicedF		(1<<2)
#define kVowel1F		(1<<3)
#define kSonorantF		(1<<4)
#define kSonorant1F		(1<<5)
#define kNasalF			(1<<6)
#define kLiqGlideF		(1<<7)
#define kSonorConsonF	(1<<8)
#define kPlosiveF		(1<<9)
#define kPlosFricF		(1<<10)
#define kObstF			(1<<11)
#define kStopF			(1<<12)
#define kAlveolarF		(1<<13)
#define kVelar			(1<<14)
#define kLabialF		(1<<15)
#define kDentalF		(1<<16)
#define kPalatalF		(1<<17)
#define kYGlideStartF	(1<<18)
#define kYGlideEndF		(1<<19)
#define kGStopF			(1<<20)
#define kFrontF			(1<<21)
#define kDiphthongF		(1<<22)
#define kHasReleaseF 	(1<<23)
#define kAffricateF		(1<<24)
#define kLiqGlide2F		(1<<25)
#define kVocLiq			(1<<26)
#define	kFric			(1<<27)

#define kFlagMask1 (kLabialF+kDentalF+kPalatalF+kAlveolarF+kVelar+kGStopF)
#define kFlagMask2 (kAlveolarF-1)

/*------------------*/
/* Rank order		*/
/*------------------*/
#define kFrontR			0
#define kMiddleR		1
#define kBackR			2
#define kConsonantR		3
#define kRoundR			4



/*----------------------*/
/* ParsePhons return	*/
/*----------------------*/
#define kGotPhrase 0
#define kGotPhon 0
#define kPhonError 1
#define kBufEmpty 2


/*----------------------*/
/* Misc Constants		*/
/*----------------------*/
#define kStepSizeRes 3			/* 2^3 = xxxx xxxx xxxx x.xxx	*/

#define k1pct 655				/* 0.01 * 65536	*/
#define pct 655					/* Macro		*/
#define kOneHalf 0x8000			/* 0.5 in fixed	*/
#define k100percent 0x10000		/* 1.0 in fixed	*/

#define kPhonBufSize 512
#define kPhonBuf_Yellow_Zone kPhonBufSize-20
#define kPhonBuf_Red_Zone kPhonBufSize-10

#define kNormal_Speech_Rate 180		/* 180 wpm	*/


#define kMinRate 40

#define kFormantSynth	0
//#define kConcatSynth	1


#define	kDur_One	0x100		/* 1.00	*/
#define	kDurStepRes	8







#define	kPitchStress_Flg 	0x1
#define	kPitchRiseFall_Flg	0x2
#define	kPitchBoundry_Flg	0x4
#define	kResetDecline 		0x8
#define	kPhraseReset 		0x10
#define	kPitchRiseFall1_Flg	0x20



#define kSpeakNewPhon 1
#define kSpeakPhon 2
#define kSpeakLastFrame 3
#define kSpeakEnd 4
#define kSpeakDone 5
#define kNeverHappens (-10000)

struct CmdElem
{
	short			unused;
	short			type;
	long			data;
};
typedef struct CmdElem CmdElem;



/*----------------------------------*/
/* FRONT-END DEFINES				*/
/*----------------------------------*/

#define DICT_VERSION			0x00000001				/* current format version code for binary dictionary */
#define HASH_ENTRIES			('Z' - 'A' + 2) 		/* number of entries in the dictionary hash table */
#define BIG_WORD				32						/* longest word allowed (used by MakeDict) */

#define kNullCh					0
#define kEOFCh					0

#define kMaxTokenLen			32						/* longest token string 	*/
#define kEngToPPad				4						/* chars of padding to allocate for calls to EngToP()	*/
#define kTokenStrSize			(1 + kMaxTokenLen + kEngToPPad) /* add extra byte for length	*/
#define kTokenPhonemeSize 		(2 * kMaxTokenLen - 3)	/* keep the (-3) in here  (Trust me)	*/
#define kMaxWordLen				32						/* longest word allowed - longer strings will be broken	*/
#define kMagicOpcodeMapSize 	64						/* number of elements in the "Magic Opcode Map resource	*/
#define kLookAheadChars			2						/* number of chars we look ahead in input text	*/

#define defaultCmdBeginDelim	'[['					/* initial delimiter to begin an embedded command	*/
#define defaultCmdEndDelim		']]'					/* initial delimiter to end an embedded command	*/
#define CmdSeparator			';'						/* command separator within embedded commands	*/


/* Char attributes	*/

#define kWordSep		0x0001	/* set if char qualifies as a word separator (i.e. space char)	*/
#define kPeriod			0x0002	/* set if char qualifies as a period/decimal	*/
#define kComma	 		0x0004	/* set if char qualifies as a comma	*/
#define kApostrophe		0x0008	/* set if char qualifies as an apostrophe	*/
#define kPuncMark 		0x0010	/* set if char qualifies as punctuation	*/
#define kSymbol			0x0020	/* set if char qualifies as a symbol char	*/
#define kOther			0x0040	/* set if char doesn't fall in other categories	*/
#define kLetter 		0x0080	/* set if char is a-z or A-Z	*/
#define kDigit			0x0100	/* set if char is 0-9	*/
#define kDash			0x0200	/* set if char is '-'	*/
#define kCap			0x0400	/* set if char is A-Z	*/
#define kVow			0x0800	/* set if char is A, E, I, O, U, Y	*/




/*--------------------------*/
/* Token types				*/
/*--------------------------*/
#define kUnknownTok		(-2)		/* unknown token	*/
#define kEOFTok			(-1)		/* end-of-input token	*/
#define kNullTok		0		/* no token	*/
#define kAlphaTok		1		/* letters only	*/
#define kNumericTok		2		/* digits only	*/
#define kAlphaNumericTok 3		/* letters & digits	*/
#define kWordSepTok		4		/* word separator (i.e. white space)	*/
#define kPeriodTok		5		/* period token	*/
#define kDecimalTok		6		/* period which appears with numbers (subtle, no?)	*/
#define kCommaTok		7		/* comma token	*/
#define kApostropheTok	8		/* apostrophe token	*/
#define kPuncTok		9		/* punctuation mark	*/
#define kLiteralTok		10		/* literal token -- speak it letter by letter	*/
#define kSmartNumberTok	11		/* number token -- speak intelligently	*/
#define kRawPhonemeTok	12		/* raw phoneme input token	*/
#define kECommandTok	13		/* embedded command token	*/
#define kAcronTok		14

typedef short FETokenType;
typedef unsigned short TokenAttr;






/* Embedded Command Deferred Processing	*/


#define kNewInputMode			0x00000001	/* means there is a new input mode pending	*/
#define	kNewCharMode			0x00000002	/* means there is a new char-by-char vs normal mode pending	*/
#define	kNewDigitMode			0x00000004	/* means there is a new digit-by-digit vs normal mode pending	*/
#define	kNewSymbolMode			0x00000008	/* means there is a new literal symbol vs normal mode pending	*/
#define	kNewDelimiters			0x00000010	/* means there are new command delimiters pending	*/
#define	kNewEmphasis			0x00000020	/* means we should override the word prominence of the following token	*/
#define	kNewPitchBase			0x00000040	/* means we have a new baseline pitch pending	*/
#define	kNewPitchBaseRelative	0x00000080	/* means we have a new relative baseline pitch pending	*/
#define	kNewPitchMod			0x00000100	/* means we have a new pitch modulation pending	*/
#define	kNewPitchModRelative	0x00000200	/* means we have a new relative pitch modulation pending	*/
#define	kNewRate				0x00000400	/* means we have a new speaking rate pending	*/
#define	kNewRateRelative		0x00000800	/* means we have a new relative speaking rate pending	*/
#define	kNewVolume				0x00001000	/* means we have a new volume pending	*/
#define	kNewVolumeRelative		0x00002000	/* means we have a new volume pending	*/
#define	kNewSilence				0x00004000	/* means we have a new silence command pending	*/
#define	kNewVoice				0x00008000	/* means we have a new switch voice request pending	*/
#define	kNewSync				0x00010000	/* means we have a new sync callback request pending	*/
#define	kNewReset				0x00020000	/* means we have a new reset request pending	*/

#define	kNewNote				0x00040000	/* means we have a new note pending	*/
#define	kNewTempo				0x00080000	/* means we have a new note pending	*/
#define	kNewMarker				0x00100000	/* means we have a new note pending	*/



/*--------------------------*/
/* various speaking modes	*/
/*--------------------------*/
#define	kRawPhonemes		0x0001	/* interpret input text as raw phonemes & markup symbols	*/
#define	kCharByChar			0x0002	/* speak everything literally character-by-character	*/
#define	kDigitByDigit		0x0004	/* speak numbers digit-by-digit	*/
#define	kSymbols			0x0008	/* speak symbols literally (char-by-char)	*/
#define	kCommand			0x0010	/* subsequent tokens should be interpreted as embedded command tokens	*/
#define	kDisableCallBacks	0x0020	/* holds off all callbacks (set during TextToPhoneme conversions)	*/

typedef short SpeechMode;


/*--------------------------*/
/* Speaking states			*/
/*--------------------------*/
#define	kNormal 	0				/* normal unpaused, unstopped mode	*/
#define	kPaused		1				/* PauseSpeechAt has been called	*/
#define	kStopped	2				/* StopSpeech or StopSpeechAt has been called	*/

typedef short SpeechState;


/*--------------------------*/
/* addFlags					*/
/*--------------------------*/
#define	kAddDollar 		(1 << 0)
#define	kAddCent 		(1 << 1)
#define	kAddPercent 	(1 << 2)
#define	kClockSpecial 	(1 << 3)
#define	kMoreThanOne 	(1 << 4)
#define	kYearSpecial 	(1 << 5)
#define	kHasComma 		(1 << 6)



#define	kTokMax 50

//#pragma pack (push, 1)	// BYTE aligned
struct FEToken 
	{
	unsigned char	tokStr  [kTokenStrSize];		/* holds the chars in the token, plus padding for EngToP()	*/
	short			tokLen;							/* length of original token str (for Morph)	*/
	FETokenType		tokType;						/* token type code	*/
	TokenAttr		tokAttr;						/* composite attribute bits	*/
	unsigned char	phonStr [kTokenPhonemeSize];	/* equivalent phoneme opcode string (Pascal style)	*/
	unsigned char	phonHold [kTokenPhonemeSize];	/* equivalent phoneme opcode string (Pascal style)	*/
	SpeechMode		latchedMode;					/* global mode settings at time token was parsed	*/
	short			tokPos;							/* position in tokStr during letter-by-letter and number speaking	*/
	unsigned long	bufOffset;						/* byte offset from start of utterance to this token	*/
	short			tokEmphasis;					/* emphasis override to be applied to this token	*/
	short			inDict;							/* true if word found in dictionary	*/
	short			inMorph;						/* true if word morph decomposed	*/
	short			hasAlt;
	short			altChoice;
	short			POScode1[4];
	short			POScode2[4];
	short			POScount1;
	short			POScount2;
	short			POSchoice;
	short			hiRank;
	short			phraseEnd;
	unsigned long	compPOS1;
	unsigned long	compPOS2;
	short			firstCap;
	short			periodEnd;						/* word ended with period	*/
	short			isAbbriv;
	short			suffix;
	short			phrasingBND;
	short			add_BND;
	unsigned short	addFlags;
	};
//#pragma pack (pop)

typedef struct FEToken FEToken;
typedef FEToken *FETokenPtr;
typedef FETokenPtr *FETokenHandle;


/* Dictionary Data Structures	*/

#define dictLocked 0x00000001						/* dictionary locked down, internal ptrs are absolute (vs. relative)	*/

typedef unsigned char *IndexEntry;

#define kPOS_Slots 128


/*----------------------------------------------*/
/* In order to keep dict phon set below 64,		*/
/* Compound and Word are re-assigned			*/
/*----------------------------------------------*/
#define	kDictComp	_pRise_
#define	kDictWord	_pFall_
#define kPrimeStress 0x40			// OR's with phon


/*------------------*/
/* Dict delimiters	*/
/*------------------*/
#define kAltFlag 0xFF
#define kEndFlag 0x80				// OR's with POS

/*------------------*/
/* Dict types		*/
/*------------------*/
#define kCompressDict	2
#define kEncryptDict	1
#define kUserDict		0

/*------------------*/
/* For compression	*/
/*------------------*/
#define kDashCh	0
#define kPer	1
#define kApos	2


/*
 * DictDisk: on-disk format of a dictionary (big-endian 32-bit Mac binary).
 * All "pointer" fields are stored as 32-bit relative offsets in the file.
 * Used only in MakeDictPtrsAbsolute() for parsing.
 */
#pragma pack(push, 1)
struct DictDisk
	{
	uint32_t	nextDict_off;
	uint32_t	version;
	uint32_t	type;
	uint32_t	wordCount;
	uint32_t	hash[HASH_ENTRIES];
	short		POScodes[kPOS_Slots][4];
	uint32_t	words_off;
	uint32_t	index_off;
	uint32_t	flags;
	uint32_t	data[1];
	};
#pragma pack(pop)
typedef struct DictDisk DictDisk;

/*
 * Dict: in-memory dictionary representation.
 * Populated by MakeDictPtrsAbsolute(); pointer fields are real 64-bit pointers.
 */
struct Dict
	{
	struct Dict		*nextDict;
	uint32_t		version;
	uint32_t		type;
	uint32_t		wordCount;
	uint32_t		hash[HASH_ENTRIES];
	short			POScodes[kPOS_Slots][4];
	uint32_t		flags;
	unsigned char	*words;		/* absolute pointer into raw buffer	*/
	IndexEntry		*index;		/* allocated array of entry pointers	*/
	};

typedef struct Dict Dict, *DictPtr, **DictHandle;





/* Morph Suffix types	*/

#define kIZING_suffix	1	//*
#define kIZINGS_suffix	2	//*
#define kIZES_suffix	3	//*
#define kIZER_suffix	4	//*
#define kIZERS_suffix	5	//*
#define kIES_suffix		6
#define kIERS_suffix	7
#define kIER_suffix		8
#define kIED_suffix		9
#define kIEST_suffix	10
#define kERS_suffix		11
#define kER_suffix		12
#define kEST_suffix		13
#define kINGS_suffix	14
#define kING_suffix		15
#define kABLE_suffix	16
#define kBLY_suffix		17
#define kCALLY_suffix	18
#define kLY_suffix		19
#define kIMENTS_suffix	20
#define kIMENT_suffix	21
#define kMENTS_suffix	22
#define kMENT_suffix	23
#define kORS_suffix		24
#define kOR_suffix		25
#define kINESS_suffix	26
#define kINESSES_suffix	27
#define kNESS_suffix	28
#define kNESSES_suffix	29
#define kIZED_suffix	30
#define kIZE_suffix		31
#define kISMS_suffix	32
#define kISM_suffix		33
#define kED_suffix		34
#define kES_suffix		35
#define kS_suffix		36

#define kNo_suffix		(-1)





typedef struct voiceVar voiceVar;
typedef voiceVar *voiceVarPtr;


typedef	void	(*_i_Last_Snd_Buffer_Ptr) (voiceVarPtr vv);
typedef	void	(*_i_Cur_Sample_Buffer_Ptr) (voiceVarPtr vv, unsigned char	*sampleBuffer, long sampleLen);
typedef	void	(*_i_First_Sample_Buffer_Ptr) (voiceVarPtr vv, unsigned char	*sampleBuffer, long sampleLen);
typedef	void 	(*_i_EngineError_Ptr) ( voiceVarPtr vv, OSErr err, unsigned long bytePos );


typedef short	(*e_OpenSpeechChannel_Ptr) ( voiceVarPtr vv );
typedef short	(*e_SpeakBuffer_Ptr) ( voiceVarPtr vv, Ptr textBuf, long byteLen, long controlFlags ); 
typedef void	(*e_PauseSpeechAt_Ptr) (voiceVarPtr vv, unsigned long whereToPause); 
typedef void	(*e_StopSpeechAt_Ptr) (voiceVarPtr vv, unsigned long whereToPause); 
typedef void	(*e_ContinueSpeech_Ptr) ( voiceVarPtr vv ); 
typedef void	(*e_GetSpeechStatus_Ptr) (voiceVarPtr vv, SpeechStatusInfo *info);
typedef void	(*e_GetSpeechRate_Ptr) (voiceVarPtr vv, long *info);
typedef void	(*e_GetSpeechPitch_Ptr) (voiceVarPtr vv, long *info);
typedef void	(*e_GetSpeechVolume_Ptr) (voiceVarPtr vv, long *info);
typedef void	(*e_GetSpeechMod_Ptr) (voiceVarPtr vv, long *info);
typedef void	(*e_SetSpeechRate_Ptr) (voiceVarPtr vv, long info);
typedef void	(*e_SetSpeechPitch_Ptr) (voiceVarPtr vv, long info);
typedef void	(*e_SetSpeechVolume_Ptr) (voiceVarPtr vv, long info);
typedef void	(*e_SetSpeechMod_Ptr) (voiceVarPtr vv, long info);
typedef void	(*e_ResetParams_Ptr) (voiceVarPtr vv);
typedef short	(*e_UseVoice_Ptr) ( voiceVarPtr vv, void *tvPtr, unsigned char *sample ); 
typedef void	(*e_FillNextSampBuffer_Ptr) (voiceVarPtr vv);
typedef void	(*e_ReinitVoice_Ptr) (voiceVarPtr vv);
typedef void	(*e_SetTempo_Ptr) (voiceVarPtr vv, short tempo);


typedef short	(*e_TextToPhonemes_Ptr) ( voiceVarPtr vv, Ptr textBuf, long textBytes, Ptr phonemeBuf, long *phonBytes);
typedef void	(*e_GetSpeechErrors_Ptr) (voiceVarPtr vv, SpeechErrorInfo *info);
typedef void	(*e_GetInputMode_Ptr) (voiceVarPtr vv, unsigned long *info);
typedef void	(*e_GetCharacterMode_Ptr) (voiceVarPtr vv, unsigned long *info);
typedef void	(*e_GetNumberMode_Ptr) (voiceVarPtr vv, unsigned long *info);
typedef short	(*e_SetInputMode_Ptr) (voiceVarPtr vv, unsigned long *info);
typedef short	(*e_SetCharacterMode_Ptr) (voiceVarPtr vv, unsigned long *info);
typedef short	(*e_SetNumberMode_Ptr) (voiceVarPtr vv, unsigned long *info);
typedef void	(*e_SetCommandDelimiter_Ptr) (voiceVarPtr vv, DelimiterInfo *info);
typedef void 	(*e_Set_Word_CB_State_Ptr) ( voiceVarPtr vv, short state );

/* BE -> FE	*/
typedef void	(*e_InitFE_Ptr) (voiceVarPtr vv);
typedef FETokenPtr 	(*e_ParseNextWord_Ptr) ( voiceVarPtr vv );
typedef void 	(*e_AbortParse_Ptr) ( voiceVarPtr vv );
typedef short 	(*e_StartParse_Ptr) ( voiceVarPtr vv, Ptr theStr, unsigned long byteLen, unsigned long controlFlags );
typedef void	(*e_ResetFE_Ptr) (voiceVarPtr vv);

/* Synth -> BE	*/
typedef void	(*e_Fill_Next_Frame_Ptr) (voiceVarPtr vv);
typedef short 	(*e_HzToPitch_Ptr) (voiceVarPtr vv, short hz);
typedef short	(*e_MidiToPitch_Ptr) (short midiNote);
typedef short	(*e_LogToLin_Ptr) (voiceVarPtr vv, short logVal);
typedef short	(*e_GetPhon_Ptr) (voiceVarPtr vv, short index);
typedef long	(*e_GetPhonCtrl_Ptr) (voiceVarPtr vv, short index);

/*--------------------------------------*/
/* Synthesizer							*/
/*--------------------------------------*/
typedef void	(*synth_Init_Ptr) (voiceVarPtr vv);
typedef void	(*synth_Start_Talk_Ptr) (voiceVarPtr vv);
typedef void	(*synth_StartNewPhon_Ptr) (voiceVarPtr vv);
typedef void	(*synth_SpeakPhon_Ptr) (voiceVarPtr vv);
typedef void	(*synth_AdjustPhons1_Ptr) (voiceVarPtr vv);
typedef void	(*synth_AdjustPhons2_Ptr) (voiceVarPtr vv);
typedef	void	(*synth_FillNextSampBuffer_Ptr) (voiceVarPtr vv);
typedef void 	(*synth_SetVolume_Ptr) (voiceVarPtr vv, short vol);
typedef short	(*synth_NewVoice_Ptr) (voiceVarPtr vv, void *vd, unsigned char *sample);
typedef void	(*synth_ResetVoice_Ptr) (voiceVarPtr vv);

typedef void	(*fSynth_GetVoiceParams_Ptr) (voiceVarPtr vv, void *info );
typedef void	(*fSynth_SetVoiceParams_Ptr) (voiceVarPtr vv, void *info );
typedef void	(*fSynth_SetSoundSample_Ptr) (voiceVarPtr vv, unsigned char **info );
typedef void	(*fSynth_GetVoiceTables_Ptr) (voiceVarPtr vv, unsigned long **info );
typedef void	(*fSynth_SetVoiceTables_Ptr) (voiceVarPtr vv, unsigned long **info );
typedef void	(*fSynth_GetGlotGain_Ptr) (voiceVarPtr vv, short *info );
typedef void	(*fSynth_SetGlotGain_Ptr) (voiceVarPtr vv, short *info );
typedef void	(*fSynth_GetSampleGain_Ptr) (voiceVarPtr vv, short *info );
typedef void	(*fSynth_SetSampleGain_Ptr) (voiceVarPtr vv, short *info );
typedef void	(*fSynth_GetSamplePitch_Ptr) (voiceVarPtr vv, short *info );
typedef void	(*fSynth_SetSamplePitch_Ptr) (voiceVarPtr vv, short *info );
typedef void	(*fSynth_GetVoiceVars_Ptr) (voiceVarPtr vv, unsigned long *info );




struct moduleFunc
{
	/*----------------------------------	*/
	/* Function Pointers (Shell)	*/
	/*----------------------------------	*/
	_i_Last_Snd_Buffer_Ptr		_i_Last_Snd_Buffer_FUNC;
	_i_Cur_Sample_Buffer_Ptr	_i_Cur_Sample_Buffer_FUNC;
	_i_First_Sample_Buffer_Ptr	_i_First_Sample_Buffer_FUNC;
	_i_EngineError_Ptr			_i_EngineError_FUNC;

	/*----------------------------------	*/
	/* Function Pointers (BE)	*/
	/*----------------------------------	*/
	e_OpenSpeechChannel_Ptr		e_OpenSpeechChannel_FUNC;
	e_SpeakBuffer_Ptr			e_SpeakBuffer_FUNC;
	e_PauseSpeechAt_Ptr			e_PauseSpeechAt_FUNC;
	e_StopSpeechAt_Ptr			e_StopSpeechAt_FUNC;
	e_ContinueSpeech_Ptr		e_ContinueSpeech_FUNC;
	e_GetSpeechStatus_Ptr		e_GetSpeechStatus_FUNC;
	e_GetSpeechRate_Ptr			e_GetSpeechRate_FUNC;
	e_GetSpeechPitch_Ptr		e_GetSpeechPitch_FUNC;
	e_GetSpeechVolume_Ptr		e_GetSpeechVolume_FUNC;
	e_GetSpeechMod_Ptr			e_GetSpeechMod_FUNC;
	e_SetSpeechRate_Ptr			e_SetSpeechRate_FUNC;
	e_SetSpeechPitch_Ptr		e_SetSpeechPitch_FUNC;
	e_SetSpeechVolume_Ptr		e_SetSpeechVolume_FUNC;
	e_SetSpeechMod_Ptr			e_SetSpeechMod_FUNC;
	e_ResetParams_Ptr			e_ResetParams_FUNC;
	e_UseVoice_Ptr				e_UseVoice_FUNC;
	e_ReinitVoice_Ptr			e_ReinitVoice_FUNC;
	e_SetTempo_Ptr				e_SetTempo_FUNC;

	e_Fill_Next_Frame_Ptr		e_Fill_Next_Frame_FUNC;
	e_HzToPitch_Ptr				e_HzToPitch_FUNC;
	e_MidiToPitch_Ptr			e_MidiToPitch_FUNC;
	e_LogToLin_Ptr				e_LogToLin_FUNC;
	e_GetPhon_Ptr				e_GetPhon_FUNC;
	e_GetPhonCtrl_Ptr			e_GetPhonCtrl_FUNC;


	/*--------------------------------------*/
	/* Synthesizer							*/
	/*--------------------------------------*/
	synth_Init_Ptr					synth_Init_FUNC;
	synth_Start_Talk_Ptr			synth_Start_Talk_FUNC;
	synth_StartNewPhon_Ptr			synth_StartNewPhon_FUNC;
	synth_SpeakPhon_Ptr				synth_SpeakPhon_FUNC;
	synth_AdjustPhons1_Ptr			synth_AdjustPhons1_FUNC;
	synth_AdjustPhons2_Ptr			synth_AdjustPhons2_FUNC;
	synth_FillNextSampBuffer_Ptr	synth_FillNextSampBuffer_FUNC;
	synth_SetVolume_Ptr				synth_SetVolume_FUNC;
	synth_NewVoice_Ptr				synth_NewVoice_FUNC;
	synth_ResetVoice_Ptr			synth_ResetVoice_FUNC;

	fSynth_GetVoiceParams_Ptr		fSynth_GetVoiceParams_FUNC;
	fSynth_SetVoiceParams_Ptr		fSynth_SetVoiceParams_FUNC;
	fSynth_SetSoundSample_Ptr		fSynth_SetSoundSample_FUNC;
	fSynth_GetVoiceTables_Ptr		fSynth_GetVoiceTables_FUNC;
	fSynth_SetVoiceTables_Ptr		fSynth_SetVoiceTables_FUNC;
	fSynth_GetGlotGain_Ptr			fSynth_GetGlotGain_FUNC;
	fSynth_SetGlotGain_Ptr			fSynth_SetGlotGain_FUNC;
	fSynth_GetSampleGain_Ptr		fSynth_GetSampleGain_FUNC;
	fSynth_SetSampleGain_Ptr		fSynth_SetSampleGain_FUNC;
	fSynth_GetSamplePitch_Ptr		fSynth_GetSamplePitch_FUNC;
	fSynth_SetSamplePitch_Ptr		fSynth_SetSamplePitch_FUNC;
	fSynth_GetVoiceVars_Ptr			fSynth_GetVoiceVars_FUNC;

	/*--------------------------------------*/
	/* Function Pointers (FE)				*/
	/*--------------------------------------*/
	e_TextToPhonemes_Ptr		e_TextToPhonemes_FUNC;
	e_GetSpeechErrors_Ptr		e_GetSpeechErrors_FUNC;
	e_GetInputMode_Ptr			e_GetInputMode_FUNC;
	e_GetCharacterMode_Ptr		e_GetCharacterMode_FUNC;
	e_GetNumberMode_Ptr			e_GetNumberMode_FUNC;
	e_SetInputMode_Ptr			e_SetInputMode_FUNC;
	e_SetCharacterMode_Ptr		e_SetCharacterMode_FUNC;
	e_SetNumberMode_Ptr			e_SetNumberMode_FUNC;
	e_SetCommandDelimiter_Ptr	e_SetCommandDelimiter_FUNC;
	e_Set_Word_CB_State_Ptr		e_Set_Word_CB_State_FUNC;

	/* BE -> FE	*/
	e_InitFE_Ptr				e_InitFE_FUNC;
	e_ParseNextWord_Ptr			e_ParseNextWord_FUNC;
	e_AbortParse_Ptr			e_AbortParse_FUNC;
	e_StartParse_Ptr			e_StartParse_FUNC;
	e_ResetFE_Ptr				e_ResetFE_FUNC;
};
typedef struct moduleFunc moduleFunc;
typedef moduleFunc *moduleFuncPtr;




#define kAccentPitchPoints				7
#define kPerPhonePitchControls			6		// Limit for number of Apple text formatted phone pitches (4)
#define	kPitchBufSize					(kPhonBufSize * kPerPhonePitchControls)
#define kAbsolutePitchCeiling			240		// Some kind of modal or normal pitch range
#define kAbsolutePitchFloor				60		// Some kind of modal or normal pitch range 
#define	kLen1	5				// gt=6
#define	kLen2	6				// gt=7
#define	kLen3	7				// gt=8



struct voiceVar
{
	/*----------------------------------	*/
	/*	These are filled by the shell	*/
	/*----------------------------------	*/
	Ptr				shellV;				/* pointer to shell channel globals	*/
	moduleFuncPtr	funcList;			/* pointer to function ptr list	*/
	Ptr				Rules;				/* local copy of ptr to the pronunciation rules data	*/
	DictPtr			Dict;				/* local copy of ptr to the exception dictionary data	*/
	DictPtr			Symbols;			/* local copy of ptr to the symbol dictionary data	*/
	short			bit16_Sound;		/* true = 16 bit sound output	*/
	unsigned char	*waveBuffers;		/* sample buffers	*/
	FETokenPtr		tokBuffer;			/* FE token buffer	*/
	Ptr				reverbBufPtr;
	short			cpuIsFast;			// CPU speed > 68030
	
	/*----------------------------------*/
	/*	Back-end vars					*/
	/*----------------------------------*/
	short			*XlateToAllo;
	short			*phonTypeTbl;
	long			*phonFlags2;
	short			*maxDurTbl;
	short			*minDurTbl;
	short			*phonPitchTbl;
	short			*logToLinPtr;
	short			*logOf2Tbl;
	unsigned char	*SineWavePtr;			/* ptr to 'SineWave'	for modulation$$	*/
	unsigned short	*ExpOf2Tbl;
	unsigned short	*OctFreqTbl;

	unsigned short 	*Opcode_To_ASCII;
	unsigned char	*AndPhonStr;
	unsigned char	*PowerStr;
	unsigned char	*CommaPhonStr;			/* NOT USED */
	unsigned char	*ControlPhonStr;
	unsigned char	*NonASCIIPhonStr;		/* NOT USED */
	unsigned char	*SilencePhonStr;
	unsigned char	*OSTypeStr;
	unsigned short 	*divisorsPtr;
	unsigned char	*SuffixTab;
	short 			*SuffixType;
	unsigned char	*AppleSCII;
	unsigned char	*DollarPhonStr;
	unsigned char	*CentPhonStr;
	unsigned char	*ClockPhonStr;
	unsigned char	*OhPhonStr;
	short			*BoundryDurTbl;			/* Phrase dur	*/
	short			*Allo_to_Phon;			/* ptr to 'Allo_to_Phon'	*/
	short			*Phon_to_Phon;			/* ptr to 'Phon_to_Phon'	*/


	/*----------------------------------*/
	/*	Backend parse - 1st pass		*/
	/*----------------------------------*/
	short		phon_Buf_1[kPhonBufSize];			// phons
	long		phon_Ctrl_Buf_1[kPhonBufSize];		// control flags
	short		user_Pitch_Buf1[kPhonBufSize];		// pitch raise and fall embedded cmds
	short		user_Dur_Buf1[kPhonBufSize];		// duration increment and decrement embedded cmds
	short		user_Rate_Buf1[kPhonBufSize];		// speaking rate embedded cmds
	short		user_Cmd_Buf1[kPhonBufSize];		// all other embedded cmds
	short		user_Note_Buf1[kPhonBufSize];		// singing pitch

	short		phonBuf_1_In_Index;					// current index when writing INTO buffers - 1st pass
	
	/*----------------------------------*/
	/*	Backend parse - 2nd pass		*/
	/*----------------------------------*/
	short		phon_Buf_2[kPhonBufSize];			// phons
	long		phon_Ctrl_Buf_2[kPhonBufSize];		// control flags
	short		user_Pitch_Buf2[kPhonBufSize];		// pitch raise and fall embedded cmds
	short		user_Dur_Buf2[kPhonBufSize];		// duration increment and decrement embedded cmds
	short		user_Rate_Buf2[kPhonBufSize];		// speaking rate embedded cmds	
	short		user_Cmd_Buf2[kPhonBufSize];		// all other embedded cmds (index into 'CMDQueue')
	short		user_Note_Buf2[kPhonBufSize];		// singing pitch
	short		dur_Buf[kPhonBufSize];				// phon durations
	CmdElem		CMDQueue[kCQsize];					// holds the embedded commands
		
	short		phonBuf_1_Out_Index;				// 
	short		phonBuf_2_In_Index;

	/*----------------------------------*/
	/*	Pitch command buffers			*/
	/*----------------------------------*/
	short		pitch_Buf_Freq[kPhonBufSize];		// pitch target
	short		pitch_Buf_Time[kPhonBufSize];		// target time
	short		pitch_Buf_Flags[kPhonBufSize];		// control flags

	short		pitchBuf_In_Index;

	
	short		pitchCmdStep;						// size of pitch raise and fall step
	short		durCmdStep;							// size of duration increment and decrement step
	
	short		FEinputDone;						// set when FE has no more data (kEOFTok was parsed)
	
	short		end_Punctuation;					// punctuation for this parse
	
	short		start_of_Paragraph_Flag;
	short		is_Compound_Noun;
	
	short		speech_Rate;
	long		rate_Ratio;				/* fixed	*/
	long		rate_Ratio_LowGain;		/* fixed	*/

	short		cur_PhonBuf_Index_CF;
	short		dur_Done_in_Phon_CF;
	short		cur_Phon_CF;
	short		cur_Phon_Dur_CF;
	long		cur_PhonCtrl_CF;
	long		cur_PhonFlags_CF;
	short		next_Phon_CF;
	long		next_PhonCtrl_CF;
	long		next_PhonFlags_CF;
	short		prev_Phon_CF;
	long		prev_PhonFlags_CF;
	long		prev_PhonCtrl_CF;
	short		prev2_Phon_CF;			/* 2 phons back	*/
	short		prev2_PhonCtrl_CF;		/* 2 phons back	*/
	
	short		baselineFall_START;		/* sentence start	*/
	short		baselineFall_END;		/* target to fall to	*/

	short		baseline_Start_Level;	/* sentence start	*/
	short		baseline_End_Level;		/* target to fall to	*/
	
	short		pitch_Time_Offset;
	short		VP_riseAmt;
	short		VP_fallAmt;
	short		VP_riseAmt1;
	short		VP_fallAmt1;
	long		VP_stressGain;				/* fixed ratio	*/
	long		VP_assertiveness;			/* fixed ratio	*/
	short		VP_baselineFall;
	short		VP_quickness;
	long		VP_pitchRange;
	long		VP_intonation;
	short		VP_baselinePitch;
	short		VP_baselinePitch_Save1;
	short		VP_baselinePitch_Save2;
	short		last_baseline;
	short		voiceNaturalPitch;

	long		pFilter_Out1_Save1;
	long		pFilter_Out2_Save1;
	long		down_Ramp_Offset_Save1;
	short		fallRise_Offset_Save1;
	short		fallRise1_Offset_Save1;
	short		stress_Target_Save1;
	short		punct_Offset_Save1;

	long		pFilter_Out1_Save2;
	long		pFilter_Out2_Save2;
	long		down_Ramp_Offset_Save2;
	short		fallRise_Offset_Save2;
	short		fallRise1_Offset_Save2;
	short		stress_Target_Save2;
	short		punct_Offset_Save2;

	long		pFilter_Out1;
	long		pFilter_Out2;
	long		down_Ramp_Offset;
	long		down_Ramp_Step;
	short		fallRise_Offset;
	short		fallRise1_Offset;
	short		stress_Target;
	short		punct_Offset;
	
	short		basePitch_Offset;
	long		pFilter_In_Gain;
	long		pFilter_FB_Gain;
	short		uvPhon_Pitch_Targ;
	
	short		baseline_Start_Offset;
	short		baseline_End_Offset;
	short		pbHold, pbLowGain;
	
	short		next_PitchBuf_Time;
	short		phon_Index_Targ;
	short		phon_Index_CP;
	short		pitchBuf_Out_Index;
	short		time_IntoPhon_CP;
	short		cur_Phon_Dur_CC;
	short		cur_PhonDur_CP;
	short		cur_PitchBuf_Time;
	short		time_IntoPhon_Targ;
	
	short		next_PitchBuf_Time_Save1;
	short		phon_Index_Targ_Save1;
	short		phon_Index_CP_Save1;
	short		pitchBuf_Out_Index_Save1;
	short		time_IntoPhon_CP_Save1;
	short		cur_Phon_Dur_CC_Save1;
	short		cur_PhonDur_CP_Save1;
	short		cur_PitchBuf_Time_Save1;
	short		time_IntoPhon_Targ_Save1;

	short		next_PitchBuf_Time_Save2;
	short		phon_Index_Targ_Save2;
	short		phon_Index_CP_Save2;
	short		pitchBuf_Out_Index_Save2;
	short		time_IntoPhon_CP_Save2;
	short		cur_Phon_Dur_CC_Save2;
	short		cur_PhonDur_CP_Save2;
	short		cur_PitchBuf_Time_Save2;
	short		time_IntoPhon_Targ_Save2;

	short		low_Gain_CP;
	short		pitch_Boundry;
	short		pitch_Clause_StartTime;
	short		phon_Dur_Delay;
	short		lastWordStart;
	short		nLastWordStart;
	
	short		cur_PitchBuf_Pitch;
	short		cur_PitchBuf_Flags;
	short		phon_Pitch_Offset_1;
	short		phon_Pitch_Offset;
	short		stress_Active_Time;
	short		stress_Duration;
	short		stressDurTime;
	short		baseLine_Offset;


	
	short			controlF0;
	
	Fixed			vibrato_Phase1;
	Fixed			vibrato_Phase2;
		
	short			speakState;
	
	/* Component vars	*/
	
	short			Busy;					/* TRUE if speaking	*/
	short			outputPaused;			/* TRUE = voice is paused	*/
	unsigned long	embedded_Data;
	short			cmdBufCount;
	short			cmdBufCount_Save1;	
	short			cmdBufCount_Save2;	
	
	short			starting_New_Phon;
	short			ctrlCount;
	short			singing;
	short			hzGlide;			/* true while a note>37 Hz-glide phoneme is active */
	short			musicalNoteActive;	/* true once a note<=37 has been set; carries fwd to noteless phonemes */
	short			pendingSingPhoneme;
	unsigned int	pendingSingDuration;
	unsigned int	pendingSingNote;
	short			scanIndex;
	short			Note_Times[16];		/* note durations	*/

	
	long			time1;
	long			time2;
	long			total_T;
	long			count_T;
	


	/*----------------------------------	*/
	/*	Front-end vars	*/
	/*----------------------------------	*/
	Handle				opcodeBuf;				/* temp buffer used to hold opcodes while we speak	*/
	
	Ptr					StrBuf;					/* pointer to the active character buffer 	*/
	long				StrLen;					/* byte length of active character buffer 	*/
	Ptr					StrEOF;					/* pointer to first byte beyond the character buffer 	*/
	Ptr					StrPos;					/* pointer to next char to be processed by parser (one char beyond gNextCh) 	*/
	long				ControlFlags;			/* control bits received in SpeakBuffer calls	*/
	
	Byte 				Ch;						/* current char being processed	*/
	Byte				NextCh;					/* next char to be processed	*/
	Byte				PrevCh;					/* previous char processed		*/
	TokenAttr			ChAttr;					/* attributes of current char	*/
	TokenAttr			NextAttr;				/* attributes of next char		*/
	TokenAttr			PrevAttr;				/* attributes of previous char	*/
	short				wordIsAcronym;			/* "FileMaker", etc				*/

	short				AtCmdBegin;				/* true if g->Ch is start of begin-embedded-command delimiter	*/
	short				AtCmdEnd;				/* true if g->Ch is start of end-embedded-command delimiter	*/
	
	Byte				CmdBeginDelim [2]; 		/* embedded command delimiter characters	*/
	Byte				CmdEndDelim [2];

	unsigned long		PendingCommands;		/* bits designating embedded commands which need to be acted upon	*/
	SpeechMode			NewMode;				/* new input processing mode bits which are pending 	*/
	Byte				NewCmdBeginDelim [2]; 	/* new embedded command delimiter characters	*/
	Byte				NewCmdEndDelim [2];
	Fixed				NewPitchBase;			/* new baseline pitch	*/
	Fixed				NewPitchMod;			/* new pitch modulation depth	*/
	Fixed				NewRate;				/* new speaking rate	*/
	Fixed				NewVolume;				/* new speaking volume	*/
	Fixed				NewNote;				/* new singing note		*/
	Fixed				NewTempo;				/* new singing note		*/
	Fixed				NewMarker;				/* new singing note		*/
	unsigned long		NewSilence;				/* additional silence/delay to introduce	*/
	unsigned short		NewVoice;				/* new request to switch voices	*/
	unsigned long		NewSync;				/* new request for sync callback	*/
	short				NewEmphasis;			/* word prominence override value from 'emph' embedded-command	*/
	unsigned long		NewReset;				/* Reset selector from 'rset' embedded-command	*/
	
	SpeechMode			Mode;					/* current speaking mode	*/
	
	short				CurTok;					/* index to current token	*/
	short				LastTok;				/* index to end token		*/
	
	short				parseBusy;				/* set true by NewParse, goes false when EOF token is parsed	*/
	SpeechState			speechState;			/* initially kNormal, set to kPaused on PauseSpeechAt, set to kStopped on Stop, back to kNormal on ContinueSpeech	*/

	unsigned short		opCount;				/* number of opcodes remaining to be sent to back-end in current "word" token	*/
	FETokenPtr			opTok;					/* ptr to prior token we are sending to back-end	*/

	unsigned long		bufOffset;				/* count of chars processed in current utterance before current StrBuf	*/
	unsigned long		parseErrorPos;			/* ptr to start of token where error has been detected	*/
	short				parseErrorLogged;		/* true if we have already reported an error on the current embedded command	*/
	SpeechErrorInfo		Errors;					/* current unreported error status	*/
	
	short				*MagicCharMap;			/* used with MagicOpcodeMap to convert raw phoneme text into opcodes	*/
	short				*MagicOpcodeMap;		/* used with MagicCharMap to convert raw phoneme text into opcodes	*/
	TokenAttr			*CharAttr;				/* hold char attribute bits for ASCII charset	*/
	short				WordCB;					/* flag true to insert word markers	*/

	/* EngToP.c		*/
	short				e_direction;        	/* scan direction	*/
	char 				*rule_debug;			/* DEBUG	*/
	short				rule_count;				/* DEBUG	*/
	short				*hash;
	unsigned char		*rule;
	unsigned char		*kind;
	unsigned char		*dashruletab;
	unsigned char		*atruletab;
	unsigned char		*lruletab;
	unsigned char		*mruletab;
	unsigned char		*zruletab;
	unsigned char		*percentruletab;
	unsigned char		*bruletab;
	
	short				curSndFrame;
	
	short				tempo;					/* tempo for singing	*/
	short				markerIndex;
	short				lastMarkerIndex;
	long				markerBuf[kMaxMarkers];
	short				sync_On_Marker;
	long				frameMarker;
	short				add_a_boundry;
	short				boundry_to_add;
	
	short				voiceVers;
	
	short				lastRate;

	long				rampSteps[kMaxRamps];
	short				curRamp;
	short				newSentence;

	long				vibratoDepth1;
	long				vibratoDepth2;
	long				vibratoFreq;
	short				portamento;
	long				portamentoStep;
	long				portamentoAccum;
	short				newPortaTarget;
	short				user_Volume;				// Speech volume
	short				synthTech;					// kFormantSynth or kConcatSynth
	

//*******************************
// Sample OUTPUT buffers
//*******************************
	unsigned char	*sampleBuffer;		/* ptr to the current sample buffer	*/
	short			nextSampBuf;		/* Set to kBuf1 or kBuf2			*/
	unsigned char	*sampleBuffer1;		/* ptr to the sample buffer #1		*/
	unsigned char	*sampleBuffer2;		/* ptr to the sample buffer	#2  	*/
	long			waveIndex;			/* current sample buffer index'		*/

	void			*synthVars;
	void			*hWnd;				/* window handle (unused on Linux) */
};



#endif