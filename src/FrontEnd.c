/*
	File:		FrontEnd.c

	Contains:	Implementation of MacInTalk2 FrontEnd

	Written by:	Tim Schaaff

	Copyright:	� 1991-1992 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

		 <9>	 6/2/93		MC		Reset word index in NewParse
		 <8>	 12/8/92	TIM		Enable dictionaries to support both short-sized index offset
									ptrs as well as long-sized absolute index ptrs. When doing
									dictionary lookups of single chars, trap option chars.
		 <7>	11/10/92	TIM		Make SearchSingleDict be happy if passed a null dictionary
									pointer.
		 <6>	 10/8/92	TIM		Fixed odd-address bug in ProcessPendingCommands with embedded
									command delimiter changes.
		 <5>	 7/22/92	TIM		Make 'rset' embedded command get passed to Back-End.
		 <4>	 7/21/92	TIM		Fix bug in SearchSingleDict with tiny dictionaries.
		 <3>	 7/21/92	TIM		Fix odd-address access bug for Classic in StuffBECommand.
		 <2>	 7/17/92	JDR		make it a link patch
		 <1>	 12/12/91	TIM		first created

*/

#include <stdio.h> //debug

#ifndef __SPEECHSAY__
	#include "MT4.h"
#endif

#ifdef		DICT_TIMER
	#include <Timer.h>
	extern UnsignedWide		micro64;
	extern short			dictCounter;
	extern long				dictStart;
	extern long				dictEnd;
	extern long				dictTime;
	
	extern short			feCounter;
	extern long				feStart;
	extern long				feEnd;
	extern long				feTime;
#endif

/*-----------------	*/
/* EngToP.c	*/
/*-----------------	*/
extern void 	EngToP 	(voiceVarPtr vv, char *text, char *phonemes); 			/* letter to phoneme conversion routine	*/

/*-----------------	*/
/* EmbeddedCmd.c	*/
/*-----------------	*/
extern void		ProcessEmbeddedCommands ( voiceVarPtr vv ); 					/* parses embedded commands	*/
extern void		ChangeDelimiters ( voiceVarPtr vv, unsigned short newBegin, unsigned short newEnd );
extern void		ResetFE (voiceVarPtr vv);

/*-----------------	*/
/* Morph.c	*/
/*-----------------	*/
extern short	DoMorph (voiceVarPtr vv, FETokenPtr tok);
extern void		ResolvePOS (voiceVarPtr vv);
extern void		SetPOS_FromSuffix (FETokenPtr tok);
extern void		PlacePhrasing (voiceVarPtr vv);



//---------------------------
// Export
//---------------------------
void			Init_FEFuncPtrs (moduleFuncPtr mfp);

short	 		TextDoneCallBack		 ( voiceVarPtr vv );
void 			CheckForBeginCommand	 ( voiceVarPtr vv );
void 			CheckForEndCommand		 ( voiceVarPtr vv );
void 			GetNextCh 				 ( voiceVarPtr vv );
BECommand 		GetNextPhonemeOpcode 	 ( voiceVarPtr vv );
void 			CollectPhonemeToken 	 ( voiceVarPtr vv, FETokenPtr tok );
void 			CollectAlphaNumericToken ( voiceVarPtr vv, FETokenPtr tok );
void 			ProcessPendingCommands 	 ( voiceVarPtr vv, FETokenPtr tok );
void	 		GetNextPartialToken 	 ( voiceVarPtr vv, FETokenPtr tok );
void 			InitTokens 				 ( voiceVarPtr vv );
short	 		ConcatPhonStr			 ( unsigned char* dest, unsigned char* sou );
short	 		ConcatStr 				 ( unsigned char* dest, unsigned char* sou );
short	 		SearchSingleDict		 ( unsigned char* text, FETokenPtr tok, DictPtr dict, short saveIt ); /* routine to search for word in a dictionary	*/
short	 		SearchAllDicts		     ( voiceVarPtr vv, unsigned char* text, FETokenPtr tok, DictPtr mainDict, short saveIt ); /* routine to search for word in app dioctionaries and a main dictionary	*/
void 			WordToPhonemes			 ( voiceVarPtr vv, FETokenPtr t, DictPtr dict ); /* computes the appropriate phoneme string for this word	*/
void 			LiteralCharToPhonemes 	 ( voiceVarPtr vv, Byte c, FETokenPtr tok ); 	/* computes the literal phoneme string for specified character	*/
void 			AppendTwoDigitPhonemes	 ( voiceVarPtr vv, FETokenPtr t, Byte tens, Byte units);
void 			AppendThreeDigitPhonemes ( voiceVarPtr vv, FETokenPtr t, Byte hundreds, Byte tens, Byte units);
void 			PartialNumberToPhonemes	 ( voiceVarPtr vv, FETokenPtr t ); /* converts part of token into intelligent number phonemes	*/
void 			SpeakTokenCharByChar 	 ( FETokenPtr tok );
void 			SpeakTokenAsNumber		 ( FETokenPtr tok ); /* will force next token to be spoken intelligently as a number	*/
void	 		GetNextToken 			 ( voiceVarPtr vv );
void			LogError 				 ( voiceVarPtr vv, OSErr err);
void 			SetPOStoVal 			 ( FETokenPtr tok, short posVal);

short	e_TextToPhonemes ( voiceVarPtr vv, Ptr textBuf, long textBytes, Ptr phonemeBuf, long *phonBytes);
void	e_GetSpeechErrors (voiceVarPtr vv, SpeechErrorInfo *info);
void	e_GetInputMode (voiceVarPtr vv, unsigned long *info);
void	e_GetCharacterMode (voiceVarPtr vv, unsigned long *info);
void	e_GetNumberMode (voiceVarPtr vv, unsigned long *info);
short	e_SetInputMode (voiceVarPtr vv, unsigned long *info);
short	e_SetCharacterMode (voiceVarPtr vv, unsigned long *info);
short	e_SetNumberMode (voiceVarPtr vv, unsigned long *info);
void	e_SetCommandDelimiter (voiceVarPtr vv, DelimiterInfo *info);
void	e_InitFE (voiceVarPtr vv);
void 	e_AbortParse ( voiceVarPtr vv );
short 	e_StartParse ( voiceVarPtr vv, Ptr theStr, unsigned long byteLen, unsigned long controlFlags );
void 	e_Set_Word_CB_State ( voiceVarPtr vv, short state );
void	e_ResetFE (voiceVarPtr vv);
FETokenPtr e_ParseNextWord ( voiceVarPtr vv );


void	Init_FEFuncPtrs (moduleFuncPtr mfp)
{
	mfp->e_TextToPhonemes_FUNC 			= (e_TextToPhonemes_Ptr) &e_TextToPhonemes;
	mfp->e_GetSpeechErrors_FUNC 		= (e_GetSpeechErrors_Ptr) &e_GetSpeechErrors;
	mfp->e_GetInputMode_FUNC 			= (e_GetInputMode_Ptr) &e_GetInputMode;
	mfp->e_GetCharacterMode_FUNC 		= (e_GetCharacterMode_Ptr) &e_GetCharacterMode;
	mfp->e_GetNumberMode_FUNC 			= (e_GetNumberMode_Ptr) &e_GetNumberMode;
	mfp->e_SetInputMode_FUNC 			= (e_SetInputMode_Ptr) &e_SetInputMode;
	mfp->e_SetCharacterMode_FUNC 		= (e_SetCharacterMode_Ptr) &e_SetCharacterMode;
	mfp->e_SetNumberMode_FUNC 			= (e_SetNumberMode_Ptr) &e_SetNumberMode;
	mfp->e_SetCommandDelimiter_FUNC 	= (e_SetCommandDelimiter_Ptr) &e_SetCommandDelimiter;
	mfp->e_InitFE_FUNC 					= (e_InitFE_Ptr) &e_InitFE;
	mfp->e_ParseNextWord_FUNC 			= (e_ParseNextWord_Ptr) &e_ParseNextWord;
	mfp->e_AbortParse_FUNC 				= (e_AbortParse_Ptr) &e_AbortParse;
	mfp->e_StartParse_FUNC 				= (e_StartParse_Ptr) &e_StartParse;
	mfp->e_Set_Word_CB_State_FUNC 		= (e_Set_Word_CB_State_Ptr) &e_Set_Word_CB_State;
	mfp->e_ResetFE_FUNC			 		= (e_ResetFE_Ptr) &e_ResetFE;
}



void SetPOStoVal ( FETokenPtr tok, short posVal)
{
	tok->POScode1[0] = posVal;
	tok->compPOS1 = 1 << posVal;				/* fold POS into flag bit	*/
	tok->hiRank = posVal;
	tok->POScount1 = 1;
}



 void CheckForBeginCommand ( voiceVarPtr vv )
{
	if ((( vv->CmdBeginDelim[0]) && (vv->Ch	  == vv->CmdBeginDelim[0]))
	&& 	((!vv->CmdBeginDelim[1]) || (vv->NextCh == vv->CmdBeginDelim[1])))
		vv->AtCmdBegin = true, vv->AtCmdEnd = false;
	else
		vv->AtCmdBegin = false;
}

 void CheckForEndCommand ( voiceVarPtr vv )
{
	if (((( vv->CmdEndDelim[0]) && (vv->Ch		== vv->CmdEndDelim[0]))
	&& 	((!vv->CmdEndDelim[1]) || (vv->NextCh == vv->CmdEndDelim[1])))
	||	(vv->Ch == kEOFCh))							/* we've hit the end of the input	*/
		vv->AtCmdBegin = false, vv->AtCmdEnd = true;
	else
		vv->AtCmdEnd = false;
}






short	LookAheadCh ( voiceVarPtr vv )
{
	unsigned short	mNextCh;
	Ptr				mStrPos;

	mStrPos = vv->StrPos;
	do
		{
		if ( mStrPos >= vv->StrEOF)						/* input text has been completely processed	*/
			{
			mNextCh = kEOFCh;							/* just return the EOF marker end processing	*/
			break;
			}
		else											/* normal case -- just get the next char	*/
			{
			mNextCh = *mStrPos++;
			if (mNextCh > 0x7F)
				mNextCh = vv->AppleSCII[mNextCh - 0x80];			/* translate hi chars	*/
			}
		}
	while (mNextCh <= ' ');							/* Skip control chars and word separators	*/

	return (mNextCh);
}





