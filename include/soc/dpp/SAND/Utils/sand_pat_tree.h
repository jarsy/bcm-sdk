/* $Id: sand_pat_tree.h,v 1.8 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __SOC_SAND_PAT_INCLUDED__
/* { */
#define __SOC_SAND_PAT_INCLUDED__


/*************
* INCLUDES  *
*************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Utils/sand_occupation_bitmap.h>
#include <soc/dpp/SAND/Utils/sand_u64.h>


/* } */
/*************
* DEFINES   *
*************/
/* { */

#define SOC_SAND_PAT_TREE_NOF_NODE_CHILD 2
#define SOC_SAND_PAT_TREE_PREFIX_MAX_LEN 32
#define SOC_SAND_PAT_TREE_NULL SOC_SAND_U32_MAX
#define SOC_SAND_PAT_TREE_PREFIX_INVALID 255


/* 
 * identical payload flags: 
 */

/* check for found node, all sub-tree agress in same FEC (as FEC never same) --> subtree is empty*/
#define SOC_SAND_PAT_TREE_FIND_IDENTICAL 1
/* for optimization store and reuse last traverse node */
#define SOC_SAND_PAT_TREE_FIND_LOAD_NODE 2
#define SOC_SAND_PAT_TREE_FIND_STORE_NODE  4
/* find exact key */
#define SOC_SAND_PAT_TREE_FIND_EXACT  8


/* } */

/*************
 * MACROS    *
 *************/
/* { */

#define SOC_SAND_PAT_TREE_ITER_IS_END(iter)    \
            (((iter)->arr[0] == SOC_SAND_PAT_TREE_NULL) && (((iter)->arr[1] == SOC_SAND_PAT_TREE_NULL)))

#define SOC_SAND_PAT_TREE_IS_CACHED_INST( _inst)   \
        (uint8)SOC_SAND_GET_BITS_RANGE(_inst,31,31)

#define SOC_SAND_PAT_TREE_TREE_INST( _inst)   \
  SOC_SAND_GET_BITS_RANGE(_inst,30,0)

/* } */

/*************
* TYPE DEFS *
*************/
/* { */

typedef  uint32 SOC_SAND_PAT_TREE_KEY;
typedef uint32 SOC_SAND_PAT_TREE_NODE_PLACE;


/* $Id: sand_pat_tree.h,v 1.8 Broadcom SDK $
 * the ADT of Patricia tree, use this type to manipulate the PAT tree
 */
typedef uint32  SOC_SAND_PAT_TREE_ID;


typedef enum
{
  /*
   *  According to this type the iterator traverse the table
   *  unordered, but it provides an efficient traverse of the
   *  table.
   */
  SOC_SAND_PAT_TREE_ITER_TYPE_FAST=0,
  /*
   *  According to this type the iterator traverse the table
   *  ordered according to (IP, Prefix), it's slower than the
   *  fast type.
   */
  SOC_SAND_PAT_TREE_ITER_TYPE_KEY_PREFIX_ORDERED=1,
  /*
   *  According to this type the iterator traverse the table
   *  ordered according to (Prefix, IP), it's slower than the
   *  previous types (fast and (IP, Prefix)).
   */
  SOC_SAND_PAT_TREE_ITER_TYPE_PREFIX_KEY_ORDERED=2
}SOC_SAND_PAT_TREE_ITER_TYPE;


#define SAND_PAT_TREE_NODE_FLAGS_PREFIX  (0x1)

typedef struct
{
  SOC_SAND_PAT_TREE_NODE_PLACE child[SOC_SAND_PAT_TREE_NOF_NODE_CHILD];
 /*
  *  Drop, Route or Trap
  */
  SOC_SAND_PAT_TREE_KEY key;
 /*
  *  Forwarding Destination
  */
  uint32 data;

 /*
  * bit 0: is prefix 
  */
  uint8 prefix;

  uint8 is_prefix;
}SOC_SAND_PAT_TREE_NODE;



