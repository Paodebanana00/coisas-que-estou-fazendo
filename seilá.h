#pragma once

#include "quantum.h"
#include "ch.h"
#include "hal.h"
#include "rgblight.h"

/* ================================================================
 * CONFIGURAÇÃO
 * ================================================================ */

#define RGB_PWM_CLOCK       1000000U
#define RGB_PWM_PERIOD      255U

#define DRIVER_VERMELHO     (&PWMD3)
#define DRIVER_VERDE_AZUL   (&PWMD5)

#define CANAL_VERMELHO      0
#define CANAL_VERDE         0
#define CANAL_AZUL          1

/* ================================================================
 * COR RECEBIDA DO RGBLIGHT
 * ================================================================ */

static volatile uint8_t rgb_pwm_r = 0;
static volatile uint8_t rgb_pwm_g = 0;
static volatile uint8_t rgb_pwm_b = 0;

static volatile bool rgb_pwm_dirty = false;

/* ================================================================
 * PWM CONFIG
 * ================================================================ */

static const PWMConfig rgb_pwm_cfg_3 = {
    RGB_PWM_CLOCK,
    RGB_PWM_PERIOD,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL}
    }
};

static const PWMConfig rgb_pwm_cfg_5 = {
    RGB_PWM_CLOCK,
    RGB_PWM_PERIOD,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL}
    }
};

/* ================================================================
 * RGBLIGHT CUSTOM DRIVER
 * ================================================================ */

void rgblight_driver_init(void) {
}

void rgblight_driver_set_color(
    int index,
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    (void)index;

    rgb_pwm_r = r;
    rgb_pwm_g = g;
    rgb_pwm_b = b;

    rgb_pwm_dirty = true;
}

void rgblight_driver_set_color_all(
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    rgb_pwm_r = r;
    rgb_pwm_g = g;
    rgb_pwm_b = b;

    rgb_pwm_dirty = true;
}

void rgblight_driver_flush(void) {
}

/* ================================================================
 * BALANÇO DE BRANCO
 * ================================================================ */

static inline uint8_t rgb_pwm_scale(
    uint8_t value,
    uint8_t scale
) {
    return ((uint16_t)value * scale) / 255U;
}

/* ================================================================
 * THREAD
 * ================================================================ */

static THD_WORKING_AREA(waRgbPwmThread, 256);

static THD_FUNCTION(RgbPwmThread, arg) {
    (void)arg;

    chRegSetThreadName("rgb_pwm");

    pwmStart(DRIVER_VERMELHO, &rgb_pwm_cfg_3);
    pwmStart(DRIVER_VERDE_AZUL, &rgb_pwm_cfg_5);

    pwmEnableChannel(
        DRIVER_VERMELHO,
        CANAL_VERMELHO,
        0
    );

    pwmEnableChannel(
        DRIVER_VERDE_AZUL,
        CANAL_VERDE,
        0
    );

    pwmEnableChannel(
        DRIVER_VERDE_AZUL,
        CANAL_AZUL,
        0
    );

    while (true) {

        if (!rgblight_is_enabled()) {

            pwmEnableChannel(
                DRIVER_VERMELHO,
                CANAL_VERMELHO,
                0
            );

            pwmEnableChannel(
                DRIVER_VERDE_AZUL,
                CANAL_VERDE,
                0
            );

            pwmEnableChannel(
                DRIVER_VERDE_AZUL,
                CANAL_AZUL,
                0
            );

        } else if (rgb_pwm_dirty) {

            uint8_t r = rgb_pwm_r;
            uint8_t g = rgb_pwm_g;
            uint8_t b = rgb_pwm_b;

            rgb_pwm_dirty = false;

            /*
             * Seu ajuste de balanço de branco.
             */
            g = rgb_pwm_scale(g, 150);
            b = rgb_pwm_scale(b, 200);

            pwmEnableChannel(
                DRIVER_VERMELHO,
                CANAL_VERMELHO,
                r
            );

            pwmEnableChannel(
                DRIVER_VERDE_AZUL,
                CANAL_VERDE,
                g
            );

            pwmEnableChannel(
                DRIVER_VERDE_AZUL,
                CANAL_AZUL,
                b
            );
        }

        chThdSleepMilliseconds(1);
    }
}

/* ================================================================
 * INICIALIZAÇÃO
 * ================================================================ */

static inline void rgb_pwm_init(void) {

    palSetLineMode(
        GP22,
        PAL_MODE_ALTERNATE_PWM
    );

    palSetLineMode(
        GP26,
        PAL_MODE_ALTERNATE_PWM
    );

    palSetLineMode(
        GP27,
        PAL_MODE_ALTERNATE_PWM
    );

    chThdCreateStatic(
        waRgbPwmThread,
        sizeof(waRgbPwmThread),
        NORMALPRIO - 1,
        RgbPwmThread,
        NULL
    );
}