void GetNextCh ( voiceVarPtr vv )
{
	vv->PrevCh 	  	= vv->Ch;						/* cur -> last, next -> cur	*/
	vv->PrevAttr	= vv->ChAttr;
	vv->Ch 	  		= vv->NextCh;					/* cur -> last, next -> cur	*/
	vv->ChAttr		= vv->NextAttr;

	do
		{
		if (vv->StrPos >= vv->StrEOF)					/* input text has been completely processed	*/
			{
			vv->NextCh = kEOFCh;					/* just return the EOF marker end processing	*/
			break;
			}
		else											/* normal case -- just get the next char	*/
			{
			vv->NextCh = *(vv->StrPos++);				/* pick up next char in buffer, increment string ptr	*/
	
			if (vv->NextCh > 0x7F)
				vv->NextCh = vv->AppleSCII[vv->NextCh - 0x80];			/* translate hi chars	*/
				
			//else if ( (vv->NextCh < ' ') && (vv->Ch != '.') )
			else if (vv->NextCh < ' ')
				vv->NextCh = ' ';								/* Convert cntl chars to spaces if NOT followed by period	*/
			}
		}
	while (vv->NextCh < ' ');							/* Skip control chars	*/

	vv->NextAttr = vv->CharAttr[vv->NextCh]; 		/* pick up attributes for new char	*/

	/* The following two tests (CheckForBegin� & CheckForEnd�) are performed here in order to	*/
	/* optimize things a little.  The routines will set a couple of flags which can be quickly	*/
	/* tested in the higher-level code.  Otherwise we would have to do a fairly nasty comparison	*/
	/* in many places in the higher-level code.	*/

	if (vv->Mode & kCommand)						/* processing embedded commands	*/
		CheckForEndCommand(vv);						/* see if we are about to end the command	*/
	else
		CheckForBeginCommand(vv);					/* see if we are about to hit an embedded command	*/

}





 BECommand GetNextPhonemeOpcode ( voiceVarPtr vv )
{
	unsigned short	index;
	short			opcode;
	short			phonPair, numOfChars;
	short			*tPhonTblPtr;


	opcode = (-1); 											/* initialize to impossible value	*/
	while ( (!vv->AtCmdBegin) 								/* not at the start of an embedded command	*/
			&& (opcode < 0) 								/* don't have an opcode yet	*/
			&& (vv->Ch != kEOFCh))							/* not at the end of the input text	*/
		{
		/*------------------------------------	*/
		/* Search MagicCharMap for Phon char	*/
		/*------------------------------------	*/
		phonPair = (vv->Ch << 8) + vv->NextCh;					/* 1111 1111 2222 2222	*/
		for (numOfChars = 1; numOfChars >= 0; --numOfChars) /* lets start search for 2 char phons	*/
			{
			tPhonTblPtr = vv->MagicCharMap;
			index = 0;										/* start at table top	*/
			for (;;)
				{
				opcode = tPhonTblPtr[index];
				if (opcode == phonPair)						/* we got a match!	*/
					{
					opcode = vv->MagicOpcodeMap[index];		/* get the actual phoneme opcode	*/
					break;
					}
				if ( opcode == (-1) )
					{
					break;									/* Reached end of table! Phon not defined!	*/
					}
				index++;
				}
				
			if (opcode >= 0)
				break;										/* we found a match	*/
			phonPair &= 0xFF00;								/* strip off the 2nd char for 1 char search	*/
			}

		if (numOfChars)
			GetNextCh(vv);									/* advance an extra time since it's a 2-char phoneme	*/
		}

	if ((opcode < 0) && (vv->Ch == kEOFCh))					/* have hit the end of the text input	*/
		{
		opcode = BE_EOF;									/* return something reasonable	*/
		}

	return opcode;
}





 void CollectPhonemeToken ( voiceVarPtr vv, FETokenPtr tok )
{
	BECommand	opcode = 0;
	Byte		tokLen = 0;

	while (true) 													/* loop until we have gotten a complete token	*/
		{
		opcode = GetNextPhonemeOpcode(vv);							/* parse another phoneme or inflection control	*/

		if ( ((opcode == _Word_) || (opcode == _EmphWord_)) 		/* we got a word prominence marker	*/	
			 && (tokLen) )											/* and it's not the first opcode in our token	*/
			{
			break;
			}
			
		else
			{
			if ( (opcode != BE_EOF) && !(vv->AtCmdBegin) )					/* and write the opcode into our token buffer	*/
				tok->phonStr[++tokLen] = (Byte) opcode;

			if ( (tokLen >= kTokenPhonemeSize)		/* no more room in this token	*/
				 ||	(vv->AtCmdBegin)						/* or at the start of an embedded command	*/
				 ||	(opcode == BE_EOF))					/* or we have hit the end of our input text	*/
				{
				break;
				}
			}
		GetNextCh(vv);										/* advance to start of next	phoneme	*/
		}

	if (tokLen)											/* got some phoneme opcodes	*/
		{
		tok->tokType 	= kRawPhonemeTok;				/* token will be raw phonemes	*/
		tok->phonStr[0] = tokLen;						/* set the length of the phoneme opcode list	*/
		if ( tok->POScount1 == 0 )						/* No POS extended embedded command	proceeded this	*/
			{
			SetPOStoVal (tok, kNoun);
			}

		tok->tokEmphasis = vv->NewEmphasis;
		vv->NewEmphasis = kNoEmphasis;
		}
		
	else if (opcode == BE_EOF) 						/* have hit the end of the input text	*/
		{										 		/* synthesize an EOF token	*/
		tok->tokType 	= kEOFTok;
		tok->tokStr[0] 	= 1;
		tok->tokStr[1] 	= kEOFCh;
		tok->tokAttr 	= vv->CharAttr[kEOFCh];
		}

}




 void CollectNumericToken ( voiceVarPtr vv, FETokenPtr tok )
{
	short		tokLen, i;
	

	tok->tokType = kNumericTok;
	tok->tokAttr = kDigit;
	tokLen = 0;

	tok->tokEmphasis = vv->NewEmphasis;
	vv->NewEmphasis = kNoEmphasis;
	
	while ((tokLen < kMaxTokenLen) 									/* room for more chars in token	*/
	&&	   (!vv->AtCmdBegin)										/* not at start of an embedded command	*/
	&&	   ( vv->ChAttr & (kLetter + kDigit + kComma + kDash)) ) 	/* char is letter, digit, comma or dash	*/
		{
		if (vv->Ch >= 'a' && vv->Ch <= 'z') 		/* lowercase letter	*/
			{
			vv->Ch -= ('a' - 'A');					/* convert to uppercase	*/
			vv->ChAttr = vv->CharAttr[vv->Ch];		/* and pick up the attributes of the uppercased letter	*/
			}

		if (vv->ChAttr & kDash)
			{
			if ( vv->NextAttr & (kLetter + kDigit) )
				{
				tok->tokStr[++tokLen] = 1;			/* this will become a silence phon	*/
				tok->tokType = kAlphaNumericTok;
				}
			else
				break;
			}

		if (vv->ChAttr & kComma)
			{
			if ( !(vv->NextAttr & kDigit) )
				break;
			else
				tok->addFlags |= kHasComma;
			}

		else if (vv->ChAttr & kLetter)
			{
			tok->tokStr[++tokLen] = vv->Ch;			/* save LETTER in token buffer	*/
			tok->tokType = kAlphaNumericTok;
			tok->tokAttr |= kLetter;
			}

		else if (vv->ChAttr & kDigit)
			{
			tok->tokStr[++tokLen] = vv->Ch;			/* save NUMBER in token buffer	*/
			}

		GetNextCh(vv);								/* step to next char		*/
		}
	
	if (tok->tokType == kNumericTok)
		{
		for (i = 1; i <= tokLen; i++)
			{
			
			if ( (tok->tokStr[i] > '0') && (i < tokLen) )
				{
				tok->addFlags |= kMoreThanOne;
				break;
				}
			else if ( (tok->tokStr[i] != '1') && (i == tokLen) )
				{
				tok->addFlags |= kMoreThanOne;
				break;
				}
			}
		}
		
	while (vv->Ch == ' ')							/* Filter out any spaces	*/
		GetNextCh(vv);								/* step to next char		*/		

	tok->tokStr[0]  = tokLen;						/* save token length		*/

}




 void CollectAlphaToken ( voiceVarPtr vv, FETokenPtr tok )
{
enum {lowMode, upMode};

	short		tokLen;
	short		firstCap, hasLow, hasVowel;
	Byte		next2Ch;
	TokenAttr	next2Attr;
	short		caseMode;
	short		maybeAcronym;
	

	tok->tokType = kAlphaTok;
	tok->tokAttr = kLetter;											/* composite attributes	*/
	tokLen = 0;

	tok->tokEmphasis = vv->NewEmphasis;
	vv->NewEmphasis = kNoEmphasis;

	hasLow = false;
	hasVowel = false;
	maybeAcronym = false;
	
	if (vv->Ch >= 'A' && vv->Ch <= 'Z')
		{
		firstCap = true; 											/* 1st letter is an uppercase letter	*/
		caseMode = upMode;
		}
	else
		{
		firstCap = false;
		caseMode = lowMode;
		}
	
	while ((tokLen < kMaxTokenLen) 									/* room for more chars in token	*/
	&&	   (!vv->AtCmdBegin)										/* not at start of an embedded command	*/
	&&	   ( vv->ChAttr & (kLetter + kDigit + kPeriod + kDash + kApostrophe)) ) 	/* char is letter, digit, comma or dash	*/
		{
		if (vv->Ch >= 'a' && vv->Ch <= 'z') 		/* lowercase letter	*/
			{
			vv->Ch -= ('a' - 'A');					/* convert to uppercase	*/
			vv->ChAttr = vv->CharAttr[vv->Ch];		/* and pick up the attributes of the uppercased letter	*/
			hasLow = true;
			caseMode = lowMode;
			}
		else if (vv->Ch >= 'A' && vv->Ch <= 'Z') 	/* uppercase letter	*/
			{
			if (caseMode != upMode)
				{
				if (vv->wordIsAcronym)
					goto STILL_ACRONYM;
				
				maybeAcronym = true;
				}
			}

		if (vv->ChAttr & kVow)
			hasVowel = true;						/* There's at least 1 vowel in this word	*/
			
		if (vv->ChAttr & kDash)
			{
			//if ( vv->NextAttr & kLetter )
			//	tok->tokStr[++tokLen] = vv->Ch;		/* save DASH in token buffer	*/
			//else
				break;
			}

		else if (vv->ChAttr & kPeriod)
			{
			next2Ch = LookAheadCh(vv);
			next2Attr = vv->CharAttr[ next2Ch ];
			//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			//if ( (vv->NextCh == kEOFCh) ||
			//	 ((vv->NextAttr & kWordSep ) && (((next2Attr & kLetter) && (next2Attr & kCap)) || !(next2Attr & kLetter))) )
			//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
			if ( (vv->NextCh == kEOFCh) || (vv->NextAttr & kWordSep ) )
				{
				if ( ((next2Attr & kLetter) && (next2Attr & kCap)) || !(next2Attr & kLetter) )
					{
					tok->tokStr[tokLen+1] = '.';					/* Add period for search	*/
					tok->tokStr[0]  = tokLen+1;
					if ( (firstCap) && (SearchAllDicts(vv, tok->tokStr, tok, vv->Dict, true)) )
						{
						if ((tok->isAbbriv)  && (vv->NextCh != kEOFCh) )
							tok->tokStr[++tokLen] = vv->Ch;		/* Mr. Smith...	*/
						else
							break;								/* end of sentence	*/
						}
					else
						break;									/* end of sentence	*/
					}
				else
					{
					tok->tokStr[tokLen+1] = '.';					/* Add period for search	*/
					tok->tokStr[0]  = tokLen+1;
					if (SearchAllDicts(vv, tok->tokStr, tok, vv->Dict, true) )
						{
						if ((tok->isAbbriv)  && (vv->NextCh != kEOFCh) )
							tok->tokStr[++tokLen] = vv->Ch;		/* Mr. Smith...	*/
						else
							break;								/* end of sentence	*/
						}
					else
						break;									/* end of sentence	*/
					}
				}
				
			else if ( (vv->NextCh == '.') && (next2Ch == '.') )
				{
				while (vv->NextCh == '.')							/* Filter out any periods	*/
					GetNextCh(vv);									/* step to next char		*/		
				break;												/* special "..."	*/
				}
				
			else
				tok->tokStr[++tokLen] = vv->Ch;		/* save PERIOD in token buffer as part of abbriviation	*/
			}

		else if (vv->ChAttr & kDigit)
			{
			tok->tokStr[++tokLen] = vv->Ch;			/* save NUMBER in token buffer	*/
			tok->tokType = kAlphaNumericTok;
			tok->tokAttr |= kDigit;
			}

		else if (vv->ChAttr & (kLetter + kApostrophe) )
			{
			tok->tokStr[++tokLen] = vv->Ch;			/* save LETTER in token buffer	*/
			}

		GetNextCh(vv);								/* step to next char		*/
		}

	vv->wordIsAcronym = false;					/* End-of-word boundry		*/
		
STILL_ACRONYM:
	if (maybeAcronym)
		vv->wordIsAcronym = true;
		
	while 	(vv->Ch == ' ')							/* Filter out any spaces	*/
		GetNextCh(vv);								/* step to next char		*/		

	if (tok->tokStr[tokLen] == '.')
		{
		//tokLen--;									/* omit trailing period for abbriviation	*/
		tok->periodEnd = true;
		}
	
	if (hasLow == false)
		{
		if ( (tokLen == 1) && ((tok->tokStr[1] == 'a') || (tok->tokStr[1] == 'i')) )
			{
			}
		else
			tok->tokType = kAcronTok;				/* maybe speak all caps word by letters	*/
		}
		
	if ( !(hasVowel) && (tok->tokType == kAlphaTok) )
		tok->tokType = kAcronTok;					/* No vowels in word maybe speak by letters	*/
		
	if ( (firstCap == true) && (hasLow == true) && (tok->tokType == kAlphaTok) )
		tok->firstCap = true;						/* it's a capitalized word	*/
		
	tok->tokStr[0]  = tokLen;						/* save token length		*/

}