/*********************************************************************
* NAME:
*     SOC_SAND_PAT_TREE_SW_DB_TREE_INFO_SET
* FUNCTION:
*  call back to set information from the SW DB of the device.
* INPUT:
*  SOC_SAND_IN  uint32              tree_ndx -
*   handle of the tree to set the info for.
*  SOC_SAND_OUT  SOC_SAND_PAT_TREE_T*      pat_tree -
*   the information of the pat tree.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_PAT_TREE_SW_DB_TREE_ROOT_SET)(
      SOC_SAND_IN  int                     prime_handle,
      SOC_SAND_IN  uint32                     sec_handle,
      SOC_SAND_IN  SOC_SAND_PAT_TREE_NODE_PLACE     root_place
    );

/*********************************************************************
* NAME:
*     SOC_SAND_PAT_TREE_SW_DB_TREE_INFO_GET
* FUNCTION:
*  call back to get information from the SW DB of the device.
* INPUT:
*  SOC_SAND_IN  uint32              tree_ndx -
*   handle of the tree to get the info for.
*  SOC_SAND_OUT  SOC_SAND_PAT_TREE_T*      pat_tree -
*   the information of the pat tree.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_PAT_TREE_SW_DB_TREE_ROOT_GET)(
      SOC_SAND_IN  int                     prime_handle,
      SOC_SAND_IN  uint32                     sec_handle,
      SOC_SAND_OUT  SOC_SAND_PAT_TREE_NODE_PLACE    *root_place
    );


/*********************************************************************
* NAME:
*     SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_INFO_SET
* FUNCTION:
*  call back to set the node information from the SW DB of the device.
* INPUT:
*  SOC_SAND_IN  uint32              tree_ndx -
*   handle of the tree to set the info for.
*  SOC_SAND_IN  uint32               node_place -
*   the place of the node in the memory "array".
*  SOC_SAND_IN SOC_SAND_PAT_TREE_NODE     *node_info -
*   the information of the node.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_INFO_SET)(
      SOC_SAND_IN  int                   prime_handle,
      SOC_SAND_IN  uint32                   sec_handle,
      SOC_SAND_IN SOC_SAND_PAT_TREE_NODE_PLACE    node_place,
      SOC_SAND_IN SOC_SAND_PAT_TREE_NODE          *node_info
    );


/*********************************************************************
* NAME:
*     SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_INFO_GET
* FUNCTION:
*  call back to set the node information from the SW DB of the device.
* INPUT:
*  SOC_SAND_IN  uint32              tree_ndx -
*   handle of the tree to set the info for.
*  SOC_SAND_IN  uint32               node_place -
*   the place of the node in the memory "array".
*  SOC_SAND_IN SOC_SAND_PAT_TREE_NODE     *node_info -
*   the information of the node.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_INFO_GET)(
      SOC_SAND_IN  int                 prime_handle,
      SOC_SAND_IN  uint32                 sec_handle,
      SOC_SAND_IN SOC_SAND_PAT_TREE_NODE_PLACE  node_place,
      SOC_SAND_OUT SOC_SAND_PAT_TREE_NODE       *node_info
    );
/*********************************************************************
* NAME:
*     SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_INFO_GET
* FUNCTION:
*  call back to set the node information from the SW DB of the device.
* INPUT:
*  SOC_SAND_IN  uint32              tree_ndx -
*   handle of the tree to set the info for.
*  SOC_SAND_IN  uint32               node_place -
*   the place of the node in the memory "array".
*  SOC_SAND_IN SOC_SAND_PAT_TREE_NODE     *node_info -
*   the information of the node.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_REF_GET)(
      SOC_SAND_IN  int                 prime_handle,
      SOC_SAND_IN  uint32                 sec_handle,
      SOC_SAND_IN SOC_SAND_PAT_TREE_NODE_PLACE  node_place,
      SOC_SAND_OUT SOC_SAND_PAT_TREE_NODE       **node_info
    );

/*********************************************************************
* NAME:
*     SOC_SAND_PAT_TREE_NODE_IS_SKIP_IN_LPM_IDENTICAL_DATA_QUERY_FUN
* FUNCTION:
*  call back to check whether the node should be skipped in checking
*  whether nodes below the current node have identical payload 
*  
*  By default (if not set, or set to NULL), all nodes are checked.
* INPUT:
*  SOC_SAND_IN SOC_SAND_PAT_TREE_NODE     *node_info -
*   the information of the node.
* REMARKS:
*     None.
* RETURNS:
*     uint8 - TRUE if the nod should be skipped, FALSE otherwise
*********************************************************************/
typedef
  uint8
    (*SOC_SAND_PAT_TREE_NODE_IS_SKIP_IN_LPM_IDENTICAL_DATA_QUERY_FUN)(
      SOC_SAND_IN SOC_SAND_PAT_TREE_NODE       *node_info
    );


