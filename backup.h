#pragma once

#include "quantum.h"
#include "rgblight.h"
#include "rgblight_drivers.h"
#include "ch.h"
#include "hal.h"

/*
 * ================================================================
 * PWM RP2040 VIA CHIBIOS
 * ================================================================
 *
 * GP22 -> PWM3A
 * GP26 -> PWM5A
 * GP28 -> PWM6A
 */

#define RGB_PWM_RED_DRIVER    (&PWMD3)
#define RGB_PWM_GREEN_DRIVER  (&PWMD5)
#define RGB_PWM_BLUE_DRIVER   (&PWMD6)

#define RGB_PWM_RED_CHANNEL    RP2040_PWM_CHANNEL_A
#define RGB_PWM_GREEN_CHANNEL  RP2040_PWM_CHANNEL_A
#define RGB_PWM_BLUE_CHANNEL   RP2040_PWM_CHANNEL_A

/*
 * ================================================================
 * PWM
 * ================================================================
 */

#define RGB_PWM_CLOCK   1000000U
#define RGB_PWM_PERIOD  255U

/*
 * ================================================================
 * LIMITE DE DUTY
 * ================================================================
 */

#define RGB_PWM_MAX 100U

/*
 * ================================================================
 * BALANÇO DE BRANCO
 * ================================================================
 */

#define RGB_PWM_RED_SCALE    255U
#define RGB_PWM_GREEN_SCALE  150U
#define RGB_PWM_BLUE_SCALE   200U

/*
 * ================================================================
 * ESTADO RGB
 * ================================================================
 */

static volatile uint8_t rgb_pwm_r = 0;
static volatile uint8_t rgb_pwm_g = 0;
static volatile uint8_t rgb_pwm_b = 0;

static volatile bool rgb_pwm_dirty = false;

/*
 * ================================================================
 * CONFIGURAÇÃO PWM3
 * ================================================================
 */

static const PWMConfig rgb_pwm_cfg_3 = {
    RGB_PWM_CLOCK,
    RGB_PWM_PERIOD,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL}
    }
};

/*
 * ================================================================
 * CONFIGURAÇÃO PWM5
 * ================================================================
 */

static const PWMConfig rgb_pwm_cfg_5 = {
    RGB_PWM_CLOCK,
    RGB_PWM_PERIOD,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL}
    }
};

/*
 * ================================================================
 * CONFIGURAÇÃO PWM6
 * ================================================================
 */

static const PWMConfig rgb_pwm_cfg_6 = {
    RGB_PWM_CLOCK,
    RGB_PWM_PERIOD,
    NULL,
    {
        {PWM_OUTPUT_ACTIVE_HIGH, NULL},
        {PWM_OUTPUT_DISABLED, NULL}
    }
};

/*
 * ================================================================
 * THREAD
 * ================================================================
 */

static THD_WORKING_AREA(
    waRgbPwmThread,
    256
);

static THD_FUNCTION(
    RgbPwmThread,
    arg
);

/*
 * ================================================================
 * PROTÓTIPO
 * ================================================================
 */

static inline void rgb_pwm_init(void);

/*
 * ================================================================
 * ESCALA
 * ================================================================
 */

static inline uint8_t rgb_pwm_scale(
    uint8_t value,
    uint8_t scale
) {
    return (uint8_t)(
        ((uint16_t)value * scale) / 255U
    );
}

/*
 * ================================================================
 * LIMITE DE DUTY
 * ================================================================
 */

static inline uint8_t rgb_pwm_limit(
    uint8_t value
) {
    return (uint8_t)(
        ((uint16_t)value * RGB_PWM_MAX) / 255U
    );
}

/*
 * ================================================================
 * RGBLIGHT DRIVER INIT
 * ================================================================
 */

void rgblight_driver_init(void) {

    dprintf("RGB INIT: driver_init\n");

    rgb_pwm_init();

    dprintf("RGB INIT: rgb_pwm_init retornou\n");
}

/*
 * ================================================================
 * DEFINE UMA COR
 * ================================================================
 */

void rgblight_driver_set_color(
    int index,
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    dprintf(
        "RGB SET: index=%d R=%u G=%u B=%u\n",
        index,
        r,
        g,
        b
    );

    (void)index;

    rgb_pwm_r = r;
    rgb_pwm_g = g;
    rgb_pwm_b = b;

    rgb_pwm_dirty = true;

    dprintf("RGB SET: dirty=1\n");
}

/*
 * ================================================================
 * DEFINE COR DE TODOS OS LEDS
 * ================================================================
 */

void rgblight_driver_set_color_all(
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    dprintf(
        "RGB ALL: R=%u G=%u B=%u\n",
        r,
        g,
        b
    );

    rgb_pwm_r = r;
    rgb_pwm_g = g;
    rgb_pwm_b = b;

    rgb_pwm_dirty = true;

    dprintf("RGB ALL: dirty=1\n");
}

/*
 * ================================================================
 * FLUSH
 * ================================================================
 */

void rgblight_driver_flush(void) {

    dprintf("RGB FLUSH\n");

    rgb_pwm_dirty = true;
}

/*
 * ================================================================
 * THREAD
 * ================================================================
 */

