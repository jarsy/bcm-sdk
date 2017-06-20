/**
 * \file diag_sand_prt.h
 *
 * Set of macros and routines to print beautified tabular output. Main purpose to provide
 * standard output format for all tabular data in bcm shell
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef DIAG_SAND_PRT_H
#define DIAG_SAND_PRT_H

#if !defined(__KERNEL__)
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include <shared/utilex/utilex_rhlist.h>
#include <shared/utilex/utilex_str.h>
#include <shared/shrextend/shrextend_debug.h>
#include <appl/diag/sand/diag_sand_utils.h>


#define PRT_COLUMN_WIDTH     (RHNAME_MAX_SIZE + 1)
#define PRT_COLUMN_WIDTH_BIG (16 * RHNAME_MAX_SIZE + 1)
#define PRT_COLUMN_MAX_NUM  24
#define PRT_TITLE_ID       PRT_COLUMN_MAX_NUM
#define PRT_MAX_SHIFT_NUM   5

typedef struct
{
    uint32 width;
    char format[RHKEYWORD_MAX_SIZE];
    char *next_ptr;
} prt_format_t;

typedef enum
{
    PRT_XML_NONE,
    PRT_XML_CHILD,
    PRT_XML_ATTRIBUTE
} PRT_XML_TYPE;

typedef enum
{
    PRT_FLEX_NONE,
    PRT_FLEX_ASCII,
    PRT_FLEX_BINARY
} PRT_FLEX_TYPE;

typedef struct
{
    /**
     * Column name - used in table header or as XML node name
     * Established only by COLUMN_ADD
     */
    char name[PRT_COLUMN_WIDTH];
    /**
     * Flag marking whether string should be split as binary, ascii or none
     * Established only by COLUMN_ADD
     */
    PRT_FLEX_TYPE flex_type;
    /**
     * Maximum size for strings in this column
     * Established only by COLUMN_ADD
     */
    int max_width;
    /**
     * Cell offset in the row
     * Established only by COLUMN_ADD
     */
    int cell_offset;
    void *node[PRT_MAX_SHIFT_NUM];
    int cur_shift_id;
    PRT_XML_TYPE type;
    char *parent_name;
    int parent_id;
    int depth;
} prt_column_t;

typedef struct
{
    /*
     * Assigned once when we now the sum of all column widths by the first row added
     */
    rhlist_t *list;
    /*
     * Modified by PRT_COLUMN_ADD, first PRT_ROW_ADD blocks the option for column changes
     */
    int col_num;
    /*
     * Sum of the all column max widths, modified only by PRT_COLUMN_ADD
     */
    int row_width;
    /**
     * Desirable display width, set around in PRT_TITLE_SET or left undefined which prompts 64 character per column
     */
    int display_width;
    /*
     * Contain per column info - keep + 1 - it is for title node
     */
    prt_column_t col[PRT_COLUMN_MAX_NUM + 1];
    /*
     * Dynamic parameter - used to keep current entry
     */
    void *row_cur;
    /*
     * Dynamic parameter - current column in the row
     */
    int col_cur;
} prt_control_t;

/**
 * \brief Get the width of the terminal to best output fit
 * \par DIRECT OUTPUT
 * \retval - terminal width
 */
int diag_sand_prt_width(
    void);

/**
 * \brief Print number of identical characters
 * \par DIRECT INPUT
 *  \param num [in] number of characters to print
 *  \param ch [in] character to print
 * \par DIRECT OUTPUT
 * \retval - terminal width
 */
void
diag_sand_prt_char(
    int num,
    char ch);

/**
 * \brief Actual output of prepared data either to display or XML file
 * \par DIRECT INPUT:
 *      \param prt_ctr [in/out] pointer to the control list of table print
 *      \param prt_file [in] xml file name
 * \par INDIRECT OUTPUT: printing or formatted output to the standard output
 * \remark automatically frees the list
 */
