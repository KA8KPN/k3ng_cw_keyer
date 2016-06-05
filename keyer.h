#ifndef keyer_h
#define keyer_h

// Do not change these !

// Variable macros
#define STRAIGHT 1
#define IAMBIC_B 2
#define IAMBIC_A 3
#define BUG 4
#define ULTIMATIC 5

#define PADDLE_NORMAL 0
#define PADDLE_REVERSE 1

#define KEYER_NORMAL 0
#define BEACON 1
#define KEYER_COMMAND_MODE 2

extern byte keyer_machine_mode;   // KEYER_NORMAL, BEACON, KEYER_COMMAND_MODE
#define OMIT_LETTERSPACE 1

#define SIDETONE_OFF 0
#define SIDETONE_ON 1
#define SIDETONE_PADDLE_ONLY 2

#define SENDING_NOTHING 0
#define SENDING_DIT 1
#define SENDING_DAH 2

#define SPEED_NORMAL 0
#define SPEED_QRSS 1
extern byte speed_mode;

#define CW 0
#define HELL 1

#ifdef FEATURE_PS2_KEYBOARD
  #define PS2_KEYBOARD_NORMAL 0
#endif //FEATURE_PS2_KEYBOARD

#define SERIAL_CLI 0
#define SERIAL_WINKEY_EMULATION 1

#define SERIAL_SEND_BUFFER_SPECIAL_START 13
#define SERIAL_SEND_BUFFER_WPM_CHANGE 14        
#define SERIAL_SEND_BUFFER_PTT_ON 15            
#define SERIAL_SEND_BUFFER_PTT_OFF 16           
#define SERIAL_SEND_BUFFER_TIMED_KEY_DOWN 17    
#define SERIAL_SEND_BUFFER_TIMED_WAIT 18        
#define SERIAL_SEND_BUFFER_NULL 19              
#define SERIAL_SEND_BUFFER_PROSIGN 20           
#define SERIAL_SEND_BUFFER_HOLD_SEND 21         
#define SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE 22 
#define SERIAL_SEND_BUFFER_MEMORY_NUMBER 23
#define SERIAL_SEND_BUFFER_TX_CHANGE 24
#define SERIAL_SEND_BUFFER_SPECIAL_END 25

#if defined(OPTION_PROSIGN_SUPPORT)
  #define PROSIGN_START 127
  #define PROSIGN_AA 128
  #define PROSIGN_AS 129
  #define PROSIGN_BK 130
  #define PROSIGN_CL 131
  #define PROSIGN_CT 132
  #define PROSIGN_KN 133
  #define PROSIGN_NJ 134
  #define PROSIGN_SK 135
  #define PROSIGN_SN 136
  #define PROSIGN_HH 137    // iz0rus
  #define PROSIGN_END 138  // iz0rus
#endif

#define SERIAL_SEND_BUFFER_NORMAL 0
#define SERIAL_SEND_BUFFER_TIMED_COMMAND 1
#define SERIAL_SEND_BUFFER_HOLD 2

