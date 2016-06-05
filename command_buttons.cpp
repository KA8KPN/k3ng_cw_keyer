// TODO:  Put this in a file that everybody can include
#ifndef HARDWARE_CUSTOM
  #include "keyer_features_and_options.h"
#endif

#ifdef FEATURE_COMMAND_BUTTONS

#include <stdio.h>
#include <Arduino.h>

#include "keyer_dependencies.h"
#include "keyer.h"
#include "keyer_settings.h"
#include "keyer_pin_settings.h"

#include "config.h"

#include "command_buttons.h"
#include "potentiometer.h"
#include "display.h"
#include "rotary_encoder.h"

int button_array_high_limit[analog_buttons_number_of_buttons];
int button_array_low_limit[analog_buttons_number_of_buttons];
long button_last_add_to_send_buffer_time = 0;

void command_mode ()
{

  keyer_machine_mode = KEYER_COMMAND_MODE;
  
  #ifdef DEBUG_COMMAND_MODE
    debug_serial_port->println(F("command_mode: entering"));
  #endif
  
  byte looping;
  byte button_that_was_pressed = 0;
  byte paddle_hit = 0;
  unsigned long last_element_time = 0;
  int cw_char;
  byte stay_in_command_mode = 1;
  byte speed_mode_before = speed_mode;
  speed_mode = SPEED_NORMAL;                 // put us in normal speed mode (life is too short to do command mode in QRSS)
  byte keyer_mode_before = configuration.keyer_mode;
  if ((configuration.keyer_mode != IAMBIC_A) && (configuration.keyer_mode != IAMBIC_B)) {
    configuration.keyer_mode = IAMBIC_B;                   // we got to be in iambic mode (life is too short to make this work in bug mode)
  }

  command_mode_disable_tx = 0;

  boop_beep();
  #ifdef command_mode_active_led
    if (command_mode_active_led) {digitalWrite(command_mode_active_led,HIGH);}
  #endif //command_mode_active_led

  #ifdef FEATURE_DISPLAY
    lcd_clear();
    lcd_center_print_timed("Command Mode", 0, default_display_msg_delay);
  #endif 

  #if defined(FEATURE_WINKEY_EMULATION) && defined(OPTION_WINKEY_SEND_BREAKIN_STATUS_BYTE)
    winkey_breakin_status_byte_inhibit = 1;
  #endif

  while (stay_in_command_mode) {
    cw_char = 0;
 //   cw_char = get_cw_input_from_user(0);


    looping = 1;
    while (looping) {
      #ifdef FEATURE_POTENTIOMETER
        if (configuration.pot_activated) {
          check_potentiometer();
        }
      #endif
      
        CHECK_ROTARY_ENCODER();

      check_paddles();

      if (dit_buffer) {
        send_dit(MANUAL_SENDING);
        dit_buffer = 0;
        paddle_hit = 1;
        cw_char = (cw_char * 10) + 1;
        last_element_time = millis();
      }
      if (dah_buffer) {
        send_dah(MANUAL_SENDING);
        dah_buffer = 0;
        paddle_hit = 1;
        cw_char = (cw_char * 10) + 2;
        last_element_time = millis();
      }
      if ((paddle_hit) && (millis() > (last_element_time + (float(600/configuration.wpm) * length_letterspace)))) {
        #ifdef DEBUG_GET_CW_INPUT_FROM_USER
          debug_serial_port->println(F("get_cw_input_from_user: hit length_letterspace"));
        #endif
        looping = 0;
      }

      if (analogbuttonpressed() < analog_buttons_number_of_buttons){  // check for a button press
        looping = 0;
        cw_char = 9;
        delay(50);
        button_that_was_pressed = analogbuttonpressed();
        while (analogbuttonpressed() < analog_buttons_number_of_buttons) {}
      }

      #if defined(FEATURE_SERIAL)
        check_serial();
      #endif

    } //while (looping)



// end new code

    #ifdef DEBUG_COMMAND_MODE
      debug_serial_port->print(F("command_mode: cwchar: "));
      debug_serial_port->println(cw_char);
    #endif
    if (cw_char > 0) {              // do the command      
      switch (cw_char) {
        case 12: // A - Iambic mode
          configuration.keyer_mode = IAMBIC_A;
          keyer_mode_before = IAMBIC_A;
          config_dirty = 1;
          #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("Iambic A", 0, default_display_msg_delay);
          #endif
          send_dit(AUTOMATIC_SENDING);
          break; 
        case 2111: // B - Iambic mode
          configuration.keyer_mode = IAMBIC_B;
          keyer_mode_before = IAMBIC_B;
          config_dirty = 1;
          #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("Iambic B", 0, default_display_msg_delay);
          #endif          
          send_dit(AUTOMATIC_SENDING);
          break;
        case 1: // E - announce spEed
          char c[4];
          delay(250);
          sprintf(c, "%d", configuration.wpm);
          send_char(c[0],KEYER_NORMAL);
          send_char(c[1],KEYER_NORMAL);
          break; 
        case 211: // D - Ultimatic mode
          configuration.keyer_mode = ULTIMATIC;
          keyer_mode_before = ULTIMATIC;
          config_dirty = 1;
          #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("Ultimatic", 0, default_display_msg_delay);
          #endif                    
          send_dit(AUTOMATIC_SENDING);
          break; 
        case 1121: command_sidetone_freq_adj(); break;                    // F - adjust sidetone frequency
        case 221: // G - switch to buG mode
          configuration.keyer_mode = BUG;
          keyer_mode_before = BUG;
          config_dirty = 1;
          #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("Bug", 0, default_display_msg_delay);
          #endif          
          send_dit(AUTOMATIC_SENDING);
          break;  
        case 11:                                                     // I - toggle TX enable / disable
          if (command_mode_disable_tx) {
            command_mode_disable_tx = 0;
            #ifdef FEATURE_DISPLAY
              lcd_center_print_timed("TX On", 0, default_display_msg_delay);
            #endif            
          } else {
            command_mode_disable_tx = 1;
            #ifdef FEATURE_DISPLAY
              lcd_center_print_timed("TX Off", 0, default_display_msg_delay);
            #endif            
          }
          send_dit(AUTOMATIC_SENDING);
          break;
        case 1222: command_dah_to_dit_ratio_adjust(); break;                        // J - dah to dit ratio adjust
        #ifdef FEATURE_MEMORIES
          case 1221: command_program_memory(); break;                       // P - program a memory
        #endif //FEATURE_MEMORIES  Acknowledgement: LA3ZA fixed!
        case 21: // N - paddle mode toggle
          if (configuration.paddle_mode == PADDLE_NORMAL) {
            configuration.paddle_mode = PADDLE_REVERSE;
            #ifdef FEATURE_DISPLAY
              lcd_center_print_timed("Paddle Reverse", 0, default_display_msg_delay);
            #endif //FEATURE_DISPLAY
          } else {
            #ifdef FEATURE_DISPLAY
              lcd_center_print_timed("Paddle Normal", 0, default_display_msg_delay);
            #endif //FEATURE_DISPLAY         
            configuration.paddle_mode = PADDLE_NORMAL;
          }
          config_dirty = 1;
          send_dit(AUTOMATIC_SENDING);
          break;  
        case 222: // O - toggle sidetone on and off
          if ((configuration.sidetone_mode == SIDETONE_ON) || (configuration.sidetone_mode == SIDETONE_PADDLE_ONLY)) {
            #ifdef FEATURE_DISPLAY
              lcd_center_print_timed("Sidetone Off", 0, default_display_msg_delay);
            #endif 
            #ifdef DEBUG_COMMAND_MODE
              debug_serial_port->println(F("command_mode: SIDETONE_OFF"));
            #endif
            configuration.sidetone_mode = SIDETONE_OFF;
            //boop();
          } else {
            #ifdef FEATURE_DISPLAY
              lcd_center_print_timed("Sidetone On", 0, default_display_msg_delay);
            #endif 
            #ifdef DEBUG_COMMAND_MODE
              debug_serial_port->println(F("command_mode: SIDETONE_ON"));
            #endif             
            configuration.sidetone_mode = SIDETONE_ON;
            //beep();
          }
          config_dirty = 1;        
          send_dit(AUTOMATIC_SENDING);
          break; 
        case 2: command_tuning_mode(); break;                             // T - tuning mode
        #ifdef FEATURE_POTENTIOMETER
          case 1112:  // V - toggle pot active
            if (configuration.pot_activated) {
              configuration.pot_activated = 0; 
              #ifdef FEATURE_DISPLAY
                lcd_center_print_timed("Pot Deactivated", 0, default_display_msg_delay);
              #endif             
            } else {
              configuration.pot_activated = 1;
              #ifdef FEATURE_DISPLAY
                lcd_center_print_timed("Pot Activated", 0, default_display_msg_delay);
              #endif 
            }
            config_dirty = 1;
            send_dit(AUTOMATIC_SENDING);
            break; 
        #endif
        case 122: command_speed_mode(); break;                            // W - change wpm
        #ifdef FEATURE_MEMORIES
          case 2122: command_set_mem_repeat_delay(); break; // Y - set memory repeat delay
        #endif  
        case 2112: stay_in_command_mode = 0; break;     // X - exit command mode
        #ifdef FEATURE_AUTOSPACE
          case 2211: // Z - Autospace
            if (configuration.autospace_active) {
              configuration.autospace_active = 0;
              config_dirty = 1;
              #ifdef FEATURE_DISPLAY
                lcd_center_print_timed("Autospace Off", 0, default_display_msg_delay);
                send_dit(AUTOMATIC_SENDING);
              #else
                send_char('O',KEYER_NORMAL);
                send_char('F',KEYER_NORMAL);
                send_char('F',KEYER_NORMAL);
              #endif
            } else {
              configuration.autospace_active = 1;
              config_dirty = 1;
              #ifdef FEATURE_DISPLAY
                lcd_center_print_timed("Autospace On", 0, default_display_msg_delay);
                send_dit(AUTOMATIC_SENDING);
              #else            
                send_char('O',KEYER_NORMAL);
                send_char('N',KEYER_NORMAL);
              #endif
            }
            break;
        #endif
        #ifdef FEATURE_MEMORIES
          case 12222: play_memory(0); break;
          case 11222: play_memory(1); break;
          case 11122: play_memory(2); break;
          case 11112: play_memory(3); break;
          case 11111: play_memory(4); break;
        #endif
        #ifdef FEATURE_ALPHABET_SEND_PRACTICE
          case 111:
            send_dit(AUTOMATIC_SENDING); 
            command_alphabet_send_practice(); // S - Alphabet Send Practice
            stay_in_command_mode = 0;
            break;
        #endif  //FEATURE_ALPHABET_SEND_PRACTICE
        case 9: // button was hit
          #if defined(FEATURE_MEMORIES)
            if (button_that_was_pressed == 0){  // button 0 was hit - exit
              stay_in_command_mode = 0;
            } else {
              program_memory(button_that_was_pressed - 1); // a button other than 0 was pressed - program a memory
            }
          #else 
            stay_in_command_mode = 0;
          #endif
          break;                           
        default: // unknown command, send a ?
          #ifdef FEATURE_DISPLAY
            lcd_center_print_timed("Unknown command", 0, default_display_msg_delay);          
          #endif
          send_char('?',KEYER_NORMAL); 
          break;                                   
      }
    }
  }
  beep_boop();
  #if defined(FEATURE_WINKEY_EMULATION) && defined(OPTION_WINKEY_SEND_BREAKIN_STATUS_BYTE)
    winkey_breakin_status_byte_inhibit = 0;
  #endif  

  #ifdef command_mode_active_led
    if (command_mode_active_led) {digitalWrite(command_mode_active_led,LOW);}
  #endif //command_mode_active_led

  keyer_machine_mode = KEYER_NORMAL;

  speed_mode = speed_mode_before;   // go back to whatever speed mode we were in before
  configuration.keyer_mode = keyer_mode_before;

  #ifdef DEBUG_COMMAND_MODE
    if (command_mode_disable_tx) {
      debug_serial_port->print(F("command_mode: command_mode_disable_tx set"));
    }
  #endif //DEBUG_COMMAND_MODE
}

