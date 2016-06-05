#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#ifdef FEATURE_PS2_KEYBOARD

#ifdef OPTION_USE_ORIGINAL_VERSION_2_1_PS2KEYBOARD_LIB
#include <PS2Keyboard.h>
#else //OPTION_USE_ORIGINAL_VERSION_2_1_PS2KEYBOARD_LIB
#include <K3NG_PS2Keyboard.h>
#endif

extern byte ps2_keyboard_mode;
extern byte ps2_keyboard_command_buffer[25];
extern byte ps2_keyboard_command_buffer_pointer;

void initialize_ps2_keyboard(void);
void check_ps2_keyboard(void);
void ps2_usb_keyboard_play_memory(byte memory_number);
void ps2_keyboard_program_memory(byte memory_number);
int ps2_keyboard_get_number_input(byte places, int lower_limit, int upper_limit);

#define INITIALIZE_PS2_KEYBOARD() initialize_ps2_keyboard()
#define CHECK_PS2_KEYBOARD() check_ps2_keyboard()
#define PS2_USB_KEYBOARD_PLAY_MEMORY(memory_number)  ps2_usb_keyboard_play_memory(memory_number)
#define PS2_KEYBOARD_PROGRAM_MEMORY(memory_number) ps2_keyboard_program_memory(memory_number)
#define PS2_KEYBOARD_GET_NUMBER_INPUT(places, lower_limit, upper_limit) ps2_keyboard_get_number_input(places, lower_limit, upper_limit)

#else // !FEATURE_PS2_KEYBOARD

#define INITIALIZE_PS2_KEYBOARD()
#define CHECK_PS2_KEYBOARD()
#define PS2_USB_KEYBOARD_PLAY_MEMORY(memory_number)
#define PS2_KEYBOARD_PROGRAM_MEMORY(memory_number)
#define PS2_KEYBOARD_GET_NUMBER_INPUT(places, lower_limit, upper_limit)

#endif // !FEATURE_PS2_KEYBOARD
#endif // PS2_KEYBOARD_H
