/*  Data Structures and Related Macros
    ==================================  */

/*
 *  Trailing
 *  --------
 *  Record bindings made during the search through the trie so that
 *  these variables can be unbound when an alternate path is explored.
 *  We will use XSB's system Trail to accomodate XSB's unification
 *  algorithm, which is needed at certain points in the collection
 *  process.
 */

#ifdef SUBSUMPTION_XSB

#ifndef MULTI_THREAD
static CPtr *trail_base;    /* ptr to topmost used Cell on the system Trail;
			       the beginning of our local trail for this
			       operation. */

static CPtr *orig_trreg;            
static CPtr orig_hreg;      /* Markers for noting original values of WAM */
static CPtr orig_hbreg;     /* registers so they can be reset upon exiting */
static CPtr orig_ebreg;
#endif

/* 
 * Set backtrack registers to the tops of stacks to force trailing in
 * unify().  Save hreg as the heap may be used to construct bindings.
 */
#define Save_and_Set_WAM_Registers	\
   orig_hbreg = hbreg;			\
   orig_hreg = hbreg = hreg;		\
   orig_ebreg = ebreg;			\
   ebreg = top_of_localstk;		\
   orig_trreg = trreg;			\
   trreg = top_of_trail

#define Restore_WAM_Registers		\
   trreg = orig_trreg;			\
   hreg = orig_hreg;			\
   hbreg = orig_hbreg;			\
   ebreg = orig_ebreg
   
#define Sys_Trail_Unwind(UnwindBase)      table_undo_bindings(UnwindBase)

#else /* YAP */

static tr_fr_ptr trail_base;
static tr_fr_ptr orig_trreg;
static CPtr orig_hreg;
static CPtr orig_hbreg;

#define Save_and_Set_WAM_Registers  \
  orig_hbreg = hbreg; \
  orig_hreg = hbreg = hreg; \
  orig_trreg = trreg;  \
  trreg = top_of_trail
  
#define Restore_WAM_Registers \
  trreg = orig_trreg; \
  hreg = orig_hreg; \
  hbreg = orig_hbreg

#endif /* SUBSUMPTION_XSB */

/* TLS: 12/05.  There was an bug in the routine
   tst_collect_relevant_answers().  This routine used Bind_and_Trail
   or Bind_and_Conditionally Trail (see old copies below) to bind trie
   symbols to variables in the answer template (substitution factor).
   For some reason these macros trailed, but did not actually bind.
   This meant that answers were occasionlly being returned to subsumed
   calls that didn't actually unify with these calls.  It didn't seem
   to happen often -- only when an subsumed call S_d has a
   variable/variable binding that a subsuming call S_g does not have
   AND S_d and S_g are in the same SCC.  Apparently we have not often
   encountered both of these cases at the same time (or tested for
   them).
*/
/*
 *  Create a binding and conditionally trail it.  TrieVarBindings[] cells
 *  are always trailed, while those in the WAM stacks are trailed based on
 *  the traditional trailing test.  As we traverse the TST and lay choice
 *  points, we update the hbreg as in the WAM since structures may be
 *  built on the heap for binding.  Therefore, this condition serves as in
 *  the WAM.
 */

#define Trie_bind_copy(Addr,Val)		\
  Trie_Conditionally_Trail(Addr,Val);		\
  *(Addr) = Val

#define Trie_Conditionally_Trail(Addr, Val)		\
   if ( IsUnboundTrieVar(Addr) || conditional(Addr) )	\
     { pushtrail0(Addr, Val) }

#define Bind_and_Conditionally_Trail(Addr, Val)	Trie_bind_copy(Addr,Val) \

#ifdef SUBSUMPTION_XSB	
/*
 *  Create a binding and trail it.
 */
#define Bind_and_Trail(Addr, Val) pushtrail0(Addr, Val) \
   *(Addr) = Val
#endif /* SUBSUMPTION_XSB */

/*******************************************
OLD VERSIONS
 * #define Bind_and_Trail(Addr, Val)	pushtrail0(Addr, Val)

 * #define Bind_and_Conditionally_Trail(Addr, Val)	\
 *  if ( IsUnboundTrieVar(Addr) || conditional(Addr) )	\
 *    { pushtrail0(Addr, Val) }
 *******************************************/

/* ------------------------------------------------------------------------- */

/*
 *  tstCPStack
 *  ----------
 *  For saving state information so the search may resume from that
 *  point down an alternate path in the time-stamped trie.
 */

#ifndef MULTI_THREAD
static struct tstCPStack_t tstCPStack;
#endif

/* Use these to access the frame to which `top' points */
#define tstCPF_AlternateNode    ((tstCPStack.top)->alt_node)
#define tstCPF_TermStackTopIndex     ((tstCPStack.top)->ts_top_index)
#define tstCPF_TSLogTopIndex         ((tstCPStack.top)->log_top_index)
#define tstCPF_TrailTop         ((tstCPStack.top)->trail_top)
#define tstCPF_HBreg            ((tstCPStack.top)->heap_bktrk)

#define CPStack_IsEmpty    (tstCPStack.top == tstCPStack.base)
#define CPStack_IsFull     (tstCPStack.top == tstCPStack.ceiling)

void initCollectRelevantAnswers(CTXTdecl) {

  tstCPStack.ceiling = tstCPStack.base + TST_CPSTACK_SIZE;
}

#define CPStack_PushFrame(AlternateTSTN)				\
   if ( IsNonNULL(AlternateTSTN) ) {					\
     CPStack_OverflowCheck						\
     tstCPF_AlternateNode = AlternateTSTN;				\
     tstCPF_TermStackTopIndex = TermStack_Top - TermStack_Base + 1;	\
     tstCPF_TSLogTopIndex = TermStackLog_Top - TermStackLog_Base;	\
     tstCPF_TrailTop = trreg;						\
     tstCPF_HBreg = hbreg;						\
     hbreg = hreg;							\
     tstCPStack.top++;							\
   }

#define CPStack_Pop     tstCPStack.top--

/*
 *  Backtracking to a previous juncture in the trie.
 */
