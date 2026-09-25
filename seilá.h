#pragma once

#include "quantum.h"
#include "ch.h"
#include "hal.h"
#include "rgblight.h"

/*
 * ================================================================
 * RGB PWM HARDWARE - RP2040 / ChibiOS
 *
 * GP22 -> PWM3A -> VERMELHO
 * GP26 -> PWM5A -> VERDE
 * GP28 -> PWM6A -> AZUL
 *
 * Todos os canais ficam em slices PWM independentes.
 * ================================================================
 */


/* ================================================================
 * CONFIGURAÇÃO GERAL
 * ================================================================
 */

/*
 * Clock do contador PWM.
 *
 * 1 MHz / 256 ~= 3906 Hz
 */
#define RGB_PWM_CLOCK       1000000U
#define RGB_PWM_PERIOD      255U


/*
 * Intensidade máxima geral.
 *
 * 255 = 100%
 *
 * Começamos conservadores por causa dos 70 LEDs.
 * Depois você pode aumentar.
 */
#define RGB_PWM_MAX         100U


/*
 * Correção individual dos canais.
 *
 * Vermelho: 100%
 * Verde:    ~59%
 * Azul:     ~78%
 */
#define RGB_PWM_RED_SCALE   255U
#define RGB_PWM_GREEN_SCALE 150U
#define RGB_PWM_BLUE_SCALE  200U


/*
 * Defasagem.
 *
 * 0     = vermelho
 * 85    = ~120 graus
 * 170   = ~240 graus
 *
 * Como o período é 255 contagens:
 *
 * 255 / 3 ~= 85
 */
#define RGB_PWM_PHASE_R     0U
#define RGB_PWM_PHASE_G     85U
#define RGB_PWM_PHASE_B     170U


/* ================================================================
 * DRIVERS
 * ================================================================
 */

#define RGB_PWM_RED_DRIVER      (&PWMD3)
#define RGB_PWM_GREEN_DRIVER    (&PWMD5)
#define RGB_PWM_BLUE_DRIVER     (&PWMD6)


#define RGB_PWM_RED_CHANNEL     0
#define RGB_PWM_GREEN_CHANNEL   0
#define RGB_PWM_BLUE_CHANNEL    0


/* ================================================================
 * CORES RECEBIDAS DO RGBLIGHT
 * ================================================================
 */

static volatile uint8_t rgb_pwm_r = 0;
static volatile uint8_t rgb_pwm_g = 0;
static volatile uint8_t rgb_pwm_b = 0;

static volatile bool rgb_pwm_dirty = false;


/* ================================================================
 * CONFIGURAÇÃO DOS PWM
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


/* ================================================================
 * ESCALA
 * ================================================================
 */

static inline uint8_t rgb_pwm_scale(
    uint8_t value,
    uint8_t scale
) {
    return (uint8_t)(((uint16_t)value * scale) / 255U);
}


/*
 * Aplica o limite geral de intensidade.
 */
static inline uint8_t rgb_pwm_limit(
    uint8_t value
) {
    return (uint8_t)(
        ((uint16_t)value * RGB_PWM_MAX) / 255U
    );
}


/* ================================================================
 * RGBLIGHT -> NOSSO DRIVER
 * ================================================================
 *
 * O RGBLIGHT fornece R/G/B como uint8_t.
 *
 * Aqui NÃO fazemos HSV.
 *
 * O QMK já fez:
 *
 * HSV -> RGB
 *
 * Nós recebemos:
 *
 * R 0..255
 * G 0..255
 * B 0..255
 */

void rgblight_driver_init(void) {
    /*
     * Nada aqui.
     *
     * O hardware será iniciado por rgb_pwm_init().
     */
}


void rgblight_driver_set_color(
    int index,
    uint8_t r,
    uint8_t g,
    uint8_t b
) {
    (void)index;

    /*
     * Como nosso hardware possui apenas um conjunto
     * físico R/G/B, não precisamos armazenar cada LED
     * individualmente.
     *
     * Pegamos a cor calculada pelo RGBLIGHT.
     */
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
     * O PWM é atualizado pela thread.
     */
}


/* ================================================================
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
     * ------------------------------------------------------------
     * Inicializa os três slices PWM.
     * ------------------------------------------------------------
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
     * ------------------------------------------------------------
     * Pinos em função PWM.
     *
     * RP2040:
     *
     * GP22 = PWM3A
     * GP26 = PWM5A
     * GP28 = PWM6A
     * ------------------------------------------------------------
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
     * ------------------------------------------------------------
     * Começa desligado.
     * ------------------------------------------------------------
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
     * ------------------------------------------------------------
     * LOOP
     * ------------------------------------------------------------
     */

    while (true) {

        /*
         * RGBLIGHT desligado:
         *
         * força tudo para zero.
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

        } else if (rgb_pwm_dirty) {

            uint8_t r;
            uint8_t g;
            uint8_t b;


            /*
             * Copia os valores produzidos pelo RGBLIGHT.
             */
            r = rgb_pwm_r;
            g = rgb_pwm_g;
            b = rgb_pwm_b;


            rgb_pwm_dirty = false;


            /*
             * ----------------------------------------------------
             * LIMITADOR GERAL
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
             * APLICA DUTY
             * ----------------------------------------------------
             */

            pwmEnableChannel(
                RGB_PWM_RED_DRIVER,
                RGB_PWM_RED_CHANNEL,
                r
            );

            pwmEnableChannel(
                RGB_PWM_GREEN_DRIVER,
                RGB_PWM_GREEN_CHANNEL,
                g
            );

            pwmEnableChannel(
                RGB_PWM_BLUE_DRIVER,
                RGB_PWM_BLUE_CHANNEL,
                b
            );
        }


        /*
         * Muito importante:
         *
         * Essa thread NÃO fica monopolizando a CPU.
         *
         * O QMK continua processando:
         *
         * teclas
         * USB
         * layers
         * timers
         * RGBLIGHT
         * etc.
         */
        chThdSleepMilliseconds(1);
    }
}


/* ================================================================
 * INICIALIZAÇÃO PÚBLICA
 * ================================================================
 */

static inline void rgb_pwm_init(void) {

    chThdCreateStatic(
        waRgbPwmThread,
        sizeof(waRgbPwmThread),
        NORMALPRIO - 1,
        RgbPwmThread,
        NULL
    );
}
