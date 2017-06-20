/*! \file dbx_file.h
 * Purpose:    Routines used when working with data IO
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef DBX_FILE_H_INCLUDED
#define DBX_FILE_H_INCLUDED

#include <shared/shrextend/shrextend_error.h>
#include <shared/dbx/dbx_xml.h>

typedef enum
{
  /*!
   *  No .ext for in filename
   */
  DBX_FILE_NONE,
  /*!
   * There is an extension but not the one known
   */
  DBX_FILE_UKNOWN,
  /*!
   * Comma separated format
   */
  DBX_FILE_CSV,
  /*!
   * XML format
   */
  DBX_FILE_XML,
  /*!
   * Verilog format
   */
  DBX_FILE_VERILOG,
  /*!
   * Must come last
   */
  DBX_FILE_MAX
} DBX_FILE_TYPE_E;

/*!
 * \brief Return file type according to extension.
 * String after last dot is compared to predefined set of extensions
 * \par DIRECT INPUT
 *   \param [in] filename - string representing filename to be checked
 * \par DIRECT OUTPUT:
 *   \retval file type as per DBX_FILE_TYPE_E
 */
DBX_FILE_TYPE_E dbx_file_get_type(
  char *filename);

/*!
 * \brief Check directory existence. Return TRUE if exists, FALSE if not
 * \par DIRECT INPUT
 *   \param [in] dir directory path - relative or absolute
 *   \param [in] filename name of the file
 *   \param [in,out] filepath pointer to string where resulting file path should be copied
 * \par DIRECT OUTPUT:
 *   \retval _SHR_E_NONE if path was successfully created
 *   \retval _SHR_E_PARAM if there was some problem with parameters
 */
shr_error_e dbx_file_get_path(
  char *dir,
  char *filename,
  char *filepath);

/*!
 * \brief Create full file path based on directory and file name
 * \par DIRECT INPUT
 *   \param [in] dirpath directory path - relative or absolute
 * \par DIRECT OUTPUT:
 *   \retval FALSE if file do not exists
 *   \retval TRUE if file exists
 */
int dbx_file_dir_exists(
  char *dirpath);

/*!
 * \brief Create list of all xml file's names in a directory and its sub directories
 * \par DIRECT INPUT
 *   \param [in] chip_db - chip name
 *   \param [in] dirpath directory path - relative or absolute
 *   \param [in] files_list - ptr to strings array
 * \par INDIRECT OUTPUT:
 *   \param files_list - will be updated with the file's names
 * \par DIRECT OUTPUT:
 *   \retval number of files in list
 */
int dbx_file_get_xml_files_from_dir(
  char *chip_db,
  char *dirpath,
  char **files_list);

/*!
 * \brief Check file existence. Return TRUE if exists, FALSE if not
 * Oriented for xml files only
 * \par DIRECT INPUT
 *   \param [in] filename name of the files which existence should be checked
 * \par DIRECT OUTPUT:
 *   \retval FALSE if file do not exists
 *   \retval TRUE if file exists
 * \remark
 *   If filename has no xml extension, it is added before existence check
 */
int dbx_file_exists(
  char *filename);

/*!
 * \brief Get full path for DB directory and file.
 * \par DIRECT INPUT 
 *   \param [in] chip_db directory inside DB which contains files relevant to the chip 
 *   \param [in] filename name of the file
 *   \param [in,out] filepath pointer to string where resulting file path should be copied
 * \par DIRECT OUTPUT:
 *   \retval _SHR_E_NONE if path was successfully created
 *   \retval _SHR_E_PARAM if there was some problem with parameters
 *   \retval _SHR_E_NOT_FOUND if the file or directory were not found
 *   \retval _SHR_E_INTERNAL if file or directory names exceed the allowed limit
 */
shr_error_e
dbx_file_get_full_path(
  char *chip_db,
  char *filename,
  char *filepath);

/*!
 * \brief Get handle to the top node of XML file
 * \par DIRECT INPUT
 *   \param [in] chip_db directory inside DB which contains files relevant to the chip
 *   \param [in] filename of XML to be opened
 *   \param [in] topname name of top node inside XML file
 *   \param [in] flags options for XML file opening
 * \par DIRECT OUTPUT:
 *   \retval handle to the node if operation succeeds
 *   \retval NULL on failure
 * \remark
 *   Failure may be either file does not exist or top name is wrong
 */
xml_node dbx_file_get_xml_top(
  char *chip_db,
  char *filename,
  char *topname,
  int flags);

#endif /* DBX_FILE_H_INCLUDED */
