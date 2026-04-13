/*
	File:		EmbeddedCmd.c

	Contains:	Implementation of MacInTalk2 FrontEnd Embedded Command Processor

	Written by:	Tim Schaaff

	Copyright:	© 1991-1992 by Apple Computer, Inc., all rights reserved.

	Change History (most recent first):

		<10>	 6/2/93		MC		Fix unterminated embedded cmds.
		 <9>	 6/2/93		MC		Remove "TX' and 'PH' arguments.
		 <8>	10/21/92	TIM		Parsing code for fractional part of 32-bit Fixed numbers got
									broken doing change <5>. Fixed it.
		 <7>	 10/8/92	TIM		Fixed odd-address bug in ChangeDelimiters.
		 <6>	 9/15/92	TIM		Map unimplCmd, badParmVal, & badParmCount errors to existing
									error return values.
		 <5>	  9/4/92	TIM		Got rid of some nasty inline code that wasn't really necessary.
		 <4>	 7/22/92	TIM		Make 'rset' embedded command get passed to Back-End.
		 <3>	 7/22/92	TIM		Fix bug in parsing of 'xtnd' command
		 <2>	 7/17/92	JDR		make it a link patch
		 <1>	  3/10/92	TIM		first created

*/



#ifndef __SPEECHSAY__
	#include "MT4.h"
#endif

#ifndef __SPEECHVERSIONS__
	#include "Versions.h"
#endif



/*--------------*/
/* FrontEnd.c	*/
/*--------------*/
extern void 	LogError	( voiceVarPtr vv, OSErr err);	/* will update internal error info & do error callback if required	*/
extern void 	GetNextCh 	( voiceVarPtr vv );				/* reads next character from input buffer	*/
extern void 	SetPOStoVal ( FETokenPtr tok, short posVal);


/* LOCAL PROTOTYPES	*/

void			LogParseError 			 ( voiceVarPtr vv, OSErr err);
short 			IsDigit					 ( Byte ch );
short 			IsHexDigit				 ( Byte ch );
void 			SkipWhiteSpace 			 ( voiceVarPtr vv );		/* skips over spaces and control chars	*/
void 			SkipToNextCommand 		 ( voiceVarPtr vv );		/* skips over non-command-delimiting chars	*/
void 			DoneWithCommand			 ( voiceVarPtr vv );
unsigned long 	Get32BitHexValue		 ( voiceVarPtr vv );		/* parse a 32-bit Fixed-point value	*/
unsigned long 	Get32BitFixedValue		 ( voiceVarPtr vv );		/* parse a 32-bit Fixed-point value	*/
unsigned long 			GetSelectorValue 		 ( voiceVarPtr vv );
unsigned long 	Get32BitValue			 ( voiceVarPtr vv, short useFixed );		/* parse a 32-bit value (Fixed or Hex)	*/
void 			ChangeDelimiters		 ( voiceVarPtr vv, unsigned short newBegin, unsigned short newEnd );
void 			ChangeCharMode			 ( voiceVarPtr vv, unsigned long newMode );
void 			ChangeInputMode			 ( voiceVarPtr vv, unsigned long newMode );
void			ChangeNumberMode		 ( voiceVarPtr vv, unsigned long newMode );
void 			ChangePitchBase			 ( voiceVarPtr vv, Fixed newPitch );
void 			ChangePitchModulation	 ( voiceVarPtr vv, Fixed newMod );
void 			ChangeRate				 ( voiceVarPtr vv, Fixed newRate );
void 			ChangeVolume			 ( voiceVarPtr vv, Fixed newVolume );
void 			HandleReset				 ( voiceVarPtr vv, unsigned long resetMsg );
void 			Parse_char_Command		 ( voiceVarPtr vv );
void 			Parse_cmnt_Command 		 ( voiceVarPtr vv );
void 			Parse_dlim_Command		 ( voiceVarPtr vv );
void 			Parse_emph_Command		 ( voiceVarPtr vv );
void 			Parse_mode_Command		 ( voiceVarPtr vv );
void 			Parse_nmbr_Command		 ( voiceVarPtr vv );
void 			Parse_pbas_Command		 ( voiceVarPtr vv );
void 			Parse_pmod_Command		 ( voiceVarPtr vv );
void 			Parse_rate_Command		 ( voiceVarPtr vv );
void 			Parse_rset_Command		 ( voiceVarPtr vv );
void 			Parse_slnc_Command		 ( voiceVarPtr vv );
void 			Parse_sync_Command		 ( voiceVarPtr vv );
void 			Parse_vers_Command		 ( voiceVarPtr vv );
void 			Parse_volm_Command		 ( voiceVarPtr vv );
void 			Parse_xtnd_Command		 ( voiceVarPtr vv );
void 			ProcessEmbeddedCommands  ( voiceVarPtr vv );		/* parses embedded commands	*/
void			ResetFE 				 ( voiceVarPtr vv );


