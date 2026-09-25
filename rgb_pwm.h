#pragma once

/*
 * ================================================================
 * INCLUDES
 * ================================================================
 */

#include "quantum.h"
#include "ch.h"
#include "hal.h"
#include "rgblight.h"

/*
 * API de baixo nível do PWM do RP2040.
 *
 * Usada somente para o phase shifting.
 */

#include "hardware/pwm.h"


/*
 * ================================================================
 * CONFIGURAÇÃO GERAL
 * ================================================================
 */


/*
 * ================================================================
 * PHASE SHIFT
 * ================================================================
 *
 * 0 = PWM normal
 *
 * 1 = tentativa de phase shifting:
 *
 *     R =   0°
 *     G = 120°
 *     B = 240°
 *
 * Comece testando com 0.
 *
 * Depois altere para 1.
 */

#define RGB_PWM_PHASE_SHIFT_ENABLE 0


/*
 * ================================================================
 * CLOCK E PERÍODO
 * ================================================================
 *
 * 1 MHz / 256
 *
 * ≈ 3906 Hz
 */

#define RGB_PWM_CLOCK   1000000U
#define RGB_PWM_PERIOD  255U


/*
 * ================================================================
 * LIMITE ELÉTRICO
 * ================================================================
 *
 * 255 = 100%
 *
 * 100 ≈ 39%
 *
 * Esse valor limita o duty máximo enviado aos LEDs.
 *
 * IMPORTANTE:
 *
 * Isso NÃO é uma garantia de corrente segura.
 * É apenas um limite conservador inicial.
 */

#define RGB_PWM_MAX 100U


/*
 * ================================================================
 * BALANÇO DE BRANCO
 * ================================================================
 *
 * Cada canal pode ter uma escala diferente.
 *
 * Vermelho = 255/255
 * Verde    = 150/255
 * Azul     = 200/255
 */

#define RGB_PWM_RED_SCALE    255U
#define RGB_PWM_GREEN_SCALE  150U
#define RGB_PWM_BLUE_SCALE   200U


/*
 * ================================================================
 * PINOS / SLICES
 * ================================================================
 *
 * RP2040:
 *
 * GP22 -> PWM3A
 * GP26 -> PWM5A
 * GP28 -> PWM6A
 */

#define RGB_PWM_RED_SLICE    3U
#define RGB_PWM_GREEN_SLICE  5U
#define RGB_PWM_BLUE_SLICE   6U


/*
 * Drivers ChibiOS.
 */

#define RGB_PWM_RED_DRIVER    (&PWMD3)
#define RGB_PWM_GREEN_DRIVER  (&PWMD5)
#define RGB_PWM_BLUE_DRIVER   (&PWMD6)


/*
 * Todos usamos canal A.
 */

#define RGB_PWM_RED_CHANNEL    0
#define RGB_PWM_GREEN_CHANNEL  0
#define RGB_PWM_BLUE_CHANNEL   0


/*
 * ================================================================
 * MÁSCARA DOS SLICES
 * ================================================================
 */

#define RGB_PWM_SLICE_MASK \
    ((1U << RGB_PWM_RED_SLICE)   | \
     (1U << RGB_PWM_GREEN_SLICE) | \
     (1U << RGB_PWM_BLUE_SLICE))


/*
 * ================================================================
 * FASES
 * ================================================================
 *
 * Período = 256 contagens.
 *
 * 256 / 3 ≈ 85,33
 *
 * Portanto usamos aproximadamente:
 *
 * R =   0°
 * G = 120°
 * B = 240°
 *
 * A discretização em 8 bits impede que seja exatamente
 * 120°/240°.
 */

#define RGB_PWM_PHASE_R 0U
#define RGB_PWM_PHASE_G 85U
#define RGB_PWM_PHASE_B 170U


/*
 * ================================================================
 * CORES ATUAIS
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
 * GP22 -> PWM3A
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
 * GP26 -> PWM5A
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
 * GP28 -> PWM6A
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
 * ESCALA DE CANAL
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
 *
 * Mantém a proporcionalidade.
 *
 * RGB = 255
 * MAX = 100
 *
 * resultado = 100
 *
 * RGB = 128
 * MAX = 100
 *
 * resultado ≈ 50
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
 * CONVERSÃO DE FASE
 * ================================================================
 *
 * Queremos deslocar a posição do pulso dentro do período.
 *
 * A posição inicial do contador é escolhida de acordo com
 * a fase desejada.
 */

