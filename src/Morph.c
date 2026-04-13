#ifndef __SPEECHSAY__
	#include "MT4.h"
#endif


extern short	 SearchAllDicts ( voiceVarPtr vv, unsigned char* text, FETokenPtr tok, DictPtr mainDict, short saveIt ); /* routine to search for word in app dioctionaries and a main dictionary	*/



// LOCAL PROTOTYPES

short	DoMorph (voiceVarPtr vv, FETokenPtr tok);
void	ResolvePOS (voiceVarPtr vv);
void	SetPOS_FromSuffix (FETokenPtr tok);
void	PlacePhrasing (voiceVarPtr vv);




void	PlacePhrasing (voiceVarPtr vv)
{

	short      			initial_Adv;
	short      			inParen;
	short      			YesNo_Phrase;
	short        		word_Count;
	short		 		prev_POS;
	unsigned short		cur_POS;
	short				ambig_POS;
	short				ambig1_POS;
	short				det_Flag;
	short				next_POS;
	short				next_Punct;
	short				next2_POS;
	short				next2_Punct;
	short				phonOut;
	short				next3_POS;
	short				next3_Punct;
	short				short_Sent;
	short				WH_maybe;
	FETokenPtr			cur_Tok, next_Tok, next2_Tok, next3_Tok, prev_Tok;
	FETokenType			cur_TokType;
	short				cur_Bnd, punct;
	
	cur_Bnd = kBND_None;
	prev_POS = kUndefPOS;
	phonOut = false;
	ambig1_POS = false;
	YesNo_Phrase = true;
	WH_maybe = false;
	det_Flag = false;
	
	word_Count = 0;
	inParen = false;
	initial_Adv = false;
	
	if (vv->LastTok <= 9) 
		short_Sent = true;
	else
		short_Sent = false;
		
	
	//------------------------------------------------------------------------
	// Step through sentence, looking for next relevant item
	//------------------------------------------------------------------------
	for (vv->CurTok = 1; vv->CurTok < vv->LastTok; vv->CurTok++)
		{
		prev_Tok = &vv->tokBuffer[vv->CurTok-1];
		cur_Tok = &vv->tokBuffer[vv->CurTok];
		next_Tok = &vv->tokBuffer[vv->CurTok+1];
		next2_Tok = &vv->tokBuffer[vv->CurTok+2];
		next3_Tok = &vv->tokBuffer[vv->CurTok+3];

		
		next_Punct = false;
		next_POS = kUndefPOS;
		next2_Punct = false;
		next2_POS = kUndefPOS;
		next3_Punct = false;
		next3_POS = kUndefPOS;
		
		//------------------------------------
		// Look ahead to next position
		//------------------------------------
		next_POS = next_Tok->POSchoice;
		if (vv->CurTok == vv->LastTok-1)
			next_Punct = true;
			
		//------------------------------------
		// Look ahead 2 positions
		//------------------------------------
		if (vv->CurTok < vv->LastTok-2)
			{
			next2_POS = next2_Tok->POSchoice;
			if (vv->CurTok == vv->LastTok-2)
				next2_Punct = true;
			}
		
		//------------------------------------
		// Look ahead 3 positions
		//------------------------------------
		if (vv->CurTok < vv->LastTok-3)
			{
			next3_POS = next3_Tok->POSchoice;
			if (vv->CurTok == vv->LastTok-3)
				next3_Punct = true;
			}
			
		cur_POS = cur_Tok->POSchoice;
		cur_TokType = cur_Tok->tokType;
		if ( (cur_Tok->POScount1 + cur_Tok->POScount2) > 1 )
			ambig_POS = true;
		else
			ambig_POS = false;
		
		//------------------------------------
		// Test for parenthesized clause
		//------------------------------------
		if (cur_TokType == kPuncTok)
			{
			//Debugger();
			}
		else
		//=-=-=-=-=-=-=-=-=-=-=
		// Process a word
		//=-=-=-=-=-=-=-=-=-=-=
			{
		
			//------------------------------------------------------------------------
			// If next element is punctuation, don't bother to place minor boundaries
			//------------------------------------------------------------------------			
			if ((next_Punct == false) && (!inParen))
				{
				cur_Bnd = kBND_None;
				
				//------------------------------------------------------------------------
				// Test whether the clause could turn out to be a yes/no question
				//------------------------------------------------------------------------
				if ( (vv->CurTok == 1) && (cur_POS == kInterr) ) 
					YesNo_Phrase = false;
				if ( (vv->CurTok == 1) && ((cur_POS == kPrep) || (cur_POS == kConj)) ) 
					WH_maybe = true;
				if ( (vv->CurTok == 2) && (WH_maybe) && ((cur_POS == kInterr) || (cur_POS == kRelPro))) 
					YesNo_Phrase = false;
		
				
				
				//------------------------------------------------------------------------
				// SEP1: Insert boundary after sentence-initial adverb
				//------------------------------------------------------------------------
				if (initial_Adv)
					{
					cur_Bnd = kBND_Sep1;
					initial_Adv = false;
					goto GOT_BND;
					}
					
				if ( (prev_POS == kUndefPOS) && (cur_POS == kAdv) && 
					 ((next_POS == kArt) || (next_POS == kDet)) ) 
					initial_Adv = true;
				else 
					initial_Adv = false;
				
				
				
				//------------------------------------------------------------------------
				// SEP2:Insert boundary before coordinating conjunctions
				//------------------------------------------------------------------------
	 			if ( ((prev_POS != kUndefPOS) && (cur_POS == kCConj) && 
					 (det_Flag == false) && (word_Count > 3) && (next2_POS != kConj)) ||
						
					 ((cur_POS == kAdv) && (word_Count > 4) && (next_POS != kAdj)) ||
					 
					 ((prev_POS == kObjPron) && (word_Count > 2)) ||
					
					 ( ((cur_POS == kSubjPron) || (cur_POS == kContr) ) && 
					 (word_Count > 3) && (prev_POS != kRelPro) && (prev_POS != kConj)) ||
					 
					( (cur_POS == kInterr) && (word_Count > 4) )  )
					{
					cur_Bnd = kBND_Sep2;
					goto GOT_BND;
					}
		
				
				
				//------------------------------------------------------------------------
				// SEP3:Insert boundary after subject noun phrase cued by aux verb following
				//------------------------------------------------------------------------
				if ( ((word_Count > 2) && 
					 ( ((prev_POS == kNoun) || (prev_POS == kVerb)) && 
					   (prev_POS != kVaux) && (prev_POS != kRVaux)) && 
					 ((cur_POS == kVaux) || (cur_POS == kRVaux)) ) ||
			
					((prev_POS == kNoun) && ((next_POS != kRelPro) && 
					(next_POS != kVaux) && (next_POS != kRVaux) && 
					(next2_POS != kVaux) && (next2_POS != kRVaux)) && 
					(word_Count > 4) && 
					((cur_POS == kVaux) || (cur_POS == kRVaux))) ||
				
					((prev_POS == kNoun) && (next_POS != kRelPro) && 
					(ambig1_POS) && (next_POS != kRVaux)  && (next_POS != kConj) &&  
					(next_POS != kCConj) && (word_Count > 3)  && (cur_POS == kVerb)) ||
					
					((prev_POS == kNoun) && (cur_POS != kRelPro) && 
					(cur_POS != kRVaux) && (cur_POS != kInf) && (cur_POS != kCConj) && 
					(cur_POS != kConj) && (word_Count > 2) && (ambig1_POS) && 
					((word_Count > 2) || (short_Sent)) && (cur_POS == kVerb)))
					{
					cur_Bnd = kBND_Sep3;
					goto GOT_BND;
					}
		
				
				
				//------------------------------------------------------------------------
				// SEP4:Insert boundary before conjunction
				//------------------------------------------------------------------------
				if ( ((cur_POS == kConj) && (word_Count > 3) && 
					 (cur_POS != kInf) && (next_Punct == false) && 
					 (prev_POS != kConj) && (prev_POS != kCConj) &&
					 (next2_Punct == false)) ||
					 
					( (prev_POS == kVPart) && (cur_POS != kPrep) && 
					  (cur_POS != kDet) && (cur_POS != kArt) && 
					  (word_Count > 2) && 
					  ((cur_POS == kNoun) || (cur_POS == kNoun) || (cur_POS == kAdj))) ||
					  
					( (cur_POS == kInterr) && (word_Count > 2) && 
					  (cur_POS == kSubjPron)) ||
					  
					( (cur_POS == kInf) && (word_Count > 3) && 
					  (next_Punct == false) && (next2_Punct == false) && (next3_Punct == false))	)
					{
					cur_Bnd = kBND_Sep4;
					goto GOT_BND;
					}
		
				
				
				//------------------------------------------------------------------------
				// SEP5:Insert boundary before relative pronoun
				//------------------------------------------------------------------------
				if ( ( (cur_POS == kRelPro) && (word_Count >= 3)  && 
					   (prev_POS != kPrep) && (next3_POS != kVaux) && 
					   (next3_POS != kRVaux)  && 
					   ( (prev_POS == kNoun) || (prev_POS == kVerb) ) ) ||
					   
					 ( (cur_POS == kQuant) && (word_Count > 5) && 
					   (prev_POS != kAdj) && (prev_POS != kArt) && 
					   (prev_POS != kVaux) && (prev_POS != kRVaux) && 
					   (prev_POS != kDet) && (next2_POS != kCConj) && 
					   (next_Punct == false)))
					{
					cur_Bnd = kBND_Sep5;
					goto GOT_BND;
					}
		
				
				
				//------------------------------------------------------------------------
				// SEP6:Silverman87-style, content/function tone group boundaries. 
				// Does trivial sentence-final function word look-ahead check.
				//------------------------------------------------------------------------
				if ( ( (prev_POS == kNoun) || (prev_POS == kVerb) || (prev_POS == kAdj) || (prev_POS == kAdv)) 
					&& ((cur_POS != kNoun) && (cur_POS != kVerb) && (cur_POS != kAdj) && (cur_POS != kAdv))
					&& (next_Punct == false)) 
					{
					cur_Bnd = kBND_Sep6;
					//goto GOT_BND;
					}
					
			
			
				//------------------------------------------------------------------------
				// If phrasing was generated, save it
				//------------------------------------------------------------------------
GOT_BND:		if ( (cur_Bnd != kBND_None) && !(prev_Tok->add_BND) && !(cur_Tok->add_BND) )
					{
					cur_Tok->phrasingBND = cur_Bnd;
					cur_Tok->add_BND = true;
					}
				}

			if (cur_Tok->phonStr[0])
				phonOut = true;				// phon string has started
		
			prev_POS = cur_POS;
			
			//------------------------------------------------------------------------
			// Keep track of when determiners encountered to help in deciding 
			// when to allow a strong 'and' boundary (SEP2)
			//------------------------------------------------------------------------
			if (word_Count > 1) 
				det_Flag = false;
			if ((cur_POS == kArt) || (cur_POS == kDet)) 
				det_Flag = true;
				
			ambig1_POS = ambig_POS;
			}
		word_Count++;
		}
	
	//------------------------------------------------------------------------
	// Process sentence punctuation
	//------------------------------------------------------------------------
	cur_Tok = &vv->tokBuffer[vv->LastTok];
	
	if (cur_Tok->tokType == kPuncTok)
		{
		punct = cur_Tok->phonStr[1];
		
		if (punct == _Comma_)
			{
			cur_Bnd = kBND_Pause;
			}
	
		else if (punct == _Exclam_)
			{
			cur_Bnd = kBND_Emph;
			}
	
		else if (punct == _Quest_)
			{
			//------------------------------------
			// Test for yes/no question clause
			//------------------------------------
			if (YesNo_Phrase)
				{
				//------------------------------------------
				// Put out a final yes/no question marker
				//------------------------------------------
				cur_Bnd = kBND_Quest;
				}
			else 
				{
			
				//------------------------------------------------------------------------
				// Put out a final declarative clause marker (for WH questions)
				//------------------------------------------------------------------------
				cur_Tok->phonStr[1] = _Period_;
				cur_Bnd = kBND_Decl;
				}
			}
		else
			//------------------------------------------------------------------------
			// Put out a final declarative clause marker
			//------------------------------------------------------------------------
			{
			cur_Tok->phonStr[1] = _Period_;
			cur_Bnd = kBND_Decl;
			}
		}
}