#define TST_Backtrack				\
   CPStack_Pop;					\
   ResetParentAndCurrentNodes;			\
   RestoreTermStack;				\
   Sys_Trail_Unwind(tstCPF_TrailTop);		\
   ResetHeap_fromCPF

/*
 *  For continuing with forward execution.  When we match, we continue
 *  the search with the next subterm on the tstTermStack and the children
 *  of the trie node that was matched.
 */
#define Descend_Into_TST_and_Continue_Search	\
   parentTSTN = cur_chain;			\
   cur_chain = TSTN_Child(cur_chain);		\
   goto While_TSnotEmpty

/*
 * Not really necessary to set the parent since it is only needed once a
 * leaf is reached and we step (too far) down into the trie, but that's
 * when its value is set.
 */
#define ResetParentAndCurrentNodes		\
   cur_chain = tstCPF_AlternateNode;		\
   parentTSTN = TSTN_Parent(cur_chain)


#define RestoreTermStack			\
   TermStackLog_Unwind(tstCPF_TSLogTopIndex);	\
   TermStack_SetTOS(tstCPF_TermStackTopIndex)


#define ResetHeap_fromCPF			\
   hreg = hbreg;				\
   hbreg = tstCPF_HBreg


#define CPStack_OverflowCheck						\
   if (CPStack_IsFull)							\
     TST_Collection_Error("tstCPStack overflow.", RequiresCleanup)

/* ========================================================================= */

/*  General Macro Utilities
    =======================  */

/*
 *  Determine whether the TSTN's timestamp allows it to be selected.
 */
#define IsValidTS(SymbolTS,CutoffTS)      (SymbolTS > CutoffTS)

/*
 *  Create a new answer-list node, set it to point to an answer,  
 *  and place it at the head of a chain of answer-list nodes.
 *  For MT engine: use only for private,subsumed tables.
 */
#define ALN_InsertAnswer(pAnsListHead,pAnswerNode) {			\
    ALNptr newAnsListNode;						\
    New_Private_ALN(newAnsListNode,(void *)pAnswerNode,pAnsListHead);	\
    pAnsListHead = newAnsListNode;					\
  }

/*
 *  Error handler for the collection algorithm.
 */
#define RequiresCleanup    TRUE
#define NoCleanupRequired  FALSE

#ifdef SUBSUMPTION_YAP
#define ReturnErrorCode FALSE
#else
#define ReturnErrorCode NULL
#endif /* SUBSUMPTION_YAP */

#define TST_Collection_Error(String, DoesRequireCleanup) {	\
   tstCollectionError(CTXTc String, DoesRequireCleanup);	\
   return ReturnErrorCode;							\
 }

static void tstCollectionError(CTXTdeclc char *string, xsbBool cleanup_needed) {
  fprintf(stderr, "Error encountered in Time-Stamped Trie "
	  "Collection algorithm!\n");
  if (cleanup_needed) {
    Sys_Trail_Unwind(trail_base);
    Restore_WAM_Registers;
  }
  xsb_abort(string);
}


/* ========================================================================= */

/*
 *  Algorithmic Shorthands
 *  ======================
 */


#define backtrack      break


/*
 *  Return the first TSTN in a chain with a valid timestamp (if one exists),
 *  otherwise return NULL.
 */
#define Chain_NextValidTSTN(Chain,TS,tsAccessMacro)			\
   while ( IsNonNULL(Chain) && (! IsValidTS(tsAccessMacro(Chain),TS)) )	\
     Chain = TSTN_Sibling(Chain)

/*
 *  Return the next TSTN in the TSI with a valid timestamp (if one exists),
 *  otherwise return NULL.
 */
#define TSI_NextValidTSTN(ValidTSIN,TS)				\
   ( ( IsNonNULL(TSIN_Next(ValidTSIN)) &&			\
       IsValidTS(TSIN_TimeStamp(TSIN_Next(ValidTSIN)),TS) )	\
     ? TSIN_TSTNode(TSIN_Next(ValidTSIN))			\
     : NULL )

/* ------------------------------------------------------------------------- */

#define SetMatchAndUnifyChains(Symbol,SymChain,VarChain) {	\
								\
   TSTHTptr ht = (TSTHTptr)SymChain;				\
   TSTNptr *buckets = TSTHT_BucketArray(ht);			\
								\
   SymChain = buckets[TrieHash(Symbol,TSTHT_GetHashSeed(ht))];	\
   VarChain = buckets[TRIEVAR_BUCKET];				\
 }

/* ------------------------------------------------------------------------- */

/* 
 *  Exact matches are only looked for in cases where the TSTN is hashed
 *  and the subterm hashes to a bucket different from TRIEVAR_BUCKET.
 *  Once a match has been found, there is no need to search further since
 *  there is only one occurrance of a symbol in any chain.  If the node's
 *  timestamp is valid then the state is saved, with bucket TRIEVAR_BUCKET
 *  as the TSTN continuation, and we branch back to the major loop of the
 *  algorithm.  If the timestamp is not valid, then we exit the loop.  If
 *  no match is found, then execution exits this block with a NULL
 *  cur_chain.  cur_chain may be NULL at block entry.
 *
 *  TermStack_PushOp is provided so that this macro can be used for all
 *  nonvariable symbols.  Constants pass in TermStack_NOOP, functors use
 *  TermStack_PushFunctorArgs(), and lists useTermStack_PushListArgs().
 */

#define SearchChain_ExactMatch(SearchChain,TrieEncodedSubterm,TS,	\
			       ContChain,TermStack_PushOp) 		\
   while ( IsNonNULL(SearchChain) ) {					\
     if ( TrieEncodedSubterm == TSTN_Symbol(SearchChain) ) {		\
       if ( IsValidTS(TSTN_GetTSfromTSIN(SearchChain),TS) ) {		\
	 Chain_NextValidTSTN(ContChain,TS,TSTN_GetTSfromTSIN);		\
	 CPStack_PushFrame(ContChain);					\
	 TermStackLog_PushFrame;					\
	 TermStack_PushOp;						\
	 Descend_Into_TST_and_Continue_Search;				\
       }								\
       else								\
	 break;   /* matching symbol's TS is too old */			\
     }									\
     SearchChain = TSTN_Sibling(SearchChain);				\
   }

