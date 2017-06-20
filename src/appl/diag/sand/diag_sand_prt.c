/*
 * $Id: diag_sand_prt.c,v 1.00 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    diag_sand_prt.c
 * Purpose:    Beautified cli output routine
 */

#include <sal/core/libc.h>
#include <sal/core/alloc.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/types.h>
#include <shared/bsl.h>
#include <shared/util.h>

#include <shared/dbx/dbx_xml.h>
#include <shared/utilex/utilex_rhlist.h>
#include <appl/diag/sand/diag_sand_prt.h>

int diag_sand_get_shift(char *str)
{
    int i_shift;
    for(i_shift = 0; str[i_shift] == ' '; i_shift++)
    {
    }
    /* Each shift has 2 spaces */
    return i_shift/2;
}

void
diag_sand_prt_char(
    int num,
    char ch)
{
    while (num--)
        cli_out("%c", ch);
    return;
}

static void
diag_sand_prt_row(
    int num,
    char ch)
{
    while (num--)
        cli_out("%c", ch);
    cli_out("\n");
    return;
}

int
diag_sand_prt_width(
    void)
{
#if !defined(VXWORKS) && !defined(__KERNEL__)
    struct winsize w;

    ioctl(0, TIOCGWINSZ, &w);
    return w.ws_col;
#else
    return 0;
#endif
}

static int prt_get_token_size(char *prt_str, int column_width)
{
    int token_size = 0, prt_size = sal_strlen(prt_str);
    int i_ch;

    for (i_ch = 0; i_ch < prt_size; i_ch++)
    {
        /*
         * Take next row on new line
         */
        if(prt_str[i_ch] == '\n')
        {
            token_size = i_ch + 1;
            prt_str[i_ch] = ' ';
            break;
        }

        if ((prt_str[i_ch] == ' ') || (prt_str[i_ch] == ',') || (prt_str[i_ch] == '.') || (prt_str[i_ch] == '\t'))
        {
            if (i_ch < column_width)
            {   /* Keep last character that we may break the line on it */
                token_size = i_ch + 1;
            }
            else
            {
                if (token_size == 0)
                {
                    cli_out("single token is longer than:%d\n", column_width);
                    token_size = column_width - 1;
                }
                break;
            }
        }
    }

    return token_size;
}

