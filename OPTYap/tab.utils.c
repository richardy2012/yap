/**********************************************************************
                                                               
                       The OPTYap Prolog system                
  OPTYap extends the Yap Prolog system to support or-parallel tabling
                                                               
  Copyright:   R. Rocha and NCC - University of Porto, Portugal
  File:        tab.utils.h
                                                                     
**********************************************************************/

#include "Yap.h"
#include "tab.utils.h"
#include "Yatom.h"
#include "tab.macros.h"

#ifdef TABLING_CALL_SUBSUMPTION

static void printTrieSymbol(FILE* fp, Cell symbol);

CellTag cell_tag(Term t)
{
  if(IsVarTerm(t))
    return TAG_REF;
    
  if(IsAtomTerm(t))
    return TAG_ATOM;
    
  if(IsIntTerm(t))
    return TAG_INT;
    
  if(IsPairTerm(t))
    return TAG_LIST;
    
  if (IsApplTerm(t)) {
    Functor f = FunctorOfTerm(t);
    
    if (f == FunctorDouble)
      return TAG_FLOAT;
      
    if (f == FunctorLongInt)
      return TAG_LONG_INT;
    
    if (f == FunctorDBRef)
      return TAG_DB_REF;
    
    if (f == FunctorBigInt)
      return TAG_BIG_INT;
    
    return TAG_STRUCT;
  }
  
  return TAG_UNKNOWN;
}

static inline
CellTag TrieSymbolType(Term t)
{
  if(IsVarTerm(t))
    return XSB_TrieVar;
    
  if(IsIntTerm(t))
    return TAG_INT;
  
  if(IsAtomTerm(t))
    return TAG_ATOM;
  
  if(IsPairTerm(t))
    return TAG_LIST;
  
  if(IsApplTerm(t)) {
    Functor f = (Functor) RepAppl(t);
    if (f == FunctorDouble)
      return TAG_FLOAT;
    
    if(f == FunctorLongInt)
      return TAG_LONG_INT;
    
    return TAG_STRUCT;
  }
  
  return TAG_UNKNOWN;
}

xsbBool are_identical_terms(Cell term1, Cell term2)
{
begin_are_identical_terms:
  XSB_Deref(term1);
  XSB_Deref(term2);
  
  if(term1 == term2)
    return TRUE;
  
  if(cell_tag(term1) != cell_tag(term2))
    return FALSE;
  
  if(cell_tag(term1) == XSB_STRUCT) {
    CPtr cptr1 = clref_val(term1);
    CPtr cptr2 = clref_val(term2);
    Psc psc1 = (Psc)*cptr1;
    int i;
    
    if(psc1 != (Psc)*cptr2)
      return FALSE;
    
    for(cptr1++, cptr2++, i = 0; i < (int)get_arity(psc1)-1; cptr1++, cptr2++, i++ )
      if(!are_identical_terms(*cptr1, *cptr2))
        return FALSE;
    term1 = *cptr1;
    term2 = *cptr2;
    goto begin_are_identical_terms;  
  }
  else if (cell_tag(term1) == XSB_LIST) {
    CPtr cptr1 = clrefp_val(term1);
    CPtr cptr2 = clrefp_val(term2);
    
    if(are_identical_terms(*cptr1, *cptr2)) {
      term1 = *(cptr1 + 1);
      term2 = *(cptr2 + 1);
      
      goto begin_are_identical_terms;
    } else return FALSE;
  }
#ifdef SUBSUMPTION_YAP
  else if(cell_tag(term1) == TAG_LONG_INT) {
    printf("identical long int\n");
    Int v1 = LongIntOfTerm(term1);
    Int v2 = LongIntOfTerm(term2);
    
    return v1 == v2;
  }
  else if(cell_tag(term1) == TAG_FLOAT) {
    printf("identical float term\n");
    
    Float dbl1 = FloatOfTerm(term1);
    Float dbl2 = FloatOfTerm(term2);
    
    return dbl1 == dbl2;
  }
#endif /* SUBSUMPTION_YAP */
  else return FALSE;
}

void printTrieNode(FILE *fp, BTNptr pTN)
{
  fprintf(fp, "Trie Node: Addr(%p) Symbol: ", pTN);
  printTrieSymbol(fp, BTN_Symbol(pTN));
}