//-------------------------------------------------------------------------------------------------------

void command_dah_to_dit_ratio_adjust () {

  byte looping = 1;

  #ifdef FEATURE_DISPLAY
  lcd_center_print_timed("Adj dah to dit", 0, default_display_msg_delay);          
  #endif

  while (looping) {
   send_dit(AUTOMATIC_SENDING);
   send_dah(AUTOMATIC_SENDING);
   if (paddle_pin_read(paddle_left) == LOW) {
     adjust_dah_to_dit_ratio(10);
   }
   if (paddle_pin_read(paddle_right) == LOW) {
     adjust_dah_to_dit_ratio(-10);
   }
   while ((paddle_pin_read(paddle_left) == LOW && paddle_pin_read(paddle_right) == LOW) || (analogbuttonread(0))) { // if paddles are squeezed or button0 pressed - exit
     looping = 0;
   }
   
  }
  while (paddle_pin_read(paddle_left) == LOW || paddle_pin_read(paddle_right) == LOW || analogbuttonread(0) ) {}  // wait for all lines to go high
  dit_buffer = 0;
  dah_buffer = 0;
}

//-------------------------------------------------------------------------------------------------------

void command_tuning_mode() {

  byte looping = 1;
  byte latched = 0;
  
  
  #ifdef FEATURE_DISPLAY
  lcd_center_print_timed("Tune Mode", 0, default_display_msg_delay);          
  #endif  
  
  send_dit(AUTOMATIC_SENDING);
  key_tx = 1;
  while (looping) {

    if (paddle_pin_read(paddle_left) == LOW) {
      tx_and_sidetone_key(1,MANUAL_SENDING);
      ptt_key();
      latched = 0;
    } else {
       if (paddle_pin_read(paddle_left) == HIGH && latched == 0) {
         tx_and_sidetone_key(0,MANUAL_SENDING);
         ptt_unkey();
       }
    }

    if (paddle_pin_read(paddle_right) == LOW && latched == 0) {
      latched = 1;
      tx_and_sidetone_key(1,MANUAL_SENDING);
      ptt_key();
      while ((paddle_pin_read(paddle_right) == LOW) && (paddle_pin_read(paddle_left) == HIGH)) {
        delay(10);
      }
    } else {
      if ((paddle_pin_read(paddle_right) == LOW) && (latched)) {
        latched = 0;
        tx_and_sidetone_key(0,MANUAL_SENDING);
        ptt_unkey();
        while ((paddle_pin_read(paddle_right) == LOW) && (paddle_pin_read(paddle_left) == HIGH)) {
          delay(10);
        }
      }
    }
   if ((analogbuttonread(0)) || ((paddle_pin_read(paddle_left) == LOW) && (paddle_pin_read(paddle_right) == LOW))) { // if paddles are squeezed or button0 pressed - exit
     looping = 0;
   }
   
  }
  tx_and_sidetone_key(0,MANUAL_SENDING);
  ptt_unkey();
  while (paddle_pin_read(paddle_left) == LOW || paddle_pin_read(paddle_right) == LOW || analogbuttonread(0) ) {}  // wait for all lines to go high
  key_tx = 0;
  send_dit(AUTOMATIC_SENDING);
  dit_buffer = 0;
  dah_buffer = 0;
}


