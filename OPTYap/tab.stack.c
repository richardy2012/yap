/**********************************************************************
                                                               
                       The OPTYap Prolog system                
  OPTYap extends the Yap Prolog system to support or-parallel tabling
                                                               
  Copyright:   R. Rocha and NCC - University of Porto, Portugal
  File:        tab.stack.c
                                                                     
**********************************************************************/

#include "opt.config.h"

#ifdef TABLING_CALL_SUBSUMPTION

#include <stdio.h>
#include <stdlib.h>
#include "tab.stack.h"

void dynamic_stack_print(DynamicStack ds)
{
  fprintf(stdout, "Dynamic Stack: %s\n"
      " Stack Base: %8p\tFrame Size:  %u bytes\n"
      " Stack Top:  %8p\tCurrent Size: %u frames\n"
      " Stack Ceiling: %8p\tInitial Size: %u frames",
      DynStk_Name(ds),
      DynStk_Base(ds), DynStk_FrameSize(ds),
      DynStk_Top(ds), DynStk_CurSize(ds),
      DynStk_Ceiling(ds), DynStk_InitSize(ds));
}

void dynamic_stack_init(DynamicStack *ds, size_t stack_size, size_t frame_size, char* name)
{
  size_t total_bytes = stack_size * frame_size;
  
  DynStk_Base(*ds) = malloc(total_bytes); // XXX
  DynStk_Top(*ds) = DynStk_Base(*ds);
  DynStk_Ceiling(*ds) = (char*)DynStk_Base(*ds) + total_bytes;
  DynStk_FrameSize(*ds) = frame_size;
  DynStk_InitSize(*ds) = DynStk_CurSize(*ds) = stack_size;
  DynStk_Name(*ds) = name;
}

#endif /* TABLING_CALL_SUBSUMPTION */