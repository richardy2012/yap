/**********************************************************************
                                                               
                       The OPTYap Prolog system                
  OPTYap extends the Yap Prolog system to support or-parallel tabling
                                                               
  Copyright:   Flávio Cruz - University of Porto, Portugal
  File:        tab.sub_structs.h
  version:      
                                                                     
**********************************************************************/

/* THIS FILE MUST BE INCLUDED THROUGH tab.structs.h */

#ifdef TABLING_CALL_SUBSUMPTION

#define AT_BLOCK_SIZE 8

typedef struct sub_subgoal_trie_node {
  struct subgoal_trie_node base;
  unsigned num_generators;
} *subg_node_ptr;

#define TrNode_num_gen(X)     ((X)->num_generators)
#define TrNode_index_node(X)  ((gen_index_ptr)TrNode_num_gen(X))

EXTEND_STRUCT(sub_subgoal_trie_node, subg_node_ptr, long, Int long_int);
EXTEND_STRUCT(sub_subgoal_trie_node, subg_node_ptr, float, Float float_val);

/* ----------------------- **
** generator indexes       **
** ----------------------- */

typedef struct gen_index_node *gen_index_ptr;
struct gen_index_node {
  gen_index_ptr prev;
  gen_index_ptr next;
  unsigned num_generators;
  subg_node_ptr node;
};

#define GNIN_node(X)        ((X)->node)
#define GNIN_num_gen(X)     ((X)->num_generators)
#define GNIN_prev(X)        ((X)->prev)
#define GNIN_next(X)        ((X)->next)

/* ---------------------------- **
** subgoal hash for subsumption **
** ---------------------------- */
typedef struct sub_subgoal_trie_hash {
  struct subgoal_trie_hash base;
  gen_index_ptr index_head;
} *subg_hash_ptr;

#define Hash_index_head(X)    ((X)->index_head)

/* ---------------------------- **
**       subgoal frames         **
** ---------------------------- */

typedef struct subsumed_consumer_subgoal_frame subsumptive_consumer_sf;
typedef struct subsumptive_producer_subgoal_frame {
  variant_sf var_sf;
  subsumptive_consumer_sf *consumers; /* List of properly subsumed subgoals */
} subsumptive_producer_sf;

typedef subsumptive_producer_sf *subprod_fr_ptr;

#define SgFr_prod_consumers(X) ((X)->consumers)
#define SgFr_prod_timestamp(X) TSTN_time_stamp((tst_node_ptr)SgFr_answer_trie(X))
#define SgFr_subsumes_subgoals(X) \
  (SgFr_is_sub_producer(X) && SgFr_prod_consumers(X) != NULL)

struct subsumed_consumer_subgoal_frame {
  subgoal_frame_type type; /* subgoal frame type */
  subgoal_state state_flag;
  
  yamop *code_of_subgoal;
  
#if defined(YAPOR) || defined(THREADS)
  lockvar lock;
#endif
  
  sg_node_ptr leaf_ptr;
  
  /* first consumer choice point */
  choiceptr choice_point;

  continuation_ptr first_answer;
  continuation_ptr last_answer;
  
  struct subsumed_consumer_subgoal_frame *next;
  
  time_stamp ts;
  subprod_fr_ptr producer;
  CELL* answer_template;
  int at_size;
  CELL at_block[AT_BLOCK_SIZE];
  
  /* Chain link for properly subsumed subgoals */
  subsumptive_consumer_sf *consumers;
};

typedef subsumptive_consumer_sf *subcons_fr_ptr;

#define SgFr_timestamp(X)       ((X)->ts)
#define SgFr_producer(X)        ((X)->producer)
#define SgFr_consumers(X)       ((X)->consumers)
#define SgFr_answer_template(X) ((X)->answer_template)
#define SgFr_at_size(X)         ((X)->at_size)
#define SgFr_at_block(X)        ((X)->at_block)

typedef struct grounded_subgoal_frame *grounded_sf_ptr;

struct grounded_subgoal_frame {
  subgoal_frame_type type;
  subgoal_state state_flag;
  
  yamop *code_of_subgoal;
  
#if defined(YAPOR) || defined(THREADS)
  lockvar lock;
#endif

  sg_node_ptr leaf_ptr;
  
  choiceptr choice_point;
  
  continuation_ptr first_answer;
  continuation_ptr last_answer;
  
  struct grounded_subgoal_frame *next;
  
  time_stamp ts;
  grounded_sf_ptr producer;
  CELL* answer_template;
  int at_size;
  CELL at_block[AT_BLOCK_SIZE];
  
  continuation_ptr try_answer;
  
  int is_most_general;
};

#define SgFr_is_most_general(X)     ((X)->is_most_general)

#define TabEnt_ground_trie(X)       (TrNode_next(TabEnt_subgoal_trie(X)))
#define TabEnt_has_ground_trie(X)   (TabEnt_ground_trie(X) != NULL)
#define TabEnt_ground_time_stamp(X) (TabEnt_ground_trie(X) ? \
                                      TSTN_time_stamp((tst_node_ptr)TabEnt_ground_trie(X))  \
                                      : 0)
#define TabEnt_proper_consumers(X)  (TrNode_parent(TabEnt_subgoal_trie(X)))

#define GROUND_SUBGOAL_FRAME_MASK 0xC0

#define SgFr_is_ground(X)          (SgFr_type(X) & GROUND_SUBGOAL_FRAME_MASK)
#define SgFr_is_ground_producer(X) (SgFr_type(X) == GROUND_PRODUCER_SFT)
#define SgFr_is_ground_consumer(X) (SgFr_type(X) == GROUND_CONSUMER_SFT)

#endif /* TABLING_CALL_SUBSUMPTION */