/* ------------------------------------------------------------------------- */

/*
 *  Overview:
 *  --------
 *  There are 4 cases when this operation should be used:
 *   1) Searching an unhashed chain.
 *   2) Searching bucket TRIEVAR_BUCKET after searching the hashed-to bucket.
 *   3) Searching bucket TRIEVAR_BUCKET which is also the hashed-to bucket.
 *   4) Searching some hashed chain that has been restored through
 *        backtracking.
 *
 *  (1) and (3) clearly require a general algorithm, capable of dealing
 *  with vars and nonvars alike.  (4) must use this since we may be
 *  continuing an instance of (3).  (2) also requires a deref followed by
 *  inspection, since a derefed variable may (or may not) lead to the
 *  symbol we are interested in.
 *
 *  Detail:
 *  --------
 *  'cur_chain' should be non-NULL upon entry.  Get_TS_Op allows
 *  this code to be used for both hashed and unhashed node chains as
 *  each requires a different procedure for locating a node's timestamp.
 *
 *  Nodes are first pruned by timestamp validity.  If the node's timestamp
 *  is valid and a unification is possible, the state is saved, with
 *  cur_chain's sibling as the TSTN continuation, and we branch back to
 *  the major loop of the algorithm.  Otherwise the chain is searched to
 *  completion, exiting the block when cur_chain is NULL.
 */

#define SearchChain_UnifyWithConstant(Chain,Subterm,TS,Get_TS_Op) {	\
   Chain_NextValidTSTN(Chain,TS,Get_TS_Op);				\
   while ( IsNonNULL(Chain) ) {						\
     alt_chain = TSTN_Sibling(Chain);					\
     Chain_NextValidTSTN(alt_chain,TS,Get_TS_Op);			\
     symbol = TSTN_Symbol(Chain);					\
     TrieSymbol_Deref(symbol);						\
     if ( isref(symbol) ) {						\
       /*								\
	*  Either an unbound TrieVar or some unbound prolog var.	\
	*/								\
       CPStack_PushFrame(alt_chain);					\
       Bind_and_Conditionally_Trail((CPtr)symbol, Subterm);		\
       TermStackLog_PushFrame;						\
       Descend_Into_TST_and_Continue_Search;				\
     }									\
     else if (symbol == Subterm) {					\
       CPStack_PushFrame(alt_chain);					\
       TermStackLog_PushFrame;						\
       Descend_Into_TST_and_Continue_Search;				\
     }									\
     Chain = alt_chain;							\
   }									\
 }
 
#ifdef SUBSUMPTION_YAP
/* ------------------------------------------------------------------------- */

/*
 *  Overview:
 *  --------
 *  There are 4 cases when this operation should be used:
 *   1) Searching an unhashed chain.
 *   2) Searching bucket TRIEVAR_BUCKET after searching the hashed-to bucket.
 *   3) Searching bucket TRIEVAR_BUCKET which is also the hashed-to bucket.
 *   4) Searching some hashed chain that has been restored through
 *        backtracking.
 *
 *  (1) and (3) clearly require a general algorithm, capable of dealing
 *  with vars and nonvars alike.  (4) must use this since we may be
 *  continuing an instance of (3).  (2) also requires a deref followed by
 *  inspection, since a derefed variable may (or may not) lead to the
 *  symbol we are interested in.
 *
 *  Detail:
 *  --------
 *  'cur_chain' should be non-NULL upon entry.  Get_TS_Op allows
 *  this code to be used for both hashed and unhashed node chains as
 *  each requires a different procedure for locating a node's timestamp.
 *
 *  Nodes are first pruned by timestamp validity.  If the node's timestamp
 *  is valid and a unification is possible, the state is saved, with
 *  cur_chain's sibling as the TSTN continuation, and we branch back to
 *  the major loop of the algorithm.  Otherwise the chain is searched to
 *  completion, exiting the block when cur_chain is NULL.
 */
#define SearchChain_UnifyWithLong(Chain,Subterm,TS,Get_TS_Op) { \
  Int li = LongIntOfTerm(Subterm);            \
  Chain_NextValidTSTN(Chain,TS,Get_TS_Op);  \
  while(IsNonNULL(Chain)) { \
    alt_chain = TSTN_Sibling(Chain);  \
    Chain_NextValidTSTN(alt_chain,TS,Get_TS_Op);  \
    symbol = TSTN_Symbol(Chain);  \
    TrieSymbol_Deref(symbol); \
    if(isref(symbol)) { \
      /*                              \
       * Either an unbound TrieVar or some unbound prolog var.  \
       */                             \
      CPStack_PushFrame(alt_chain); \
      Bind_and_Conditionally_Trail((CPtr)symbol, Subterm);  \
      TermStackLog_PushFrame; \
      Descend_Into_TST_and_Continue_Search; \
    } \
    else {  \
      int go = FALSE; \
        \
      if(TrNode_is_long(Chain)) \
        go = TSTN_long_int((long_tst_node_ptr)Chain) == li; \
      else {  \
        if(TrieSymbolType(symbol) == TAG_LONG_INT)  \
          go = LongIntOfTerm(symbol) == li; \
      } \
        \
      if(go) {  \
        CPStack_PushFrame(alt_chain);         \
        TermStackLog_PushFrame; \
        Descend_Into_TST_and_Continue_Search; \
      } \
    } \
    Chain = alt_chain;  \
  } \
}

/* ------------------------------------------------------------------------- */