static void printTrieSymbol(FILE* fp, Cell symbol)
{
  switch(TrieSymbolType(symbol)) {
    case XSB_INT:
      fprintf(fp, IntegerFormatString, int_val(symbol));
      break;
    case XSB_FLOAT:
      fprintf(fp, "float");
      break;
    case XSB_STRING:
      fprintf(fp, "%s", string_val(symbol));
      break;
    case XSB_TrieVar:
      fprintf(fp, "V" IntegerFormatString, DecodeTrieVar(symbol));
      break;
    case XSB_STRUCT:
      {
        Psc psc;

        psc = DecodeTrieFunctor(symbol);
        fprintf(fp, "%s/%d", get_name(psc), get_arity(psc));
        break;
      }
    case XSB_LIST:
      fprintf(fp, "LIST");
      break;
    case TAG_LONG_INT:
      fprintf(fp, "LONG INT");
      break;
    default:
      fprintf(fp, "Unknown symbol (tag = %d)", cell_tag(symbol));
      break;
  }
}

static
void symstkPrintNextTrieTerm(CTXTdeclc FILE *fp, xsbBool list_recursion)
{
  Cell symbol;
  
  if(SymbolStack_IsEmpty) {
    fprintf(fp, "<no subterm>");
    return;
  }
  
  SymbolStack_Pop(symbol);
  
  if(IsIntTerm(symbol)) {
    if(list_recursion)
      fprintf(fp, "|" IntegerFormatString "]", int_val(symbol));
    else
      fprintf(fp, IntegerFormatString, int_val(symbol));
  } else if(IsVarTerm(symbol)) {
    if(list_recursion)
      fprintf(fp, "|V" IntegerFormatString "]", DecodeTrieVar(symbol));
    else
      fprintf(fp, "V" IntegerFormatString, DecodeTrieVar(symbol));
  } else if(IsAtomTerm(symbol)) {
    char *string = string_val(symbol);
    
    if(list_recursion) {
      if(symbol == TermNil)
        fprintf(fp, "]");
      else
        fprintf(fp, "|%s]", string);
    }
    else
      fprintf(fp, "%s", string);
  } else if(IsApplTerm(symbol)) {
    Functor f = (Functor) RepAppl(symbol);
    
    if(f == FunctorDouble) {
      volatile Float dbl;
      volatile Term *t_dbl = (Term *)((void *) &dbl);
      
#if SIZEOF_DOUBLE == 2 * SIZEOF_INT_P
      SymbolStack_Pop(*(t_dbl + 1));
#endif /* SIZEOF_DOUBLE x SIZEOF_INT_P */
      SymbolStack_Pop(*t_dbl);
      
      if(list_recursion)
        fprintf(fp, "|" FloatFormatString "]", dbl);
      else
        fprintf(fp, FloatFormatString, dbl);
        
    } else if(f == FunctorLongInt) {
      Int li;
      SymbolStack_Pop(li);
      if(list_recursion)
        fprintf(fp, "|" LongIntFormatString "]", li);
      else
        fprintf(fp, LongIntFormatString, li);
    } else {
      int i;
      
      if(list_recursion)
        fprintf(fp, "|");
      
      fprintf(fp, "%s(", get_name(f));
      for(i = 1; i < (int)get_arity(f); i++) {
        symstkPrintNextTrieTerm(CTXTc fp, FALSE);
        fprintf(fp, ",");
      }
      symstkPrintNextTrieTerm(CTXTc fp, FALSE);
      fprintf(fp, ")");
      if(list_recursion)
        fprintf(fp, "]");
    }
  } else if(IsPairTerm(symbol)) {
#ifdef TRIE_COMPACT_PAIRS
    if(symbol == CompactPairInit) {
      int cnt;
      
      fprintf(fp, "[");
      
      for(cnt = 0; ; ++cnt) {
        SymbolStack_Peek(symbol);
        
        if(symbol == CompactPairEndList) {
          SymbolStack_BlindDrop;
          if(cnt)
            fprintf(fp, ",");
          symstkPrintNextTrieTerm(CTXTc fp, FALSE);
          fprintf(fp, "]");
          break;
        } else if(symbol == CompactPairEndTerm) {
          SymbolStack_BlindDrop;
          if(cnt)
            fprintf(fp, "|");
          symstkPrintNextTrieTerm(CTXTc fp, FALSE);
          fprintf(fp, "]");
          break;
        }
        
        if(cnt)
          fprintf(fp, ",");
        
        symstkPrintNextTrieTerm(CTXTc fp, FALSE);
      }
    }
#else
    if(list_recursion)
      fprintf(fp, ",");
    else
      fprintf(fp, "[");
  
    symstkPrintNextTrieTerm(CTXTc fp, FALSE);
    symstkPrintNextTrieTerm(CTXTc fp, TRUE);
#endif
  } else {
    if(list_recursion)
      fprintf(fp, "uknown_symbol]");
    else
      fprintf(fp, "unknown_symbol");
  }
}