#ifdef FEATURE_WINKEY_EMULATION
#define WINKEY_NO_COMMAND_IN_PROGRESS 0
#define WINKEY_UNBUFFERED_SPEED_COMMAND 1
#define WINKEY_UNSUPPORTED_COMMAND 2
#define WINKEY_POINTER_COMMAND 3
#define WINKEY_ADMIN_COMMAND 4
#define WINKEY_PAUSE_COMMAND 5
#define WINKEY_KEY_COMMAND 6
#define WINKEY_SETMODE_COMMAND 7
#define WINKEY_SIDETONE_FREQ_COMMAND 8
#define WINKEY_ADMIN_COMMAND_ECHO 9
#define WINKEY_BUFFERED_SPEED_COMMAND 10
#define WINKEY_DAH_TO_DIT_RATIO_COMMAND 11
#define WINKEY_KEYING_COMPENSATION_COMMAND 12
#define WINKEY_FIRST_EXTENSION_COMMAND 13
#define WINKEY_PTT_TIMES_PARM1_COMMAND 14
#define WINKEY_PTT_TIMES_PARM2_COMMAND 15
#define WINKEY_SET_POT_PARM1_COMMAND 16
#define WINKEY_SET_POT_PARM2_COMMAND 17
#define WINKEY_SET_POT_PARM3_COMMAND 18
#define WINKEY_SOFTWARE_PADDLE_COMMAND 19
#define WINKEY_CANCEL_BUFFERED_SPEED_COMMAND 20
#define WINKEY_BUFFFERED_PTT_COMMMAND 21
#define WINKEY_HSCW_COMMAND 22
#define WINKEY_BUFFERED_HSCW_COMMAND 23
#define WINKEY_WEIGHTING_COMMAND 24
#define WINKEY_KEY_BUFFERED_COMMAND 25
#define WINKEY_WAIT_BUFFERED_COMMAND 26
#define WINKEY_POINTER_01_COMMAND 27
#define WINKEY_POINTER_02_COMMAND 28
#define WINKEY_POINTER_03_COMMAND 29
#define WINKEY_FARNSWORTH_COMMAND 30
#define WINKEY_MERGE_COMMAND 31
#define WINKEY_MERGE_PARM_2_COMMAND 32
#define WINKEY_SET_PINCONFIG_COMMAND 33
#define WINKEY_EXTENDED_COMMAND 34
#ifdef OPTION_WINKEY_2_SUPPORT
#define WINKEY_SEND_MSG 35
#endif //OPTION_WINKEY_2_SUPPORT
#define WINKEY_LOAD_SETTINGS_PARM_1_COMMAND 101
#define WINKEY_LOAD_SETTINGS_PARM_2_COMMAND 102
#define WINKEY_LOAD_SETTINGS_PARM_3_COMMAND 103
#define WINKEY_LOAD_SETTINGS_PARM_4_COMMAND 104
#define WINKEY_LOAD_SETTINGS_PARM_5_COMMAND 105
#define WINKEY_LOAD_SETTINGS_PARM_6_COMMAND 106
#define WINKEY_LOAD_SETTINGS_PARM_7_COMMAND 107
#define WINKEY_LOAD_SETTINGS_PARM_8_COMMAND 108
#define WINKEY_LOAD_SETTINGS_PARM_9_COMMAND 109
#define WINKEY_LOAD_SETTINGS_PARM_10_COMMAND 110
#define WINKEY_LOAD_SETTINGS_PARM_11_COMMAND 111
#define WINKEY_LOAD_SETTINGS_PARM_12_COMMAND 112
#define WINKEY_LOAD_SETTINGS_PARM_13_COMMAND 113
#define WINKEY_LOAD_SETTINGS_PARM_14_COMMAND 114
#define WINKEY_LOAD_SETTINGS_PARM_15_COMMAND 115

#define WINKEY_HOUSEKEEPING 0
#define SERVICE_SERIAL_BYTE 1
#endif //FEATURE_WINKEY_EMULATION

#define AUTOMATIC_SENDING 0
#define MANUAL_SENDING 1

#define ULTIMATIC_NORMAL 0
#define ULTIMATIC_DIT_PRIORITY 1
#define ULTIMATIC_DAH_PRIORITY 2


#define PRINTCHAR 0
#define NOPRINT 1

extern byte command_mode_disable_tx;

extern byte dit_buffer;     // used for buffering paddle hits in iambic operation
extern byte dah_buffer;     // used for buffering paddle hits in iambic operation
extern byte button0_buffer;
extern byte length_letterspace;
extern byte config_dirty;
extern byte key_tx;         // 0 = tx_key_line control suppressed
#ifdef FEATURE_MEMORIES
  extern byte play_memory_prempt;
  extern long last_memory_button_buffer_insert;
  extern byte repeat_memory;
  extern unsigned long last_memory_repeat_time;