/*********************************************************************
* NAME:
*     SOC_SAND_PAT_TREE_NODE_DATA_IS_IDENTICAL_FUN
* FUNCTION:
*  call back to check whether payloads of two nodes are identical 
*  
*  By default (if not set, or set to NULL), bitwise comparison is used.
* INPUT:
*  SOC_SAND_IN SOC_SAND_PAT_TREE_NODE     *node_info_0 -
*   the information of the node.
*  SOC_SAND_IN SOC_SAND_PAT_TREE_NODE     *node_info_1 -
*   the information of the node.
* REMARKS:
*     None.
* RETURNS:
*     uint8 - TRUE if the payloads are identical, FALSE otherwise
*********************************************************************/
typedef
  uint8
    (*SOC_SAND_PAT_TREE_NODE_DATA_IS_IDENTICAL_FUN)(
      SOC_SAND_IN SOC_SAND_PAT_TREE_NODE       *node_info_0,
      SOC_SAND_IN SOC_SAND_PAT_TREE_NODE       *node_info_1
  );



typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
 /*
  *  the value of the key
  */
    uint32  val;
 /*
  *  how many bits to consider from the above value,
  *  starting from msb.
  */
  uint8 prefix;
}SOC_SAND_PAT_TREE_NODE_KEY;


typedef struct {
  SOC_SAND_MAGIC_NUM_VAR

  SOC_SAND_PAT_TREE_NODE_PLACE root;
  /*
   * the memory to allocate from the nodes of the tree
  */
  SOC_SAND_PAT_TREE_NODE *tree_memory;
  /*
   * Mapping of the tree memory, for efficient manipulation.
   * This is the identifying handle of the tree.
   */
  SOC_SAND_OCC_BM_PTR memory_use ;
  /*
   * Mapping of the tree memory: This is the actual pointer to the tree.
   * Added here specifically for ARAD which still uses old interface to
   * occupation bit map (old sw state)
   */
  SOC_SAND_OCC_BM_T *memory_use_ptr ;
  /*
   * Mapping of the tree memory: This is the unit corresponding to 'memory_use_ptr'.
   * Added here specifically for ARAD which still uses old interface to
   * occupation bit map (old sw state)
   */
  int               memory_use_unit ;

  uint8 cache_enabled;

  SOC_SAND_PAT_TREE_NODE *tree_memory_cache;
  SOC_SAND_PAT_TREE_NODE_PLACE root_cache;

  /* use to optimize commit, to do from the relevant (changed) subtree*/
  SOC_SAND_PAT_TREE_NODE_KEY cache_change_head;

  /* for traverse optimization*/
  SOC_SAND_PAT_TREE_NODE_PLACE current_node_place;

} SOC_SAND_PAT_TREE_T;


/*
 * includes the information user has to supply in the Patricia Tree creation
 */