void printSubgoalTriePath(CTXTdeclc FILE *fp, BTNptr pLeaf, tab_ent_ptr tab_entry)
{
  BTNptr pRoot;
  
  if(IsNULL(pLeaf)) {
    fprintf(fp, "NULL");
    return;
  }
  
  SymbolStack_ResetTOS;
  SymbolStack_PushPathRoot(pLeaf, pRoot);
  
  fprintf(fp, "%s", string_val((Term)TabEnt_atom(tab_entry)));
  fprintf(fp, "(");
  symstkPrintNextTrieTerm(CTXTc fp, FALSE);
  while(!SymbolStack_IsEmpty) {
    fprintf(fp, ",");
    symstkPrintNextTrieTerm(CTXTc fp, FALSE);
  }
  fprintf(fp, ")");
}

static int variable_counter = 0;

static void
recursivePrintSubterm(FILE *fp, Term symbol, xsbBool list_recursion)
{
  XSB_Deref(symbol);
  
  if(IsIntTerm(symbol)) {
    if(list_recursion)
      fprintf(fp, "|" IntegerFormatString "]", int_val(symbol));
    else
      fprintf(fp, IntegerFormatString, int_val(symbol));
  } else if(IsVarTerm(symbol)) {
    if(list_recursion)
      fprintf(fp, "|");
    
    int var_index;
            
    if(IsStandardizedVariable(symbol)) {
      var_index = IndexOfStdVar(symbol);
    } else {
      Trail_Push(symbol);
      StandardizeVariable(symbol, variable_counter);
      var_index = variable_counter++;
    }
    
    fprintf(fp, "V" IntegerFormatString, var_index);  
    
    if(list_recursion)
      fprintf(fp, "]");
  } else if(IsAtomTerm(symbol)) {
    char *string = string_val(symbol);
    
    if(list_recursion) {
      if(symbol == TermNil)
        fprintf(fp, "]");
      else
        fprintf(fp, "|%s]", string);
    }
    else
      fprintf(fp, "%s", string);
  } else if(IsApplTerm(symbol)) {
    Functor f = FunctorOfTerm(symbol);
    
    if(f == FunctorDouble) {
      Float dbl = FloatOfTerm(symbol);
      
      if(list_recursion)
        fprintf(fp, "|" FloatFormatString "]", dbl);
      else
        fprintf(fp, FloatFormatString, dbl);
    } else if(f == FunctorLongInt) {
      Int li = LongIntOfTerm(symbol);
      
      if(list_recursion)
        fprintf(fp, "|" LongIntFormatString "]", li);
      else
        fprintf(fp, LongIntFormatString, li);
    } else {
      int i;
      
      if(list_recursion)
        fprintf(fp, "|");
      
      fprintf(fp, "%s(", get_name(f));
      
      for(i = 1; i <= (int)get_arity(f); i++) {
        if(i > 1)
          fprintf(fp, ",");
        recursivePrintSubterm(fp, *(RepAppl(symbol) + i), FALSE);
      }
      
      fprintf(fp, ")");
      
      if(list_recursion)
        fprintf(fp, "]");
    }
  } else if(IsPairTerm(symbol)) {
    if(list_recursion)
      fprintf(fp, ",");
    else
      fprintf(fp, "[");
    
    recursivePrintSubterm(fp, *(RepPair(symbol)), FALSE);
    recursivePrintSubterm(fp, *(RepPair(symbol) + 1), TRUE);
  }
}

void printSubterm(FILE *fp, Term term)
{
  Trail_ResetTOS;
  variable_counter = 0;
  recursivePrintSubterm(fp, term, FALSE);
  Trail_Unwind_All;
}

void printCalledSubgoal(FILE *fp, yamop *preg)
{
  int i, arity = preg->u.Otapl.s;
  tab_ent_ptr tab_ent = preg->u.Otapl.te;
  
  Trail_ResetTOS;
  variable_counter = 0;
  
  fprintf(fp, "SUBGOAL: %s(", string_val((Term)TabEnt_atom(tab_ent)));
  for(i = 1; i <= arity; ++i) {
    if(i > 1)
      fprintf(fp, ",");
    recursivePrintSubterm(fp, XREGS[i], FALSE);
  }
  fprintf(fp, ")");
}

#endif /* TABLING_CALL_SUBSUMPTION */