void	ResolvePOS (voiceVarPtr vv)
{
	short			cur_noun, next_noun;
	short			cur_verb, next_verb;
	short			cur_adj, next_adj;
	short			cur_adv, next_adv;
	short			cur_prep, next_prep;
	short			cur_ppron, next_ppron;
	short			cur_relpro, next_relpro;
	short			cur_dpron, next_dpron;
	short			cur_ipron, next_ipron;
	short			cur_vaux, next_vaux;
	short			cur_rvaux, next_rvaux;
	short			cur_interj, next_interj;
	short			cur_conj, next_conj;
	short			cur_cconj, next_cconj;
	short			cur_interr, next_interr;
	short			cur_art, next_art;
	short			cur_det, next_det;
	short			cur_inf, next_inf;
	short			cur_gen, next_gen;
	short			cur_contr, next_contr;
	short			cur_vpart, next_vpart;
	short			cur_quant, next_quant;
	short			cur_subjpron, next_subjpron;
	short			cur_objpron, next_objpron;
	short			both_Verbs;
	
	unsigned long	compPOS;
	short			POS_Choice;
	short			alt_Choice;
	short			prev_POS;
	short			prev2_POS;
	FETokenPtr		cur_Tok, next_Tok;
	short			det_Count;
	short			first_Aux;
	short			next_punc, j;


	prev_POS = kUndefPOS;
	prev2_POS = kUndefPOS;
	first_Aux = false;
	det_Count = 100;

	for (vv->CurTok = 1; vv->CurTok < vv->LastTok; vv->CurTok++)
		{
		cur_Tok = &vv->tokBuffer[vv->CurTok];
		next_Tok = &vv->tokBuffer[vv->CurTok+1];
		
		POS_Choice = kUndefPOS;
		alt_Choice = kUndefPOS;
		
		
		if ( ((cur_Tok->POScount1 + cur_Tok->POScount2) == 1) && 
			 !(cur_Tok->compPOS1 & kHas_Prep) &&
			 !(cur_Tok->compPOS1 & kHas_CConj) &&
			 !(cur_Tok->compPOS1 & kHas_PPron) )
			{
			POS_Choice = cur_Tok->POScode1[0];
			}
		else
			{
			if (vv->CurTok+1 == vv->LastTok)
				next_punc = true;
			else
				next_punc = false;
			
			if ( (cur_Tok->compPOS1 & kHas_Verb) && (cur_Tok->compPOS2 & kHas_Verb) )
				both_Verbs = true;
			else
				both_Verbs = false;
	
			compPOS = cur_Tok->compPOS1 | cur_Tok->compPOS2;
	
			cur_noun = false;
			cur_verb = false;
			cur_adj = false;
			cur_adv = false;
			cur_prep = false;
			cur_ppron = false;
			cur_relpro = false;
			cur_dpron = false;
			cur_ipron = false;
			cur_vaux = false;
			cur_rvaux = false;
			cur_interj = false;
			cur_conj = false;
			cur_cconj = false;
			cur_interr = false;
			cur_art = false;
			cur_det = false;
			cur_inf = false;
			cur_gen = false;
			cur_contr = false;
			cur_vpart = false;
			cur_quant = false;
			cur_subjpron = false;
			cur_objpron = false;
	
			if (compPOS & kHas_Noun)	cur_noun = true;
			if (compPOS & kHas_Verb)	cur_verb = true;
			if (compPOS & kHas_Adj)		cur_adj = true;
			if (compPOS & kHas_Adv)		cur_adv = true;
			if (compPOS & kHas_Prep)	cur_prep = true;
			if (compPOS & kHas_PPron)	cur_ppron = true;
			if (compPOS & kHas_RelPro)	cur_relpro = true;
			if (compPOS & kHas_DPron)	cur_dpron = true;
			if (compPOS & kHas_IPron)	cur_ipron = true;
			if (compPOS & kHas_Vaux)	cur_vaux = true;
			if (compPOS & kHas_RVaux)	cur_rvaux = true;
			if (compPOS & kHas_Interj)	cur_interj = true;
			if (compPOS & kHas_Conj)	cur_conj = true;
			if (compPOS & kHas_CConj)	cur_cconj = true;
			if (compPOS & kHas_Interr)	cur_interr = true;
			if (compPOS & kHas_Art)		cur_art = true;
			if (compPOS & kHas_Det)		cur_det = true;
			if (compPOS & kHas_Inf)		cur_inf = true;
			if (compPOS & kHas_Gen)		cur_gen = true;
			if (compPOS & kHas_Contr)	cur_contr = true;
			if (compPOS & kHas_Quant)	cur_quant = true;
			if (compPOS & kHas_VPart)	cur_vpart = true;
			if (compPOS & kHas_SubjPron) cur_subjpron = true;
			if (compPOS & kHas_ObjPron)	cur_objpron = true;
	
	
			compPOS = next_Tok->compPOS1 | next_Tok->compPOS2;
	
			next_noun = false;
			next_verb = false;
			next_adj = false;
			next_adv = false;
			next_prep = false;
			next_ppron = false;
			next_relpro = false;
			next_dpron = false;
			next_ipron = false;
			next_vaux = false;
			next_rvaux = false;
			next_interj = false;
			next_conj = false;
			next_cconj = false;
			next_interr = false;
			next_art = false;
			next_det = false;
			next_inf = false;
			next_gen = false;
			next_contr = false;
			next_vpart = false;
			next_quant = false;
			next_subjpron = false;
			next_objpron = false;
	
			if (compPOS & kHas_Noun)	next_noun = true;
			if (compPOS & kHas_Verb)	next_verb = true;
			if (compPOS & kHas_Adj)		next_adj = true;
			if (compPOS & kHas_Adv)		next_adv = true;
			if (compPOS & kHas_Prep)	next_prep = true;
			if (compPOS & kHas_PPron)	next_ppron = true;
			if (compPOS & kHas_RelPro)	next_relpro = true;
			if (compPOS & kHas_DPron)	next_dpron = true;
			if (compPOS & kHas_IPron)	next_ipron = true;
			if (compPOS & kHas_Vaux)	next_vaux = true;
			if (compPOS & kHas_RVaux)	next_rvaux = true;
			if (compPOS & kHas_Interj)	next_interj = true;
			if (compPOS & kHas_Conj)	next_conj = true;
			if (compPOS & kHas_CConj)	next_cconj = true;
			if (compPOS & kHas_Interr)	next_interr = true;
			if (compPOS & kHas_Art)		next_art = true;
			if (compPOS & kHas_Det)		next_det = true;
			if (compPOS & kHas_Inf)		next_inf = true;
			if (compPOS & kHas_Gen)		next_gen = true;
			if (compPOS & kHas_Contr)	next_contr = true;
			if (compPOS & kHas_Quant)	next_quant = true;
			if (compPOS & kHas_VPart)	next_vpart = true;
			if (compPOS & kHas_SubjPron) next_subjpron = true;
			if (compPOS & kHas_ObjPron)	next_objpron = true;
	

			//------------------------------------		
			// Distinguish "reed" vs. "red"
			//------------------------------------		
			if ( (cur_Tok->POScount1 == 1) && (cur_Tok->POScount2 == 1) &&
				 (both_Verbs) )
				{
				//------------------------------------		
				// Here are the "rIYd" cases
				//------------------------------------		
				if ( (prev_POS == kInf) || (first_Aux)  || 
					 (prev_POS == kUndefPOS) || (prev_POS == kContr) || 
					 (prev_POS == kVaux) || 
					 (prev_POS == kRVaux) )
					{
					alt_Choice = 1;
					}
				else
					{
					alt_Choice = 0;
					}
				}
						
			else
				{
				//--------------------------------------------------------------------------------------
				// If aux precedes and verb choice exists, POS is verb
				//		- could address
				//--------------------------------------------------------------------------------------
				if (((prev_POS == kVaux) || (prev_POS == kRVaux)) && (cur_verb)) 
					POS_Choice = kVerb;

				
				//--------------------------------------------------------------------------------------
				// Choose adjective following article
				//--------------------------------------------------------------------------------------
				else if ((cur_adj) && (!cur_noun) && ((prev_POS == kArt) || (prev_POS == kDet))) 
					POS_Choice = kAdj;
			
				//--------------------------------------------------------------------------------------
				// Choose verb following verbal aux
				//--------------------------------------------------------------------------------------
				else if (((cur_verb) && (!cur_rvaux) && (!cur_vaux)) && ((prev_POS == kVaux) || (prev_POS == kRVaux)) && (!next_punc)) 
					POS_Choice = kVerb;
			
				//--------------------------------------------------------------------------------------
				// Change 'that' to demonstrative if preceded by preposition or conjunction
				//--------------------------------------------------------------------------------------
				else if ( (cur_relpro) && (cur_conj) &&  					// "that"
						  (!next_ppron) && (!next_subjpron) && 
						  (!next_contr) && (prev_POS != kNoun) &&
						  ((prev_POS == kConj) || (next_punc) || (next_rvaux) || (next_vaux) || (prev_POS == kPrep)) )
					{
					POS_Choice = kDPron;
					cur_Tok->POScode1[0] = POS_Choice;
					}

				//--------------------------------------------------------------------------------------
				// Choose noun over verb if following dpron
				//--------------------------------------------------------------------------------------
				else if ( (prev_POS == kDPron) && (cur_verb) && (cur_noun) ) 
					POS_Choice = kNoun;
							
				//--------------------------------------------------------------------------------------
				// Choose adjective over verb if following raux
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kRVaux) && (cur_verb) && (cur_adj)) 
					POS_Choice = kAdj;
								
				//--------------------------------------------------------------------------------------
				// Coerce a reflexive pronoun to be a regular noun except when following a verb @@@@
				//--------------------------------------------------------------------------------------
				#if 0
				else if ((cur_ppron) && (prev_POS != kVerb) )
					{
					POS_Choice = kNoun;
					cur_Tok->POScode1[0] = POS_Choice;
					}
				#endif
				
				//--------------------------------------------------------------------------------------
				// Consider 'do' as main verb (for prominence) under certain conditions @@@@
				//--------------------------------------------------------------------------------------
				else if (((cur_verb) && (cur_rvaux)) && 
						((prev_POS == kVaux) || (prev_POS == kRVaux) || (prev_POS == kSubjPron) || (prev_POS == kIPron) || (prev_POS == kPPron) ||
						(prev_POS == kAdv) ||  (next_verb) || (next_punc) || (prev_POS == kInf)) &&
						(!next_ppron) && (!next_subjpron) && (!next_ipron) )
					{
				 	POS_Choice = kVerb;
					}
			
				//--------------------------------------------------------------------------------------
				// Take a verb particle choice following a verb
				//--------------------------------------------------------------------------------------
				else if ((cur_vpart) && ((next_cconj) || (next_conj) || (next_prep) || (next_inf))) 
					POS_Choice = kVPart;
				
				//--------------------------------------------------------------------------------------
				// Take a verb particle choice following a verb
				//--------------------------------------------------------------------------------------
				else if ((cur_vpart) && ((prev_POS == kPPron) && (next_punc))) 
					POS_Choice = kVPart;
				
				//--------------------------------------------------------------------------------------
				// Take a verb particle choice following a verb  @@@@
				//--------------------------------------------------------------------------------------
				else if ( (cur_vpart) && (prev_POS == kVerb) )  
					POS_Choice = kVPart;

				//--------------------------------------------------------------------------------------
				// Take a verb particle choice following a verb + pron. This is a special case, in that
				// we are forcing a POS choice that is not explicitly listed in the lexical entry.
				//--------------------------------------------------------------------------------------
				else if ( (cur_vpart) && (prev2_POS == kVerb)
						&& ((prev_POS == kObjPron) || (prev_POS == kIPron)
						|| (prev_POS == kDPron) || (prev_POS == kPPron)))
					{
					POS_Choice = kVPart;
					}

				//--------------------------------------------------------------------------------------
				// Take a verb particle choice following a verb. This is a special case, in that
				// we are forcing a POS choice that is not explicitly listed in the lexical entry.
				//--------------------------------------------------------------------------------------
				else if (((cur_prep) && (!cur_inf)) && ((prev_POS == kVerb) &&  (next_punc)))
					{
					POS_Choice = kVPart;
					cur_Tok->POScode1[0] = POS_Choice;
					}
				
				//--------------------------------------------------------------------------------------
				// Take a verb particle choice following a verb + pron. This is a special case, in that
				// we are forcing a POS choice that is not explicitly listed in the lexical entry.
				//--------------------------------------------------------------------------------------
				else if (((cur_prep) && (!cur_inf)) && (prev2_POS == kVerb) &&
						((prev_POS == kObjPron) || (prev_POS == kIPron)
						|| (prev_POS == kDPron) || (prev_POS == kPPron)))
					{
					POS_Choice = kVPart;
					cur_Tok->POScode1[0] = POS_Choice;
					}
				
				//--------------------------------------------------------------------------------------
				// Take a verb particle choice before an aux verb					
				//--------------------------------------------------------------------------------------
				else if ((cur_vpart) && ((next_rvaux) || (next_vaux))) 
					POS_Choice = kVPart;
			
				//--------------------------------------------------------------------------------------
				// Take a preposition choice from verb particle or preposition				
				//--------------------------------------------------------------------------------------
				else if ((cur_vpart) && (cur_prep) && (cur_adj)) 
					POS_Choice = kPrep;
			
				//--------------------------------------------------------------------------------------
				// Take a preposition choice from 'to'					
				//--------------------------------------------------------------------------------------
				else if ((next_punc) && (cur_prep)) 
					POS_Choice = kPrep;
			
				//--------------------------------------------------------------------------------------
				// If coordinating conjunction choice exists, take that choice
				//--------------------------------------------------------------------------------------
				else if (cur_cconj) 
					POS_Choice = kCConj;
			
				//--------------------------------------------------------------------------------------
				// If nothing precedes and interr choice exists, POS is interr
				//--------------------------------------------------------------------------------------
				else if (((prev_POS == kUndefPOS) || (prev_POS == kVerb)) && (cur_interr)) 
					POS_Choice = kInterr;
			
				//--------------------------------------------------------------------------------------
				// If something precedes and relpro choice exists, POS is relpro
				//--------------------------------------------------------------------------------------
				else if ((prev_POS != kUndefPOS) && (cur_interr) && (cur_relpro)) 
					POS_Choice = kRelPro;
			
				//--------------------------------------------------------------------------------------
				// If conjunction choice exists, take that choice
				//--------------------------------------------------------------------------------------
				else if ((cur_conj) && (cur_adv) && ((next_rvaux) || (next_vaux))) 
					POS_Choice = kAdv;
						
				//--------------------------------------------------------------------------------------
				// If conjunction choice exists, take that choice
				//--------------------------------------------------------------------------------------
				else if ((cur_conj) && (!next_punc)) 
					POS_Choice = kConj;
			
				//--------------------------------------------------------------------------------------
				// If 'can' or 'will', take as aux if another aux ('be') follows
				//--------------------------------------------------------------------------------------
				else if (((cur_noun) && ((cur_vaux) || (cur_rvaux))) &&
					((next_rvaux) || (next_vaux) || (prev_POS == kUndefPOS) || (prev_POS == kNoun) || (next_verb))) 
					{
					if (cur_vaux) 
						POS_Choice = kVaux;
					else 
						POS_Choice = kRVaux;
					}
						
				//--------------------------------------------------------------------------------------
				// If prep choice exists and adj choice exists, and det or art precedes, choice is adj
				//--------------------------------------------------------------------------------------
				else if ((cur_prep) && (cur_adj) && ((prev_POS == kArt) || (prev_POS == kDet))) 
					POS_Choice = kPrep;
			
				//--------------------------------------------------------------------------------------
				// If prep choice exists and det or art follows, choice is prep
				//--------------------------------------------------------------------------------------
				else if ((cur_prep) && ((next_art) || (next_det))) 
					POS_Choice = kPrep;
			
				//--------------------------------------------------------------------------------------
				// If  verb choice exists, and lka is det/art, POS is verb
				//--------------------------------------------------------------------------------------
				else if ((cur_verb) && ((next_det) || (next_art))) 
					POS_Choice = kVerb;
			
				//--------------------------------------------------------------------------------------
				// If  noun choice exists, and det_Count < 2, POS is noun
				//--------------------------------------------------------------------------------------
				else if ((cur_noun) && (det_Count < 2)) 
					POS_Choice = kNoun;

				//--------------------------------------------------------------------------------------
				// If noun choice exists after quant, take noun choice
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kQuant) && (cur_noun)) 
					POS_Choice = kNoun;
				
				//--------------------------------------------------------------------------------------
				// Take an adverb choice at end of sentence
				//--------------------------------------------------------------------------------------
				else if ((cur_adv) && (next_punc)) 
					POS_Choice = kAdv;
			
				//--------------------------------------------------------------------------------------
				// If aux choice exists after subj pron, take aux choice
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kSubjPron) && (cur_rvaux)) 
					POS_Choice = kRVaux;
				else if ((prev_POS == kSubjPron) && (cur_vaux)) 
					POS_Choice = kVaux;
			
				//--------------------------------------------------------------------------------------
				// If noun choice exists before subjpron, take noun choice
				//--------------------------------------------------------------------------------------
				else if ((next_subjpron) && (cur_noun)) 
					POS_Choice = kNoun;
			
				//--------------------------------------------------------------------------------------
				// If noun choice exists and verbal aux follows, take noun choice
				//--------------------------------------------------------------------------------------
				else if ((cur_noun) && ((next_rvaux) || (next_vaux))) 
					POS_Choice = kNoun;
			
				//--------------------------------------------------------------------------------------
				// If infinitive marker precedes and verb choice exists, POS is verb
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kInf) && ((cur_verb) && (!cur_rvaux) && (!cur_vaux))) 
					POS_Choice = kVerb;
			
				//--------------------------------------------------------------------------------------
				// If primary verb, take as aux always (this will sometimes be wrong)
				//--------------------------------------------------------------------------------------
				else if ((cur_vaux) || (cur_rvaux))
				{
					if (cur_vaux) 
						POS_Choice = kVaux;
					else 
						POS_Choice = kRVaux;
				}
				
				//--------------------------------------------------------------------------------------
				// If inf marker choice exists, POS is inf marker unless det or article follows
				//--------------------------------------------------------------------------------------
				else if ((cur_inf) && ((next_det == false) && (next_art == false) && (next_ipron == false))) 
					POS_Choice = kInf;
			
				//--------------------------------------------------------------------------------------
				// If inf marker choice exists otherwise, consider it a true infinitive marker
				//--------------------------------------------------------------------------------------
				else if ((cur_inf) && (cur_prep)) 
					POS_Choice = kPrep;
			
				//--------------------------------------------------------------------------------------
				// If nothing precedes and adjective choice exists, POS is adjective
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kUndefPOS) && (cur_adj)) 
					POS_Choice = kAdj;
			
				//--------------------------------------------------------------------------------------
				// If something precedes and relpro choice exists, POS is relpro
				//--------------------------------------------------------------------------------------
				else if ((prev_POS != kUndefPOS) && (cur_relpro)) 
					POS_Choice = kRelPro;

				//--------------------------------------------------------------------------------------
				// If det precedes and adjective choice exists, POS is adjective
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kDet) && (cur_adj)) 
					POS_Choice = kAdj;
			
				//--------------------------------------------------------------------------------------
				// If preposition precedes and adjective choice exists, POS is adjective
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kPrep) && (cur_adj)) 
					POS_Choice = kAdj;

				//--------------------------------------------------------------------------------------
				// If determiner precedes and noun choice exists, POS is noun
				//--------------------------------------------------------------------------------------
				else if (((prev_POS == kDet) || (prev_POS == kArt)) && (cur_noun)) 
					POS_Choice = kNoun;
			
				//--------------------------------------------------------------------------------------
				// If article precedes and adj choice exists, POS is adj
				//--------------------------------------------------------------------------------------
				else if (((prev_POS == kDet) || (prev_POS == kArt)) && (cur_adj)) 
					POS_Choice = kAdj;
			
				//--------------------------------------------------------------------------------------
				// If adj precedes and noun choice exists, POS is noun
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kAdj) && (cur_noun)) 
					POS_Choice = kNoun;

				//--------------------------------------------------------------------------------------
				// If preposition precedes and noun choice exists, POS is noun
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kPrep) && (cur_noun)) 
					POS_Choice = kNoun;

				//--------------------------------------------------------------------------------------
				// Take an verb choice at end of sentence
				//--------------------------------------------------------------------------------------
				else if ((cur_verb) && (prev_POS == kSubjPron)) 
					POS_Choice = kVerb;
			
				//--------------------------------------------------------------------------------------
				// If personal pronoun precedes and verb choice exists, POS is verb
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kPPron) && (cur_verb)) 
					POS_Choice = kVerb;

				//--------------------------------------------------------------------------------------
				// If adv precedes and verb choice exists, POS is verb
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kAdv) && (cur_verb)) 
					POS_Choice = kVerb;

				//--------------------------------------------------------------------------------------
				// If adj precedes and noun choice follows, POS is adj
				//--------------------------------------------------------------------------------------
				else if ((prev_POS == kAdj) && (cur_adj) && (next_noun)) 
					POS_Choice = kAdj;
			
				//--------------------------------------------------------------------------------------
				// If noun choice and verb choice exists and previous is conj, take noun
				//--------------------------------------------------------------------------------------
				else if ((cur_verb) && (cur_noun) && (prev_POS == kConj)) 
					POS_Choice = kNoun;
				
				//--------------------------------------------------------------------------------------
				// If noun choice and verb choice exists and next can only be verb, take noun
				//--------------------------------------------------------------------------------------
				else if ((cur_verb) && (cur_noun) && (next_verb) && (next_noun == false) && (next_adj == false)) 
					POS_Choice = kNoun;
			
				//--------------------------------------------------------------------------------------
				// If noun choice and verb choice exists and previous is verb, take noun
				//--------------------------------------------------------------------------------------
				else if ((cur_verb) && (cur_noun) && (prev_POS == kVerb) && (!next_punc)) 
					POS_Choice = kNoun;
			
				//--------------------------------------------------------------------------------------
				// If verb and adj choice exists and previous is conj, take adj
				//--------------------------------------------------------------------------------------
				else if ((cur_verb) && (cur_adj) && ((prev_POS == kConj) || (prev_POS == kVerb))) 
					POS_Choice = kAdj;
				
				//--------------------------------------------------------------------------------------
				// If noun choice and verb choice exists and previous is noun, take verb
				//--------------------------------------------------------------------------------------
				else if ((cur_verb) && (cur_noun) && (prev_POS == kNoun) && (!next_punc)) 
					POS_Choice = kVerb;
			
				//--------------------------------------------------------------------------------------
				// If noun choice and verb choice exists and previous is noun, take noun
				//--------------------------------------------------------------------------------------
				else if ((cur_verb) && (cur_noun) && (prev_POS == kNoun)) 
					POS_Choice = kNoun;
			
				//--------------------------------------------------------------------------------------
				// Default if verb choice exists is verb
				//--------------------------------------------------------------------------------------
				else if ((cur_verb) && ((cur_rvaux == false) && (cur_vaux == false))) 
					POS_Choice = kVerb;
			
				//--------------------------------------------------------------------------------------
				// Default if noun choice exists is noun
				//--------------------------------------------------------------------------------------
				else if (cur_noun) 
					POS_Choice = kNoun;

				//--------------------------------------------------------------------------------------
				// Else if adjective choice exists default is adjective
				//--------------------------------------------------------------------------------------
				else if (cur_adj) 
					POS_Choice = kAdj;

				//--------------------------------------------------------------------------------------
				// Default if conjunction choice exists is conjunction
				//--------------------------------------------------------------------------------------
				else if (cur_conj) 
					POS_Choice = kConj;
			
				//--------------------------------------------------------------------------------------
				// Default if preposition choice exists is preposition
				//--------------------------------------------------------------------------------------
				else if (cur_prep) 
					POS_Choice = kPrep;
			
				//--------------------------------------------------------------------------------------
				// Else take highest rank POS choice
				//--------------------------------------------------------------------------------------
				else POS_Choice = cur_Tok->hiRank;
			
			}
		}
		
		//--------------------------------------------------------------------------------------
		// Set det_Count for later disentanglement of noun/verb ambiguity
		//--------------------------------------------------------------------------------------
		if ((POS_Choice == kDet) || (POS_Choice == kArt)) 
			det_Count = 0;
		else 
			det_Count++;
		
			
		if ( cur_Tok->hasAlt ) 
			{
			if (alt_Choice == kUndefPOS )
				{
				alt_Choice = 0;
				for (j = 0; j < 4; j++)
					{
					if (cur_Tok->POScode2[j] == POS_Choice)
						alt_Choice = 1;
					}
				}
			cur_Tok->altChoice = alt_Choice;
			}
		
		cur_Tok->POSchoice = POS_Choice;
		
		//--------------------------------------------------------------------------------------
		// Set the left context to whatever was chosen, for the next word around
		//--------------------------------------------------------------------------------------
		if ( (prev_POS == kUndefPOS) && 
			 ((POS_Choice == kRVaux) || (POS_Choice == kVaux)) )
			{
			first_Aux = true;
			}
			
		prev2_POS = prev_POS;
		prev_POS = POS_Choice;
		
	}
}