static void StuffBECommand (unsigned char* buf, unsigned long cmd, unsigned long val)
{
	BECommandPtr cb;					/* BECommand buffer base ptr	*/

	cb = &( (BECommandPtr) &buf[1] )[buf[0]];	/* compute our write ptr (byte zero holds BECommand count)	*/

	cb[0] = BE_ECmd;					/* stuff opcode which denotes this is an embedded command	*/
	cb[1] = 6;							/* commands of this form are six words long (BE_ECmd, Length, unsigned long, 32-bit Value)	*/

	cb[2] = (uint16_t)((uint32_t)cmd >> 16);		/* stuff command code, high word	*/
	cb[3] = (uint16_t)((uint32_t)cmd & 0xFFFF);	/* stuff command code, low word		*/
	cb[4] = (uint16_t)((uint32_t)val >> 16);		/* stuff value, high word			*/
	cb[5] = (uint16_t)((uint32_t)val & 0xFFFF);	/* stuff value, low word			*/

	buf[0] += 6;						/* increment opcode count for the buffer by number of words we stuffed	*/
}






 void ProcessPendingCommands ( voiceVarPtr vv, FETokenPtr tok )
{

	while (vv->PendingCommands 							/* more deferred commands to handle	*/
	&& 	   (tok->tokType == kNullTok))					/* and we don't have any tokens to return	*/
		{
		/*------------------------------*/
		/* svox command					*/
		/* MUST BE PARSED BEFORE OTHERS	*/
		/*------------------------------*/
		if (vv->PendingCommands & kNewVoice) 		/* change speaking voice	*/
			{
			vv->PendingCommands &= ~kNewVoice;			/* clear the bit	*/

			tok->tokType = kECommandTok;

			StuffBECommand(tok->phonStr, EC_svox, (unsigned long) vv->NewVoice);
			}
			
		/*------------------------------*/
		/* InputMode command			*/
		/*------------------------------*/
		else if (vv->PendingCommands & kNewInputMode)	/* change input mode	*/
			{
			vv->PendingCommands &= ~kNewInputMode;		/* clear the bit	*/

			if (vv->NewMode & kRawPhonemes)
				{
				vv->NewMode &= ~kRawPhonemes;			/* turn off bit to keep debugging info readable	*/
				vv->Mode |= kRawPhonemes;				/* enable raw phoneme input mode	*/
				}
			else
				vv->Mode &= ~kRawPhonemes;				/* disable raw phoneme input mode	*/
			}
		else if (vv->PendingCommands & kNewCharMode) 	/* change character processing mode	*/
			{
			vv->PendingCommands &= ~kNewCharMode;		/* clear the bit	*/

			if (vv->NewMode & kCharByChar)
			{
				vv->NewMode &= ~kCharByChar;				/* turn off bit to keep debugging info readable	*/
				vv->Mode |= kCharByChar;					/* enable char-by-char mode	*/
			}
			else
				vv->Mode &= ~kCharByChar;				/* disable char-by-char mode	*/
		}
		else if (vv->PendingCommands & kNewDigitMode) 	/* change number processing mode	*/
		{
			vv->PendingCommands &= ~kNewDigitMode;		/* clear the bit	*/

			if (vv->NewMode & kDigitByDigit)
			{
				vv->NewMode &= ~kDigitByDigit;			/* turn off bit to keep debugging info readable	*/
				vv->Mode |= kDigitByDigit;				/* enable digit-by-digit mode	*/
			}
			else
				vv->Mode &= ~kDigitByDigit;				/* disable digit-by-digit mode	*/
		}
		else if (vv->PendingCommands & kNewSymbolMode) 	/* change symbol processing mode	*/
		{
			vv->PendingCommands &= ~kNewSymbolMode;		/* clear the bit	*/

			if (vv->NewMode & kSymbols)
			{
				vv->NewMode &= ~kSymbols;				/* turn off bit to keep debugging info readable	*/
				vv->Mode |= kSymbols;					/* enable literal symbol speaking mode	*/
			}
			else
				vv->Mode &= ~kSymbols;					/* disable literal symbol speaking mode	*/
		}
		else if (vv->PendingCommands & kNewDelimiters) 	/* change embedded command delimiters	*/
		{
			vv->PendingCommands &= ~kNewDelimiters;		/* clear the bit	*/

			vv->CmdBeginDelim[0] = vv->NewCmdBeginDelim[0]; /* copy the new delimiter values	*/
			vv->CmdBeginDelim[1] = vv->NewCmdBeginDelim[1]; /* into the channel globals	*/
			vv->CmdEndDelim[0] 	= vv->NewCmdEndDelim[0];
			vv->CmdEndDelim[1] 	= vv->NewCmdEndDelim[1];

			CheckForBeginCommand(vv);					/* new delimiters, see if we are looking at another embedded command	*/
		}
		else if (vv->PendingCommands & kNewSilence) 		/* insert a period of silence in the output speech	*/
		{
			vv->PendingCommands &= ~kNewSilence;			/* clear the bit	*/

			tok->tokType = kECommandTok;

			StuffBECommand(tok->phonStr, EC_slnc, (unsigned long) vv->NewSilence);
		}
		else if (vv->PendingCommands & kNewEmphasis) 	/* override word prominence of following token	*/
		{
			vv->PendingCommands &= ~kNewEmphasis;		/* clear the bit	*/
			//tok->tokEmphasis = vv->NewEmphasis;			/* and set token field which will override word prominence	*/
		}
		else if (vv->PendingCommands & kNewPitchBase) 	/* change baseline pitch	*/
		{
			vv->PendingCommands &= ~kNewPitchBase;		/* clear the bit	*/

			tok->tokType = kECommandTok;

			if (vv->NewPitchBase & (0xC000 << 16))		/* relative pitch change desired	*/
			{
				if (vv->NewPitchBase < 0)
					vv->NewPitchBase = -(vv->NewPitchBase & 0x3FFFFFFF);
				else
					vv->NewPitchBase &= 0x3FFFFFFF;

				StuffBECommand(tok->phonStr, EC_pbar, (unsigned long) vv->NewPitchBase);
			}
			else
				StuffBECommand(tok->phonStr, EC_pbas, (unsigned long) vv->NewPitchBase);
		}
		else if (vv->PendingCommands & kNewPitchMod) 	/* change pitch modulation depth	*/
		{
			vv->PendingCommands &= ~kNewPitchMod;		/* clear the bit	*/

			tok->tokType = kECommandTok;

			if (vv->NewPitchMod & (0xC000 << 16))		/* relative pitch change desired	*/
			{
				if (vv->NewPitchMod < 0)
					vv->NewPitchMod = -(vv->NewPitchMod & 0x3FFFFFFF);
				else
					vv->NewPitchMod &= 0x3FFFFFFF;

				StuffBECommand(tok->phonStr, EC_pmor, (unsigned long) vv->NewPitchMod);
			}
			else
				StuffBECommand(tok->phonStr, EC_pmod, (unsigned long) vv->NewPitchMod);
		}
		else if (vv->PendingCommands & kNewRate) 		/* change speaking rate	*/
		{
			vv->PendingCommands &= ~kNewRate;			/* clear the bit	*/

			tok->tokType = kECommandTok;

			if (vv->NewRate & (0xC000 << 16))			/* relative rate change desired	*/
			{
				if (vv->NewRate < 0)
					vv->NewRate = -(vv->NewRate & 0x3FFFFFFF);
				else
					vv->NewRate &= 0x3FFFFFFF;

				StuffBECommand(tok->phonStr, EC_ratr, (unsigned long) vv->NewRate);
			}
			else
				StuffBECommand(tok->phonStr, EC_rate, (unsigned long) vv->NewRate);
		}
		else if (vv->PendingCommands & kNewVolume) 		/* change speaking volume	*/
		{
			vv->PendingCommands &= ~kNewVolume;			/* clear the bit	*/

			tok->tokType = kECommandTok;

			if (vv->NewVolume & (0xC000 << 16))			/* relative volume change desired	*/
			{
				if (vv->NewVolume < 0)
					vv->NewVolume = -(vv->NewVolume & 0x3FFFFFFF);
				else
					vv->NewVolume &= 0x3FFFFFFF;

				StuffBECommand(tok->phonStr, EC_volr, (unsigned long) vv->NewVolume);
			}
			else
				StuffBECommand(tok->phonStr, EC_volm, (unsigned long) vv->NewVolume);
		}

		else if (vv->PendingCommands & kNewNote) 		/* issue NOTE command	*/
			{
			vv->PendingCommands &= ~kNewNote;			/* clear the bit	*/
			tok->tokType = kECommandTok;
			StuffBECommand(tok->phonStr, EC_note, (unsigned long) vv->NewNote);
			}

		else if (vv->PendingCommands & kNewTempo) 		/* issue TEMPO command	*/
			{
			vv->PendingCommands &= ~kNewTempo;			/* clear the bit	*/
			tok->tokType = kECommandTok;
			StuffBECommand(tok->phonStr, EC_tempo, (unsigned long) vv->NewTempo);
			}

		else if (vv->PendingCommands & kNewMarker) 		/* issue TEMPO command	*/
			{
			vv->PendingCommands &= ~kNewMarker;			/* clear the bit	*/
			tok->tokType = kECommandTok;
			StuffBECommand(tok->phonStr, EC_marker, (unsigned long) vv->NewMarker);
			}

		else if (vv->PendingCommands & kNewSync) 		/* send new sync callback request to backend	*/
		{
			vv->PendingCommands &= ~kNewSync;			/* clear the bit	*/

			tok->tokType = kECommandTok;

			StuffBECommand(tok->phonStr, EC_sync, (unsigned long) vv->NewSync);
		}
		else if (vv->PendingCommands & kNewReset) 		/* reset current back-end voice parameters	*/
		{
			vv->PendingCommands &= ~kNewReset;			/* clear the bit	*/

			tok->tokType = kECommandTok;

			StuffBECommand(tok->phonStr, EC_rset, (unsigned long) vv->NewReset);
		}
		else
			vv->PendingCommands = 0;						/* clear any other bits	*/
	}

}





 void GetNextPartialToken ( voiceVarPtr vv, FETokenPtr tok )
{
	FETokenType		type;
	short	    	multiSeps = 0;
	Ptr				StrPosSave;
	Byte 			ChSave;
	Byte			NextChSave;
	Byte			PrevChSave;
	TokenAttr		ChAttrSave;
	TokenAttr		NextAttrSave;
	TokenAttr		PrevAttrSave;



	while (tok->tokType == kNullTok)
		{
		while (vv->AtCmdBegin)								/* have hit the begin-embedded-command delimiter	*/
			{
SCAN_EMBEDDED_COMMANDS:										/* we will jump here from below if command delimiters are changed	*/

			vv->Mode |= kCommand;							/* enter embedded-command state	*/

			GetNextCh(vv);									/* skip over first delimiter char	*/

			if (vv->CmdBeginDelim[1])						/* command start delimiter is 2 chars long	*/
				GetNextCh(vv);								/* skip over second delimiter char	*/

			ProcessEmbeddedCommands(vv);						/* routine completely parses the embedded command	*/

			/* ProcessEmbeddedCommands clears the kCommand bit of vv->Mode.	*/

			/* At this point, vv->Ch will be pointing at the first	*/
			/* character following the End-Command delimiter	*/
			}

		/* We defer the activation of embedded commands until the entire embedded command sequence	*/
		/* has been processed.  This is done to ensure that mode changes do not affect the	*/
		/* embeddded command as it is being parsed.  For example, if the embedded command to	*/
		/* change command delimiters were processed inside the command itself, all hell would	*/
		/* break loose.  By defering the processing until the end of the command, we are able to	*/
		/* provide a more reliable framework for mdoe changes.	*/

		if (vv->PendingCommands)								/* got some embedded commands along the way	*/
			ProcessPendingCommands(vv, tok);					/* now it's time to make them happen	*/

		if (vv->AtCmdBegin)									/* we might have just changed to new delimiters	*/
			goto SCAN_EMBEDDED_COMMANDS;					/* in which case we should scan more commands	*/

		/* The next statement is extremely important for deskewing Mode changes from the	*/
		/* higher level token processing code.  Since the parsing scheme is pipelined, we	*/
		/* need to be sure that mode changes which occur after this token is scanned will not	*/
		/* affect how this token is processed.  We latch the current mode values with the token	*/
		/* to provide the desired decoupling.  Code which executes at a higher level than	*/
		/* GetNextPartialToken MUST NOT use or look at vv->Mode!	*/

		tok->latchedMode = vv->Mode;							/* sample current mode info for subsequent processing	*/

		/* Save byte offset from start of utterance to the start of this token.	*/
		/* Info is used to generate word callbacks.  Note that the current StrPos	*/
		/* is actually two characters ahead due to look-ahead scheme.	*/

		tok->bufOffset = vv->bufOffset + (vv->StrPos - vv->StrBuf);

		/* this will be closer to our word since we use a 2-char look-ahead	*/
		
		tok->bufOffset = (tok->bufOffset > kLookAheadChars) ? tok->bufOffset - kLookAheadChars : 0;

		if (tok->tokType == kNullTok)						/* means that ProcessPendingCommands did	*/
			{												/* not generate any commands for Back-End	*/
			if (tok->latchedMode & kRawPhonemes)			/* raw phoneme input mode	*/
				CollectPhonemeToken(vv, tok);

			else											/* normal text processing mode	*/
				{
				while ((vv->ChAttr   & kWordSep)
				&& 	   (vv->NextAttr & kWordSep))			/* cur and next chars are white space	*/
					{
					multiSeps++;							/* count up number of white space chars	*/
					GetNextCh(vv);							/* skip over multiple "spaces"	*/
					}

				if (vv->ChAttr & kLetter)
					{
					if (!vv->wordIsAcronym)
						{
						/*------------------------------------------------------*/
						/* Save input buffer state in case word is an acronym	*/
						/*------------------------------------------------------*/
						StrPosSave = vv->StrPos;
						ChSave = vv->Ch;
						NextChSave = vv->NextCh;
						PrevChSave = vv->PrevCh;
						ChAttrSave = vv->ChAttr;
						NextAttrSave = vv->NextAttr;
						PrevAttrSave = vv->PrevAttr;
						
						CollectAlphaToken(vv, tok); 			/* assume it's a sequence of letters	*/
						if ( (vv->wordIsAcronym) && !(SearchAllDicts(vv, tok->tokStr, tok, vv->Dict, true)) && !(DoMorph(vv, tok)) )
							{
							/*------------------------------------------------------*/
							/* Go back to begining of word, read only roots			*/
							/*------------------------------------------------------*/
							vv->StrPos = StrPosSave;
							vv->Ch = ChSave;
							vv->NextCh = NextChSave;
							vv->PrevCh = PrevChSave;
							vv->ChAttr = ChAttrSave;
							vv->NextAttr = NextAttrSave;
							vv->PrevAttr = PrevAttrSave;
							vv->AtCmdBegin = false;
							CollectAlphaToken(vv, tok);
							}
						else
							vv->wordIsAcronym = false;
						}
					else
						CollectAlphaToken(vv, tok); 			/* assume it's a sequence of letters	*/
					}

				else if (vv->ChAttr & kDigit)
					CollectNumericToken(vv, tok); 			/* assume it's a sequence of digits	*/

				else										/* miscellaneous single character tokens	*/
					{
						if (vv->ChAttr & kWordSep)						/* word separator (white space)	*/
							{
							type = kWordSepTok;
							}
							
						else if ( (vv->ChAttr & kPeriod) && (vv->ChAttr & kPuncMark) )			/* period or decimal	*/
							{
							type = kPeriodTok;
							if ( (vv->NextAttr & kWordSep) || (vv->NextCh == kEOFCh) )	/* not a number decimal point	*/
								tok->phraseEnd = true;
							}
							
						else if ( (vv->ChAttr & kComma) && (vv->ChAttr & kPuncMark) )			/* comma	*/
							{
							type = kCommaTok;
							tok->phraseEnd = true;
							}
							
						else if (vv->ChAttr & kApostrophe)					/* apostrophe or single quote	*/
							{
							type = kApostropheTok;
							}
							
						else if (vv->ChAttr & kPuncMark)					/* all other punctuation marks	*/
							{
							if (vv->Ch == ':')
								{
								if  ( (vv->PrevAttr & kDigit) && (vv->NextAttr & kDigit) )
									{
									tok->addFlags |= kClockSpecial;
									GetNextCh(vv);							/* skip ':'	*/
									CollectNumericToken(vv, tok);
									goto NOT_SINGLE;
									}
								}

							type = kPuncTok;
							tok->phraseEnd = true;
							}
							
						else if (vv->ChAttr & kOther)						/* some char we should speak literally	*/
							{
							if (vv->Ch == '$')
								{
								if (vv->NextAttr & kDigit)
									{
									tok->addFlags |= kAddDollar;
									GetNextCh(vv);							/* skip '$'	*/
									CollectNumericToken(vv, tok);
									goto NOT_SINGLE;
									}
								else
									type = kLiteralTok;
								}

							else if ( (vv->Ch == '"') || (vv->Ch == '(') || (vv->Ch == ')') )
								{
								vv->add_a_boundry = true;
								vv->boundry_to_add = kBND_Paren_L;			/* Add boundry to next token	*/
								type = kUnknownTok;
								}
							else
								{
								tok->tokPos = 0;							/* reset char index to speak from start of tokStr	*/
								type = kLiteralTok;
								}
							}
							
						else
							{
							if (vv->Ch == kEOFCh)			/* at end of input string	*/
								{
								type = kEOFTok;
								}
							else							/* some char we should speak literally	*/
								type = kUnknownTok;
							}

					if (type != kWordSepTok)
						{
						tok->tokStr[0] = 1;					/* assuming for now it's only 1 char in len	*/
						tok->tokStr[1] = vv->Ch;				/* save away char	*/
						}
					else
						{
						multiSeps++;						/* total number of separators we hit	*/

						if (multiSeps > kMaxTokenLen)		/* limit number of chars we write	*/
							multiSeps = kMaxTokenLen;		/* to token max	*/

						tok->tokStr[0] = (Byte) multiSeps; 	/* fill token string with word separator chars (spaces presumably)	*/

						while (multiSeps)
							tok->tokStr[multiSeps--] = vv->Ch;
						}

					tok->tokType = type;					/* and its corresponding token type	*/
					tok->tokAttr = vv->ChAttr;				/* latch the attributes, too	*/

					GetNextCh(vv);							/* and get a new char	*/
					}
				}
			}
		}
NOT_SINGLE:
	return;
}







 short ConcatPhonStr ( unsigned char *dest, unsigned char *sou )
{
	short 	  		i;
	unsigned char	*d; 				/* point at first byte beyond end	*/
	unsigned char	*s;					/* point at first char to copy	*/


	d = &dest[dest[0] + 1];
	s = &sou[1];
	i = dest[0] + sou[0];								/* length of output string after concat	*/

	if (i > (kTokenPhonemeSize < 255 ? kTokenPhonemeSize : 255))	/* about to overflow max phoneme string length or Pascal string length	*/
		return false;							/* cannot complete concatenation	*/

	i = sou[0];									/* 'i' now contains number of chars to copy	*/

	dest[0] += (Byte) i;						/* set new dest length	*/

	for (i = (short) (i - 1); i >= 0; --i)		/* and copy source chars to end of dest string	*/
		*d++ = *s++;

	return true;								/* concatenation was successful	*/
}





 short ConcatStr ( unsigned char *dest, unsigned char *sou )
{
	short 	  		i;
	unsigned char	*d; 				/* point at first byte beyond end	*/
	unsigned char	*s;					/* point at first char to copy	*/


	d = &dest[dest[0] + 1];
	s = &sou[1];
	i = dest[0] + sou[0];								/* length of output string after concat	*/

	if (i > (kMaxTokenLen < 255 ? kMaxTokenLen : 255))	/* about to overflow max token string length or Pascal string length	*/
		return false;							/* cannot complete concatenation	*/

	i = sou[0];									/* 'i' now contains number of chars to copy	*/

	dest[0] += (Byte) i;						/* set new dest length	*/

	for (i = (short) (i - 1); i >= 0; --i)		/* and copy source chars to end of dest string	*/
		*d++ = *s++;

	return true;								/* concatenation was successful	*/
}






 short SearchSingleDict (unsigned char* text, FETokenPtr tok, DictPtr dict, short saveIt)
{
	IndexEntry		*index;			/* ptr to the index ptrs	*/
	long			hi, lo;			/* index values into index ptr array <4> TIM Make signed to fix search bug	*/
	long			testKey;		/* index of dict entry to compare	 <4> TIM Make signed to fix search bug	*/
	unsigned char*	textTargPtr;	/* ptr to target text		*/
	unsigned char*	phonOutPtr;
	unsigned char*	dictPtr;		/* ptr to dictionary word	*/
	short			tLen, dLen;
	short			len;
	short			diff;
	short			i, POSindex, POSval;


	if (!dict)					/* passed a null dictionary ptr	*/
		return false;			/* be friendly	*/

	/* Routine implements a simple binary search algorithm.	*/
	/* In order to speed up search a bit a small hash table is used	*/
	/* to narrow search range based upon 1st letter of word we are searching for.	*/
	/* NOTE: Hash scheme is only used if first letter is between A-Z (uppercase).	*/

	if ((text[1] >= 'A') && (text[1] <= 'Z'))	/* use hash table to narrow search	*/
		{
		diff = (short) (text[1] - 'A');			/* compute index into hash table	*/
		lo = dict->hash[diff];					/* get index of first entry starting with this letter	*/
		hi = dict->hash[diff + 1] - 1;			/* get index of last  entry starting with this letter	*/
		}
	else if (text[1] < 'A')						/* search words which come before 'A' words	*/
		{
		lo = 0;									/* index of first word in dictionary	*/
		hi = dict->hash['A' - 'A'] - 1;			/* index of first word before the 'A' words	*/
		}
	else										/* search words which come after 'Z' words	*/
		{
		lo = dict->hash['Z' - 'A'];				/* index of first 'Z' entry	*/
		hi = dict->wordCount - 1;				/* index of last entry in dictionary	*/
		}

	index = dict->index;

	tLen = (short) text[0];						/* get length of target text	*/
	
	while (lo <= hi)
		{
		testKey = (hi + lo) >> 1;				/* this is the next dictionary entry to compare against	*/

		/* perform a character by character compare of the target text with the dictionary entry	*/
		dLen = ((short) *index[testKey]);		/* get length of dictionary word entry	*/
		dictPtr = &(index[testKey])[1];						/* set up ptr to 1st char of dictionary entry	*/
		
		textTargPtr = &text[1];								/* set up ptr to 1st char of target text	*/

		for (len = (short) (((tLen < dLen) ? tLen : dLen) - 1); len >= 0; --len)
			{
			if ((diff = (*textTargPtr++ - *dictPtr++)) != 0) 	/* compute difference between two chars	*/
					break;					 					/* quit compare and continue binary search	*/
			}

		if (diff == 0)							/* words are the same so far	*/
			{
			/* at this point, the two elements are equal if lengths are equal	*/
			/* otherwise shorter element < longer element	*/

			diff = (short) (tLen - dLen);
			}

		if (diff > 0)
			lo = testKey + 1;
		else if (diff < 0)
			hi = testKey - 1;
			
		else
			{
			 if (saveIt)
			 	{
				/*------------------------------------------------------------------*/
				/* Dict entry format:												*/
				/*																	*/
				/* len | Eng str | Phon str | POS index +  kEndFlag |				*/
				/* kAltFlag | phon str | Alt POS index +  kEndFlag |				*/
				/*------------------------------------------------------------------*/
				
				dictPtr = index[testKey] + dLen + 1; 	/* get ptr to phonemes for this dict entry	*/
				phonOutPtr = &tok->phonStr[1];			/* get ptr to phoneme output buffer	*/
	
				/*------------------------------------------------------------------*/
				/* dict entry normaly does not begin with a word prominence code	*/
				/* so prefix phoneme string with one								*/
				/*------------------------------------------------------------------*/
				*phonOutPtr++ = _Word_;					/* "pre-pend" word prominence opcode to the phoneme opcode list	*/
				len = 1;								/* pick up phoneme string length from dict entry	*/
				while ( !(*dictPtr & kEndFlag) )
					{
					len++;
					*phonOutPtr++ = *dictPtr++;			/* copy the dictionary phoneme string into the output buffer	*/
					}
				tok->phonStr[0] = len;					/* set length of phoneme string (including word prominence code)	*/
				
				tok->POScount1 = 0;
				tok->POScount2 = 0;
				tok->compPOS1 = 0;
				tok->compPOS2 = 0;
				tok->hiRank = 0;
				POSindex = (*dictPtr++) & ~kEndFlag;	/* Strip flag bit	*/
				for (i = 0; i < 4; i++)
					{
					POSval = dict->POScodes[POSindex][i];
					if (POSval != kUndefPOS)
						{
						if (POSval == kAbriv)
							tok->isAbbriv = true;
						tok->POScode1[i] = POSval & kPOSmask;				/* POS is 5 bits	*/
						tok->compPOS1 |= ( 1 << (tok->POScode1[i]) );		/* fold POS into composite flag bits	*/
						tok->POScount1++;
						if (tok->POScode1[i] > tok->hiRank)
							tok->hiRank = tok->POScode1[i];
						}
					else
						{
						tok->POScode1[i] = kUndefPOS;
						}
					}
	
				if ( *dictPtr == kAltFlag )					/* Alternate pronunciation?	*/
					{
					dictPtr++;								/* Skip over the Alt flag	*/
					phonOutPtr = &tok->phonHold[1];			/* get ptr to phoneme output buffer	*/
		
					/*------------------------------------------------------------------*/
					/* dict entry normaly does not begin with a word prominence code	*/
					/* so prefix phoneme string with one								*/
					/*------------------------------------------------------------------*/
					*phonOutPtr++ = _Word_;					/* "pre-pend" word prominence opcode to the phoneme opcode list	*/
					len = 1;								/* pick up phoneme string length from dict entry	*/
					while ( !(*dictPtr & kEndFlag) )
						{
						len++;
						*phonOutPtr++ = *dictPtr++;			/* copy the dictionary phoneme string into the output buffer	*/
						}
					tok->phonHold[0] = len;					/* set length of phoneme string (including word prominence code)	*/
					
					POSindex = (*dictPtr++) & ~kEndFlag;	/* Strip flag bit	*/
					for (i = 0; i < 4; i++)
						{
						POSval = dict->POScodes[POSindex][i];
						if (POSval != kUndefPOS)
							{
							if (POSval == kAbriv)
								tok->isAbbriv = true;
							tok->POScode2[i] = POSval & kPOSmask;				/* POS is 5 bits	*/
							tok->compPOS2 |= ( 1 << (tok->POScode2[i]) );		/* fold POS into composite flag bits	*/
							tok->POScount2++;
							if (tok->POScode2[i] > tok->hiRank)
								tok->hiRank = tok->POScode2[i];
							}
						else
							{
							tok->POScode2[i] = kUndefPOS;
							}
						}
					tok->hasAlt = true;
					}
				else
					tok->hasAlt = false;
				}
			return true;
			}
		}

	return false;
}



