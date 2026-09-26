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
 * PHASE SHIFT:
 *
 * RED   ->   0
 * GREEN ->  85
 * BLUE  -> 170
 *
 * Aproximadamente:
 *
 * RED   =   0°
 * GREEN = 120°
 * BLUE  = 240°
 *
 * ================================================================
 */

/* ================================================================
 * RP2040 PWM REGISTERS
 * ================================================================
 *
 * NÃO usamos Pico SDK aqui.
 *
 * Acesso direto aos registradores do RP2040.
 *
 * PWM base:
 *
 *     0x40050000
 *
 * Cada slice ocupa 0x14 bytes:
 *
 *     +0x00 = CSR
 *     +0x04 = DIV
 *     +0x08 = CTR
 *     +0x0C = CC
 *     +0x10 = TOP
 *
 * Registrador global:
 *
 *     +0xA0 = EN
 *
 * ================================================================
 */

#define RGB_PWM_BASE_ADDRESS       0x40050000UL

#define RGB_PWM_SLICE_SIZE         0x14UL

#define RGB_PWM_CSR_OFFSET         0x00UL
#define RGB_PWM_DIV_OFFSET         0x04UL
#define RGB_PWM_CTR_OFFSET         0x08UL
#define RGB_PWM_CC_OFFSET          0x0CUL
#define RGB_PWM_TOP_OFFSET         0x10UL

#define RGB_PWM_EN_OFFSET          0xA0UL

#define RGB_PWM_REG32(address) \
    (*(volatile uint32_t *)(address))

#define RGB_PWM_SLICE_REG(slice, offset) \
    RGB_PWM_REG32( \
        RGB_PWM_BASE_ADDRESS + \
        ((uint32_t)(slice) * RGB_PWM_SLICE_SIZE) + \
        (offset) \
    )

#define RGB_PWM_EN_REG \
    RGB_PWM_REG32( \
        RGB_PWM_BASE_ADDRESS + RGB_PWM_EN_OFFSET \
    )

/* ================================================================
 * DRIVERS
 * ================================================================ */

#define RGB_PWM_RED_DRIVER      (&PWMD3)
#define RGB_PWM_GREEN_DRIVER    (&PWMD5)
#define RGB_PWM_BLUE_DRIVER     (&PWMD6)

/*
 * pwmEnableChannel() usa índice começando em 0.
 *
 * CHANNEL 0 = PWM A
 */
#define RGB_PWM_RED_CHANNEL     0
#define RGB_PWM_GREEN_CHANNEL   0
#define RGB_PWM_BLUE_CHANNEL    0

/* ================================================================
 * PWM SLICES
 * ================================================================ */

#define RGB_PWM_RED_SLICE       3U
#define RGB_PWM_GREEN_SLICE     5U
#define RGB_PWM_BLUE_SLICE      6U

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
 * PHASE SHIFT
 * ================================================================ */

#define RGB_PWM_PHASE_RED       0U
#define RGB_PWM_PHASE_GREEN     85U
#define RGB_PWM_PHASE_BLUE      170U

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
 * PHASE SHIFT
 * ================================================================ */

static inline void rgb_pwm_apply_phase(void) {

    /*
     * Máscara dos três slices usados pelo RGB:
     *
     * PWM3 = bit 3
     * PWM5 = bit 5
     * PWM6 = bit 6
     */

    const uint32_t rgb_pwm_mask =
        (1U << RGB_PWM_RED_SLICE) |
        (1U << RGB_PWM_GREEN_SLICE) |
        (1U << RGB_PWM_BLUE_SLICE);

    /*
     * ------------------------------------------------------------
     * 1. DESABILITA SOMENTE OS TRÊS SLICES RGB
     * ------------------------------------------------------------
     *
     * Preserva os demais PWM do sistema.
     */

    RGB_PWM_EN_REG &= ~rgb_pwm_mask;

    /*
     * ------------------------------------------------------------
     * 2. POSICIONA OS CONTADORES
     * ------------------------------------------------------------
     */

    RGB_PWM_SLICE_REG(
        RGB_PWM_RED_SLICE,
        RGB_PWM_CTR_OFFSET
    ) = RGB_PWM_PHASE_RED;

    RGB_PWM_SLICE_REG(
        RGB_PWM_GREEN_SLICE,
        RGB_PWM_CTR_OFFSET
    ) = RGB_PWM_PHASE_GREEN;

    RGB_PWM_SLICE_REG(
        RGB_PWM_BLUE_SLICE,
        RGB_PWM_CTR_OFFSET
    ) = RGB_PWM_PHASE_BLUE;

    /*
     * ------------------------------------------------------------
     * 3. HABILITA NOVAMENTE OS TRÊS
     * ------------------------------------------------------------
     */

    RGB_PWM_EN_REG |= rgb_pwm_mask;
}

/* ================================================================
 * DEBUG PHASE
 * ================================================================ */

static inline void rgb_pwm_debug_phase(void) {

    uprintf(
        "RGBDBG PHASE: R=%u G=%u B=%u\n",
        (unsigned)RGB_PWM_PHASE_RED,
        (unsigned)RGB_PWM_PHASE_GREEN,
        (unsigned)RGB_PWM_PHASE_BLUE
    );

    uprintf(
        "RGBDBG SLICES: R=%u G=%u B=%u\n",
        (unsigned)RGB_PWM_RED_SLICE,
        (unsigned)RGB_PWM_GREEN_SLICE,
        (unsigned)RGB_PWM_BLUE_SLICE
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

    uprintf(
        "RGBDBG PWM WRITE: R=%u G=%u B=%u\n",
        r,
        g,
        b
    );

    /*
     * SOMENTE altera o duty.
     *
     * NÃO altera os contadores.
     *
     * Assim o phase shift continua intacto.
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

    rgb_pwm_write(
        0,
        0,
        0
    );

    rgb_pwm_r = 0;
    rgb_pwm_g = 0;
    rgb_pwm_b = 0;

    uprintf(
        "RGBDBG INIT 08 PWM ZERO OK\n"
    );

    /* ============================================================
     * PHASE SHIFT
     * ============================================================
     */

    rgb_pwm_debug_phase();

    rgb_pwm_apply_phase();

    uprintf(
        "RGBDBG INIT 09 PHASE SHIFT OK\n"
    );

    uprintf(
        "RGBDBG INIT 10 COMPLETE\n"
    );
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

    uprintf(
        "RGBDBG SET_COLOR DONE\n"
    );
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

    uprintf(
        "RGBDBG SET_COLOR_ALL DONE\n"
    );
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
     * O PWM é atualizado imediatamente.
     *
     * A fase continua sendo mantida pelo hardware.
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
