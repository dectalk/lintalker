

#ifndef __SPEECHSAY__
	#include "MT4.h"
#endif






/********************* Function Prototypes ************/
void	EngToP (voiceVarPtr g, char *inalpha,char *phon);
short	FindConsonant (voiceVarPtr g, unsigned char **p_i_ptr);
short	FindSibilant(voiceVarPtr g, unsigned char **p_i_ptr);
short	FindVowel (voiceVarPtr g, unsigned char **p_i_ptr);
short	search_special (voiceVarPtr g, unsigned char **p_i_ptr, unsigned char *sprule);
/*short	exclaim_rule (char * *p_i_ptr);	*/
unsigned char 	*dorule (voiceVarPtr g, unsigned char *i_ptr, unsigned char *r_ptr);


/********************* Constants ***********************/
/*#define DELIMITER '{'	*/
/* rule delimiter character.	*/
#define DELIMITER 0xFF

/* 'kind of char' bit type table.	*/
#define CONSON  0x01
#define HISCON  0x02    /* sibilant consonant.	*/
#define UMODR   0x04
#define VOICON  0x08    /* voiced consonant	*/
#define VOWEL   0x10
#define FRONT   0x20    /* front vowel	*/
#define SPCHAR  0x80    /* special rule macro table.	*/






/***************************************************************************
  FUNCTION:     engtop
  DESCRIPTION:  preforms a english-to-phonetic translation on a WORD!
                The word should be all caps, with some spaces after it.

  RETURNS:      void
  PARAMETERS:   char * english string -
                char * phonetic string - memory to store the resulting
                    phonetic string in.
  SIDE EFFECTS: No length checking is preformed. The phonetic string should be
                roughly twice the length of the english text.
                the input string should also be a little longer that what it
                contains - we'll eat up four or so bytes, and the string
                is no longer a string after we're done with it.
  REVISIONS:    ????    FB
                3/31/90 -jf-
****************************************************************************/
void EngToP (voiceVarPtr g, char *inalpha, char *phon)
{
    unsigned char 		*scan_ptr,*input_ptr;
    unsigned char 		*rule_ptr,*nextrule;
	short				strLen;
	char				*phonStr;

	
	phonStr = phon+1;						/* point past length byte	*/
	strLen = inalpha[0];					/* we'll restore the len byte when done	*/
	inalpha[0] = ' ';						/* mark word start with space	*/
    input_ptr = (unsigned char*)&inalpha[1];  				/* start scanning ahead of the space.	*/
	
	*phonStr++ = _Word_;			/* prefix output with normal-stress word-prominence code	*/
	
    while (*input_ptr != ' ') 				/* until were done.	*/
    	{   								/* this is the top of the new input letter loop.	*/
        if (*input_ptr == '\'' || *input_ptr == '.')
        	{
            input_ptr++;
            continue;       				/* Ignore ' and . completely	*/
        	}
        nextrule = g->rule + g->hash[*input_ptr-'A'];

        while (true)
        	{   							/* this is the top of the search for a rule loop.	*/
            								/* all those continue statements that follow end up back here.	*/
            scan_ptr = input_ptr;
            rule_ptr = nextrule;
            nextrule = rule_ptr + *rule_ptr;    /* Find the location of the next rule, save it.	*/
            rule_ptr++;                     /* scan past the length byte	*/
            while (true)                    /* scan out the entire middle rule	*/
            	{
                scan_ptr++;
                if (*scan_ptr != *rule_ptr) break;
                rule_ptr++;
            	}
            if (*rule_ptr != DELIMITER)
                continue;               /* do the next rule if we don't match	*/
            rule_ptr++;                 /* past the delimiter	*/
            g->e_direction = (-1);
            if ((rule_ptr = dorule(g, input_ptr-1, rule_ptr)) == NULL)  /* left half	*/
                continue;               /* no match - do next rule.	*/
            g->e_direction = 1;
            if ((rule_ptr = dorule(g, scan_ptr,rule_ptr)) == NULL)     /* right half	*/
                continue;
            input_ptr = scan_ptr;         /* point at the next text to decode	*/
                                        /* this is used at the top of the loop.	*/

            /* here we have found a rule, lets copy it over	*/
            while (*rule_ptr != DELIMITER)
            	{
                *phonStr = (*rule_ptr) -1;	/* @@@@ Remove bias (Rule sys not happy with 0 phon - IY)	*/
                phonStr++;
				rule_ptr++;
            	}   					/* copy the rule over into the output string.	*/
            break;  					/* out of the rule scan while loop.	*/
        	}
    	}   							/* end of the while loop for letter search.	*/
	
	inalpha[0] = strLen;				/* restore the len byte	*/
	phon[0] = phonStr - phon -1;

}



