/* $Id: sand_occupation_bitmap.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/
#ifndef SOC_SAND_OCCUPATION_BITMAP_H_INCLUDED
/* { */
#define SOC_SAND_OCCUPATION_BITMAP_H_INCLUDED



/*************
* INCLUDES  *
*************/
/* { */
#include <shared/swstate/sw_state.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>

/* } */

/*************
* DEFINES   *
*************/
/* { */
/*
 * Maximal number of bit map array to assign at init.
 * See soc_sand_occ_bm_init(), soc_sand_occ_bm_create()
 */
#define MAX_NOF_DSS (500 * SOC_DPP_DEFS_GET(unit, nof_cores))
/* } */


/*************
* MACROS    *
*************/
/* { */

/* } */

/*************
* TYPE DEFS *
*************/
/* { */


/* $Id: sand_occupation_bitmap.h,v 1.4 Broadcom SDK $
 * type definition for the bitmap
 */

/*
 * the ADT of bit map , use this type to manipulate the bit map.
 */

typedef struct
{
  /*
   *  the size of the occupation bitmap
   */
  uint32 size;
  /*
   *  initial value to fill the bitmap with.
   */
  uint8  init_val;
  /*
   *  support caching
   */
  uint8  support_cache;

}SOC_SAND_OCC_BM_INIT_INFO;


typedef struct 
{
  /*
   *  array to hold offsets in the levels_buffer.
   */
  PARSER_HINT_ARR uint32 *levels;
  /*
   *  array to hold the size (in bits of every level)
   */
  PARSER_HINT_ARR uint32 *levels_size;
  /*
   * number of levels in the bitmap (length of levels array)
   */
  uint32  nof_levels;
  /*
   * The size of the bitmap.
   */
  uint32  size;
  /*
   * init value
   */
  uint8  init_val;
   /*
    *  support caching
    */
  uint8  support_cache;
  uint8  cache_enabled;
  /*
   *  array to hold offsets in the levels_cache_buffer.
   */
  PARSER_HINT_ARR uint32 *levels_cache;

  /*
   *  the allocated buffer of 'levels' and 'levels_cache'
   */
  SW_STATE_BUFF *levels_buffer;
  SW_STATE_BUFF *levels_cache_buffer;

  /*
   *  the size of levels_buffer/levels_cache_buffer
   */
  uint32 buffer_size;

} SOC_SAND_OCC_BM_T;


/*
 * Replace: typedef SOC_SAND_OCC_BM_T*  SOC_SAND_OCC_BM_PTR;
 * because the new software state does not use pointers, only handles.
 * So now, SOC_SAND_OCC_BM_PTR is just a handle to the 'bit map'
 * structure (actually, index into 'dss_array' {of pointers})
 *
 * Note that the name is kept as is to minimize changes in current code.
 */
typedef uint32  SOC_SAND_OCC_BM_PTR ;

/*
 * Control Structure for all created bitmaps. Each bitmap is pointed
 * by dss_array. See soc_sand_occ_bm_init()
 */
typedef struct soc_sand_sw_state_occ_bitmap_s
{
                      uint32              max_nof_dss;
                      uint32              in_use;
  PARSER_HINT_ARR_PTR SOC_SAND_OCC_BM_T   **dss_array;
  PARSER_HINT_ARR     SHR_BITDCL          *occupied_dss;
} soc_sand_sw_state_occ_bitmap_t;

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
*     soc_sand_occ_bm_init
* TYPE:
*   PROC
* DATE:
*   May 12 2015
* FUNCTION:
*     Initialize control structure for all bitmap instances expected.
* INPUT:
*   SOC_SAND_IN  int unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint32 max_nof_dss -
*     Maximal number of bitmaps which can be sustained simultaneously.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_init(
    SOC_SAND_IN       int                          unit,
    SOC_SAND_IN       uint32                       max_nof_dss
  ) ;

