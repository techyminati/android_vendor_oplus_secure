/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 * Version: V1.0
 * Description:
 * History:
 */

#include "gf_error.h"
#include "gf_type_define.h"
#include "gf_common.h"
#include "gf_hal_log.h"

#define LOG_TAG "[GF_HAL][gf_hal_test_stub]"

uint8_t g_test_interrupt_pin_flag = 0;  // test interrupt pin flag
uint32_t g_fpc_config_had_downloaded;  // g_fpc_config_had_downloaded


/**
 * Function: gf_hal_test_invoke_command
 * Description: stub code.
 * Input: NONE
 * Output: NONE
 * Return: gf_error_t
 */
gf_error_t gf_hal_test_invoke_command(gf_cmd_id_t cmd_id, void *buffer, uint32_t len)
{
    gf_error_t err = GF_ERROR_TEST_NOT_SUPPORTED;
    FUNC_ENTER();
    UNUSED_VAR(cmd_id);
    UNUSED_VAR(buffer);
    UNUSED_VAR(len);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_test_cmd
 * Description: stub code.
 * Input: NONE
 * Output: NONE
 * Return: gf_error_t
 */
gf_error_t gf_hal_test_cmd(void *dev, uint32_t cmd_id, const uint8_t *param,
                           uint32_t param_len)
{
    gf_error_t err = GF_ERROR_TEST_NOT_SUPPORTED;
    FUNC_ENTER();
    UNUSED_VAR(dev);
    UNUSED_VAR(cmd_id);
    UNUSED_VAR(param);
    UNUSED_VAR(param_len);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_test_cancel
 * Description: stub code.
 * Input: NONE
 * Output: NONE
 * Return: gf_error_t
 */
gf_error_t gf_hal_test_cancel(void *dev)
{
    gf_error_t err = GF_ERROR_TEST_NOT_SUPPORTED;
    FUNC_ENTER();
    UNUSED_VAR(dev);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: gf_hal_test_prior_cancel
 * Description: stub code.
 * Input: NONE
 * Output: NONE
 * Return: gf_error_t
 */
gf_error_t gf_hal_test_prior_cancel(void *dev)
{
    gf_error_t err = GF_ERROR_TEST_NOT_SUPPORTED;
    FUNC_ENTER();
    UNUSED_VAR(dev);
    FUNC_EXIT(err);
    return err;
}

/**
 * Function: hal_milan_a_series_irq_test
 * Description: stub code.
 * Input: NONE
 * Output: NONE
 * Return: void
 */
void hal_milan_a_series_irq_test(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();
    UNUSED_VAR(cmd);
    UNUSED_VAR(err_code);
    VOID_FUNC_EXIT();
}

/**
 * Function: gf_hal_milan_irq_test
 * Description: stub code.
 * Input: NONE
 * Output: NONE
 * Return: void
 */
void gf_hal_milan_irq_test(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();
    UNUSED_VAR(cmd);
    UNUSED_VAR(err_code);
    VOID_FUNC_EXIT();
}

/**
 * Function: hal_oswego_irq_test
 * Description: stub code.
 * Input: NONE
 * Output: NONE
 * Return: void
 */
void hal_oswego_irq_test(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();
    UNUSED_VAR(cmd);
    UNUSED_VAR(err_code);
    VOID_FUNC_EXIT();
}

/**
 * Function: hal_milan_an_series_irq_test
 * Description: stub code.
 * Input: NONE
 * Output: NONE
 * Return: void
 */
void hal_milan_an_series_irq_test(gf_irq_t *cmd, gf_error_t *err_code)
{
    VOID_FUNC_ENTER();
    UNUSED_VAR(cmd);
    UNUSED_VAR(err_code);
    VOID_FUNC_EXIT();
}

/**
 * Function: hal_notify_test_fpc_detected
 * Description: stub code.
 * Input: NONE
 * Output: NONE
 * Return: void
 */
void hal_notify_test_fpc_detected(gf_error_t err, uint32_t  finger_event, uint32_t status)
{
    VOID_FUNC_ENTER();
    UNUSED_VAR(err);
    UNUSED_VAR(finger_event);
    UNUSED_VAR(status);
    VOID_FUNC_EXIT();
}

/**
 * Function: hal_notify_test_fpc_reset_fwcfg
 * Description: stub code.
 * Input: NONE
 * Output: NONE
 * Return: void
 */
void hal_notify_test_fpc_reset_fwcfg(gf_error_t err)
{
    UNUSED_VAR(err);
}
