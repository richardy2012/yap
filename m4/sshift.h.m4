/*************************************************************************
*									 *
*	 YAP Prolog   %W% %G%
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		sshift.h						 *
* Last rev:	19/2/88							 *
* mods:									 *
* comments:	stack shifter functionality for YAP			 *
*									 *
*************************************************************************/


/* The difference between the old stack pointers and the new ones */
extern Int HDiff,
  GDiff,
  LDiff,
  TrDiff,
  XDiff,
  DelayDiff;

/* The old stack pointers */
extern CELL    *OldASP, *OldLCL0;
extern tr_fr_ptr OldTR;
extern CELL    *OldGlobalBase, *OldH, *OldH0;
extern ADDR     OldTrailBase, OldTrailTop;
extern ADDR     OldHeapBase, OldHeapTop;

#define CharP(ptr)	((char *) (ptr))

Inline(IsHeapP, int, CELL *, ptr, (ptr >= (CELL *)HeapBase && ptr <= (CELL *)HeapTop) )

/* Adjusting cells and pointers to cells */
Inline(PtoGloAdjust, CELL *, CELL *, ptr, ((CELL *)(CharP(ptr) + GDiff)) )
Inline(PtoDelayAdjust, CELL *, CELL *, ptr, ((CELL *)(CharP(ptr) + DelayDiff)) )
Inline(PtoTRAdjust, tr_fr_ptr, tr_fr_ptr, ptr, ((tr_fr_ptr)(CharP(ptr) + TrDiff)) )
Inline(CellPtoTRAdjust, CELL *, CELL *, ptr, ((CELL *)(CharP(ptr) + TrDiff)) )
Inline(PtoLocAdjust, CELL *, CELL *, ptr, ((CELL *)(CharP(ptr) + LDiff)) )
Inline(ChoicePtrAdjust, choiceptr, choiceptr, ptr, ((choiceptr)(CharP(ptr) + LDiff)) )
#ifdef TABLING
Inline(ConsumerChoicePtrAdjust, choiceptr, choiceptr, ptr, ((choiceptr)(CharP(ptr) + LDiff)) )
Inline(GeneratorChoicePtrAdjust, choiceptr, choiceptr, ptr, ((choiceptr)(CharP(ptr) + LDiff)) )
#endif /* TABLING */