/*********************************************************************
* NAME:
*   soc_sand_occ_bm_get_ptr_from_handle
* TYPE:
*   PROC
* DATE:
*   May 18 2015
* FUNCTION:
*   Get bitmap structure pointer from handle.
* INPUT:
*   SOC_SAND_IN  int                               unit -
*     Identifier of the device to access.
*   SOC_SAND_OUT  SOC_SAND_OCC_BM_PTR              bit_map -
*     Handle to bitmap to get pointer to.
* REMARKS:
*   This procedure is exceptional and is added here only for Arad (and
*   on Arad, for pat_tree. See soc_sand_pat_tree_create()).
*   Range of legal handles: 1 -> max_nof_dss
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_get_ptr_from_handle(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR          bit_map,
    SOC_SAND_OUT SOC_SAND_OCC_BM_T            **bit_map_ptr_ptr
  ) ;

/*********************************************************************
* NAME:
*     soc_sand_occ_bm_create
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Creates a new bitmap instance.
* INPUT:
*   SOC_SAND_IN  int                               unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  SOC_SAND_OCC_BM_INIT_INFO          info -
*     information needed for the bitmap creation
*
*  SOC_SAND_OUT  SOC_SAND_OCC_BM_PTR              *bit_map -
*     The created bitmap.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_create(
    SOC_SAND_IN       int                         unit,
    SOC_SAND_IN       SOC_SAND_OCC_BM_INIT_INFO  *info,
    SOC_SAND_OUT      SOC_SAND_OCC_BM_PTR        *bit_map
  );

/*********************************************************************
* NAME:
*     soc_sand_occ_bm_destroy
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     free the memory allocated by the bitmap instance.
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT  SOC_SAND_OCC_BM_PTR bit_map -
*     The bitmap to destroy.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_destroy(
    SOC_SAND_IN        int                   unit,
    SOC_SAND_OUT       SOC_SAND_OCC_BM_PTR   bit_map
  );

/*********************************************************************
* NAME:
*     soc_sand_occ_bm_clear
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Free all occupied entries, without freeing the memory
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT  SOC_SAND_OCC_BM_PTR bit_map -
*     The bitmap to free.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_clear(
    SOC_SAND_IN        int                   unit,
    SOC_SAND_OUT       SOC_SAND_OCC_BM_PTR   bit_map
  );

/*********************************************************************
* NAME:
*     soc_sand_occ_bm_get_next
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Get the first zero bit on the bitmap (starting from bit 0 )
* INPUT:
*   SOC_SAND_IN  int                unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  SOC_SAND_OCC_BM_PTR bit_map -
*     The bitmap to perform the get operation at.
*   SOC_SAND_OUT  uint32           *place -
*     the next place (starting from zero) with bit = 0.
*   SOC_SAND_OUT  uint8            *found -
*     whether a bit with zero was found in the bitmap
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_get_next(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR   bit_map,
    SOC_SAND_OUT uint32               *place,
    SOC_SAND_OUT uint8                *found
  );

/*********************************************************************
* NAME:
*     soc_sand_occ_bm_get_next_in_range
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Get the first/last bit in a range with a given value.
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_OCC_BM_PTR bit_map -
*     The bitmap to perform the get operation at.
*   SOC_SAND_IN   uint32             start -
*     start bit of the range
*   SOC_SAND_IN   uint32             end -
*     end bit of the range
*   SOC_SAND_IN   uint8              forward -
*     TRUE: look forward (from first bit),
*     FALSE: look backward (from last bit)
*   SOC_SAND_OUT  uint32            *place -
*     found place relevant only if found = TRUE.
*   SOC_SAND_OUT  uint8             *found-
*     whether a bit with zero was found in the bitmap in the given range
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_get_next_in_range(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR       bit_map,
    SOC_SAND_IN  uint32                    start,
    SOC_SAND_IN  uint32                    end,
    SOC_SAND_IN  uint8                     forward,
    SOC_SAND_OUT uint32                   *place,
    SOC_SAND_OUT uint8                    *found
  );

/*********************************************************************
* NAME:
*     soc_sand_occ_bm_alloc_next
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     find the first free(zero) bit and set to 1
* INPUT:
*   SOC_SAND_IN  int                unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  SOC_SAND_OCC_BM_PTR bit_map -
*     The bitmap to perform the get operation at.
*   SOC_SAND_OUT  uint32           *place -
*     the allocated place.
*   SOC_SAND_OUT  uint8            *found-
*     whether a bit with zero was found in the bitmap
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_alloc_next(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR         bit_map,
    SOC_SAND_OUT  uint32                    *place,
    SOC_SAND_OUT  uint8                     *found
  );

/*********************************************************************
* NAME:
*     soc_sand_occ_bm_occup_status_set
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Set the occupation status a of the given bit.
* INPUT:
*   SOC_SAND_IN  int                   unit -
*     Identifier of the device to access.
*  SOC_SAND_INOUT  SOC_SAND_OCC_BM_PTR bit_map -
*     The bitmap to perform the set operation at.
*  SOC_SAND_IN  uint32                 place -
*     bit to start the setting from
*  SOC_SAND_IN  uint8                  occupied -
*     the status (occupied/unoccupied) to set for the give bits.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_occup_status_set(
    SOC_SAND_IN     int                     unit,
    SOC_SAND_INOUT  SOC_SAND_OCC_BM_PTR     bit_map,
    SOC_SAND_IN     uint32                  place,
    SOC_SAND_IN     uint8                   occupied
  );

/*********************************************************************
* NAME:
*     soc_sand_occ_bm_is_occupied
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Get the occupation status a of the given of bit.
* INPUT:
*   SOC_SAND_IN  int                unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  SOC_SAND_OCC_BM_PTR bit_map -
*     The bitmap to perform the get operation at.
*  SOC_SAND_IN  uint32              place -
*     bit to get the status (occupied/unoccupied) for.
*  SOC_SAND_OUT  uint8             *occupied -
*     the status (occupied/unoccupied) of the given bit
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_is_occupied(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR        bit_map,
    SOC_SAND_IN  uint32                     place,
    SOC_SAND_OUT uint8                     *occupied
  );

uint32
  soc_sand_occ_bm_cache_set(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR        bit_map,
    SOC_SAND_IN  uint8                      cached
  );

uint32
  soc_sand_occ_bm_cache_commit(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR        bit_map,
    SOC_SAND_IN  uint32                     flags
  );

uint32
  soc_sand_occ_bm_cache_rollback(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR        bit_map,
    SOC_SAND_IN  uint32                     flags
  );

/*********************************************************************
* NAME:
*     soc_sand_occ_bm_get_size_for_save
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     returns the size of the buffer needed to return the bitmap as buffer.
*     in order to be loaded later
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_OCC_BM_PTR bit_map -
*     The bitmap to get the size for.
*   SOC_SAND_OUT  uint32            *size -
*     the size of the buffer.
* REMARKS:
*   None.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_get_size_for_save(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR          bit_map,
    SOC_SAND_OUT  uint32                     *size
  );

/*********************************************************************
* NAME:
*     soc_sand_occ_bm_save
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     saves the given bitmap in the given buffer
* INPUT:
*   SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  SOC_SAND_OCC_BM_PTR bit_map -
*     The bitmap to save.
*   SOC_SAND_OUT  uint8             *buffer -
*     buffer to include the hast table
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by soc_sand_occ_bm_get_size_for_save.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_save(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR    bit_map,
    SOC_SAND_OUT uint8                 *buffer,
    SOC_SAND_IN  uint32                 buffer_size_bytes,
    SOC_SAND_OUT uint32                *actual_size_bytes
  );


/*********************************************************************
* NAME:
*     soc_sand_occ_bm_load
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Load from the given buffer the bitmap saved in this buffer.
* INPUT:
*   SOC_SAND_IN  int                  unit -
*     Identifier of the device to access.
*   SOC_SAND_IN  uint8               *buffer -
*     buffer includes the hast table
*   SOC_SAND_OUT  SOC_SAND_OCC_BM_PTR bit_map -
*     The bitmap to load.
* REMARKS:
*   - the size of the buffer has to be at least as the value returned
*     by soc_sand_occ_bm_get_size_for_save.
*   - this function will update buffer to point
*     to next place, for further loads.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_load(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint8                           **buffer,
    SOC_SAND_OUT  SOC_SAND_OCC_BM_PTR             *bit_map
  );
void
  soc_sand_SAND_OCC_BM_INIT_INFO_clear(
    SOC_SAND_INOUT SOC_SAND_OCC_BM_INIT_INFO *info
  );

uint32
  soc_sand_occ_bm_get_buffer_size(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR          bit_map,
    SOC_SAND_OUT  uint32                      *buffer_size_p
  ) ;
uint32
  soc_sand_occ_bm_get_support_cache(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR          bit_map,
    SOC_SAND_OUT  uint8                       *support_cache_p
  ) ;
/*
 * Get handle to occ bitmap which will be considered illegal
 * by all occ bitmap utilities.
 */