void diag_sand_prt(
    prt_control_t *prt_ctr, char *prt_file);

/*
 * Modes for list entries 
 */
typedef enum
{
    PRT_ROW_SEP_NONE,
    PRT_ROW_SEP_UNDERSCORE,
    PRT_ROW_SEP_UNDERSCORE_BEFORE,
    PRT_ROW_SEP_EQUAL
} PRT_ROW_SEP_MODE;

/*
 * Facilities for pretty print 
 */
/**
 * \brief Initialize variables used by pretty printing If filename_m is not NULL XML files with this name will be
 * recorded
 * \par DIRECT INPUT
 * \param [in] filename_m - filename for XML output
 * \par INDIRECT INPUT: PRT_INIT_ROW_VARS - initialization of per row local variables
 * \par INDIRECT OUTPUT: rhlist_t *prt_list - pointer to linked list of rows
 *                       void *prt_row_start - pointer to beginning of the row
 *                       int prt_column_num - number of columns in the output
 */
#define PRT_INIT_VARS                prt_control_t *prt_ctr = NULL

/**
 * \brief Free the list
 * \par INDIRECT INPUT: prt_list - pointer to the list of rows, constituting the table
 * \remark
 *  Used when leaving the routine after at least one PRT_ROW_ADD but without invocation of PRT_COMMIT
 */
#define PRT_FREE                     if(prt_ctr != NULL)                                                               \
                                     {                                                                                 \
                                         if(prt_ctr->list != NULL)                                                     \
                                             utilex_rhlist_free_all(prt_ctr->list);                                    \
                                         sal_free(prt_ctr);                                                            \
                                         prt_ctr = NULL;                                                               \
                                     }

/**
 * \brief Actual printing of prepared data
 * \par INDIRECT INPUT: prt_list - pointer to the list of rows, constituting the table
 *                      prt_column_num - number of columns in the table
 * \par INDIRECT OUTPUT: printing or formatted output to the standard output
 * \remark automatically frees the list
 */
#define PRT_COMMITF(prt_filename)                                                                                      \
                   {                                                                                                   \
                      diag_sand_prt(prt_ctr, prt_filename);                                                            \
                      PRT_FREE;                                                                                        \
                   }

#define PRT_COMMIT    PRT_COMMITF(NULL)

#define PRT_COMMITX { \
                      char *prt_filename;                                                                              \
                      SH_SAND_GET_STR("file", prt_filename);                                                           \
                      PRT_COMMITF(prt_filename);                                                                       \
                   }

#define PRT_CELL_VERIFY \
            if(prt_ctr->col_cur >= prt_ctr->col_num)                                                                   \
            {                                                                                                          \
                int col_num = prt_ctr->col_num;                                                                        \
                PRT_FREE;                                                                                              \
                SHR_CLI_EXIT(_SHR_E_INTERNAL, "Cell number in row exceeded column number:%d\n", col_num);              \
            }

#define _PRT_FORMAT(mc_ptr, mc_length, mc_format,...) {                                                                \
                                       snprintf(mc_ptr, mc_length, mc_format "%s", __VA_ARGS__);                       \
                                     }
#define PRT_FORMAT(mc_ptr, mc_length,...) {                                                                            \
                                       _PRT_FORMAT(mc_ptr, mc_length, __VA_ARGS__, "");                                \
                                     }

/**
 * \brief Fill cell with formatted data
 * \par DIRECT INPUT printf style variadic variables
 * \par INDIRECT INPUT prt_ctr - Control structure containing information on table to be pronted
 * \par INDIRECT OUTPUT prt_ctr->col_cur is updated, so to reflect that cell was filled
 * \remark MACRO verifies the number of cells in the row has not grown bigger than column number
 */
