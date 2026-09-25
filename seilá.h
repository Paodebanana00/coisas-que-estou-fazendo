#include "ch.h"
#include "hal.h"
#include "rgblight.h" // Traz as estruturas e animações nativas do QMK

// ====================================================================
// CONFIGURAÇÕES DOS TIMERS DE PWM POR HARDWARE (RP2040 / ChibiOS)
// ====================================================================

// Bloco 3 (Vermelho): Normal (Active High)
static const PWMConfig pwmcfg_3 = {
  1000000, /* Clock de 1MHz */
  255,     /* Período de 255 passos (8-bits) */
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}, // Canal 0 (Saída A do Bloco 3 -> GP22)
    {PWM_OUTPUT_DISABLED, NULL}     // Canal 1 Desativado
  }
};

// Bloco 5 (Verde/Azul): Também Active High para os transistores NPN
static const PWMConfig pwmcfg_5 = {
  1000000, /* Clock de 1MHz */
  255,     /* Período de 255 passos (8-bits) */
  NULL,
  {
    {PWM_OUTPUT_ACTIVE_HIGH, NULL},  // Canal 0 (Saída A do Bloco 5 -> GP26)
    {PWM_OUTPUT_ACTIVE_HIGH, NULL}   // Canal 1 (Saída B do Bloco 5 -> GP27)
  }
};  

#define DRIVER_VERMELHO   &PWMD3
#define CANAL_VERMELHO    0  // GP22

#define DRIVER_VERDE_AZUL &PWMD5
#define CANAL_VERDE       0  // GP26
#define CANAL_AZUL        1  // GP27

// Alvos de brilho instantâneos de hardware
static uint8_t target_r = 0;
static uint8_t target_g = 0;
static uint8_t target_b = 0;

// ====================================================================
// THREAD DE GERENCIAMENTO PWM COM INTERCEPTAÇÃO E PHASE-SHIFTING
// ====================================================================
static THD_WORKING_AREA(waLedThread, 128);
static THD_FUNCTION(LedThread, arg) {
    (void)arg;
    chRegSetThreadName("hardware_pwm_manager");
    
    // Inicializa os blocos elétricos de hardware através do HAL do ChibiOS
    pwmStart(DRIVER_VERMELHO, &pwmcfg_3);
    pwmStart(DRIVER_VERDE_AZUL, &pwmcfg_5);

    // CORREÇÃO SEM SDK: Acessando os registradores usando as estruturas internas do ChibiOS.
    // Usamos a setinha (->) pois CH agora aponta corretamente para o layout de memória do chip.
    // Bit 2 do CSR ativa a inversão física do Canal B (Salva os C945 e divide a carga na USB!)
    PWMD5.pwm->CH->CSR |= (1 << 2); 

    // Alinha os cronômetros no mesmo nanossegundo absoluto de partida
    PWMD3.pwm->CH->CTR = 0;
    PWMD5.pwm->CH->CTR = 0;

    while (true) {
        // Se o LED geral estiver desativado no QMK, força o estado zero elétrico
        if (!rgblight_is_enabled()) {
            target_r = 0; target_g = 0; target_b = 0;
        } 
        else {
            // ROUBO DE SINAL: O array global 'leds' do QMK é um ponteiro/array.
            // Lemos o primeiro índice [0] do LED virtual que está rodando a animação.
            uint8_t r_qmk = leds[0].r;
            uint8_t g_qmk = leds[0].g;
            uint8_t b_qmk = leds[0].b;

            // BALANÇO DE BRANCO DA SUCATA (Para resistores de 80R e 40R nas 5 fileiras)
            target_r = r_qmk;
            target_g = (g_qmk * 150) / 255; // Capa o "Verde-Shrek" para ~60% do brilho
            target_b = (b_qmk * 200) / 255; // Ajusta o Azul solto/resistor de 470R para ~80%
        }

        // ENVIANDO OS VALORES DIRETAMENTE PARA O DUTY CYCLE DO HARDWARE
        pwmEnableChannel(DRIVER_VERMELHO, CANAL_VERMELHO, target_r);
        pwmEnableChannel(DRIVER_VERDE_AZUL, CANAL_VERDE, target_g);
        pwmEnableChannel(DRIVER_VERDE_AZUL, CANAL_AZUL, target_b);

        // 10ms deixa todas as animações nativas do QMK ultra fluidas
        chThdSleepMilliseconds(10);
    }
}

// Gancho de inicialização automática do QMK
void keyboard_post_init_user(void) {
    // Redireciona os pinos físicos da Pico usando as definições de linhas (PAL) do ChibiOS
    palSetLineMode(GP22, PAL_MODE_ALTERNATE(4)); 
    palSetLineMode(GP26, PAL_MODE_ALTERNATE(4)); 
    palSetLineMode(GP27, PAL_MODE_ALTERNATE(4)); 

    // Dispara a Thread em segundo plano no ChibiOS com prioridade segura
    chThdCreateStatic(waLedThread, sizeof(waLedThread), NORMALPRIO - 1, LedThread, NULL);
}
