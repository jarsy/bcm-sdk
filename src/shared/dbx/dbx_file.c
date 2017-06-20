/*
 * $Id: dbx_file.c,v 1.00 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    dbx_file.c
 * Purpose:    Misc. routines used by export/import/show facilities
 */

#include <sal/core/libc.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/types.h>
#include <shared/bsl.h>

#include <shared/dbx/dbx_file.h>
#include <shared/utilex/utilex_str.h>

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SHAREDSWDNX_UTILSDNX

DBX_FILE_TYPE_E
dbx_file_get_type(
  char *filename)
{
  char **tokens;
  uint32 realtokens = 0;
  DBX_FILE_TYPE_E ret = DBX_FILE_MAX;
  char *extension;

  if ((tokens = utilex_str_split(filename, ".", 10, &realtokens)) == NULL)
  {
    LOG_WARN(BSL_LOG_MODULE, (BSL_META_U(0, "Failed to split:%s\n"), filename));
    return ret;
  }

  if ((realtokens == 1) || (realtokens == 0))
  {
    ret = DBX_FILE_NONE;
  }
  else
  {
    extension = tokens[realtokens - 1];
    if (!sal_strcmp(extension, "csv") || !sal_strcmp(extension, "txt"))
      ret = DBX_FILE_CSV;
    else if (!sal_strcmp(extension, "xml"))
      ret = DBX_FILE_XML;
    else if (!sal_strcmp(extension, "v"))
      ret = DBX_FILE_VERILOG;
    else
      ret = DBX_FILE_UKNOWN;
  }

  utilex_str_split_free(tokens, realtokens);
  return ret;
}

shr_error_e
dbx_file_get_path(
  char *dir,
  char *filename,
  char *filepath)
{
  int length = 6;
  DBX_FILE_TYPE_E file_type;

  if (!ISEMPTY(dir))
    length += sal_strlen(dir);

  if (!ISEMPTY(filename))
    length += sal_strlen(filename);
  else
  {
    LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(0, "Filename cannot be empty\n")));
    return _SHR_E_PARAM;
  }

  if (length >= RHFILE_MAX_SIZE)
  {
    LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(0, "Filename length:%d exceeded limit:%d\n"), length, RHFILE_MAX_SIZE));
    return _SHR_E_PARAM;
  }

  if (dir != NULL)
  {
    sal_strcpy(filepath, dir);
    sal_strcat(filepath, "/");
  }
  else  /* Make it NULL string */
    filepath[0] = 0;

  sal_strcat(filepath, filename);
  file_type = dbx_file_get_type(filename);
  if ((file_type != DBX_FILE_XML) && (file_type != DBX_FILE_CSV))
  {
    LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(0, "xml extension added to %s\n"), filename));
    strcat(filepath, ".xml");
  }
  return _SHR_E_NONE;
}

#if !defined(NO_FILEIO)
#include <stdlib.h>
/*
 * {
 */
int
dbx_file_dir_exists(
  char *dirpath)
{
  SAL_DIR *dir;

  dir = sal_opendir(dirpath);
  if (!dir)
  {
    /*
     * No such directory
     */
    return FALSE;
  }

  sal_closedir(dir);
  return TRUE;
}

static
void dbx_file_add_dir_to_file_path(
  char *file_name,
  char *dir)
{
  char full_path[RHFILE_MAX_SIZE];

  sal_strncpy(full_path, dir, sizeof(full_path) - 1);
  full_path[sizeof(full_path) - 1] = '\0';
  sal_strncat(full_path, "/", sizeof(full_path) - sal_strlen(full_path) - 1);
  sal_strncat(full_path, file_name, sizeof(full_path) - sal_strlen(full_path) - 1);
  sal_strcpy(file_name,full_path);
}

int
dbx_file_get_xml_files_from_dir(
  char *chip_db,
  char *dirpath,
  char **files_list)
{
  int idx=0;
  SAL_DIR *dir;
  struct sal_dirent* dirent;
  int unit = 0;
  
  char *db_dir = NULL;
  char db_path[RHFILE_MAX_SIZE];
  char sub_dir[RHFILE_MAX_SIZE];
  char last_sub_dir[RHFILE_MAX_SIZE];

  /*
   * check if main directory exists
   */
  db_dir = getenv("DPP_DB_PATH");
  if (ISEMPTY(db_dir) || (dbx_file_dir_exists(db_dir) == FALSE))
  {
    LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "DB_PATH was not found in %s\n"), db_dir));
    db_dir = "./db";
  }

  sal_strncpy(db_path, db_dir, sizeof(db_path) - 1);
  db_path[sizeof(db_path) - 1] = '\0';
  sal_strncat(db_path, "/", sizeof(db_path) - sal_strlen(db_path) - 1);
  sal_strncat(db_path, chip_db, sizeof(db_path) - sal_strlen(db_path) - 1);
  sal_strncat(db_path, "/", sizeof(db_path) - sal_strlen(db_path) - 1);
  sal_strncat(db_path, dirpath, sizeof(db_path) - sal_strlen(db_path) - 1);

  if(!dbx_file_dir_exists(db_path))
  {
    return 0;
  }

  dir = sal_opendir(db_path);

  while ((dirent = sal_readdir(dir)))
  {
    if(sal_strcmp(dirent->d_name, ".") == 0 || sal_strcmp(dirent->d_name, "..") == 0)
    {
      continue;
    }

    sal_strncpy(sub_dir, db_path, sizeof(sub_dir) - 1);
    sub_dir[sizeof(sub_dir) - 1] = '\0';
    sal_strncat(sub_dir, "/", sizeof(sub_dir) - sal_strlen(sub_dir) - 1);
    sal_strncat(sub_dir, dirent->d_name, sizeof(sub_dir) - sal_strlen(sub_dir) - 1);
    if (dbx_file_dir_exists(sub_dir))
    {
      /*
       * directory - recuirsive call
       */
      int nof_xml_in_sub;
      int ii;

      sal_strncpy(sub_dir, dirpath, sizeof(sub_dir) - 1);
      sub_dir[sizeof(sub_dir) - 1] = '\0';
      sal_strncat(sub_dir, "/", sizeof(sub_dir) - sal_strlen(sub_dir) - 1);
      sal_strncat(sub_dir, dirent->d_name, sizeof(sub_dir) - sal_strlen(sub_dir) - 1);
      sal_strncpy(last_sub_dir, dirent->d_name, sizeof(last_sub_dir) - 1);
      last_sub_dir[sizeof(last_sub_dir) - 1] = '\0';

      nof_xml_in_sub = dbx_file_get_xml_files_from_dir(chip_db,sub_dir,&(files_list[idx]));
      for (ii = 0 ; ii < nof_xml_in_sub ; ii++)
      {
        dbx_file_add_dir_to_file_path(files_list[idx+ii],last_sub_dir);
      }
      idx += nof_xml_in_sub;
    }
    else if (dbx_file_get_type(dirent->d_name) == DBX_FILE_XML)
    {
      /*
       * xml file - add to files list
       */
      sal_strcpy(files_list[idx++],dirent->d_name);
    }
  }

  sal_closedir(dir);
  return idx;
}