typedef struct {
  SOC_SAND_MAGIC_NUM_VAR
 /*
  * size of the tree in nodes, how many nodes the tree can holds
  * including glue nodes.
  */
  uint32
    tree_size;
  SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_INFO_SET node_set_fun;
  SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_INFO_GET node_get_fun;
  SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_REF_GET node_ref_get_fun;
  SOC_SAND_PAT_TREE_SW_DB_TREE_ROOT_SET root_set_fun;
  SOC_SAND_PAT_TREE_SW_DB_TREE_ROOT_GET root_get_fun;
  SOC_SAND_PAT_TREE_NODE_DATA_IS_IDENTICAL_FUN node_data_is_identical_fun;
  SOC_SAND_PAT_TREE_NODE_IS_SKIP_IN_LPM_IDENTICAL_DATA_QUERY_FUN node_is_skip_in_lpm_identical_data_query_fun;
  int prime_handle;
  uint32
    sec_handle;
  uint8
    support_cache;

 /*
  * saving warm boot variable index get/set of pat_tree variables.
  */
  int32 wb_var_index;

 /*
  * this to be manipulated by the soc_sand, caller shouldn't
  * modify these information, used to interconnect specific device memory
  * with the general algorithm in the soc_sand.
  */
  SOC_SAND_PAT_TREE_T
    pat_tree_data;
} SOC_SAND_PAT_TREE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
 /*
  *  the data of the node
  */
    uint32  payload;
}SOC_SAND_PAT_TREE_NODE_DATA;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
 /*
  *  the data of the node
  */
  SOC_SAND_PAT_TREE_NODE_KEY  key;
  SOC_SAND_PAT_TREE_NODE_DATA data;
  /* id/place of node */
  SOC_SAND_PAT_TREE_NODE_PLACE node_place;

}SOC_SAND_PAT_TREE_NODE_INFO;

typedef struct
{
  SOC_SAND_MAGIC_NUM_VAR
    /* flags */
  uint32    flags ;
  /* call-backs */
  SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_INFO_SET node_set_fun;
  SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_INFO_GET node_get_fun;
  SOC_SAND_PAT_TREE_SW_DB_TREE_NODE_REF_GET node_ref_get_fun;
  SOC_SAND_PAT_TREE_SW_DB_TREE_ROOT_SET root_set_fun;
  SOC_SAND_PAT_TREE_SW_DB_TREE_ROOT_GET root_get_fun;
  SOC_SAND_PAT_TREE_NODE_DATA_IS_IDENTICAL_FUN node_data_is_identical_fun;
  SOC_SAND_PAT_TREE_NODE_IS_SKIP_IN_LPM_IDENTICAL_DATA_QUERY_FUN node_is_skip_in_lpm_identical_data_query_fun;
}SOC_SAND_PAT_TREE_LOAD_INFO;

/*********************************************************************
* NAME:
*     SOC_SAND_PAT_TREE_FOREACH_FUN
* FUNCTION:
*  call back to invoke on each node in PAT tree 
*  
*  By default (if not set, or set to NULL), bitwise comparison is used.
* INPUT:
*  SOC_SAND_IN SOC_SAND_PAT_TREE_INFO       *tree_info -
*    tree info
*  SOC_SAND_IN void                     *vparam -
*   free argument
* REMARKS:
*     None.
* RETURNS:
*     uint8 - TRUE if the payloads are identical, FALSE otherwise
*********************************************************************/
typedef
  uint32
    (*SOC_SAND_PAT_TREE_FOREACH_FUN)(
      SOC_SAND_INOUT SOC_SAND_PAT_TREE_INFO *pat_tree,
      SOC_SAND_IN SOC_SAND_PAT_TREE_KEY key,
      SOC_SAND_IN uint8 prefix,
      SOC_SAND_INOUT uint32 *data,
      SOC_SAND_IN uint8 is_prefix,
      SOC_SAND_INOUT void* vparam
    );

/* } */

/*************
* GLOBALS   *
*************/
/* { */

/* } */

/*************
* FUNCTIONS *
*************/
/* { */