static void
diag_sand_prt_xml(
    prt_control_t * prt_ctr,
    char *output_file)
{
#if !defined(NO_FILEIO)
    xml_node top_node;
    int k, k_cl;
    uint32 res;
    rhhandle_t prt_row_start;
    char *prt_row_offset;
    void *parent_node;
    PRT_XML_TYPE xml_type = PRT_XML_CHILD;
    int parent_id = PRT_TITLE_ID;
    int shift_n;

    if (prt_ctr->list == NULL)
    {
        cli_out("No content for table:%s\n", prt_ctr->col[PRT_TITLE_ID].name);
        return;
    }

    if (prt_ctr->col_num == 0)
    {
        cli_out("No columns defined for table:%s\n", prt_ctr->col[PRT_TITLE_ID].name);
        return;
    }

    if ((top_node = dbx_xml_top_get(output_file, "top", CONF_OPEN_CREATE)) == NULL)
    {
        cli_out("failed to create:%s\n", output_file);
        goto exit;
    }

    /*
     * Check if advanced table creation was used
     */
    if(prt_ctr->col[0].parent_id == 0)
    { /* First column having parent_id 0 is signal than no advanced XMl formatting was used  */
        /* Always assign title id to first column parent and make it child */
        for (k = 0; k < prt_ctr->col_num; k++)
        {
            prt_ctr->col[k].parent_id = parent_id;
            prt_ctr->col[k].type = xml_type;
            if(xml_type == PRT_XML_ATTRIBUTE)
            {   /* From this column and forward all columns will be attribute of the first column without empty cells */
                continue;
            }
            RHITERATOR(prt_row_start, prt_ctr->list)
            {
                prt_row_offset = ((char *) prt_row_start) + sizeof(rhentry_t) + prt_ctr->col[k].cell_offset;
                if(ISEMPTY(prt_row_offset))
                {
                    break;
                }
            }
            /*
             *  If all rows have non empty strings on specific column then next column may be its attribute
             *  If there are holes, next column will be child
             */
            parent_id = k;
            if(prt_row_start == NULL)
            { /* End of list reached, no empty cells discovered, make all next columns attributes */
                xml_type = PRT_XML_ATTRIBUTE;
            }
            else
            { /* empty cells were found, so next column will be child and not attribute */
                xml_type = PRT_XML_CHILD;
            }
        }
    }
    /*
     *  Header comes from special ID
     */
    if((prt_ctr->col[PRT_TITLE_ID].node[0] = dbx_xml_child_add(top_node, "command", prt_ctr->col[PRT_TITLE_ID].depth)) == NULL)
    {
        cli_out("failed to add:%s\n", prt_ctr->col[PRT_TITLE_ID].name);
        goto exit;
    }
    RHDATA_SET_STR(prt_ctr->col[PRT_TITLE_ID].node[0], "command_line", prt_ctr->col[PRT_TITLE_ID].name);

    RHITERATOR(prt_row_start, prt_ctr->list)
    {
        for (k = 0; k < prt_ctr->col_num; k++)
        {
            prt_row_offset = ((char *) prt_row_start) + sizeof(rhentry_t) + prt_ctr->col[k].cell_offset;
            /*
             * No use for empty cells
             */
            if (ISEMPTY(prt_row_offset))
                continue;
            /*
             * Check if it is shifted and by how many
             */
            shift_n = diag_sand_get_shift(prt_row_offset);
            /*
             * Eliminate white spaces
             */
            prt_row_offset += shift_n *2;
            /*
             * Figure out who is the parent
             */
            if((shift_n != 0) && (prt_ctr->col[k].type == PRT_XML_CHILD))
            {   /*
                 * We have shift and this column is child, we make shifted cell child to its non-shifted or less shifted
                 * brother instead of parent
                 * If shifted cell is attribute it will be assigned to its parent column anyway
                 */
                parent_node = prt_ctr->col[k].node[shift_n - 1];
            }
            else
            {
                int parent_id = prt_ctr->col[k].parent_id;
                /*
                 * If parent node is NULL look for its parent and so on until title
                 */
                do
                {
                    shift_n = prt_ctr->col[parent_id].cur_shift_id;
                    parent_node = prt_ctr->col[parent_id].node[shift_n];
                    parent_id = prt_ctr->col[parent_id].parent_id;
                }
                while ((parent_node == NULL) && (parent_id != PRT_TITLE_ID));
            }

            if (parent_node == NULL)
            {
                cli_out("Inconsistency in inheritance Parent:%d for:%s is not node\n",
                                            prt_ctr->col[k].parent_id, prt_ctr->col[k].name);
                goto exit;
            }

            switch (prt_ctr->col[k].type)
            {
                case PRT_XML_CHILD:
                    {   /*
                         * If the place is occupied it should be ended together with all underlying shifted nodes
                         * and further columns
                         */
                        int i_sh;
                        for (k_cl = k; k_cl < prt_ctr->col_num; k_cl++)
                        {
                            if(prt_ctr->col[k_cl].type == PRT_XML_CHILD)
                            {
                                for(i_sh = shift_n; i_sh < PRT_MAX_SHIFT_NUM; i_sh++)
                                {
                                    if (prt_ctr->col[k_cl].node[i_sh] != NULL)
                                    {
                                        dbx_xml_node_end(prt_ctr->col[k_cl].node[i_sh], prt_ctr->col[k_cl].depth + i_sh);
                                        prt_ctr->col[k_cl].node[i_sh] = NULL;
                                        prt_ctr->col[k_cl].cur_shift_id = (i_sh != 0) ? (i_sh - 1) : 0;
                                    }
                                }
                            }
                        }
                    }
                    prt_ctr->col[k].depth        = prt_ctr->col[prt_ctr->col[k].parent_id].depth + 1;
                    /*
                     * Make sure shift id is pointing to right place
                     */
                    prt_ctr->col[k].cur_shift_id = shift_n;
                    if(k == 0)
                    { /* First column node will be named after content not after column name */
                        prt_ctr->col[k].node[shift_n] = dbx_xml_child_add(parent_node, prt_row_offset,
                                                                                    prt_ctr->col[k].depth + shift_n);
                    }
                    else
                    {
                        prt_ctr->col[k].node[shift_n] = dbx_xml_child_add(parent_node, prt_ctr->col[k].name,
                                                                                    prt_ctr->col[k].depth + shift_n);
                        RHDATA_SET_STR(prt_ctr->col[k].node[shift_n], prt_ctr->col[k].name, prt_row_offset);
/*                      prt_ctr->col[k].node[shift_n] = dbx_xml_child_set_content_str(parent_node,
                                                prt_ctr->col[k].name, prt_row_offset, prt_ctr->col[k].depth + shift_n); */
                    }
                    break;
                case PRT_XML_ATTRIBUTE:
                    RHDATA_SET_STR(parent_node, prt_ctr->col[k].name, prt_row_offset);
                    break;
                case PRT_XML_NONE:
                default:
                    break;
            }
        }
    }

    for (k = 0; k < prt_ctr->col_num; k++)
    {
        for(shift_n = 0; shift_n < PRT_MAX_SHIFT_NUM; shift_n++)
        {
            if (prt_ctr->col[k].node[shift_n] != NULL)
            {
                dbx_xml_node_end(prt_ctr->col[k].node[shift_n], prt_ctr->col[k].depth + shift_n);
            }
        }
    }

    dbx_xml_node_end(prt_ctr->col[PRT_TITLE_ID].node[0], prt_ctr->col[PRT_TITLE_ID].depth);

    dbx_xml_top_save(top_node, output_file);
    dbx_xml_top_close(top_node);

exit:
#endif /* !defined NO_FILEIO */
    return;
}