#define kGetFixed	true
#define	kGetLong	false


/*	============================================
		Argument & Number Parsing Primitives
	============================================ */

void LogParseError ( voiceVarPtr vv, OSErr err)
{
	if (!vv->parseErrorLogged)							/* limit parse errors to one per embedded command	*/
	{
		vv->parseErrorLogged = true;
		LogError(vv, err);
	}
}

short IsDigit ( Byte ch )
{
	return ((ch >= '0') && (ch <= '9'));
}

short IsHexDigit ( Byte ch )
{
	return (	((ch >= '0') && (ch <= '9'))
			||  ((ch >= 'a') && (ch <= 'f'))
			||	((ch >= 'A') && (ch <= 'F')));
}



void SkipWhiteSpace ( voiceVarPtr vv )	/* skips over spaces and control chars	*/
{

	while ((!vv->AtCmdEnd)
	&&	   (vv->Ch != CmdSeparator)
	&&	   (vv->Ch != kEOFCh)
	&&	   (vv->Ch <= ' '))
		GetNextCh(vv);

}



void SkipToNextCommand ( voiceVarPtr vv )	/* skips over non-command-delimiting chars	*/
{

	while ((!vv->AtCmdEnd)
	&&	   (vv->Ch != CmdSeparator)
	&& 	   (vv->Ch != kEOFCh))
		GetNextCh(vv);

	if (vv->Ch == CmdSeparator)						/* we have hit the command separator	*/
		GetNextCh(vv);								/* skip it	*/

}

void DoneWithCommand ( voiceVarPtr vv )
{

	SkipWhiteSpace(vv);					/* scan past white space	*/

	if ((!vv->AtCmdEnd) 					/* we still have more chars at end of command	*/
	&&  (vv->Ch != CmdSeparator))
	{
		vv->parseErrorPos = (unsigned long)vv->StrPos;	/* latch position of start of this error	*/

		LogParseError(vv, kParamError);
		SkipToNextCommand(vv);			/* scan past any garbage at end of command	*/
	}

	else if (vv->Ch == CmdSeparator)		/* looking at a command delimiter	*/
		GetNextCh(vv);					/* step past it	*/
}




unsigned long Get32BitHexValue ( voiceVarPtr vv )	/* parse a 32-bit Fixed-point value	*/
{
	unsigned long 	value = 0;
	unsigned short	base;
	short			gotDigit = false;


	if (vv->AtCmdEnd || (vv->Ch == CmdSeparator))	/* have hit end of command	*/
	{
		LogParseError(vv, kParamError);				/* expected parameter but hit end of command	*/
		return 0;
	}

	while (IsHexDigit(vv->Ch))
	{
		gotDigit = true;

		if (vv->Ch >= 'a')
			base = 'a' - 10;
		else if (vv->Ch >= 'A')
			base = 'A' - 10;
		else
			base = '0';

		value = (value << 4) + (vv->Ch - base);

		GetNextCh(vv);
	}

	if (!gotDigit)
		LogParseError(vv, kParamError);

	return value;
}




unsigned long Get32BitFixedValue ( voiceVarPtr vv )	/* parse an UNSIGNED 32-bit Fixed-point value	*/
{
	unsigned short	msb = 0;
	unsigned short 	lsb = 0;
	unsigned long  	amt;
	short			i;
	short			gotDigit = false;


	if (vv->AtCmdEnd || (vv->Ch == CmdSeparator))	/* have hit end of command	*/
		{
		LogParseError(vv, kParamError);				/* expected parameter but hit end of command	*/
		return 0;
		}

	while (IsDigit(vv->Ch))
		{
		gotDigit = true;

		msb = msb * 10 + (vv->Ch - '0');

		GetNextCh(vv);
		}

	if (vv->Ch == '.')							/* input value contains a decimal point	*/
		{
		GetNextCh(vv);							/* skip past the decimal point	*/

		i = 0;
		while (IsDigit(vv->Ch) && (i < 5))
			{
			gotDigit = true;

			amt = ((vv->Ch - '0') << 16) + (vv->divisorsPtr[i] >> 1);
			amt = amt/vv->divisorsPtr[i];

			if (i == 4)							/* last digit is special case	*/
				amt >>= 1;						/* divisor[4] is only half what it should be	*/
												/* this will give us one more digit of input precision	*/
			lsb += amt;

			GetNextCh(vv);
			i++;
			}

		while (IsDigit(vv->Ch))					/* skip over any remaining (insignificant) digits	*/
			GetNextCh(vv);
		}

	if (!gotDigit)
		LogParseError(vv, kParamError);

	return (msb << 16) | lsb;					/* construct the full 32-bit Fixed value	*/
}


