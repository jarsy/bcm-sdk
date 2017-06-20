/*
 * $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
/*
 **************************************************************************************
 Copyright 2009-2012 Broadcom Corporation

 This program is the proprietary software of Broadcom Corporation and/or its licensors,
 and may only be used, duplicated, modified or distributed pursuant to the terms and 
 conditions of a separate, written license agreement executed between you and 
 Broadcom (an "Authorized License").Except as set forth in an Authorized License, 
 Broadcom grants no license (express or implied),right to use, or waiver of any kind 
 with respect to the Software, and Broadcom expressly reserves all rights in and to 
 the Software and all intellectual property rights therein.  
 IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY 
 WAY,AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.  

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization, constitutes the 
    valuable trade secrets of Broadcom, and you shall use all reasonable efforts to 
    protect the confidentiality thereof,and to use this information only in connection
    with your use of Broadcom integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH 
    ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER 
    EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM 
    SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, 
    NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR 
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. 
    YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS 
    BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES 
    WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE 
    THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; 
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF 
    OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING 
    ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 **************************************************************************************
 */
 #include "nlmrangemgr.h"

extern NlmErrNum_t NlmRangeMgr2__pvt_Init(
	NlmRangeMgr		*self,
    NlmReasonCode	*o_reason
	);

#define NLM_RANGE_TREE_TBL_DEPTH  2049
#define NLM_RANGE_NUM_OF_TOP_RANGES (100)/* Number of top-ranges the Software maintains  */
#define NLM_RANGE_3B_ENCODING_CODE_VALUE (0x8CE)
#define NLM_RANGE_2B_ENCODING_CODE_VALUE (0x0CA)   /*Value to bypass encoding (NKG)*/

typedef enum NlmRangeShiftDirection
{
    NLM_RANGE_DOWN_SHIFT,
    NLM_RANGE_UP_SHIFT        
}NlmRangeShiftDirection;

typedef struct NlmRangeElement_s
{
    nlm_u32 range;
    nlm_u32 count;
    struct NlmRangeElement_s *left;
    struct NlmRangeElement_s *right;
}NlmRangeElement;

typedef struct NlmListTopRange_s
{
    nlm_u32 range;
    nlm_u32 count;
    struct NlmListTopRange_s *prev;
    struct NlmListTopRange_s *next;
}NlmListTopRange;

typedef struct NlmRangeMgr__pvtdata
{
    NlmListTopRange *m_head;
    NlmListTopRange *m_last;
    nlm_u32 elements_in_list;
    NlmRangeElement *range_trees[NLM_RANGE_TREE_TBL_DEPTH];
} NlmRangeMgr__pvtdata;


static NlmRangeDb* 
NlmRangeMgr__pvt_CreateDb(
	NlmRangeMgr		*self,
	nlm_u8			id,
    NlmRangeDbAttrSet   *dbAttributes,
	NlmReasonCode	*o_reason
	);

static NlmErrNum_t 
NlmRangeMgr__pvt_DestroyDb(
	NlmRangeMgr     *self,
	NlmRangeDb      *rangeDb,
	NlmReasonCode	*o_reason
	);

static NlmErrNum_t 
NlmRangeMgr__pvt_AddRange(
	NlmRangeMgr		*self,
	NlmRangeDb		*rangeDb,
	NlmRange		*range,
	NlmReasonCode	*o_reason
	);

static NlmErrNum_t 
NlmRangeMgr__pvt_DeleteRange(
	NlmRangeMgr     *self,
	NlmRangeDb 	    *rangeDb,
	nlm_u32			id,
	NlmReasonCode	*o_reason
	);

static NlmErrNum_t 
NlmRangeMgr__pvt_CreateEncodings(
	NlmRangeMgr		*self,
	NlmRangeDb		*rangeDb,
	NlmReasonCode	*o_reason
	);

static NlmErrNum_t 
NlmRangeMgr__pvt_AssignRange(
	NlmRangeMgr		*self,
	NlmRangeDb		*rangeDb,
	NlmRangeType	rangeType,
	NlmReasonCode	*o_reason
	);

static NlmRangeEncoded* 
NlmRangeMgr__pvt_GetRangeEncoding(
	NlmRangeMgr 		*self,
	NlmRangeDb 			*rangeDb,
	nlm_u32				 id,
	NlmReasonCode		*o_reason
	);


static NlmRangeDbStats* 
NlmRangeMgr__pvt_GetDbStats(
	NlmRangeMgr         *self,
	NlmRangeDb          *rangeDb,
	NlmReasonCode		*o_reason
	);

static NlmErrNum_t 
NlmRangeMgr__pvt_ConfigRangeMatching(
    NlmRangeMgr *self,
    nlm_u8 ltrNum,
    NlmRangeSrchAttrs *srchAttrs,
    NlmReasonCode *o_reason
    );


/****************************************
 * Function : NlmRangeMgr__pvt_AddtoRangeList
 * Summary:
 * This function adds a Range node to the Doubly-Linked List of Range-nodes.
 * Note: This function basically takes advantage of the fact that "where" the new node is
 * inserted has no relevance. Hence it inserts the newNode at the begining.
 ****************************************/
static void NlmRangeMgr__pvt_AddtoRangeList(
	    								NlmRangeList* list,
		    							NlmRange* newRange,
			    						NlmCmAllocator *alloc_p
				    					)
{
	NlmRangeNode* newNode;

	/* Note that here the memory for the node is allocated only.
	 * Memory for the Range should be allocated by the upper layer.
	 */
	newNode = NlmCmAllocator__malloc(alloc_p, sizeof(NlmRangeNode));
	NlmCmDemand((newNode != NULL), "Out of memory.\n");
	newNode->m_range = newRange;

	newNode->m_next = list->m_firstRange;
    newNode->m_prev = NULL;
    if(list->m_firstRange)
        list->m_firstRange->m_prev = newNode;
    list->m_firstRange = newNode;    
}


/****************************************
 * Function : NlmRangeMgr__pvt_DelfromRangeList
 * Summary:
 * This function deletes a Range node from the Doubly-Linked List of Range-nodes
 ****************************************/
nlm_u32 NlmRangeMgr__pvt_DelfromRangeList(
									NlmRangeList* list,
									nlm_u32 id,
									NlmRange** rangeToBeDeleted,
									NlmCmAllocator *alloc_p
							   	  )
{
	NlmRangeNode* node;
	/* Check the inputs. */
	if((NULL == list)  || (NULL == list->m_firstRange) )	
		return NLMERR_FAIL;

    node = list->m_firstRange;
	while(node)
	{
		/* If you have found the entry to be deleted */
		if(node->m_range->m_id == id)
		{
			/* Pass this range to the caller so that it can free the memory allocated
			 * by the NlmRange. Note that here the node containing the rangeToBeDeleted
			 * is freed not the actual range. It's caller's responisibility
			 */
			*rangeToBeDeleted = node->m_range;
			if (NULL == node->m_prev)
                list->m_firstRange= node->m_next;
			else
				node->m_prev->m_next = node->m_next;

            if(node->m_next != NULL)
                node->m_next->m_prev = node->m_prev;
			/* Note that here the memory for the node is de-allocated only.
			 * Memory for the Database should be De-allocated by the upper layer.
			 */
			
            NlmCmAllocator__free(alloc_p, node);
			return NLMERR_OK;
		}

		node = node->m_next;
	}
	return NLMERR_FAIL;
}

/****************************************
 * Function : NlmRangeMgr__pvt_SearchRangeList
 * Summary:
 * This function searches a Range node from the Doubly-Linked List of Range-nodes
 ****************************************/
NlmRangeNode* NlmRangeMgr__pvt_SearchRangeList(
									NlmRangeList* list,
									nlm_u32 id
                                    )
{
	NlmRangeNode* node;

	if((NULL == list)  || (NULL == list->m_firstRange) )
	{
		return NULL;
	}
    node = list->m_firstRange;
	while(node)
	{
		/* If you have found the entry with correct id */
        if(node->m_range->m_id == id)
			return node;

        node = node->m_next;
	}
	return NULL;
}
/*
 *  NlmRangeMgr__pvt_ExchPosition exchanges the position of two nodes in the
 *  linked list maintaining the MCORs
 *
 *  side effect: the list is modified.
 *  Return value: NONE
 */
void NlmRangeMgr__pvt_ExchPosition(
                NlmListTopRange *first,
                NlmListTopRange *second
                )
{
        NlmListTopRange  *prevprev, *nextnext;

        if (!first || !second || (first == second))
                return;

        prevprev = first->prev; 
        nextnext = second->next;

        if (prevprev)
                prevprev->next = second;
        second->next = first;
        second->prev = prevprev;
        first->next = nextnext;
        if (nextnext)
                nextnext->prev = first;
        first->prev = second;
        return;
}
/*
 *  NlmRangeMgr__pvt_RearrangeList: Rearranges the list containing the top
 *  ranges. Due to repeated addition/deletion of ranges, the #occurences of
 *  the ranges may keep changing and hence the list has to be rearranged.
 *
 *  side effect: The "from" node is moves to its correct position.
 *  Return value: NONE
 */

void NlmRangeMgr__pvt_RearrangeList(
        NlmRangeDb *rdb,
        NlmListTopRange *from, /* Node where rearrangement starts */
        NlmRangeShiftDirection direction    /* rearrangement direction */
        )
{
        nlm_u32 break_out = 0;
        NlmRangeMgr__pvtdata *pvtdata;
        if (!rdb || !from)
                return;
        pvtdata = (NlmRangeMgr__pvtdata *) rdb->m_pvtdata;

        if (!pvtdata->m_head || !pvtdata->m_last ||
            ((from == pvtdata->m_head) && (NLM_RANGE_UP_SHIFT == direction)) ||
            ((from == pvtdata->m_last) && (NLM_RANGE_DOWN_SHIFT == direction)))
                return;


        if (NLM_RANGE_UP_SHIFT == direction)
        {
                /*
                 * The start node may have to be moved upward
                 */

                while (from && from->prev)
                {
                        if (from->count > from->prev->count)
                        {

                                if (from->prev == pvtdata->m_head)
                                {
                                        pvtdata->m_head = from; /*This is the new head*/
                                        break_out = 1;
                                }
                                NlmRangeMgr__pvt_ExchPosition(from->prev, from);
                                if (from == pvtdata->m_last)
                                {
                                        pvtdata->m_last = from->next;
                                }
                                if (break_out) break;
                        }
                        else
                                break;
                }
        }
        else if (NLM_RANGE_DOWN_SHIFT == direction)
        {
                /*
                 * The start node may have to be moved downward
                 */

                while (from && from->next)
                {
                        if (from->count < from->next->count)
                        {
                                if (from->next == pvtdata->m_last)
                                {
                                        pvtdata->m_last = from;
                                        break_out = 1;
                                }
                                NlmRangeMgr__pvt_ExchPosition(from, from->next);
                                if (from == pvtdata->m_head)
                                {
                                        pvtdata->m_head = from->prev;
                                }
                                if (break_out) break;
                        }
                        else
                                break;
                }
        }
}


/*
 *  NlmRangeMgr__pvt_AddtoTop: May add a range to the MCOR list or may cause
 *  rearrangement of the MCOR list.
 *
 *  side effect: MCOR list may get modified.
 *  Return value: NONE
 */