/***************************************************************************
  FUNCTION:     FindConsonant
  DESCRIPTION:  hat macro rule - searches input string for 1 consonant.
                QU and GU are in the list of consonants.
                THIS FUNCTION IS THE PLACE TO START WHEN OPTIMIZING!
  RETURNS:      0 - good match
                 != 0  missed match
  PARAMETERS:   pointer to the pointer to the input string
  SIDE EFFECTS: .
  REVISIONS:    4/6/90  -jf-
****************************************************************************/
short	FindConsonant (voiceVarPtr g, unsigned char **p_i_ptr)
{

    if (g->kind[**p_i_ptr]&CONSON)
    	{
		(*p_i_ptr) += g->e_direction;       /* increment the pointer	*/
        return(0);
    	}
    if (g->e_direction == (-1))
    	{
        if (**p_i_ptr == 'U')
            if (*(*p_i_ptr-1) == 'G' || *(*p_i_ptr-1) == 'Q')
            	{
                (*p_i_ptr)-=2;
                return(0);      /* get the QU or GU, and adjust the pointer	*/
            	}
    	}
    else    /* g->e_direction == 1 -  WE'LL NEVER GET HERE!	*/
    	{
        if (**p_i_ptr == 'Q' || **p_i_ptr == 'G')
            if (*(*p_i_ptr+1) == 'U')
            	{
                (*p_i_ptr)+=2;
                return(0);
            	}
    	}
    return(-1);
}


/***************************************************************************
  FUNCTION:     FindSibilant
  DESCRIPTION:  macro rule - searches input string for 1 sibalant consonant.
                CH and SH are in the list of consonants.
  RETURNS:      0 - good match
                != 0  missed match
  PARAMETERS:   pointer to the pointer to the input string
  SIDE EFFECTS: .
  REVISIONS:    4/6/90  -jf-
****************************************************************************/
short	FindSibilant (voiceVarPtr g, unsigned char **p_i_ptr)
{
    if (g->kind[**p_i_ptr]&HISCON)
    	{
		(*p_i_ptr) += g->e_direction;       /* increment the pointer	*/
        return(0);
    	}
    if (g->e_direction == 1)     /* I DON'T BELEIVE THAT WE EVER GET HERE EITHER.	*/
    	{
        if (**p_i_ptr == 'C' || **p_i_ptr == 'S')
            if (*(*p_i_ptr+1) == 'H')
            {
                (*p_i_ptr)+=2;
                return(0);
            }
    }
    else    /* g->e_direction == -1	*/
    {
        if (**p_i_ptr == 'H')
            if (*(*p_i_ptr-1) == 'C' || *(*p_i_ptr-1) == 'S')
            {
                (*p_i_ptr)-=2;
                return(0);
            }
    }
    return(-1);
}



/***************************************************************************
  FUNCTION:     FindVowel
  DESCRIPTION:  macro rule - searches input string for 1 vowel
                this could be a macro, and it would run faster.
  RETURNS:      0 - good match
                 != 0  missed match
  PARAMETERS:   pointer to the pointer to the input string
  SIDE EFFECTS: .
  REVISIONS:    4/6/90  -jf-
****************************************************************************/
short	FindVowel (voiceVarPtr g, unsigned char **p_i_ptr)
{
    if (g->kind[**p_i_ptr]&VOWEL)
    	{
		(*p_i_ptr) += g->e_direction;       /* increment the pointer	*/
        return(0);
    	}
    return(-1);
}