/*********************************************************************
* NAME:
*     soc_sand_pat_tree_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new Patricia Tree instance.
* INPUT:
*   SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO       *tree_info -
*     information to use in order to create the Patricia Tree
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_create(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO       *tree_info
  );

/*********************************************************************
* NAME:
*     soc_sand_pat_tree_destroy
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     free the Patricia Tree instance.
* INPUT:
*  SOC_SAND_OUT  SOC_SAND_PAT_TREE_INFO pat_tree -
*     The Patricia Tree to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_destroy(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO       *tree_info
  );


/*********************************************************************
* NAME:
*     soc_sand_pat_tree_clear
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     clear the Patricia Tree instance, without freeing the memory
* INPUT:
*  SOC_SAND_OUT  SOC_SAND_PAT_TREE_INFO pat_tree -
*     The Patricia Tree to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_clear(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO       *tree_info
  );

/*********************************************************************
* NAME:
*     soc_sand_pat_tree_clear_nodes
* TYPE:
*   PROC
* FUNCTION:
*   clear patricia tree, by invalidate memory used by nodes 
*   and make root point to NULL 
*   useful in case nodes memory is shared among many trees
* INPUT:
*  SOC_SAND_OUT  SOC_SAND_PAT_TREE_INFO pat_tree -
*     The Patricia Tree to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_clear_nodes(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO       *tree_info_ndx
  );

/*********************************************************************
* NAME:
*     soc_sand_pat_tree_root_reset
* TYPE:
*   PROC
* FUNCTION:
*   in order to optimize clearing all patricia trees/
*   call soc_sand_pat_tree_clear once then this API for each patricia tree
* INPUT:
*  SOC_SAND_OUT  SOC_SAND_PAT_TREE_INFO pat_tree -
*     The Patricia Tree to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_root_reset(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO       *tree_info_ndx
  );

/*********************************************************************
* NAME:
*     soc_sand_pat_tree_node_add
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*  Insert an entry into the Patricia Tree,
* INPUT:
*   SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO pat_tree -
*     The Patricia Tree to add a node (key and data) to.
*   SOC_SAND_IN  SOC_SAND_PAT_TREE_NODE_KEY  *node_key -
*     The key to add into the Patricia Tree
*   SOC_SAND_IN  SOC_SAND_PAT_TREE_NODE_DATA  *node_data -
*     the data to add into the Patricia Tree and to be associated with
*     the given key
*   SOC_SAND_OUT  uint8                 *success -
*     whether the add operation success,
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_node_add(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO       *tree_info,
    SOC_SAND_IN  SOC_SAND_PAT_TREE_NODE_KEY     *node_key,
    SOC_SAND_IN  SOC_SAND_PAT_TREE_NODE_DATA    *node_data,
    SOC_SAND_OUT  uint8                 *success
  );

/*********************************************************************
* NAME:
*     soc_sand_pat_tree_node_remove
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*  Remove node from the PAT tree according to given key
* INPUT:
*   SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO       *tree_info -
*     The Patricia Tree instance.
*   SOC_SAND_IN  SOC_SAND_HASH_TABLE_KEY    key -
*     to remove the node with this key
* REMARKS:
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_node_remove(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO       *tree_info,
    SOC_SAND_IN  SOC_SAND_PAT_TREE_NODE_KEY    *node_key
  );


/*********************************************************************
* NAME:
*     soc_sand_pat_tree_lpm_get
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*  Get the longest prefix match for a given key.
* INPUT:
*   SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO       *tree_info -
*     The Patricia Tree instance.
*   SOC_SAND_IN  SOC_SAND_PAT_TREE_NODE_KEY     *node_key -
*     PAT tree key (val/prefix)
*   SOC_SAND_IN  uint8                  identical_payload -
*     if TRUE then find a node in the PAT tree with
*     longest prefix match where the payload of all the nodes under the found
*     node have the same payload.
*     if FALSE then find LPM node.
*   SOC_SAND_OUT  SOC_SAND_PAT_TREE_NODE_INFO   *lpm_info -
*     the found node if any.
*   SOC_SAND_OUT  uint8                 *found -
*     wether such node was found
* REMARKS:
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_lpm_get(
    SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO       *tree_info,
    SOC_SAND_IN  SOC_SAND_PAT_TREE_NODE_KEY     *node_key,
    SOC_SAND_IN  uint8                  identical_payload,
    SOC_SAND_OUT  SOC_SAND_PAT_TREE_NODE_INFO   *lpm_info,
    SOC_SAND_OUT  uint8                 *found
  );


uint32
  soc_sand_pat_tree_node_at_place(
    SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO        *tree_info_ndx,
    SOC_SAND_IN  SOC_SAND_PAT_TREE_NODE_PLACE  node_place,
    SOC_SAND_OUT  SOC_SAND_PAT_TREE_NODE_INFO   *lpm_info,
    SOC_SAND_OUT  uint8                 *exist);


/* when cached return head of changes */
uint32
  soc_sand_pat_tree_head_key_of_changes(
    SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO     *tree_info_ndx,
    SOC_SAND_OUT  SOC_SAND_PAT_TREE_NODE_KEY     *cached_head
  );