void
diag_sand_prt(
    prt_control_t * prt_ctr, char *prt_filename)
{
    int i_rows, k, n;

    prt_format_t *column;
    int column_width;
    int total_width = 1;
    rhhandle_t prt_row_start;
    char *prt_row_offset;
    rhentry_t *entry;
    int row_num;

    if(!ISEMPTY(prt_filename))
    {
        diag_sand_prt_xml(prt_ctr, prt_filename);
        return;
    }

    if (prt_ctr->list == NULL)
    {
        cli_out("No content for table:%s\n", prt_ctr->col[PRT_TITLE_ID].name);
        return;
    }

    if (prt_ctr->col_num == 0)
    {
        cli_out("No columns\n");
        return;
    }

    row_num = RHLNUM(prt_ctr->list);
    column = sal_alloc(sizeof(prt_format_t) * prt_ctr->col_num, "column format");
    memset(column, 0, sizeof(prt_format_t) * prt_ctr->col_num);

    /**
     * Step 1 - Figure out longest cells in each column
     */
    RHITERATOR(prt_row_start, prt_ctr->list)
    {
        for (k = 0; k < prt_ctr->col_num; k++)
        {
            prt_row_offset = ((char *) prt_row_start) + sizeof(rhentry_t) + prt_ctr->col[k].cell_offset;

            column_width = sal_strlen(prt_row_offset);
            /*
             * remove trailing white spaces - keep just 4 of them max
             */
            for (n = column_width - 1; n > 4; n--)
                if (prt_row_offset[n] == ' ')
                    prt_row_offset[n] = 0;
                else
                    break;
            if (sal_strlen(prt_row_offset) > column[k].width)
                column[k].width = sal_strlen(prt_row_offset);
        }
    }

    /**
     * Step 2 - Create format per column
     */
    for (k = 0; k < prt_ctr->col_num; k++)
    {
        /*
         * Sync with Header Cell(actually column name) width
         */
        column[k].width = (strlen(prt_ctr->col[k].name) > column[k].width) ? strlen(prt_ctr->col[k].name) : column[k].width;
        /**
         * Cannot be more that PRT_COLUMN_WIDTH
         */
        if(column[k].width >= PRT_COLUMN_WIDTH)
            column[k].width = PRT_COLUMN_WIDTH -1;
        /*
         * aggregate total row size
         */
        if (column[k].width == 0)
        {
            /*
             * Just add | without spaces - it is intended for special separation
             */
            total_width += column[k].width + 1; /* for | between fields and spaces before and after */
            /*
             * create format string for each column;
             */
            sprintf(column[k].format, "|%ss", "%");
        }
        else
        {
            total_width += column[k].width + 3; /* for | between fields and spaces before and after */
            /*
             * create format string for each column;
             */
            sprintf(column[k].format, "| %s-%ds", "%", column[k].width + 1);
        }
    }

    /*
     * Step 3- Print title surrounded by '=' lines
     */

    if(!ISEMPTY(prt_ctr->col[PRT_TITLE_ID].name))
    {
        int title_width, header_width;
        char title_format[20];
        /* In case title is longer than table width - make header separatrion line same size as title */
        title_width = sal_strlen(prt_ctr->col[PRT_TITLE_ID].name);
        /*  "- 3" is two '|' and one space ' ' */
        header_width = title_width < (total_width - 3) ? total_width : title_width + 3;
        sprintf(title_format, "| %s-%ds|\n", "%", header_width - 3);

        diag_sand_prt_row(header_width, '=');
        cli_out(title_format, prt_ctr->col[PRT_TITLE_ID].name);
        diag_sand_prt_row(header_width, '=');
    }
    else
        diag_sand_prt_row(total_width, '=');
    /*
     * Step 4 - Print Header Line from prt_ctr->col surrounded by '='
     */
    for (k = 0; k < prt_ctr->col_num; k++)
    {
        cli_out(column[k].format, prt_ctr->col[k].name);
    }
    cli_out("|\n");
    diag_sand_prt_row(total_width, '=');

    /*
     * Step 5 - Print cells
     */
    i_rows = 0;
    RHITERATOR(prt_row_start, prt_ctr->list)
    {
        int cells_to_print;
        char tmp_str[PRT_COLUMN_WIDTH_BIG];
        char *to_ptr;
        int token_size;
        /**
         * Row is the one allocated by PRT user, may have multiple printed lines inside due to long cells
         */
        int i_line = 0;
        entry = (rhentry_t *) prt_row_start;
        if ((i_rows != 0) && (entry->mode == PRT_ROW_SEP_UNDERSCORE_BEFORE))
        {
            diag_sand_prt_row(total_width, '_');
        }
        do
        {
            /**
             * For each line zero cell_to_print, as if there will be no more lines in the row
             */
            cells_to_print = 0;
            if(i_line++ == 0)
            {
                /*
                 * Initialize next pointer for the first line in the row
                 */
                for (k = 0; k < prt_ctr->col_num; k++)
                {
                    column[k].next_ptr = ((char *) prt_row_start) + sizeof(rhentry_t) + prt_ctr->col[k].cell_offset;
                }
            }
            else
            {   /*
                 * If the line is not first, issue EOL
                 */
                cli_out("|\n");
            }

            for (k = 0; k < prt_ctr->col_num; k++)
            {
                token_size = sal_strlen(column[k].next_ptr);
                if(token_size > column[k].width)
                {   /**
                     * Split the string to print portions fitting into the width
                     * Adjust token_size
                     */

                    switch(prt_ctr->col[k].flex_type)
                    {
                        case PRT_FLEX_BINARY:
                            token_size = column[k].width;
                            break;
                        case PRT_FLEX_ASCII:
                            token_size = prt_get_token_size(column[k].next_ptr, column[k].width);
                            break;
                        case PRT_FLEX_NONE:
                            cli_out("Small column width:%d for the string:%d\n", column[k].width, token_size);
                            goto exit;
                            break;
                    }
                    memcpy(tmp_str, column[k].next_ptr, token_size);
                    cells_to_print++;
                    /*
                     * Make token NULL ended
                     */
                    tmp_str[token_size] = 0;
                    to_ptr = tmp_str;
                }
                else
                {
                    to_ptr = column[k].next_ptr;
                }
                cli_out(column[k].format, to_ptr);
                column[k].next_ptr += token_size;
            }
        } while(cells_to_print != 0);

        cli_out("|\n");
        if ((i_rows == row_num - 1) || (entry->mode == PRT_ROW_SEP_EQUAL))
        {
            diag_sand_prt_row(total_width, '=');
        }
        else if (entry->mode == PRT_ROW_SEP_UNDERSCORE)
        {
            diag_sand_prt_row(total_width, '-');
        }
        i_rows++;
    }

exit:
    sal_free(column);
    return;
}