short	DecompressString (Ptr dest, unsigned char *src)
{
	short			strLen, bits, pass1;
	unsigned short	w1;
	short			compBytes;

	w1 = *src++;
	compBytes = 1;
	strLen = (w1 >> 3) + 1;
	w1 <<= 5;
	bits = 5;
	pass1 = true;

	for (;;)
		{
		
		if (bits <= 8)
			{
			if (!pass1)
				{
				*(short*)dest = w1 + 0x3E00;		// 'A' - 3
				if (*dest < 'A')
					{
					if (*dest == ('A' - 3))
						*dest = '-';
					else if (*dest == ('A' - 2))
						*dest = '.';
					else
						*dest = 0x27;
					}
				}
			else
				{
				*(short*)dest = w1;
				pass1 = false;
				}
			dest++;
			strLen--;
			if (strLen == 0)
				break;
			w1 &= 0x00FF;
			w1 <<= 5;
			bits += 5;
			}
		else
			{
			bits -= 8;
			w1 |= ((*src++) << bits);
			compBytes++;
			}
		}
	return (compBytes);
}





 short SearchSingleDict_C (voiceVarPtr vv, unsigned char* text, FETokenPtr tok, DictPtr dict, short saveIt)
{
	IndexEntry		*index;			/* ptr to the index ptrs	*/
	long			hi, lo;			/* index values into index ptr array <4> TIM Make signed to fix search bug	*/
	long			testKey;		/* index of dict entry to compare	 <4> TIM Make signed to fix search bug	*/
	unsigned char*	textTargPtr;	/* ptr to target text		*/
	unsigned char*	phonOutPtr;
	unsigned char*	dictPtr;		/* ptr to dictionary word	*/
	unsigned short	tLen, dLen;
	short			len;
	short			diff;
	short			i, POSindex, POSval;
	unsigned char	decompText[33];
	short			compBytes;
	unsigned char	phon;


	if (!dict)					/* passed a null dictionary ptr	*/
		return false;			/* be friendly	*/
		
	/* Routine implements a simple binary search algorithm.	*/
	/* In order to speed up search a bit a small hash table is used	*/
	/* to narrow search range based upon 1st letter of word we are searching for.	*/
	/* NOTE: Hash scheme is only used if first letter is between A-Z (uppercase).	*/

	if ((text[1] >= 'A') && (text[1] <= 'Z'))	/* use hash table to narrow search	*/
		{
		diff = (short) (text[1] - 'A');			/* compute index into hash table	*/
		lo = dict->hash[diff];					/* get index of first entry starting with this letter	*/
		hi = dict->hash[diff + 1] - 1;			/* get index of last  entry starting with this letter	*/
		}
	else if (text[1] < 'A')						/* search words which come before 'A' words	*/
		{
		lo = 0;									/* index of first word in dictionary	*/
		hi = dict->hash['A' - 'A'] - 1;			/* index of first word before the 'A' words	*/
		}
	else										/* search words which come after 'Z' words	*/
		{
		lo = dict->hash['Z' - 'A'];				/* index of first 'Z' entry	*/
		hi = dict->wordCount - 1;				/* index of last entry in dictionary	*/
		}

	index = dict->index;

	tLen = (short) text[0];						/* get length of target text	*/

	
	while (lo <= hi)
		{
		testKey = (hi + lo) >> 1;				/* this is the next dictionary entry to compare against	*/
		
		/*----------------------------------*/
		/* Decompress text from dict		*/
		/*----------------------------------*/
		compBytes = DecompressString ((Ptr) &decompText, (unsigned char*) index[testKey]);

		/*------------------------------------------------------------------------------------------*/
		/* perform a character by character compare of the target text with the dictionary entry	*/
		/*------------------------------------------------------------------------------------------*/
		dLen = (short) decompText[0];						/* get length of dictionary word entry	*/
		dictPtr = &decompText[1];							/* set up ptr to 1st char of dictionary entry	*/
		
		textTargPtr = &text[1];								/* set up ptr to 1st char of target text	*/

		for (len = (short) (((tLen < dLen) ? tLen : dLen) - 1); len >= 0; --len)
			{
			if ((diff = (*textTargPtr++ - *dictPtr++)) != 0) 	/* compute difference between two chars	*/
					break;					 					/* quit compare and continue binary search	*/
			}

		if (diff == 0)							/* words are the same so far	*/
			{
			/* at this point, the two elements are equal if lengths are equal	*/
			/* otherwise shorter element < longer element	*/

			diff = (short) (tLen - dLen);
			}

		if (diff > 0)
			lo = testKey + 1;
		else if (diff < 0)
			hi = testKey - 1;
			
		else
			{
			 if (saveIt)
			 	{
				/*------------------------------------------------------------------*/
				/* Dict entry format:												*/
				/*																	*/
				/* len | Eng str | Phon str | POS index +  kEndFlag |				*/
				/* kAltFlag | phon str | Alt POS index +  kEndFlag |				*/
				/*------------------------------------------------------------------*/
				dictPtr = index[testKey] + compBytes; 	/* get ptr to phonemes for this dict entry	*/
				phonOutPtr = &tok->phonStr[1];			/* get ptr to phoneme output buffer			*/
	
				/*------------------------------------------------------------------*/
				/* dict entry normaly does not begin with a word prominence code	*/
				/* so prefix phoneme string with one								*/
				/*------------------------------------------------------------------*/
				*phonOutPtr++ = _Word_;					/* "pre-pend" word prominence opcode to the phoneme opcode list	*/
				len = 1;								/* pick up phoneme string length from dict entry	*/
				while ( !(*dictPtr & kEndFlag) )
					{
					len++;
					phon = *dictPtr++;
					if (phon & kPrimeStress)
						{
						*phonOutPtr++ = _Stress1_;		/* Add primary stress	*/
						len++;
						phon &= ~kPrimeStress;			/* strip the flag bit	*/
						}
					else if (phon == kDictComp)
						phon = _Comp_;
					else if (phon == kDictWord)
						phon = _Word_;
					*phonOutPtr++ = phon;				/* copy the dictionary phoneme string into the output buffer	*/
					}
				tok->phonStr[0] = len;					/* set length of phoneme string (including word prominence code)	*/
				
				tok->POScount1 = 0;
				tok->POScount2 = 0;
				tok->compPOS1 = 0;
				tok->compPOS2 = 0;
				tok->hiRank = 0;
				POSindex = (*dictPtr++) & ~kEndFlag;	/* Strip flag bit	*/
				for (i = 0; i < 4; i++)
					{
					POSval = dict->POScodes[POSindex][i];
					if (POSval != kUndefPOS)
						{
						if (POSval == kAbriv)
							tok->isAbbriv = true;
						tok->POScode1[i] = POSval & kPOSmask;				/* POS is 5 bits	*/
						tok->compPOS1 |= ( 1 << (tok->POScode1[i]) );		/* fold POS into composite flag bits	*/
						tok->POScount1++;
						if (tok->POScode1[i] > tok->hiRank)
							tok->hiRank = tok->POScode1[i];
						}
					else
						{
						tok->POScode1[i] = kUndefPOS;
						}
					}
	
				if ( *dictPtr == kAltFlag )					/* Alternate pronunciation?	*/
					{
					dictPtr++;								/* Skip over the Alt flag	*/
					phonOutPtr = &tok->phonHold[1];			/* get ptr to phoneme output buffer	*/
		
					/*------------------------------------------------------------------*/
					/* dict entry normaly does not begin with a word prominence code	*/
					/* so prefix phoneme string with one								*/
					/*------------------------------------------------------------------*/
					*phonOutPtr++ = _Word_;					/* "pre-pend" word prominence opcode to the phoneme opcode list	*/
					len = 1;								/* pick up phoneme string length from dict entry	*/
					while ( !(*dictPtr & kEndFlag) )
						{
						len++;
						phon = *dictPtr++;
						if (phon & kPrimeStress)
							{
							*phonOutPtr++ = _Stress1_;		/* Add primary stress	*/
							len++;
							phon &= ~kPrimeStress;			/* strip the flag bit	*/
							}
						else if (phon == kDictComp)
							phon = _Comp_;
						else if (phon == kDictWord)
							phon = _Word_;
						*phonOutPtr++ = phon;				/* copy the dictionary phoneme string into the output buffer	*/
						}
					tok->phonHold[0] = len;					/* set length of phoneme string (including word prominence code)	*/
					
					POSindex = (*dictPtr++) & ~kEndFlag;	/* Strip flag bit	*/
					for (i = 0; i < 4; i++)
						{
						POSval = dict->POScodes[POSindex][i];
						if (POSval != kUndefPOS)
							{
							if (POSval == kAbriv)
								tok->isAbbriv = true;
							tok->POScode2[i] = POSval & kPOSmask;				/* POS is 5 bits	*/
							tok->compPOS2 |= ( 1 << (tok->POScode2[i]) );		/* fold POS into composite flag bits	*/
							tok->POScount2++;
							if (tok->POScode2[i] > tok->hiRank)
								tok->hiRank = tok->POScode2[i];
							}
						else
							{
							tok->POScode2[i] = kUndefPOS;
							}
						}
					tok->hasAlt = true;
					}
				else
					tok->hasAlt = false;
				}
			return true;
			}
		}

	return false;
}