//-------------------------------------------------------------------------------------------------------
void command_sidetone_freq_adj() {

  byte looping = 1;

  #ifdef FEATURE_DISPLAY
  lcd_center_print_timed("Sidetone " + String(configuration.hz_sidetone) + " Hz", 0, default_display_msg_delay);   
  #endif

  while (looping) {
    tone(sidetone_line, configuration.hz_sidetone);
    if (paddle_pin_read(paddle_left) == LOW) {
      #ifdef FEATURE_DISPLAY
      sidetone_adj(5);      
      lcd_center_print_timed("Sidetone " + String(configuration.hz_sidetone) + " Hz", 0, default_display_msg_delay);        
      #else
      sidetone_adj(1);
      #endif
      delay(10);
    }
    if (paddle_pin_read(paddle_right) == LOW) {
      #ifdef FEATURE_DISPLAY
      sidetone_adj(-5);
      lcd_center_print_timed("Sidetone " + String(configuration.hz_sidetone) + " Hz", 0, default_display_msg_delay);       
      #else
      sidetone_adj(-1);
      #endif
      delay(10);
    }
    while ((paddle_pin_read(paddle_left) == LOW && paddle_pin_read(paddle_right) == LOW) || (analogbuttonread(0))) { // if paddles are squeezed or button0 pressed - exit
      looping = 0;
    }
    

  }
  while (paddle_pin_read(paddle_left) == LOW || paddle_pin_read(paddle_right) == LOW || analogbuttonread(0) ) {}  // wait for all lines to go high
  noTone(sidetone_line);

}