void	Zap_POS (FETokenPtr tok)
{
	short		j;

	for (j = 0; j < 4; j++)
		{
		tok->POScode1[j] = kUndefPOS;
		tok->POScode2[j] = kUndefPOS;
		}
	tok->compPOS1 = 0;
	tok->compPOS2 = 0;
	tok->POScount1 = 0;
	tok->POScount2 = 0;
}



void	SetPOS_FromSuffix (FETokenPtr tok)
{
	short		POSselect, j;
	
	
	if ( tok->suffix != kNo_suffix )
		{
		POSselect = kUndefPOS;
		
		switch (tok->suffix)
			{
			case kS_suffix:
				break;

			case kES_suffix:
				if ( (tok->POScount1 == 1) && (tok->compPOS1 & kHas_Verb) )
					POSselect = kVerb;
				else
					POSselect = kNoun;
				break;
				
			case kIES_suffix:
				POSselect = kNoun;
				break;
				
			case kED_suffix:
				POSselect = kVerb;
				
				#if 0
				if (tok->compPOS1 & kHas_Verb)
					POSselect = kAdj;
				else if (tok->compPOS1 & kHas_Noun)
					POSselect = kVerb;
				else
					POSselect = kNoun;
				#endif
				
				break;
				
			case kER_suffix:
				POSselect = kNoun;
				break;
				
			case kERS_suffix:
				POSselect = kNoun;
				break;
				
			case kEST_suffix:
				POSselect = kAdj;
				break;
				
			case kIED_suffix:
				POSselect = kVerb;
				break;
				
			case kIER_suffix:
				if ((tok->compPOS1 & kHas_Verb) && (tok->compPOS1 & kNoun))
					POSselect = kAdj;
				else if (tok->compPOS1 & kHas_Verb)
					POSselect = kNoun;
				else
					POSselect = kAdj;
				break;
				
			case kIERS_suffix:
				POSselect = kNoun;
				break;
				
			case kIEST_suffix:
				POSselect = kAdj;
				break;
				
			case kING_suffix:
				POSselect = kVerb;
				break;
				
			case kINGS_suffix:
				POSselect = kNoun;
				break;
				
			case kMENT_suffix:
				POSselect = kNoun;
				break;
				
			case kMENTS_suffix:
				POSselect = kNoun;
				break;
				
			case kIMENT_suffix:
				POSselect = kNoun;
				break;
				
			case kIMENTS_suffix:
				POSselect = kNoun;
				break;
				
			case kBLY_suffix:
				POSselect = kAdv;
				break;
				
			case kLY_suffix:
				POSselect = kAdv;
				break;

			case kOR_suffix:
				POSselect = kNoun;
				break;

			case kORS_suffix:
				POSselect = kNoun;
				break;

			case kIZE_suffix:
				POSselect = kVerb;
				break;

			case kIZED_suffix:
				POSselect = kVerb;
				break;

			case kIZES_suffix:
				POSselect = kVerb;
				break;

			case kNESS_suffix:
				POSselect = kNoun;
				break;

			case kNESSES_suffix:
				POSselect = kNoun;
				break;

			case kISM_suffix:
				POSselect = kNoun;
				break;

			case kISMS_suffix:
				POSselect = kNoun;
				break;

			case kABLE_suffix:
				POSselect = kAdj;
				break;
				
			}
		if ( (POSselect != kUndefPOS) && !(tok->hasAlt) )
			{
			tok->POScode1[0] = POSselect;
			tok->hiRank = POSselect;
			tok->POScount1 = 1;
			tok->compPOS1 = ( 1 << POSselect );
			
			for (j = 1; j < 4; j++)
				{
				tok->POScode1[j] = kUndefPOS;
				}
			}
			
		if ( (POSselect != kUndefPOS) && (tok->hasAlt) )
			{
			for (j = 0; j < 4; j++)
				{
				if ( (tok->POScode1[j] == POSselect) && (tok->POScode1[j] != tok->POScode2[j]) )  /* pick rEHd / rIYd 	*/
					{
					Zap_POS (tok);
					tok->POScode1[0] = POSselect;
					tok->hiRank = POSselect;
					tok->POScount1 = 1;
					tok->compPOS1 = ( 1 << POSselect );
					break;
					}
				else if (tok->POScode2[j] == POSselect)
					{
					Zap_POS (tok);
					tok->POScode2[0] = POSselect;
					tok->hiRank = POSselect;
					tok->POScount2 = 1;
					tok->compPOS2 = ( 1 << POSselect );
					tok->altChoice = 1;
					break;
					}
				}

			}
		}
}