void NlmRangeMgr__pvt_AddtoTop(
                NlmRangeMgr *self,
                NlmRangeDb *rdb,
                NlmRangeElement *tmp,
                NlmBool add_to_list
           )
{
       NlmListTopRange *topl, *newl;
       NlmRangeMgr__pvtdata *pvtdata;

       if (!self || !rdb || !tmp) return;

       pvtdata = (NlmRangeMgr__pvtdata *)rdb->m_pvtdata;

       if (!pvtdata->m_head)
       {
               /* MCOR list was empty. Add the first Node*/
                pvtdata->m_head = ( NlmListTopRange *)
                        NlmCmAllocator__calloc(self->m_alloc_p,1,
                                        sizeof(NlmListTopRange));
                pvtdata->m_last = pvtdata->m_head;
                pvtdata->m_head->count = tmp->count;
                pvtdata->m_head->range = tmp->range;
                pvtdata->elements_in_list++;
                goto get_out1;
       }

       if ((NLM_RANGE_NUM_OF_TOP_RANGES == pvtdata->elements_in_list) &&
                       (pvtdata->m_last->count >= tmp->count))
       {
               /* Not a range to be part of current top ranges */
               goto get_out1;
       }

       topl = pvtdata->m_head;

       /* Find if the range is already there */

       while (topl)
       {
                if (topl->range == tmp->range)
                {
                        if (NLMTRUE == add_to_list)
                        {
                                /*
                                 * We have added this range one more time
                                 * We may have to move this node up
                                 */
                                topl->count++;
                                NlmRangeMgr__pvt_RearrangeList(rdb, topl, NLM_RANGE_UP_SHIFT);
                        }
                        else
                        {
                                /*
                                 * We decremented the #occurence count and hence
                                 * we may have move this node downward
                                 */
                                NlmRangeMgr__pvt_RearrangeList(rdb, topl, NLM_RANGE_DOWN_SHIFT);
                        }
                        goto get_out1;

                }

                topl = topl->next;

       }

       if (NLM_RANGE_NUM_OF_TOP_RANGES != pvtdata->elements_in_list )
       {
               /*
                * A new entry to the top range list
                */

                topl = pvtdata->m_head;

                while (topl)
                {
                        if (NULL == topl->next)
                        {
                                topl->next =
                                    (NlmListTopRange *)
                                       NlmCmAllocator__calloc(self->m_alloc_p,1,
                                                    sizeof(NlmListTopRange));
                                newl = topl;
                                topl = topl->next;
                                topl->range = tmp->range;
                                topl->count = tmp->count;
                                topl->prev = newl;
                                pvtdata->m_last = topl;
                                pvtdata->elements_in_list++;
                                goto get_out1;
                        }
                        topl = topl->next;
                }

       }
       else
       {
               /*
                * we have NLM_RANGE_NUM_OF_TOP_RANGES of entries in the list already.
                */

                topl = pvtdata->m_head;
                while (topl)
                {
                        if (topl->count < tmp->count)
                        {
                               if (topl == pvtdata->m_head )
                               {
                                        /*
                                         * Last node is moved to first position
                                         * and its content is modified,
                                         */
                                        newl = pvtdata->m_last;
                                        pvtdata->m_last->prev->next = NULL;
                                        pvtdata->m_last = pvtdata->m_last->prev;
                                        newl->count = tmp->count;
                                        newl->range = tmp->range;
                                        newl->next = topl;
                                        newl->prev = NULL;
                                        topl->prev = newl;
                                        pvtdata->m_head = newl;
                                        goto get_out1;

                               }
                               else if (topl == pvtdata->m_last)
                               {
                                       /* Simply modify the pvtdata->m_last elem */
                                       pvtdata->m_last->count = tmp->count;
                                       pvtdata->m_last->range = tmp->range;
                                       goto get_out1;

                               }
                               else /* Needs insertion in between */
                               {
                                       NlmListTopRange *x;
                                       x = topl->prev;
                                       newl = pvtdata->m_last;
                                       pvtdata->m_last->prev->next = NULL;
                                       pvtdata->m_last = pvtdata->m_last->prev;
                                       newl->next = topl;
                                       topl->prev = newl;
                                       x->next = newl;
                                       newl->prev = x;
                                       newl->count = tmp->count;
                                       newl->range = tmp->range;
                                       NlmRangeMgr__pvt_RearrangeList(rdb, newl,
                                                       NLM_RANGE_UP_SHIFT);
                                       goto get_out1;
                               }

                        }
                        topl = topl->next;
                }
       }

get_out1:
 ;
}


/*
 *  NlmRangeMgr__pvt_AddOccur : Adds a range to a  range tree. If the range is
 *  already existing, then simply increment its count, else create a new node
 *  for the range.
 *
 *  side effect:  There may be a new entry in the range tree or the count for
 *  some range is incremented by one.
 *
 *  Return Value: NONE
 */

void NlmRangeMgr__pvt_AddOccur(
                NlmRangeMgr *self,
                NlmRangeDb *rdb,
                NlmRange *rm
                )
{
        NlmRangeElement *tmp;
        nlm_u32 range, index;
        NlmRangeMgr__pvtdata *pvtdata;


        if (!self || !rdb || !rm)
                return;

        range = (rm->m_start) | (rm->m_end << 16);

        index = range % NLM_RANGE_TREE_TBL_DEPTH;

        pvtdata = (NlmRangeMgr__pvtdata *) rdb->m_pvtdata;

        tmp  = pvtdata->range_trees[index];

        if (!tmp)
        {
                pvtdata->range_trees[index] = tmp
                       =  (NlmRangeElement *) NlmCmAllocator__calloc(self->m_alloc_p,1, sizeof(NlmRangeElement));
                goto get_out;
        }

        while (tmp)
        {
                if (tmp->range == range)
                {
                        break;
                }
                if (tmp->range > range)
                {
                        if (tmp->left)
                        {
                                tmp = tmp->left;
                                continue;
                        }
                        tmp->left = (NlmRangeElement *) NlmCmAllocator__calloc(self->m_alloc_p,1, sizeof(NlmRangeElement));
                        tmp = tmp->left;
                        break;
                }
                if (tmp->range < range)
                {
                        if (tmp->right)
                        {
                                tmp = tmp->right;
                                continue;
                        }
                        tmp->right = (NlmRangeElement *) NlmCmAllocator__calloc(self->m_alloc_p,1, sizeof(NlmRangeElement));
                        tmp = tmp->right;
                        break;
                }
        }


	get_out:
        tmp->range = range;
        tmp->count++;

        /*
         * We may have to modify the top range list
         */

        NlmRangeMgr__pvt_AddtoTop(self, rdb, tmp, NLMTRUE);

}

#define NLM_RANGE_ST_SIZE 20

/*
 *  NlmRangeMgr__pvt_AdjustTopRange  : Traverses a range tree and keep adjusting
 *  the top range list whenever its necessary.
 *  It recursively traverses a tree to see if adjustment in the top range list
 *  is necessary. Because of the recursive nature of tree traversal, we may get
 *  a stack overflow problem when the tree has many many nodes. To minimize the
 *  stack overflow problem, the function processes ST_SIZE nodes (at max) of the
 *  tree at a time. Hence  #recursion  is reduced to almost
 *  "(nuber_of_nodes_in_tree / ST_SIZE)  + "nuber_of_nodes_in_tree % ST_SIZE)"
 *
 *  side effect: The top range list may get modified.
 *  Return value: NONE
 */

static void NlmRangeMgr__pvt_AdjustTopRange(
       NlmRangeMgr *self,
       NlmRangeDb *rdb,
       NlmRangeElement *tmp)
{
        NlmRangeElement *ptrs[NLM_RANGE_ST_SIZE]; /* array to hold range elements*/
        nlm_u32  last_elem = 0;
        nlm_u32 cs = 0 , ce = 0, i, j,found, added =0;

        if (!self || !tmp || !rdb) return;

        NlmCm__memset(ptrs, 0 , NLM_RANGE_ST_SIZE * sizeof(NlmRangeElement *));
        /* Put tree nodes into the array of nodeptrs */
        ptrs[0] = tmp;
        while (ce  < (NLM_RANGE_ST_SIZE  -1)  && ptrs[cs]){
                added = 0;
                while (cs <= ce  && ( ce + added )< (NLM_RANGE_ST_SIZE - 1) && ptrs[cs]) {
                    if (ptrs[cs]->right) {
                            ptrs[ce + ++added] = ptrs[cs]->right;
                    }
                    if (ce + added == NLM_RANGE_ST_SIZE - 1)
                            break;

                    if (ptrs[cs]->left) {
                            ptrs[ce + ++added] = ptrs[cs]->left;
                    }
                    if (ce + added == NLM_RANGE_ST_SIZE - 1)
                            break;
                    cs++;
                }
                cs = ce + 1;
                ce = ce + added;
        }

        last_elem = ce;

        /* Process the nodes put on the array */

        for (i = 0; i <= last_elem; i++) {
            if (ptrs[i])
            {
                if (ptrs[i]->left)
                {
                    /* see if ptrs[i]->left is put on the array */

                    found = 0;
                    for (j = i + 1; j <= last_elem; j++)
                        if (ptrs[i]->left == ptrs[j]){
                            found = 1;
                            break;
                        }
                    if (!found)
                        NlmRangeMgr__pvt_AdjustTopRange(self, rdb, ptrs[i]->left);
                }

                if (ptrs[i]->right)
                {
                    /* see if ptrs[i]->right is put on the array */

                    found = 0;
                    for (j = i + 1; j <= last_elem; j++)
                        if ((void *)ptrs[i]->right == (void *)ptrs[j]){
                            found = 1;
                            break;
                        }
                    if (!found)
                        NlmRangeMgr__pvt_AdjustTopRange(self, rdb, ptrs[i]->right);
                }
                if (ptrs[i]->count)
                    NlmRangeMgr__pvt_AddtoTop(self, rdb, ptrs[i], NLMFALSE);
            }
            else
                    break;
        }

}


/*
 *  NlmRangeMgr__pvt_RecalcMCOR  : Travesrses all the range trees in the rdb and
 *  finds the new MCOR.
 *  side effect: the top range list is recreated.
 *  Return value: NONE
 */

static void  NlmRangeMgr__pvt_RecalcMCOR(
                NlmRangeMgr *self,
                NlmRangeDb *rdb)
{
        NlmRangeMgr__pvtdata *pvtdata;
        nlm_u32 i;

        if (!self || !rdb) return;

        pvtdata = (NlmRangeMgr__pvtdata *) rdb->m_pvtdata;

        if (!pvtdata) return;

        for (i = 0; i < NLM_RANGE_TREE_TBL_DEPTH; i++)
        {
                if (pvtdata->range_trees[i])
                        NlmRangeMgr__pvt_AdjustTopRange(self, rdb, pvtdata->range_trees[i]);
        }
}

/*
 *  NlmRangeMgr__pvt_AdjustTop : Adjusts the top range list. Due to addition of
 *  ranges, new ranges may become  top range candidates. This routine will
 *  check if the range being added should go to the top range list.
 *
 *  side effect: the top range list may get modified.
 *  Return value: NONE
 */

static void NlmRangeMgr__pvt_AdjustTop(
                NlmRangeMgr *self,
                NlmRangeDb *rdb,
                NlmRangeElement *tmp
           )
{
       NlmRangeMgr__pvtdata *pvtdata = NULL;
       NlmListTopRange *listelem = NULL;
       NlmBool recalc = NLMFALSE;

       if (!self || !rdb || !tmp)
              return;

       pvtdata = (NlmRangeMgr__pvtdata *)rdb->m_pvtdata;


       if (!pvtdata || !pvtdata->m_last ||
                       (pvtdata->m_last->count > (tmp->count + 1))) return;

       /* Find if the range is there in the top list */

        listelem = pvtdata->m_head;

        while (listelem)
        {
                if (listelem->range == tmp->range)
                        break;
                listelem = listelem->next;
        }

        if (!listelem) return;

        listelem->count--;

        if (listelem->count < pvtdata->m_last->count)
            /* A different range may be a candidate for the top range list*/
                recalc = NLMTRUE;


        NlmRangeMgr__pvt_RearrangeList(rdb, listelem, NLM_RANGE_DOWN_SHIFT);

        if (0 == listelem->count)
        {
                pvtdata->elements_in_list--;

                if (0 == pvtdata->elements_in_list)
                        pvtdata->m_head = pvtdata->m_last = NULL;
                else if (pvtdata->m_head == listelem)
                {
                       pvtdata->m_head = listelem->next;
                       if (pvtdata->m_head)
                              pvtdata->m_head->prev = NULL;
                }
                else if (pvtdata->m_last == listelem)
                {
                        pvtdata->m_last = listelem->prev;
                        if (pvtdata->m_last)
                               pvtdata->m_last->next = NULL;
                }
                else
                {
                        NlmListTopRange *nextnext, *prevprev;
                        nextnext = listelem->next;
                        prevprev = listelem->prev;
                        nextnext->prev = prevprev;
                        prevprev->next = nextnext;
                }
                NlmCmAllocator__free(self->m_alloc_p, listelem);
        }



        if ((pvtdata->elements_in_list < NLM_RANGE_NUM_OF_TOP_RANGES) || recalc)
        {
                NlmRangeMgr__pvt_RecalcMCOR(self, rdb);
        }

}


/*
 *  NlmRangeMgr__pvt_AdjustOccur : It should be called when a range is deleted.
 *  It decrements the count for the range node and if the count for the range
 *  node becomes zero, we have to delete the node itself.
 *
 *  side effect: The range tree gets modified and top range list may get
 *  modified too.
 *
 *  Return Value: NONE
 */