unsigned long Get32BitLongValue ( voiceVarPtr vv )	/* parse an UNSIGNED 32-bit Fixed-point value	*/
{
	unsigned long	val = 0;
	short			gotDigit = false;


	if (vv->AtCmdEnd || (vv->Ch == CmdSeparator))	/* have hit end of command	*/
		{
		LogParseError(vv, kParamError);				/* expected parameter but hit end of command	*/
		return 0;
		}

	while (IsDigit(vv->Ch))
		{
		gotDigit = true;

		val = val * 10 + (vv->Ch - '0');

		GetNextCh(vv);
		}

	if (!gotDigit)
		LogParseError(vv, kParamError);

	return (val);
}







unsigned long GetSelectorValue ( voiceVarPtr vv )
{
	Byte 	delimiter = ' '; /* assume we will scan an un-quoted 4 Character unsigned long selector	*/
	Byte	len = 0;
	unsigned long	selector = 0;


	if (vv->AtCmdEnd || (vv->Ch == CmdSeparator))	/* have hit end of command	*/
	{
		LogParseError(vv, kParamError);			/* expected parameter but hit end of command	*/
		return 0;
	}

	if (vv->Ch == '\'')							/* expect a single-quoted selector ('XXXX')	*/
		delimiter = '\'';
	else if (vv->Ch == '\"')						/* expect a double-quoted selector ("XXXX")	*/
		delimiter = '\"';

	if (delimiter == ' ')						/* scan an undelimited unsigned long value	*/
	{
		while ((vv->Ch > ' ')					/* but stop scanning if space char or non-printable char	*/
		&&	   (!vv->AtCmdEnd) 					/* or end-command delimiter found	*/
		&&	   (vv->Ch != CmdSeparator)) 		/* or command separator char found	*/
		{
			selector = (selector << 8) | vv->Ch;	/* OR in the next char	*/
			len++;
			GetNextCh(vv);						/* and get a new character	*/
		}
	}
	else										/* scan a quoted unsigned long value	*/
	{
		GetNextCh(vv);							/* skip past the delimiter	*/

		while ((vv->Ch != delimiter)				/* scan until we find a match	*/
		&&	   (!vv->AtCmdEnd) 					/* or end-command delimiter found	*/
		&&	   (vv->Ch != CmdSeparator)) 		/* or command separator char found	*/
		{
			selector = (selector << 8) | vv->Ch;	/* OR in the next char	*/
			len++;
			GetNextCh(vv);						/* and get a new character	*/
		}

		if (vv->Ch == delimiter)
			GetNextCh(vv);						/* step to next character	*/
	}

	if (len > 4)								/* we scanned too many chars	*/
		LogParseError(vv, kParamError);

	return selector;
}