short SearchAllDicts ( voiceVarPtr vv, unsigned char *text, FETokenPtr tok, DictPtr mainDict, short saveIt ) /* routine to search for word in app dioctionaries and a main dictionary	*/
{
	DictPtr		dict;
	short		match = false;

	if (!match)
		{
		if (mainDict->type == kCompressDict)
			{
			match = SearchSingleDict_C (vv, text, tok, mainDict, saveIt);	/* look for word in the specified compressed main dictionary	*/
			}
		else
			match = SearchSingleDict (text, tok, mainDict, saveIt);		/* look for word in the specified main dictionary	*/
		}

	return match;
}





 void WordToPhonemes ( voiceVarPtr vv, FETokenPtr t, DictPtr dict) /* computes the appropriate phoneme string for this token	*/
{
	Ptr		d;
	short	i;


	if (SearchAllDicts(vv, (t->tokStr), t, dict, true))
		{
		t->inDict = true;							/* word was in dictionary	*/
		}
		
	else if ( DoMorph(vv, t) )
		t->inMorph = true;							/* word was in morph'ed	*/
		
	else											/* use EngToP rule system to convert letters to phonemes	*/
		{
		/* NOTE: YOU MIGHT WANT TO ADD A CHECK HERE SOMEDAY TO LOOK FOR	*/
		/*		 CHARS THAT ENGTOP DOESN'T LIKE.  RIGHT NOW THE ROUTINE	*/
		/*		 ASSUMES IT WILL ONLY GET CALLED WITH WORDS COMPOSED OF	*/
		/*		 LETTERS, PERIODS, AND APOSTROPHES.	*/


		d = (Ptr)&t->tokStr[t->tokStr[0] + 1]; 		/* ptr to first byte beyond string	*/

		for (i = kEngToPPad - 1; i >= 0; --i)		/* pad string with spaces for EngToP	*/
			*d++ = ' ';

		t->tokStr[0] += kEngToPPad;					/* forget this and die!	*/

		EngToP (vv, (char*)t->tokStr, (char*)t->phonStr); 		/* letter to phoneme conversion routine	*/

		/* NOTE: ENGTOP ASSUMES THAT THE PHONEME OUTPUT BUFFER IS LARGE ENOUGH TO HOLD	*/
		/*		 WHATEVER IT GENERATES.  SO BE CAREFUL ABOUT WHAT YOU PASS TO IT�	*/

		t->tokStr[0] -= kEngToPPad;					/* remove padding from ASCII token string	*/
		
		SetPOStoVal (t, kNoun);
		SetPOS_FromSuffix (t);
		}

	//t->phonStr[0] = 4;
	//t->phonStr[1] = _Word_;
	//t->phonStr[2] = _k_;
	//t->phonStr[3] = _AE_;
	//t->phonStr[4] = _t_;

}




 void LiteralCharToPhonemes ( voiceVarPtr vv, Byte c, FETokenPtr tok ) /* computes the literal phoneme string for specified character	*/
{
	Byte	ch [2];


	tok->phonHold[0] = 0;
	ch[0] = 1;
	ch[1] = c;
	if (c < ' ')														/* it's a control character	*/
		{
		if (vv->Mode & kCharByChar)
			{
			(void) ConcatPhonStr(tok->phonHold, vv->ControlPhonStr);	/* prefix with the word "Control"	*/
			ch[1] += '@';												/* bring character into the range from '@' to '_'	*/
			(void) SearchAllDicts(vv, ch, tok, vv->Symbols, true); 		/* lookup character in symbols dictionary	*/
			(void) ConcatPhonStr(tok->phonHold, tok->phonStr);			/* append the letter	*/
			}
		else
			{
			tok->phonHold[0] = 0;
			(void) ConcatPhonStr(tok->phonHold, vv->SilencePhonStr);	/* substitute silence phon	*/
			}
		}
	else if (c > '~')													/* it's not ASCII (code > 127)	*/
		{
		tok->phonHold[0] = 0;
		(void) ConcatPhonStr(tok->phonHold, vv->SilencePhonStr);		/* substitute silence phon	*/
		}
	else																/* it's a printable character	*/
		{
		(void) SearchAllDicts(vv, ch, tok, vv->Symbols, true); 				/* lookup character in symbols dictionary	*/
		(void) ConcatPhonStr(tok->phonHold, tok->phonStr);
		}

	tok->phonStr[0] = 0;
	(void) ConcatPhonStr(tok->phonStr, tok->phonHold);

	SetPOStoVal (tok, kNoun);
}




 void AppendTwoDigitPhonemes ( voiceVarPtr vv, FETokenPtr tok, Byte tens, Byte units)
{
	Byte			numStr[4];

	if (tens == '0')											/* 00 <= number <= 09	*/
		{
		numStr[0] = 1;
		numStr[1] = units;

		(void) SearchAllDicts(vv, numStr, tok, vv->Symbols, true); 	/* lookup digits in symbols dictionary	*/
		(void) ConcatPhonStr(tok->phonHold, tok->phonStr);		/* append the phonemes to the token	*/
		}
	else if (tens == '1')										/* 10 <= number <= 19	*/
		{
		numStr[0] = 2;
		numStr[1] = tens;
		numStr[2] = units;

		(void) SearchAllDicts(vv, numStr, tok, vv->Symbols, true); 	/* lookup digits in symbols dictionary	*/
		(void) ConcatPhonStr(tok->phonHold, tok->phonStr);		/* append the phonemes to the token	*/
		}
	else														/* 20 <= number <= 99	*/
		{
		numStr[0] = 2;
		numStr[1] = tens;
		numStr[2] = '0';

		(void) SearchAllDicts(vv, numStr, tok, vv->Symbols, true); 	/* lookup digits in symbols dictionary	*/
		(void) ConcatPhonStr(tok->phonHold, tok->phonStr);		/* append the phonemes to the token	*/

		if (units != '0')
			AppendTwoDigitPhonemes(vv, tok, '0', units);			/* append the units value as well	*/
		}
}



 void AppendThreeDigitPhonemes ( voiceVarPtr vv, FETokenPtr tok, Byte hundreds, Byte tens, Byte units)
{

	if (hundreds > '0')
		{
		AppendTwoDigitPhonemes(vv, tok, '0', hundreds);			/* how many hundreds?	*/

		(void) SearchAllDicts(vv, (unsigned char*)"\p100", tok, vv->Symbols, true); 	/* lookup digits in symbols dictionary	*/
		(void) ConcatPhonStr(tok->phonHold, tok->phonStr);		/* append the phonemes to the token	*/
		}

	if (tens != '0' || units != '0')
		AppendTwoDigitPhonemes(vv, tok, tens, units);				/* append the tens and units now	*/
}






 void PartialNumberToPhonemes ( voiceVarPtr vv, FETokenPtr tok ) /* converts part of token into intelligent number phonemes	*/
{
	short 			digitsLeft;
	short 			power;
	Byte			hundreds, tens, units;
	short 			haveSpoken = (tok->phonStr[0] != 0);			/* true if prior calls for this token have resulted in phonemes to speak	*/


	vv->PowerStr[0] = 4;
	tok->phonStr[0] = 0;								/* reset phoneme string length	*/
	tok->phonHold[0] = 0;								/* reset phoneme string length	*/
	digitsLeft = tok->tokStr[0] - tok->tokPos;			/* number of digits still to be spoken	*/

	if (digitsLeft == 0)								/* hope we never get here in this case	*/
		return;											/* should be safe to just return	*/

	while ((tok->phonHold[0] == 0) && digitsLeft)
	{
		if (tok->addFlags & kYearSpecial)
			{
			tens  = tok->tokStr[++tok->tokPos];
			units = tok->tokStr[++tok->tokPos];

			if (tens != '0')
				{
				AppendTwoDigitPhonemes(vv, tok, tens, units);
				}
			else if ( (tens == '0') && (units != '0') )
				{
				(void) ConcatPhonStr(tok->phonHold, vv->OhPhonStr);
				AppendTwoDigitPhonemes(vv, tok, '0', units);
				}
			else
				{
				(void) SearchAllDicts(vv, (unsigned char*)"\p100", tok, vv->Symbols, true); 	/* lookup digits in symbols dictionary	*/
				(void) ConcatPhonStr(tok->phonHold, tok->phonStr);				/* append the phonemes to the token	*/
				}
			break;
			}

		if ( (tok->addFlags & kClockSpecial) && (tok->tokStr[0] == 2) )
			{
			tens  = tok->tokStr[++tok->tokPos];
			units = tok->tokStr[++tok->tokPos];
			
			if (tens != '0')
				{
				AppendTwoDigitPhonemes(vv, tok, tens, units);
				}
			else if ( (tens == '0') && (units != '0') )
				{
				(void) ConcatPhonStr(tok->phonHold, vv->OhPhonStr);
				AppendTwoDigitPhonemes(vv, tok, '0', units);
				}
			else
				{
				(void) ConcatPhonStr(tok->phonHold, vv->ClockPhonStr);
				}
			break;
			}
			
		switch (digitsLeft)
		{
			case 1:											/* 0 <= number <= 9	*/
				AppendTwoDigitPhonemes(vv, tok, '0', tok->tokStr[++tok->tokPos]);
				break;

			case 2:											/* 10 <= number <= 99	*/
				tens  = tok->tokStr[++tok->tokPos];
				units = tok->tokStr[++tok->tokPos];

				if (tens != '0' || units != '0' || !haveSpoken)
					{
					if  ((tok->tokStr[0] >= 3) && haveSpoken)	/* complete number has at least 3 digits	*/
						(void) ConcatPhonStr(tok->phonHold, vv->AndPhonStr);

					AppendTwoDigitPhonemes(vv, tok, tens, units);
					}
				break;

			case 3:											/* 100 <= number <= 999	*/
				AppendThreeDigitPhonemes(vv, tok, tok->tokStr[++tok->tokPos], '0', '0');
				break;

			default:										/* 1000 <= number	*/
				power = (short) (digitsLeft - 1) / 3;		/* power to which 1000 must be raised to get number	*/

				hundreds = '0';
				tens	 = '0';

				switch (digitsLeft % 3)
				{
					case 0:
						hundreds = tok->tokStr[++tok->tokPos];
					case 2:
						tens = tok->tokStr[++tok->tokPos];
					case 1:
						units = tok->tokStr[++tok->tokPos];
				}

				if (hundreds != '0' || tens != '0' || units != '0')
				{
					AppendThreeDigitPhonemes(vv, tok, hundreds, tens, units);

					if (power < 10)
						vv->PowerStr[0] = 3, vv->PowerStr[3] = power + '0';
					else
						vv->PowerStr[4] = power - 10 + '0';

					/*
						LIMITATION:  No check is made for the condition where the power is too big.  If
						the dictionary does not contain the string, we hang forever! We should fix this
						someday.  Right now it's not a problem, since our longest token string is shorter
						than the biggest number in the dictionary.  Someday we'll get bit by this, though.
					*/

					(void) SearchAllDicts(vv, vv->PowerStr, tok, vv->Symbols, true); 	/* find "big number" symbols in dictionary	*/
					(void) ConcatPhonStr(tok->phonHold, tok->phonStr);				/* append the phonemes to the token	*/
				}
				break;
		}

		digitsLeft = tok->tokStr[0] - tok->tokPos;		/* number of digits still to be spoken	*/
	}
	tok->phonStr[0] = 0;								/* Prepre to COPY...		*/
	(void) ConcatPhonStr(tok->phonStr, tok->phonHold);	/* ...final to 'phonStr' 	*/
	tok->inDict = true;					   				/* partial number was in dictionary	*/

	if ( (tok->addFlags & kAddDollar) && (digitsLeft == 0) )
		{
		(void) ConcatPhonStr(tok->phonStr, vv->DollarPhonStr);
		if ( !(tok->addFlags & kMoreThanOne) )
			tok->phonStr[0] -= 1;						/* make it singular	*/
		}
	else if ( (tok->addFlags & kAddCent) && (digitsLeft == 0) )
		{
		(void) ConcatPhonStr(tok->phonStr, vv->CentPhonStr);
		if ( !(tok->addFlags & kMoreThanOne) )
			tok->phonStr[0] -= 1;						/* make it singular	*/
		}

	tok->POScode1[0] = kAdj;
	tok->compPOS1 = kHas_Adj;
	tok->hiRank = kAdj;
	tok->POScount1 = 1;
}






 void InitToken ( voiceVarPtr vv, short tokIndex )
{
	short 		j;
	FETokenPtr	tok;


	tok = &vv->tokBuffer[tokIndex];

	tok->tokStr[0]  = 0;
	tok->tokType    = kNullTok;
	tok->tokAttr    = 0;
	tok->phonStr[0] = 0;
	tok->phonHold[0] = 0;

	tok->hasAlt    = false;
	for (j = 0; j < 4; j++)
		{
		tok->POScode1[j] = kUndefPOS;
		tok->POScode2[j] = kUndefPOS;
		}
	tok->POSchoice  = kUndefPOS;
	tok->phraseEnd = false;
	tok->compPOS1 = 0;
	tok->compPOS2 = 0;
	tok->POScount1 = 0;
	tok->POScount2 = 0;
	tok->isAbbriv = false;

	tok->firstCap = false;
	tok->periodEnd = false;
	tok->altChoice = 0;
	tok->hiRank = 0;
	tok->suffix = kNo_suffix;
	tok->tokEmphasis = 0;
	tok->addFlags = 0;

	if (vv->add_a_boundry)
		{
		vv->add_a_boundry = false;
		tok->add_BND = true;
		tok->phrasingBND = kBND_Paren_L;
		}
	else
		{
		tok->phrasingBND = kBND_None;
		tok->add_BND = false;
		}
}