//-------------------------------------------------------------------------------------------------------

void command_speed_mode()
{

  byte looping = 1;
  String wpm_string;
  
  #ifdef FEATURE_DISPLAY
  lcd_center_print_timed("Adjust Speed", 0, default_display_msg_delay);        
  #endif
  

  while (looping) {
    send_dit(AUTOMATIC_SENDING);
    if ((paddle_pin_read(paddle_left) == LOW)) {
      speed_change(1);
    }
    if ((paddle_pin_read(paddle_right) == LOW)) {
      speed_change(-1);
    }
    while ((paddle_pin_read(paddle_left) == LOW && paddle_pin_read(paddle_right) == LOW) || (analogbuttonread(0) ))  // if paddles are squeezed or button0 pressed - exit
    {
      looping = 0;
    }

  }
  while (paddle_pin_read(paddle_left) == LOW || paddle_pin_read(paddle_right) == LOW || analogbuttonread(0) ) {}  // wait for all lines to go high
  #ifndef FEATURE_DISPLAY
  // announce speed in CW
  wpm_string = String(configuration.wpm, DEC);
  send_char(wpm_string[0],KEYER_NORMAL);
  send_char(wpm_string[1],KEYER_NORMAL);
  #endif

  dit_buffer = 0;
  dah_buffer = 0;

}