unsigned long Get32BitValue ( voiceVarPtr vv, short useFixed )	/* parse a 32-bit value (Fixed, Long, OSType or Hex)	*/
{
	unsigned long	value = 0;

	SkipWhiteSpace(vv);							/* scan past leading white space	*/

	vv->parseErrorPos = (unsigned long)vv->StrPos;				/* latch position of start of this token	*/

	if (vv->AtCmdEnd || (vv->Ch == CmdSeparator))	/* have hit end of command	*/
	{
		LogParseError(vv, kParamError);			/* expected more but hit end of command	*/
		return 0;
	}

	if ((((vv->Ch << 8) | vv->NextCh) == '0x') 	/* it's a hexadecimal value in 0x00000000 format	*/
	||  (((vv->Ch << 8) | vv->NextCh) == '0X')) 	/* it's a hexadecimal value in 0X00000000 format	*/
	{
		GetNextCh(vv);							/* skip past the '0x' prefix	*/
		GetNextCh(vv);
		value = Get32BitHexValue(vv);			/* parse a hexadecimal value	*/
	}
	else if (vv->Ch == '$')						/* it's a hexadecimal value in $00000000 format	*/
	{
		GetNextCh(vv);							/* skip past the '$' prefix	*/
		value = Get32BitHexValue(vv);			/* parse a hexadecimal value	*/
	}
	else if (IsDigit(vv->Ch) || (vv->Ch == '.'))	/* it's a decimal format number	*/
		{
		if (useFixed)
			value = (unsigned long) Get32BitFixedValue(vv);
		else
			value = (unsigned long) Get32BitLongValue(vv);
		}
	else										/* otherwise try to get a 32-bit unsigned long selector value 'ABCD' or ABCD	*/
		value = (unsigned long) GetSelectorValue(vv);

	SkipWhiteSpace(vv);					/* scan past white space to next arguments (if any)	*/
	return value;
}


/*	============================================
			Individual Command Handlers
	============================================ */

void ChangeDelimiters ( voiceVarPtr vv, unsigned short newBegin, unsigned short newEnd )
{
	if (!newBegin || !newEnd)								/* if either is null, then disable embedded command processing	*/
		newBegin = newEnd = 0;

	vv->PendingCommands |= kNewDelimiters;					/* set flag for higher-level code	*/

	vv->NewCmdBeginDelim[0] 	= newBegin >> 8;				/* copy new delimiter chars into	*/
	vv->NewCmdBeginDelim[1] 	= newBegin & 0xFF;				/* channel change globals	*/
	vv->NewCmdEndDelim[0] 	= newEnd >> 8;
	vv->NewCmdEndDelim[1] 	= newEnd & 0xFF;
}

void ChangeCharMode ( voiceVarPtr vv, unsigned long newMode )
{
	switch (newMode)
	{
		case modeNormal:		/* switch into normal word speaking mode	*/
			vv->PendingCommands |= kNewCharMode;
			vv->NewMode &= ~kCharByChar;
			break;

		case modeLiteral:		/* switch into char-by-char speaking mode	*/
			vv->PendingCommands |= kNewCharMode;
			vv->NewMode |= kCharByChar;
			break;
	}
}

void ChangeInputMode ( voiceVarPtr vv, unsigned long newMode )
{
	switch (newMode)
	{
		case modePhonemes:
			vv->PendingCommands |= kNewInputMode;
			vv->NewMode |= kRawPhonemes;
			break;

		case modeText:
			vv->PendingCommands |= kNewInputMode;
			vv->NewMode &= ~kRawPhonemes;
			break;
	}
}

void ChangeNumberMode ( voiceVarPtr vv, unsigned long newMode )
{
	switch (newMode)
	{
		case modeNormal:		/* switch into normal number speaking mode	*/
			vv->PendingCommands |= kNewDigitMode;
			vv->NewMode &= ~kDigitByDigit;
			break;

		case modeLiteral:		/* switch into digit-by-digit speaking mode	*/
			vv->PendingCommands |= kNewDigitMode;
			vv->NewMode |= kDigitByDigit;
			break;
	}
}

void ChangePitchBase ( voiceVarPtr vv, Fixed newPitch )
{

	vv->PendingCommands |= kNewPitchBase;						/* set flag for higher-level code	*/
	vv->NewPitchBase = newPitch;									/* here's the new pitch to send to back-end	*/
}

void ChangePitchModulation ( voiceVarPtr vv, Fixed newMod )
{

	vv->PendingCommands |= kNewPitchMod;							/* set flag for higher-level code	*/
	vv->NewPitchMod = newMod;									/* here's the new mod depth to send to back-end	*/
}

void ChangeRate ( voiceVarPtr vv, Fixed newRate )
{

	vv->PendingCommands |= kNewRate;								/* set flag for higher-level code	*/
	vv->NewRate = newRate;										/* here's the new rate to send to back-end	*/
}

void ChangeVolume ( voiceVarPtr vv, Fixed newVolume )
{

	vv->PendingCommands |= kNewVolume;							/* set flag for higher-level code	*/
	vv->NewVolume = newVolume;									/* here's the new volume to send to back-end	*/
}



void	ResetFE (voiceVarPtr vv)
{
	vv->Mode = 0;
	ChangeDelimiters(vv, defaultCmdBeginDelim, defaultCmdEndDelim);
}