#define Swap2Tokens(a, b, temp)			temp = a, a = b, b = temp
#define Rotate3Tokens(a, b, c, temp)	temp = a, a = b, b = c, c = temp

 void SpeakTokenCharByChar ( FETokenPtr tok ) /* will force token to be spoken character-by-character	*/
{
	tok->tokType = kLiteralTok;					/* set type to force this token to be spoken one char at a time	*/
	tok->tokPos = 0;							/* reset char index to speak from start of tokStr	*/
}

 void SpeakTokenAsNumber ( FETokenPtr tok ) /* will force next token to be spoken intelligently as a number	*/
{
	tok->tokType = kSmartNumberTok;				/* set type to force this token to be as a number	*/
	tok->tokPos = 0;							/* reset char index to speak from start of tokStr	*/
	if ( (tok->tokStr[0] == 4) && !(tok->addFlags & kAddDollar) && 
		 !(tok->addFlags & kAddCent) && !(tok->addFlags & kHasComma) &&
		 (tok->tokStr[1] == '1') )
		{
		if ( (tok->tokStr[2] != '0') || (tok->tokStr[3] != '0') || (tok->tokStr[4] != '0') )		/* Don't do 1000	*/
			tok->addFlags |= kYearSpecial;
		}
}






 void	GetNextToken ( voiceVarPtr vv )
{
	FETokenPtr			cur_Tok, prev_Tok, next_Tok;
	FETokenType			lastType;
	short 				i;


	
	cur_Tok = &vv->tokBuffer[vv->CurTok];
	prev_Tok = &vv->tokBuffer[vv->CurTok-1];
	next_Tok = &vv->tokBuffer[vv->CurTok+1];

	lastType = prev_Tok->tokType;							/* latch type of prior token	*/
	
	if (cur_Tok->latchedMode & kCharByChar)					/* token should be spoken letter by letter	*/
		{
		if ( (cur_Tok->tokType > kNullTok) &&				/* it's a "real" token (not kUnknownTok, kEOFTok, or kNullTok)	*/
			 (cur_Tok->tokStr[0]) )							/* and token string is not empty	*/
			{
			SpeakTokenCharByChar(cur_Tok);					/* force token to be spoken char-by-char	*/
			}
		}
		
	else switch (cur_Tok->tokType)							/* branch on token type	*/
		{
		case kNullTok:
			break;

		case kAlphaTok:										/* letters-only token	*/
			if (cur_Tok->tokStr[0] == 1)
				{
				if ( (prev_Tok->tokStr[0] > 1) || (next_Tok->tokStr[0] > 1) )
					WordToPhonemes(vv, cur_Tok, vv->Dict);	/* single char is NOT a symbol	*/
				else
					SpeakTokenCharByChar(cur_Tok);			/* speak single character in char-by-char mode	*/
				}
			else
				{
				WordToPhonemes(vv, cur_Tok, vv->Dict);		/* compute the appropriate phoneme string for current token	*/
				}
			break;

		case kAcronTok:										/* capital letters-only token	*/
			if ( SearchAllDicts(vv, cur_Tok->tokStr, cur_Tok, vv->Dict, true) )
				{
				cur_Tok->tokType = kAlphaTok;
				cur_Tok->inDict = true;						/* word was in dictionary	*/
				}
			else if ( DoMorph(vv, cur_Tok) )
				{
				cur_Tok->tokType = kAlphaTok;
				cur_Tok->inMorph = true;					/* word was in morph'ed	*/
				}
			else
				{
				cur_Tok->tokType = kAlphaNumericTok;
				SpeakTokenCharByChar(cur_Tok);				/* speak the token in char-by-char mode	*/
				}
			break;

		case kNumericTok:
			if ( (cur_Tok->latchedMode & kDigitByDigit) ||	/* we want to force digit by digit mode	*/
				 (lastType == kDecimalTok))					/* or prior token was a decimal point	*/
				{
				SpeakTokenCharByChar(cur_Tok);				/* speak token in digit-by-digit mode	*/
				}
			else											/* be as smart as you can be	*/
				{
				if (prev_Tok->addFlags & kAddCent)
					{
					cur_Tok->addFlags |= kAddCent;
					}
				SpeakTokenAsNumber(cur_Tok);				/* speak token as a number	*/
				}
			break;

		case kAlphaNumericTok:
			/*------------------------------------------------------------------------------*/
			/* just speak the sequence of letters & digits literally character by character	*/
			/*------------------------------------------------------------------------------*/
			SpeakTokenCharByChar(cur_Tok);					/* speak the token in char-by-char mode	*/
			break;

		case kWordSepTok:
			if (cur_Tok->tokStr[0] > 1)
				{
				cur_Tok->phonStr[0] = (Byte) (cur_Tok->tokStr[0] - 1);	/* for 2nd & successive word separators, introduce a little pause	*/
				for (i = 1; i <= cur_Tok->phonStr[0]; i++)
					cur_Tok->phonStr[i] = (Byte) _SIL_;
				}
			break;

		case kPeriodTok:
			if ( (next_Tok->tokType == kNumericTok)	||			/* if next token is a number	*/
				 (next_Tok->tokType == kAlphaNumericTok) ||		/* or an alphanumeric string	*/
				 
				 ( ((lastType == kNumericTok) || (lastType == kAlphaNumericTok) || (lastType == kSmartNumberTok)) &&	/* if last token was a number	*/
				   ((next_Tok->tokType == kNumericTok) || (next_Tok->tokType == kAlphaNumericTok))))					/* and next token is a number or an alphanumeric string	*/
				{
				/*------------------------------------------------------*/
				/* KLUDGE: turn the period token into the word "POINT"	*/
				/*------------------------------------------------------*/
				if (prev_Tok->addFlags & kAddDollar)
					{
					cur_Tok->tokStr[0] = 0;								/* erase prior string value	*/
					(void) ConcatStr(cur_Tok->tokStr, (unsigned char*)"\pANDD");		/* stuff with the word "AND"	*/
					cur_Tok->tokType = kNumericTok;
					cur_Tok->tokAttr = kLetter;
					cur_Tok->addFlags |= kAddCent;
					}
				else
					{
					cur_Tok->tokStr[0] = 0;								/* erase prior string value	*/
					(void) ConcatStr(cur_Tok->tokStr, (unsigned char*)"\pPOINT");		/* stuff with the word "POINT"	*/
					cur_Tok->tokType = kDecimalTok;
					cur_Tok->tokAttr = kLetter;
					}

				WordToPhonemes(vv, cur_Tok, vv->Dict); 		/* compute the appropriate phoneme string for the new token	*/
				}
			else												/* just handle as a sentence end delimiter	*/
				{
				cur_Tok->tokType = kPuncTok;
				cur_Tok->phonStr[0] = 2;
				cur_Tok->phonStr[1] = _Period_;
				cur_Tok->phonStr[2] = _SIL_;
				cur_Tok->phrasingBND = kBND_Decl;
				}
			break;

		case kDecimalTok:										/* should never get here since kDecimalTok is a meta-token	*/
			break;												/* synthesized by this routine	*/

		case kCommaTok:
			cur_Tok->tokType = kPuncTok;
			cur_Tok->phonStr[0] = 2;
			cur_Tok->phonStr[1] = _Comma_;
			cur_Tok->phonStr[2] = _SIL_;
			cur_Tok->phrasingBND = kBND_Pause;
			break;

		case kApostropheTok:
			break;

		case kPuncTok:
			cur_Tok->phonStr[0] = 2;
			switch (cur_Tok->tokStr[1])
				{
				case '!':
					cur_Tok->phonStr[1] = _Exclam_;
					cur_Tok->phonStr[2] = _SIL_;
					cur_Tok->phrasingBND = kBND_Emph;
					break;

				case '\?':
					cur_Tok->phonStr[1] = _Quest_;
					cur_Tok->phonStr[2] = _SIL_;
					cur_Tok->phrasingBND = kBND_Quest;
					break;

				case ':':									/* these punctuation are aliased to period	*/
				case ';':
				/* case '\xef\xbf\xbd': */ /* Mac Roman char, omitted for clang */
					cur_Tok->phonStr[1] = _Comma_;
					cur_Tok->phonStr[2] = _SIL_;
					cur_Tok->phrasingBND = kBND_Pause;
					break;

				case '-':									/* hyphen/dash/minus/endash/emdash	*/
					cur_Tok->tokType = kNullTok;
					cur_Tok->phonStr[0] = 0;				/* assume we will make the "dash" silent	*/
					break;

				default:
					cur_Tok->phonStr[0] = 1;
					cur_Tok->tokType = kWordSepTok;
					cur_Tok->phonStr[1] = (Byte) _SIL_;		/* map everything else to Silence	*/
					break;
				}
			break;

		case kLiteralTok:
			break;

		case kSmartNumberTok:									/* should never get here since kSmartNumberTok is a meta-token	*/
			break;												/* synthesized by this routine	*/

		case kRawPhonemeTok:									/* nothing to do with these types of tokens except	*/
			break;												/* to just pass them along to higher-level code	*/

		case kECommandTok:										/* nothing to do with these types of tokens except	*/
			break;												/* to just pass them along to higher-level code	*/

		default:												/* better be either kUnknownTok or kEOFTok	*/
			break;

	}	/* end of switch on cur_Tok->tokType	*/

	if ( (cur_Tok->tokType == kLiteralTok) ||				/* last token should be spoken letter by letter	*/
		 (cur_Tok->tokType == kDecimalTok) ||
		 (cur_Tok->tokType == kWordSepTok) )
		{
		SetPOStoVal (cur_Tok, kNoun);
		}

	else if (cur_Tok->tokType == kSmartNumberTok)		/* we are trying to speak a number intelligently	*/
		{
		SetPOStoVal (cur_Tok, kAdj);
		}
}