//------------------------------------------------------------------

byte analogbuttonpressed() {

  int analog_line_read_average = 0;
  int analog_read_temp = 0;
  
  #if !defined(OPTION_REVERSE_BUTTON_ORDER)
    if (analogRead(analog_buttons_pin) <= button_array_high_limit[analog_buttons_number_of_buttons-1]) {
      
      for (byte x = 0;x < 19;x++){
        analog_read_temp = analogRead(analog_buttons_pin);
        if (analog_read_temp <= button_array_high_limit[analog_buttons_number_of_buttons-1]){
          analog_line_read_average = (analog_line_read_average + analog_read_temp) / 2;
        }
      }
      
      for (int x = 0;x < analog_buttons_number_of_buttons;x++) {
        if ((analog_line_read_average > button_array_low_limit[x]) && (analog_line_read_average <=  button_array_high_limit[x])) {
          #ifdef DEBUG_BUTTONS
          //if (!debug_flag) {
            debug_serial_port->print(F(" analogbuttonpressed: returning: "));
            debug_serial_port->println(x);
          //  debug_flag = 1;
          //}
          #endif         
          return x;
        }  
      }    
      
    }

  #else //OPTION_REVERSE_BUTTON_ORDER

    if (analogRead(analog_buttons_pin) <= button_array_high_limit[0]) {
      \
      for (byte x = 0;x < 19;x++){
        analog_read_temp = analogRead(analog_buttons_pin);
        if (analog_read_temp <= button_array_high_limit[0]){
          analog_line_read_average = (analog_line_read_average + analog_read_temp) / 2;
        }
      }
      
      #ifdef DEBUG_BUTTONS
      debug_serial_port->print(F(" analogbuttonpressed: analog_line_read_average: "));
      debug_serial_port->println(analog_line_read_average);
      #endif 

      for (int x = 0;x < analog_buttons_number_of_buttons;x++) {
        if ((analog_line_read_average > button_array_low_limit[x]) && (analog_line_read_average <=  button_array_high_limit[x])) {
          #ifdef DEBUG_BUTTONS
          //if (!debug_flag) {
            debug_serial_port->print(F(" analogbuttonpressed: returning: "));
            debug_serial_port->println(x);
          //  debug_flag = 1;
          //}
          #endif         
          return x;
        }  
      }    
      
    }

  #endif //OPTION_REVERSE_BUTTON_ORDER
  

  /*

  int analog_line_read = analogRead(analog_buttons_pin);
  
  static byte samplecounts = 0;
  static int running_analog_line_read_average = 0;
  
  #ifdef DEBUG_BUTTONS
  static byte debug_flag = 0;
  #endif
  
  if (analog_line_read < 1000) {

    running_analog_line_read_average = running_analog_line_read_average + analog_line_read;
    samplecounts++;
    if (samplecounts > 19) {        
      analog_line_read = running_analog_line_read_average / samplecounts;
      
      #ifdef DEBUG_BUTTONS
      if (!debug_flag) {
        primary_serial_port->print(F("\nanalogbuttonpressed: analog_line_read: "));
        primary_serial_port->print(analog_line_read);
        primary_serial_port->print(F(" samplecounts: "));
        primary_serial_port->print(samplecounts);
      }
      #endif        
      
      for (int x = 0;x < analog_buttons_number_of_buttons;x++) {
        if ((analog_line_read > button_array_low_limit[x]) && (analog_line_read <=  button_array_high_limit[x])) {
          #ifdef DEBUG_BUTTONS
          if (!debug_flag) {
            primary_serial_port->print(F(" analogbuttonpressed: returning: "));
            primary_serial_port->println(x);
            debug_flag = 1;
          }
          #endif
          samplecounts = 0;
          running_analog_line_read_average = 0;          
          return x;
        }  
      }
    }  //(samplecounts > 9)
  } else {  //(analog_line_read < 1000)
    samplecounts = 0;
    running_analog_line_read_average = 0;
  }
   
  #ifdef DEBUG_BUTTONS
  debug_flag = 0;
  #endif
  
  
  */
  
  return 255; 
}
  
//------------------------------------------------------------------

