#ifndef COMMAND_BUTTONS_H
#define COMMAND_BUTTONS_H

#ifdef FEATURE_COMMAND_BUTTONS
void check_command_buttons(void);
byte analogbuttonread(byte button);
void initialize_analog_button_array(void);
void check_button0(void);
#if defined(FEATURE_MEMORIES)
void check_the_memory_buttons(void);
void command_program_memory(void);
#else // !defined(FEATURE_MEMORIES)
#define check_the_memory_buttons()
#define command_program_memory()
#endif
#ifdef FEATURE_DL2SBA_BANKSWITCH
void setOneButton(int button, int index);
#else // !FEATURE_DL2SBA_BANKSWITCH
#define setOneButton(a, b)
#endif //  !FEATURE_DL2SBA_BANKSWITCH
#else // !FEATURE_COMMAND_BUTTONS
#define check_command_buttons()
#define check_the_memory_buttons()
#define analogbuttonread(a) (0)
#define initialize_analog_button_array()
#define setOneButton(a, b)
#define command_program_memory()
#define check_button0()
#endif // !FEATURE_COMMAND_BUTTONS


#endif // COMMAND_BUTTONS_H
