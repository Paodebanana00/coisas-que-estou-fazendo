#pragma once
#include "ch.h"
#include "hal.h"

// ====================================================================
// CONFIGURAÇÕES DO PWM CONFIG (RP2040 / ChibiOS) - PARA TRANSISTOR NPN
// ====================================================================
// Usamos ACTIVE_HIGH porque o transistor NPN joga o LED pro terra com sinal HIGH!
static const PWMConfig pwmcfg_3 = {
  1000000, /* Clock de 1MHz */
  255,     /* Período de 255 (8-bits para controle suave) */
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}, // Canal 0 (Saída A do Bloco 3 -> GP22)
    {PWM_OUTPUT_DISABLED, NULL}     // Canal 1 Desativado
  }
};

static const PWMConfig pwmcfg_5 = {
  1000000, /* Clock de 1MHz */
  255,     /* Período de 255 (8-bits) */
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}, // Canal 0 (Saída A do Bloco 5 -> GP26)
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}  // Canal 1 (Saída B do Bloco 5 -> GP27)
  }
};

// Mapeamento dos Drivers com base nas trilhas físicas do RP2040
#define DRIVER_VERMELHO   &PWMD3
#define CANAL_VERMELHO    0  // GP22

#define DRIVER_VERDE_AZUL &PWMD5
#define CANAL_VERDE       0  // GP26
#define CANAL_AZUL        1  // GP27

// Alvos de brilho que o silício vai modular (Escala real de 0 a 255)
static uint8_t target_r = 0;
static uint8_t target_g = 0;
static uint8_t target_b = 0;

// Vincula o código do LED com as variáveis de estado declaradas no seu keymap.c
extern bool rgb_led_enabled;
extern uint8_t cor_atual;
extern uint8_t brilho_atual;

// Atualiza os alvos de cor baseado nas teclas pressionadas
void aplica_estado_led(void) {
    if (!rgb_led_enabled) {
        target_r = 0; target_g = 0; target_b = 0;
    } else {
        switch (cor_atual) {
            case 0: // Branco (Mistura de todas as cores usando o brilho dinâmico)
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
        }
    }
}

// ====================================================================
// THREAD EM SEGUNDO PLANO PARA ATUALIZAR O HARDWARE
// ====================================================================
static THD_WORKING_AREA(waLedThread, 128);
static THD_FUNCTION(LedThread, arg) {
    (void)arg;
    chRegSetThreadName("hardware_pwm_update");

    // Inicializa os periféricos de hardware de PWM no chip
    pwmStart(DRIVER_VERMELHO, &pwmcfg_3);
    pwmStart(DRIVER_VERDE_AZUL, &pwmcfg_5);

    while (true) {
        // Alimenta o circuito do RP2040 com os valores de Duty Cycle desejados.
        pwmEnableChannel(DRIVER_VERMELHO, CANAL_VERMELHO, target_r);
        pwmEnableChannel(DRIVER_VERDE_AZUL, CANAL_VERDE, target_g);
        pwmEnableChannel(DRIVER_VERDE_AZUL, CANAL_AZUL, target_b);

        // Dorme por 20ms para liberar totalmente a CPU para as teclas
        chThdSleepMilliseconds(20);
    }
}

// ====================================================================
// GANCHO DE INICIALIZAÇÃO AUTOMÁTICA DO QMK
// ====================================================================
void keyboard_post_init_user(void) {
    // Redireciona a função física das pernas GP22, GP26 e GP27 do chip para as saídas de PWM
    palSetLineMode(GP22, PAL_MODE_ALTERNATE(4)); 
    palSetLineMode(GP26, PAL_MODE_ALTERNATE(4)); 
    palSetLineMode(GP27, PAL_MODE_ALTERNATE(4)); 

    // Calcula as cores iniciais baseando-se no valor de brilho_atual
    aplica_estado_led();

    // Dispara o gerenciador estável em segundo plano
    chThdCreateStatic(waLedThread, sizeof(waLedThread), NORMALPRIO - 1, LedThread, NULL);
}