void	Fill_Tok_Buffer (voiceVarPtr vv)
{
	FETokenPtr		tok;


	InitToken (vv, 0);							/* initialize 1st token record to null	*/
	vv->CurTok = 1;								/* start at 2nd token					*/

	while ( (vv->Ch <= ' ') && (vv->Ch != kEOFCh) )
		GetNextCh (vv);
		
	
	/*----------------------------------*/
	/* 1st pass - fill partial tokens	*/
	/*----------------------------------*/
	vv->NewEmphasis = kNoEmphasis;
	for (;;)
		{
		tok = &vv->tokBuffer[vv->CurTok];
		InitToken (vv, vv->CurTok);				/* initialize cur token record to null state	*/
		GetNextPartialToken (vv, tok);			/* fill all the "partial" tokens	*/
		
		if ( (tok->tokType != kEOFTok) && (tok->phraseEnd) && (vv->CurTok == 1) )			/* no tokens in phrase	*/
			{
			}

		else if ( (tok->tokType == kEOFTok) || (tok->phraseEnd) || (vv->CurTok >= kTokMax-5) )
			{
			break;
			}
		else
			vv->CurTok++;							/* try to fill next token	*/
		}
		
	vv->LastTok = vv->CurTok;					/* remember were we ended	*/
	InitToken (vv, vv->LastTok+1);				/* initialize 1 past last token record to null	*/
	
	/*----------------------------------*/
	/* 2nd pass - fill full tokens	*/
	/*----------------------------------*/
	for (vv->CurTok = 1; vv->CurTok <= vv->LastTok; vv->CurTok++)
		{
		GetNextToken (vv);						/* finish filling all the tokens	*/
		}
	
	ResolvePOS (vv);
	PlacePhrasing (vv);
	
	vv->CurTok = 1;								/* start at 2nd token					*/
	vv->opCount = 0;							/* number opcodes in current "word" left to send to back-end	*/
	vv->opTok = &vv->tokBuffer[vv->CurTok];		/* point to 1st token	*/
}






short	NewParse ( voiceVarPtr vv, Ptr theStr, unsigned long byteLen, unsigned long controlFlags )
{



	/*------------------------------------------*/
	/* check for null string and/or zero length	*/
	/* assumes string is locked down			*/
	/*------------------------------------------*/

	vv->bufOffset = 0;							/* <9>	*/
	vv->StrBuf = theStr;						/* latch global ptrs to string	*/
	vv->StrPos = vv->StrBuf;					/* set ptr to first char	*/
	vv->StrLen = byteLen;						/* and save buffer length	*/
	vv->StrEOF = vv->StrBuf + vv->StrLen;		/* points just beyond last char in string	*/
	vv->ControlFlags = controlFlags;			/* latch control flag bits	*/
	vv->add_a_boundry = false;

	vv->NextCh = vv->Ch = kNullCh;				  		/* initialize to harmless values	*/
	vv->NextAttr = vv->ChAttr = vv->CharAttr[vv->Ch];	/* for following calls to GetNextCh()	*/

	vv->parseBusy = true;
	vv->speechState = kNormal;

	GetNextCh (vv);								/* prime the pumps (vv->Ch, vv->NextCh, and char attributes)	*/
	GetNextCh (vv);
	Fill_Tok_Buffer (vv);
	return noErr;
}





short ParseBusy ( voiceVarPtr vv )
{

	return (vv->parseBusy);						/* have not scanned kEOFTok token yet	*/
}





FETokenPtr e_ParseNextWord ( voiceVarPtr vv )
{
	FETokenPtr 	tok;
	FETokenPtr 	tok1;
	Byte		c;

	if (vv->CurTok > vv->LastTok)
		{
		Fill_Tok_Buffer (vv);
		}

	tok = &vv->tokBuffer[vv->CurTok];

	/*---------------------------------------------------------------------------------
		Both kLiteralTok and kSmartNumberTok token types have been created to solve
		a problem that arises due to the large expansion factor in converting
		text to phonemes.  In particular, when you are speaking a string in
		character-by-character mode or speaking a large number, the phoneme output
		buffer associated with a token is often much too small to contain the
		entire converted phoneme string.  Therefore when we are speaking tokens
		of this type, we iterate repeatedly on the same token, generating a smaller
		chunk of phonemes at a time. This is perhaps a little ugly, but it saves
		RAM and is a more efficient way of processing the text.
	----------------------------------------------------------------------------------*/

	if (tok->tokType == kLiteralTok)				/* last token should be spoken letter by letter	*/
		{
		while (tok->tokPos < tok->tokStr[0])		/* we have not spoken entire token yet	*/
			{
			if (tok->tokPos > 0)
				tok->add_BND = false;				/* otherwise every char will have one	*/
				
			c = tok->tokStr[++tok->tokPos];			/* get next character to speak	*/

			LiteralCharToPhonemes(vv, c, tok);		/* compute phoneme string for this char	*/
			tok->inDict = true;					   	/* character was in dictionary	*/
			
			if (tok->tokPos >= tok->tokStr[0])		/* after this, we will have spoken entire token		*/
				break;
				
			if (tok->phonStr[0])
				return tok;							/* return this token again with new phonemes	*/
			}
		}

	else if (tok->tokType == kSmartNumberTok)		/* we are trying to speak a number intelligently	*/
		{
		while (tok->tokPos < tok->tokStr[0])		/* we have not spoken entire prior token yet	*/
			{
			PartialNumberToPhonemes(vv, tok);		/* speak some more of the number	*/

			if (tok->tokPos >= tok->tokStr[0])		/* after this, we will have spoken entire token		*/
				break;
				
			if (tok->phonStr[0])
				return tok;							/* return this token again with new phonemes	*/
			}
		}

	tok1 = &vv->tokBuffer[++vv->CurTok];

	if (tok1->tokType == kEOFTok)					/* this is the end of the parse	*/
		{
		vv->parseBusy = false;						/* clear the global flag	*/
		}

	return tok;
}





/*	======================================
			Error Logging Handler
	====================================== */