void HandleReset ( voiceVarPtr vv, unsigned long resetMsg )
{

	ResetFE (vv);
	vv->PendingCommands |= kNewReset;							/* set flag for higher-level code	*/
	vv->NewReset = resetMsg;										/* here's the reset message to send to back-end	*/
}


/*	============================================
			Individual Command Parsers
	============================================ */

void Parse_char_Command ( voiceVarPtr vv )
{
	unsigned long	mode;

	mode = (unsigned long) Get32BitValue(vv, kGetFixed);	/* pick up character processing mode argument	*/
	mode &= 0xDFDFDFDF;										/* convert to uppercase	*/

	if ((mode == modeNormal) || (mode == modeLiteral))
	{
		if (!vv->parseErrorLogged)		/* we did not get any errors	*/
			ChangeCharMode(vv, mode);
	}
	else
		LogParseError(vv, kParamError);

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}

void Parse_cmnt_Command ( voiceVarPtr vv )
{
	while (!vv->AtCmdEnd)
		SkipToNextCommand(vv);			/* skip over the rest of the embedded command	*/
}

void Parse_dlim_Command ( voiceVarPtr vv )
{
	unsigned long newBegin;
	unsigned long newEnd;

	newBegin = Get32BitValue(vv, kGetFixed);			/* get the new begin-embedded-command delimiter	*/

	if (newBegin >> 16)						/* delimiters can only be 2-bytes long	*/
		LogParseError(vv, kParamError);
	else
	{
		if (!(newBegin & 0xFF00))			/* delimiter is 0 or 1 chars	*/
			newBegin <<= 8;					/* make sure single char is in most-significant byte	*/

		SkipWhiteSpace(vv);					/* scan past leading white space	*/

		newEnd = Get32BitValue(vv, kGetFixed);			/* get the new end-embedded-command delimiter	*/

		if (newEnd >> 16)					/* delimiters can only be 2-bytes long	*/
			LogParseError(vv, kParamError);
		else								/* we got two good delimiters	*/
		{
			if (!(newEnd & 0xFF00))			/* delimiter is 0 or 1 chars	*/
				newEnd <<= 8;				/* make sure single char is in most-significant byte	*/

			ChangeDelimiters(vv, newBegin, newEnd);
		}
	}

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}

void Parse_emph_Command ( voiceVarPtr vv )
{
	if (vv->Ch == '+')							/* a valid argument for the 'emph' command	*/
		{
		vv->PendingCommands |= kNewEmphasis;
		vv->NewEmphasis = kEmphasizeWord;
		GetNextCh(vv);							/* step over the character	*/
		}

	else if (vv->Ch == '-')							/* a valid argument for the 'de-emph' command	*/
		{
		vv->PendingCommands |= kNewEmphasis;
		vv->NewEmphasis = kDeemphasizeWord;
		GetNextCh(vv);							/* step over the character	*/
		}

	else if (	(vv->Ch == CmdSeparator)		/* if we have hit the end of the command	*/
				|| (vv->AtCmdEnd))					/* then we didn't get enough parameters	*/
		LogParseError(vv, kParamError);

	else									/* otherwise we just got a bad argument	*/
		LogParseError(vv, kParamError);

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}



void Parse_mode_Command ( voiceVarPtr vv )
{
	unsigned long	mode;

	mode = (unsigned long) Get32BitValue(vv, kGetFixed);		/* pick up input mode argument	*/
	mode &= 0xDFDFDFDF;						/* convert to uppercase	*/

	if ( (mode == modeText) || (mode == modePhonemes) )
	{
		if (!vv->parseErrorLogged)			/* we did not get any errors	*/
			ChangeInputMode(vv, mode);
	}
	else
		LogParseError(vv, kParamError);

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}



void Parse_nmbr_Command ( voiceVarPtr vv )
{
	unsigned long	mode;

	mode = (unsigned long) Get32BitValue(vv, kGetFixed);	/* pick up number mode argument	*/
	mode &= 0xDFDFDFDF;						/* convert to uppercase	*/

	if ((mode == modeNormal) || (mode == modeLiteral))
	{
		if (!vv->parseErrorLogged)			/* we did not get any errors	*/
			ChangeNumberMode(vv, mode);
	}
	else
		LogParseError(vv, kParamError);

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}

