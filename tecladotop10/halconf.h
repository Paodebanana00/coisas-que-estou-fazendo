#pragma once

#include_next <halconf.h>

// Força o ChibiOS a ativar os drivers de PWM internos do sistema
#undef HAL_USE_PWM
#define HAL_USE_PWM TRUE
#undef HAL_USE_SERIAL_USB
#define HAL_USE_SERIAL_USB TRUE