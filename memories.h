#ifndef MEMORIES_H
#define MEMORIES_H

#ifdef FEATURE_MEMORIES
extern byte play_memory_prempt;
extern long last_memory_button_buffer_insert;
extern byte repeat_memory;
extern unsigned long last_memory_repeat_time;

void check_memory_repeat(void);
void put_memory_button_in_buffer(byte memory_number_to_put_in_buffer);
void command_set_mem_repeat_delay();
void initialize_eeprom_memories();
byte memory_nonblocking_delay(unsigned long delaytime);
byte play_memory(byte memory_number);
void program_memory(int memory_number);
int memory_start(byte memory_number);
int memory_end(byte memory_number);

#if defined(FEATURE_PS2_KEYBOARD) || defined(FEATURE_USB_KEYBOARD)
void repeat_memory_msg(byte memory_number);
#endif // defined(FEATURE_PS2_KEYBOARD) || defined(FEATURE_USB_KEYBOARD)

#if defined(FEATURE_SERIAL) && defined(FEATURE_COMMAND_LINE_INTERFACE)
void serial_set_memory_repeat(HardwareSerial * port_to_use);
void repeat_play_memory(HardwareSerial * port_to_use);
void serial_play_memory(byte memory_number);
void serial_program_memory(HardwareSerial * port_to_use);
#endif // defined(FEATURE_SERIAL) && defined(FEATURE_COMMAND_LINE_INTERFACE)

#if defined(FEATURE_COMMAND_LINE_INTERFACE)
void serial_status_memories(HardwareSerial * port_to_use);
#endif // defined(FEATURE_COMMAND_LINE_INTERFACE)

#else // !FEATURE_MEMORIES
#endif // !FEATURE_MEMORIES

#endif // MEMORIES_H