void Parse_pbas_Command ( voiceVarPtr vv )
{
	Fixed 	value;
	Byte	relative = 0;

	if ((vv->Ch == '+') || (vv->Ch == '-'))	/* following baseline pitch is a relative change value	*/
	{
		relative = vv->Ch;					/* save relative direction	*/

		GetNextCh(vv);						/* skip this char	*/

		SkipWhiteSpace(vv);					/* and skip over white space	*/
	}

	value = (Fixed) Get32BitValue(vv, kGetFixed);		/* parse a 32-bit value (Fixed or Hex)	*/

	if (value & (0xC000 << 16))				/* value is greater than our allowed max	*/
		value = 0x3FFFFFFF;					/* limit to our max	*/

	if (relative)
	{
		if (relative == '+')
			value |= 0x40000000;			/* means relative addition	*/
		else
			value |= 0xC0000000;			/* means relative subtraction	*/
	}

	if (!vv->parseErrorLogged)				/* we did not get any errors	*/
		ChangePitchBase(vv, value);

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}

void Parse_pmod_Command ( voiceVarPtr vv )
{
	Fixed 	value;
	Byte	relative = 0;

	if ((vv->Ch == '+') || (vv->Ch == '-'))	/* following pitch modulation is a relative change value	*/
	{
		relative = vv->Ch;					/* save relative direction	*/

		GetNextCh(vv);						/* skip this char	*/

		SkipWhiteSpace(vv);					/* and skip over white space	*/
	}

	value = (Fixed) Get32BitValue(vv, kGetFixed);		/* parse a 32-bit value (Fixed or Hex)	*/

	if (value & (0xC000 << 16))				/* value is greater than our allowed max	*/
		value = 0x3FFFFFFF;					/* limit to our max	*/

	if (relative)
	{
		if (relative == '+')
			value |= 0x40000000;			/* means relative addition	*/
		else
			value |= 0xC0000000;			/* means relative subtraction	*/
	}

	if (!vv->parseErrorLogged)				/* we did not get any errors	*/
		ChangePitchModulation(vv, value);

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}

void Parse_rate_Command ( voiceVarPtr vv )
{
	Fixed 	value;
	Byte	relative = 0;

	if ((vv->Ch == '+') || (vv->Ch == '-'))	/* following rate is a relative change value	*/
	{
		relative = vv->Ch;					/* save relative direction	*/

		GetNextCh(vv);						/* skip this char	*/

		SkipWhiteSpace(vv);					/* and skip over white space	*/
	}

	value = (Fixed) Get32BitValue(vv, kGetFixed);		/* parse a 32-bit value (Fixed or Hex)	*/

	if (value & (0xC000 << 16))				/* value is greater than our allowed max	*/
		value = 0x3FFFFFFF;					/* limit to our max	*/

	if (relative)
	{
		if (relative == '+')
			value |= 0x40000000;			/* means relative addition	*/
		else
			value |= 0xC0000000;			/* means relative subtraction	*/
	}

	if (!vv->parseErrorLogged)				/* we did not get any errors	*/
		ChangeRate(vv, value);

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}

void Parse_rset_Command ( voiceVarPtr vv )
{
	unsigned long value;

	value = Get32BitValue(vv, kGetFixed);	/* parse a 32-bit value (Fixed or Hex)	*/

	if (value == 0)
	{
		if (!vv->parseErrorLogged)			/* we did not get any errors	*/
			HandleReset(vv, value);
	}
	else
		LogParseError(vv, kParamError);

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}

void Parse_slnc_Command ( voiceVarPtr vv )
{
	unsigned long value;

	value = Get32BitValue(vv, kGetFixed);				/* parse a 32-bit value (Fixed or Hex)	*/

	vv->PendingCommands |= kNewSilence;	/* set flag for higher-level code	*/
	vv->NewSilence = value;				/* accumulate total delay  to handle multiple 'slnc' commands ?????	*/

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}

void Parse_sync_Command ( voiceVarPtr vv )
{
	Fixed 	value;

	value = (Fixed) Get32BitValue(vv, kGetLong);	/* parse a 32-bit value (Long or Hex)	*/

	/* LIMITATION:  If you embed more than one 'sync' command in a single command << ... >>)	*/
	/* only the last callback will happen.  I don't know if this will be a problem.	*/

	vv->PendingCommands |= kNewSync;			/* set flag for higher-level code	*/
	vv->NewSync = value;						/* here's the sync message to return to app	*/

	DoneWithCommand(vv);							/* test for extra args and get ready for next command	*/
}