void	AddPhon (FETokenPtr tok, short phon)
{
	short			phonLen;

	phonLen = (tok->phonStr[0]) + 1;
	tok->phonStr[phonLen] = (unsigned char)phon;
	tok->phonStr[0] = phonLen;
	
	if (tok->hasAlt)
		{
		phonLen = (tok->phonHold[0]) + 1;
		tok->phonHold[phonLen] = (unsigned char)phon;
		tok->phonHold[0] = phonLen;
		}
}





void	Store_S_or_Z (voiceVarPtr vv, FETokenPtr tok)
{
	short			phonLen;
	unsigned char	lastPhon;
	long			phonFlags;

	phonLen = tok->phonStr[0];
	lastPhon = tok->phonStr[phonLen];
	phonFlags = vv->phonFlags2[lastPhon];
	
	if ( (phonFlags & kPalatalF) || (lastPhon == _s_) || (lastPhon == _z_) )
		{
		AddPhon (tok, _IX_);
		AddPhon (tok, _z_);
		}
		
	else if ( (phonFlags & kConsonantF) && !(phonFlags & kVoicedF) )
		{
		//-----------------------------
		// After voiceless consonants
		//-----------------------------
		AddPhon (tok, _s_);
		}
	else
		{
		//-----------------------------
		// All others
		//-----------------------------
		AddPhon (tok, _z_);
		}
}





