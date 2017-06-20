/* $Id: dcmn_mbist.c,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
 
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MBIST
#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/dcmn/dcmn_mbist.h>
#include <soc/dcmn/dcmn_mem.h>
#include <soc/drv.h>
#include <soc/memory.h>
#include <soc/mem.h>


#define COMMAND_OFFSET ((unsigned)(commands - first_command))
uint32 mbist_get_uint32(int unit, uint32 *out_val, uint8  **commands,int32 *nof_commands)
{
    SOCDNX_INIT_FUNC_DEFS;
    if (*nof_commands < 4) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("not enough data left for uint32")));
    }
    *out_val = (*(*commands)++) << 24;
    *out_val |= (*(*commands)++) << 16;
    *out_val |= (*(*commands)++) << 8;
    *out_val |= *((*commands)++);
    *nof_commands -= 4;
exit:
    SOCDNX_FUNC_RETURN; 
}

/* 
 * test using a given MBIST script
 */
uint32
soc_dcmn_run_mbist_script(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN dcmn_mbist_script_t *script,  
    SOC_SAND_IN dcmn_mbist_device_t *mbist_device, 
    SOC_SAND_INOUT dcmn_mbist_dynamic_t *dynamic
    )
{
    uint8 *first_command  = (uint8 *)script->commands;
    char **comment_ptr = (char **)script->comments;
    char  *name, *last_comment = "";
#ifdef _MBIST_PRINT_TWO_COMMENTS
    char *prev_comment = "";
#endif
    uint8 *commands = (uint8 *)script->commands;
    uint32 script_line = 0, val32, expected_value, mask;
    int32 nof_commands = script->nof_commands;
    uint32 nof_comments = script->nof_comments;
    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(script);
    name = script->script_name;
    if (!first_command || !comment_ptr || !name) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_SOCDNX_MSG("invalid MBIST script")));
    } else if (dynamic->skip_errors && !dynamic->ser_test_delay) {
        LOG_VERBOSE(BSL_LS_SOC_MBIST, (BSL_META_U(unit, "MBIST script %s starting after %u us\n"), name, sal_time_usecs() - dynamic->measured_time));
    } else {
        LOG_DEBUG(BSL_LS_SOC_MBIST, (BSL_META_U(unit, "MBIST script %s starting after %u us\n"), name, sal_time_usecs() - dynamic->measured_time));
    }

    while (nof_commands > 0) {
        --nof_commands;
        switch (DCMN_MBIST_COMMAND_MASK & *commands) {
        case DCMN_MBIST_COMMAND_READ:
            val32 = (DCMN_MBIST_COMMAND_INVMASK & *(commands++)) << 8; /* assumes DCMN_MBIST_COMMAND_READ == 0 */
            val32 += *(commands++);
            if (nof_commands-- < 9) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Write command exceeds script %s size"), name));
            }
            script_line += val32;
            SOCDNX_IF_ERR_EXIT(mbist_get_uint32(unit, &mask, &commands, &nof_commands));
            SOCDNX_IF_ERR_EXIT(mbist_get_uint32(unit, &expected_value, &commands, &nof_commands));
            /* if mask ==0 then read command is actually dummy read command  intend to save the original line number
             * which was too big to save inside 14 bits 
             * and save it in the expected_value field whic is 32 bits long
            */
            if ((!mask)) {
                script_line = expected_value;
                break;
            }
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, mbist_device->reg_tap_data_out, REG_PORT_ANY, 0, &val32)); /* read MBIST output */
            if ((val32 & mask) != expected_value) { /* check the success of the given test */
#ifdef _MBIST_PRINT_TWO_COMMENTS
                LOG_ERROR(BSL_LS_SOC_MBIST, (BSL_META_U(unit,
                          "unit %d at %s MBIST failure:%u expected to get result 0x%x with mask 0x%x, and instead got 0x%x. script comments:\n%s\n%s\n"),
                          unit, name, script_line, expected_value, mask, (val32 & mask), prev_comment, last_comment));
#else
                LOG_ERROR(BSL_LS_SOC_MBIST, (BSL_META_U(unit,
                          "unit %d at %s MBIST failure:%u expected to get result 0x%x with mask 0x%x, and instead got 0x%x. script comment:\n%s\n"),
                          unit, name, script_line, expected_value, mask, (val32 & mask), last_comment));
#endif
                ++dynamic->nof_errors;
                if (!dynamic->skip_errors) {
                    _rv = SOC_E_FAIL;
                    SOC_EXIT;
                }
            }
            break;
        case DCMN_MBIST_COMMAND_WRITE:
            if (*(commands++) != DCMN_MBIST_COMMAND_WRITE) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,(_BSL_SOCDNX_MSG("invalid write command in script %s offset %u"), name, COMMAND_OFFSET));
            }
            SOCDNX_IF_ERR_EXIT(mbist_get_uint32(unit, &val32, &commands, &nof_commands));
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mbist_device->reg_tap_data_in, REG_PORT_ANY, 0, val32)); /* write MBIST input */
            sal_usleep(script->sleep_after_write);
            break;
        case DCMN_MBIST_COMMAND_COMMENT:
            if (*(commands++) != DCMN_MBIST_COMMAND_COMMENT) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,(_BSL_SOCDNX_MSG("invalid comment command in script %s offset %u"), name, COMMAND_OFFSET));
            }
            if (!nof_comments--) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,(_BSL_SOCDNX_MSG("comments number mismatch in script %s"), name));
            }