static void NlmRangeMgr__pvt_AdjustOccur(
                NlmRangeMgr *self,
                NlmRangeDb *rdb,
                NlmRange *rm
                )
{
        NlmRangeElement *tmp, *parent = NULL;
        nlm_u32 range, index;
        NlmRangeMgr__pvtdata *pvtdata;


        if (!self || !rdb || !rm)
                return;

        range = (rm->m_start) | (rm->m_end << 16);

        index = range % NLM_RANGE_TREE_TBL_DEPTH;

        pvtdata = (NlmRangeMgr__pvtdata *) rdb->m_pvtdata;

        tmp  = pvtdata->range_trees[index];

        if (!tmp)
        {
                return;
        }

        while (tmp)
        {
                if (tmp->range == range)
                {
                        break;
                }
                if (tmp->range > range)
                {
                        if (!tmp->left) return;
                        parent = tmp;
                        tmp = tmp->left;
                }
                if (tmp->range < range)
                {
                        if (!tmp->right) return;
                        parent = tmp;
                        tmp = tmp->right;
                }
        }


        tmp->count--;
        NlmRangeMgr__pvt_AdjustTop(self, rdb, tmp);

        if (tmp->count) return;

        /* reference count zero , so delete it*/

        if (!parent)
        {
                if (tmp->left && !tmp->right)
                        pvtdata->range_trees[index] = tmp->left;
                else if (!tmp->left && tmp->right)
                        pvtdata->range_trees[index] = tmp->right;
                else if (!tmp->left && !tmp->right)
                        pvtdata->range_trees[index] = NULL;
                else
                {
                        pvtdata->range_trees[index] = tmp->left;
                        {
                                NlmRangeElement *elem = tmp->left;
                                while (elem->right)
                                      elem = elem->right;

                                elem->right = tmp->right;
                        }
                }
        }
        else
        {
                NlmBool right_child = NLMFALSE;

                if (parent->right == tmp) right_child = NLMTRUE;

                if (!tmp->left && !tmp->right)
                {
                        (NLMTRUE == right_child) ? (parent->right = NULL) :
                                                        (parent->left = NULL);


                }
                else if (!tmp->left && tmp->right)
                {
                       (NLMTRUE == right_child) ? (parent->right = tmp->right):
                                                   (parent->left = tmp->right);
                }
                else if (tmp->left && !tmp->right)
                {
                       (NLMTRUE == right_child) ? (parent->right = tmp->left):
                                                  (parent->left = tmp->left);


                }
                else
                {
                        if (NlmTRUE == right_child)
                            parent->right = tmp->left;
                        else
                            parent->left = tmp->left;

                        {
                                NlmRangeElement *elem = tmp->left;
                                while (elem->right)
                                      elem = elem->right;
                                elem->right = tmp->right;
                        }
                }

        }

        NlmCmAllocator__free(self->m_alloc_p, tmp);
}


/* -------------------------------- RangeDb Related -------------------------------- */

/****************************************
 * Function : NlmRangeMgr__pvt_AddtoDbList
 * Summary:
 * This function adds a database node in to the Doubly-Linked List of database.
 * Note: This function basically takes advantage of the fact that "where" the new node is
 * inserted has no relevance. Hence it inserts the newNode at the begining.
  ****************************************/
 static nlm_u32 NlmRangeMgr__pvt_AddtoDbList(NlmRangeDbList* dbList_p,
                                    NlmRangeDb* newDb,
									NlmCmAllocator *alloc_p
							   )
{
	NlmRangeDbNode* newNode;

	/* Note that here the memory for the node is allocated only.
	 * Memory for the Database should be allocated by the upper layer.
	 */
	newNode = NlmCmAllocator__calloc(alloc_p, 1, sizeof(NlmRangeDbNode));
	NlmCmDemand((newNode != NULL), "Out of memory.\n");
    newNode->m_database = newDb;

	newNode->m_prev		= NULL;
    newNode->m_next		= dbList_p->m_firstDb;
    if(dbList_p->m_firstDb)
        dbList_p->m_firstDb->m_prev = newNode;
    dbList_p->m_firstDb		= newNode;
    return NLMERR_OK;
}

/****************************************
 * Function : NlmRangeMgr__pvt_DelfromDbList
 * Summary:
 * This function deletes a database node from the Doubly-Linked List of database-nodes
 ****************************************/
 static nlm_u32 NlmRangeMgr__pvt_DelfromDbList(NlmRangeDbList* dbList_p,
									NlmRangeDb* deleteDb,
									NlmCmAllocator *alloc_p
							   	  )
{
	NlmRangeDbNode* node;
	/* Check the inputs. */
	if((NULL == dbList_p) || (NULL == deleteDb) || (dbList_p->m_firstDb == NULL) )
	{
		return NLMERR_FAIL;
	}
	node = dbList_p->m_firstDb;
	while(node)
	{
		/* If you have found the entry to be deleted */
		if(node->m_database == deleteDb)
		{
			if (NULL == node->m_prev)
                dbList_p->m_firstDb = node->m_next;
			else
				node->m_prev->m_next = node->m_next;

			if (NULL != node->m_next)				
			    node->m_next->m_prev = node->m_prev;

			/* Note that here the memory for the node is de-allocated only.
			 * Memory for the Database should be De-allocated by the upper layer.
			 */
			NlmCmAllocator__free(alloc_p, node);
			return NLMERR_OK;
		}

		node = node->m_next;
	}
    return NLMERR_FAIL;
}

/*
 *  NlmRangeMgr__FreeRangeTr:  Deallocates the memory for the range tree.
 *
 */

static void NlmRangeMgr__pvt_FreeRangeTr(NlmRangeMgr *self,
                               NlmRangeElement *tree)
{
        if (!self || !tree)
                return;

        if (tree->left) NlmRangeMgr__pvt_FreeRangeTr(self, tree->left);
        if (tree->right) NlmRangeMgr__pvt_FreeRangeTr(self, tree->right);

        NlmCmAllocator__free(self->m_alloc_p, tree);

}
/*
 *  NlmRangeMgr__pvt_FreePvtData: Frees the range trees and the top range list for
 *  the rdb.
 */
static void NlmRangeMgr__pvt_FreePvtData(NlmRangeMgr     *self,
                                     NlmRangeMgr__pvtdata *data
                                     )
{
        NlmListTopRange *ltop, *lnext = NULL;
        nlm_u32  i;

        if (!self || !data)
                return;

        ltop = data->m_head;
        if (ltop)
                lnext = ltop->next;
        while (ltop)
        {
                NlmCmAllocator__free(self->m_alloc_p, ltop);
                if (!lnext)
                        break;
                ltop = lnext;
                lnext = ltop->next;
        }

        for (i = 0; i < NLM_RANGE_HT_DEPTH; i++)
        {
			NlmRangeMgr__pvt_FreeRangeTr(self, data->range_trees[i]);
        }
}

/*
 *  NlmRangeMgr__pvt_GetTopRanges writes the NLM_RANGE_NUM_TOP_RANGES (or less
 *  than that) to range db statistics structure.
 *
 *  side effect: Range DB is updated with the MCORs.
 *  Return value: NONE
 */
void NlmRangeMgr__pvt_GetTopRanges(
                NlmRangeDb *rdb
                )
{
       NlmRangeDbStats *stats_p;
       nlm_u32 i = 0;
       NlmRangeMgr__pvtdata *pvtdata;
       NlmListTopRange *tr;

       if (!rdb) return;

       stats_p = &rdb->m_stats;
       pvtdata = (NlmRangeMgr__pvtdata *)rdb->m_pvtdata;

       tr = pvtdata->m_head;
       NlmCm__memset(&stats_p->m_topRanges[0][0],
                        0, sizeof(stats_p->m_topRanges));
       while(tr && (i < NLM_RANGE_NUM_TOP_RANGES))
       {
           stats_p->m_topRanges[i][1] = (nlm_u16)(tr->range >> 16);
           stats_p->m_topRanges[i][0] = (nlm_u16)(tr->range & 0xffff);
           i++;
           tr = tr->next;
       }
}

nlm_32 NlmRangeMgr__pvt_IsTopRange(
                NlmRangeDb *rdb,
                NlmRangeNode *rnode)
{
        nlm_u32 i;

        if (!rdb || !rnode)
                return -1;

        for (i = 0; i < NLM_RANGE_NUM_TOP_RANGES; i++)
        {
            if ((rdb->m_stats.m_topRanges[i][0] == rnode->m_range->m_start)
                &&(rdb->m_stats.m_topRanges[i][1] == rnode->m_range->m_end))
                return i;
        }

        return -1;
}

/* Structure to hold the chunks into which the 16-bit incoming range is broken into.
 * By default all the chunks are of size 2 bits
 */
typedef struct Chunk
{
	nlm_u32 width;
	nlm_u32 s;
	nlm_u32 e;
}Chunk;

/* Singly Liniked-list subranges into which the ranges are broken*/
typedef struct subRangeNode
{
	nlm_u32 numChunks;
	Chunk * chunks;	/* Array to store all the chunks for the particular range */
	struct subRangeNode * ptr2nxt;
}subRangeNode;

/* The Singly Linked-list of the subranges. The structure contains the first 
 * and the last node in the list.
 */
typedef struct subRangeList
{
	subRangeNode * head;
	subRangeNode * tail;
}subRangeList;

/****************************************
 * Function : NlmRangeMgr__pvt_genMAX
 * Parameters :
 * 	n = number of bits
 * Return value :
 * 	Largest nlm_u32eger that can be represented using n bits in binary.
 ****************************************/
static nlm_u32 NlmRangeMgr__pvt_genMAX(nlm_u32 n)
{
	nlm_u32 res=0;
	nlm_u32 i;
	for (i=0; i<n; i++)
	{
		res |= (1<<i);
	}
	return (res);
}

/****************************************
 * Function : NlmRangeMgr__pvt_AddSubRange
 * Parameters :
 * 	srl = Pointer to subrangelist to which new element needs to be added
 * 	numChunnks = Number of chunks for which space needs to be allocated
 * Return value :
 * 	None
 * Description :
 * 	This function just allocates space for a new array of chunks.
 * 	The number of chunks in the array is specified by numChunks.
 * 	The values in the array can be then stored by the parent.
 * 	The new node is added to the tail of the list (list of chunk arrays)
 ****************************************/
static void NlmRangeMgr__pvt_AddSubRange(subRangeList * srl, 
                                         nlm_u32 numChunks,
                                         NlmCmAllocator* alloc)
{
	subRangeNode * newNode = (subRangeNode *)NlmCmAllocator__malloc(alloc, sizeof(subRangeNode));
	newNode->chunks = (Chunk *)NlmCmAllocator__malloc(alloc, numChunks * sizeof(Chunk));
	newNode->numChunks = numChunks;
	newNode->ptr2nxt = NULL;

	if (srl->head == NULL)
	{
		srl->head = srl->tail = newNode;
	}
	else
	{
		srl->tail->ptr2nxt = newNode;
		srl->tail = newNode;
	}
}

/****************************************
 * Function : NlmRangeMgr__pvt_DelSubRangeList
 * Parameters :
 * 	srl = Pointer to subrangelist to which new element needs to be added
 * Return value :
 * 	None
 * Description :
 * 	Frees the memory allocated for a sub range list (each of its nodes).
 * 	It also frees the memory that was allocated for the the chunk arrays in each node.
 ****************************************/
void NlmRangeMgr__pvt_DelSubRangeList(
           NlmCmAllocator *alloc,
           subRangeList * srl
           )
{
	subRangeNode * tmp;

        if (!srl || !alloc) return;

	while(srl->head != NULL)
	{
		NlmCmAllocator__free(alloc, srl->head->chunks);
		tmp = srl->head->ptr2nxt;
		NlmCmAllocator__free(alloc,srl->head);
		srl->head = tmp;
	}
}


#define NLM_RANGE_SET_RESET_BIT(x, y, bit)\
{\
	nlm_u32 tmp_bit;\
	tmp_bit = (bit);\
        if ((tmp_bit) > 0)\
        {\
                (x) |= (1 << (y));\
        }\
}
/****************************************
 * Function : NlmRangeMgr__pvt_GetChunksEncoding
 * Parameters :
 * 	s = Lower bound of range
 * 	e = Upper bound of range
 * 	width = Width of the field to be encoded (16 for SRC or DST port field)
 * Return value :
 * 	None
 * Description :
 * 	Does the encoding for a particular range
 ****************************************/