#define PRT_CELL_SET(...) \
    {                                                                                                                  \
        char *prt_row_offset;                                                                                          \
        PRT_CELL_VERIFY                                                                                                \
        prt_row_offset = (char *)prt_ctr->row_cur + sizeof(rhentry_t) + prt_ctr->col[prt_ctr->col_cur].cell_offset;    \
        PRT_FORMAT(prt_row_offset, prt_ctr->col[prt_ctr->col_cur].max_width, __VA_ARGS__);                             \
        if (prt_ctr->col[prt_ctr->col_cur].flex_type == PRT_FLEX_ASCII)                                                \
        {                                                                                                              \
            utilex_str_shrink(prt_row_offset);                                                                         \
        }                                                                                                              \
        prt_row_offset[prt_ctr->col[prt_ctr->col_cur].max_width - 1] = 0;                                              \
        prt_ctr->col_cur++;                                                                                            \
    }

/**
 * \brief Fill cell with formatted data
 * \par DIRECT INPUT variadic variables
 * \param [in] prt_n - number of shift in the cell, each shift is 2 spaces
 * \par INDIRECT INPUT prt_ctr - Control structure containing information on table to be printed
 * \par INDIRECT OUTPUT prt_ctr->col_cur is updated, so to reflect that cell was filled
 * \remark MACRO verifies the number of cells in the row has not grown bigger than column number
 */
#define PRT_CELL_SET_SHIFT(prt_n, ...) \
{                                                                                                                      \
    char *prt_row_offset;                                                                                              \
    prt_row_offset = (char *)prt_ctr->row_cur + sizeof(rhentry_t) + prt_ctr->col[prt_ctr->col_cur].cell_offset;        \
    PRT_CELL_VERIFY                                                                                                    \
    {                                                                                                                  \
      int shift_num  = prt_n;                                                                                          \
      while(shift_num--)                                                                                               \
      {                                                                                                                \
         PRT_FORMAT(prt_row_offset + strlen(prt_row_offset), prt_ctr->col[prt_ctr->col_cur].max_width, "  ");          \
      }                                                                                                                \
    }                                                                                                                  \
    PRT_FORMAT(prt_row_offset + strlen(prt_row_offset), prt_ctr->col[prt_ctr->col_cur].max_width, __VA_ARGS__);        \
    prt_row_offset[prt_ctr->col[prt_ctr->col_cur].max_width - 1] = 0;                                                  \
    prt_ctr->col_cur++;                                                                                                \
}

/**
 * \brief When filling table allows to skip cell
 * \par DIRECT INPUT
 * \param [in] mc_skip_num number of cells to be skipped
 * \par INDIRECT OUTPUT prt_ctr->col_cur is updated, so to reflect that cell was filled
 * \remark MACRO verifies the number of cells in the row has not grown bigger than column number
 */
#define PRT_CELL_SKIP(mc_skip_num)   { PRT_CELL_VERIFY                                                                 \
                                       prt_ctr->col_cur += mc_skip_num;                                                \
                                     }

/**
 * \brief Add flexible column that may grow up to width and may be splitted into multiple lines
 * \par DIRECT INPUT variadic variables
 * \par INDIRECT INPUT prt_ctr - Control structure containing information on table to be printed
 * \remark MACRO verifies the number of column does not exceed MAX
 */
#define _PRT_COLUMN_ADD(flag, width, ...) \
        {                                                                                                              \
            if(prt_ctr == NULL)                                                                                        \
            {                                                                                                          \
                SHR_CLI_EXIT(_SHR_E_INTERNAL, "Title was not defined\n");                                              \
            }                                                                                                          \
            if(prt_ctr->col_num  >= PRT_COLUMN_MAX_NUM)                                                                \
            {                                                                                                          \
                PRT_FREE;                                                                                              \
                SHR_CLI_EXIT(_SHR_E_INTERNAL, "Column num exceeded maximum:%d\n", PRT_COLUMN_MAX_NUM);                 \
            }                                                                                                          \
            prt_ctr->col[prt_ctr->col_num].cell_offset = prt_ctr->row_width;                                           \
            prt_ctr->col[prt_ctr->col_num].max_width = width;                                                          \
            prt_ctr->col[prt_ctr->col_num].flex_type = flag;                                                           \
            PRT_FORMAT(prt_ctr->col[prt_ctr->col_num].name, prt_ctr->col[prt_ctr->col_num].max_width, __VA_ARGS__);    \
            prt_ctr->row_width += prt_ctr->col[prt_ctr->col_num].max_width;                                            \
            prt_ctr->col_num++;                                                                                        \
        }