/***************************************************************************
  FUNCTION:     search_special
  DESCRIPTION:  searches a special rule table, compares to input string
                Returns the result of the search. Using comma's is indeed
                slightly inefficient. This function is not very high on the
                most-called though.
  RETURNS:      0 - good match
                 != 0 - no match
  PARAMETERS:   pointer to the pointer to the input string
                pointer to the beginning of the rule table to search
  SIDE EFFECTS: .
  REVISIONS:    4/6/90-jf-
****************************************************************************/
short	search_special (voiceVarPtr g, unsigned char **p_i_ptr, unsigned char *sprule)
{
    unsigned char	*i_ptr;

    i_ptr=*p_i_ptr;     /* set up local pointer	*/
    while (*sprule != 0)      /*0 means end-of-rule string	*/
    {
        if (*i_ptr != *sprule)    /* this be a missed match	*/
        {
            while (*sprule != ',')    /* scan up to a delimiter	*/
                sprule++;
            sprule++;               /* und past the delimiter.	*/
            i_ptr=*p_i_ptr;
            continue;               /* do next rule.	*/
        }
        i_ptr += g->e_direction;
        sprule++;
        if (*sprule == ',')           /* good find	*/
        {
            *p_i_ptr=i_ptr;         /* update the caller's pointer	*/
            return(0);
        }
    }
    return(-1);
}


/***************************************************************************
  FUNCTION:     exclaim_rule
  DESCRIPTION:  does the exclaimation point rule- only works for the
                forward e_direction. Since there are no ! rules in the
                table, this is useless code.
  RETURNS:      0 - good match,
                !0 - no match
  PARAMETERS:   pointer to the pointer to the input string
  SIDE EFFECTS: .
  REVISIONS:    4/10/90 -jf-

****************************************************************************/

#if 0
short	exclaim_rule(unsigned char ** p_i_ptr)
{

    if (**p_i_ptr == 'S' && *(*p_i_ptr+1) <'A') return(0);
    if (**p_i_ptr == 'L')
    {
        if ( *(*p_i_ptr+1) == 'Y' && *(*p_i_ptr+2) <'A') return(0);
    }
    /* not S? or LY?, how about MENT or NESS	*/
    if (strncmp(*p_i_ptr,"MENT",4))
        if (strncmp(*p_i_ptr,"NESS",4))
            return(-1);
    return(0);  /* if we match either of the two strings, return this.	*/
                /* remember campers, strncmp() is FALSE for a match.	*/
}
#endif

/***************************************************************************
  FUNCTION:     dorule
  DESCRIPTION:  scans out one side of a rule, returns the result.
                The bigass switch statement has been ordered in order of
                average usage (* is the most used special character). If
                optimizing, start at the top of this switch and work down.
                Note that the *,:,v, and # rules are recursive.

  RETURNS:      rule pointer - points to the end of the rule that we have
                just scanned past, including the delimiter.
                NULL if we got a missed match.
  PARAMETERS:   pointer to the start of char string to match.
                pointer to the rule table
  SIDE EFFECTS: e_direction better be valid before the call.
                this function is recursive.
  REVISIONS:    4/5/90  -jf-
****************************************************************************/