/*********************************************************************
* NAME:
*     soc_sand_pat_tree_get_block
* TYPE:
*   PROC
* DATE:
*   May 27 2008
* FUNCTION:
*     Get block of entries from the PAT tree table in a given
*     range.
* INPUT:
*   SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO       *tree_info -
*     The Patricia Tree instance.
*   SOC_SAND_IN SOC_SAND_PAT_TREE_ITER_TYPE     iter_type -
*     Determine how to traverse the PAT tree.
*       - FAST: according to the memory order.
*               and no any order over the keys.
*       - REFIX_KEY_ORDERED: ordered according to the keys value and then according prefix len
*       - PREFIX_KEY_ORDERED: ordered according to the prefix len and then according keys value
*  SOC_SAND_INOUT  SOC_SAND_U64                *iter -
*     Iterator indicates from where start scanning the PAT tree
*     and from where to continue in the next  time.
*  SOC_SAND_IN  uint32                  entries_to_scan -
*     Number of entries to scan in the PAT tree
*  SOC_SAND_IN  uint32                  entries_to_get -
*     Number of entries to get from the PAT tree
*  SOC_SAND_OUT  SOC_SAND_PAT_TREE_NODE_INFO   *nodes -
*     to include the block of nodes
*  SOC_SAND_OUT uint32                  *nof_entries -
*     number of valid entries in nodes.
* REMARKS:
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_get_block(
    SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO       *tree_info,
    SOC_SAND_IN SOC_SAND_PAT_TREE_ITER_TYPE     iter_type,
    SOC_SAND_INOUT  SOC_SAND_U64                *iter,
    SOC_SAND_IN  uint32                   entries_to_scan,
    SOC_SAND_IN  uint32                   entries_to_get,
    SOC_SAND_OUT  SOC_SAND_PAT_TREE_NODE_INFO   *nodes,
    SOC_SAND_OUT uint32                   *nof_entries
  );


/*********************************************************************
* NAME:
*     soc_sand_pat_tree_get_size
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*  Get sizes/status of the PAT tree.
* INPUT:
*   SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO       *tree_info -
*     The Patricia Tree instance.
*   SOC_SAND_OUT  uint32                    *total_nodes -
*     Total nodes (prefix/glue) inserted to the PAT tree.
*   SOC_SAND_OUT  uint32                    *prefix_nodes -
*     prefix nodes inserted to the PAT tree.
*   SOC_SAND_OUT  uint32                    *free_nodes -
*     free nodes, not allocated in the TREE and can be allocated later.
* REMARKS:
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_get_size(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO       *tree_info,
    SOC_SAND_OUT  uint32                    *total_nodes,
    SOC_SAND_OUT  uint32                    *prefix_nodes,
    SOC_SAND_OUT  uint32                    *free_nodes
  );

/*********************************************************************
* NAME:
*     soc_sand_pat_tree_foreach
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*  Invoke function on each node
* INPUT:
*   SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO       *tree_info -
*     The Patricia Tree instance.
*   SOC_SAND_IN  SOC_SAND_PAT_TREE_FOREACH_FUN       fun -
*     Function to apply.
*   SOC_SAND_INOUT void                       *param - 
*     free argument, will be forwarded to foreach fun.
* REMARKS:
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_foreach(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO         *tree_info,
    SOC_SAND_IN     SOC_SAND_PAT_TREE_FOREACH_FUN  fun,
    SOC_SAND_INOUT     void                        *param
  );

uint32
  soc_sand_pat_tree_cache_set(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO        *tree_info_ndx,
    SOC_SAND_IN  uint8             cached
  );

uint32
  soc_sand_pat_tree_cache_commit(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO        *tree_info_ndx,
    SOC_SAND_IN  uint32               flags
  );

uint32
  soc_sand_pat_tree_cache_rollback(
    SOC_SAND_INOUT  SOC_SAND_PAT_TREE_INFO        *tree_info_ndx,
    SOC_SAND_IN  uint32               flags
  );


/*********************************************************************
* NAME:
*     soc_sand_pat_tree_get_size_for_save
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     returns the size of the buffer needed to return the pat-tree as buffer.
*     in order to be loaded later
* INPUT:
*   SOC_SAND_IN  SOC_SAND_PAT_TREE_PTR tree_info -
*     The pat-tree to get the size for.
*   SOC_SAND_OUT  uint32   *size -
*     the size of the buffer.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_get_size_for_save(
    SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO              *tree_info,
    SOC_SAND_IN  uint32                        flags,
    SOC_SAND_OUT  uint32                       *size
  );

/*********************************************************************
* NAME:
*     soc_sand_pat_tree_save
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     saves the given pat-tree in the given buffer
* INPUT:
*   SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO *tree_info -
*     The pat-tree to save.
*   SOC_SAND_OUT  uint8                 *buffer -
*     buffer to include the hast table
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by soc_sand_pat_tree_get_size_for_save.
*   - call back functions are not saved.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_save(
    SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO     *tree_info,
    SOC_SAND_IN  uint32                flags,
    SOC_SAND_OUT uint8                 *buffer,
    SOC_SAND_IN  uint32                buffer_size_bytes,
    SOC_SAND_OUT uint32                *actual_size_bytes
  );

/*********************************************************************
* NAME:
*     soc_sand_pat_tree_load
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     load from the given buffer the pat-tree saved in this buffer.
* INPUT:
*   SOC_SAND_IN  uint8                 *buffer -
*     buffer includes the hast table
*   SOC_SAND_IN  SOC_SAND_MULTISET_SW_DB_ENTRY_SET     set_function -
*     the hash function of the pat-tree.
*   SOC_SAND_IN  SOC_SAND_MULTISET_SW_DB_ENTRY_GET     get_function -
*     the hash function of the pat-tree.
*   SOC_SAND_OUT  SOC_SAND_PAT_TREE_PTR tree_info -
*     The pat-tree to load.
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by soc_sand_pat_tree_get_size_for_save.
*   - there is need to supply the hash and rehash function (in case they are not
*     the default implementation, cause in the save they are not saved.
*     by soc_sand_pat_tree_get_size_for_save.
*   - this function will update buffer to point
*     to next place, for further loads.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_pat_tree_load(
    SOC_SAND_IN  uint8                           **buffer,
    SOC_SAND_IN  SOC_SAND_PAT_TREE_LOAD_INFO           *load_info,
    SOC_SAND_OUT SOC_SAND_PAT_TREE_INFO               *tree_info
  );  

void
  soc_sand_SAND_PAT_TREE_LOAD_INFO_clear(
    SOC_SAND_PAT_TREE_LOAD_INFO *info
  );
void
  soc_sand_SAND_PAT_TREE_INFO_clear(
    SOC_SAND_PAT_TREE_INFO *info
  );

#if SOC_SAND_DEBUG

uint32
  soc_sand_pat_tree_print(
    SOC_SAND_IN  SOC_SAND_PAT_TREE_INFO       *tree_info_ndx,
    SOC_SAND_IN  SOC_SAND_PAT_TREE_NODE_PLACE   node_place,
    SOC_SAND_IN  uint8                  including_glue,
    SOC_SAND_IN  uint8                  print_subtree
  );

#endif /* SOC_SAND_DEBUG */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } __SOC_SAND_PAT_TREE_INCLUDED__*/
#endif
