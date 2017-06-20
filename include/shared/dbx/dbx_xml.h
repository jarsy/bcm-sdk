/*
 * $Id: dbx_xml.h,v 1.00 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    dbx_xml.h
 * Purpose:    Types and structures used when working with data IO
 */

#ifndef DBX_XML_H_INCLUDED
#define DBX_XML_H_INCLUDED

#include <shared/utilex/utilex_str.h>

typedef void *xml_node;

/* open file flags */
#define  CONF_OPEN_CREATE     0x0001
#define  CONF_OPEN_OVERWRITE  0x0002

typedef struct
{
  char name[RHNAME_MAX_SIZE];
  char value[RHNAME_MAX_SIZE];
} attribute_param_t;

xml_node dbx_xml_top_get(
  char *filepath,
  char *topname,
  int flags);
xml_node dbx_xml_top_create(
  char *topname);
void dbx_xml_top_close(
  xml_node top);
void dbx_xml_top_save(
  xml_node top,
  char *filepath);

xml_node dbx_xml_child_get_first(
  xml_node parent,
  char *childname);
xml_node dbx_xml_child_get_next(
  xml_node sibling);
xml_node dbx_xml_child_get_content_str(
  xml_node parent,
  char *childname,
  char *target,
  int size);
xml_node dbx_xml_child_get_content_int(
  xml_node parent,
  char *childname,
  int *target);
xml_node dbx_xml_child_set_content_str(
  xml_node parent,
  char *childname,
  char *source,
  int depth);
xml_node dbx_xml_child_set_content_int(
  xml_node parent,
  char *childname,
  int source,
  int depth);
xml_node dbx_xml_child_add(
  xml_node parent,
  char *childname,
  int indent_num);
xml_node dbx_xml_child_add_prev(
  xml_node parent,
  xml_node sibling,
  char *childname,
  int indent_num);

int dbx_xml_node_get_content_str(
  xml_node node,
  char *target,
  int size);
xml_node dbx_xml_node_add_str(
  xml_node node,
  char *str);
int dbx_xml_node_end(
  xml_node node,
  int depth);
void dbx_xml_node_delete(
  xml_node node);

int dbx_xml_property_set_str(
  xml_node node,
  char *property,
  char *source);
int dbx_xml_property_set_int(
  xml_node node,
  char *property,
  int source);
int dbx_xml_property_set_hex(
  xml_node node,
  char *property,
  int source);
int dbx_xml_property_mod_str(
  xml_node node,
  char *property,
  char *source);
int dbx_xml_property_mod_int(
  xml_node node,
  char *property,
  int source);
int dbx_xml_property_delete(
  xml_node node,
  char *property);
int dbx_xml_property_get_all(
  xml_node node,
  attribute_param_t * attribute_param,
  char *exclude_property,
  int max_num);
int dbx_xml_property_get_str(
  xml_node node,
  char *property,
  char *target,
  uint32 target_size);
int dbx_xml_property_get_int(
  xml_node node,
  char *property,
  int *target);

#define RHDATA_ITERATOR(node_mac, parent_mac, name_mac)                        \
    for(node_mac = dbx_xml_child_get_first(parent_mac, name_mac); node_mac; node_mac = dbx_xml_child_get_next(node_mac))

#define RHDATA_SAFE_ITERATOR(node_mac, parent_mac, name_mac)                   \
  xml_node next_node_mac;\
  for(node_mac = dbx_xml_child_get_first(parent_mac, name_mac), next_node_mac = dbx_xml_child_get_next(node_mac); node_mac; node_mac = next_node_mac, next_node_mac = xml_get_next(node_mac))

#define RHDATA_GET_NODE_NUM(num_mac, parent_mac, name_mac) {      \
            xml_node node_mac;                                    \
            RHDATA_ITERATOR(node_mac, parent_mac, name_mac)       \
                num_mac++;                                        \
        }

#define RHCONTENT_GET_STR_DEF(parent_mac, childname_mac, target_mac, default_str)                         \
    if((dbx_xml_child_get_content_str(parent_mac, childname_mac, target_mac, RHNAME_MAX_SIZE)) == NULL) { \
        strcpy(target_mac, default_str);                                                                  \
    }

#define RHCONTENT_GET_INT_DEF(parent_mac, childname_mac, target_mac, def_value_mac)         \
    if(dbx_xml_child_get_content_int(parent_mac, childname_mac, target_mac) == NULL){       \
        *target_mac = def_value_mac;                                                        \
    }