void	Consonant_Doubling_Adjust (unsigned char *wordPtr)
{
	short			len;
	unsigned char	endChar;
	
	len = wordPtr[0];
	endChar = wordPtr[len];
	switch (endChar)
		{
		// filter the vowels
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
		
		// Consonants that are doubled in root		
		case 'S':			// stressing -> stress
		case 'L':			// called 	 -> call
		case 'F':			// sniffing  -> sniff
			break;			// no doubling adjust
		
		default:
			if (--len > 0)	// n-1 from end
				{
				//----------------------------
				// Remove doubled mutation
				// 		canned 	 -> can
				//		slurring -> slur
				//----------------------------
				if (endChar == wordPtr[len])
					wordPtr[0] = len;			// shorten str len by 1
				}
		}
	
}



short	Decompose_E_Common (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short				wordLen, 	gotMorph;
	unsigned char		charSave;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	wordLen = wordPtr[0];
	gotMorph = false;

	//----------------------------
	// Check for e termination:
	// 		timed -> time
	//----------------------------
	wordPtr[0] = ++wordLen;
	charSave = wordPtr[wordLen];			// save orig before we...
	wordPtr[wordLen] = 'E';		 			// ...end root with E
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		wordPtr[wordLen] = charSave;		// restore original char...
		wordPtr[0] = --wordLen;				// ...remove the E from root
		gotMorph = true;
		}
	else
		{
		wordPtr[wordLen] = charSave;		// restore original char...
		wordPtr[0] = --wordLen;				// ...remove the E from root
		//----------------------------
		// Check for doubling:
		// 		napped -> nap
		//----------------------------
		Consonant_Doubling_Adjust (wordPtr);
		if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
			{
			gotMorph = true;
			}
		}
		
	wordPtr[0] = wordLen;					// restore len
	return (gotMorph);
}




