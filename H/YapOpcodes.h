
  /* This file was generated automatically by "yap -L misc/buildops"
     please do not update */

  OPCODE(Ystop                      ,l),
  OPCODE(Nstop                      ,e),
  OPCODE(try_me                     ,Otapl),
  OPCODE(retry_me                   ,Otapl),
  OPCODE(trust_me                   ,Otapl),
  OPCODE(enter_profiling            ,p),
  OPCODE(retry_profiled             ,p),
  OPCODE(profiled_retry_me          ,Otapl),
  OPCODE(profiled_trust_me          ,Otapl),
  OPCODE(profiled_retry_logical     ,OtaLl),
  OPCODE(profiled_trust_logical     ,OtILl),
  OPCODE(count_call                 ,p),
  OPCODE(count_retry                ,p),
  OPCODE(count_retry_me             ,Otapl),
  OPCODE(count_trust_me             ,Otapl),
  OPCODE(count_retry_logical        ,OtaLl),
  OPCODE(count_trust_logical        ,OtILl),
  OPCODE(lock_lu                    ,p),
  OPCODE(unlock_lu                  ,e),
  OPCODE(alloc_for_logical_pred     ,L),
  OPCODE(copy_idb_term              ,e),
  OPCODE(unify_idb_term             ,e),
  OPCODE(spy_or_trymark             ,Otapl),
  OPCODE(try_and_mark               ,Otapl),
  OPCODE(count_retry_and_mark       ,Otapl),
  OPCODE(profiled_retry_and_mark    ,Otapl),
  OPCODE(retry_and_mark             ,Otapl),
  OPCODE(trust_fail                 ,e),
  OPCODE(op_fail                    ,e),
  OPCODE(cut                        ,e),
  OPCODE(cut_t                      ,e),
  OPCODE(cut_e                      ,e),
  OPCODE(save_b_x                   ,x),
  OPCODE(save_b_y                   ,y),
  OPCODE(commit_b_x                 ,xp),
  OPCODE(commit_b_y                 ,yp),
  OPCODE(execute                    ,pp),
  OPCODE(dexecute                   ,pp),
  OPCODE(fcall                      ,Osbpp),
  OPCODE(call                       ,Osbpp),
  OPCODE(procceed                   ,p),
  OPCODE(allocate                   ,e),
  OPCODE(deallocate                 ,p),
#ifdef BEAM
  OPCODE(retry_eam                  ,e),
#endif
#ifdef BEAM
  OPCODE(run_eam                    ,os),
