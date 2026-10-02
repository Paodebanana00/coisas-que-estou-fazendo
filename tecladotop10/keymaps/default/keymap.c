// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "rgblight.h"
#include "os_detection.h"
#include "rgb_anodo_comum.h"

enum custom_keycodes {
    MY_RGB_TOG = SAFE_RANGE,
    MY_RGB_MODE,
    MY_RGB_VAI,
    MY_RGB_VAD,
    MY_RGB_HUI,
    MY_RGB_HUD,
    MY_RGB_SAI,
    MY_RGB_SAD,
    MY_RGB_SPI,
    MY_RGB_SPD,
    MY_RGB_WHITE
};

    /*
     * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┬───┐
     * │Esc│ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │ 8 │ 9 │ 0 │ - │ = │ Backsp│Hom│
     * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┼───┤
     * │ Tab │ Q │ W │ E │ R │ T │ Y │ U │ I │ O │ P │ [ │ ] │  \  │PgU│
     * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┼───┤
     * │ Caps │ A │ S │ D │ F │ G │ H │ J │ K │ L │ ; │ ' │  Enter │PgD│
     * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────┬───┼───┤
     * │ Shift  │ Z │ X │ C │ V │ B │ N │ M │ , │ . │ / │ Shift│ ↑ │End│
     * ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴──┬┴──┬┴──┬───┼───┼───┤
     * │Ctrl│GUI │Alt │                        │Alt│ Fn│Ctl│ ← │ ↓ │ → │
     * └────┴────┴────┴────────────────────────┴───┴───┴───┴───┴───┴───┘
     */
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_65_ansi(
        KC_ESC, KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,          KC_PGUP,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,          KC_DEL,
        KC_CAPS, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,           KC_MUTE,
        KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT, KC_UP,   KC_PGDN,
        KC_LCTL, KC_LGUI, KC_LALT,                            KC_SPC,                    KC_RCTL, KC_RALT, MO(1),            KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [1] = LAYOUT_65_ansi(
        KC_GRV,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,           KC_HOME,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, QK_BOOT, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_PSCR, KC_SCRL, MY_RGB_MODE,      KC_INS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MY_RGB_HUI, MY_RGB_HUD, KC_TRNS, KC_PAUSE,         KC_TRNS,          MY_RGB_TOG,
        KC_TRNS,          KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, NK_OFF,  NK_ON,   KC_TRNS, MY_RGB_SAI, MY_RGB_SAD, KC_TRNS,       KC_TRNS, MY_RGB_VAI, KC_END,
        KC_TRNS, KC_TRNS, KC_TRNS,                            KC_TRNS,                   KC_TRNS,  MO(2),  KC_TRNS,         MY_RGB_SPD, MY_RGB_VAD, MY_RGB_SPI
    ),
        [2] = LAYOUT_65_ansi(
        KC_TRNS, KC_MSEL, KC_VOLD, KC_VOLU, KC_MUTE, KC_STOP, KC_MPRV, KC_MPLY, KC_MNXT, KC_MAIL, KC_MYCM, KC_WHOM, KC_CALC, KC_TRNS,          KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, MY_RGB_WHITE,     KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,          KC_TRNS,          KC_TRNS,
        KC_TRNS,          KC_TRNS, MY_RGB_WHITE, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,       KC_TRNS, KC_TRNS, KC_TRNS,
        KC_TRNS, KC_TRNS, KC_TRNS,                            KC_TRNS,                   KC_TRNS, KC_TRNS, KC_TRNS,          KC_TRNS, KC_TRNS, KC_TRNS
    )
};
    /*
     * ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┬───┐
     * │ ` │F1 │F2 │F3 │F4 │F5 │F6 │F7 │F8 │F9 │F10│F11│F12│ Delete│   │
     * ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┼───┤
     * │     │   │   │   │   │   │Ins│   │   │   │   │PSc│Scr│Pause│   │
     * ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┼───┤
     * │      │   │   │   │   │   │   │   │   │   │   │   │        │   │
     * ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────┬───┼───┤
     * │        │   │   │   │   │   │   │Mut│Vl-│Vl+│   │      │   │   │
     * ├────┬───┴┬──┴─┬─┴───┴───┴───┴───┴───┴──┬┴──┬┴──┬┴──┬───┼───┼───┤
     * │    │    │    │                        │   │   │   │   │   │   │
     * └────┴────┴────┴────────────────────────┴───┴───┴───┴───┴───┴───┘¨*/

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [0] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [1] = { ENCODER_CCW_CW(MY_RGB_VAD, MY_RGB_VAI) },
    [2] = { ENCODER_CCW_CW(MY_RGB_HUD, MY_RGB_HUI) }
};
#endif



bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    if (!record->event.pressed) {
        return true;
    }

    switch (keycode) {

        case MY_RGB_TOG:
            rgblight_toggle();
            return false;


        case MY_RGB_MODE:
            rgblight_step();
            return false;


        case MY_RGB_VAI:
            rgblight_increase_val();
            return false;

        case MY_RGB_VAD:
            rgblight_decrease_val();
            return false;


        case MY_RGB_HUI:
            rgblight_increase_hue();
            return false;

        case MY_RGB_HUD:
            rgblight_decrease_hue();
            return false;


        case MY_RGB_SAI:
            rgblight_increase_sat();
            return false;

        case MY_RGB_SAD:
            rgblight_decrease_sat();
            return false;


        case MY_RGB_SPI:
            rgblight_increase_speed();
            return false;

        case MY_RGB_SPD:
            rgblight_decrease_speed();
            return false;

        case MY_RGB_WHITE:
            rgblight_sethsv(0, 0, rgblight_get_val());
            return false;
    }

    return true;
}


bool process_detected_host_os_user(os_variant_t detected_os) {
    switch (detected_os) {
        case OS_WINDOWS:
        case OS_LINUX:
        case OS_MACOS:
        case OS_IOS:
            keymap_config.nkro = true;
            break;

        case OS_UNSURE:
        default:
            keymap_config.nkro = false;
            break;
    }

    return true;
}