Inline(GlobalAdjust, CELL, CELL, val, (val+GDiff) )
Inline(DelayAdjust, CELL, CELL, val, (val+DelayDiff) )
Inline(GlobalAddrAdjust, ADDR, ADDR, ptr, (ptr+GDiff) )
Inline(DelayAddrAdjust, ADDR, ADDR, ptr, (ptr+DelayDiff) )
Inline(LocalAdjust, CELL, CELL, val, (val+LDiff) )
Inline(LocalAddrAdjust, ADDR, ADDR, ptr, (ptr+LDiff) )
Inline(TrailAdjust, CELL, CELL, val, (val+TrDiff) )
Inline(TrailAddrAdjust, ADDR, ADDR, ptr, (ptr+TrDiff) )
/* heap data structures */
Inline(FuncAdjust, Functor, Functor, f, (Functor)(CharP(f)+HDiff) )
Inline(CellPtoHeapAdjust, CELL *, CELL *, ptr, ((CELL *)(CharP(ptr) + HDiff)) )
#if	USE_OFFSETS
Inline(AtomAdjust, Atom, Atom, at, (at) )
Inline(AtomTermAdjust, Term, Term, at, (at) )
Inline(PropAdjust, Prop, Prop, p, (p) )
#else
Inline(AtomAdjust, Atom, Atom, at, (Atom)(CharP(at)+HDiff) )
#if MMAP_ADDR >= 0x40000000
Inline(AtomTermAdjust, Term, Term, at, (at) )
#else
Inline(AtomTermAdjust, Term, Term, at, MkAtomTerm((Atom)(CharP(AtomOfTerm(at)+HDiff))) )
#endif
Inline(PropAdjust, Prop, Prop, p, (Prop)(CharP(p)+HDiff) )
#endif
#if TAGS_FAST_OPS
Inline(BlobTermAdjust, Term, Term, t, (t-HDiff) )
#else
Inline(BlobTermAdjust, Term, Term, t, (t+HDiff) )
#endif
Inline(AtomEntryAdjust, AtomEntry *, AtomEntry *, at, (AtomEntry *)(CharP(at)+HDiff) )
Inline(ConsultObjAdjust, union CONSULT_OBJ *, union CONSULT_OBJ *, co, (union CONSULT_OBJ *)(CharP(co)+HDiff) )
Inline(DBRefAdjust, DBRef, DBRef, dbr, (DBRef)(CharP(dbr)+HDiff) )
Inline(CodeAdjust, Term, Term, dbr, ((Term)(dbr)+HDiff) )
Inline(AddrAdjust, ADDR, ADDR, addr, (ADDR)(CharP(addr)+HDiff) )
Inline(CodeAddrAdjust, CODEADDR, CODEADDR, addr, (CODEADDR)(CharP(addr)+HDiff) )
Inline(BlockAdjust, BlockHeader *, BlockHeader *, addr, (BlockHeader *)(CharP(addr)+HDiff) )
Inline(PtoOpAdjust, yamop *, yamop *, ptr, ((yamop *)(CharP(ptr) + HDiff)) )
Inline(PtoHeapCellAdjust, CELL *, CELL *, ptr, ((CELL *)(CharP(ptr) + HDiff)) )
Inline(PtoPredAdjust, PredEntry *, PredEntry *, ptr, ((CELL *)(CharP(ptr) + HDiff)) )
#if PRECOMPUTE_REGADDRESS
Inline(XAdjust, AREG, AREG, reg, (AREG)((reg)+XDiff) )
#else
Inline(XAdjust, AREG, AREG, reg, (reg) )
#endif
Inline(YAdjust, YREG, YREG, reg, (reg) )

Inline(IsOldLocal, int, CELL, reg, IN_BETWEEN(OldASP, reg, OldLCL0))

/* require because the trail might contain dangling pointers */
Inline(IsOldLocalInTR, int, CELL, reg, IN_BETWEEN(OldH, reg, OldLCL0) )
Inline(IsOldLocalInTRPtr, int, CELL *, ptr, IN_BETWEEN(OldH, ptr, OldLCL0) )

Inline(IsOldH, int, CELL, reg, ( CharP(reg) == CharP(OldH) ) )


Inline(IsOldGlobal, int, CELL, reg, IN_BETWEEN(OldH0, reg, OldH) )
Inline(IsOldGlobalPtr, int, CELL *, ptr, IN_BETWEEN( OldH0, ptr, OldH) )
Inline(IsOldDelay, int, CELL, reg, IN_BETWEEN(OldGlobalBase, reg, OldH0) )
Inline(IsOldDelayPtr, int, CELL *, ptr, IN_BETWEEN( OldGlobalBase, ptr, OldH0) )
Inline(IsOldTrail, int, CELL, reg, IN_BETWEEN(OldTrailBase, reg, OldTR) )
Inline(IsOldTrailPtr, int, CELL *, ptr, IN_BETWEEN(OldTrailBase, ptr, OldTR) )
Inline(IsOldCode, int, CELL, reg, IN_BETWEEN(OldHeapBase, reg, OldHeapTop) )
Inline(IsOldCodeCellPtr, int, CELL *, ptr, IN_BETWEEN(OldHeapBase, ptr, OldHeapTop) )
Inline(IsGlobal, int, CELL, reg, IN_BETWEEN(GlobalBase, reg, H) )

void STD_PROTO(AdjustStacksAndTrail, (void));
void STD_PROTO(AdjustRegs, (int));