static inline uint16_t rgb_pwm_counter_for_phase(
    uint16_t phase
) {
    if (phase == 0U) {
        return 0U;
    }

    return (uint16_t)(
        (RGB_PWM_PERIOD + 1U) - phase
    );
}


/*
 * ================================================================
 * RGBLIGHT DRIVER
 * ================================================================
 *
 * O RGBLIGHT fornece:
 *
 *     R = 0..255
 *     G = 0..255
 *     B = 0..255
 *
 * Nós apenas guardamos esses valores.
 */


/*
 * Inicialização exigida pelo driver customizado.
 */

void rgblight_driver_init(void) {
    /*
     * A inicialização real dos PWM ocorre na thread.
     */
}


/*
 * Define uma cor.
 */

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


/*
 * Define a cor de todos os LEDs.
 *
 * Para o nosso hardware existem três linhas PWM,
 * portanto tratamos como uma única saída RGB.
 */

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


/*
 * Flush.
 *
 * Não precisamos aplicar o PWM aqui.
 * A thread faz isso.
 */

void rgblight_driver_flush(void) {
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
     * INICIA OS TRÊS DRIVERS
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
     * GPIOs
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
     * CONFIGURAÇÃO INICIAL DOS CONTADORES
     * ============================================================
     */

#if RGB_PWM_PHASE_SHIFT_ENABLE

    /*
     * ------------------------------------------------------------
     * COM PHASE SHIFT
     * ------------------------------------------------------------
     *
     * R = 0°
     * G = 120°
     * B = 240°
     *
     * Os três slices são preparados antes de serem habilitados.
     */

    pwm_set_mask_enabled(0);


    /*
     * Vermelho
     */

    pwm_set_counter(
        RGB_PWM_RED_SLICE,
        rgb_pwm_counter_for_phase(
            RGB_PWM_PHASE_R
        )
    );


    /*
     * Verde
     */

    pwm_set_counter(
        RGB_PWM_GREEN_SLICE,
        rgb_pwm_counter_for_phase(
            RGB_PWM_PHASE_G
        )
    );


    /*
     * Azul
     */

    pwm_set_counter(
        RGB_PWM_BLUE_SLICE,
        rgb_pwm_counter_for_phase(
            RGB_PWM_PHASE_B
        )
    );


    /*
     * Liga os três slices juntos.
     */

    pwm_set_mask_enabled(
        RGB_PWM_SLICE_MASK
    );

#else

    /*
     * ------------------------------------------------------------
     * SEM PHASE SHIFT
     * ------------------------------------------------------------
     */

    pwm_set_mask_enabled(0);


    pwm_set_counter(
        RGB_PWM_RED_SLICE,
        0
    );

    pwm_set_counter(
        RGB_PWM_GREEN_SLICE,
        0
    );

    pwm_set_counter(
        RGB_PWM_BLUE_SLICE,
        0
    );


    /*
     * Liga os três juntos.
     */

    pwm_set_mask_enabled(
        RGB_PWM_SLICE_MASK
    );

#endif


    /*
     * ============================================================
     * COMEÇA COM OS LEDs DESLIGADOS
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
     * LOOP PRINCIPAL
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


            /*
             * Copia os três valores juntos.
             */

            chSysLock();

            r = rgb_pwm_r;
            g = rgb_pwm_g;
            b = rgb_pwm_b;

            rgb_pwm_dirty = false;

            chSysUnlock();


            /*
             * ----------------------------------------------------
             * LIMITE GERAL
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
             * VERMELHO
             * ----------------------------------------------------
             */

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

            pwmEnableChannel(
                RGB_PWM_BLUE_DRIVER,
                RGB_PWM_BLUE_CHANNEL,
                b
            );
        }


        /*
         * --------------------------------------------------------
         * NÃO PRENDE A CPU
         * --------------------------------------------------------
         *
         * O PWM continua sendo produzido pelo hardware.
         *
         * A thread apenas atualiza os duty cycles.
         */

        chThdSleepMilliseconds(1);
    }
}


/*
 * ================================================================
 * INICIALIZAÇÃO DO DRIVER
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