#ifdef _MBIST_PRINT_TWO_COMMENTS
            prev_comment = last_comment;
#endif
            last_comment = *(comment_ptr++);
            break;
        case DCMN_MBIST_COMMAND_WAIT:
            ++nof_commands;
            SOCDNX_IF_ERR_EXIT(mbist_get_uint32(unit, &val32, &commands, &nof_commands)); /* assumes DCMN_MBIST_COMMAND_WAIT == 0 */
            if (val32 == DCMN_MBIST_TEST_LONG_WAIT_VALUE) { /* Is this a special long sleep used for SER testing? */
                if (dynamic->ser_test_delay & DCMN_MBIST_TEST_LONG_WAIT_DELAY_IS_SEC) { /* a long sleep of the test specified in seconds or in useconds */
                    sal_sleep(dynamic->ser_test_delay & ~DCMN_MBIST_TEST_LONG_WAIT_DELAY_IS_SEC);
                } else {
                    sal_usleep(dynamic->ser_test_delay);
                }
            } else {
                sal_usleep((val32) / mbist_device->sleep_divisor); /* a regular MBIST sleep */
            }
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL,(_BSL_SOCDNX_MSG("internal case error in MBIST")));
        }

    }

    if (nof_commands != 0) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Exceeded script %s size"), name));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/* function to initialize the MBIST mechanism */
uint32 soc_dcmn_mbist_init(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN dcmn_mbist_device_t *mbist_device, 
    dcmn_mbist_dynamic_t *dynamic
    )
{
    SOCDNX_INIT_FUNC_DEFS;
    LOG_VERBOSE(BSL_LS_SOC_MBIST, (BSL_META_U(unit, "Memory BIST Started\n")));
    dynamic->measured_time = sal_time_usecs();
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mbist_device->reg_tap_command, REG_PORT_ANY, 0, 3));
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mbist_device->reg_tap_command, REG_PORT_ANY, 0, 2));
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mbist_device->reg_tap_command, REG_PORT_ANY, 0, 3));
    sal_usleep(10000);
exit:
    SOCDNX_FUNC_RETURN;
}

/* function to de-initialize the MBIST mechanism */
uint32 soc_dcmn_mbist_deinit(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN dcmn_mbist_device_t *mbist_device,  
    SOC_SAND_IN dcmn_mbist_dynamic_t *dynamic
    )
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, mbist_device->reg_tap_command, REG_PORT_ANY, 0, 0));
    if (!dynamic->ser_test_delay) { /* Do not report this this in SER testing iteration end */
        LOG_VERBOSE(BSL_LS_SOC_MBIST, (BSL_META_U(unit, "Memory BIST done in %u Microseconds\n"), sal_time_usecs() - dynamic->measured_time));
    }
exit:
    SOCDNX_FUNC_RETURN;
}


#undef _ERR_MSG_MODULE_NAME