#endif //FEATURE_MEMORIES

// These are from after I started the reorganization.  I hope to make some or all of them disappear eventually
void say_hi(void);
void send_dit(byte sending_type);
void send_dah(byte sending_type);
void speed_change(int change);
void switch_to_tx(byte tx);
long get_cw_input_from_user(unsigned int exit_time_milliseconds);

// These are all from before.  They should mostly disappear quickly
void initialize_pins(void);
void initialize_keyer_state(void);
void initialize_default_modes(void);
void initialize_watchdog(void);
void initialize_ethernet_variables(void);
void check_eeprom_for_initialization(void);
void check_for_beacon_mode(void);
void check_for_debug_modes(void);
void initialize_serial_ports(void);
void initialize_ps2_keyboard(void);
void initialize_usb(void);
void initialize_cw_keyboard(void);
void initialize_display(void);
void initialize_ethernet(void);
void initialize_udp(void);
void initialize_web_server(void);
void initialize_debug_startup(void);
void check_paddles(void);
void service_dit_dah_buffers(void);
void check_ptt_tail(void);
void check_ps2_keyboard(void);
void check_for_dirty_configuration(void);
void check_memory_repeat(void);
void loop_element_lengths(float lengths, float additional_time_ms, int speed_wpm_in, byte sending_type);
byte analogbuttonpressed(void);
void send_char(byte cw_char, byte omit_letterspace);
void ps2_usb_keyboard_play_memory(byte memory_number);
void tx_and_sidetone_key(int state, byte sending_type);
void ps2_keyboard_program_memory(byte memory_number);
int ps2_keyboard_get_number_input(byte places, int lower_limit, int upper_limit);
int convert_cw_number_to_ascii(long number_in);
int memory_end(byte memory_number);
int memory_start(byte memory_number);
byte play_memory(byte memory_number);
void program_memory(int memory_number);

void write_settings_to_eeprom(int);
void add_to_send_buffer(byte incoming_serial_byte);
void sidetone_adj(int freq);
void adjust_dah_to_dit_ratio(int ratio);
void speed_set(int speed);
void clear_send_buffer(void);
void put_serial_number_in_send_buffer(void);
void boop_beep(void);
void beep(void);
void ptt_unkey(void);
void ptt_key(void);
void switch_to_tx_silent(byte tx);
int uppercase(int c);
void boop(void);
void check_dit_paddle(void);
void check_dah_paddle(void);
void initialize_eeprom_memories(void);
int paddle_pin_read(int paddle);
void analog_button_read(int);
int analog_button_pressed(void);
void command_sidetone_freq_adj(void);
void command_dah_to_dit_ratio_adjust(void);
void command_tuning_mode(void);
void command_speed_mode(void);
void command_set_mem_repeat_delay(void);
void command_play_memory(int memory_number);
void beep_boop(void);
void remove_from_send_buffer(void);

void check_serial(void);
void check_paddle_echo(void);
void service_paddle_echo(void);
void process_serial_command(HardwareSerial *port);
void serial_set_serial_number(HardwareSerial *port);
void serial_set_sidetone_freq(HardwareSerial *port);
void repeat_play_memory(HardwareSerial *port);
void serial_set_memory_repeat(HardwareSerial *port);
void serial_play_memory(byte memory);
void serial_program_memory(HardwareSerial *port);
void serial_status(HardwareSerial *port);
void serial_set_dit_to_dah_ratio(HardwareSerial *port);
void serial_set_weighting(HardwareSerial *port);
void serial_tune_command(HardwareSerial *port);
void serial_wpm_set(HardwareSerial *port);
void serial_switch_tx(HardwareSerial *port);
void serial_change_wordspace(HardwareSerial *port);
int serial_get_number_input(byte places, int lower_limit, int upper_limit, HardwareSerial *port);
void serial_status_memories(HardwareSerial *port);

void clear_display_row(byte row);

#endif //keyer_h