uint32
   soc_sand_occ_bm_get_illegal_bitmap_handle(void) ;
/*********************************************************************
* NAME:
*   soc_sand_occ_is_bitmap_active
* TYPE:
*   PROC
* DATE:
*   May 13 2015
* FUNCTION:
*   Get indication on whether specified bitmap is currently in use.
* INPUT:
*  SOC_SAND_IN  int unit -
*    Identifier of the device to access.
*  SOC_SAND_IN  SOC_SAND_OCC_BM_PTR bit_map -
*    Handle of the bitmap to perform the check on.
*  SOC_SAND_OUT  uint8 *in_use -
*    Return a non-zero value if bit map is in use.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_is_bitmap_active(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  SOC_SAND_OCC_BM_PTR  bit_map,
    SOC_SAND_OUT uint8                *in_use
  ) ;

/*********************************************************************
* NAME:
*     soc_sand_occ_bm_print
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     print the bitmap content.
* INPUT:
*  SOC_SAND_IN  int    unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  SOC_SAND_OCC_BM_PTR bit_map -
*     The bitmap to print.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  soc_sand_occ_bm_print(
    SOC_SAND_IN int                 unit,
    SOC_SAND_IN SOC_SAND_OCC_BM_PTR bit_map
  );

#if SOC_SAND_DEBUG
/* { */
/*********************************************************************
* NAME:
*     soc_sand_occ_bm_tests
* TYPE:
*   PROC
* DATE:
*   Mar  6 2008
* FUNCTION:
*     Tests the bit map module
*
*INPUT:
*   SOC_SAND_IN  int    unit -
*     Identifier of the device to access.
*  SOC_SAND_DIRECT:
*    SOC_SAND_IN uint32 silent -
*    Indicator.
*    1 - Do not print debuging info.
*    0 - Print various debuging info.
*  SOC_SAND_INDIRECT:
*    None.
*OUTPUT:
*  SOC_SAND_DIRECT:
*    uint32 -
*    Indicator.
*    1 - Test pass.
*    0 - Test fail.
*  SOC_SAND_INDIRECT:
*    NON
*REMARKS:* SW License Agreement: Dune Networks (c). CONFIDENTIAL PROPRIETARY INFORMATION.
* Any use of this Software is subject to Software License Agreement
* included in the Driver User Manual of this device.
* Any use of this Software constitutes an agreement to the terms
* of the above Software License Agreement.
********************************************************************/
uint32
  soc_sand_occ_bm_tests(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint8 silent
);
/* } */
#endif /* SOC_SAND_DEBUG */

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } SOC_SAND_OCCUPATION_BITMAP_H_INCLUDED*/
#endif

