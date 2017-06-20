/**
 * \file diag_sand_resource.c
 *
 * Framework for sand shell commands development
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <appl/diag/sand/diag_sand_framework.h>
#include <appl/diag/sand/diag_sand_utils.h>

sh_sand_option_t sh_sand_sys_options[] = {
    {"CoLuMNs",  SAL_FIELD_TYPE_INT32, "Maximum columns number for output",                                 "80"},
    {"core",     SAL_FIELD_TYPE_INT32, "Core ID for multi-core devices",                                    "-1"},
    {"file",     SAL_FIELD_TYPE_STR,   "File name where command output will be placed",                     ""},
    {"MaRGin",   SAL_FIELD_TYPE_INT32, "Left&Right margin of defined display width",                        "7"},
    {"TABular",  SAL_FIELD_TYPE_BOOL,  "Print usage in tabular view",                                       "No"},
    {"VERbose",  SAL_FIELD_TYPE_BOOL,  "Print all options",                                                 "No"},
    {NULL}
};

sh_sand_option_t sh_sand_options[] = {
    {NULL}
};

/*
 * List should be arranged in alphabet order
 */
sh_sand_keyword_t sh_sand_keywords[] = {
    {"ACCess"},
    {"Add"},
    {"ALL"},
    {"app_id"},
    {"BLock"},
    {"bool"},
    {"cache"},
    {"CHanged"},
    {"CLear"},
    {"CoMmit"},
    {"CouNT"},
    {"DaTa"},
    {"DBaL"},
    {"DeLeTe"},
    {"DESC"},
    {"DIRECT"},
    {"DuMP"},
    {"EM"},
    {"ENTry"},
    {"enum"},
    {"FieLD"},
    {"FLaGs"},
    {"from"},
    {"Get"},
    {"HanDLe"},
    {"ID"},
    {"INDex"},
    {"INFO"},
    {"ip4"},
    {"ip6"},
    {"Key"},
    {"Key_SIZE"},
    {"LaBeL"},
    {"List"},
    {"LPM"},
    {"LoGger"},
    {"mac"},
    {"MDB"},
    {"MoDe"},
    {"NaMe"},
    {"order"},
    {"over"},
    {"PARAMeter"},
    {"PayLoad"},
    {"PayLoad_SIZE"},
    {"prefix_length"},
    {"PRoPerty"},
    {"Read"},
    {"search"},
    {"Set"},
    {"SeVeRity"},
    {"SHow"},
    {"SIGnal"},
    {"STAGE"},
    {"StaTuS"},
    {"STRUCTure"},
    {"TaBLe"},
    {"TeST"},
    {"TO"},
	{"Type"},
    {"variable"},
    {"WB"},
    {NULL}
};