void Parse_vers_Command ( voiceVarPtr vv )
{
	unsigned long value;

	value = Get32BitValue(vv, kGetFixed);	/* parse a 32-bit value (Fixed or Hex)	*/

	if (value != 0x00010000)				/* expect to see version 1.0, accept version 0.0, too	*/
		LogParseError(vv, kParamError);

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}





void Parse_volm_Command ( voiceVarPtr vv )
{
	Fixed 	value;
	Byte	relative = 0;

	if ((vv->Ch == '+') || (vv->Ch == '-'))	/* following volume is a relative change value	*/
	{
		relative = vv->Ch;					/* save relative direction	*/

		GetNextCh(vv);						/* skip this char	*/

		SkipWhiteSpace(vv);					/* and skip over white space	*/
	}

	value = (Fixed) Get32BitValue(vv, kGetFixed);		/* parse a 32-bit value (Fixed or Hex)	*/

	if (value & (0xC000 << 16))				/* value is greater than our allowed max	*/
		value = 0x3FFFFFFF;					/* limit to our max	*/

	if (relative)
	{
		if (relative == '+')
			value |= 0x40000000;			/* means relative addition	*/
		else
			value |= 0xC0000000;			/* means relative subtraction	*/
	}

	if (!vv->parseErrorLogged)				/* we did not get any errors	*/
		ChangeVolume(vv, value);

	DoneWithCommand(vv);						/* test for extra args and get ready for next command	*/
}



void Parse_note_Command ( voiceVarPtr vv )
{
	Fixed 	value;

	value = (Fixed) Get32BitValue(vv, kGetFixed);		/* parse a 32-bit value (Fixed or Hex)	*/

	if (!vv->parseErrorLogged)				/* we did not get any errors	*/
		{
		vv->PendingCommands |= kNewNote;	/* set flag for higher-level code	*/
		vv->NewNote = value;				/* here's the new volume to send to back-end	*/
		}

	DoneWithCommand(vv);					/* test for extra args and get ready for next command	*/
}



void Parse_tempo_Command ( voiceVarPtr vv )
{
	Fixed 	value;

	value = (Fixed) Get32BitValue(vv, kGetFixed);		/* parse a 32-bit value (Fixed or Hex)	*/

	if (!vv->parseErrorLogged)				/* we did not get any errors	*/
		{
		vv->PendingCommands |= kNewTempo;	/* set flag for higher-level code	*/
		vv->NewTempo = value;				/* here's the new volume to send to back-end	*/
		}

	DoneWithCommand(vv);					/* test for extra args and get ready for next command	*/
}


void Parse_marker_Command ( voiceVarPtr vv )
{
	Fixed 	value;

	value = (Fixed) Get32BitValue(vv, kGetFixed);		/* parse a 32-bit value (Fixed or Hex)	*/

	if (!vv->parseErrorLogged)				/* we did not get any errors	*/
		{
		vv->PendingCommands |= kNewMarker;	/* set flag for higher-level code	*/
		vv->NewMarker = value;				/* here's the new volume to send to back-end	*/
		}

	DoneWithCommand(vv);					/* test for extra args and get ready for next command	*/
}




void Parse_svox_Command ( voiceVarPtr vv )
{
	Fixed 	value;

	value = (Fixed) Get32BitValue(vv, kGetFixed);		/* parse a 32-bit value (Fixed or Hex)	*/
	value >>= 16;							/* no fractions allowed	*/
	
	if ( (value < 1) || (value > kMaxVoice) )
		value = 1;

	if (!vv->parseErrorLogged)				/* we did not get any errors	*/
		{
		vv->PendingCommands |= kNewVoice;	/* set flag for higher-level code	*/
		vv->NewVoice = value;				/* here's the new volume to send to back-end	*/
		}

	DoneWithCommand(vv);					/* test for extra args and get ready for next command	*/
}