/*
 *  Overview:
 *  --------
 *  There are 4 cases when this operation should be used:
 *   1) Searching an unhashed chain.
 *   2) Searching bucket TRIEVAR_BUCKET after searching the hashed-to bucket.
 *   3) Searching bucket TRIEVAR_BUCKET which is also the hashed-to bucket.
 *   4) Searching some hashed chain that has been restored through
 *        backtracking.
 *
 *  (1) and (3) clearly require a general algorithm, capable of dealing
 *  with vars and nonvars alike.  (4) must use this since we may be
 *  continuing an instance of (3).  (2) also requires a deref followed by
 *  inspection, since a derefed variable may (or may not) lead to the
 *  symbol we are interested in.
 *
 *  Detail:
 *  --------
 *  'cur_chain' should be non-NULL upon entry.  Get_TS_Op allows
 *  this code to be used for both hashed and unhashed node chains as
 *  each requires a different procedure for locating a node's timestamp.
 *
 *  Nodes are first pruned by timestamp validity.  If the node's timestamp
 *  is valid and a unification is possible, the state is saved, with
 *  cur_chain's sibling as the TSTN continuation, and we branch back to
 *  the major loop of the algorithm.  Otherwise the chain is searched to
 *  completion, exiting the block when cur_chain is NULL.
 */
#define SearchChain_UnifyWithFloat(Chain,Subterm,TS,Get_TS_Op) { \
  Float flt = FloatOfTerm(Subterm);            \
  Chain_NextValidTSTN(Chain,TS,Get_TS_Op);  \
  while(IsNonNULL(Chain)) { \
    alt_chain = TSTN_Sibling(Chain);  \
    Chain_NextValidTSTN(alt_chain,TS,Get_TS_Op);  \
    symbol = TSTN_Symbol(Chain);  \
    TrieSymbol_Deref(symbol); \
    if(isref(symbol)) { \
      /*                              \
       * Either an unbound TrieVar or some unbound prolog var.  \
       */                             \
      CPStack_PushFrame(alt_chain); \
      Bind_and_Conditionally_Trail((CPtr)symbol, Subterm);  \
      TermStackLog_PushFrame; \
      Descend_Into_TST_and_Continue_Search; \
    } \
    else {  \
      int go = FALSE; \
        \
      if(TrNode_is_float(Chain))  \
        go = TSTN_float((float_tst_node_ptr)Chain) == flt;  \
      else {  \
        if(TrieSymbolType(symbol) == TAG_FLOAT) \
          go = FloatOfTerm(symbol) == flt;  \
      } \
        \
      if(go) {  \
        CPStack_PushFrame(alt_chain);         \
        TermStackLog_PushFrame; \
        Descend_Into_TST_and_Continue_Search; \
      } \
    } \
    Chain = alt_chain;  \
  } \
}

#endif /* SUBSUMPTION_YAP */

/* ------------------------------------------------------------------------- */

/*
 *  Overview:
 *  --------
 *  There are 4 cases when this operation should be used:
 *   1) Searching an unhashed chain.
 *   2) Searching bucket TRIEVAR_BUCKET after searching the hashed-to bucket.
 *   3) Searching bucket TRIEVAR_BUCKET which is also the hashed-to bucket.
 *   4) Searching some hashed chain that has been restored through
 *        backtracking.
 *
 *  (1) and (3) clearly require a general algorithm, capable of dealing
 *  with vars and nonvars alike.  (4) must use this since we may be
 *  continuing an instance of (3).  (2) also requires a deref followed by
 *  inspection, since a derefed variable may (or may not) lead to the
 *  symbol we are interested in.
 *
 *  Detail:
 *  --------
 *  'cur_chain' should be non-NULL upon entry.  Get_TS_Op allows
 *  this code to be used for both hashed and unhashed node chains as
 *  each requires a different procedure for locating a node's timestamp.
 *
 *  Nodes are first pruned by timestamp validity.  If the node's timestamp
 *  is valid and a unification is possible, the state is saved, with
 *  cur_chain's sibling as the TSTN continuation, and we branch back to
 *  the major loop of the algorithm.  Otherwise the chain is searched to
 *  completion, exiting the block when cur_chain is NULL.
 */

#define SearchChain_UnifyWithFunctor(Chain,Subterm,TS,Get_TS_Op) {	  \
									  \
   Cell sym_tag;							  \
									  \
   Chain_NextValidTSTN(Chain,TS,Get_TS_Op);				  \
   while ( IsNonNULL(Chain) ) {						  \
     alt_chain = TSTN_Sibling(Chain);					  \
     Chain_NextValidTSTN(alt_chain,TS,Get_TS_Op);			  \
     symbol = TSTN_Symbol(Chain);					  \
     sym_tag = TrieSymbolType(symbol);					  \
     TrieSymbol_Deref(symbol);						  \
     if ( isref(symbol) ) {						  \
       /*								  \
	* Either an unbound TrieVar or some unbound Prolog var.  The	  \
	* variable is bound to the entire subterm (functor + args), so	  \
	* we don't need to process its args; simply continue the search	  \
	* through the trie.						  \
	*/								  \
       CPStack_PushFrame(alt_chain);					  \
       Bind_and_Conditionally_Trail((CPtr)symbol, Subterm);		  \
       TermStackLog_PushFrame;						  \
       Descend_Into_TST_and_Continue_Search;				  \
     }									  \
     else if ( IsTrieFunctor(symbol) ) {				  \
       /*								  \
	* Need to be careful here, because TrieVars may be bound to heap- \
	* resident structures and a deref of the trie symbol doesn't	  \
	* tell you whether we have something in the trie or in the heap.  \
	*/								  \
       if ( sym_tag == XSB_STRUCT ) {					  \
	 if ( get_str_psc(Subterm) == DecodeTrieFunctor(symbol) ) {	  \
	   /*								  \
	    *  We must process the rest of the term ourselves.		  \
	    */								  \
	   CPStack_PushFrame(alt_chain);				  \
	   TermStackLog_PushFrame;					  \
	   TermStack_PushFunctorArgs(Subterm);				  \
	   Descend_Into_TST_and_Continue_Search;			  \
	 }								  \
       }								  \
       else {								  \
	 /*								  \
	  * We have a TrieVar bound to a heap XSB_STRUCT-term; use a	  \
	  * standard unification algorithm to check the match and	  \
	  * perform any additional unification.				  \
	  */								  \
	 if (unify(CTXTc Subterm, symbol)) {				  \
	   CPStack_PushFrame(alt_chain);				  \
	   TermStackLog_PushFrame;					  \
	   Descend_Into_TST_and_Continue_Search;			  \
	 }								  \
       }								  \
     }									  \
     Chain = alt_chain;							  \
   }									  \
 }