static void NlmRangeMgr__pvt_GetChunksEncoding(nlm_u32 s, 
                                               nlm_u32 e, 
                                               nlm_u32 width,
                                               nlm_u32 *encoded_data,
                                               nlm_u32 *encoded_mask,
                                               nlm_u32 *cur_pos1,
                                               nlm_u32 *cur_pos2
                                               )
{
	if (s > e)
	{
		return;
	}
	else
	{
		nlm_u32 i, j = 0 , numZ, encwidth = (1 << width) - 1, diffw;
		numZ = NlmRangeMgr__pvt_genMAX(width) - e;

                diffw = encwidth - width;

		 for (i=0; i<numZ; i++, j++)
                {

                        if (j < diffw)
                        {
                                NLM_RANGE_SET_RESET_BIT(*encoded_data,
                                        *cur_pos2,
                                        0);
                                NLM_RANGE_SET_RESET_BIT(*encoded_mask,
                                        *cur_pos2,
                                        0);
                                (*cur_pos2)--;

                        }
                        else
                        {

                                NLM_RANGE_SET_RESET_BIT(*encoded_data,
                                        *cur_pos1,
                                        0);
                                NLM_RANGE_SET_RESET_BIT(*encoded_mask,
                                        *cur_pos1,
                                        0);
                                (*cur_pos1)--;
                        }
                }

		for (i=0; i<(e - s); i++, j++)
                {

                        if (j < diffw)
                        {
                            NLM_RANGE_SET_RESET_BIT(*encoded_data,
                                        *cur_pos2,
                                        0);
                            NLM_RANGE_SET_RESET_BIT(*encoded_mask,
                                        *cur_pos2 ,
                                        1);
                            (*cur_pos2)--;
                        }
                        else
                        {
                             NLM_RANGE_SET_RESET_BIT(*encoded_data,
                                        *cur_pos1,
                                        0);

                                NLM_RANGE_SET_RESET_BIT(*encoded_mask,
                                        *cur_pos1 ,
                                        1);
                                (*cur_pos1)--;
                        }
                }
		for (i=0; i<s; i++, j++)
                {
                        if (j < diffw)
                        {
                                NLM_RANGE_SET_RESET_BIT(*encoded_data,
                                        *cur_pos2 ,
                                        1);
                                NLM_RANGE_SET_RESET_BIT(*encoded_mask,
                                        *cur_pos2,
                                        0);
                                (*cur_pos2)--;
                        }

                        else
                        {

                                NLM_RANGE_SET_RESET_BIT(*encoded_data,
                                        *cur_pos1 ,
                                        1);
                                NLM_RANGE_SET_RESET_BIT(*encoded_mask,
                                        *cur_pos1 ,
                                        0);
                                (*cur_pos1)--;
                        }
                }
	}
}

/****************************************
 * Function : NlmRangeMgr__pvt_FindSplitChunk
 * Parameters :
 * 	ch[] = The array of chunks among which the split chunk must be found
 * 	numChunks = Number of chunks in the array
 * Return Value :
 * 	Index of the 'split chunk'
 ****************************************/
nlm_u32 NlmRangeMgr__pvt_FindSplitChunk(Chunk ch[], nlm_u32 numChunks)
{
	nlm_u32 i;
	for (i=0; i<numChunks; i++)
	{
		if (ch[i].s != ch[i].e)
		{
			return i;
		}
	}
	return 0;
}
/****************************************
 * Function : NlmRangeMgr__pvt_FindLastNonZero
 * Parameters :
 * 	ch[] = The array of chunks in which the last NonZero chunk must be found
 * 	numChunks = Number of chunks in the array
 * 	c = Index of split chunk
 * Return value :
 * 	If there exists a chunk with (index >) c with non zero value of ch.s
 * 		Returns the index of this chunk
 * 	Else
 * 		Returns c
 ****************************************/
nlm_u32 NlmRangeMgr__pvt_FindLastNonZero(Chunk ch[], nlm_u32 numChunks, nlm_u32 c)
{
	nlm_u32 i;

	for (i=numChunks-1; i>c; i--)
	{
		if (ch[i].s != 0)
			return i;
	}
	return c;
}

/****************************************
 * Function : NlmRangeMgr__pvt_FindLastNonFull
 * Parameters :
 * 	ch[] = The array of chunks in which the last NonZero chunk must be found
 * 	numChunks = Number of chunks in the array
 * 	c = Index of split chunk
 * Return value :
 * 	If there exists a chunk with (index > c) with a value of ch.e less than the maximum representable with its width
 * 		Returns the index of this chunk
 * 	Else
 * 		Returns c
 ****************************************/
nlm_u32 NlmRangeMgr__pvt_FindLastNonFull(Chunk ch[], nlm_u32 numChunks, nlm_u32 c)
{
	nlm_u32 i;

	for (i=numChunks-1; i>c; i--)
	{
		if (ch[i].e != NlmRangeMgr__pvt_genMAX(ch[i].width))
			return i;
	}
	return c;
}


/****************************************
 * Function : NlmRangeMgr__pvt_FindSubRanges
 * Parameters:
 *	srl = Pointer to the subRangeList struct where the subranges for the input range need to be stored
 *	fieldwidth = Width of the field for which SubRanges need to be found (16 for SRC or DST port value)
 *	v1 = Lower bound of the range
 *	v2 = Upper bound of the range
 *	chunkwidths[] = widths of the chunks. Sum of chunkwidths should be equal to fieldwidth
 * Return value:
 * 	The subranges for the input range are stored at the location pointed to by srl. This memory needs to be freed by the parent
 ****************************************/
static NlmRangeEncoded* 
NlmRangeMgr__pvt_FindSubRanges(subRangeList * srl, 
                               nlm_u32 fieldwidth,
                               NlmRange *range, 
                               nlm_u32 numChunks, 
                               nlm_u32 chunkWidths[], 
                               NlmCmAllocator* alloc)
{
	nlm_u32 encoded_data,encoded_mask  ;
	nlm_u32 i, j;		/* Indices used in loops */
	nlm_u32 fieldwidthcheck;	/* Width of the field */
	nlm_u32 c;			/* Split Chunk */
	nlm_u32 lastNonZero, lastNonFull;
    nlm_u32 v1, v2;
	Chunk splitChunk;
    nlm_u32 outputWidth = 0;
    NlmRangeEncoded *encodedRange = NULL;

	Chunk * chunks;		/* To store all the chunks */

        if (!srl || !range )
        {
                return NULL;
        }

	srl->head = srl->tail = NULL;
        v1 = range->m_start;
        v2 = range->m_end;

	if ((v1 < 0) || (v1 > NlmRangeMgr__pvt_genMAX(fieldwidth)))
	{
		return NULL;
	}

	if ((v2 < 0) || (v2 > NlmRangeMgr__pvt_genMAX(fieldwidth)))
	{
		return NULL;
	}

	if (v1 > v2)
	{
		return NULL;
	}

	chunks = (Chunk*) NlmCmAllocator__malloc(alloc, numChunks * sizeof(Chunk));

	for (i=0; i<numChunks; i++)
	{
                if (chunkWidths[i] <= 0)
                {
                        NlmCmAllocator__free(alloc, chunks) ;
                        return NULL;
                }
		chunks[i].width = chunkWidths[i];
                outputWidth +=  (1 << chunkWidths[i]) - 1;
	}


	/* Assign values to the fields of chunks */
	fieldwidthcheck = 0;
	for (i=0; i<numChunks; i++)
	{
		fieldwidthcheck += chunks[i].width;
		chunks[i].s = (v1 >> (fieldwidth - fieldwidthcheck)) & NlmRangeMgr__pvt_genMAX(chunks[i].width);
		chunks[i].e = (v2 >> (fieldwidth - fieldwidthcheck)) & NlmRangeMgr__pvt_genMAX(chunks[i].width);
	}
	if (fieldwidthcheck != fieldwidth)
	{
                if (chunks)
                        NlmCmAllocator__free(alloc, chunks);
		return NULL;
	}


	if (v1 == v2)
	{

        subRangeNode *head;
		NlmRangeDMFormat *rdm;
		NlmRangeMgr__pvt_AddSubRange(srl, numChunks,alloc);
		for (i=0; i<numChunks; i++)
		{
			srl->tail->chunks[i] = chunks[i];
		}
		head = srl->head;
		encodedRange = (NlmRangeEncoded *)NlmCmAllocator__malloc(alloc, sizeof(NlmRangeEncoded));
		encodedRange->m_num_entries = 1;
		encodedRange->m_id = range->m_id;
		encodedRange->m_entries_p = (NlmRangeDMFormat *)
			NlmCmAllocator__malloc(alloc, sizeof(NlmRangeDMFormat));

		rdm = encodedRange->m_entries_p;

	    while (head)
        {
                nlm_u32 position1;
                nlm_u32 position2;
                position1 = 31;
                position2 = 31 - fieldwidth;

                encoded_data = 0;
                encoded_mask = 0xff;

                for (i = 0; i < srl->tail->numChunks ; i++ )
                {
                        NlmRangeMgr__pvt_GetChunksEncoding(head->chunks[i].s,
                                                           head->chunks[i].e,
                                                           head->chunks[i].width, 
                                                           &encoded_data,
                                                           &encoded_mask,
                                                           &position1,
                                                           &position2);

                }
                rdm->m_data = encoded_data;	/*
                                                *  mask bit is 1, same 1
                                                * may be in data also
                                                */

                rdm->m_mask = encoded_mask;
                rdm++;
                head = head->ptr2nxt;
        }
		NlmRangeMgr__pvt_DelSubRangeList(alloc,srl);
                if (chunks)
                        NlmCmAllocator__free(alloc, chunks);
		return encodedRange;
	}

	c = NlmRangeMgr__pvt_FindSplitChunk(chunks, numChunks);

	lastNonZero = NlmRangeMgr__pvt_FindLastNonZero(chunks, numChunks, c);
	lastNonFull = NlmRangeMgr__pvt_FindLastNonFull(chunks, numChunks, c);

	splitChunk.width = chunks[c].width;

	if (lastNonFull != c)
		splitChunk.e = chunks[c].e - 1;
	else
		splitChunk.e = chunks[c].e;

	if (lastNonZero != c)
		splitChunk.s = chunks[c].s + 1;
	else
		splitChunk.s = chunks[c].s;


	/* Now form the TCAM entry for the biggest chunk */
	/* It is the range [s + 1, e - 1] for the place value of chunk c */

	if (splitChunk.s <= splitChunk.e)
	{
		NlmRangeMgr__pvt_AddSubRange(srl, numChunks, alloc);
		for (i=0; i<numChunks; i++)
		{
			srl->tail->chunks[i] = chunks[i];
		}

		srl->tail->chunks[c] = splitChunk;

		for (i=c+1; i<numChunks; i++)
		{
			srl->tail->chunks[i].s = 0;
			srl->tail->chunks[i].e = NlmRangeMgr__pvt_genMAX(chunks[i].width);
			srl->tail->chunks[i].width = chunks[i].width;
		}
	}

	/* Now form entries for the >=s portion */

	if (lastNonZero > c)
	{
		for (i=c+1; i<=lastNonZero; i++)
		{
			if ((i != lastNonZero) && (chunks[i].s == NlmRangeMgr__pvt_genMAX(chunks[i].width)))
				continue;

			NlmRangeMgr__pvt_AddSubRange(srl, numChunks, alloc);

			for (j=0; j<i; j++)
			{
				srl->tail->chunks[j].s = chunks[j].s;
				srl->tail->chunks[j].e = chunks[j].s;
				srl->tail->chunks[j].width = chunks[j].width;
			}

			if (i == lastNonZero)
			{
				srl->tail->chunks[i].s = chunks[i].s;
				srl->tail->chunks[i].e = NlmRangeMgr__pvt_genMAX(chunks[i].width);
				srl->tail->chunks[i].width = chunks[i].width;
			}
			else if (chunks[i].s != NlmRangeMgr__pvt_genMAX(chunks[i].width))
			{
				srl->tail->chunks[i].s = chunks[i].s + 1;
				srl->tail->chunks[i].e = NlmRangeMgr__pvt_genMAX(chunks[i].width);
				srl->tail->chunks[i].width = chunks[i].width;
			}

			for (j=i+1; j<numChunks; j++)
			{
				srl->tail->chunks[j].s = 0;
				srl->tail->chunks[j].e = NlmRangeMgr__pvt_genMAX(chunks[j].width);
				srl->tail->chunks[j].width = chunks[j].width;
			}
		}
	}	/* End of >=s portion */

	/* Now form entries for the <=e portion */

	if (lastNonFull > c)
	{
		for (i=c+1; i<=lastNonFull; i++)
		{
			if ((i != lastNonFull) && (chunks[i].e == 0))
				continue;

			NlmRangeMgr__pvt_AddSubRange(srl, numChunks, alloc);

			for (j=0; j<i; j++)
			{
				srl->tail->chunks[j].s = chunks[j].e;
				srl->tail->chunks[j].e = chunks[j].e;
				srl->tail->chunks[j].width = chunks[j].width;
            }

			if (i == lastNonFull)
			{
				srl->tail->chunks[i].s = 0;
				srl->tail->chunks[i].e = chunks[i].e;
				srl->tail->chunks[i].width = chunks[i].width;
			}
			else if (chunks[i].e != 0)
			{
				srl->tail->chunks[i].s = 0;
				srl->tail->chunks[i].e = chunks[i].e - 1;
				srl->tail->chunks[i].width = chunks[i].width;
			}

			for (j=i+1; j<numChunks; j++)
			{
				srl->tail->chunks[j].s = 0;
				srl->tail->chunks[j].e = NlmRangeMgr__pvt_genMAX(chunks[j].width);
				srl->tail->chunks[j].width = chunks[j].width;
			}
		}
	}	/* End of <=e portion */

        NlmCmAllocator__free(alloc, chunks);

        {

                subRangeList *tmp;
                subRangeNode *head;
				NlmRangeDMFormat *rdm;
				nlm_u32 i, num_of_encoded_entries = 0;
                tmp = srl;


                head = tmp->head;

                /* Find the number of encoded entries */

                while(head)
                {
                        head = head->ptr2nxt;
                        num_of_encoded_entries++;
                }


                encodedRange = (NlmRangeEncoded *)
                        NlmCmAllocator__malloc(alloc,sizeof(NlmRangeEncoded));
                if (!encodedRange )
                {

                        NlmRangeMgr__pvt_DelSubRangeList(alloc,srl);
                        return NULL;
                }

                encodedRange->m_id = range->m_id;
                encodedRange->m_num_entries = num_of_encoded_entries;

                encodedRange->m_entries_p = (NlmRangeDMFormat *)
                      NlmCmAllocator__malloc(alloc, sizeof(NlmRangeDMFormat) * num_of_encoded_entries);

                if (!encodedRange->m_entries_p)
                {
                       NlmCmAllocator__free(alloc, encodedRange);
                        NlmRangeMgr__pvt_DelSubRangeList(alloc,srl);
                        return NULL;
                }

                rdm = encodedRange->m_entries_p;

                head = tmp->head;

                while (head)
                {
                        nlm_u32 position1;
                        nlm_u32 position2;
                        position1 = 31;
                        position2 = 31 - fieldwidth;

                        encoded_data = 0;
                        encoded_mask = 0xff;

                        for (i = 0; i < tmp->tail->numChunks ; i++ )
                        {
                                NlmRangeMgr__pvt_GetChunksEncoding(head->chunks[i].s,
                                                                   head->chunks[i].e,
                                                                   head->chunks[i].width,
                                                                   &encoded_data,
                                                                   &encoded_mask,
                                                                   &position1,
                                                                   &position2);

                        }
                        rdm->m_data = encoded_data; /*
                                                     *  mask bit is 1, same 1
                                                     * may be in data also
                                                     */

                        rdm->m_mask = encoded_mask;
                        rdm++;
                        head = head->ptr2nxt;
                }


        }

        NlmRangeMgr__pvt_DelSubRangeList(alloc,srl);
        return encodedRange;

}	/* End of NlmRangeMgr__pvt_FindSubRanges */