#define RHCONTENT_GET_STR_STOP(parent_mac, childname_mac, target_mac)                       \
    dbx_xml_child_get_content_str(parent_mac, childname_mac, target_mac, RHNAME_MAX_SIZE);  \
    if(target_mac == NULL) {                                                                \
       goto exit;                                                                           \
    }

#define RHCONTENT_GET_INT_STOP(parent_mac, childname_mac, target_mac)                       \
    if((dbx_xml_child_get_content_int(parent_mac, childname_mac, target_mac)) == NULL) {    \
        goto exit;                                                                          \
    }

#define RHCONTENT_GET_INT(parent_mac, childname_mac, target_mac)             \
    dbx_xml_child_get_content_int(parent_mac, childname_mac, target_mac)

#define RHCHDATA_GET_STR_STOP(node_mac, childname_mac ,string_mac, target_mac)                      \
{   xml_node child_node_mac;					                                                   \
	child_node_mac = dbx_xml_child_get_first(node_mac,childname_mac);                              \
	if(child_node_mac != NULL) {																   \
        if((res = dbx_xml_property_get_str(child_node_mac, string_mac, target_mac, RHNAME_MAX_SIZE)) != _SHR_E_NONE) { \
            goto exit;                                                                             \
        }                                                                                          \
    }                                                                                              \
    else                                                                                            \
    {                                                                                               \
      goto exit;                                                                                    \
    }                                                                                               \
}

#define RHCHDATA_GET_INT_STOP(node_mac, childname_mac ,string_mac, target_mac)                      \
{   xml_node child_node_mac;					                                                   \
	child_node_mac = dbx_xml_child_get_first(node_mac,childname_mac);                              \
	if(child_node_mac != NULL) {																   \
        if((res = dbx_xml_property_get_int(child_node_mac, string_mac, target_mac)) != _SHR_E_NONE) { \
            goto exit;                                                                             \
        }                                                                                          \
    }                                                                                              \
    else                                                                                            \
    {                                                                                               \
      goto exit;                                                                                    \
    }                                                                                               \
}

#define RHCHDATA_GET_STR_DEF(node_mac, childname_mac ,string_mac, target_mac, default_str)       \
{   xml_node child_node_mac;                                                                       \
    child_node_mac = dbx_xml_child_get_first(node_mac,childname_mac);                              \
    if(child_node_mac != NULL) {                                                                   \
		if((res = dbx_xml_property_get_str(child_node_mac, string_mac, target_mac, RHNAME_MAX_SIZE)) != _SHR_E_NONE) { \
            strcpy(target_mac, default_str);                                                       \
	}																							   \
	}																							   \
    else                                                                                           \
    {                                                                                              \
        strcpy(target_mac, default_str);                                                           \
    }                                                                                              \
}

#define RHCHDATA_GET_INT_DEF(node_mac, childname_mac ,string_mac, target_mac, def_value_mac)       \
{   xml_node child_node_mac;                                                                       \
    child_node_mac = dbx_xml_child_get_first(node_mac,childname_mac);                              \
    if(child_node_mac != NULL) {                                                                   \
		if((res = dbx_xml_property_get_int(child_node_mac, string_mac, &target_mac)) != _SHR_E_NONE) { \
            target_mac = def_value_mac;                                                            \
		}																						   \
	}																							   \
    else                                                                                           \
    {                                                                                              \
        target_mac = def_value_mac;                                                                \
    }                                                                                              \
}

#define RHDATA_GET_STR_CONT(node_mac, string_mac, target_mac)                                      \
    if((dbx_xml_property_get_str(node_mac, string_mac, target_mac, RHNAME_MAX_SIZE)) != _SHR_E_NONE) {    \
        continue;                                                                                  \
    }

#define RHDATA_GET_LSTR_CONT(node_mac, string_mac, target_mac)                                     \
    if((dbx_xml_property_get_str(node_mac, string_mac, target_mac, RHSTRING_MAX_SIZE)) != _SHR_E_NONE) {  \
        continue;                                                                                  \
    }

#define RHDATA_GET_STR_STOP(node_mac, string_mac, target_mac)                                               \
    if((res = dbx_xml_property_get_str(node_mac, string_mac, target_mac, RHNAME_MAX_SIZE)) != _SHR_E_NONE) { \
        goto exit;                                                                              \
    }

