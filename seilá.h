#pragma once

#include "quantum.h"
#include "rgblight.h"
#include "rgblight_drivers.h"
#include "hal.h"

/*
 * ================================================================
 * RGB PWM - RP2040 / ChibiOS
 * ================================================================
 *
 * GP22 -> PWM3A -> RED
 * GP26 -> PWM5A -> GREEN
 * GP28 -> PWM6A -> BLUE
 *
 * PWM clock  = 1 MHz
 * PWM period = 255
 *
 * duty 0   = 0%
 * duty 255 = 100%
 * ================================================================
 */

/* ================================================================
 * DRIVERS
 * ================================================================ */

#define RGB_PWM_RED_DRIVER      (&PWMD3)
#define RGB_PWM_GREEN_DRIVER    (&PWMD5)
#define RGB_PWM_BLUE_DRIVER     (&PWMD6)

#define RGB_PWM_RED_CHANNEL     RP2040_PWM_CHANNEL_A
#define RGB_PWM_GREEN_CHANNEL   RP2040_PWM_CHANNEL_A
#define RGB_PWM_BLUE_CHANNEL    RP2040_PWM_CHANNEL_A

/* ================================================================
 * PIN MODE
 * ================================================================ */

#define RGB_PWM_PAL_MODE \
    (PAL_MODE_ALTERNATE_PWM | PAL_RP_PAD_DRIVE12 | PAL_RP_GPIO_OE)

/* ================================================================
 * PWM
 * ================================================================ */

#define RGB_PWM_CLOCK   1000000U
#define RGB_PWM_PERIOD  255U

/* ================================================================
 * WHITE BALANCE
 * ================================================================ */

#define RGB_PWM_RED_SCALE    255U
#define RGB_PWM_GREEN_SCALE  255U
#define RGB_PWM_BLUE_SCALE   255U

/* ================================================================
 * PWM CONFIG
 * ================================================================ */

static const PWMConfig rgb_pwm_cfg_3 = {
    .frequency = RGB_PWM_CLOCK,
    .period    = RGB_PWM_PERIOD,
    .callback  = NULL,
    .channels  = {
        [0] = {
            .mode     = PWM_OUTPUT_ACTIVE_HIGH,
            .callback = NULL
        },
        [1] = {
            .mode     = PWM_OUTPUT_DISABLED,
            .callback = NULL
        }
    }
};

static const PWMConfig rgb_pwm_cfg_5 = {
    .frequency = RGB_PWM_CLOCK,
    .period    = RGB_PWM_PERIOD,
    .callback  = NULL,
    .channels  = {
        [0] = {
            .mode     = PWM_OUTPUT_ACTIVE_HIGH,
            .callback = NULL
        },
        [1] = {
            .mode     = PWM_OUTPUT_DISABLED,
            .callback = NULL
        }
    }
};

static const PWMConfig rgb_pwm_cfg_6 = {
    .frequency = RGB_PWM_CLOCK,
    .period    = RGB_PWM_PERIOD,
    .callback  = NULL,
    .channels  = {
        [0] = {
            .mode     = PWM_OUTPUT_ACTIVE_HIGH,
            .callback = NULL
        },
        [1] = {
            .mode     = PWM_OUTPUT_DISABLED,
            .callback = NULL
        }
    }
};

/* ================================================================
 * ESTADO
 * ================================================================ */

static uint8_t rgb_pwm_r = 0;
static uint8_t rgb_pwm_g = 0;
static uint8_t rgb_pwm_b = 0;

/* ================================================================
 * SCALE
 * ================================================================ */

static inline uint8_t rgb_pwm_scale(
    uint8_t value,
    uint8_t scale
) {
    return (uint8_t)(
        ((uint16_t)value * scale) / 255U
    );
}

/* ================================================================
 * WRITE PWM
 * ================================================================ */