nlm_u32 NlmRangeMgr__pvt_GetPrefixEncodingCount(
    nlm_u32 startValue,
    nlm_u32 endValue,
    nlm_u32 lowerBound,
    nlm_u32 upperBound
    )
{
    unsigned int middle;
    unsigned int result1=0;
    unsigned int result2=0;
	unsigned int tmpEndValue;
    unsigned int tmpStartValue;

	if(startValue > upperBound || endValue < lowerBound) /* out of bound */
		return 0;
	else if(startValue == lowerBound && endValue == upperBound) /* match the bound */
		return 1;
	else
    {	/* doesn't match the bound; split the bound */
		middle = (lowerBound + upperBound) /2;

        tmpEndValue = endValue;
		if(endValue > middle)
			tmpEndValue = middle;
		if(startValue <= tmpEndValue)
			result1 = NlmRangeMgr__pvt_GetPrefixEncodingCount(startValue, tmpEndValue, lowerBound, middle);

		tmpStartValue = startValue;
		if(startValue < (middle + 1))
			tmpStartValue = middle + 1;
		if(tmpStartValue <= endValue)
			result2 = NlmRangeMgr__pvt_GetPrefixEncodingCount(tmpStartValue, endValue, middle + 1, upperBound);
		return result1+result2;
	}
}

nlm_u32 NlmRangeMgr__pvt_ReplaceTrailingZeroesWithOnes(
    nlm_u32 value
    )
{
    nlm_u32 i = 0;

    if(value == 0)
        return 0xFFFF;

    while(((value >> i) & 1) == 0)
        i++;
    return (value + ((1 << i) - 1));
}

void NlmRangeMgr__pvt_GetPfxEncoding(nlm_u32 *startValue,
                                     nlm_u32 endValue,
                                     NlmRangeDMFormat *encoding
                                     )
{
    nlm_u32 tmp = 1;
    nlm_u32 iter = 1;

    nlm_u32 start = *startValue;
    if(start % 2)
    {
        encoding->m_data = (start << 16);
        encoding->m_mask = 0xFFFF;
        (*startValue)++;
        return;
    }
    while((start + (tmp - 1)) <= endValue)
    {
        encoding->m_data = (start << 16);
        encoding->m_mask = ((tmp - 1) << 16) | 0xFFFF;
        *startValue = start + tmp;
        tmp = 1 << (iter++);
    }

    return;
}

NlmRangeEncoded* NlmRangeMgr__pvt_GeneratePrefixEncodings(
    NlmRange *range_p,
    NlmCmAllocator *alloc_p
    )
{

    NlmRangeEncoded *encodedHead;
    nlm_u32 encodingsCount;
    nlm_u32 tmp;
    nlm_u32 startValue = (nlm_u32)range_p->m_start;
    nlm_u32 endValue = (nlm_u32)range_p->m_end;

    encodedHead = malloc(sizeof(NlmRangeEncoded));

    /* Get the prefix encodings count */
    encodingsCount = NlmRangeMgr__pvt_GetPrefixEncodingCount(startValue,
                                                             endValue,
                                                             0,
                                                             ((1 << 16) - 1));

    encodedHead->m_entries_p = NlmCmAllocator__calloc(alloc_p,
                                                      encodingsCount,
                                                      sizeof(NlmRangeDMFormat));
    encodedHead->m_id = range_p->m_id;
    encodedHead->m_num_entries = 0;

    while(startValue <= endValue)
    {
        /* Replace the trailing zeroes of the "start value" with ones
        if the value obtained "tmp" is less than the "end value" get the prefix
        encoding for [start value - tmp] else get the prefix encoding for the
        start value - end value]*/
        tmp = NlmRangeMgr__pvt_ReplaceTrailingZeroesWithOnes(startValue);

        if(tmp < endValue)
            NlmRangeMgr__pvt_GetPfxEncoding(&startValue,
                                            (nlm_u16)tmp,
                                            &encodedHead->m_entries_p[encodedHead->m_num_entries]
                                            );
        else
            NlmRangeMgr__pvt_GetPfxEncoding(&startValue,
                                            endValue,
                                            &encodedHead->m_entries_p[encodedHead->m_num_entries]
                                            );
        encodedHead->m_num_entries++;
    }

    return encodedHead;
}

#define NLM_RANGE_MGR_EXTRACT_BITS(data, numbits)  ((nlm_u8)(data & (~(~0 << numbits))))

void NlmRangeMgr__pvt_DoConditioningBasedOnDbAttrs(
    NlmRangeEncoded *rangeEncodings,
    NlmRangeDb *rangeDb
    )
{
    nlm_u32 encodingNum;
    NlmRangeDMFormat *encoding;
    nlm_u8 bData = 0;
    nlm_u8 bMask = 0;  
    nlm_u8 aData = 0;
    nlm_u8 aMask = 0;      
    nlm_u16 paData = 0;
    nlm_u16 paMask = 0;


    for(encodingNum = 0; encodingNum < rangeEncodings->m_num_entries; encodingNum++)
    {
        encoding = &rangeEncodings->m_entries_p[encodingNum];
        if(rangeDb->m_range_code == NLM_RANGE_3B_ENCODING)
        {
            /* Separate Fence Encoding Part From and MCOR Encodings */
            bData = (nlm_u8)(encoding->m_data & 0xFF);
            bMask = (nlm_u8)(encoding->m_mask & 0xFF);  
            aData = (nlm_u8)((encoding->m_data >> 0x8) & 0xFF);
            aMask = (nlm_u8)((encoding->m_mask >> 0x8) & 0xFF);
            paData = (nlm_u16)((encoding->m_data >> 16) & 0xFFFF);
            paMask = (nlm_u16)((encoding->m_mask >> 16) & 0xFFFF);            

            /* Put the dont cares at the Bits which are not valid */
            paMask = paMask | (0xFFFF << rangeDb->m_valid_bits);
            aMask = aMask | (0xFF << (rangeDb->m_valid_bits/2));                        
        }
        else 
        {
            /* Separate Prefix Encoding Part From and MCOR Encodings */
            bData = (nlm_u8)(encoding->m_data & 0xFF);
            bMask = (nlm_u8)(encoding->m_mask & 0xFF);  
            paData = (nlm_u16)((encoding->m_data >> 16) & 0xFFFF);
            paMask = (nlm_u16)((encoding->m_mask >> 16) & 0xFFFF);       
            
            /* Put the dont cares at the Bits which are not valid */
            paMask = paMask | (0xFFFF << rangeDb->m_valid_bits);     
        }
        
        switch (rangeDb->m_num_bits)/* Format the encodings based on num_of_bits */
        {
            case 32: /* MCOR + Fence Encoding (B + A + PA) */
                    /* Bits[31:24] - MCOR Encoding */
                    /* Bits[23:0] - Fence Encoding */
                encoding->m_data = (bData << 24) | (aData << 16) | paData;
                encoding->m_mask = (bMask << 24) | (aMask << 16) | paMask;                
                break;
            case 24:

                if(rangeDb->m_range_code == NLM_RANGE_3B_ENCODING)
                {
                    /* Only Fence Encoding (A + PA) */                    
                    /* Bits[23:0] - Fence Encoding */
                    encoding->m_data = (aData << 16) | paData;
                    encoding->m_mask = (aMask << 16) | paMask;  
                }
                else
                {
                    /* MCOR + Prefix Encoding (B + PA) */
                    /* Bits[23:16] - MCOR Encoding */
                    /* Bits[15:0] - Prefix Encoding */
                    encoding->m_data = (bData << 16) | paData;
                    encoding->m_mask = (bMask << 16) | paMask;     
                }
                break;

            case 16:/* Only Prefix Encoding (PA)   */                   
                    /* Bits[15:0] - Prefix Encoding */
                encoding->m_data = paData;
                encoding->m_mask = paMask;
                break;

            case 8:/* Only MCOR (B)   */                   
                    /* Bits[7:0] - MCOR  */
                encoding->m_data = bData;
                encoding->m_mask = bMask;
                break;
            
            default:
                NlmCmAssert(0, "Invalid Range Db Attrs: Number of Bits");
                return;            
        }
    }
}



/****************************************
 * Function : NlmRangeMgr__AddRange
 *
 * Parameters:
 * NlmRangeMgr *pRangeMgr	= Pointer to the Range Manager
 * NlmRangeDb *rangeDb	= Pointer to the Database which is to be modified
 * NlmRange *newRange		= Pointer to the new range field which is to be added
 * NlmReasonCode *o_reason	= Location to save the reason code
 *
 * Summary:
 * Ranges can only be added to pre-created, configured Databases. Every range has a unique id, and
 * duplicate range values are supported within the same Database. The mapping of duplicate ranges
 * to Processor entries may be identical and will be maintained in the Range Manager. This function does
 * not cause any change in the actual device, but only adds the range to the Database inside SW.
 ****************************************/