#define RHDATA_GET_LSTR_STOP(node_mac, string_mac, target_mac)                                            \
    if((res = dbx_xml_property_get_str(node_mac, string_mac, target_mac, RHSTRING_MAX_SIZE)) != _SHR_E_NONE) { \
        goto exit;                                                                                \
    }

#define RHDATA_GET_STR_DEF_NULL(node_mac, string_mac, target_mac)                                      \
    if((dbx_xml_property_get_str(node_mac, string_mac, target_mac, RHNAME_MAX_SIZE)) != _SHR_E_NONE) { \
        target_mac[0] = 0;                                                                             \
    }

#define RHDATA_GET_STR_DEF(node_mac, string_mac, target_mac, default_str)                               \
    if((dbx_xml_property_get_str(node_mac, string_mac, target_mac, RHNAME_MAX_SIZE)) != _SHR_E_NONE) {  \
        strcpy(target_mac, default_str);                                                                \
    }

#define RHDATA_GET_INT_CONT(node_mac, string_mac, target_mac)                                           \
    if((dbx_xml_property_get_int(node_mac, string_mac, (int *)(&(target_mac)))) != _SHR_E_NONE) {       \
        continue;                                                                                       \
    }

#define RHDATA_GET_INT_STOP(node_mac, string_mac, target_mac)                                           \
    if((res = dbx_xml_property_get_int(node_mac, string_mac, &(target_mac))) != _SHR_E_NONE) {          \
        goto exit;                                                                                      \
    }

#define RHDATA_GET_INT_DEF(node_mac, string_mac, target_mac, def_value_mac)                             \
    if((dbx_xml_property_get_int(node_mac, string_mac, (int *)(&(target_mac)))) != _SHR_E_NONE) {       \
        target_mac = def_value_mac;                                                                     \
    }

#define RHDATA_GET_NUM(node_mac, string_mac, target_mac)                                                \
    if((dbx_xml_property_get_num(node_mac, string_mac, (void *)&(target_mac), sizeof(target_mac))) != _SHR_E_NONE) { \
        continue;                                                                                       \
    }

#define RHDATA_GET_NUM_CONT(node_mac, string_mac, target_mac)                                              \
    if((dbx_xml_property_get_num(node_mac, string_mac, (void *)&(target_mac), sizeof(target_mac))) != _SHR_E_NONE) { \
        continue;                                                                              \
    }

#define RHDATA_GET_NUM_STOP(node_mac, string_mac, target_mac)                                              \
    if((res = dbx_xml_property_get_num(node_mac, string_mac, (void *)&(target_mac), sizeof(target_mac))) != _SHR_E_NONE) { \
        goto exit;                                                                             \
    }

#define RHDATA_GET_NUM_DEF(node_mac, string_mac, target_mac, def_value_mac)                                    \
    if((dbx_xml_property_get_num(node_mac, string_mac, (void *)&(target_mac), sizeof(target_mac))) != _SHR_E_NONE) {        \
        target_mac = def_value_mac;                                                                    \
    }

#define RHDATA_SET_INT(node_mac, string_mac, source_mac)                                    \
    if((res = dbx_xml_property_set_int(node_mac, string_mac, source_mac)) != _SHR_E_NONE) { \
        goto exit;                                                                          \
    }

#define RHDATA_SET_HEX(node_mac, string_mac, source_mac)                                    \
    if((res = dbx_xml_property_set_hex(node_mac, string_mac, source_mac)) != _SHR_E_NONE) { \
        goto exit;                                                                          \
    }

#define RHDATA_SET_STR(node_mac, string_mac, source_mac)                                    \
    if((res = dbx_xml_property_set_str(node_mac, string_mac, source_mac)) != _SHR_E_NONE) { \
        goto exit;                                                                          \
    }

#define RHDATA_SET_BOOL(node_mac, string_mac, source_mac)                                   \
    if((res = dbx_xml_property_set_str(node_mac, string_mac, (source_mac != 0) ? "Yes":"No")) != _SHR_E_NONE) {  \
        goto exit;                                                                          \
    }

#define RHDATA_MOD_INT(node_mac, string_mac, source_mac)                                    \
    if((res = dbx_xml_property_mod_int(node_mac, string_mac, source_mac)) != _SHR_E_NONE) { \
        goto exit;                                                                          \
    }

#define RHDATA_MOD_STR(node_mac, string_mac, source_mac)                                    \
    if((res = dbx_xml_property_mod_str(node_mac, string_mac, source_mac)) != _SHR_E_NONE) { \
        goto exit;                                                                          \
    }

#endif /* DBX_XML_H_INCLUDED */