static inline void rgb_pwm_write(
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    r = rgb_pwm_scale(r, RGB_PWM_RED_SCALE);
    g = rgb_pwm_scale(g, RGB_PWM_GREEN_SCALE);
    b = rgb_pwm_scale(b, RGB_PWM_BLUE_SCALE);

    uprintf(
        "RGBDBG PWM WRITE: R=%u G=%u B=%u\n",
        r,
        g,
        b
    );

    pwmEnableChannel(
        RGB_PWM_RED_DRIVER,
        RGB_PWM_RED_CHANNEL,
        128
    );

    pwmEnableChannel(
        RGB_PWM_GREEN_DRIVER,
        RGB_PWM_GREEN_CHANNEL,
        128
    );

    pwmEnableChannel(
        RGB_PWM_BLUE_DRIVER,
        RGB_PWM_BLUE_CHANNEL,
        128
    );
}

/* ================================================================
 * INIT
 * ================================================================ */

void rgblight_driver_init(void) {

    uprintf("RGBDBG INIT 01\n");

    /* PWM3 */
    pwmStart(
        RGB_PWM_RED_DRIVER,
        &rgb_pwm_cfg_3
    );

    uprintf("RGBDBG INIT 02 PWM3 OK\n");

    /* PWM5 */
    pwmStart(
        RGB_PWM_GREEN_DRIVER,
        &rgb_pwm_cfg_5
    );

    uprintf("RGBDBG INIT 03 PWM5 OK\n");

    /* PWM6 */
    pwmStart(
        RGB_PWM_BLUE_DRIVER,
        &rgb_pwm_cfg_6
    );

    uprintf("RGBDBG INIT 04 PWM6 OK\n");

    /* GPIO -> PWM alternate function */
    palSetLineMode(
        GP22,
        RGB_PWM_PAL_MODE
    );

    uprintf("RGBDBG INIT 05 GP22 OK\n");

    palSetLineMode(
        GP26,
        RGB_PWM_PAL_MODE
    );

    uprintf("RGBDBG INIT 06 GP26 OK\n");

    palSetLineMode(
        GP28,
        RGB_PWM_PAL_MODE
    );

    uprintf("RGBDBG INIT 07 GP28 OK\n");

    /* Começa desligado */
    rgb_pwm_write(0, 0, 0);

    rgb_pwm_r = 0;
    rgb_pwm_g = 0;
    rgb_pwm_b = 0;

    uprintf("RGBDBG INIT 08 PWM ZERO OK\n");
    uprintf("RGBDBG INIT 09 COMPLETE\n");
}

/* ================================================================
 * SET COLOR
 * ================================================================ */

void rgblight_driver_set_color(
    int index,
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    (void)index;

    uprintf(
        "RGBDBG SET_COLOR: R=%u G=%u B=%u\n",
        r,
        g,
        b
    );

    rgb_pwm_r = r;
    rgb_pwm_g = g;
    rgb_pwm_b = b;

    rgb_pwm_write(
        r,
        g,
        b
    );

    uprintf("RGBDBG SET_COLOR DONE\n");
}

/* ================================================================
 * SET COLOR ALL
 * ================================================================ */

void rgblight_driver_set_color_all(
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    uprintf(
        "RGBDBG SET_COLOR_ALL: R=%u G=%u B=%u\n",
        r,
        g,
        b
    );

    rgb_pwm_r = r;
    rgb_pwm_g = g;
    rgb_pwm_b = b;

    rgb_pwm_write(
        r,
        g,
        b
    );

    uprintf("RGBDBG SET_COLOR_ALL DONE\n");
}

/* ================================================================
 * FLUSH
 * ================================================================ */

void rgblight_driver_flush(void) {

    uprintf(
        "RGBDBG FLUSH: R=%u G=%u B=%u\n",
        rgb_pwm_r,
        rgb_pwm_g,
        rgb_pwm_b
    );

    /*
     * Nada a fazer.
     *
     * O PWM é atualizado imediatamente em set_color().
     */
}

/* ================================================================
 * DRIVER
 * ================================================================ */

const rgblight_driver_t rgblight_driver = {
    .init           = rgblight_driver_init,
    .set_color      = rgblight_driver_set_color,
    .set_color_all  = rgblight_driver_set_color_all,
    .flush          = rgblight_driver_flush,
};