NlmErrNum_t NlmRangeMgr__AddRange(
	NlmRangeMgr		*self,
	NlmRangeDb		*rangeDb,
	NlmRange		*range,
	NlmReasonCode	*o_reason
	)
{
	NlmErrNum_t errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock( &self->m_spinLock );
#endif

	errNum = NlmRangeMgr__pvt_AddRange(self,
										rangeDb,
										range,
										o_reason);
#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock( &self->m_spinLock );
#endif

	return errNum;
}

static NlmErrNum_t 
NlmRangeMgr__pvt_AddRange(
	NlmRangeMgr		*self,
	NlmRangeDb		*rangeDb,
	NlmRange		*range,
	NlmReasonCode	*o_reason
	)
{
	NlmRange* newRange = NULL;

	/* Check the inputs */
	if(NULL == self)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE_MGR;
		return NLMERR_NULL_PTR;
	}
	if(NULL == rangeDb)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATABASE;
		return NLMERR_NULL_PTR;
	}
	if(NULL == range)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE;
		return NLMERR_NULL_PTR;
	}
	if(range->m_end < range->m_start)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE;
		return NLMERR_FAIL;
	}
    if(range->m_start >= (1  << rangeDb->m_valid_bits)
        ||range->m_end >= (1  << rangeDb->m_valid_bits))
    {
        /* Range Start Value and End Value needs to be within the valid bits */
        if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE;
		return NLMERR_FAIL;
    }

	if(NULL != ( NlmRangeMgr__pvt_SearchRangeList(
						&(rangeDb->m_entries[range->m_id % NLM_RANGE_HT_DEPTH]),
									range->m_id)))
	{
		if(o_reason)
			*o_reason = NLMRSC_DUPLICATE_RANGE_ID;
		return NLMERR_FAIL;
	}

       /* Check if 8 MCOR ranges are already present with MCOR only encoding then 
       return error. */       
       if((rangeDb->m_range_count == 8) && (rangeDb->m_num_bits == 8))
       {
               if(o_reason)
             *o_reason = NLMRSC_RANGE_DATABASE_FULL;
         return NLMERR_FAIL;
         
       }

	/* Allocate memory for the New Range in software memory space */
	newRange = NlmCmAllocator__malloc(self->m_alloc_p, sizeof(NlmRange));
    NlmCmDemand((newRange != NULL), "Out of memory.\n");
	NlmCm__memcpy(newRange, range, sizeof(NlmRange));

	/* Making the encoded_head of the range to be NULL as there no encodings for it now.
	 * Note:- Making it NULL is mandatory here because the information that whether the
	 * newRange->m_encoded_p points to valid data or not is based on the fact that this pointer
	 * is NULL or not, this check is used in __CreateEncodings()
	 */
	newRange->m_encoded_p = NULL;

	/* Selecting the list in which the new range is to be added.
	 * We feed the id to the hash-function and then select the corresponding index
	 * produced by the hash function, thus choosing the 'link-list of ranges'
	 * which is going to contain the new-range
	 */
	NlmRangeMgr__pvt_AddtoRangeList(
		&(rangeDb->m_entries[(newRange->m_id) % NLM_RANGE_HT_DEPTH]),
		newRange,
		self->m_alloc_p ); 

	/* increase the count  */
	rangeDb->m_range_count++;
    
    if(rangeDb->m_use_mcor)    
        NlmRangeMgr__pvt_AddOccur(self, rangeDb, range);    

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;

}

/****************************************
 * Function : NlmRangeMgr__DeleteRange
 *
 * Parameters:
 * NlmRangeMgr *pRangeMgr	= Pointer to the Range Manager
 * NlmRangeDb *rangeDb	= Pointer to the Database to which the range to be deleted belongs
 * nlm_u32   id				= The id of the Range to be deleted from database
 * NlmReasonCode *o_reason	= Location to save the reason code
 *
 * Summary:
 * This API deletes the range only from the SW, and does not impact the range in HW. Also, deleting
 * one range of X <= R1 <= Y will not impact another range X <= R2 <= Y. Every range has a unique
 * range ID and is treated separately.
 ****************************************/
NlmErrNum_t NlmRangeMgr__DeleteRange(
	NlmRangeMgr     *self,
	NlmRangeDb 	    *rangeDb,
	nlm_u32			id,
	NlmReasonCode	*o_reason
	)
{
	NlmErrNum_t errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT

	NlmCmMt__SpinLock( &self->m_spinLock );
#endif
	
	errNum = NlmRangeMgr__pvt_DeleteRange(self,
										rangeDb,
										id,
										o_reason);

#if defined NLM_MT_OLD || defined NLM_MT

		NlmCmMt__SpinUnlock( &self->m_spinLock );
#endif

	return errNum;
}

static NlmErrNum_t 
NlmRangeMgr__pvt_DeleteRange(
	NlmRangeMgr     *self,
	NlmRangeDb 	    *rangeDb,
	nlm_u32			id,
	NlmReasonCode	*o_reason
	)
{
	NlmRange* rangeToBeDeleted;

	/* Check the inputs. */
	if(NULL == self)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE_MGR;
		return NLMERR_NULL_PTR;
	}
	if(NULL == rangeDb)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATABASE;
		return NLMERR_NULL_PTR;
	}

	/* Delete the node from the Doubly Linked List by calling the internal function */
	if(NLMERR_OK != NlmRangeMgr__pvt_DelfromRangeList(
			&(rangeDb->m_entries[id % NLM_RANGE_HT_DEPTH]),
			id,
			&rangeToBeDeleted,
			self->m_alloc_p
			))
    {
		if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE;
		return NLMERR_FAIL;
	}	

    NlmRangeMgr__pvt_AdjustOccur(self, rangeDb, rangeToBeDeleted);

    if(rangeToBeDeleted->m_encoded_p)/* If there are encodings free them */
    {
        NlmCmAllocator__free(self->m_alloc_p, rangeToBeDeleted->m_encoded_p->m_entries_p);
        NlmCmAllocator__free(self->m_alloc_p, rangeToBeDeleted->m_encoded_p);
    }

	NlmCmAllocator__free(self->m_alloc_p, rangeToBeDeleted);

    /* decrease the count  */
	rangeDb->m_range_count--;
	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;

}

/****************************************
 * Function : NlmRangeMgr__AssignRange
 *
 * Parameters:
 * NlmRangeMgr *pRangeMgr	= Pointer to the Range Manager
 * NlmRangeDb *rangeDb	= Pointer to the Range Database which is MCORs are being used
 * NlmRangeType   rangeType	= The rangeType to be assigned to the database (A, B, C or D). See the .h file.
 * NlmReasonCode *o_reason	= Location to save the reason code
 *
 * Summary:
 * This API assigns which database, among the existing databases,
 * would be using the MCORs; One database of each range type can use 
 * MCORs; 
 *****************************************/
NlmErrNum_t NlmRangeMgr__AssignRange(
	NlmRangeMgr		*self,
	NlmRangeDb		*rangeDb,
	NlmRangeType	rangeType,
	NlmReasonCode	*o_reason
	)
{
	NlmErrNum_t errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT

	NlmCmMt__SpinLock( &self->m_spinLock );
#endif

	errNum = NlmRangeMgr__pvt_AssignRange(self,
										rangeDb,
										rangeType,
										o_reason);
#if defined NLM_MT_OLD || defined NLM_MT

		NlmCmMt__SpinUnlock( &self->m_spinLock );
#endif

	return errNum;
}

static NlmErrNum_t 
NlmRangeMgr__pvt_AssignRange(
	NlmRangeMgr		*self,
	NlmRangeDb		*rangeDb,
	NlmRangeType	rangeType,
	NlmReasonCode	*o_reason
	)
{	
	 NlmReasonCode dummyReasonCode;

        if(!o_reason)
        o_reason = &dummyReasonCode;

        /* Check the inputs. */
	if(NULL == self)
	{
		*o_reason = NLMRSC_INVALID_RANGE_MGR;
		return NLMERR_NULL_PTR;
	}
	if(NULL == rangeDb)
	{
		*o_reason = NLMRSC_INVALID_DATABASE;
		return NLMERR_NULL_PTR;
	}
	if((NLM_RANGE_TYPE_A != rangeType) && (NLM_RANGE_TYPE_B != rangeType)
        && (NLM_RANGE_TYPE_C != rangeType) && (NLM_RANGE_TYPE_D != rangeType))
	{
		*o_reason = NLMRSC_INVALID_RANGE_SELECTION;
		return NLMERR_FAIL;
	}
    if(rangeDb->m_num_bits == 16 || 
        (rangeDb->m_num_bits == 24 && rangeDb->m_range_code == NLM_RANGE_3B_ENCODING))
     {
        *o_reason = NLMRSC_INVALID_MCOR_CONFIG;
	 return NLMERR_FAIL;
    }
    rangeDb->m_use_mcor = 1;
    rangeDb->m_range_type = rangeType;	

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;
}

/****************************************
 * Function : NlmRangeMgr__CreateEncodings
 *
 * Parameters:
 * NlmRangeMgr	*pRangeMgr	= Pointer to the Range Manager
 * NlmRangeDb *rangeDb	= Pointer to the Database, the range fields of which are to be encoded
 * NlmReasonCode *o_reason	= Location to save the reason code
 *
 * Summary:
 * This function goes over all the ranges in a Range Database, and creates the optimal mappings/encodings
 * for all of them. Since the encoding for the most popular ranges depends on the presence of other
 * ranges in the Database, this function will create the most optimal mapping only when all the ranges
 * have been added to the Database already. Calling this function after every AddRange will lead to
 * expensive recalculations for all ranges. If MCORs are used for the Database then this function
 * updates the hardware with MCOR values.
 ****************************************/
NlmErrNum_t NlmRangeMgr__CreateEncodings(
	NlmRangeMgr		*self,
	NlmRangeDb		*rangeDb,
	NlmReasonCode	*o_reason
	)
{
	NlmErrNum_t errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT

	NlmCmMt__SpinLock( &self->m_spinLock );
#endif

	errNum = NlmRangeMgr__pvt_CreateEncodings(self,
												rangeDb,
												o_reason);
#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock( &self->m_spinLock );
#endif

	return errNum;

}

static NlmErrNum_t 
NlmRangeMgr__pvt_CreateEncodings(
	NlmRangeMgr		*self,
	NlmRangeDb		*rangeDb,
	NlmReasonCode	*o_reason
	)
{
	nlm_u32	iter;
	NlmRangeNode* node = NULL ;
	NlmRangeEncoded* encoded_head = NULL;
	subRangeList srl;
	nlm_u32 chunkws[] = {2,2,2,2,2,2,2,2};
    NlmErrNum_t catchErr;

	/* Check the inputs. */
	if(NULL == self)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE_MGR;
		return NLMERR_NULL_PTR;
	}
	if(NULL == rangeDb)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATABASE;
		return NLMERR_NULL_PTR;
    }

    if(rangeDb->m_use_mcor)/* If MCORs used for the database */
    {
        /*Get the snapshot of Top 8 MCORs */
        NlmRangeMgr__pvt_GetTopRanges(rangeDb);

        /* Write the MCOR values in the Corresponding Range Bound Registers  */
        if(NLMERR_OK != (catchErr = self->m_vtbl.m_initBoundRegsFnPtr(self,
                                            rangeDb->m_stats.m_topRanges,
                                            rangeDb->m_valid_bits,
                                            rangeDb->m_range_type,
                                            o_reason)))
		return catchErr;
    }

    srl.head = NULL;
	srl.tail = NULL;

	for(iter = 0; iter < NLM_RANGE_HT_DEPTH; iter++)
	{
        nlm_32 ret_val;
		node = rangeDb->m_entries[iter].m_firstRange;
		while(node)
		{
            /* Free the encodings from the previous __CreateEncodings() call */
			if(node->m_range->m_encoded_p)
            {
                NlmCmAllocator__free(self->m_alloc_p, node->m_range->m_encoded_p->m_entries_p);
                NlmCmAllocator__free(self->m_alloc_p, node->m_range->m_encoded_p);
            }
            if (rangeDb->m_use_mcor && ((ret_val = NlmRangeMgr__pvt_IsTopRange(rangeDb, node)) >= 0))
            {
                 /* If the range is within the Top 8 Ranges do MCOR encoding */
                encoded_head = (NlmRangeEncoded *)
                    NlmCmAllocator__malloc(self->m_alloc_p, sizeof(NlmRangeEncoded));
                encoded_head->m_num_entries = 1;
                encoded_head->m_entries_p =
                    NlmCmAllocator__calloc(self->m_alloc_p, 1,  sizeof(NlmRangeDMFormat));
                encoded_head->m_entries_p->m_data = 0xFFFFFF00 | (1 << ret_val);
                encoded_head->m_entries_p->m_mask = 0xFFFFFF00 | (~(1 << ret_val));
            }
            else
            {
                if(rangeDb->m_range_code == NLM_RANGE_3B_ENCODING)/* If 3b encoding perform Fence Encoding*/
                    encoded_head = NlmRangeMgr__pvt_FindSubRanges(&srl, 16,node->m_range, 8, chunkws, self->m_alloc_p);
                else if(rangeDb->m_range_code == NLM_RANGE_2B_ENCODING)/* If 2b encoding perform Prefix Encoding */
                    encoded_head = NlmRangeMgr__pvt_GeneratePrefixEncodings(node->m_range,
                                                                       self->m_alloc_p);
            }
            
            /* Do conditioning based on Db Attrs */
            NlmRangeMgr__pvt_DoConditioningBasedOnDbAttrs(encoded_head, rangeDb);

			node->m_range->m_encoded_p = encoded_head;
			node= node->m_next;
		}
	}

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;
}


