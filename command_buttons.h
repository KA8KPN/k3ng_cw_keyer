#ifndef COMMAND_BUTTONS_H
#define COMMAND_BUTTONS_H

#ifdef FEATURE_COMMAND_BUTTONS
void check_command_buttons(void);
byte analogbuttonread(byte button);
void initialize_analog_button_array(void);
void check_button0(void);

#if defined(FEATURE_MEMORIES)
void check_the_memory_buttons(void);

#define CHECK_THE_MEMORY_BUTTONS()       check_the_memory_buttons()
#else // !defined(FEATURE_MEMORIES)
#define CHECK_THE_MEMORY_BUTTONS()
#endif // !defined(FEATURE_MEMORIES)

#define CHECK_COMMAND_BUTTONS()          check_command_buttons()
#define ANALOGBUTTONREAD(button)         analogbuttonread(button)
#define INITIALIZE_ANALOG_BUTTON_ARRAY() initialize_analog_button_array()
#define CHECK_BUTTON0()                  check_button0()

#else // !FEATURE_COMMAND_BUTTONS

#define CHECK_COMMAND_BUTTONS()
#define CHECK_THE_MEMORY_BUTTONS()
#define ANALOGBUTTONREAD(a) (0)
#define INITIALIZE_ANALOG_BUTTON_ARRAY()
#define SETONEBUTTON(a, b)
#define COMMAND_PROGRAM_MEMORY()
#define CHECK_BUTTON0()

#endif // !FEATURE_COMMAND_BUTTONS


#endif // COMMAND_BUTTONS_H