#endif
  OPCODE(get_x_var                  ,xx),
  OPCODE(get_y_var                  ,yx),
  OPCODE(get_yy_var                 ,yyxx),
  OPCODE(get_x_val                  ,xx),
  OPCODE(get_y_val                  ,yx),
  OPCODE(get_atom                   ,xc),
  OPCODE(get_2atoms                 ,cc),
  OPCODE(get_3atoms                 ,ccc),
  OPCODE(get_4atoms                 ,cccc),
  OPCODE(get_5atoms                 ,ccccc),
  OPCODE(get_6atoms                 ,cccccc),
  OPCODE(get_list                   ,x),
  OPCODE(get_struct                 ,xfa),
  OPCODE(get_float                  ,xd),
  OPCODE(get_longint                ,xi),
  OPCODE(get_bigint                 ,xc),
  OPCODE(get_dbterm                 ,xc),
  OPCODE(glist_valx                 ,xx),
  OPCODE(glist_valy                 ,yx),
  OPCODE(gl_void_varx               ,xx),
  OPCODE(gl_void_vary               ,yx),
  OPCODE(gl_void_valx               ,xx),
  OPCODE(gl_void_valy               ,yx),
  OPCODE(unify_x_var                ,ox),
  OPCODE(unify_x_var_write          ,ox),
  OPCODE(unify_l_x_var              ,ox),
  OPCODE(unify_l_x_var_write        ,ox),
  OPCODE(unify_x_var2               ,oxx),
  OPCODE(unify_x_var2_write         ,oxx),
  OPCODE(unify_l_x_var2             ,oxx),
  OPCODE(unify_l_x_var2_write       ,oxx),
  OPCODE(unify_y_var                ,oy),
  OPCODE(unify_y_var_write          ,oy),
  OPCODE(unify_l_y_var              ,oy),
  OPCODE(unify_l_y_var_write        ,oy),
  OPCODE(unify_x_val                ,ox),
  OPCODE(unify_x_val_write          ,ox),
  OPCODE(unify_l_x_val              ,ox),
  OPCODE(unify_l_x_val_write        ,ox),
  OPCODE(unify_y_val                ,oy),
  OPCODE(unify_y_val_write          ,oy),
  OPCODE(unify_l_y_val              ,oy),
  OPCODE(unify_l_y_val_write        ,oy),
  OPCODE(unify_x_loc                ,ox),
  OPCODE(unify_x_loc_write          ,ox),
  OPCODE(unify_l_x_loc              ,ox),
  OPCODE(unify_l_x_loc_write        ,ox),
  OPCODE(unify_y_loc                ,oy),
  OPCODE(unify_y_loc_write          ,oy),
  OPCODE(unify_l_y_loc              ,oy),
  OPCODE(unify_l_y_loc_write        ,oy),
  OPCODE(unify_void                 ,o),
  OPCODE(unify_void_write           ,o),
  OPCODE(unify_l_void               ,o),
  OPCODE(unify_l_void_write         ,o),
  OPCODE(unify_n_voids              ,os),
  OPCODE(unify_n_voids_write        ,os),
  OPCODE(unify_l_n_voids            ,os),
  OPCODE(unify_l_n_voids_write      ,os),
  OPCODE(unify_atom                 ,oc),
  OPCODE(unify_atom_write           ,oc),
  OPCODE(unify_l_atom               ,oc),
  OPCODE(unify_l_atom_write         ,oc),
  OPCODE(unify_n_atoms              ,osc),
  OPCODE(unify_n_atoms_write        ,osc),
  OPCODE(unify_float                ,od),
  OPCODE(unify_float_write          ,od),
  OPCODE(unify_l_float              ,od),
  OPCODE(unify_l_float_write        ,od),
  OPCODE(unify_longint              ,oi),
  OPCODE(unify_longint_write        ,oi),
  OPCODE(unify_l_longint            ,oi),
  OPCODE(unify_l_longint_write      ,oi),
  OPCODE(unify_bigint               ,oc),
  OPCODE(unify_l_bigint             ,oc),
  OPCODE(unify_dbterm               ,oc),
  OPCODE(unify_l_dbterm             ,oc),
  OPCODE(unify_list                 ,o),
  OPCODE(unify_list_write           ,o),
  OPCODE(unify_l_list               ,o),
  OPCODE(unify_l_list_write         ,o),
  OPCODE(unify_struct               ,ofa),
  OPCODE(unify_struct_write         ,ofa),
  OPCODE(unify_l_struc              ,ofa),
  OPCODE(unify_l_struc_write        ,ofa),
  OPCODE(put_x_var                  ,xx),
  OPCODE(put_y_var                  ,yx),
  OPCODE(put_x_val                  ,xx),
  OPCODE(put_xx_val                 ,xxxx),
  OPCODE(put_y_val                  ,yx),
  OPCODE(put_y_vals                 ,yyxx),
  OPCODE(put_unsafe                 ,yx),
  OPCODE(put_atom                   ,xc),
  OPCODE(put_float                  ,xd),
  OPCODE(put_longint                ,xi),
  OPCODE(put_list                   ,x),
  OPCODE(put_struct                 ,xfa),
  OPCODE(write_x_var                ,x),
  OPCODE(write_void                 ,e),
  OPCODE(write_n_voids              ,s),
  OPCODE(write_y_var                ,y),
  OPCODE(write_x_val                ,x),
  OPCODE(write_x_loc                ,x),
  OPCODE(write_y_val                ,y),
  OPCODE(write_y_loc                ,y),
  OPCODE(write_atom                 ,c),
  OPCODE(write_float                ,d),
  OPCODE(write_longint              ,i),
  OPCODE(write_n_atoms              ,sc),
  OPCODE(write_list                 ,e),
  OPCODE(write_l_list               ,e),
  OPCODE(write_struct               ,fa),
  OPCODE(write_l_struc              ,fa),
  OPCODE(save_pair_x                ,ox),
  OPCODE(save_pair_x_write          ,ox),
  OPCODE(save_pair_y                ,oy),
  OPCODE(save_pair_y_write          ,oy),
  OPCODE(save_appl_x                ,ox),
  OPCODE(save_appl_x_write          ,ox),
  OPCODE(save_appl_y                ,oy),
  OPCODE(save_appl_y_write          ,oy),
  OPCODE(jump                       ,l),
  OPCODE(move_back                  ,l),
  OPCODE(skip                       ,l),
  OPCODE(either                     ,Osblp),
  OPCODE(or_else                    ,Osblp),
  OPCODE(pop_n                      ,s),
  OPCODE(pop                        ,e),
  OPCODE(call_cpred                 ,Osbpp),
  OPCODE(execute_cpred              ,pp),
  OPCODE(call_usercpred             ,Osbpp),
  OPCODE(call_c_wfail               ,slp),
  OPCODE(try_c                      ,OtapFs),
  OPCODE(retry_c                    ,OtapFs),
