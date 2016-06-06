#include "keyer_features_and_options.h"

#ifdef FEATURE_MEMORIES

#include <stdio.h>
#include <Arduino.h>

#include "memories.h"
#include "display.h"
#include "command_buttons.h"
#include "potentiometer.h"
#include "rotary_encoder.h"
#include "ps2_keyboard.h"

#include "keyer_pin_settings.h"
#include "keyer_settings.h"
#include "keyer.h"
#include "config.h"
#include <EEPROM.h>

byte play_memory_prempt = 0;
long last_memory_button_buffer_insert = 0;
byte repeat_memory = 255;
unsigned long last_memory_repeat_time = 0;

//-------------------------------------------------------------------------------------------------------
void check_memory_repeat() {

  #ifdef DEBUG_LOOP
  debug_serial_port->println(F("loop: entering check_memory_repeat"));
  #endif    
  
  if ((repeat_memory < number_of_memories) && ((millis() - last_memory_repeat_time) > configuration.memory_repeat_time)) {
    add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
    add_to_send_buffer(repeat_memory);
    last_memory_repeat_time = millis();
    #ifdef DEBUG_MEMORIES
    debug_serial_port->print(F("check_memory_repeat: added repeat_memory to send buffer\n\r"));
    #endif
  }
  
  if (repeat_memory == 255){last_memory_repeat_time = 0;}

}

//-------------------------------------------------------------------------------------------------------

#if (defined(FEATURE_PS2_KEYBOARD) || defined(FEATURE_USB_KEYBOARD))
void repeat_memory_msg(byte memory_number){
  repeat_memory = memory_number;
  LCD_CENTER_PRINT_TIMED("Repeat Memory " + String(memory_number+1), 0, default_display_msg_delay);
  SERVICE_DISPLAY();
}
#endif //defined(FEATURE_PS2_KEYBOARD) || defined(FEATURE_USB_KEYBOARD)

//-------------------------------------------------------------------------------------------------------

void put_memory_button_in_buffer(byte memory_number_to_put_in_buffer)
{

  if (memory_number_to_put_in_buffer < number_of_memories) {
    #ifdef DEBUG_MEMORIES
    debug_serial_port->print(F("put_memory_button_in_buffer: memory_number_to_put_in_buffer:"));
    debug_serial_port->println(memory_number_to_put_in_buffer,DEC);
    #endif
    repeat_memory = 255;
    if ((millis() - last_memory_button_buffer_insert) > 400) {    // don't do another buffer insert if we just did one - button debounce
      #ifdef FEATURE_WINKEY_EMULATION
      if (winkey_sending && winkey_host_open) {
        winkey_port_write(0xc0|winkey_sending|winkey_xoff);
        winkey_interrupted = 1;
      }
      #endif

      add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
      add_to_send_buffer(memory_number_to_put_in_buffer);
      last_memory_button_buffer_insert = millis();
    }
  } else {
    #ifdef DEBUG_MEMORIES
    debug_serial_port->println(F("put_memory_button_in_buffer: bad memory_number_to_put_in_buffer"));
    #endif
  }
}

//-------------------------------------------------------------------------------------------------------
void command_set_mem_repeat_delay() {
  
  byte character_count = 0;;
  int cw_char = 0;
  byte number_sent = 0;
  unsigned int repeat_value = 0;
  byte error_flag = 0;
  
  for (character_count = 0; character_count < 4; character_count++) {
    cw_char = get_cw_input_from_user(0);
    number_sent = (convert_cw_number_to_ascii(cw_char) - 48);
    if ((number_sent > -1) && (number_sent < 10)) {
      repeat_value = (repeat_value * 10) + number_sent;
    } else { // we got a bad value
      error_flag = 1;
      character_count = 5;
    }      
  }
  
  if (error_flag) {
    boop();
  } else {
    configuration.memory_repeat_time = repeat_value;
    config_dirty = 1;
    beep();
  }
  
}

//---------------------------------------------------------------------
#if defined(FEATURE_SERIAL) && defined(FEATURE_COMMAND_LINE_INTERFACE)
void serial_set_memory_repeat(HardwareSerial * port_to_use) {

  int temp_int = serial_get_number_input(5, -1, 32000, port_to_use);
  if (temp_int > -1) {
    configuration.memory_repeat_time = temp_int;
    config_dirty = 1;
  }

}

//---------------------------------------------------------------------
void repeat_play_memory(HardwareSerial * port_to_use) {

  byte memory_number = serial_get_number_input(2,0, (number_of_memories+1), port_to_use);
  #ifdef DEBUG_CHECK_SERIAL
    debug_serial_port->print(F("repeat_play_memory: memory_number:"));
    debug_serial_port->println(memory_number);
  #endif //DEBUG_SERIAL
  if (memory_number > -1) {
    repeat_memory = memory_number - 1;
  }

}