int
dbx_file_exists(
  char *filename)
{
  char filepath[RHFILE_MAX_SIZE];
  FILE *in;

  if (dbx_file_get_path(NULL, filename, filepath) != _SHR_E_NONE)
  {
    return FALSE;
  }

  in = sal_fopen(filepath, "r");
  if (!in)
  {
    /*
     * No such file - not an error - just warning 
     */
    return FALSE;
  }

  sal_fclose(in);
  return TRUE;
}

shr_error_e
dbx_file_get_full_path(
  char *descr,
  char *filename,
  char *filepath)
{
  char dir[RHFILE_MAX_SIZE];
  int unit = 0;
  char *db_dir = NULL;
  shr_error_e rv;

  if (ISEMPTY(filename))
  {
    LOG_ERROR(BSL_LOG_MODULE, (BSL_META_U(unit, "DB Resource not defined\n")));
    return _SHR_E_PARAM;
  }

  db_dir = getenv("DPP_DB_PATH");
  if (ISEMPTY(db_dir) || (dbx_file_dir_exists(db_dir) == FALSE))
  {
    LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "DB was not found in %s\n"), db_dir));
    db_dir = "./db";
  }

  if (!ISEMPTY(db_dir))
  {
    if (dbx_file_dir_exists(db_dir) == FALSE)
    {
      LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "DB was not found in %s\n"), db_dir));
      return _SHR_E_NOT_FOUND;
    }
  }

  if (sal_strlen(db_dir) > (RHFILE_MAX_SIZE - 5))
  {
    LOG_WARN(BSL_LOG_MODULE, (BSL_META_U(unit, "PATH %s is too long\n"), db_dir));
    return _SHR_E_INTERNAL;
  }

  if (dbx_file_dir_exists(db_dir) == FALSE)
  {
    LOG_VERBOSE(BSL_LOG_MODULE, (BSL_META_U(unit, "DB was not found in %s\n"), db_dir));
    return _SHR_E_NOT_FOUND;
  }

  strcpy(dir, db_dir);
  if (!ISEMPTY(descr))
  {
    strcat(dir, "/");
    if (sal_strlen(descr) > (RHFILE_MAX_SIZE - (sal_strlen(db_dir) + 5)))
    {
      LOG_WARN(BSL_LOG_MODULE, (BSL_META_U(unit, "Directory %s is too long\n"), descr));
      return _SHR_E_INTERNAL;
    }

    strcat(dir, descr);
  }

  if (dbx_file_dir_exists(dir) == FALSE)
  {
    LOG_WARN(BSL_LOG_MODULE, (BSL_META_U(unit, "Device DB:%s does not exist\n"), dir));
    return _SHR_E_NOT_FOUND;
  }

  rv = dbx_file_get_path(dir, filename, filepath);
  if (rv != _SHR_E_NONE)
  {
    return rv;
  }

  return _SHR_E_NONE;
}

xml_node
dbx_file_get_xml_top(
  char *descr,
  char *filename,
  char *topname,
  int flags)
{
  void *curTop = NULL;
  char filepath[RHFILE_MAX_SIZE];
  int unit = 0;

  if (dbx_file_get_full_path(descr, filename, filepath) != _SHR_E_NONE) {
    goto out;
  }
  if ((curTop = dbx_xml_top_get(filepath, topname, flags)) == NULL)
  {
    LOG_WARN(BSL_LOG_MODULE, (BSL_META_U(unit, "Device DB was not found in %s\n"), filepath));
    goto out;
  }

out:
  return curTop;
}
#else
xml_node
dbx_file_get_xml_top(
  char *descr,
  char *filename,
  char *topname,
  int flags)
{
  return NULL;
}
/*
 * }
 */
#endif /* !defined(NO_FILEIO) */