/**
 * \brief Add column
 * \par DIRECT INPUT variadic variables
 * \par INDIRECT INPUT prt_ctr - Control structure containing information on table to be printed
 * \remark MACRO verifies the number of column does not exceed MAX
 */
#define PRT_COLUMN_ADD(...) _PRT_COLUMN_ADD(PRT_FLEX_NONE, PRT_COLUMN_WIDTH, __VA_ARGS__)

/**
 * \brief Add flexible column with enlarged width
 * \par DIRECT INPUT variadic variables
 * \param [in] flag - points to the FLEX type of the column to be created, either BINARY or ASCII, see PRT_FLEX_TYPE
 * \par INDIRECT INPUT prt_ctr - Control structure containing information on table to be printed
 * \remark MACRO verifies the number of column does not exceed MAX
 */
#define PRT_COLUMN_ADD_FLEX(flag, ...) _PRT_COLUMN_ADD(flag, PRT_COLUMN_WIDTH_BIG, __VA_ARGS__)

/**
 * \brief Add column
 * \par DIRECT INPUT variadic variables
 * \param [in] prt_type - type of XML node to be created, may be either PRT_XML_CHILD, PRT_XML_ATTRUBUTE or PRT_XML_NONE
 * \param [in] prt_parent_id - id for parent XML node
 * \par DIRECT OUTPUT
 * \param [out] prt_my_id_ptr - pointer for id of this column
 * \par INDIRECT INPUT prt_ctr - Control structure containing information on table to be printed
 * \remark MACRO verifies the number of column does not exceed MAX
 */
#define PRT_COLUMN_ADDX(prt_type, prt_parent_id, prt_my_id_ptr, ...)                                                   \
        {                                                                                                              \
            _PRT_COLUMN_ADD(PRT_FLEX_NONE, PRT_COLUMN_WIDTH, __VA_ARGS__)                                              \
            {                                                                                                          \
            int *loc_my_id_ptr = prt_my_id_ptr;                                                                        \
            prt_ctr->col[prt_ctr->col_num - 1].type      = prt_type;                                                   \
            prt_ctr->col[prt_ctr->col_num - 1].parent_id = prt_parent_id;                                              \
            if(loc_my_id_ptr != NULL)                                                                                  \
                *loc_my_id_ptr = prt_ctr->col_num - 1;                                                                 \
            }                                                                                                          \
        }

/**
 * \brief Add column
 * \par DIRECT INPUT variadic variables
 * \param [in] flag - points to the FLEX type of the column to be created, either BINARY or ASCII, see PRT_FLEX_TYPE
 * \param [in] prt_type - type of XML node to be created, may be either PRT_XML_CHILD, PRT_XML_ATTRUBUTE or PRT_XML_NONE
 * \param [in] prt_parent_id - id for parent XML node
 * \par DIRECT OUTPUT
 * \param [out] prt_my_id_ptr - pointer for id of this column
 * \par INDIRECT INPUT prt_ctr - Control structure containing information on table to be printed
 * \remark MACRO verifies the number of column does not exceed MAX
 */