//---------------------------------------------------------------------
void serial_play_memory(byte memory_number) {

  if (memory_number < number_of_memories) {
    add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
    add_to_send_buffer(memory_number);
    repeat_memory = 255;
  }

}
#endif // defined(FEATURE_SERIAL) && defined(FEATURE_COMMAND_LINE_INTERFACE)


//---------------------------------------------------------------------
void initialize_eeprom_memories()
{
  for (int x = 0; x < number_of_memories; x++) {
    EEPROM.write(memory_start(x),255);
  }
}

//---------------------------------------------------------------------
#if defined(FEATURE_COMMAND_LINE_INTERFACE)
void serial_status_memories(HardwareSerial * port_to_use)
{
  int last_memory_location;

  #if defined(OPTION_PROSIGN_SUPPORT)
    byte eeprom_temp = 0;
    static char * prosign_temp = "";
  #endif

  for (int x = 0; x < number_of_memories; x++) {
    last_memory_location = memory_end(x) + 1 ;
    port_to_use->write("Memory ");
    port_to_use->print(x+1);
    port_to_use->write(":");
    if ( EEPROM.read(memory_start(x)) == 255) {
      port_to_use->write("{empty}");
    } else {
      for (int y = (memory_start(x)); (y < last_memory_location); y++) {
        if (EEPROM.read(y) < 255) {
          #if defined(OPTION_PROSIGN_SUPPORT)
            eeprom_temp = EEPROM.read(y);
            if ((eeprom_temp > PROSIGN_START) && (eeprom_temp < PROSIGN_END)){
              prosign_temp = convert_prosign(eeprom_temp);
              port_to_use->print(prosign_temp[0]);
              port_to_use->print(prosign_temp[1]);
            } else {
              port_to_use->write(eeprom_temp);
            }
          #else         
            port_to_use->write(EEPROM.read(y));
          #endif //OPTION_PROSIGN_SUPPORT
        } else {
          port_to_use->write("$");
          y = last_memory_location;
        }
      }
    }
    port_to_use->println();
  }
}
#endif

//---------------------------------------------------------------------
#if defined(FEATURE_SERIAL) && defined(FEATURE_COMMAND_LINE_INTERFACE)
void serial_program_memory(HardwareSerial * port_to_use)
{

  byte incoming_serial_byte;
  byte memory_number;
  byte looping = 1;
  int memory_index = 0;

  while (port_to_use->available() == 0) {        // wait for the next keystroke
    if (keyer_machine_mode == KEYER_NORMAL) {          // might as well do something while we're waiting
      check_paddles();
      service_dit_dah_buffers();
      //CHECK_THE_MEMORY_BUTTONS();
    }
  }
  incoming_serial_byte = port_to_use->read();
  if (incoming_serial_byte == 48) {incoming_serial_byte = 58;} // 0 = memory 10
  if ((incoming_serial_byte > 48) && (incoming_serial_byte < (49 + number_of_memories))) {    
    memory_number = incoming_serial_byte - 49;
    port_to_use->print(memory_number+1);
    while (looping) {
      while (port_to_use->available() == 0) {
        if (keyer_machine_mode == KEYER_NORMAL) {          // might as well do something while we're waiting
          check_paddles();
          service_dit_dah_buffers();
        }
      }
      incoming_serial_byte = port_to_use->read();
      if (incoming_serial_byte == 13) {        // did we get a carriage return?
        looping = 0;
      } else {
        port_to_use->write(incoming_serial_byte);
        incoming_serial_byte = uppercase(incoming_serial_byte);
        EEPROM.write((memory_start(memory_number)+memory_index),incoming_serial_byte);
        #ifdef DEBUG_EEPROM
        debug_serial_port->print(F("serial_program_memory: wrote "));
        debug_serial_port->print(incoming_serial_byte);
        debug_serial_port->print(F(" to location "));
        debug_serial_port->println((memory_start(memory_number)+memory_index));
        #endif
        memory_index++;
        if ((memory_start(memory_number) + memory_index) == memory_end(memory_number)) {    // are we at last memory location?
          looping = 0;
          port_to_use->println(F("Memory full, truncating."));
        }
      }
    }  //while (looping)
    // write terminating 255
    EEPROM.write((memory_start(memory_number)+memory_index),255);
    #ifdef DEBUG_EEPROM
      debug_serial_port->print(F("serial_program_memory: wrote 255 to location "));
      debug_serial_port->println((memory_start(memory_number)+memory_index));
    #endif
    port_to_use->print(F("\n\rWrote memory #"));
    port_to_use->print(memory_number+1);
    port_to_use->println();
  } else {
    port_to_use->println(F("\n\rError"));
  }

}