short	Decompose_I_Common (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			wordLen, gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	wordLen = wordPtr[0] + 1;
	gotMorph = false;

	//----------------------------
	// Check for Y-mutation:
	// 		steadiest -> steady
	//----------------------------
	wordPtr[wordLen] = 'Y';
	wordPtr[0] = wordLen;
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		gotMorph = true;
		}

	wordPtr[wordLen] = 'I';				// restore to orig spelling
	return (gotMorph);
}



short	Do_IZING_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		organizing   -> organ
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _AY_);
		AddPhon (tok, _z_);
		AddPhon (tok, _IH_);
		AddPhon (tok, _NG_);
		gotMorph = true;
		}

	return (gotMorph);
}


short	Do_IZINGS_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		organizing   -> organ
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _AY_);
		AddPhon (tok, _z_);
		AddPhon (tok, _IH_);
		AddPhon (tok, _NG_);
		AddPhon (tok, _z_);
		gotMorph = true;
		}

	return (gotMorph);
}


short	Do_IZER_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		organizer   -> organ
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _AY_);
		AddPhon (tok, _z_);
		AddPhon (tok, _ER_);
		gotMorph = true;
		}

	return (gotMorph);
}


short	Do_IZERS_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		organizers   -> organ
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _AY_);
		AddPhon (tok, _z_);
		AddPhon (tok, _ER_);
		AddPhon (tok, _z_);
		gotMorph = true;
		}

	return (gotMorph);
}




short	Do_IZE_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short				gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		materialize -> material
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		gotMorph = true;
		AddPhon (tok, _AY_);
		AddPhon (tok, _z_);
		}

	return (gotMorph);
}



short	Do_IZED_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short				gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		materialized -> material
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _AY_);
		AddPhon (tok, _z_);
		AddPhon (tok, _d_);
		gotMorph = true;
		}

	return (gotMorph);
}


short	Do_IZES_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short				gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		materializes -> material
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _AY_);
		AddPhon (tok, _z_);
		AddPhon (tok, _IX_);
		AddPhon (tok, _z_);
		gotMorph = true;
		}

	return (gotMorph);
}



short	Do_INESS_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	short			wordLen, gotMorph;
	unsigned char		*wordPtr;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	wordLen = wordPtr[0] +1;
	gotMorph = false;

	//----------------------------
	// Check for Y-mutation:
	// 		sexy -> sexiness
	//----------------------------
	wordPtr[wordLen] = 'Y';
	wordPtr[0] = wordLen;
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _n_);
		AddPhon (tok, _IX_);
		AddPhon (tok, _s_);
		gotMorph = true;
		}
	wordPtr[wordLen] = 'I';				// restore to orig spelling
	wordLen -= 1;
	wordPtr[0] = wordLen;
		
	if ( !(gotMorph) && (wordPtr[wordLen] == 'L'))
		{
		wordPtr[0] -= 1;				// remove the 'L'
		if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
			{
			//----------------------------
			//  Root + LY + NESS:
			// 	lone + ly + ness = loneliness
			//----------------------------
			AddPhon (tok, _l_);
			AddPhon (tok, _IY_);
			AddPhon (tok, _n_);
			AddPhon (tok, _IX_);
			AddPhon (tok, _s_);
			gotMorph = true;
			}
		wordPtr[0] += 1;
		}
	return (gotMorph);
}


short	Do_INESSES_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short			wordLen, gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	wordLen = wordPtr[0] + 1;
	gotMorph = false;

	//----------------------------
	// Check for Y-mutation:
	// 		sexy -> sexinesses
	//----------------------------
	wordPtr[wordLen] = 'Y';
	wordPtr[0] = wordLen;
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _n_);
		AddPhon (tok, _IX_);
		AddPhon (tok, _s_);
		AddPhon (tok, _IX_);
		AddPhon (tok, _z_);
		gotMorph = true;
		}
	wordPtr[wordLen] = 'I';				// restore to orig spelling
	wordLen -= 1;
	wordPtr[0] = wordLen;
		
	if ( !(gotMorph) && (wordPtr[wordLen] == 'L'))
		{
		wordPtr[0] -= 1;				// remove the 'L'
		if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
			{
			//----------------------------
			//  Root + LY + NESSES:
			// 	lone + ly + nesses = lonelinesses
			//----------------------------
			AddPhon (tok, _l_);
			AddPhon (tok, _IY_);
			AddPhon (tok, _n_);
			AddPhon (tok, _IX_);
			AddPhon (tok, _s_);
			AddPhon (tok, _IX_);
			AddPhon (tok, _s_);
			gotMorph = true;
			}
		wordPtr[0] += 1;
		}
	return (gotMorph);
}




short	Do_NESS_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short				gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		massiveness -> massive
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _n_);
		AddPhon (tok, _IX_);
		AddPhon (tok, _s_);
		gotMorph = true;
		}

	return (gotMorph);
}




short	Do_NESSES_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short				gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		massivenesses -> massive
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _n_);
		AddPhon (tok, _IX_);
		AddPhon (tok, _s_);
		AddPhon (tok, _IX_);
		AddPhon (tok, _z_);
		gotMorph = true;
		}

	return (gotMorph);
}






short	Do_ISM_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short				gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		symbolism -> symbol
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _IX_);
		AddPhon (tok, _z_);
		AddPhon (tok, _AX_);
		AddPhon (tok, _m_);
		gotMorph = true;
		}

	return (gotMorph);
}


short	Do_ISMS_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short				gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		symbolisms -> symbol
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _IX_);
		AddPhon (tok, _z_);
		AddPhon (tok, _AX_);
		AddPhon (tok, _m_);
		AddPhon (tok, _z_);
		gotMorph = true;
		}

	return (gotMorph);
}





short	Do_OR_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short	wordLen, 	gotMorph;
	unsigned char		charSave;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	wordLen = wordPtr[0];
	gotMorph = false;

	//----------------------------
	// Check for e termination:
	// 		senator -> senate
	//----------------------------
	wordPtr[0] = ++wordLen;
	charSave = wordPtr[wordLen];			// save orig before we...
	wordPtr[wordLen] = 'E';		 			// ...end root with E
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		wordPtr[wordLen] = charSave;		// restore original char...
		wordPtr[0] = --wordLen;				// ...remove the E from root
		AddPhon (tok, _ER_);
		gotMorph = true;
		}
	else
		{
		wordPtr[wordLen] = charSave;		// restore original char...
		wordPtr[0] = --wordLen;				// ...remove the E from root
		//----------------------------
		//  No mutations:
		// 		sailor -> sail
		//----------------------------
		if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
			{
			gotMorph = true;
			AddPhon (tok, _ER_);
			}
		}

	return (gotMorph);
}




short	Do_ORS_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short	wordLen, 	gotMorph;
	unsigned char		charSave;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	wordLen = wordPtr[0];
	gotMorph = false;

	//----------------------------
	// Check for e termination:
	// 		senators -> senate
	//----------------------------
	wordPtr[0] = ++wordLen;
	charSave = wordPtr[wordLen];			// save orig before we...
	wordPtr[wordLen] = 'E';		 			// ...end root with E
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		wordPtr[wordLen] = charSave;		// restore original char...
		wordPtr[0] = --wordLen;				// ...remove the E from root
		AddPhon (tok, _ER_);
		AddPhon (tok, _z_);
		gotMorph = true;
		}
	else
		{
		wordPtr[wordLen] = charSave;		// restore original char...
		wordPtr[0] = --wordLen;				// ...remove the E from root
		//----------------------------
		//  No mutations:
		// 		sailors -> sail
		//----------------------------
		if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
			{
			gotMorph = true;
			AddPhon (tok, _ER_);
			AddPhon (tok, _z_);
			}
		}

	return (gotMorph);
}





short	Do_BLY_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short				wordLen, gotMorph;
	unsigned char		origLen;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	origLen = wordPtr[0];
	wordLen = origLen + 3;
	gotMorph = false;

	//----------------------------
	// Check for e termination:
	// 		possibly -> possible
	//----------------------------
	wordPtr[0] = wordLen;					// blY...
	wordPtr[wordLen] = 'E';		 			// ... -> blE
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		gotMorph = true;
		AddPhon (tok, _l_);
		AddPhon (tok, _IY_);
		}

	wordPtr[wordLen] = 'Y';				// restore to orig spelling
	wordPtr[0] = origLen;
	return (gotMorph);
}


short	Do_CALLY_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	short			gotMorph;
	unsigned char		*wordPtr;
	
	wordPtr = (unsigned char*)&tok->tokStr;
	wordPtr[0] += 1;			// get 'c' back in there
	gotMorph = false;

	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _l_);
		AddPhon (tok, _IY_);
		gotMorph = true;
		}
	return (gotMorph);
}






short	Do_LY_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char		*wordPtr;
	short				gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		madly -> mad
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _l_);
		AddPhon (tok, _IY_);
		gotMorph = true;
		}

	return (gotMorph);
}