unsigned char	*dorule (voiceVarPtr g, unsigned char *i_ptr, unsigned char *r_ptr)
{	
    unsigned char	*r_local;

    if (*r_ptr == DELIMITER) return(r_ptr+1);
    while (*r_ptr != DELIMITER)
    	{
        if (g->kind[*r_ptr] == SPCHAR)     /* special character	*/
        	{
            switch (*r_ptr)
            	{
                case '*':           /* one or more consomamt rule	*/
                    if (FindConsonant(g, &i_ptr)) return(NULL);  /* we did not have any consonants	*/
                    while (g->kind[*i_ptr] & CONSON)     /* next char is a consonant	*/
                    	{
                        /* recurse and scan.	*/
                        if ( (r_local = dorule (g, i_ptr, r_ptr+1) ) != NULL)
                            return(r_local);
                        /* else we did NOT find a match, so do the next input char	*/
                        i_ptr += g->e_direction;
                    	}
                    break;
                case '$':           /* one vowel	*/
                    if (FindVowel(g, &i_ptr)) return(NULL);
                    break;
                case '^':           /* one consonant.	*/
                    if (FindConsonant(g, &i_ptr)) return(NULL);
                    break;
                case ':':           /* 0 or more consonants	*/
                    FindConsonant(g, &i_ptr);
                    while (g->kind[*i_ptr]&CONSON)     /* next char is a consonant	*/
                    	{
                        if ((r_local=dorule(g, i_ptr,r_ptr+1)) != NULL)
                            return(r_local);
                        /* else we did NOT find a match, so do the next input char	*/
                        i_ptr += g->e_direction;
                    	}
                    break;
                case '+':           /* front vowel	*/
                    if (!(g->kind[*i_ptr] & FRONT)) return(NULL);  /* no match	*/
                    i_ptr += g->e_direction;
                    break;
                case 'v':           /* 0 or more vowels	*/
                    FindVowel(g, &i_ptr);
                    while (g->kind[*i_ptr]&VOWEL)
                    	{
                        if ((r_local=dorule(g, i_ptr,r_ptr+1)) != NULL)
                            return(r_local);
                        /* else we did NOT find a match, so do the next input char	*/
                        i_ptr += g->e_direction;
                    	}
                    break;
                case 'l':
                    if (search_special(g, &i_ptr,g->lruletab))
                        return(NULL);
                    break;
                case '-':
                    if (search_special(g, &i_ptr,g->dashruletab))
                        return(NULL);
                    break;
                case '%':
                    if (search_special(g, &i_ptr,g->percentruletab))
                        return(NULL);
                    break;
                case 'z':
                    if (search_special(g, &i_ptr,g->zruletab))
                        return(NULL);
                    break;
                case 'b':
                    if (search_special(g, &i_ptr,g->bruletab))
                        return(NULL);
                    break;
                case '#':           /* one or more vowels	*/
                    if (FindVowel(g, &i_ptr)) return(NULL);
                    while (g->kind[*i_ptr]&VOWEL)
                    	{
                        if ((r_local=dorule(g, i_ptr,r_ptr+1)) != NULL)
                            return(r_local);
                        /* else we did NOT find a match, so do the next input char	*/
                        i_ptr += g->e_direction;
                    	}
                    break;
                case '.':           /* voiced consonant.	*/
                    if (!(g->kind[*i_ptr] & VOICON)) return(NULL);  /* no match	*/
                    i_ptr += g->e_direction;
                    break;
                case '&':           /* Sibilant consonant.	*/
                    if (FindSibilant(g, &i_ptr)) return(NULL);
                    break;
                case '@':
                    if (search_special(g, &i_ptr,g->atruletab))
                        return(NULL);
                    break;
                case 'm':
                    if (search_special(g, &i_ptr,g->mruletab))
                        return(NULL);
                    break;
                /*case '!':	*/
                /*    if (exclaim_rule(&i_ptr)) return(NULL);	*/
                /*    break;	*/

            	} /* end of switch	*/
            r_ptr++;        /* if we get here, we matched the rule	*/

        	}       /* end of if special char	*/
        else    /* normal character.	*/
        	{
            if (*i_ptr != *r_ptr++) return(NULL);
            i_ptr += g->e_direction;
        	}
    	}   						/* end of while ( != delimiter) loop.	*/
    return (r_ptr +1);
}