/****************************************
 * Function : NlmRangeMgr__GetRangeEncoding
 *
 * Parameters:
 * NlmRangeMgr *pRangeMgr	= Pointer to the Range Manager
 * NlmRangeDb *rangeDb	= Pointer to the existing Database having that range-field,
 *							  the encoding of which is to be fetched
 * nlm_u32    id			= The id that was used in NlmRange when the range was created
 * NlmReasonCode *o_reason	= Location to save the reason code
 *
 * Summary:
 * This function returns the encoded bitmap for the input range. 
 * This encoded bit map will be searched in HW when the field with a fixed value is searched
 * as a part of the search key.
 ****************************************/
NlmRangeEncoded* NlmRangeMgr__GetRangeEncoding(
	NlmRangeMgr 		*self,
	NlmRangeDb 			*rangeDb,
	nlm_u32				 id,
	NlmReasonCode		*o_reason
	)
{
	NlmRangeEncoded *rangeEncoded_p = NULL; 

#if defined NLM_MT_OLD || defined NLM_MT

	NlmCmMt__SpinLock( &self->m_spinLock );
#endif

	rangeEncoded_p = NlmRangeMgr__pvt_GetRangeEncoding(self,
													rangeDb,
													id,
													o_reason);
#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock( &self->m_spinLock );
#endif


	return rangeEncoded_p;

}

static NlmRangeEncoded* 
NlmRangeMgr__pvt_GetRangeEncoding(
	NlmRangeMgr 		*self,
	NlmRangeDb 			*rangeDb,
	nlm_u32				 id,
	NlmReasonCode		*o_reason
	)
{
	/* Check the inputs. */
	NlmRangeNode* node;
	if(NULL == self)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE_MGR;
		return NULL;
	}
	if(NULL == rangeDb)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATABASE;
		return NULL;
    }

	if(NULL == (node= NlmRangeMgr__pvt_SearchRangeList(
									&(rangeDb->m_entries[id % NLM_RANGE_HT_DEPTH]),
									id)))

	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE;
		return NULL;
	}

	if(node->m_range->m_encoded_p == NULL)
	{
		nlm_32 ret_val;
		NlmRangeEncoded* encoded_head = NULL;
		subRangeList srl;
		nlm_u32 chunkws[] = {2,2,2,2,2,2,2,2};		

        if (rangeDb->m_use_mcor && ((ret_val = NlmRangeMgr__pvt_IsTopRange(rangeDb, node)) >= 0))
        {
            /* If the range is within the Top 8 Ranges do MCOR encoding */
            encoded_head = (NlmRangeEncoded *)
                NlmCmAllocator__malloc(self->m_alloc_p, sizeof(NlmRangeEncoded));
            encoded_head->m_num_entries = 1;
            encoded_head->m_entries_p =
                NlmCmAllocator__calloc(self->m_alloc_p, 1,  sizeof(NlmRangeDMFormat));
            encoded_head->m_entries_p->m_data = 0xFFFFFF00 | 1 << ret_val;
            encoded_head->m_entries_p->m_mask = 0xFFFFFF00 | (~(1 << ret_val));
        }
        else
        {
            if(rangeDb->m_range_code == NLM_RANGE_3B_ENCODING)/* If 3b encoding perform Fence Encoding*/
                encoded_head = NlmRangeMgr__pvt_FindSubRanges(&srl, 16,node->m_range, 8, chunkws, self->m_alloc_p);
            else if(rangeDb->m_range_code == NLM_RANGE_2B_ENCODING)/* If 2b encoding perform Prefix Encoding */
                encoded_head = NlmRangeMgr__pvt_GeneratePrefixEncodings(node->m_range, self->m_alloc_p);
        }
        
        /* Do conditioning based on Db Attrs */
        NlmRangeMgr__pvt_DoConditioningBasedOnDbAttrs(encoded_head, rangeDb);
		node->m_range->m_encoded_p = encoded_head;
    }

    if(o_reason)
			*o_reason = NLMRSC_REASON_OK;
    return (node->m_range->m_encoded_p);

}

/****************************************
 * Function : NlmRangeMgr__ConfigRangeMatching
 *
 * Parameters:
 * NlmRangeMgr *self				= Pointer to the Range Manager
 * nlm_u8 ltrNum				= ltrnum to be configured
 * NlmRangeSrchAttrs *srchAttrs	= Holds the Range Srch Attributes
 * NlmReasonCode *o_reason				= Location to save the reason code
 *
 * Summary:
 * This API configures the specified range matching ltr registers with the attributes
 * specified in srchAttrs
 * Note: If a particular range is not being used for the specified LTR then rangeInsertStartBytes
 * for that range for the all the keys should be made as NLMDEV_RANGE_DO_NOT_INSERT
 * Note: If Range Manager is used as standalone(without NETL DEVMGR) then programming of
 *        MISC LTR with BMR information should be done after this API and also
 *        the range fields of this register initialized by this API should be re-written
 *        along with the BMR fields
 ****************************************/
NlmErrNum_t NlmRangeMgr__ConfigRangeMatching(
    NlmRangeMgr *self,
    nlm_u8 ltrNum,
    NlmRangeSrchAttrs *srchAttrs,
    NlmReasonCode *o_reason
    )
{
	NlmErrNum_t errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT

	NlmCmMt__SpinLock( &self->m_spinLock );
#endif

	errNum = NlmRangeMgr__pvt_ConfigRangeMatching(self,
												ltrNum,
												srchAttrs,
												o_reason);

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinUnlock( &self->m_spinLock );
#endif

	return errNum;

}

static NlmErrNum_t 
NlmRangeMgr__pvt_ConfigRangeMatching(
    NlmRangeMgr *self,
    nlm_u8 ltrNum,
    NlmRangeSrchAttrs *srchAttrs,
    NlmReasonCode *o_reason
    )
{
    NlmErrNum_t catch_err;
    if(self == NULL)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE_MGR;
		return NLMERR_NULL_PTR;
    }
    if(srchAttrs == NULL)
    {
        if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE_ATTRIBUTES;
		return NLMERR_NULL_PTR;
    }    
    if((catch_err = self->m_vtbl.m_initRangeLtrFnPtr(
                                            self,
                                            ltrNum,
                                            srchAttrs,
                                            o_reason))!=NLMERR_OK)
        return catch_err;

    if(o_reason)
        *o_reason = NLMRSC_REASON_OK;
    return NLMERR_OK;
}

/****************************************
 * Function : NlmRangeMgr__GetDbStats
 *
 * Parameters:
 * NlmRangeMgr *self	= Pointer to the Range Manager
 * NlmRangeDb *rangeDb	= Pointer to the Database to get its Statistics
 * NlmReasonCode *o_reason	= Location to save the reason code
 *
 * Summary:
 * Some interesting stats - Number of ranges, Number of mapped entries, list of top 8 popular
 * ranges. Statistics of a specific range are present within each NlmRangeMgr_range structure,
 * and can be looked upon any time.
 ****************************************/
NlmRangeDbStats* NlmRangeMgr__GetDbStats(
	NlmRangeMgr         *self,
	NlmRangeDb          *rangeDb,
	NlmReasonCode		*o_reason
	)
{
	NlmRangeDbStats* rangeDbStats_p = NULL;

#if defined NLM_MT_OLD || defined NLM_MT

	NlmCmMt__SpinLock( &self->m_spinLock );
#endif

	rangeDbStats_p = NlmRangeMgr__pvt_GetDbStats(self,
									rangeDb,
									o_reason);

#if defined NLM_MT_OLD || defined NLM_MT

		NlmCmMt__SpinUnlock( &self->m_spinLock );
#endif

	return rangeDbStats_p;
}

static NlmRangeDbStats* 
NlmRangeMgr__pvt_GetDbStats(
	NlmRangeMgr         *self,
	NlmRangeDb          *rangeDb,
	NlmReasonCode		*o_reason
	)
{
	nlm_u32 encodedCount = 0;
	nlm_u32 iter;
	NlmRangeNode* rangeNode_p;
	/* Check the inputs. */
	if(NULL == self)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE_MGR;
		return NULL;
	}
	if(NULL == rangeDb)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATABASE;
		return NULL;
	}

	/* ------- Iterating through all the ranges in the DB --------- */
	for(iter = 0; iter < NLM_RANGE_HT_DEPTH; iter++)
	{
		rangeNode_p = rangeDb->m_entries[iter].m_firstRange;

		while(rangeNode_p)
		{
			/* Getting the sum of all the num_of_entries created */
			if( NULL != rangeNode_p->m_range->m_encoded_p)
			{
				encodedCount += rangeNode_p->m_range->m_encoded_p->m_num_entries;
			}
			rangeNode_p = rangeNode_p->m_next;

		}
	}

	/* ------- -------------------------------- --------- */

	rangeDb->m_stats.m_numOfRanges = rangeDb->m_range_count;
	rangeDb->m_stats.m_numOfMappedEntries = encodedCount;

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
    return (&rangeDb->m_stats);

}


/****************************************
 * Function : NlmRangeMgr__CreateDb
 *
 * Parameters:
 * NlmRangeMgr *pRangeMgr	= Pointer to the Range Manager
 * nlm_u8 id				= User assigned id for this database
 * NlmRangeDbAttrSet  *rmDbAttributes	= Holds the Configuration Information for the Database
 * NlmReasonCode *o_reason = Location to save the reason code
 *
 * Summary:
 * Adds a new Database with specified attributes to the Range Manager. 
 * The Database can have following attribute values
 *      m_num_of_bits: Specifies number of bits of available for the encoding of each range 
                       added to the database; Supported values: 16, 24 and 32
 *      m_valid_bits: Specifies number of bits of range field which are valid to be encoded 
 *                    for each range added to this database; Supported Values 4, 8, 12 and 16 
 *     m_encodingType: Encoding type to be used with Database. 
 * newDb is the pointer to hold the pointer to the Database which is newly created by this module.
 * Memory for this Database is allocated within the function call, this memory is freed when DestroyDb()
 * is called.
 ****************************************/
NlmRangeDb* NlmRangeMgr__CreateDb(
	NlmRangeMgr		*self,
	nlm_u8			id,
    NlmRangeDbAttrSet   *dbAttributes,
	NlmReasonCode	*o_reason
	)
{
	NlmRangeDb* rangeDb_p = NULL;

#if defined NLM_MT_OLD || defined NLM_MT

	NlmCmMt__SpinLock( &self->m_spinLock );
#endif

	rangeDb_p = NlmRangeMgr__pvt_CreateDb(self,
								id,
								dbAttributes,
								o_reason);

#if defined NLM_MT_OLD || defined NLM_MT

	NlmCmMt__SpinUnlock( &self->m_spinLock );
#endif

	return rangeDb_p;
}