byte analogbuttonread(byte button_number) {
 
  // button numbers start with 0
  
  int analog_line_read = analogRead(analog_buttons_pin);

  #ifdef DEBUG_BUTTONS
  static byte debug_flag = 0;
  #endif
  
  if (analog_line_read < 1000) {  
    if ((analog_line_read > button_array_low_limit[button_number])&& (analog_line_read <  button_array_high_limit[button_number])) {
      #ifdef DEBUG_BUTTONS
      if (!debug_flag) {
        debug_serial_port->print(F("\nanalogbuttonread: analog_line_read: "));
        debug_serial_port->print(analog_line_read);
        debug_serial_port->print(F("  button pressed: "));
        debug_serial_port->println(button_number);
        debug_flag = 1;
      }
      #endif
      return 1;
    }  
  }
  #ifdef DEBUG_BUTTONS
  debug_flag = 0;
  #endif  
  return 0;
}

//------------------------------------------------------------------

void check_command_buttons()
{

  #ifdef DEBUG_LOOP
    debug_serial_port->println(F("loop: entering check_buttons"));
  #endif

  static long last_button_action = 0;
  byte analogbuttontemp = analogbuttonpressed();
  long button_depress_time;
  byte paddle_was_hit = 0;
  byte store_key_tx = key_tx;
  byte previous_sidetone_mode = 0;
  if ((analogbuttontemp < analog_buttons_number_of_buttons) && ((millis() - last_button_action) > 200)) {
    #ifdef FEATURE_MEMORIES
      repeat_memory = 255;
    #endif
    button_depress_time = millis();
    while ((analogbuttontemp == analogbuttonpressed()) && ((millis() - button_depress_time) < 1000)) {
      if ((paddle_pin_read(paddle_left) == LOW) || (paddle_pin_read(paddle_right) == LOW)) {
        button_depress_time = 1001;  // if button 0 is held and a paddle gets hit, assume we have a hold and shortcut out
      }
    }
    if ((millis() - button_depress_time) < 500) {
      if (analogbuttontemp == 0) {
        key_tx = 0;
        command_mode();
        if (command_mode_disable_tx) {
          key_tx = !store_key_tx;
        } else {
          key_tx = 1;
        }
      }
      #ifdef FEATURE_MEMORIES
      if ((analogbuttontemp > 0) && (analogbuttontemp < (number_of_memories + 1)) && ((millis() - button_last_add_to_send_buffer_time) > 400)) {
        
        #ifdef FEATURE_WINKEY_EMULATION
        #ifndef OPTION_WINKEY_2_SUPPORT
        add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
        add_to_send_buffer(analogbuttontemp - 1);
        #else //OPTION_WINKEY_2_SUPPORT
        if ((winkey_host_open) && (wk2_mode == 2)) {   // if winkey is open and in wk2 mode, tell it about the button press
          byte winkey_byte_to_send = 0xc8;
          switch(analogbuttontemp) {
            case 1: winkey_byte_to_send = winkey_byte_to_send | 1; break;
            case 2: winkey_byte_to_send = winkey_byte_to_send | 2; break;
            case 3: winkey_byte_to_send = winkey_byte_to_send | 4; break;
            case 4: winkey_byte_to_send = winkey_byte_to_send | 16; break;            
          } 
          winkey_port_write(winkey_byte_to_send);
          winkey_port_write(0xc8); // tell it that the button is unpressed
        } else {  // otherwise, have the buttons act as normal
          add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
          add_to_send_buffer(analogbuttontemp - 1);
        }  
        #endif //OPTION_WINKEY_2_SUPPORT
        #else //FEATURE_WINKEY_EMULATION
        add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
        add_to_send_buffer(analogbuttontemp - 1);
        #endif //FEATURE_WINKEY_EMULATION

        button_last_add_to_send_buffer_time = millis();
        #ifdef DEBUG_BUTTONS
        debug_serial_port->print(F("\ncheck_buttons: add_to_send_buffer: "));
        debug_serial_port->println(analogbuttontemp - 1);
        #endif //DEBUG_BUTTONS
      }
      #endif
    } else {
//      if ((millis() - button_depress_time) < 1000) {
//        if ((analogbuttontemp > 0) && (analogbuttontemp < 7)) {
//          key_tx = 0;
//          switch_to_tx(analogbuttontemp);
//          key_tx = 1;
//        }
//      } else {  // we got a button hold
        if (analogbuttontemp == 0) {
          key_tx = 0;
          // do stuff if this is a command button hold down
          while (analogbuttonpressed() == 0) {
            if (paddle_pin_read(paddle_left) == LOW) {                     // left paddle increase speed
              speed_change(1);
              previous_sidetone_mode = configuration.sidetone_mode;
              configuration.sidetone_mode = SIDETONE_ON; 
              send_dit(MANUAL_SENDING);
              configuration.sidetone_mode = previous_sidetone_mode;
              //speed_button_cmd_executed = 1;
              dit_buffer = 0;
              
              #ifdef DEBUG_BUTTONS
              debug_serial_port->println(F("\ncheck_buttons: speed_change(1)"));
              #endif //DEBUG_BUTTONS            

              #ifdef FEATURE_WINKEY_EMULATION
              #ifdef FEATURE_POTENTIOMETER
              if ((primary_serial_port_mode == SERIAL_WINKEY_EMULATION) && (winkey_host_open)) {
                winkey_port_write(((configuration.wpm-pot_wpm_low_value)|128));
                winkey_last_unbuffered_speed_wpm = configuration.wpm;
              }
              #endif
              #endif

            }
            if (paddle_pin_read(paddle_right) == LOW) {                    // right paddle decreases speed
              speed_change(-1);
              previous_sidetone_mode = configuration.sidetone_mode;
              configuration.sidetone_mode = SIDETONE_ON; 
              send_dah(MANUAL_SENDING);
              configuration.sidetone_mode = previous_sidetone_mode;              
              //speed_button_cmd_executed = 1;
              dah_buffer = 0;

              #ifdef DEBUG_BUTTONS
              debug_serial_port->println(F("\ncheck_buttons: speed_change(-1)"));
              #endif //DEBUG_BUTTONS            

              #ifdef FEATURE_WINKEY_EMULATION
              #ifdef FEATURE_POTENTIOMETER
              if ((primary_serial_port_mode == SERIAL_WINKEY_EMULATION) && (winkey_host_open)) {
                winkey_port_write(((configuration.wpm-pot_wpm_low_value)|128));
                winkey_last_unbuffered_speed_wpm = configuration.wpm;
              }
              #endif
              #endif
            }
         }
         key_tx = 1;
       }  //(analogbuttontemp == 0)
       if ((analogbuttontemp > 0) && (analogbuttontemp < analog_buttons_number_of_buttons)) {
         while (analogbuttonpressed() == analogbuttontemp) {
            if (((paddle_pin_read(paddle_left) == LOW) || (paddle_pin_read(paddle_right) == LOW)) && (analogbuttontemp < (number_of_memories + 1))){
              #ifdef FEATURE_MEMORIES
              repeat_memory = analogbuttontemp - 1;
              last_memory_repeat_time = 0;
              #ifdef DEBUG_BUTTONS
              debug_serial_port->print(F("\ncheck_buttons: repeat_memory:"));
              debug_serial_port->println(repeat_memory);
              #endif //DEBUG_BUTTONS                    
              #endif
              paddle_was_hit = 1;
            }
         }
         if (!paddle_was_hit) {  // if no paddle was hit, this was a button hold to change transmitters
             key_tx = 0;
             previous_sidetone_mode = configuration.sidetone_mode;
             configuration.sidetone_mode = SIDETONE_ON;
             switch_to_tx(analogbuttontemp);
             key_tx = 1;
             configuration.sidetone_mode = previous_sidetone_mode;
         }
       }
     //} // button hold
    }
    last_button_action = millis();
    #ifdef FEATURE_SLEEP
    last_activity_time = millis(); 
    #endif //FEATURE_SLEEP
  }
}

