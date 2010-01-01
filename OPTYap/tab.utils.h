/**********************************************************************
                                                               
                       The OPTYap Prolog system                
  OPTYap extends the Yap Prolog system to support or-parallel tabling
                                                               
  Copyright:   R. Rocha and NCC - University of Porto, Portugal
  File:        tab.utils.h
                                                                     
**********************************************************************/

#ifndef TAB_UTILS_H
#define TAB_UTILS_H

#include "opt.config.h"

#ifdef TABLING_CALL_SUBSUMPTION
#include "tab.xsb.h"
#include "tab.stack.h"

// emu/tst_aux.h

extern DynamicStack tstTermStack;

#define TST_TERMSTACK_INITSIZE 25

#define TermStack_Top ((CPtr)DynStk_Top(tstTermStack))
#define TermStack_Base ((CPtr)DynStk_Base(tstTermStack))
#define TermStack_NumTerms DynStk_NumFrames(tstTermStack)
#define TermStack_ResetTOS DynStk_ResetTOS(tstTermStack)
#define TermStack_IsEmpty DynStk_IsEmpty(tstTermStack)

#define TermStack_SetTOS(Index) \
  DynStk_Top(tstTermStack) = TermStack_Base + Index
  
#define TermStack_Push(Term) { \
    CPtr nextFrame;     \
    DynStk_Push(tstTermStack, nextFrame); \
    *nextFrame = Term;  \
  }
  
#define TermStack_BlindPush(Term) { \
    CPtr nextFrame; \
    DynStk_BlindPush(tstTermStack, nextFrame);  \
    *nextFrame = Term;  \
  }
  
#define TermStack_Pop(Term) { \
    CPtr curFrame;  \
    DynStk_BlindPop(tstTermStack, curFrame); \
    Term = *curFrame; \
  } 

#define TermStack_Peek(Term) { \
    CPtr curFrame; \
    DynStk_BlindPeek(tstTermStack, curFrame); \
    Term = *curFrame; \
  }
  
#define TermStack_NOOP /* nothing to push when constants match */


#define TermStack_PushFunctorArgs(CS_Cell)  \
  TermStack_PushLowToHighVector(clref_val(CS_Cell) + 1, \
    get_cell_arity(CS_Cell))

// TermStack_PushListArgs XXX

/*
 * The following macros enable the movement of an argument vector to
 * the TermStack.  Two versions are supplied depending on whether the
 * vector is arranged from high-to-low memory, such as an answer
 * template, or from low-to-high memory, such as the arguments of a
 * compound heap term.  The vector pointer is assumed to reference the
 * first element of the vector.
 */
 
#define TermStack_PushLowToHighVector(pVectorLow, Magnitude)  { \
  int i, numElements;  \
  CPtr pElement;   \
  numElements = Magnitude; \
  pElement = pVectorLow + numElements; \
  DynStk_ExpandIfOverflow(tstTermStack, numElements);  \
  for(i = 0; i < numElements; ++i) { \
    pElement--; \
    TermStack_BlindPush(get_term_deref(pElement)); \
  } \
}

#define TermStack_PushHighToLowVector(pVectorHigh, Magnitude) { \
    int i, numElements; \
    CPtr pElement;  \
    numElements = Magnitude;  \
    pElement = pVectorHigh - numElements; \
    DynStk_ExpandIfOverflow(tstTermStack, numElements); \
    for(i = 0; i < numElements; ++i)  { \
      pElement++; \
      TermStack_BlindPush(get_term_deref(pElement)); \
    } \
}

/*
 * This macro copies an array of terms onto the TermStack, checking for
 * overflow only once at the beginning, rather than with each push.  The
 * elements to be pushed are assumed to exist in array elements
 * 0..(NumElements-1).
 */

#define TermStack_PushArray(Array, NumElements) { \
  counter i;  \
  DynStk_ExpandIfOverflow(tstTermStack, NumElements); \
  for(i = 0; i < numElements; ++i)  \
    TermStack_BlindPush(Array[i]);  \
}

/* ------------------------------------------------------------------------- */

/*
 *  tstTermStackLog
 *  ---------------
 *  For noting the changes made to the tstTermStack during processing
 *  where backtracking is required.  Only the changes necessary to
 *  transform the tstTermStack from its current state to a prior state
 *  are logged.  Therefore, we only need record values popped from the
 *  tstTermStack.
 *
 *  Each frame of the log consists of the index of a tstTermStack
 *  frame and the value that was stored there.  tstTermStack values
 *  are reinstated as a side effect of a tstTermStackLog_Pop.
 */
 
typedef struct {
  int index;
  Cell value;
} tstLogFrame;

typedef tstLogFrame *pLogFrame;

#define LogFrame_Index(Frame) ((Frame)->index)
#define LogFrame_Value(Frame) ((Frame)->value)

extern DynamicStack tstTermStackLog;

#define TST_TERMSTACKLOG_INITSIZE 20

#define TermStackLog_Top ((pLogFrame)DynStk_Top(tstTermStackLog))
#define TermStackLog_Base ((pLogFrame)DynStk_Base(tstTermStackLog))
#define TermStackLog_ResetTOS DynStk_ResetTOS(tstTermStackLog)

#define TermStackLog_PushFrame {  \
    pLogFrame curFrame; \
    DynStk_Push(tstTermStackLog, nextFrame);  \
    LogFrame_Index(nextFrame) = TermStack_Top - TermStack_Base; \
    LogFrame_Value(nextFrame) = *(TermStack_Top); \
}

#define TermStack_PopAndReset { \
    pLogFrame curFrame; \
    DynStk_BlindPop(tstTermStackLog, curFrame); \
    TermStack_Base[LogFrame_Index(curFrame)] = LogFrame_Value(curFrame);  \
}

/*
 * Reset the TermStack elements down to and including the Index-th
 * entry in the Log.
 */
#define TermStackLog_Unwind(Index)  { \
    pLogFrame unwindBase = TermStackLog_Base + Index;  \
    while(TermStackLog_Top > unwindBase)  \
      TermStackLog_PopAndReset; \
}

/* ------------------------------------------------------------------------- */

/*
 *  tstTrail
 *  ---------
 *  For recording bindings made during processing.  This Trail performs
 *  simple WAM trailing -- it saves address locations only.
 */
 
extern DynamicStack tstTrail;
 
#define TST_TRAIL_INITSIZE  20

#define Trail_Top           ((CPtr *)DynStk_Top(tstTrail))
#define Trail_Base          ((CPtr *)DynStk_Base(tstTrail))
#define Trail_NumBindings   DynStk_NumFrames(tstTrail)
#define Trail_ResetTOS      DynStk_ResetTOS(tstTrail)

#define Trail_Push(Addr) {  \
    CPtr *nextFrame;  \
    DynStk_Push(tstTrail, nextFrame); \
    *nextFrame = (CPtr)(Addr);  \
}

#define Trail_PopAndReset { \
    CPtr *curFrame; \
    DynStk_BlindPop(tstTrail, curFrame);  \
    bld_free(*curFrame);  \
}

#define Trail_Unwind_All  Trail_Unwind(0)

/*
 * Untrail down to and including the Index-th element.
 */
#define Trail_Unwind(Index) { \
   CPtr *unwindBase = Trail_Base + Index; \
   while(Trail_Top > unwindBase)  \
    Trail_PopAndReset; \
}

#endif /* TABLING_CALL_SUBSUMPTION */

#endif