#endif


//---------------------------------------------------------------------
byte memory_nonblocking_delay(unsigned long delaytime)
{
  // 2012-04-20 was long starttime = millis();
  unsigned long starttime = millis();

  while ((millis() - starttime) < delaytime) {
    check_paddles();
    if (((dit_buffer) || (dah_buffer) || (ANALOGBUTTONREAD(0))) && (keyer_machine_mode != BEACON)) {   // exit if the paddle or button0 was hit
      dit_buffer = 0;
      dah_buffer = 0;
      while (ANALOGBUTTONREAD(0)) {}
      return 1;
    }
  }
  return 0;
}

//---------------------------------------------------------------------
byte play_memory(byte memory_number)
{
  
  unsigned int jump_back_to_y = 9999;
  byte jump_back_to_memory_number = 255;

  /*static*/ String serial_number_string;
  static byte prosign_flag = 0;
  play_memory_prempt = 0;
  byte eeprom_byte_read;  

  #if defined(OPTION_PROSIGN_SUPPORT)
    byte eeprom_temp = 0;
    static char * prosign_temp = "";
  #endif  

  if (memory_number > (number_of_memories - 1)) {
    boop();
    return 0;
  }



  #ifdef DEBUG_PLAY_MEMORY
    debug_serial_port->print(F("play_memory: called with memory_number:"));
    debug_serial_port->println(memory_number);
  #endif  
  
  #ifdef FEATURE_MEMORY_MACROS
    byte eeprom_byte_read2;
    int z;
    byte input_error;
    byte delay_result = 0;
    int int_from_macro;
  #endif //FEATURE_MEMORY_MACROS

  button0_buffer = 0;

//  #ifdef DEBUG_MEMORYCHECK
//  memorycheck();
//  #endif

  if (keyer_machine_mode == KEYER_NORMAL) {
    #if defined(FEATURE_SERIAL)
      #ifdef FEATURE_WINKEY_EMULATION
        if (primary_serial_port_mode != SERIAL_WINKEY_EMULATION) {
          primary_serial_port->println();
        }
      #else
        primary_serial_port->println();
      #endif

      #ifdef FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
        secondary_serial_port->println();
      #endif //FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
    #endif
  }
  
  for (int y = (memory_start(memory_number)); (y < (memory_end(memory_number)+1)); y++) {

    if (keyer_machine_mode == KEYER_NORMAL) {
        CHECK_POTENTIOMETER();
      
        CHECK_ROTARY_ENCODER();
      
        CHECK_PS2_KEYBOARD();

      CHECK_BUTTON0();

      SERVICE_DISPLAY();
    }

    #if defined(FEATURE_SERIAL)
      check_serial();
    #endif

    if ((play_memory_prempt == 0) && (pause_sending_buffer == 0)) {
      eeprom_byte_read = EEPROM.read(y);
      if (eeprom_byte_read < 255) {

        #ifdef DEBUG_PLAY_MEMORY
          debug_serial_port->println(F("\n\nplay_memory:\r"));
          debug_serial_port->print(F("    Memory number:"));
          debug_serial_port->println(memory_number);
          debug_serial_port->print(F("    EEPROM location:"));
          debug_serial_port->println(y);
          debug_serial_port->print(F("    eeprom_byte_read:"));
          debug_serial_port->println(eeprom_byte_read);
        #endif

        if (eeprom_byte_read != 92) {          // do we have a backslash?
          if (keyer_machine_mode == KEYER_NORMAL) {

            #if defined(OPTION_PROSIGN_SUPPORT)
              eeprom_temp = eeprom_byte_read;
              if ((eeprom_temp > PROSIGN_START) && (eeprom_temp < PROSIGN_END)){
                prosign_temp = convert_prosign(eeprom_temp);
              }
            #endif //OPTION_PROSIGN_SUPPORT

            #if defined(FEATURE_SERIAL)
              #ifndef FEATURE_WINKEY_EMULATION
                if (!cw_send_echo_inhibit) {
                  #if defined(OPTION_PROSIGN_SUPPORT)
                    if ((eeprom_temp > PROSIGN_START) && (eeprom_temp < PROSIGN_END)){
                      primary_serial_port->print(prosign_temp[0]);
                      primary_serial_port->print(prosign_temp[1]);
                      #ifdef FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
                        secondary_serial_port->print(prosign_temp[0]);
                        secondary_serial_port->print(prosign_temp[1]);
                      #endif //FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT                      
                    } else {
                      primary_serial_port->write(eeprom_byte_read);
                      #ifdef FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
                        secondary_serial_port->write(eeprom_byte_read);
                      #endif //FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
                    }
                  #else
                    primary_serial_port->write(eeprom_byte_read);
                    #ifdef FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
                      secondary_serial_port->write(eeprom_byte_read);
                    #endif //FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
                  #endif // OPTION_PROSIGN_SUPPORT
                }
              #else  //FEATURE_WINKEY_EMULATION
                if (((primary_serial_port_mode == SERIAL_WINKEY_EMULATION) && (winkey_paddle_echo_activated) && (winkey_host_open)) || (primary_serial_port_mode != SERIAL_WINKEY_EMULATION)) {
  
                  #if defined(OPTION_PROSIGN_SUPPORT)
                    if ((eeprom_temp > PROSIGN_START) && (eeprom_temp < PROSIGN_END)){
                      winkey_port_write(prosign_temp[0]);
                      winkey_port_write(prosign_temp[1]);                 
                    } else {
                      winkey_port_write(eeprom_byte_read);
                    }
                  #else
                    winkey_port_write(eeprom_byte_read);
                  #endif // OPTION_PROSIGN_SUPPORT



                }
                #ifdef FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
                  #if defined(OPTION_PROSIGN_SUPPORT)
                    if ((eeprom_temp > PROSIGN_START) && (eeprom_temp < PROSIGN_END)){
                      secondary_serial_port->print(prosign_temp[0]);
                      secondary_serial_port->print(prosign_temp[1]);                      
                    } else {
                      secondary_serial_port->write(eeprom_byte_read);
                    }
                  #else
                    secondary_serial_port->write(eeprom_byte_read);
                  #endif // OPTION_PROSIGN_SUPPORT
                #endif //FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT  

              #endif //FEATURE_WINKEY_EMULATION
            #endif //FEATURE_SERIAL

            #ifdef FEATURE_DISPLAY
              if (lcd_send_echo) {
                #if defined(OPTION_PROSIGN_SUPPORT)
                    if ((eeprom_temp > PROSIGN_START) && (eeprom_temp < PROSIGN_END)){
                      display_scroll_print_char(prosign_temp[0]);
                      display_scroll_print_char(prosign_temp[1]);                    
                    } else {
                      display_scroll_print_char(eeprom_byte_read); 
                    }
                #else 
                  display_scroll_print_char(eeprom_byte_read); 
                #endif
                service_display();
              }
            #endif    // FEATURE_DISPLAY

          }

          if (prosign_flag) {
            send_char(eeprom_byte_read,OMIT_LETTERSPACE);
            prosign_flag = 0;
          } else {
            send_char(eeprom_byte_read,KEYER_NORMAL);         // no - play the character
          }
        } else {                               // yes - we have a backslash command ("macro")
          y++;                                 // get the next memory byte
          #ifdef FEATURE_MEMORY_MACROS
          if (y < (memory_end(memory_number)+1)) {
            eeprom_byte_read = EEPROM.read(y);            // memory macros (backslash commands)
            switch (eeprom_byte_read) {
              case 48:                         // 0 - jump to memory 10
                eeprom_byte_read = 58;
              case 49:                         // 1 - jump to memory 1
              case 50:                         // 2 - jump to memory 2
              case 51:                         // 3 - jump to memory 3
              case 52:                         // 4 - jump to memory 4
              case 53:                         // 5 - jump to memory 5
              case 54:                         // 6 - jump to memory 6
              case 55:                         // 7 - jump to memory 7
              case 56:                         // 8 - jump to memory 8
              case 57:                         // 9 - jump to memory 9
                if (number_of_memories > (eeprom_byte_read-49)) {
                  memory_number = (eeprom_byte_read-49);
                  y = ((memory_start(memory_number)) - 1);
                  if (keyer_machine_mode == KEYER_NORMAL) {
                    primary_serial_port->println();
                    #ifdef FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
                      secondary_serial_port->println();
                    #endif //FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
                  }
                }
                break;
              case 'I': // insert memory #
                y++;
                if (y < (memory_end(memory_number)+1)) {  // get the next byte           
                 eeprom_byte_read = EEPROM.read(y);                 
                  if (number_of_memories > (eeprom_byte_read-49)) {
                    jump_back_to_y = y;
                    jump_back_to_memory_number = memory_number;
                    memory_number = (eeprom_byte_read-49);
                    y = ((memory_start(memory_number)) - 1);
                    if (keyer_machine_mode == KEYER_NORMAL) {
                      primary_serial_port->println();
                      #ifdef FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
                        secondary_serial_port->println();
                      #endif //FEATURE_COMMAND_LINE_INTERFACE_ON_SECONDARY_PORT
                    }
                  }       
                }           
                break;
          
              case 'S': // insert space
                send_char(' ',KEYER_NORMAL);
                break;

              case 88:                         // X - switch transmitter
                y++;
                if (y < (memory_end(memory_number)+1)) {
                  eeprom_byte_read2 = EEPROM.read(y);
                  if ((eeprom_byte_read2 > 48) && (eeprom_byte_read2 < 52)) {
                     switch (eeprom_byte_read2) {
                       case 49: switch_to_tx_silent(1); break;
                       case 50: if ((ptt_tx_2) || (tx_key_line_2)) {switch_to_tx_silent(2); } break;
                       case 51: if ((ptt_tx_3) || (tx_key_line_3)) {switch_to_tx_silent(3); } break;
                       case 52: if ((ptt_tx_4) || (tx_key_line_4)) {switch_to_tx_silent(4); } break;
                       case 53: if ((ptt_tx_5) || (tx_key_line_5)) {switch_to_tx_silent(5); } break;
                       case 54: if ((ptt_tx_6) || (tx_key_line_6)) {switch_to_tx_silent(6); } break;
                     }
                  }

                }
                break;  // case 84

              case 67:                       // C - play serial number with cut numbers T and N, then increment
                  serial_number_string = String(serial_number, DEC);
                  if (serial_number_string.length() < 3 ) {
                    send_char('T',KEYER_NORMAL);
                  }
                  if (serial_number_string.length() == 1) {
                    send_char('T',KEYER_NORMAL);
                  }
                  for (unsigned int a = 0; a < serial_number_string.length(); a++)  {
                    if (serial_number_string[a] == '0') {
                      send_char('T',KEYER_NORMAL);
                    } else {
                     if (serial_number_string[a] == '9') {
                       send_char('N',KEYER_NORMAL);
                     } else {
                       send_char(serial_number_string[a],KEYER_NORMAL);
                     }
                    }
                  }
                  serial_number++;
                break;

              case 68:                      // D - delay for ### seconds
                int_from_macro = 0;
                z = 100;
                input_error = 0;
                for (int x = 1; x < 4; x ++) {
                  y++;
                  if (y < (memory_end(memory_number)+1)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      int_from_macro = int_from_macro + ((eeprom_byte_read2 - 48) * z);
                      z = z / 10;
                    } else {
                      x = 4;           // error - exit
                      input_error = 1;
                      y--;             // go back one so we can at least play the errant character
                    }
                  } else {
                    x = 4;
                    input_error = 1;
                  }
                }
                if (input_error != 1) {   // do the delay
                  delay_result = memory_nonblocking_delay(int_from_macro*1000);
                }
                if (delay_result) {   // if a paddle or button0 was hit during the delay, exit
                  return 0;
                }
                break;  // case 68

              case 69:                       // E - play serial number, then increment
                  serial_number_string = String(serial_number, DEC);
                  for (unsigned int a = 0; a < serial_number_string.length(); a++)  {
                    send_char(serial_number_string[a],KEYER_NORMAL);
                  }
                  serial_number++;
                break;

              case 70:                       // F - change sidetone frequency
                int_from_macro = 0;
                z = 1000;
                input_error = 0;
                for (int x = 1; x < 5; x ++) {
                  y++;
                  if (y < (memory_end(memory_number)+1)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      int_from_macro = int_from_macro + ((eeprom_byte_read2 - 48) * z);
                      z = z / 10;
                    } else {
                      x = 5;           // error - exit
                      input_error = 1;
                      y--;             // go back one so we can at least play the errant character
                    }
                  }  else {
                    x = 4;
                    input_error = 1;
                  }
                }
                if ((input_error != 1) && (int_from_macro > SIDETONE_HZ_LOW_LIMIT) && (int_from_macro < SIDETONE_HZ_HIGH_LIMIT)) {
                  configuration.hz_sidetone = int_from_macro;
                }
                break;


              case 72:                       // H - Switch to Hell
                char_send_mode = HELL;
                break;

              case 76:                       // L - Switch to CW
                char_send_mode = CW;
                break;

              case 78:                       // N - decrement serial number (do not play)
                serial_number--;
                break;

              case 43:                       // + - Prosign
                prosign_flag = 1;
                break;

              case 81:                       // Q - QRSS mode and set dit length to ##
                int_from_macro = 0;
                z = 10;
                input_error = 0;
                for (int x = 1; x < 3; x ++) {
                  y++;
                  if (y < (memory_end(memory_number)+1)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      int_from_macro = int_from_macro + ((eeprom_byte_read2 - 48) * z);
                      z = z / 10;
                    } else {
                      x = 4;           // error - exit
                      input_error = 1;
                      y--;             // go back one so we can at least play the errant character
                    }
                  } else {
                    x = 4;
                    input_error = 1;
                  }
                }
                if (input_error == 0) {
                  speed_mode = SPEED_QRSS;
                  qrss_dit_length =  int_from_macro;
                  //calculate_element_length();
                }
              break;  //case 81

              case 82:                       // R - regular speed mode
                speed_mode = SPEED_NORMAL;
                //calculate_element_length();
              break;

              case 84:                      // T - transmit for ### seconds
                int_from_macro = 0;
                z = 100;
                input_error = 0;
                for (int x = 1; x < 4; x ++) {
                  y++;
                  if (y < (memory_end(memory_number)+1)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      int_from_macro = int_from_macro + ((eeprom_byte_read2 - 48) * z);
                      z = z / 10;
                    } else {
                      x = 4;           // error - exit
                      input_error = 1;
                      y--;             // go back one so we can at least play the errant character
                    }
                  } else {
                    x = 4;
                    input_error = 1;
                  }
                }
                if (input_error != 1) {   // go ahead and transmit
                  tx_and_sidetone_key(1,AUTOMATIC_SENDING);
                  delay_result = memory_nonblocking_delay(int_from_macro*1000);
                  tx_and_sidetone_key(0,AUTOMATIC_SENDING);
                }
                if (delay_result) {   // if a paddle or button0 was hit during the delay, exit
                  return 0;
                }
                break;  // case 84

              case 85:                      // U - turn on PTT
                manual_ptt_invoke = 1;
                ptt_key();
                break;

              case 86:                      // V - turn off PTT
                manual_ptt_invoke = 0;
                ptt_unkey();
                break;

              case 87:                      // W - change speed to ### WPM
                int_from_macro = 0;
                z = 100;
                input_error = 0;
                for (int x = 1; x < 4; x ++) {
                  y++;
                  if (y < (memory_end(memory_number)+1)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      int_from_macro = int_from_macro + ((eeprom_byte_read2 - 48) * z);
                      z = z / 10;
                    } else {
                      x = 4;           // error - exit
                      input_error = 1;
                      y--;             // go back one so we can at least play the errant character
                    }
                  }  else {
                    x = 4;
                    input_error = 1;
                  }
                }
                if (input_error != 1) {
                  speed_mode = SPEED_NORMAL;
                  speed_set(int_from_macro);
                }
                break;  // case 87

                case 89:                // Y - Relative WPM change (positive)
                  y++;
                  if ((y < (memory_end(memory_number)+1)) && (speed_mode == SPEED_NORMAL)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      speed_set(configuration.wpm + eeprom_byte_read2 - 48);
                    } else {
                      y--;             // go back one so we can at least play the errant character
                    }
                  } else {
                  }
                  break; // case 89

                case 90:                // Z - Relative WPM change (positive)
                  y++;
                  if ((y < (memory_end(memory_number)+1)) && (speed_mode == SPEED_NORMAL)) {
                    eeprom_byte_read2 = EEPROM.read(y);
                    if ((eeprom_byte_read2 > 47) && (eeprom_byte_read2 < 58)) {    // ascii 48-57 = "0" - "9")
                      speed_set(configuration.wpm - (eeprom_byte_read2 - 48));
                    } else {
                      y--;             // go back one so we can at least play the errant character
                    }
                  } else {
                  }
                  break; // case 90

            }

          }
          #endif //FEATURE_MEMORY_MACROS
        }
        if (keyer_machine_mode != BEACON) {
          #ifdef FEATURE_STRAIGHT_KEY
            if ((dit_buffer) || (dah_buffer) || (button0_buffer) || (digitalRead(pin_straight_key) == STRAIGHT_KEY_ACTIVE_STATE)) {   // exit if the paddle or button0 was hit
              dit_buffer = 0;
              dah_buffer = 0;
              button0_buffer = 0;
              repeat_memory = 255;
                while (ANALOGBUTTONREAD(0)) {}
              return 0;
            }
          #else //FEATURE_STRAIGHT_KEY
            if ((dit_buffer) || (dah_buffer) || (button0_buffer)) {   // exit if the paddle or button0 was hit
              dit_buffer = 0;
              dah_buffer = 0;
              button0_buffer = 0;
              repeat_memory = 255;
                while (ANALOGBUTTONREAD(0)) {}
              return 0;
            }
          #endif //FEATURE_STRAIGHT_KEY
        }

      } else {
        if (y == (memory_start(memory_number))) {      // memory is totally empty - do a boop
          repeat_memory = 255;
          NOTIFY_USER_BOOP("Memory empty", 0, default_display_msg_delay);
        }
        
        // if we had an inserted memory, jump back to the original one
        if (/*(y== (memory_end(memory_number)+1)) &&*/ (jump_back_to_y < 9999) && (jump_back_to_memory_number < 255)) {
          #ifdef DEBUG_PLAY_MEMORY
            debug_serial_port->print(F("\nplay_memory: jump back to original memory:"));
            debug_serial_port->println(jump_back_to_memory_number);
          #endif
          y = jump_back_to_y;
          memory_number = jump_back_to_memory_number;
          jump_back_to_y = 9999;
          jump_back_to_memory_number = 255;
        } else {        
        
        
        
         return 0;
        }
      }
    } else {
      if (pause_sending_buffer == 0) {
        y = (memory_end(memory_number)+1);   // we got a play_memory_prempt flag, exit out
      } else {
        y--;  // we're in a pause mode, so sit and spin awhile
      }
    }

    last_memory_repeat_time = millis();
    #ifdef DEBUG_PLAY_MEMORY
      debug_serial_port->println(F("\nplay_memory: reset last_memory_repeat_time"));  
      debug_serial_port->print("y: ");
      debug_serial_port->print(y);
      debug_serial_port->print("\tmemory_number: ");
      debug_serial_port->print(memory_number);
      debug_serial_port->print("\tmemory_end: ");
      debug_serial_port->print(memory_end(memory_number));
      debug_serial_port->print("\tjump_back_to_y: ");
      debug_serial_port->print(jump_back_to_y);
      debug_serial_port->print("\tjump_back_to_memory_number: ");
      debug_serial_port->println(jump_back_to_memory_number); 
    #endif
    
    
    // if we had an inserted memory, jump back to the original one
    /*
    if ((y== (memory_end(memory_number)+1)) && (jump_back_to_y < 99999) && (jump_back_to_memory_number < 255)) {
      primary_serial_port->print(F("\nplay_memory: jump back to original memory:"));
      primary_serial_port->println(jump_back_to_memory_number);
      y = jump_back_to_y;
      memory_number = jump_back_to_memory_number;
      jump_back_to_y = 99999;
      jump_back_to_memory_number = 255;
    }
    */
      

  }
  return 0;
}

