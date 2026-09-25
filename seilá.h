#pragma once

#include "quantum.h"
#include "ch.h"
#include "hal.h"
#include "rgblight.h"

/*
 * ================================================================
 * RGB PWM HARDWARE
 * ================================================================
 *
 * RP2040:
 *
 * GP22 -> PWM3A -> VERMELHO
 * GP26 -> PWM5A -> VERDE
 * GP28 -> PWM6A -> AZUL
 *
 * Os três estão em slices PWM diferentes.
 *
 * Frequência:
 *
 *     1 MHz / 256 = ~3906 Hz
 *
 * ================================================================
 */


/*
 * ================================================================
 * PWM
 * ================================================================
 */

#define RGB_PWM_CLOCK       1000000U
#define RGB_PWM_PERIOD      255U


/*
 * ================================================================
 * LIMITE FÍSICO
 * ================================================================
 *
 * 255 = 100%
 *
 * 100 = ~39%
 *
 * Começamos conservadores por causa dos 70 LEDs.
 *
 * Depois de medir a corrente, você pode aumentar.
 * ================================================================
 */

#define RGB_PWM_MAX         100U


/*
 * ================================================================
 * BALANÇO DE BRANCO
 * ================================================================
 *
 * Vermelho = 100%
 * Verde    = 150/255
 * Azul     = 200/255
 *
 * Estes valores são fáceis de alterar depois.
 * ================================================================
 */

#define RGB_PWM_RED_SCALE       255U
#define RGB_PWM_GREEN_SCALE     150U
#define RGB_PWM_BLUE_SCALE      200U


/*
 * ================================================================
 * DRIVERS
 * ================================================================
 */

#define RGB_PWM_RED_DRIVER      (&PWMD3)
#define RGB_PWM_GREEN_DRIVER    (&PWMD5)
#define RGB_PWM_BLUE_DRIVER     (&PWMD6)


#define RGB_PWM_RED_CHANNEL     0
#define RGB_PWM_GREEN_CHANNEL   0
#define RGB_PWM_BLUE_CHANNEL    0


/*
 * ================================================================
 * COR ATUAL DO RGBLIGHT
 * ================================================================
 */

static volatile uint8_t rgb_pwm_r = 0;
static volatile uint8_t rgb_pwm_g = 0;
static volatile uint8_t rgb_pwm_b = 0;

static volatile bool rgb_pwm_dirty = false;


/*
 * ================================================================
 * CONFIGURAÇÃO PWM3
 *
 * GP22 = PWM3A
 * ================================================================
 */

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


/*
 * ================================================================
 * CONFIGURAÇÃO PWM5
 *
 * GP26 = PWM5A
 * ================================================================
 */

static const PWMConfig rgb_pwm_cfg_5 = {
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


/*
 * ================================================================
 * CONFIGURAÇÃO PWM6
 *
 * GP28 = PWM6A
 * ================================================================
 */

static const PWMConfig rgb_pwm_cfg_6 = {
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
 * LIMITE FÍSICO
 * ================================================================
 *
 * Preserva a proporcionalidade do brilho.
 *
 * Exemplo:
 *
 * RGBLIGHT = 255
 * MAX      = 100
 *
 * resultado = 100
 *
 * RGBLIGHT = 128
 * MAX      = 100
 *
 * resultado ~= 50
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
 * RGBLIGHT CUSTOM DRIVER
 * ================================================================
 *
 * O QMK calcula HSV -> RGB.
 *
 * Nós recebemos:
 *
 *     R = 0..255
 *     G = 0..255
 *     B = 0..255
 *
 * e guardamos esses valores.
 * ================================================================
 */

void rgblight_driver_init(void) {
    /*
     * O hardware é inicializado em rgb_pwm_init().
     */
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
    /*
     * Nada aqui.
     *
     * A thread aplica o PWM.
     */
}


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
) {
    (void)arg;

    chRegSetThreadName("rgb_pwm");


    /*
     * ============================================================
     * INICIA OS TRÊS SLICES
     * ============================================================
     */

    pwmStart(
        RGB_PWM_RED_DRIVER,
        &rgb_pwm_cfg_3
    );

    pwmStart(
        RGB_PWM_GREEN_DRIVER,
        &rgb_pwm_cfg_5
    );

    pwmStart(
        RGB_PWM_BLUE_DRIVER,
        &rgb_pwm_cfg_6
    );


    /*
     * ============================================================
     * CONFIGURA OS GPIOs PARA PWM
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
         * NOVA COR RECEBIDA
         * --------------------------------------------------------
         */

        else if (rgb_pwm_dirty) {

            uint8_t r;
            uint8_t g;
            uint8_t b;


            /*
             * Faz uma cópia rápida dos valores.
             */
            chSysLock();

            r = rgb_pwm_r;
            g = rgb_pwm_g;
            b = rgb_pwm_b;

            rgb_pwm_dirty = false;

            chSysUnlock();


            /*
             * ----------------------------------------------------
             * LIMITE GERAL DE INTENSIDADE
             * ----------------------------------------------------
             */

            r = rgb_pwm_limit(r);
            g = rgb_pwm_limit(g);
            b = rgb_pwm_limit(b);


            /*
             * ----------------------------------------------------
             * BALANÇO DE BRANCO
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


            /*
             * ----------------------------------------------------
             * PWM VERMELHO
             * ----------------------------------------------------
             */

            pwmEnableChannel(
                RGB_PWM_RED_DRIVER,
                RGB_PWM_RED_CHANNEL,
                r
            );


            /*
             * ----------------------------------------------------
             * PWM VERDE
             * ----------------------------------------------------
             */

            pwmEnableChannel(
                RGB_PWM_GREEN_DRIVER,
                RGB_PWM_GREEN_CHANNEL,
                g
            );


            /*
             * ----------------------------------------------------
             * PWM AZUL
             * ----------------------------------------------------
             */

            pwmEnableChannel(
                RGB_PWM_BLUE_DRIVER,
                RGB_PWM_BLUE_CHANNEL,
                b
            );
        }


        /*
         * --------------------------------------------------------
         * DEVOLVE A CPU AO CHIBIOS/QMK
         * --------------------------------------------------------
         *
         * Isso é importante.
         *
         * A thread não fica ocupando a CPU.
         *
         * O QMK continua processando:
         *
         * - matriz de teclas
         * - USB
         * - keycodes
         * - layers
         * - timers
         * - RGBLIGHT
         * - etc.
         * --------------------------------------------------------
         */

        chThdSleepMilliseconds(1);
    }
}


/*
 * ================================================================
 * INICIALIZAÇÃO
 * ================================================================
 */

static inline void rgb_pwm_init(void) {

    /*
     * Cria a thread em prioridade abaixo da atividade normal
     * do teclado.
     */
    chThdCreateStatic(
        waRgbPwmThread,
        sizeof(waRgbPwmThread),
        NORMALPRIO - 1,
        RgbPwmThread,
        NULL
    );
}