/* ------------------------------------------------------------------------- */

/*
 *  Overview:
 *  --------
 *  Since in hashing environments LISTs only live in bucket 0, as do
 *  variables, there are only 2 cases to consider:
 *   1) Searching an unhashed chain (or subchain, via backtracking).
 *   2) Searching a hashed chain (or subchain, via backtracking).
 *
 *  Although there is at most only one LIST instance in a chain, there
 *  may be many variables, with both groups appearing in any order.
 *  Hence there is always the need to handle variables, which must be
 *  dereferenced to determine whether they are bound, and if so,
 *  whether they are bound to some LIST.
 *
 *  Detail:
 *  --------
 *  'cur_chain' should be non-NULL upon entry.  Get_TS_Op allows
 *  this code to be used for both hashed and unhashed node chains as
 *  each requires a different procedure for locating a node's timestamp.
 *
 *  Nodes are first pruned by timestamp validity.  If the node's timestamp
 *  is valid and a unification is possible, the state is saved, with
 *  cur_chain's sibling as the TSTN continuation, and we branch back to
 *  the major loop of the algorithm.  Otherwise the chain is searched to
 *  completion, exiting the block when cur_chain is NULL.
 */

#define SearchChain_UnifyWithList(Chain,Subterm,TS,Get_TS_Op) {		\
									\
   Cell sym_tag;							\
									\
   Chain_NextValidTSTN(Chain,TS,Get_TS_Op);				\
   while ( IsNonNULL(Chain) ) {						\
     alt_chain = TSTN_Sibling(Chain);					\
     Chain_NextValidTSTN(alt_chain,TS,Get_TS_Op);			\
     symbol = TSTN_Symbol(Chain);					\
     sym_tag = TrieSymbolType(symbol);					\
     TrieSymbol_Deref(symbol);						\
     if ( isref(symbol) ) {						\
       /*								\
	* Either an unbound TrieVar or some unbound Prolog var.  The	\
	* variable is bound to the entire subterm ([First | Rest]), so	\
	* we don't need to process its args; simply continue the	\
	* search through the trie.					\
	*/								\
       CPStack_PushFrame(alt_chain);					\
       Bind_and_Conditionally_Trail((CPtr)symbol,Subterm);		\
       TermStackLog_PushFrame;						\
       Descend_Into_TST_and_Continue_Search;				\
     }									\
     else if ( IsTrieList(symbol) ) {					\
       /*								\
	* Need to be careful here, because XSB_TrieVars are bound to	\
	* heap- resident structures and a deref of the (trie) symbol	\
	* doesn't tell you whether we have something in the trie or in	\
	* the heap.							\
	*/								\
       if ( sym_tag == XSB_LIST ) {					\
	 /*								\
	  *  We must process the rest of the term ourselves.		\
	  */								\
	 CPStack_PushFrame(alt_chain);					\
	 TermStackLog_PushFrame;					\
	 TermStack_PushListArgs(Subterm);				\
	 Descend_Into_TST_and_Continue_Search;				\
       }								\
       else {								\
	 /*								\
	  * We have a XSB_TrieVar bound to a heap LIST-term; use a	\
	  * standard unification algorithm to check the match.		\
	  */								\
	 if (unify(CTXTc Subterm, symbol)) {				\
	   CPStack_PushFrame(alt_chain);				\
	   TermStackLog_PushFrame;					\
	   Descend_Into_TST_and_Continue_Search;			\
	 }								\
       }								\
     }									\
     Chain = alt_chain;							\
   }									\
 }
 
/* ------------------------------------------------------------------------- */

/*
 *  Unify the timestamp-valid node 'cur_chain' with the variable subterm.
 */

static inline
xsbBool
Unify_with_Variable(CTXTdeclc Cell symbol, Cell subterm, TSTNptr node) {
  switch(TrieSymbolType(symbol)) {
   case XSB_INT:
#ifdef SUBSUMPTION_XSB
   case XSB_FLOAT:
#endif
   case XSB_STRING:
     /*     printf("before %x %x \n",(CPtr)Subterm,symbol);	*/
     Trie_bind_copy((CPtr)subterm,symbol);
     /*     printf("after %x\n",*(CPtr)Subterm);		*/
     break;

   case XSB_STRUCT:
     /*
      * Need to be careful here, because TrieVars are bound to heap-
      * resident structures and a deref of the (trie) symbol doesn't
      * tell you whether we have something in the trie or in the heap.
      */
     if ( IsTrieFunctor(TSTN_Symbol(node)) ) {
       /*
	      * Since the TSTN contains some f/n, create an f(X1,X2,...,Xn)
	      * structure on the heap so that we can bind the subterm
	      * variable to it.  Then use this algorithm to find bindings
	      * for the unbound variables X1,...,Xn in the trie.
	      */
	     Trie_bind_copy((CPtr)subterm,(Cell)hreg);
       CreateHeapFunctor(symbol);
     }
     else {
       /*
	      * We have a TrieVar bound to a heap-resident STRUCT.
	      */
       Trie_bind_copy((CPtr)subterm,symbol);
     }
     break;

   case XSB_LIST:
     if ( IsTrieList(TSTN_Symbol(node)) ) {
       /*
	      * Since the TSTN contains a (sub)list beginning, create a
	      * [X1|X2] structure on the heap so that we can bind the
	      * subterm variable to it.  Then use this algorithm to find
	      * bindings for the unbound variables X1 & X2 in the trie.
	      */
       Trie_bind_copy((CPtr)subterm,(Cell)hreg);
       CreateHeapList();
     }
     else {
       /*
	      * We have a TrieVar bound to a heap-resident LIST.
	      */
       Trie_bind_copy((CPtr)subterm,symbol);
     }
     break;

   case XSB_REF:
#ifdef SUBSUMPTION_XSB
   case XSB_REF1:
#endif
     /*
      * The symbol is either an unbound TrieVar or some unbound Prolog
      * variable.  If it's an unbound TrieVar, we bind it to the
      * Prolog var.  Otherwise, binding direction is WAM-defined.
      */
     if (IsUnboundTrieVar(symbol)) {
       Bind_and_Trail((CPtr)symbol,subterm);
     }
     else
       unify(CTXTc symbol,subterm);
     break;
#ifdef SUBSUMPTION_YAP
  case TAG_LONG_INT:
    /*
     * Need to be careful here, because TrieVars are bound to heap-
     * resident structures and a deref of the (trie) symbol doesn't
     * tell you whether we have something in the trie or in the heap.
     */
    if(TrNode_is_long(node)) {
      /*
       * Create an heap resident long and
       * bind the variable to it
       */
      Trie_bind_copy((CPtr)subterm,(Cell)hreg);
      CreateHeapLongInt(TSTN_long_int((long_tst_node_ptr)node));
    } else {
      /* TrieVar bound to a heap resident long int */
      Trie_bind_copy((CPtr)subterm, symbol);
    }
    break;
  case TAG_FLOAT:
    /*
     * Need to be careful here, because TrieVars are bound to heap-
     * resident structures and a deref of the (trie) symbol doesn't
     * tell you whether we have something in the trie or in the heap.
     */
    if(TrNode_is_float(node)) {
      /*
       * Create an heap resident float and
       * bind the variable to it
       */
      Trie_bind_copy((CPtr)subterm,(Cell)hreg);
      CreateHeapFloat(TSTN_float((float_tst_node_ptr)node));
    } else {
      /* TrieVar bound to a heap resident float */
      Trie_bind_copy((CPtr)subterm, symbol);
    }
    break;
#endif /* SUBSUMPTION_YAP */
   default:
     return FALSE;
   }  /* END switch(symbol_tag) */
   return TRUE;
}

