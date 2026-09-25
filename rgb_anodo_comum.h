#pragma once
#include "ch.h"
#include "hal.h"

// ====================================================================
// CONFIGURAÇÕES DOS TIMERS DE PWM POR HARDWARE (RP2040 / ChibiOS)
// ====================================================================

// Bloco 3 (Vermelho): Polaridade Normal (Liga no início do ciclo)
static const PWMConfig pwmcfg_3 = {
  1000000, /* Clock de 1MHz */
  255,     /* Período de 255 passos (8-bits) */
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}, // Canal 0 (Saída A do Bloco 3 -> GP22)
    {PWM_OUTPUT_DISABLED, NULL}     // Canal 1 Desativado
  }
};

// Bloco 5 (Verde/Azul): Configurado em FASE INVERTIDA (ACTIVE_LOW)
// Distribui a corrente elétrica na USB, pois eles ligam quando o Vermelho desliga!
static const PWMConfig pwmcfg_5 = {
  1000000, /* Clock de 1MHz */
  255,     /* Período de 255 passos (8-bits) */
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_LOW, NULL},  // INVERTIDO: Canal 0 (Saída A do Bloco 5 -> GP26)
    {PWM_OUTPUT_ACTIVE_LOW, NULL}   // INVERTIDO: Canal 1 (Saída B do Bloco 5 -> GP27)
  }
};

#define DRIVER_VERMELHO   &PWMD3
#define CANAL_VERMELHO    0  // GP22

#define DRIVER_VERDE_AZUL &PWMD5
#define CANAL_VERDE       0  // GP26
#define CANAL_AZUL        1  // GP27

// Variáveis globais herdadas do seu keymap.c
extern bool rgb_led_enabled;
extern uint8_t cor_atual;
extern uint8_t brilho_atual;

// Alvos de brilho instantâneos de hardware
static uint8_t target_r = 0;
static uint8_t target_g = 0;
static uint8_t target_b = 0;

// ====================================================================
// ALGORITMO AUXILIAR: CONVERSÃO HSV PARA RGB (Para o modo Arco-Íris)
// ====================================================================
void hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b) {
    uint8_t region, remainder, p, q, t;
    if (s == 0) { *r = *g = *b = v; return; }
    region = h / 43;
    remainder = (h % 43) * 6;
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    switch (region) {
        case 0:  *r = v; *g = t; *b = p; break;
        case 1:  *r = q; *g = v; *b = p; break;
        case 2:  *r = p; *g = v; *b = t; break;
        case 3:  *r = p; *g = q; *b = v; break;
        case 4:  *r = t; *g = p; *b = v; break;
        default: *r = v; *g = p; *b = q; break;
    }
}

// Atualiza a lógica de cores fixas
void aplica_estado_led(void) {
    if (!rgb_led_enabled) {
        target_r = 0; target_g = 0; target_b = 0;
    } else {
        switch (cor_atual) {
            case 0: // Branco
                target_r = brilho_atual; target_g = brilho_atual; target_b = brilho_atual;
                break;
            case 1: // Vermelho
                target_r = brilho_atual; target_g = 0;            target_b = 0;
                break;
            case 2: // Verde
                target_r = 0;            target_g = brilho_atual; target_b = 0;
                break;
            case 3: // Azul
                target_r = 0;            target_g = 0;            target_b = brilho_atual;
                break;
            default:
                break;
        }
    }
}

// ====================================================================
// THREAD DE GERENCIAMENTO COM FILTRO DE PROTEÇÃO VISUAL E ELÉTRICA
// ====================================================================
static THD_WORKING_AREA(waLedThread, 128);
static THD_FUNCTION(LedThread, arg) {
    (void)arg;
    chRegSetThreadName("hardware_pwm_manager");
    
    // Inicializa os blocos elétricos de hardware
    pwmStart(DRIVER_VERMELHO, &pwmcfg_3);
    pwmStart(DRIVER_VERDE_AZUL, &pwmcfg_5);

    uint8_t rainbow_hue = 0;

    while (true) {
        // CORREÇÃO DE SEGURANÇA: Atualiza caso mude para uma cor estática no keymap...
        // ...mas SÓ faz isso se não estiver no Arco-Íris (4), evitando que o efeito pisque!
        if (cor_atual != 4) {
            aplica_estado_led();
        }

        // Modo Arco-Íris (Caso 4) calcula dinamicamente as fusões
        if (rgb_led_enabled && cor_atual == 4) {
            hsv_to_rgb(rainbow_hue, 255, brilho_atual, &target_r, &target_g, &target_b);
            rainbow_hue++;
        }

        // --- ENVIANDO OS VALORES COM PROTEÇÃO DE INVERSÃO DE POLARIDADE ---
        if (!rgb_led_enabled) {
            // Se o LED for desligado, força o estado zero elétrico em todos os pinos.
            // Active High (Vermelho) desliga em 0. Active Low (Verde/Azul) desliga em 255!
            pwmEnableChannel(DRIVER_VERMELHO, CANAL_VERMELHO, 0);
            pwmEnableChannel(DRIVER_VERDE_AZUL, CANAL_VERDE, 255);
            pwmEnableChannel(DRIVER_VERDE_AZUL, CANAL_AZUL, 255);
        } else {
            // Canal Normal (Vermelho): Injeta o brilho direto (0 a 255)
            pwmEnableChannel(DRIVER_VERMELHO, CANAL_VERMELHO, target_r);
            
            // Canais Invertidos (Verde/Azul): A subtração corrige a inversão do ACTIVE_LOW,
            // mantendo o brilho visual idêntico, mas rodando na fase contrária do clock.
            pwmEnableChannel(DRIVER_VERDE_AZUL, CANAL_VERDE, 255 - target_g);
            pwmEnableChannel(DRIVER_VERDE_AZUL, CANAL_AZUL, 255 - target_b);
        }

        // Dorme por 20ms para liberar a CPU totalmente para as teclas do teclado
        chThdSleepMilliseconds(20);
    }
}

// ====================================================================
// GANCHO DE INICIALIZAÇÃO AUTOMÁTICA DO QMK
// ====================================================================
void keyboard_post_init_user(void) {
    // Redireciona os pinos físicos da Pico para as saídas alternativas de PWM (Hardware)
    palSetLineMode(GP22, PAL_MODE_ALTERNATE(4)); 
    palSetLineMode(GP26, PAL_MODE_ALTERNATE(4)); 
    palSetLineMode(GP27, PAL_MODE_ALTERNATE(4)); 

    aplica_estado_led();

    // Dispara a Thread em segundo plano no RTOS
    chThdCreateStatic(waLedThread, sizeof(waLedThread), NORMALPRIO - 1, LedThread, NULL);
}
