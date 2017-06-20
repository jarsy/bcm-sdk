/*
 * $Id: trie.h,v 1.23 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * trie data structure
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_TRIE_V6_H_
#define _SBX_CALADN3_TRIE_V6_H_

extern int _trie_v6_search(trie_node_t *trie,
			   unsigned int *key,
			   unsigned int length,
			   trie_node_t **payload,
			   unsigned int *result_key,
			   unsigned int *result_len,
			   unsigned int dump);

extern int _trie_v6_find_lpm(trie_node_t *trie,
			     unsigned int *key,
			     unsigned int length,
			     trie_node_t **payload,
			     trie_callback_f cb,
			     void *user_data);

extern int _trie_v6_find_bpm(trie_node_t *trie,
			     unsigned int *key,
			     unsigned int length,
			     int *bpm_length);

extern int _trie_v6_skip_node_alloc(trie_node_t **node, 
				    unsigned int *key, 
				    /* bpm bit map if bpm management is required, passing null skips bpm management */
				    unsigned int *bpm, 
				    unsigned int msb, /* NOTE: valid msb position 1 based, 0 means skip0/0 node */
				    unsigned int skip_len,
				    trie_node_t *payload,
				    unsigned int count);

extern int _trie_v6_insert(trie_node_t *trie, 
			   unsigned int *key, 
			   /* bpm bit map if bpm management is required, passing null skips bpm management */
			   unsigned int *bpm, 
			   unsigned int length,
			   trie_node_t *payload, /* payload node */
			   trie_node_t **child /* child pointer if the child is modified */);

extern int _trie_v6_delete(trie_node_t *trie, 
			   unsigned int *key,
			   unsigned int length,
			   trie_node_t **payload,
			   trie_node_t **child);

extern int _trie_v6_split_len_get(trie_node_t  *trie,
		       unsigned int *pivot,
		       unsigned int *length,
		       const unsigned int max_count,
		       const unsigned int max_split_len,
		       unsigned int *bpm);

extern int _trie_v6_split(trie_node_t  *trie,
			  unsigned int *pivot,
			  unsigned int *length,
			  unsigned int *split_count,
			  trie_node_t **split_node,
			  trie_node_t **child,
			  const unsigned int max_count,
			  const unsigned int max_split_len,
			  const int split_to_pair,
			  unsigned int *bpm,
			  trie_split_states_e_t *state);

extern int _trie_v6_propagate_prefix(trie_node_t *trie,
				     unsigned int *pfx,
				     unsigned int len,
				     unsigned int add, /* 0-del/1-add */
				     trie_propagate_cb_f cb,
				     trie_bpm_cb_info_t *cb_info);

extern int trie_v6_pivot_propagate_prefix(trie_node_t *pivot,
					  unsigned int pivot_len,
					  unsigned int *pfx,
					  unsigned int len,
					  unsigned int add, /* 0-del/1-add */
					  trie_propagate_cb_f cb,
					  trie_bpm_cb_info_t *cb_info);

extern int _trie_v6_bpm_mask_get(trie_node_t *trie,
                    unsigned int *key,
                    unsigned int length,
		            unsigned int *bpm_mask);

extern int tmu_trie_v6_split_ut(unsigned int seed);

extern int tmu_taps_trie_v6_ut(int id, unsigned int seed);

extern int tmu_taps_bpm_trie_v6_ut(int id, unsigned int seed);

#endif /* _SBX_CALADN3_TRIE_V6_H_ */