void Parse_xtnd_Command ( voiceVarPtr vv )
{
	unsigned long	data;
	unsigned long	lastPosn;

	lastPosn = vv->parseErrorPos;						/* save position in case cmd is unrecognized	*/
	data = Get32BitValue(vv, kGetFixed);							/* Get synth ID	*/

	if (data == kMacInTalkCreator)						/* ignore the command quietly unless it is directed at us	*/
		{
		data = Get32BitValue(vv, kGetFixed);						/* Get extended command selector	*/
		switch (data)
			{
			case 'wpos':										/* Word part-of-speech	*/
				data = (Get32BitValue(vv, kGetFixed)) >> 16;				/* Get POS code	(fixed to integer)	*/
				if (data <= kLastPOS)
					SetPOStoVal ((FETokenPtr) &vv->tokBuffer[vv->CurTok], (short) data);
				else
					LogParseError(vv, kParamError);
				break;

			default:
				vv->parseErrorPos = lastPosn;  			/* restore pos to start of cmd selector	*/
				LogParseError(vv, kUnknownEmbeddedCmd);
				break;
			}
		}

	SkipToNextCommand(vv);								/* skip over the rest of the embedded command	*/
}







/*	============================================
		  Main Embedded Command Parser
	============================================ */

void ProcessEmbeddedCommands ( voiceVarPtr vv ) /* parses embedded commands	*/
{
	short 		done = false;
	unsigned long			cmd;
	unsigned long	lastPos;

	/* scan embedded command tokens	*/
	/* loop until we hit end-command delimiter or EOF	*/

	/* Improvements to make:	*/
	/*		detect occurrences of '<<' (denotes doubly-embedded command)	*/
	/*		limit total command length to something reasonable to catch missing '>>'	*/

	while (!done)
	{
		SkipWhiteSpace(vv);						/* scan past leading white space	*/

		if (vv->Ch == kEOFCh)
			done = true;						/* hit end of text - time to quit	*/

		else if (vv->AtCmdEnd) 					/* have hit the end-of-command delimiter	*/
		{
			vv->Mode &= ~kCommand;				/* time to get out of embedded-command mode	*/

			GetNextCh(vv);						/* skip over first delimiter char	*/

			if (vv->CmdEndDelim[1])				/* command end delimiter is 2 chars long	*/
				GetNextCh(vv);					/* skip over second delimiter char	*/

			done = true;
		}

		if (!done)
		{
			vv->parseErrorLogged = false;		/* reset flag so we are able to report another error	*/

			cmd = (unsigned long) Get32BitValue(vv, kGetFixed);	/* pick up a command selector	*/
			
			/*----------------------------------	*/
			/* Force selector case to lower	*/
			/*----------------------------------	*/
			
			cmd |= 0x20202020;					/* NOTE: assumes embedded selectors are ALWAYS 4 char alpha	*/

			SkipWhiteSpace(vv);					/* scan past white space to arguments (if any)	*/

			lastPos = vv->parseErrorPos;			/* save position in case cmd is unrecognized	*/
			vv->parseErrorPos = (unsigned long)vv->StrPos;		/* latch position in case we hit any errors	*/

			switch (cmd)						/* branch on embedded-command type	*/
			{
				case 'char':					/* switch between normal char processing mode and literal, char-by-char mode	*/
					Parse_char_Command(vv);
					break;
				case 'cmnt':
					Parse_cmnt_Command(vv);
					break;
				case 'dlim':
					Parse_dlim_Command(vv);
					break;
				case 'emph':
					Parse_emph_Command(vv);
					break;
				case 'inpt':					/* switch between TEXT and PHON input modes	*/
					Parse_mode_Command(vv);
					break;
				case 'nmbr':
					Parse_nmbr_Command(vv);
					break;
				case 'pbas':
					Parse_pbas_Command(vv);
					break;
				case 'pmod':
					Parse_pmod_Command(vv);
					break;
				case 'rate':
					Parse_rate_Command(vv);
					break;
				case 'rset':
					Parse_rset_Command(vv);
					break;
				case 'slnc':
					Parse_slnc_Command(vv);
					break;
				case 'sync':
					Parse_sync_Command(vv);
					break;
				case 'vers':
					Parse_vers_Command(vv);
					break;
				case 'volm':
					Parse_volm_Command(vv);
					break;
				case 'note':
					Parse_note_Command(vv);
					break;
				case 'tmpo':
					Parse_tempo_Command(vv);
					break;
				case 'mark':
					Parse_marker_Command(vv);
					break;
				case 'svox':
					Parse_svox_Command(vv);
					break;
				case 'xtnd':
					Parse_xtnd_Command(vv);
					break;

				default:
					vv->parseErrorPos = lastPos;  	/* restore pos to start of cmd selector	*/
					LogParseError(vv, kUnknownEmbeddedCmd);
					SkipToNextCommand(vv);			/* skip over the rest of the embedded command	*/
					break;
			}
		}
	}

}