//---------------------------------------------------------------------
void check_button0()
{
    if (analogbuttonread(0)) {button0_buffer = 1;}
}

//------------------------------------------------------------------

#ifdef FEATURE_MEMORIES
void check_the_memory_buttons()
{

  byte analogbuttontemp = analogbuttonpressed();
  if ((analogbuttontemp > 0) && (analogbuttontemp < (number_of_memories + 1)) && ((millis() - button_last_add_to_send_buffer_time) > 400)) {
    add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
    add_to_send_buffer(analogbuttontemp - 1);
    button_last_add_to_send_buffer_time = millis();
  }
}


//---------------------------------------------------------------------

void command_program_memory()
{
  int cw_char;
  cw_char = get_cw_input_from_user(0);            // get another cw character from the user to find out which memory number
  #ifdef DEBUG_COMMAND_MODE
  debug_serial_port->print(F("command_program_memory: cw_char: "));
  debug_serial_port->println(cw_char);
  #endif
  if (cw_char > 0) {
    if ((cw_char == 12222) && (number_of_memories > 9)) { // we have a 1, this could be 1 or 1x
      cw_char = get_cw_input_from_user((1200/configuration.wpm)*14);  // give the user some time to enter a second digit
      switch (cw_char) {
        case 0: program_memory(0); break;    // we didn't get anything, it's a 1   
        case 22222: program_memory(9); break; 
        case 12222: program_memory(10); break;
        case 11222: program_memory(11); break;
        case 11122: program_memory(12); break;
        case 11112: program_memory(13); break;
        case 11111: program_memory(14); break;
        case 21111: program_memory(15); break;
        default: send_char('?',KEYER_NORMAL); break;
      }
    } else {    
      switch (cw_char) {
        case 12222: program_memory(0); break;      // 1 = memory 0
        case 11222: program_memory(1); break;
        case 11122: program_memory(2); break;
        case 11112: program_memory(3); break;
        case 11111: program_memory(4); break;
        case 21111: program_memory(5); break;
        case 22111: program_memory(6); break;
        case 22211: program_memory(7); break;
        case 22221: program_memory(8); break;
        //case 22222: program_memory(9); break;
        default: send_char('?',KEYER_NORMAL); break;
      }
    }
  }
}
#endif  // FEATURE_MEMORIES


