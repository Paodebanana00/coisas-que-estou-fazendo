#pragma once

#define RP2040_MCUCONF

#include_next <mcuconf.h>

#undef RP_PWM_USE_PWM3
#define RP_PWM_USE_PWM3 TRUE

#undef RP_PWM_USE_PWM5
#define RP_PWM_USE_PWM5 TRUE

#undef RP_PWM_USE_PWM6
#define RP_PWM_USE_PWM6 TRUE







#pragma once

#define RP2040_MCUCONF

#include_next <mcuconf.h>

#undef RP_PWM_USE_PWM3
#define RP_PWM_USE_PWM3 TRUE

#undef RP_PWM_USE_PWM5
#define RP_PWM_USE_PWM5 TRUE

#undef RP_PWM_USE_PWM6
#define RP_PWM_USE_PWM6 TRUE