#ifdef CUT_C
  OPCODE(cut_c                      ,OtapFs),
#endif
  OPCODE(try_userc                  ,OtapFs),
  OPCODE(retry_userc                ,OtapFs),
#ifdef CUT_C
  OPCODE(cut_userc                  ,OtapFs),
#endif
  OPCODE(lock_pred                  ,e),
  OPCODE(index_pred                 ,e),
#ifdef THREADS
  OPCODE(thread_local               ,e),
#endif
  OPCODE(expand_index               ,e),
  OPCODE(expand_clauses             ,sssllp),
  OPCODE(undef_p                    ,e),
  OPCODE(spy_pred                   ,e),
  OPCODE(try_clause                 ,Otapl),
  OPCODE(try_clause2                ,l),
  OPCODE(try_clause3                ,l),
  OPCODE(try_clause4                ,l),
  OPCODE(retry                      ,Otapl),
  OPCODE(retry2                     ,l),
  OPCODE(retry3                     ,l),
  OPCODE(retry4                     ,l),
  OPCODE(trust                      ,Otapl),
  OPCODE(try_in                     ,l),
  OPCODE(enter_lu_pred              ,Ills),
  OPCODE(try_logical                ,OtaLl),
  OPCODE(retry_logical              ,OtaLl),
  OPCODE(trust_logical              ,OtILl),
  OPCODE(user_switch                ,lp),
  OPCODE(switch_on_type             ,llll),
  OPCODE(switch_list_nl             ,ollll),
  OPCODE(switch_on_arg_type         ,xllll),
  OPCODE(switch_on_sub_arg_type     ,sllll),
  OPCODE(jump_if_var                ,l),
  OPCODE(jump_if_nonvar             ,xll),
  OPCODE(if_not_then                ,clll),
  OPCODE(switch_on_func             ,sssl),
  OPCODE(switch_on_cons             ,sssl),
  OPCODE(go_on_func                 ,sssl),
  OPCODE(go_on_cons                 ,sssl),
  OPCODE(if_func                    ,sssl),
  OPCODE(if_cons                    ,sssl),
  OPCODE(index_dbref                ,e),
  OPCODE(index_blob                 ,e),
  OPCODE(index_long                 ,e),
  OPCODE(native_me                  ,aFlp),
  OPCODE(p_atom_x                   ,xl),
  OPCODE(p_atom_y                   ,yl),
  OPCODE(p_atomic_x                 ,xl),
  OPCODE(p_atomic_y                 ,yl),
  OPCODE(p_integer_x                ,xl),
  OPCODE(p_integer_y                ,yl),
  OPCODE(p_nonvar_x                 ,xl),
  OPCODE(p_nonvar_y                 ,yl),
  OPCODE(p_number_x                 ,xl),
  OPCODE(p_number_y                 ,yl),
  OPCODE(p_var_x                    ,xl),
  OPCODE(p_var_y                    ,yl),
  OPCODE(p_db_ref_x                 ,xl),
  OPCODE(p_db_ref_y                 ,yl),
  OPCODE(p_primitive_x              ,xl),
  OPCODE(p_primitive_y              ,yl),
  OPCODE(p_compound_x               ,xl),
  OPCODE(p_compound_y               ,yl),
  OPCODE(p_float_x                  ,xl),
  OPCODE(p_float_y                  ,yl),
  OPCODE(p_cut_by_x                 ,xl),
  OPCODE(p_cut_by_y                 ,yl),
  OPCODE(p_plus_vv                  ,xxx),
  OPCODE(p_plus_vc                  ,xxn),
  OPCODE(p_plus_y_vv                ,yxx),
  OPCODE(p_plus_y_vc                ,yxn),
  OPCODE(p_minus_vv                 ,xxx),
  OPCODE(p_minus_cv                 ,xxn),
  OPCODE(p_minus_y_vv               ,yxx),
  OPCODE(p_minus_y_cv               ,yxn),
  OPCODE(p_times_vv                 ,xxx),
  OPCODE(p_times_vc                 ,xxn),
  OPCODE(p_times_y_vv               ,yxx),
  OPCODE(p_times_y_vc               ,yxn),
  OPCODE(p_div_vv                   ,xxx),
  OPCODE(p_div_vc                   ,xxn),
  OPCODE(p_div_cv                   ,xxn),
  OPCODE(p_div_y_vv                 ,yxx),
  OPCODE(p_div_y_vc                 ,yxn),
  OPCODE(p_div_y_cv                 ,yxn),
  OPCODE(p_and_vv                   ,xxx),
  OPCODE(p_and_vc                   ,xxn),
  OPCODE(p_and_y_vv                 ,yxx),
  OPCODE(p_and_y_vc                 ,yxn),
  OPCODE(p_or_vv                    ,xxx),
  OPCODE(p_or_vc                    ,xxn),
  OPCODE(p_or_y_vv                  ,yxx),
  OPCODE(p_or_y_vc                  ,yxn),
  OPCODE(p_sll_vv                   ,xxx),
  OPCODE(p_sll_vc                   ,xxn),
  OPCODE(p_sll_cv                   ,xxn),
  OPCODE(p_sll_y_vv                 ,yxx),
  OPCODE(p_sll_y_vc                 ,yxn),
  OPCODE(p_sll_y_cv                 ,yxn),
  OPCODE(p_slr_vv                   ,xxx),
  OPCODE(p_slr_vc                   ,xxn),
  OPCODE(p_slr_cv                   ,xxn),
  OPCODE(p_slr_y_vv                 ,yxx),
  OPCODE(p_slr_y_vc                 ,yxn),
  OPCODE(p_slr_y_cv                 ,yxn),
  OPCODE(call_bfunc_xx              ,plxxs),
  OPCODE(call_bfunc_yx              ,plxys),
  OPCODE(call_bfunc_xy              ,plxys),
  OPCODE(call_bfunc_yy              ,plyys),
  OPCODE(p_equal                    ,e),
  OPCODE(p_dif                      ,l),
  OPCODE(p_eq                       ,l),
  OPCODE(p_arg_vv                   ,xxx),
  OPCODE(p_arg_cv                   ,xxn),
  OPCODE(p_arg_y_vv                 ,yxx),
  OPCODE(p_arg_y_cv                 ,yxn),
  OPCODE(p_func2s_vv                ,xxx),
  OPCODE(p_func2s_cv                ,xxc),
  OPCODE(p_func2s_vc                ,xxn),
  OPCODE(p_func2s_y_vv              ,yxx),
  OPCODE(p_func2s_y_cv              ,yxn),
  OPCODE(p_func2s_y_vc              ,yxn),
  OPCODE(p_func2f_xx                ,xxx),
  OPCODE(p_func2f_xy                ,xxy),
  OPCODE(p_func2f_yx                ,yxx),
  OPCODE(p_func2f_yy                ,yyx),
  OPCODE(p_functor                  ,e),
  OPCODE(p_execute2                 ,Osbpp),
  OPCODE(p_execute                  ,Osbmp),
  OPCODE(p_execute_tail             ,Osbpp),