void	Do_ED_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	short			phonLen;
	unsigned char	lastPhon;
	long			phonFlags;

	phonLen = tok->phonStr[0];
	lastPhon = tok->phonStr[phonLen];
	phonFlags = vv->phonFlags2[lastPhon];
	
	if ( (lastPhon == _t_) || (lastPhon == _d_) )
		{
		AddPhon (tok, _IX_);
		AddPhon (tok, _d_);
		}
		
	else if ( (phonFlags & kConsonantF) && !(phonFlags & kVoicedF) )
		{
		//-----------------------------
		// After voiceless consonants
		//-----------------------------
		AddPhon (tok, _t_);
		}
	else
		{
		//-----------------------------
		// All others
		//-----------------------------
		AddPhon (tok, _d_);
		}
}




void	Do_ER_Morph (voiceVarPtr vv, FETokenPtr tok)
{
		AddPhon (tok, _ER_);
}

void	Do_ERS_Morph (voiceVarPtr vv, FETokenPtr tok)
{
		AddPhon (tok, _ER_);
		AddPhon (tok, _z_);
}

void	Do_EST_Morph (voiceVarPtr vv, FETokenPtr tok)
{
		AddPhon (tok, _IX_);
		AddPhon (tok, _s_);
		AddPhon (tok, _t_);
}

void	Do_IED_Morph (voiceVarPtr vv, FETokenPtr tok)
{
		AddPhon (tok, _d_);
}

void	Do_IER_Morph (voiceVarPtr vv, FETokenPtr tok)
{
		AddPhon (tok, _ER_);
}

void	Do_IERS_Morph (voiceVarPtr vv, FETokenPtr tok)
{
		AddPhon (tok, _ER_);
		AddPhon (tok, _z_);
}

short	Do_IEST_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			gotMorph, wordLen;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	wordLen = wordPtr[0];
	gotMorph = false;

	if (Decompose_I_Common (vv, tok))
		{
		AddPhon (tok, _IX_);
		AddPhon (tok, _s_);
		AddPhon (tok, _t_);
		gotMorph = true;
		}
	wordPtr[0] = wordLen;
	if ( !(gotMorph) && (wordPtr[wordLen] == 'L') )
		{
		wordPtr[0] -= 1;				// remove the 'L'
		if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
			{
			//----------------------------
			//  Root + LY + EST:
			// 	lone + ly + est = loneliest
			//----------------------------
			AddPhon (tok, _l_);
			AddPhon (tok, _IY_);
			AddPhon (tok, _IX_);
			AddPhon (tok, _s_);
			AddPhon (tok, _t_);
			gotMorph = true;
			}
		wordPtr[0] += 1;
		}


	return (gotMorph);

}



void	Do_ING_Morph (voiceVarPtr vv, FETokenPtr tok)
{
		AddPhon (tok, _IX_);
		AddPhon (tok, _NG_);
}

void	Do_INGS_Morph (voiceVarPtr vv, FETokenPtr tok)
{
		AddPhon (tok, _IX_);
		AddPhon (tok, _NG_);
		AddPhon (tok, _z_);
}

void	Do_IMENT_Morph (voiceVarPtr vv, FETokenPtr tok)
{
		AddPhon (tok, _m_);
		AddPhon (tok, _AX_);
		AddPhon (tok, _n_);
		AddPhon (tok, _t_);
}

void	Do_IMENTS_Morph (voiceVarPtr vv, FETokenPtr tok)
{
		AddPhon (tok, _m_);
		AddPhon (tok, _AX_);
		AddPhon (tok, _n_);
		AddPhon (tok, _t_);
		AddPhon (tok, _s_);
}


void	Do_ABLE_Morph (voiceVarPtr vv, FETokenPtr tok)
{

		AddPhon (tok, _AX_);
		AddPhon (tok, _b_);
		AddPhon (tok, _EL_);
}






short	Do_MENT_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		shipment   -> ship
	//		settlement -> settle
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _m_);
		AddPhon (tok, _AX_);
		AddPhon (tok, _n_);
		AddPhon (tok, _t_);
		gotMorph = true;
		}

	return (gotMorph);
}




short	Do_MENTS_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	//----------------------------
	//  No mutations:
	// 		shipments   -> ship
	//		settlements -> settle
	//----------------------------
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		AddPhon (tok, _m_);
		AddPhon (tok, _AX_);
		AddPhon (tok, _n_);
		AddPhon (tok, _t_);
		AddPhon (tok, _s_);
		gotMorph = true;
		}

	return (gotMorph);
}






short	Do_IES_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			wordLen, gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	wordLen = wordPtr[0] + 1;
	gotMorph = false;

	//----------------------------
	// Check for Y-mutation:
	// 		candies -> candy
	//----------------------------
	wordPtr[wordLen] = 'Y';
	wordPtr[0] = wordLen;
	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		wordPtr[wordLen] = 'I';				// restore to orig spelling
		Store_S_or_Z (vv, tok);
		gotMorph = true;
		}
	else
		{
		//----------------------------
		// Check for plural:
		// 		calories -> calorie
		//----------------------------
		wordPtr[wordLen] = 'I';
		wordPtr[0] = ++wordLen;				// end with IE
		if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
			{
			Store_S_or_Z (vv, tok);
			gotMorph = true;
			}
		}

	return (gotMorph);
}




short	Do_ES_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			wordLen, gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	wordLen = wordPtr[0];
	gotMorph = false;

	if ( 	(wordPtr[wordLen] == 'H') && 
			((wordPtr[wordLen-1] == 'S') || (wordPtr[wordLen-1] == 'C')) )
		{
		if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
			{
			//------------------------
			// fish 	->	fishES
			// scratch	->	scratchES
			//
			// FAILS:	ache  mustache cache
			//------------------------
			Store_S_or_Z (vv, tok);
			gotMorph = true;
			}
		}
	else if ( (wordPtr[wordLen] == 'S') && (wordPtr[wordLen-1] == 'S') )
		{
		if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
			{
			//------------------------
			// stress	->	stressES
			//------------------------
			Store_S_or_Z (vv, tok);
			gotMorph = true;
			}
		}
	else if ( wordPtr[wordLen] == 'X')
		{
		if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
			{
			//------------------------
			// box	->	boxES
			//------------------------
			Store_S_or_Z (vv, tok);
			gotMorph = true;
			}
		}
	else
		{
		wordLen++;					// keep the 'E'
		wordPtr[0] = wordLen;
		if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
			{
			//------------------------
			// house 	-> 	houseS
			// clothe 	-> 	clotheS
			// name		->	nameS
			//------------------------
			Store_S_or_Z (vv, tok);
			gotMorph = true;
			}
		else
			{
			wordLen--;					// remove the 'E'
			wordPtr[0] = wordLen;
			if ( (wordPtr[wordLen] == 'S') || (wordPtr[wordLen] == 'Z') )
				{
				if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
					{
					//------------------------
					// bus 		-> 	busES
					// waltz 	-> 	waltzES
					//------------------------
					Store_S_or_Z (vv, tok);
					gotMorph = true;
					}
				}
			}
		}

	return (gotMorph);
}



short	Do_S_Morph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			gotMorph;
	

	wordPtr = (unsigned char*)&tok->tokStr;
	gotMorph = false;

	if ( SearchAllDicts (vv, wordPtr, tok, vv->Dict, true) )
		{
		Store_S_or_Z (vv, tok);
		gotMorph = true;
		}

	return (gotMorph);
}





short	Search_Suffix (voiceVarPtr vv, unsigned char *wordPtr)
{
	short			index, len, origLen, ret;
	unsigned char	*sPtr;
	

	sPtr = vv->SuffixTab;
	index = 0;
	origLen = wordPtr[0];
	len = origLen;
	ret = 0;
	
	if (wordPtr[len] == 0x27)		// single quote (compilers have trouble with this)
		{
		len--;
		origLen--;					// skip term apos
		}
		
	
	while (*sPtr != 0xFF)				// end of suffix table?
		{
		if ( (*sPtr == wordPtr[len]) && (len > 1) )
			{
			sPtr++;
			len--;
			if (*sPtr == 0)		// end of suffix, we got a match!
				{
				wordPtr[0] = len;
				ret = vv->SuffixType[index];
				break;
				}
			}
		else
			{
			while (*sPtr != 0)	// skip to next suffix
				sPtr++;
			sPtr++;					// skip delimiter
			index++;				// next suffix
			len = origLen;			// reset word end ptr
			}
		}
	return (ret);
}