static THD_FUNCTION(
    RgbPwmThread,
    arg
) {
    (void)arg;

    chRegSetThreadName("rgb_pwm");

    dprintf("RGB THREAD: iniciou\n");

    /*
     * ============================================================
     * INICIA OS PWM
     * ============================================================
     */

    pwmStart(
        RGB_PWM_RED_DRIVER,
        &rgb_pwm_cfg_3
    );

    dprintf("RGB THREAD: PWM3 iniciado\n");

    pwmStart(
        RGB_PWM_GREEN_DRIVER,
        &rgb_pwm_cfg_5
    );

    dprintf("RGB THREAD: PWM5 iniciado\n");

    pwmStart(
        RGB_PWM_BLUE_DRIVER,
        &rgb_pwm_cfg_6
    );

    dprintf("RGB THREAD: PWM6 iniciado\n");

    /*
     * ============================================================
     * GPIO
     * ============================================================
     */

    palSetLineMode(
        GP22,
        PAL_MODE_ALTERNATE_PWM
    );

    palSetLineMode(
        GP26,
        PAL_MODE_ALTERNATE_PWM
    );

    palSetLineMode(
        GP28,
        PAL_MODE_ALTERNATE_PWM
    );

    dprintf("RGB THREAD: GPIO PWM configurados\n");

    /*
     * ============================================================
     * COMEÇA DESLIGADO
     * ============================================================
     */

    pwmEnableChannel(
        RGB_PWM_RED_DRIVER,
        RGB_PWM_RED_CHANNEL,
        0
    );

    pwmEnableChannel(
        RGB_PWM_GREEN_DRIVER,
        RGB_PWM_GREEN_CHANNEL,
        0
    );

    pwmEnableChannel(
        RGB_PWM_BLUE_DRIVER,
        RGB_PWM_BLUE_CHANNEL,
        0
    );

    dprintf("RGB THREAD: PWM inicializados em 0\n");

    /*
     * ============================================================
     * LOOP
     * ============================================================
     */

    while (true) {

        /*
         * --------------------------------------------------------
         * RGBLIGHT DESLIGADO
         * --------------------------------------------------------
         */

        if (!rgblight_is_enabled()) {

            pwmEnableChannel(
                RGB_PWM_RED_DRIVER,
                RGB_PWM_RED_CHANNEL,
                0
            );

            pwmEnableChannel(
                RGB_PWM_GREEN_DRIVER,
                RGB_PWM_GREEN_CHANNEL,
                0
            );

            pwmEnableChannel(
                RGB_PWM_BLUE_DRIVER,
                RGB_PWM_BLUE_CHANNEL,
                0
            );
        }

        /*
         * --------------------------------------------------------
         * NOVA COR
         * --------------------------------------------------------
         */

        else if (rgb_pwm_dirty) {

            uint8_t r;
            uint8_t g;
            uint8_t b;

            dprintf("RGB THREAD: dirty detectado\n");

            /*
             * ----------------------------------------------------
             * COPIA ESTADO
             * ----------------------------------------------------
             */

            chSysLock();

            r = rgb_pwm_r;
            g = rgb_pwm_g;
            b = rgb_pwm_b;

            rgb_pwm_dirty = false;

            chSysUnlock();

            dprintf(
                "RGB THREAD: estado R=%u G=%u B=%u\n",
                r,
                g,
                b
            );

            /*
             * ----------------------------------------------------
             * LIMITE GERAL
             * ----------------------------------------------------
             */

            r = rgb_pwm_limit(r);
            g = rgb_pwm_limit(g);
            b = rgb_pwm_limit(b);

            dprintf(
                "RGB THREAD: limite R=%u G=%u B=%u\n",
                r,
                g,
                b
            );

            /*
             * ----------------------------------------------------
             * WHITE BALANCE
             * ----------------------------------------------------
             */

            r = rgb_pwm_scale(
                r,
                RGB_PWM_RED_SCALE
            );

            g = rgb_pwm_scale(
                g,
                RGB_PWM_GREEN_SCALE
            );

            b = rgb_pwm_scale(
                b,
                RGB_PWM_BLUE_SCALE
            );

            dprintf(
                "RGB THREAD: final PWM R=%u G=%u B=%u\n",
                r,
                g,
                b
            );

            /*
             * ----------------------------------------------------
             * VERMELHO
             * ----------------------------------------------------
             */

            dprintf(
                "RGB PWM: aplicando R=%u\n",
                r
            );

            pwmEnableChannel(
                RGB_PWM_RED_DRIVER,
                RGB_PWM_RED_CHANNEL,
                r
            );

            /*
             * ----------------------------------------------------
             * VERDE
             * ----------------------------------------------------
             */

            dprintf(
                "RGB PWM: aplicando G=%u\n",
                g
            );

            pwmEnableChannel(
                RGB_PWM_GREEN_DRIVER,
                RGB_PWM_GREEN_CHANNEL,
                g
            );

            /*
             * ----------------------------------------------------
             * AZUL
             * ----------------------------------------------------
             */

            dprintf(
                "RGB PWM: aplicando B=%u\n",
                b
            );

            pwmEnableChannel(
                RGB_PWM_BLUE_DRIVER,
                RGB_PWM_BLUE_CHANNEL,
                b
            );

            dprintf("RGB PWM: aplicação concluída\n");
        }

        /*
         * --------------------------------------------------------
         * DEIXA O HARDWARE PWM TRABALHAR
         * --------------------------------------------------------
         */

        chThdSleepMilliseconds(1);
    }
}

/*
 * ================================================================
 * INICIALIZAÇÃO DA THREAD
 * ================================================================
 */

static inline void rgb_pwm_init(void) {

    dprintf("RGB INIT: criando thread\n");

    chThdCreateStatic(
        waRgbPwmThread,
        sizeof(waRgbPwmThread),
        NORMALPRIO - 1,
        RgbPwmThread,
        NULL
    );

    dprintf("RGB INIT: thread criada\n");
}

/*
 * ================================================================
 * QMK RGBLIGHT DRIVER
 * ================================================================
 */

const rgblight_driver_t rgblight_driver = {
    .init = rgblight_driver_init,
    .set_color = rgblight_driver_set_color,
    .set_color_all = rgblight_driver_set_color_all,
    .flush = rgblight_driver_flush,
};