#ifdef YAPOR
  OPCODE(getwork_first_time         ,e),
  OPCODE(getwork                    ,Otapl),
  OPCODE(getwork_seq                ,Otapl),
  OPCODE(sync                       ,Otapl),
#endif
#ifdef TABLING
#ifdef TABLING_INNER_CUTS
  OPCODE(clause_with_cut            ,e),
#endif
  OPCODE(table_load_answer          ,Otapl),
  OPCODE(table_try_answer           ,Otapl),
  OPCODE(table_try_single           ,Otapl),
  OPCODE(table_try_me               ,Otapl),
  OPCODE(table_try                  ,Otapl),
  OPCODE(table_retry_me             ,Otapl),
  OPCODE(table_retry                ,Otapl),
  OPCODE(table_trust_me             ,Otapl),
  OPCODE(table_trust                ,Otapl),
  OPCODE(table_new_answer           ,s),
  OPCODE(table_answer_resolution    ,Otapl),
  OPCODE(table_completion           ,Otapl),
  OPCODE(table_load_answer_jump     ,Otapl),
  OPCODE(table_load_cons_answer     ,Otapl),
  OPCODE(table_load_cons_answer_jump,Otapl),
  OPCODE(table_run_completed        ,Otapl),
  OPCODE(table_try_retroactive_answer,Otapl),
  OPCODE(trie_do_null               ,e),
  OPCODE(trie_trust_null            ,e),
  OPCODE(trie_try_null              ,e),
  OPCODE(trie_retry_null            ,e),
  OPCODE(trie_do_null_in_new_pair   ,e),
  OPCODE(trie_trust_null_in_new_pair,e),
  OPCODE(trie_try_null_in_new_pair  ,e),
  OPCODE(trie_retry_null_in_new_pair,e),
  OPCODE(trie_do_var                ,e),
  OPCODE(trie_trust_var             ,e),
  OPCODE(trie_try_var               ,e),
  OPCODE(trie_retry_var             ,e),
  OPCODE(trie_do_var_in_new_pair    ,e),
  OPCODE(trie_trust_var_in_new_pair ,e),
  OPCODE(trie_try_var_in_new_pair   ,e),
  OPCODE(trie_retry_var_in_new_pair ,e),
  OPCODE(trie_do_val                ,e),
  OPCODE(trie_trust_val             ,e),
  OPCODE(trie_try_val               ,e),
  OPCODE(trie_retry_val             ,e),
  OPCODE(trie_do_val_in_new_pair    ,e),
  OPCODE(trie_trust_val_in_new_pair ,e),
  OPCODE(trie_try_val_in_new_pair   ,e),
  OPCODE(trie_retry_val_in_new_pair ,e),
  OPCODE(trie_do_atom               ,e),
  OPCODE(trie_trust_atom            ,e),
  OPCODE(trie_try_atom              ,e),
  OPCODE(trie_retry_atom            ,e),
  OPCODE(trie_do_atom_in_new_pair   ,e),
  OPCODE(trie_trust_atom_in_new_pair,e),
  OPCODE(trie_try_atom_in_new_pair  ,e),
  OPCODE(trie_retry_atom_in_new_pair,e),
  OPCODE(trie_do_pair               ,e),
  OPCODE(trie_trust_pair            ,e),
  OPCODE(trie_try_pair              ,e),
  OPCODE(trie_retry_pair            ,e),
  OPCODE(trie_do_struct             ,e),
  OPCODE(trie_trust_struct          ,e),
  OPCODE(trie_try_struct            ,e),
  OPCODE(trie_retry_struct          ,e),
  OPCODE(trie_do_struct_in_new_pair ,e),
  OPCODE(trie_trust_struct_in_new_pair,e),
  OPCODE(trie_try_struct_in_new_pair,e),
  OPCODE(trie_retry_struct_in_new_pair,e),
  OPCODE(trie_do_extension          ,e),
  OPCODE(trie_trust_extension       ,e),
  OPCODE(trie_try_extension         ,e),
  OPCODE(trie_retry_extension       ,e),
  OPCODE(trie_do_float              ,e),
  OPCODE(trie_trust_float           ,e),
  OPCODE(trie_try_float             ,e),
  OPCODE(trie_retry_float           ,e),
  OPCODE(trie_do_float_val          ,e),
  OPCODE(trie_trust_float_val       ,e),
  OPCODE(trie_try_float_val         ,e),
  OPCODE(trie_retry_float_val       ,e),
  OPCODE(trie_do_long               ,e),
  OPCODE(trie_trust_long            ,e),
  OPCODE(trie_try_long              ,e),
  OPCODE(trie_retry_long            ,e),
  OPCODE(trie_do_long_int           ,e),
  OPCODE(trie_trust_long_int        ,e),
  OPCODE(trie_try_long_int          ,e),
  OPCODE(trie_retry_long_int        ,e),
  OPCODE(trie_do_hash               ,e),
  OPCODE(trie_retry_hash            ,e),
#endif
  /* this instruction is hardwired */
#ifdef YAPOR
  OPCODE(or_last                    ,sblp)
#else
  OPCODE(or_last                    ,p)
#endif