//---------------------------------------------------------------------
void program_memory(int memory_number)
{

  if (memory_number > (number_of_memories-1)) {
    boop();
    return;
  }
  
  #ifdef FEATURE_DISPLAY
    String lcd_print_string;
    lcd_print_string.concat("Pgm Memory ");
    lcd_print_string.concat(memory_number+1);
    LCD_CENTER_PRINT_TIMED(lcd_print_string, 0, default_display_msg_delay);
  #endif

  send_dit(AUTOMATIC_SENDING);

  byte paddle_hit = 0;
  byte loop1 = 1;
  byte loop2 = 1;
  unsigned long last_element_time = 0;
  int memory_location_index = 0;
  long cwchar = 0;
  byte space_count = 0;
  
  #ifdef FEATURE_MEMORY_MACROS
    byte macro_flag = 0;
  #endif //FEATURE_MEMORY_MACROS
  
  #if defined(FEATURE_STRAIGHT_KEY)
    long straight_key_decoded_character = 0;
  #endif

  dit_buffer = 0;
  dah_buffer = 0;
  
  #if !defined(FEATURE_STRAIGHT_KEY)
    while ((paddle_pin_read(paddle_left) == HIGH) && (paddle_pin_read(paddle_right) == HIGH) && (!ANALOGBUTTONREAD(0))) { }  // loop until user starts sending or hits the button
  #else 
    while ((paddle_pin_read(paddle_left) == HIGH) && (paddle_pin_read(paddle_right) == HIGH) && (!ANALOGBUTTONREAD(0)) && (digitalRead(pin_straight_key) == HIGH)) { }  // loop until user starts sending or hits the button
  #endif

  while (loop2) {

    #ifdef DEBUG_MEMORY_WRITE
      debug_serial_port->println(F("program_memory: entering loop2\r"));
    #endif

    cwchar = 0;
    paddle_hit = 0;
    loop1 = 1;
    


    while (loop1) {
       check_paddles();
       if (dit_buffer) {
         send_dit(MANUAL_SENDING);
         dit_buffer = 0;
         paddle_hit = 1;
         cwchar = (cwchar * 10) + 1;
         last_element_time = millis();
         #ifdef DEBUG_MEMORY_WRITE
           debug_serial_port->write(".");
         #endif
       }
       if (dah_buffer) {
         send_dah(MANUAL_SENDING);
         dah_buffer = 0;
         paddle_hit = 1;
         cwchar = (cwchar * 10) + 2;
         last_element_time = millis();
         #ifdef DEBUG_MEMORY_WRITE
           debug_serial_port->write("_");
         #endif
       }
    
       #if defined(FEATURE_STRAIGHT_KEY)
         straight_key_decoded_character = service_straight_key();
         if (straight_key_decoded_character != 0){
           cwchar = straight_key_decoded_character;
           paddle_hit = 1;
         }
       #endif       
       
       #if !defined(FEATURE_STRAIGHT_KEY)
         if ((paddle_hit) && (millis() > (last_element_time + (float(600/configuration.wpm) * length_letterspace)))) {   // this character is over
           loop1 = 0;
         }
       #else
         if (((paddle_hit) && (millis() > (last_element_time + (float(600/configuration.wpm) * length_letterspace)))) || (straight_key_decoded_character != 0))  {   // this character is over
           loop1 = 0;
         }             
       #endif


// TODO - need to add something here to handle straight key leading space
       #ifdef FEATURE_MEMORY_MACROS
         if ((!macro_flag) && (paddle_hit == 0) && (millis() > (last_element_time + ((float(1200/configuration.wpm) * configuration.length_wordspace)))) && (space_count < program_memory_limit_consec_spaces)) {   // we have a space
           loop1 = 0;
           cwchar = 9;
           space_count++;
         }
       #else
         if ((paddle_hit == 0) && (millis() > (last_element_time + ((float(1200/configuration.wpm) * configuration.length_wordspace)))) && (space_count < program_memory_limit_consec_spaces)) {   // we have a space
           loop1 = 0;
           cwchar = 9;
           space_count++;
         }       
       #endif //FEATURE_MEMORY_MACROS

         while (ANALOGBUTTONREAD(0)) {    // hit the button to get out of command mode if no paddle was hit
           loop1 = 0;
           loop2 = 0;
         }
    }  //loop1

    if (cwchar != 9) {
      space_count = 0;
    }

    // write the character to memory
    if (cwchar > 0) {

      #ifdef DEBUG_MEMORY_WRITE
        debug_serial_port->print(F("program_memory: write_character_to_memory"));
        debug_serial_port->print(F(" mem number:"));
        debug_serial_port->print(memory_number);
        debug_serial_port->print(F("  memory_location_index:"));
        debug_serial_port->print(memory_location_index);
        debug_serial_port->print(F("  EEPROM location:"));
        debug_serial_port->print(memory_start(memory_number)+memory_location_index);
        debug_serial_port->print(F("   cwchar:"));
        debug_serial_port->print(cwchar);
        debug_serial_port->print(F("   ascii to write:"));
        debug_serial_port->println(convert_cw_number_to_ascii(cwchar));
      #endif

      EEPROM.write((memory_start(memory_number)+memory_location_index),convert_cw_number_to_ascii(cwchar));
      memory_location_index++;
 
      #ifdef FEATURE_MEMORY_MACROS
        if (!macro_flag) {
          if (convert_cw_number_to_ascii(cwchar) == '\\') {macro_flag = 1;}  // if we got the \ macro character, supress spaces
        } else {
           if (convert_cw_number_to_ascii(cwchar) == '+') {    // if we're building a prosign, supress the next two spaces
             macro_flag = 2; 
           } else {
             macro_flag--; 
           }
        }
      #endif //FEATURE_MEMORY_MACROS
    }

    // are we out of memory locations?
    if ((memory_start(memory_number) + memory_location_index) == memory_end(memory_number)) {
      loop1 = 0;
      loop2 = 0;
      #ifdef DEBUG_MEMORY_WRITE
        debug_serial_port->println(F("program_memory: out of memory location"));
      #endif
    }
  }

  //write terminating 255 at end
  #ifdef DEBUG_MEMORY_WRITE
    debug_serial_port->println(F("program_memory: writing memory termination"));
  #endif

  EEPROM.write((memory_start(memory_number) + memory_location_index),255);

  #ifdef OPTION_PROG_MEM_TRIM_TRAILING_SPACES
    for (int x = (memory_location_index-1); x > 0; x--) {
      if (EEPROM.read((memory_start(memory_number) + x)) == 32) {
        EEPROM.write((memory_start(memory_number) + x),255);
      } else {
        x = 0;
      }
    }
  #endif

    LCD_CENTER_PRINT_TIMED("Done", 0, default_display_msg_delay);

  play_memory(memory_number);

//  send_dit(AUTOMATIC_SENDING);

}

//---------------------------------------------------------------------
int memory_start(byte memory_number) {
  return (memory_area_start + (memory_number * ((memory_area_end - memory_area_start) / number_of_memories)));
}

//---------------------------------------------------------------------
int memory_end(byte memory_number) {
  return (memory_start(memory_number) - 1 + ((memory_area_end - memory_area_start)/number_of_memories));
}

#endif // FEATURE_MEMORIES