#ifdef FEATURE_DL2SBA_BANKSWITCH
void setOneButton(int button, int index) { 
    int button_value = int(1023 * (float(button * analog_buttons_r2)/float((button * analog_buttons_r2) + analog_buttons_r1))); 
    int lower_button_value = int(1023 * (float((button-1) * analog_buttons_r2)/float(((button-1) * analog_buttons_r2) + analog_buttons_r1))); 
    int higher_button_value = int(1023 * (float((button+1) * analog_buttons_r2)/float(((button+1) * analog_buttons_r2) + analog_buttons_r1))); 
    button_array_low_limit[index] = (button_value - ((button_value - lower_button_value)/2)); 
    button_array_high_limit[index] = (button_value + ((higher_button_value - button_value)/2)); 
}
#endif

//------------------------------------------------------------------

void initialize_analog_button_array() {
  /* 
  
  typical button values:
  
    0: -56 - 46
    1: 47 - 131
    2: 132 - 203
    3: 203 - 264
    
  */
  

  
  #ifndef FEATURE_DL2SBA_BANKSWITCH
  
  int button_value;
  int lower_button_value;
  int higher_button_value;

  #ifdef OPTION_REVERSE_BUTTON_ORDER
  byte y = analog_buttons_number_of_buttons - 1;
  #endif

  for (int x = 0;x < analog_buttons_number_of_buttons;x++) {
    button_value = int(1023 * (float(x * analog_buttons_r2)/float((x * analog_buttons_r2) + analog_buttons_r1)));
    lower_button_value = int(1023 * (float((x-1) * analog_buttons_r2)/float(((x-1) * analog_buttons_r2) + analog_buttons_r1)));
    higher_button_value = int(1023 * (float((x+1) * analog_buttons_r2)/float(((x+1) * analog_buttons_r2) + analog_buttons_r1)));
    #ifndef OPTION_REVERSE_BUTTON_ORDER
    button_array_low_limit[x] = (button_value - ((button_value - lower_button_value)/2));
    button_array_high_limit[x] = (button_value + ((higher_button_value - button_value)/2));
    #else
    button_array_low_limit[y] = (button_value - ((button_value - lower_button_value)/2));
    button_array_high_limit[y] = (button_value + ((higher_button_value - button_value)/2));
    y--;
    #endif

    #ifdef DEBUG_BUTTON_ARRAY    
    debug_serial_port->print("initialize_analog_button_array: ");
    debug_serial_port->print(x);
    debug_serial_port->print(": ");
    debug_serial_port->print(button_array_low_limit[x]);
    debug_serial_port->print(" - ");
    debug_serial_port->println(button_array_high_limit[x]);
    #endif //DEBUG_BUTTON_ARRAY


  }
  
  #else //FEATURE_DL2SBA_BANKSWITCH
  
  setOneButton(0,0); 
  setOneButton(1,3); 
  setOneButton(2,2); 
  setOneButton(3,1); 
  setOneButton(4,9); 
  setOneButton(5,8); 
  setOneButton(6,7); 
  setOneButton(7,6); 
  setOneButton(8,5); 
  setOneButton(9,4); 
      
  #endif //FEATURE_DL2SBA_BANKSWITCH
}


#endif // FEATURE_COMMAND_BUTTONS