#define CurrentTSTN_UnifyWithVariable(Chain,Subterm,Continuation)	   \
   CPStack_PushFrame(Continuation);					   \
   TermStackLog_PushFrame;						   \
   symbol = TSTN_Symbol(Chain);						   \
   TrieSymbol_Deref(symbol);						   \
   if(!Unify_with_Variable(CTXTc symbol,Subterm,Chain)) { \
     fprintf(stderr, "subterm: unbound var (%ld),  symbol: unknown "	   \
 	     "(%ld)\n", (long int)cell_tag(Subterm), (long int)TrieSymbolType(symbol));	   \
      TST_Collection_Error("Trie symbol with bogus tag!", RequiresCleanup); \
   } \
   Descend_Into_TST_and_Continue_Search


/* ========================================================================= */


/*
 * Purpose:
 * -------
 *  From a given Time-Stamped Answer Trie, collect those answers with
 *  timestamps greater than a given value and which unify with a given
 *  template.  The unifying answers are returned in a chain of Answer
 *  List Nodes.
 *  Note that this algorithm relies on the Time Stamp Indices of the
 *  TST (which are reclaimed from the table when a subgoal completes).
 *
 * Method:
 * ------
 *  Backtrack through the entire TST, using the TimeStamp to prune paths.
 *
 * Nefarious Detail (not integral to general understanding)
 * ----------------
 *  Only when we succeed with a match do we push a subterm onto the
 *  tstTermStackLog.  This is because if we don't succeed, we backtrack,
 *  which would mean we pushed it onto the tstTermStackLog just to be
 *  popped off and stored back in the tstTermStack, and in fact back to
 *  the same location where it already resides (it wouldn't have had a
 *  chance to be overwritten).
 *
 *  When we do succeed, we would like to record the subterm just
 *  consumed, but not any bindings created as a result of the match.
 *  In the code, we push a CPF before doing any of this recording.
 *  However, the log info is, in fact, saved.  */

#ifdef SUBSUMPTION_XSB
ALNptr tst_collect_relevant_answers(CTXTdeclc TSTNptr tstRoot, TimeStamp ts,
				    int numTerms, CPtr termsRev)
#else
xsbBool tst_collect_relevant_answers(CTXTdeclc TSTNptr tstRoot, TimeStamp ts,
            int numTerms, CPtr termsRev, subcons_fr_ptr sg_fr)