void	LogError ( voiceVarPtr vv, OSErr err)	/* will update internal error info & do error callback if required	*/
{
	unsigned long position;

	if  (err != noErr)							/* we've got a real error to log	*/
		{
		position = vv->bufOffset + (vv->parseErrorPos - (unsigned long)vv->StrBuf);

		/* this will be closer to our word since we use a 2-char look-ahead	*/
		position = (position > kLookAheadChars) ? position - kLookAheadChars : 0;

		vv->Errors.count++;					/* log one more error	*/

		vv->Errors.newest = err;
		vv->Errors.newPos = position;

		if (vv->Errors.oldest == noErr)		/* have not latched our sticky error yet	*/
			{
			vv->Errors.oldest = err;
			vv->Errors.oldPos = position;
			}
		}

}






 short StuffText ( Byte **buf, unsigned long *roomLeft, unsigned char* text )
{
	short len = 0;

	if (text[0] > *roomLeft)
		return false;		/* not enough room for our text	*/

	*roomLeft -= text[0];

	while (len < text[0])
		*(*buf)++ = text[++len];

	return true;
}





 short StuffOSType ( voiceVarPtr vv, Byte **buf, unsigned long *roomLeft, unsigned long selector )
{
	short err = false;

	*(uint32_t*)(vv->OSTypeStr + 1) = (uint32_t)selector;

	if (!StuffText(buf, roomLeft, vv->OSTypeStr))
		return false;

	return true;
}






 void NumToDec ( short num, unsigned char *str )		/* 0 - 99	*/
{

	str[0] = 1;
	
	if (num > 9)
		{
		str[1] = '0';
		do
			{
			str[1]++;
			num -= 10;
			}
		while (num > 9);
		str[0]++;
		}
	str[str[0]] = '0' + num;
}


 void NumToHex ( unsigned long num, unsigned char* str ) /* converts numbers to 0xABCD format	*/
{
	short 			i;
	short 			digits;
	unsigned short 	nibble;

	str[0] = 2;
	str[1] = '0';
	str[2] = 'x';

	if (num & 0xFFFF0000)
		digits = 8;
	else
		digits = 4, num <<= 16;								/* make sure we see the digits	*/

	for (i = digits - 1; i >= 0; --i)
	{
		nibble = (num >> 28) & 0x000F;
		num <<= 4;

		str[0]++;

		if (nibble < 10)
			str[str[0]] = '0' + nibble;
		else
			str[str[0]] = 'A' + nibble - 10;
	}
}

 short StuffHexNum ( Byte **buf, unsigned long *roomLeft, unsigned long num )
{
	unsigned char	hexStr[32];
	short err = false;

	NumToHex(num, hexStr);

	if (!StuffText(buf, roomLeft, hexStr))
		return false;

	if (!StuffText(buf, roomLeft, (unsigned char*)"\p "))
		{
		return false;
		}

	return true;
}
 short Stuff1HexNum ( Byte **buf, unsigned long *roomLeft, unsigned long num )
{
	unsigned char	hexStr[32];
	short err = false;

	if (!StuffText(buf, roomLeft, (unsigned char*)"\p<<cmnt "))
		return false;

	NumToHex(num, hexStr);

	if (!StuffText(buf, roomLeft, hexStr))
		return false;

	if (!StuffText(buf, roomLeft, (unsigned char*)"\p "))
		return false;

	if (!StuffText(buf, roomLeft, (unsigned char*)"\p>> "))
		return false;

	return true;
}






 short	e_TextToPhonemes ( voiceVarPtr vv, Ptr textBuf, long textBytes, Ptr phonemeBuf, long *phonBytes)
{
	unsigned char		*tokStr;
	unsigned long		embedType;
	long				embedData;
	unsigned long		ASCII_phon;
	short				err;
	unsigned long		roomLeft;
	Byte				*phonPtr;
	short				relativeCmd;
	short				cur_Phon,cur_Phon1, orig_Phon;
	//unsigned char		decNum[4];

	

	err = kNoError;

#if 0
	vv->Mode |= kDisableCallBacks;						/* turn off callbacks during the conversion	*/
	phonPtr = (Byte*)phonemeBuf;					/* initialize ptr into output buffer	*/
	roomLeft = (unsigned long) *phonBytes;			/* see how many chars we can write to output buffer	*/
	err = NewParse (vv, textBuf, textBytes, 0);			/* get ready to start parsing this text	*/

	while (!err) 														/* Parse while no errors	*/
		{
		vv->opTok = (*(vv->funcList->e_ParseNextWord_FUNC)) (vv);		/* scan the next "word" from the input text	*/
		
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
			break;														/* We're done parsing	*/
			}

		#if 0
		/*------------------------------*/
		/* Add minor phrase boundry		*/
		/*------------------------------*/
		if (vv->opTok->add_BND)
			{
			if ( vv->opTok->phrasingBND >= kBND_Sep1 )
				{
				if ( vv->opTok->phrasingBND != kBND_Sep6 )
					{
					if (!StuffText(&phonPtr, &roomLeft, "\p%"))			/* Insert SILENCE	*/
						err = bufTooSmall;
					}
				if (!StuffText(&phonPtr, &roomLeft, "\p @"))			/* special BOUNDRY	*/
					err = bufTooSmall;

				if (roomLeft)
					{
					*phonPtr++ = (Byte) ((vv->opTok->phrasingBND - kBND_Sep1) + '0' + 1);
					roomLeft--;
					}
				if (!StuffText(&phonPtr, &roomLeft, "\p "))				/* space	*/
					err = bufTooSmall;

				}
			}
		#endif
		
				
		/*------------------------------*/
		/* Process embedded command		*/
		/*------------------------------*/
		if (vv->opTok->tokType == kECommandTok)
			{
			embedType = (tokStr[2] << 16) + tokStr[3];
			embedData = (tokStr[4] << 16) + tokStr[5];

			/*------------------------------*/
			/* Output DELIMITER string		*/
			/*------------------------------*/
			if (!StuffText(&phonPtr, &roomLeft, (unsigned char*)"\p[["))	/* regenerate a valid embedded command	*/
				err = bufTooSmall;
				
			/*------------------------------*/
			/* Output SELECTOR string		*/
			/*------------------------------*/
			relativeCmd = true;								/* assume command contains a relative value argument	*/
			switch (embedType)
					{
					case EC_pbar:
						embedType = EC_pbas;
						break;
					case EC_pmor:
						embedType = EC_pmod;
						break;
					case EC_ratr:
						embedType = EC_rate;
						break;
					case EC_volr:
						embedType = EC_volm;
						break;
					default:
						relativeCmd = false;
						break;
					}
			if (!StuffOSType(vv, &phonPtr, &roomLeft, embedType))
				err = bufTooSmall;

			/*------------------------------*/
			/* Output DATA string			*/
			/*------------------------------*/
			if (relativeCmd)
				{
				if ((Fixed) embedData < 0)
					{
					if (!StuffText(&phonPtr, &roomLeft, (unsigned char*)"\p-"))
						err = bufTooSmall;

					embedData = -embedData; 					/* convert back to positive	*/
					}
				else
					{
					if (!StuffText(&phonPtr, &roomLeft, (unsigned char*)"\p+"))
						err = bufTooSmall;
					}
				}

			if (embedType == EC_svox)							/* special case: need voice in upper half	*/
				embedData <<= 16;

			if (!StuffHexNum(&phonPtr, &roomLeft, embedData))
				err = bufTooSmall;

			/*------------------------------*/
			/* Output DELIMITER string		*/
			/*------------------------------*/
			if (!StuffText(&phonPtr, &roomLeft, (unsigned char*)"\p]] "))
				err = bufTooSmall;
			}
		else
			/*----------------------------------------------*/
			/* Convert PHONS to phoneme string				*/
			/*----------------------------------------------*/
			{
			while (vv->opCount)
				{
				cur_Phon = (short) (*tokStr++);						/* here's the next command opcode (extend from byte to BECommand)	*/
				
				cur_Phon1 = (-1);
				if (cur_Phon < 128)									/* in range of defined 1-byte opcodes	*/
					{
					if (cur_Phon < _Stress1_)
						{
						orig_Phon = cur_Phon << 1;
						cur_Phon = vv->Phon_to_Phon[ orig_Phon ]; 		/* convert to Speech Mgr phons	*/
						cur_Phon1 = vv->Phon_to_Phon[ orig_Phon +1 ];		/* in case it's 1 -> 2	*/
						}
					ASCII_phon = (vv->Opcode_To_ASCII[cur_Phon]) << 16; 	/* lookup the ASCII equivalent from table	*/
					if (cur_Phon1 >= 0)
						ASCII_phon |= vv->Opcode_To_ASCII[cur_Phon1]; 		/* lookup the ASCII equivalent from table	*/
					}
				else
					{
					ASCII_phon = 0;									/* these are all undefined right now	*/
					}

				while (ASCII_phon)
					{
					if (ASCII_phon & 0xFF000000)						/* pick off the high byte first	*/
						{
						if (roomLeft)
							{
							*phonPtr++ = (Byte) ((ASCII_phon & 0xFF000000) >> 24);
							roomLeft--;
							}
						else
							{
							err = bufTooSmall;						/* output buffer is not large enough to hold all opcodes	*/
							}
						}
					ASCII_phon <<= 8;
					}
				vv->opCount--;										/* one fewer opcodes in token	*/
				}
			}
		}
		
	if (err)											/* if anything goes wrong	*/
		e_AbortParse(vv);								/* stop the parse right away	*/

	*phonBytes = *phonBytes - roomLeft;					/* return number of chars we actually wrote into the buffer	*/

	if (err == noErr)									/* if no other errors were encountered	*/
		err = vv->Errors.oldest;						/* pick up any parsing errors which occurred	*/
		
	vv->Mode &= ~kDisableCallBacks;						/* turn callbacks back on now that we're done	*/
#endif


	return (err);
}







 void	e_GetSpeechErrors (voiceVarPtr vv, SpeechErrorInfo *info)
{
	/* need to add call to back-end that will keep back-end from calling us	*/
	/* while we are reading the Errors structure. (equivalent to turning off interrupts)	*/

	*info = vv->Errors;					/* return the whole structure to caller...	*/

	vv->Errors.count = 0;				/* ...and reset our internal data structure	*/
	vv->Errors.oldest = noErr;
	vv->Errors.oldPos = 0;
	vv->Errors.newest = noErr;
	vv->Errors.newPos = 0;
}


 void	e_GetInputMode (voiceVarPtr vv, unsigned long *info)
{
	*info = vv->Mode & kRawPhonemes ? 'PHON' : 'TEXT';
}


 void	e_GetCharacterMode (voiceVarPtr vv, unsigned long *info)
{
	*info = vv->Mode & kCharByChar ? 'LTRL' : 'NORM';
}


void	e_GetNumberMode (voiceVarPtr vv, unsigned long *info)
{
	*info = vv->Mode & kDigitByDigit ? 'LTRL' : 'NORM';
}



 short	e_SetInputMode (voiceVarPtr vv, unsigned long *info)
{
	unsigned long	mode;
	short			error = kNoError;

	mode = *info;
	if (mode == modeText)
		vv->Mode &= ~kRawPhonemes;						/* turn off direct phoneme input mode	*/
	else if (mode == modePhonemes)
		vv->Mode |= kRawPhonemes;						/* turn on direct phoneme input mode	*/
	else 
		error = kParamError;
	
	return (error);
}


 short	e_SetCharacterMode (voiceVarPtr vv, unsigned long *info)
{
	unsigned long	mode;
	short			error = kNoError;

	mode = *info;
	if (mode == modeNormal)
		vv->Mode &= ~kCharByChar;						/* turn off character-by-character mode	*/
	else if (mode == modeLiteral)
		vv->Mode |= kCharByChar;							/* turn on character-by-character mode	*/
	else error = kParamError;
	
	return (error);
}


short	e_SetNumberMode (voiceVarPtr vv, unsigned long *info)
{
	unsigned long	mode;
	short			error = kNoError;

	mode = *info;
	if (mode == modeNormal)
		vv->Mode &= ~kDigitByDigit;						/* turn off digit-by-digit number mode	*/
	else if (mode == modeLiteral)
		vv->Mode |= kDigitByDigit;						/* turn on digit-by-digit number mode	*/
	else error = kParamError;
	
	return (error);
}




 void	e_ResetFE (voiceVarPtr vv)
{
	ResetFE (vv);
}



 void	e_InitFE (voiceVarPtr vv)
{
	ResetFE (vv);
	vv->WordCB = false;
}



 void	e_SetCommandDelimiter (voiceVarPtr vv, DelimiterInfo *info)
{
	unsigned short		newBegin, newEnd;

	newBegin = 					  info->startDelimiter[0];
	newBegin = (newBegin << 8) 	| info->startDelimiter[1];
	newEnd   = 				   	  info->endDelimiter[0];
	newEnd   = (newEnd << 8) 	| info->endDelimiter[1];

	ChangeDelimiters(vv, newBegin, newEnd);				/* will change them soon (treated like an embedded command)	*/
}


 void e_AbortParse ( voiceVarPtr vv )
{

	vv->StrPos = vv->StrEOF;			/* bump our read ptr to end of input text buffer	*/
	vv->parseBusy = false;				/* and clear the busy flag	*/

}




 void e_Set_Word_CB_State ( voiceVarPtr vv, short state )
{
	vv->WordCB = state;
}






short e_StartParse ( voiceVarPtr vv, Ptr theStr, unsigned long byteLen, unsigned long controlFlags )
{
	short	error;

	error = NewParse ( vv, theStr, byteLen, controlFlags );
	return (error);
}