static NlmRangeDb* 
NlmRangeMgr__pvt_CreateDb(
	NlmRangeMgr		*self,
	nlm_u8			id,
    NlmRangeDbAttrSet   *dbAttributes,
	NlmReasonCode	*o_reason
	)
{
	NlmRangeDb* newDb = NULL;
    NlmRangeDbNode* node;
    NlmReasonCode dummyReasonCode;

    if(!o_reason)
        o_reason = &dummyReasonCode;

	/* return NULL if there is no Range manager. */
	if(NULL == self)
	{
        	*o_reason = NLMRSC_INVALID_RANGE_MGR;
		return NULL;
	}
    if(NULL == dbAttributes)
	{
		*o_reason = NLMRSC_INVALID_RANGE_DB_ATTR;
		return NULL;
	}
	
	if( 16 != dbAttributes->m_valid_bits &&
                12 != dbAttributes->m_valid_bits &&
                8 != dbAttributes->m_valid_bits &&
        	 4 != dbAttributes->m_valid_bits)
	{
		*o_reason = NLMRSC_INVALID_RANGE_DB_ATTR;
		return NULL;
	}

	if( (dbAttributes->m_num_bits != 8) && (dbAttributes->m_num_bits != 16) 
                && (dbAttributes->m_num_bits != 24) && (dbAttributes->m_num_bits != 32) )
	{
		*o_reason = NLMRSC_INVALID_RANGE_DB_ATTR;
		return NULL;
	}
       switch(dbAttributes->m_num_bits)
        {
            case 8:
                if(dbAttributes->m_encodingType != NLM_RANGE_NO_ENCODING)
                 {
                    *o_reason = NLMRSC_INVALID_RANGE_DB_ATTR;
                    return NULL;
                 }
                break;
            case 16:
                 if(dbAttributes->m_encodingType != NLM_RANGE_2B_ENCODING)
                 {
                    *o_reason = NLMRSC_INVALID_RANGE_DB_ATTR;
                    return NULL;
                 }
                break;
            case 24:
                 if((dbAttributes->m_encodingType != NLM_RANGE_2B_ENCODING)
                    && (dbAttributes->m_encodingType != NLM_RANGE_3B_ENCODING))
                {
                    *o_reason = NLMRSC_INVALID_RANGE_DB_ATTR;
                    return NULL;
                 }
                break;
            case 32:
                if(dbAttributes->m_encodingType != NLM_RANGE_3B_ENCODING)
                {
                    *o_reason = NLMRSC_INVALID_RANGE_DB_ATTR;
                    return NULL;
                 }
                break;
            default:
                *o_reason = NLMRSC_INVALID_RANGE_DB_ATTR;
                return NULL;
                break;
                
        }

	/* Checking for Duplicate Database-id */
    node = self->m_rangeDb_list.m_firstDb;
	while(node)
	{
		/* If you have found the entry to be deleted */
		if(node->m_database->m_id == id)
		{
			if(o_reason)
				*o_reason = NLMRSC_DUPLICATE_DATABASE_ID;
			return NULL;
		}

		node = node->m_next;
	}

	/* Allocate memory for the New Database */
	newDb = NlmCmAllocator__calloc(self->m_alloc_p, 1, sizeof(NlmRangeDb));
    NlmCmDemand((newDb != NULL), "Out of memory.\n");
	NlmRangeMgr__pvt_AddtoDbList(&self->m_rangeDb_list, newDb, self->m_alloc_p);
	self->m_rangeDb_count++;

	/* Initialize DB Structure fields*/    
	newDb->m_rangeMgr_p = self;
	newDb->m_alloc_p	= self->m_alloc_p;
	newDb->m_id			= id;
    newDb->m_num_bits = dbAttributes->m_num_bits;	
    newDb->m_valid_bits = dbAttributes->m_valid_bits;
	newDb->m_use_mcor	= 0;    

    newDb->m_range_code = dbAttributes->m_encodingType;
    newDb->m_range_count= 0;

    newDb->m_pvtdata = NlmCmAllocator__calloc(self->m_alloc_p,1,
                       sizeof (NlmRangeMgr__pvtdata));

	if (!newDb->m_pvtdata)
    {
        NlmCmAllocator__free(self->m_alloc_p, newDb );
        NlmCmDemand((newDb != NULL), "Out of memory.\n");
    }
	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
    return newDb;

}

/****************************************
 * Function : NlmRangeMgr__DestroyDb
 *
 * Parameters:
 * NlmRangeMgr *pRangeMgr	= Pointer to the Range Manager
 * NlmRangeDb *rangeDb	= Pointer to the Database which is to be deleted
 * NlmReasonCode *o_reason	= Location to save the reason code
 *
 * Summary:
 * Destroys the already created database and de-allocates the memory allocated for it.
 * Destroying a Database which does not exist will return error. Destroying a Database.
 ****************************************/
NlmErrNum_t NlmRangeMgr__DestroyDb(
	NlmRangeMgr     *self,
	NlmRangeDb      *rangeDb,
	NlmReasonCode	*o_reason
	)
{
	NlmErrNum_t errNum = NLMERR_OK;

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinLock( &self->m_spinLock );
#endif

	errNum = NlmRangeMgr__pvt_DestroyDb(self,
										rangeDb,
										o_reason);
#if defined NLM_MT_OLD || defined NLM_MT
		NlmCmMt__SpinUnlock( &self->m_spinLock );
#endif

	return errNum;
}

static NlmErrNum_t 
NlmRangeMgr__pvt_DestroyDb(
	NlmRangeMgr     *self,
	NlmRangeDb      *rangeDb,
	NlmReasonCode	*o_reason
	)
{
    NlmRangeMgr__pvtdata *pvtdata;
	nlm_u32 iter;
	NlmRangeNode *rangeNode_p, *tempNode_p;
	NlmErrNum_t catchErr;

	/* Check the inputs. */
	if(NULL == self)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE_MGR;
		return NLMERR_NULL_PTR;
	}
	if(NULL == rangeDb)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATABASE;
		return NLMERR_NULL_PTR;
	}

	/* ------- Deleting all ranges  from the DB --------- */
	for(iter = 0; iter < NLM_RANGE_HT_DEPTH; iter++)
	{
		rangeNode_p = rangeDb->m_entries[iter].m_firstRange;

		while(rangeNode_p)
		{
			tempNode_p = rangeNode_p->m_next;
			if(NLMERR_OK != (catchErr = NlmRangeMgr__DeleteRange
				(
				self,
				rangeDb,
				rangeNode_p->m_range->m_id,
				o_reason
				)))

				return catchErr;

			rangeNode_p = tempNode_p;

		}
	}

	/* ------- -------------------------------- --------- */
	/* Delete the node from the Doubly Linked List */
	if(NLMERR_OK != NlmRangeMgr__pvt_DelfromDbList(&self->m_rangeDb_list,
                                                    rangeDb,
                                                    self->m_alloc_p
							   						))
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_DATABASE;
		return NLMERR_FAIL;
	}
	self->m_rangeDb_count--;

    pvtdata = (NlmRangeMgr__pvtdata *)(rangeDb->m_pvtdata);
    if (pvtdata)
    {
         NlmRangeMgr__pvt_FreePvtData(self, pvtdata);
         NlmCmAllocator__free(self->m_alloc_p, pvtdata);
    }
    NlmCmAllocator__free(self->m_alloc_p, rangeDb);

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;
}

/****************************************
 * Function : NlmRangeMgr__Init
 *
 * Parameters:
 * NlmCmAllocator * alloc_p	= Memory Allocator provided by the user  
 * void *devMgr				= Pointer to the Device manager object
 * NlmDevType devType		= Type of the Device that Range Manager is working upon
 * NlmRangeEncodingType* encodingType = array of 4, Encoding type to be used with all range type.
 * NlmReasonCode *o_reason	= Location to save the reason code
 *
 * Summary:
 * Initializes the Range Manager inside the control plane software. The Range Manager needs
 * to be given a target device it should work with. The Range Manager is designed with any 
 * generic Processor architectures. Return value of NULL indicates  failure while an initialized 
 * range manager is returned on sucess, and the pointer has to be used for further function calls.
 ****************************************/
NlmRangeMgr* NlmRangeMgr__Init(
								NlmCmAllocator		*alloc_p,
								void				*devMgr,
								NlmDevType			devType,	
								NlmRangeEncodingType* encodingType,
								NlmReasonCode		*o_reason
                                )
{
	NlmRangeMgr *self = NULL;
	NlmErrNum_t catchErr;
    NlmRangeType rangeType;
    nlm_u32 rangeCodeValue =  NLM_RANGE_3B_ENCODING_CODE_VALUE;

	if(alloc_p == NULL)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_MEMALLOC_PTR;
		return NULL;
	}

    self = NlmCmAllocator__malloc(alloc_p, sizeof(NlmRangeMgr));
    NlmCmDemand((self != NULL), "Out of memory.\n");

	self->m_devType = devType;
	self->m_alloc_p = alloc_p;
	self->m_rangeDb_list.m_firstDb = NULL;
    self->m_rangeDb_count = 0;
	self->m_devMgr_p = devMgr;

	switch(devType)
	{
	    case NLM_DEVTYPE_2:
	    case NLM_DEVTYPE_2_S:
		    if(NLMERR_OK != (catchErr = NlmRangeMgr2__pvt_Init
				    (
				    self,
				    o_reason
				    )))
            {
                NlmCmAllocator__free(alloc_p, self);
		        return NULL;
            }
		    break;

	    default:
			    if(o_reason)
			        *o_reason = NLMRSC_INVALID_DEVICE_TYPE;
                NlmCmAllocator__free(alloc_p, self);
			    return NULL;
	}
   
    /* Initialize the Range Code Registers of the Range Block  */
    for(rangeType = NLM_RANGE_TYPE_A; rangeType <= NLM_RANGE_TYPE_D;  rangeType++)
    {
        /* Write the code registers with the proper encoding value base on 
        2b or 3b encoding. */
        if(encodingType[rangeType] == NLM_RANGE_2B_ENCODING)
            rangeCodeValue =  NLM_RANGE_2B_ENCODING_CODE_VALUE;
        else
            rangeCodeValue =  NLM_RANGE_3B_ENCODING_CODE_VALUE;
        if(NLMERR_OK != (catchErr = self->m_vtbl.m_initCodeRegsFnPtr
            (
                self,
                rangeType,
                rangeCodeValue,
				o_reason
              )))
        {
            NlmCmAllocator__free(alloc_p, self);
            return NULL;
        }
    }

#if defined NLM_MT_OLD || defined NLM_MT

    {
        nlm_32 ret;

	ret = NlmCmMt__SpinInit(&self->m_spinLock,
				"NlmRm_Kbp_SpinLock",
				NlmCmMtFlag);
        if(ret != 0)
        {
            *o_reason = NLMRSC_MT_SPINLOCK_INIT_FAILED;
            return NULL;
        }
    }
#endif

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;

    return self;
}

/****************************************
 * Function : NlmRangeMgr__Destroy
 *
 * Parameters:
 * NlmRangeManager *pRangeMgr	= Pointer to Range Manager previously created
 * NlmReasonCode *o_reason		= Location to save the reason code
 *
 * Summary:
 * This function destroys all the remaining Databases and finally frees the memory allocated
 * to the Range Manager.
 ****************************************/
NlmErrNum_t NlmRangeMgr__Destroy(
							NlmRangeMgr 	 *self,
							NlmReasonCode	*o_reason
							)
{
    NlmRangeDb     *rangeDb;
	NlmErrNum_t		catchErr;
	NlmCmAllocator *alloc_p;

	if(NULL == self)
	{
		if(o_reason)
			*o_reason = NLMRSC_INVALID_RANGE_MGR;
		return NLMERR_NULL_PTR;
	}

	while(self->m_rangeDb_count)
	{
        rangeDb = self->m_rangeDb_list.m_firstDb->m_database;

		if(NLMERR_OK != (catchErr = NlmRangeMgr__DestroyDb	(
											self,
											rangeDb,
											o_reason
											)))
		return catchErr;
	}

#if defined NLM_MT_OLD || defined NLM_MT
	NlmCmMt__SpinDestroy(&self->m_spinLock,
				"NlmRm_Kbp_SpinLock");
#endif

	alloc_p = self->m_alloc_p;
	NlmCmAllocator__free(alloc_p, self);

	if(o_reason)
		*o_reason = NLMRSC_REASON_OK;
	return NLMERR_OK;
}