short	DoMorph (voiceVarPtr vv, FETokenPtr tok)
{
	unsigned char	*wordPtr;
	short			gotMorph, sufType;

	wordPtr = (unsigned char*)&tok->tokStr;
	tok->tokLen = wordPtr[0];
	gotMorph = false;
	tok->suffix = kNo_suffix;
	
	if (wordPtr[wordPtr[0]] == 'S')
		{
		wordPtr[0]--;					// orig word minus "S"
		if (Do_S_Morph (vv, tok))
			{
			gotMorph = true;
			sufType = kS_suffix;
			goto GOT_IT;
			}
		else
			wordPtr[0]++;				// restore "S"
		}
	if ( (sufType = Search_Suffix (vv, wordPtr)) > 0 )
		{
		switch (sufType)
			{
			case kIZING_suffix:
				wordPtr[0] = tok->tokLen - 3;	// orig word minus "ING"
				if (Decompose_E_Common (vv, tok))
					{
					Do_ING_Morph (vv, tok);
					sufType = kING_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 5;	// orig word minus "IZING"
					if (Do_IZING_Morph (vv, tok))
						gotMorph = true;
					}
				break;

			case kIZINGS_suffix:
				wordPtr[0] = tok->tokLen - 4;	// orig word minus "INGS"
				if (Decompose_E_Common (vv, tok))
					{
					Do_INGS_Morph (vv, tok);
					sufType = kINGS_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 6;	// orig word minus "IZINGS"
					if (Do_IZINGS_Morph (vv, tok))
						gotMorph = true;
					}
				break;

			case kIZER_suffix:
				wordPtr[0] = tok->tokLen - 2;	// orig word minus "ER"
				if (Decompose_E_Common (vv, tok))
					{
					Do_ER_Morph (vv, tok);
					sufType = kER_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 4;	// orig word minus "IZER"
					if (Do_IZER_Morph (vv, tok))
						gotMorph = true;
					}
				break;
				
			case kIZERS_suffix:
				wordPtr[0] = tok->tokLen - 3;	// orig word minus "ERS"
				if (Decompose_E_Common (vv, tok))
					{
					Do_ERS_Morph (vv, tok);
					sufType = kERS_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 5;	// orig word minus "IZERS"
					if (Do_IZERS_Morph (vv, tok))
						gotMorph = true;
					}
				break;
				
				


			case kS_suffix:
				if (Do_S_Morph (vv, tok))
					{
					gotMorph = true;
					}
				break;

			case kES_suffix:
				wordPtr[0] = tok->tokLen - 1;	// orig word minus "S"
				if (Do_S_Morph (vv, tok))
					{
					sufType = kS_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 2;	// orig word minus "ES"
					if (Do_ES_Morph (vv, tok))
						gotMorph = true;
					}
				break;
				
			case kIES_suffix:
				wordPtr[0] = tok->tokLen - 1;	// orig word minus "S"
				if (Do_S_Morph (vv, tok))
					{
					sufType = kS_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 3;	// orig word minus "IES"
					if (Do_IES_Morph (vv, tok))
						gotMorph = true;
					}
				break;
				
			case kED_suffix:
				if (Decompose_E_Common (vv, tok))
					{
					Do_ED_Morph (vv, tok);
					gotMorph = true;
					}
				break;
				
			case kER_suffix:
				if (Decompose_E_Common (vv, tok))
					{
					Do_ER_Morph (vv, tok);
					gotMorph = true;
					}
				break;
				
			case kERS_suffix:
				wordPtr[0] = tok->tokLen - 1;	// orig word minus "S"
				if (Do_S_Morph (vv, tok))
					{
					sufType = kS_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 3;	// orig word minus "ERS"
					if (Decompose_E_Common (vv, tok))
						{
						Do_ERS_Morph (vv, tok);
						gotMorph = true;
						}
					}
				break;
				
			case kEST_suffix:
				if (Decompose_E_Common (vv, tok))
					{
					Do_EST_Morph (vv, tok);
					gotMorph = true;
					}
				break;
				
			case kIED_suffix:
				if (Decompose_I_Common (vv, tok))
					{
					Do_IED_Morph (vv, tok);
					gotMorph = true;
					}
				break;
				
			case kIER_suffix:
				if (Decompose_I_Common (vv, tok))
					{
					Do_IER_Morph (vv, tok);
					gotMorph = true;
					}
				break;
				
			case kIERS_suffix:
				wordPtr[0] = tok->tokLen - 1;	// orig word minus "S"
				if (Do_S_Morph (vv, tok))
					{
					sufType = kS_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 4;	// orig word minus "IERS"
					if (Decompose_I_Common (vv, tok))
						{
						Do_IERS_Morph (vv, tok);
						gotMorph = true;
						}
					}
				break;
				
			case kIEST_suffix:
				if (Do_IEST_Morph (vv, tok))
					{
					gotMorph = true;
					}
				break;
				
			case kING_suffix:
				if (Decompose_E_Common (vv, tok))
					{
					Do_ING_Morph (vv, tok);
					gotMorph = true;
					}
				break;
				
			case kINGS_suffix:
				if (Decompose_E_Common (vv, tok))
					{
					Do_INGS_Morph (vv, tok);
					gotMorph = true;
					}
				break;
				
			case kMENT_suffix:
				if (Do_MENT_Morph (vv, tok))
					{
					gotMorph = true;
					}
				break;
				
			case kMENTS_suffix:
				wordPtr[0] = tok->tokLen - 1;	// orig word minus "S"
				if (Do_S_Morph (vv, tok))
					{
					sufType = kS_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 5;	// orig word minus "MENTS"
					if (Do_MENTS_Morph (vv, tok))
						{
						gotMorph = true;
						}
					}
				break;
				
			case kIMENT_suffix:
				if (Decompose_I_Common (vv, tok))
					{
					Do_IMENT_Morph (vv, tok);
					gotMorph = true;
					}
				break;
				
			case kIMENTS_suffix:
				wordPtr[0] = tok->tokLen - 1;	// orig word minus "S"
				if (Do_S_Morph (vv, tok))
					{
					sufType = kS_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 6;	// orig word minus "IMENTS"
					if (Decompose_I_Common (vv, tok))
						{
						Do_IMENTS_Morph (vv, tok);
						gotMorph = true;
						}
					}
				break;
				
			case kBLY_suffix:
				if (Do_BLY_Morph (vv, tok))
					gotMorph = true;
				else 
					{
					wordPtr[0] += 1;				// include B
					if (Do_LY_Morph (vv, tok))		// superbly -> superb
						gotMorph = true;
					}
				break;
				
			case kCALLY_suffix:
				if (Do_CALLY_Morph (vv, tok))
					gotMorph = true;
				else
					{
					wordPtr[0] = tok->tokLen - 2;	// orig word minus "LY"
					if (Do_LY_Morph (vv, tok))
						gotMorph = true;
					}
				break;
				
			case kLY_suffix:
				if (Do_LY_Morph (vv, tok))
					gotMorph = true;
				break;

			case kOR_suffix:
				if (Do_OR_Morph (vv, tok))
					gotMorph = true;
				break;

			case kORS_suffix:
				wordPtr[0] = tok->tokLen - 1;	// orig word minus "S"
				if (Do_S_Morph (vv, tok))
					{
					sufType = kS_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 3;	// orig word minus "ORS"
					if (Do_ORS_Morph (vv, tok))
						gotMorph = true;
					}
				break;

			case kIZE_suffix:
				if (Do_IZE_Morph (vv, tok))
					gotMorph = true;
				break;

			case kIZED_suffix:
				wordPtr[0] = tok->tokLen - 2;		// orig word minus "ED"
				if (Decompose_E_Common (vv, tok))
					{
					Do_ED_Morph (vv, tok);
					sufType = kED_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 4;		// orig word minus "IZED"
					if (Do_IZED_Morph (vv, tok))
						gotMorph = true;
					}
				break;

			case kIZES_suffix:
				wordPtr[0] = tok->tokLen - 1;	// orig word minus "S"
				if (Do_S_Morph (vv, tok))
					{
					sufType = kS_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 4;	// orig word minus "IZES"
					if (Do_IZES_Morph (vv, tok))
						gotMorph = true;
					}
				break;

			case kINESS_suffix:
				if (Do_INESS_Morph (vv, tok))
					gotMorph = true;
				break;

			case kINESSES_suffix:
				wordPtr[0] = tok->tokLen - 2;	// orig word minus "ES"
				if (Do_ES_Morph (vv, tok))
					{
					sufType = kES_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 7;	// orig word minus "INESSES"
					if (Do_INESSES_Morph (vv, tok))
						gotMorph = true;
					}
				break;

			case kNESS_suffix:
				if (Do_NESS_Morph (vv, tok))
					gotMorph = true;
				break;

			case kNESSES_suffix:
				wordPtr[0] = tok->tokLen - 2;	// orig word minus "ES"
				if (Do_ES_Morph (vv, tok))
					{
					sufType = kES_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 6;	// orig word minus "NESSES"
					if (Do_NESSES_Morph (vv, tok))
						gotMorph = true;
					}
				break;

			case kISM_suffix:
				if (Do_ISM_Morph (vv, tok))
					gotMorph = true;
				break;

			case kISMS_suffix:
				wordPtr[0] = tok->tokLen - 1;	// orig word minus "S"
				if (Do_S_Morph (vv, tok))
					{
					sufType = kS_suffix;
					gotMorph = true;
					}
				else
					{
					wordPtr[0] = tok->tokLen - 4;	// orig word minus "ISMS"
					if (Do_ISMS_Morph (vv, tok))
						gotMorph = true;
					}
				break;

			case kABLE_suffix:
				if (Decompose_E_Common (vv, tok))
					{
					Do_ABLE_Morph (vv, tok);
					gotMorph = true;
					}
				break;
				
			}
GOT_IT:
		tok->suffix = sufType;
		
		#if 0
		tok->hasAlt = false;
		tok->compPOS2 = 0;
		tok->POScount2 = 0;
		for (j = 0; j < 4; j++)
			{
			tok->POScode2[j] = kUndefPOS;
			}
		#endif
		
		SetPOS_FromSuffix (tok);
		}

	wordPtr[0] = tok->tokLen;
	return (gotMorph);

}