#define PRT_COLUMN_ADDX_FLEX(flag, prt_type, prt_parent_id, prt_my_id_ptr, ...)                                        \
        {                                                                                                              \
            _PRT_COLUMN_ADD(flag, PRT_COLUMN_WIDTH_BIG, __VA_ARGS__)                                                   \
            {                                                                                                          \
            int *loc_my_id_ptr = prt_my_id_ptr;                                                                        \
            prt_ctr->col[prt_ctr->col_num - 1].type      = prt_type;                                                   \
            prt_ctr->col[prt_ctr->col_num - 1].parent_id = prt_parent_id;                                              \
            if(loc_my_id_ptr != NULL)                                                                                  \
                *loc_my_id_ptr = prt_ctr->col_num - 1;                                                                 \
            }                                                                                                          \
        }

/**
 * \brief Set Title String for tabular output, it may appear as most upper string on display and top XML node
 * \par DIRECT INPUT variadic variables
 * \par INDIRECT OUTPUT
 *         prt_ctr->col[PRT_TITLE_ID] - title prt column is updated
 */
#define PRT_TITLE_SET(...)           {                                                                                 \
                                         if(prt_ctr != NULL)                                                           \
                                         {                                                                             \
                                             PRT_FREE;                                                                 \
                                             SHR_CLI_EXIT(_SHR_E_INTERNAL, "Title may be defined only once\n");        \
                                         }                                                                             \
                                         prt_ctr = sal_alloc(sizeof(prt_control_t), "prt_control");                    \
                                         memset(prt_ctr, 0, sizeof(prt_control_t));                                    \
                                         prt_ctr->col[PRT_TITLE_ID].depth = 1;                                         \
                                         PRT_FORMAT(prt_ctr->col[PRT_TITLE_ID].name, PRT_COLUMN_WIDTH, __VA_ARGS__);   \
                                     }

/**
 * \brief Return the total number of columns in the table Used to derive number of columns in the table
 * \par DIRECT OUTPUT
 * \retval - number of columns
 */
#define PRT_COLUMN_NUM               prt_ctr->col_num

/**
 * \brief Set mode for the last row
 * \par DIRECT INPUT
 * \param [in] mc_mode Type of separator from previous line. See PRT_ROW_SEP_MODE
 */
#define PRT_ROW_SET_MODE(mc_mode)    ((rhentry_t *)prt_ctr->row_cur)->mode = mc_mode

/**
 * \brief Add new row in the table
 * \par DIRECT INPUT
 * \param [in] mc_mode Points to the type of separator from previous line. See PRT_ROW_SEP_MODE
 * \par INDIRECT INPUT prt_list - pointer to the list of rows, that this one will be added to
 *                     prt_row_offset - current place in the row to be written (cell pointer)
 *                     prt_row_start - pointer to the beginning of row
 * \par INDIRECT OUTPUT prt_list - new row will be added to this list
 *                      prt_row_start - pointer to the beginning of new row
 *                      prt_row_offset - set to the beginning of new row skipping control info
 * \remark First row filled followed by ROW_COLUMN_SET is considered of being header one
 */
#define PRT_ROW_ADD(mc_mode) \
{                                                                                                                      \
    prt_ctr->col_cur = 0;                                                                                              \
    if(prt_ctr->list == NULL)                                                                                          \
    {                                                                                                                  \
        if((prt_ctr->list = utilex_rhlist_create("prt_print", sizeof(rhentry_t) + prt_ctr->row_width, 0)) == NULL)     \
        {                                                                                                              \
            PRT_FREE;                                                                                                  \
            SHR_CLI_EXIT(_SHR_E_MEMORY,"prt_list create %s", "failed\n");                                              \
        }                                                                                                              \
    }                                                                                                                  \
    if(utilex_rhlist_entry_add_tail(prt_ctr->list, NULL, RHID_TO_BE_GENERATED, &prt_ctr->row_cur) == _SHR_E_MEMORY)    \
    {                                                                                                                  \
         PRT_FREE;                                                                                                     \
         SHR_CLI_EXIT(_SHR_E_MEMORY,"add entry to prt_list  %s", "failed\n");                                          \
    }                                                                                                                  \
    PRT_ROW_SET_MODE(mc_mode);                                                                                         \
}

#endif /* DIAG_SAND_PRT_H */