#endif /* SUBSUMPTION_XSB */
{

  /* numTerms -- size of Answer Template */
  /* termsRev -- Answer template (on heap) */

#ifdef SUBSUMPTION_YAP
  xsbBool any_answers;
  continuation_ptr first, last;
#else
  ALNptr tstAnswerList;  /* for collecting leaves to be returned */
#endif /* SUBSUMPTION_YAP */


  TSTNptr cur_chain;     /* main ptr for stepping through siblings; under
			    normal (non-hashed) circumstances, variable and
			    non-variable symbols will appear in the same
			    chain */
  TSTNptr alt_chain;     /* special case ptr used for stepping through
			      siblings while looking for a match with
			      subterm */
  TSTNptr parentTSTN;    /* the parent of TSTNs in cur_ and alt_chain */

  Cell subterm;          /* the part of the term we are inspecting */
  Cell symbol;



  /* Check that a term was passed in
     ------------------------------- */
  if (numTerms < 1)
    TST_Collection_Error("Called with < 1 terms",NoCleanupRequired);


  /* Initialize data structures
     -------------------------- */
  TermStack_ResetTOS;
  TermStack_PushHighToLowVector(termsRev,numTerms);  /* AnsTempl -> TermStack */
  TermStackLog_ResetTOS;
  tstCPStack.top = tstCPStack.base;
  trail_base = top_of_trail;
  Save_and_Set_WAM_Registers;

  parentTSTN = tstRoot;
  cur_chain = TSTN_Child(tstRoot);
#ifdef SUBSUMPTION_XSB
  tstAnswerList = NULL;
#else
  first = last = NULL;
  any_answers = FALSE;
#endif /* SUBSUMPTION_XSB */
  symbol = 0;   /* suppress compiler warning */


  /* Major loop of the algorithm
     --------------------------- */

 While_TSnotEmpty:
  while ( ! TermStack_IsEmpty ) {
    TermStack_Pop(subterm);
    XSB_Deref(subterm);
    switch(cell_tag(subterm)) {
    /* SUBTERM IS A CONSTANT
       --------------------- */
    case XSB_INT:
#ifdef SUBSUMPTION_XSB
		case XSB_FLOAT:
#endif
    case XSB_STRING:
      /*
       *  NOTE:  A Trie constant looks like a Prolog constant.
       */
      if ( IsHashHeader(cur_chain) ) {
	      symbol = EncodeTrieConstant(subterm);
	      SetMatchAndUnifyChains(symbol,cur_chain,alt_chain);
	      if ( cur_chain != alt_chain ) {
	        SearchChain_ExactMatch(cur_chain,symbol,ts,alt_chain,
				      TermStack_NOOP);
	        cur_chain = alt_chain;
	      }
	      if ( IsNULL(cur_chain) )
	        backtrack;
      }
      if ( IsHashedNode(cur_chain) )
	      SearchChain_UnifyWithConstant(cur_chain,subterm,ts,
				      TSTN_GetTSfromTSIN)
      else
	      SearchChain_UnifyWithConstant(cur_chain,subterm,ts,TSTN_TimeStamp)
      break;

    /* SUBTERM IS A STRUCTURE
       ---------------------- */
    case XSB_STRUCT:
      /*
       *  NOTE:  A trie XSB_STRUCT is a XSB_STRUCT-tagged PSC ptr,
       *  while a heap XSB_STRUCT is a XSB_STRUCT-tagged ptr to a PSC ptr.
       */
      if ( IsHashHeader(cur_chain) ) {
	symbol = EncodeTrieFunctor(subterm);
	SetMatchAndUnifyChains(symbol,cur_chain,alt_chain);
	if ( cur_chain != alt_chain ) {
	  SearchChain_ExactMatch(cur_chain,symbol,ts,alt_chain,
				 TermStack_PushFunctorArgs(subterm));
	  cur_chain = alt_chain;
	}
	if ( IsNULL(cur_chain) )
	  backtrack;
      }
      if ( IsHashedNode(cur_chain) )
	SearchChain_UnifyWithFunctor(cur_chain,subterm,ts,TSTN_GetTSfromTSIN)
      else
	SearchChain_UnifyWithFunctor(cur_chain,subterm,ts,TSTN_TimeStamp)
      break;
      
    /* SUBTERM IS A LIST
       ----------------- */
    case XSB_LIST:
      /*
       *  NOTE:  A trie XSB_LIST uses a plain XSB_LIST tag wherever a recursive
       *         substructure begins, while a heap XSB_LIST uses a XSB_LIST-
       *         tagged ptr to a pair of Cells, the first being the head
       *         and the second being the recursive tail, another XSB_LIST-
       *         tagged ptr.
       */
      if ( IsHashHeader(cur_chain) ) {
	symbol = EncodeTrieList(subterm);
	SetMatchAndUnifyChains(symbol,cur_chain,alt_chain);
	if ( cur_chain != alt_chain ) {
	  SearchChain_ExactMatch(cur_chain,symbol,ts,alt_chain,
				 TermStack_PushListArgs(subterm));
	  cur_chain = alt_chain;
	}
	if ( IsNULL(cur_chain) )
	  backtrack;
      }
      if ( IsHashedNode(cur_chain) )
	SearchChain_UnifyWithList(cur_chain,subterm,ts,TSTN_GetTSfromTSIN)
      else
	SearchChain_UnifyWithList(cur_chain,subterm,ts,TSTN_TimeStamp)
      break;
      
    /* SUBTERM IS AN UNBOUND VARIABLE
       ------------------------------ */
    case XSB_REF:
#ifdef SUBSUMPTION_XSB
    case XSB_REF1:
#endif
      /*
       *  Since variables unify with any term, only prune based on
       *  timestamps.  For Hashed/HashRoot nodes we can use the TSI to
       *  prune timestamp-invalid nodes immediately, and so we search for
       *  timestamp-valid nodes for both cur_chain and alt_chain.  (If
       *  one cannot be found for alt_chain, then there is no reason, at a
       *  future time, to backtrack to this state.  Hence, alt_chain is
       *  given the value of NULL so that no CP is created.)  For an
       *  unhashed chain, we cannot use this trick, and so must pick them
       *  out of the chain via a linear search.  In fact, we only require
       *  cur_chain to be valid in this case.  In all cases, if a valid
       *  node cannot be found (for cur_chain), we backtrack.
       */
      if ( IsHashedNode(cur_chain) )
	/*
	 *  Can only be here via backtracking...
	 *  cur_chain should be valid by virtue that we only save valid
	 *  hashed alt_chains.  Find the next valid TSTN in the chain.
	 */
	alt_chain = TSI_NextValidTSTN(TSTN_GetTSIN(cur_chain),ts);
      else if ( IsHashHeader(cur_chain) ) {
	/* Can only be here if stepping down onto this level... */
	TSINptr tsin = TSTHT_IndexHead((TSTHTptr)cur_chain);

	if ( IsNULL(tsin) )
	  TST_Collection_Error("TSI Structures don't exist", RequiresCleanup);
	if ( IsValidTS(TSIN_TimeStamp(tsin),ts) ) {
	  cur_chain = TSIN_TSTNode(tsin);
	  alt_chain = TSI_NextValidTSTN(tsin,ts);
	}
	else
	  backtrack;
      }
      else {
	/*
	 *  Can get here through forword OR backward execution...
	 *  Find the next timestamp-valid node in this UnHashed chain.
	 */
	Chain_NextValidTSTN(cur_chain,ts,TSTN_TimeStamp);
	if ( IsNULL(cur_chain) )
	  backtrack;
	alt_chain = TSTN_Sibling(cur_chain);
	Chain_NextValidTSTN(alt_chain,ts,TSTN_TimeStamp);
      }
      CurrentTSTN_UnifyWithVariable(cur_chain,subterm,alt_chain);
      break;
#ifdef SUBSUMPTION_YAP
      /* SUBTERM IS A LONG INT
       --------------------- */
    case TAG_LONG_INT:
      if(IsHashHeader(cur_chain)) {
        Int li = LongIntOfTerm(subterm);
        
        SetMatchAndUnifyChains((Term)li,cur_chain,alt_chain);
        
        if(cur_chain != alt_chain) {
          while(IsNonNULL(cur_chain)) {
            if(TrNode_is_long(cur_chain) && TSTN_long_int((long_tst_node_ptr)cur_chain) == li) {
              if(IsValidTS(TSTN_GetTSfromTSIN(cur_chain), ts)) {
                Chain_NextValidTSTN(alt_chain, ts, TSTN_GetTSfromTSIN);
                CPStack_PushFrame(alt_chain);
                TermStackLog_PushFrame;
                Descend_Into_TST_and_Continue_Search;
              }
              else
                break; /* matching long int TS is too old */
            }
            cur_chain = TSTN_Sibling(cur_chain);
          }
          cur_chain = alt_chain;
        }
        if(IsNULL(cur_chain))
          backtrack;
      }
      if(IsHashedNode(cur_chain))
        SearchChain_UnifyWithLong(cur_chain,subterm,ts,
          TSTN_GetTSfromTSIN)
      else
        SearchChain_UnifyWithLong(cur_chain,subterm,ts,TSTN_TimeStamp)
      break;
    /* SUBTERM IS A FLOAT
       ------------------ */
    case TAG_FLOAT:
      dprintf("Float\n");
      if(IsHashHeader(cur_chain)) {
        Float flt = FloatOfTerm(subterm);
        
        SetMatchAndUnifyChains((Term)flt,cur_chain,alt_chain);

        if(cur_chain != alt_chain) {  
          dprintf("Flt %lf\n", flt);
          while(IsNonNULL(cur_chain)) {
            if(TrNode_is_float(cur_chain) && TSTN_float((float_tst_node_ptr)cur_chain) == flt) {
              if(IsValidTS(TSTN_GetTSfromTSIN(cur_chain), ts)) {
                Chain_NextValidTSTN(alt_chain, ts, TSTN_GetTSfromTSIN);
                CPStack_PushFrame(alt_chain);
                TermStackLog_PushFrame;
                Descend_Into_TST_and_Continue_Search;
              }
              else
                break; /* matching float TS is too old */
            }
            cur_chain = TSTN_Sibling(cur_chain);
          }
          cur_chain = alt_chain;
        }
        if(IsNULL(cur_chain))
          backtrack;
      }
      if(IsHashedNode(cur_chain))
        SearchChain_UnifyWithFloat(cur_chain,subterm,ts,TSTN_GetTSfromTSIN)
      else
        SearchChain_UnifyWithFloat(cur_chain,subterm,ts,TSTN_TimeStamp)
      break;
#endif /* SUBSUMPTION_YAP */

    default:
      fprintf(stderr, "subterm: unknown (%ld),  symbol: ? (%ld)\n",
	      (long int)cell_tag(subterm), (long int)TrieSymbolType(symbol));
      TST_Collection_Error("Trie symbol with bogus tag!", RequiresCleanup);
      break;
    } /* END switch(subterm_tag) */

    /*
     *  We've exhausted the possibilities of the subbranch at this level,
     *  and so need to backtrack to continue the search.  If there are no
     *  remaining choice point frames, then the TST has been completely
     *  searched and we return any answers found.
     */

    if ( CPStack_IsEmpty ) {
      goto end_retrv;
    }
    TST_Backtrack;

  } /* END while( ! TermStack_IsEmpty ) */

  /*
   *  If the tstTermStack is empty, then we (should have) reached a leaf
   *  node whose corresponding term unifies with the Heap Term 'term'.  If
   *  a leaf is not reached, then we generate an error msg, and try to
   *  continue.
   */

  if ( ! IsLeafNode(parentTSTN) ) {
    xsb_warn("During collection of relevant answers for subsumed subgoal\n"
	     "TermStack is empty but a leaf node was not reached");
    xsb_dbgmsg((LOG_DEBUG, "Root "));
    dbg_printTrieNode(LOG_DEBUG, stddbg, (BTNptr)tstRoot);
    xsb_dbgmsg((LOG_DEBUG, "Last "));
    dbg_printTrieNode(LOG_DEBUG, stddbg, (BTNptr)parentTSTN);
    dbg_printAnswerTemplate(LOG_DEBUG, stddbg, termsRev,numTerms);
    xsb_dbgmsg((LOG_DEBUG,
	    "(* Note: this template may be partially instantiated *)\n"));
    fprintf(stdwarn, "Attempting to continue...\n");
  }
  else {
#ifdef SUBSUMPTION_XSB
    //    printf("ans   ");printTriePath(stderr,parentTSTN,NO);printf("\n");
    ALN_InsertAnswer(tstAnswerList, parentTSTN);
#else
    if(!any_answers) {
      any_answers = TRUE;
      first = SgFr_first_answer(sg_fr);
      last = SgFr_last_answer(sg_fr);
    }
    push_new_answer_set(parentTSTN, first, last);
#endif /* SUBSUMPTION_XSB */
  }
  if ( CPStack_IsEmpty ) {
    goto end_retrv;
  }
  TST_Backtrack;
  goto While_TSnotEmpty;
end_retrv:
#ifdef SUBSUMPTION_YAP
  if(any_answers) {
    SgFr_first_answer(sg_fr) = first;
    SgFr_last_answer(sg_fr) = last;
  }
#endif /* SUBSUMPTION_YAP */
  Sys_Trail_Unwind(trail_base);
  Restore_WAM_Registers;
#ifdef SUBSUMPTION_YAP
  return any_answers;
#else
  return tstAnswerList;
#endif /* SUBSUMPTION_YAP */
}