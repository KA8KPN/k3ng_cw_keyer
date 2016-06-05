// TODO:  Put this in a file that everybody can include
#ifndef HARDWARE_CUSTOM
  #include "keyer_features_and_options.h"
#endif

#include "keyer_dependencies.h"

#ifdef FEATURE_PS2_KEYBOARD

#include <stdio.h>
#include <Arduino.h>

#include "ps2_keyboard.h"
#include "potentiometer.h"
#include "rotary_encoder.h"
#include "display.h"

#include "keyer_pin_settings.h"
#include "keyer_settings.h"
#include "keyer.h"
#include "config.h"
#include <EEPROM.h>

byte ps2_keyboard_mode = PS2_KEYBOARD_NORMAL;
byte ps2_keyboard_command_buffer[25];
byte ps2_keyboard_command_buffer_pointer = 0;


#if defined(FEATURE_PS2_KEYBOARD)
  #ifdef OPTION_USE_ORIGINAL_VERSION_2_1_PS2KEYBOARD_LIB
    PS2Keyboard keyboard;
  #else //OPTION_USE_ORIGINAL_VERSION_2_1_PS2KEYBOARD_LIB
  K3NG_PS2Keyboard keyboard;
  #endif //OPTION_USE_ORIGINAL_VERSION_2_1_PS2KEYBOARD_LIB
#endif


//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_PS2_KEYBOARD
void check_ps2_keyboard()
{

  #ifdef DEBUG_LOOP
  debug_serial_port->println(F("loop: entering check_ps2_keyboard"));
  #endif    
  
  static byte keyboard_tune_on = 0;
  static byte ps2_prosign_flag = 0;
  int work_int = 0;
  uint8_t keystroke = 0;
  
  /* NOTE!!! This entire block of code is repeated again below the #else.  This was done to fix a bug with Notepad++ not
             collapsing code correctly when while() statements are encapsulated in #ifdef/#endifs.                        */
  
  #ifdef FEATURE_MEMORIES
  while ((keyboard.available()) && (play_memory_prempt == 0)) {      
    
    // read the next key
    keystroke = keyboard.read();

    #if defined(DEBUG_PS2_KEYBOARD)
    debug_serial_port->print("check_ps2_keyboard: keystroke: ");
    debug_serial_port->println(keystroke,DEC);
    #endif //DEBUG_PS2_KEYBOARD
    
    #ifdef FEATURE_SLEEP
    last_activity_time = millis(); 
    #endif //FEATURE_SLEEP

    if (ps2_keyboard_mode == PS2_KEYBOARD_NORMAL) {
      switch (keystroke) {
        case PS2_PAGEUP : sidetone_adj(20); break;
        case PS2_PAGEDOWN : sidetone_adj(-20); break;
        case PS2_RIGHTARROW : adjust_dah_to_dit_ratio(int(configuration.dah_to_dit_ratio/10)); break;
        case PS2_LEFTARROW : adjust_dah_to_dit_ratio(-1*int(configuration.dah_to_dit_ratio/10)); break;
        case PS2_UPARROW : speed_set(configuration.wpm+1); break;
        case PS2_DOWNARROW : speed_set(configuration.wpm-1); break;
        case PS2_HOME :
          configuration.dah_to_dit_ratio = initial_dah_to_dit_ratio;
          key_tx = 1;
          config_dirty = 1;
          #ifdef OPTION_MORE_DISPLAY_MSGS
          LCD_CENTER_PRINT_TIMED("Default ratio", 0, default_display_msg_delay);
          SERVICE_DISPLAY();
          #endif           
          break;
        case PS2_TAB :
          if (pause_sending_buffer) {
            pause_sending_buffer = 0;
            #ifdef OPTION_MORE_DISPLAY_MSGS
            LCD_CENTER_PRINT_TIMED("Resume", 0, default_display_msg_delay);
            #endif                 
          } else {
            pause_sending_buffer = 1;
            LCD_CENTER_PRINT_TIMED("Pause", 0, default_display_msg_delay);
          }
        break;  // pause

        case PS2_SCROLL :   // Prosign next two characters
          ps2_prosign_flag = 1;
          #ifdef OPTION_MORE_DISPLAY_MSGS
          LCD_CENTER_PRINT_TIMED("Prosign", 0, default_display_msg_delay);
          #endif          
          break;

        #ifdef FEATURE_MEMORIES
        case PS2_F1 : ps2_usb_keyboard_play_memory(0); break;
        case PS2_F2 : ps2_usb_keyboard_play_memory(1); break;
        case PS2_F3 : ps2_usb_keyboard_play_memory(2); break;
        #ifndef OPTION_SAVE_MEMORY_NANOKEYER
        case PS2_F4 : ps2_usb_keyboard_play_memory(3); break;
        case PS2_F5 : ps2_usb_keyboard_play_memory(4); break;
        case PS2_F6 : ps2_usb_keyboard_play_memory(5); break;
        case PS2_F7 : ps2_usb_keyboard_play_memory(6); break;
        case PS2_F8 : ps2_usb_keyboard_play_memory(7); break;
        case PS2_F9 : ps2_usb_keyboard_play_memory(8); break;
        case PS2_F10 : ps2_usb_keyboard_play_memory(9); break;
        case PS2_F11 : ps2_usb_keyboard_play_memory(10); break;
        case PS2_F12 : ps2_usb_keyboard_play_memory(11); break;
        #endif //OPTION_SAVE_MEMORY_NANOKEYER
        case PS2_F1_ALT : if (number_of_memories > 0) {repeat_memory_msg(0);} break;
        case PS2_F2_ALT : if (number_of_memories > 1) {repeat_memory_msg(1);} break;
        case PS2_F3_ALT : if (number_of_memories > 2) {repeat_memory_msg(2);} break;
        #ifndef OPTION_SAVE_MEMORY_NANOKEYER
        case PS2_F4_ALT : if (number_of_memories > 3) {repeat_memory_msg(3);} break;
        case PS2_F5_ALT : if (number_of_memories > 4) {repeat_memory_msg(4);} break;
        case PS2_F6_ALT : if (number_of_memories > 5) {repeat_memory_msg(5);} break;
        case PS2_F7_ALT : if (number_of_memories > 6) {repeat_memory_msg(6);} break;
        case PS2_F8_ALT : if (number_of_memories > 7) {repeat_memory_msg(7);} break;
        case PS2_F9_ALT : if (number_of_memories > 8) {repeat_memory_msg(8);} break;
        case PS2_F10_ALT : if (number_of_memories > 9) {repeat_memory_msg(9);} break;
        case PS2_F11_ALT : if (number_of_memories > 10) {repeat_memory_msg(10);} break;
        case PS2_F12_ALT : if (number_of_memories > 11) {repeat_memory_msg(11);} break;
        #endif //OPTION_SAVE_MEMORY_NANOKEYER
        #endif //FEATURE_MEMORIES

        case PS2_DELETE : if (send_buffer_bytes > 0) { send_buffer_bytes--; } break;
        case PS2_ESC :  // clear the serial send buffer and a bunch of other stuff
          if (manual_ptt_invoke) {
            manual_ptt_invoke = 0;
            ptt_unkey();
          }
          if (keyboard_tune_on) {
            tx_and_sidetone_key(0,MANUAL_SENDING);
            keyboard_tune_on = 0;
          }
          if (pause_sending_buffer) {
            pause_sending_buffer = 0;
          }
          clear_send_buffer();
          #ifdef FEATURE_MEMORIES
          //clear_memory_button_buffer();
          play_memory_prempt = 1;
          repeat_memory = 255;
          #endif
          LCD_CENTER_PRINT_TIMED("Abort", 0, default_display_msg_delay);
          break;

        #ifdef FEATURE_MEMORIES
        case PS2_F1_SHIFT  :
          ps2_keyboard_program_memory(0);
          break;

        case PS2_F2_SHIFT  :
          ps2_keyboard_program_memory(1);
          break;

        case PS2_F3_SHIFT  :
          ps2_keyboard_program_memory(2);
          break;


        #ifndef OPTION_SAVE_MEMORY_NANOKEYER
        case PS2_F4_SHIFT  :
          ps2_keyboard_program_memory(3);
          break;

        case PS2_F5_SHIFT  :
          ps2_keyboard_program_memory(4);
          break;

        case PS2_F6_SHIFT  :
          ps2_keyboard_program_memory(5);
          break;

        case PS2_F7_SHIFT  :
          ps2_keyboard_program_memory(6);
          break;

        case PS2_F8_SHIFT  :
          ps2_keyboard_program_memory(7);
          break;

        case PS2_F9_SHIFT  :
          ps2_keyboard_program_memory(8);
          break;

        case PS2_F10_SHIFT  :
          ps2_keyboard_program_memory(9);
          break;

        case PS2_F11_SHIFT  :
          ps2_keyboard_program_memory(10);
          break;

        case PS2_F12_SHIFT  :
          ps2_keyboard_program_memory(11);
          break;
        #endif //OPTION_SAVE_MEMORY_NANOKEYER
        #endif //FEATURE_MEMORIES

        #ifndef OPTION_SAVE_MEMORY_NANOKEYER
        case PS2_INSERT :   // send serial number and increment
          put_serial_number_in_send_buffer();
          serial_number++;
          break;

        case PS2_END :      // send serial number no increment
          put_serial_number_in_send_buffer();
          break;

        case PS2_BACKSPACE_SHIFT :    // decrement serial number
          serial_number--;
	  LCD_CENTER_PRINT_TIMED("Serial: " + String(serial_number), 0, default_display_msg_delay);
          break;
        
        #endif //OPTION_SAVE_MEMORY_NANOKEYER

        case PS2_LEFT_ALT :
          #ifdef DEBUG_PS2_KEYBOARD
          debug_serial_port->println("PS2_LEFT_ALT\n");
          #endif
          break;


        case PS2_A_CTRL :
          configuration.keyer_mode = IAMBIC_A;
          LCD_CENTER_PRINT_TIMED("Iambic A", 0, default_display_msg_delay);

          config_dirty = 1;
          break;

        case PS2_B_CTRL :
          configuration.keyer_mode = IAMBIC_B;
          LCD_CENTER_PRINT_TIMED("Iambic B", 0, default_display_msg_delay);
          config_dirty = 1;
          break;

        case PS2_D_CTRL :
          configuration.keyer_mode = ULTIMATIC;
          LCD_CENTER_PRINT_TIMED("Ultimatic", 0, default_display_msg_delay);
          config_dirty = 1;
          break;

        #ifndef OPTION_SAVE_MEMORY_NANOKEYER
        case PS2_E_CTRL :
	  NOTIFY_USER_BOOP_BEEP("Enter Serial #", 0, default_display_msg_delay);
          work_int = ps2_keyboard_get_number_input(4,0,10000);
          if (work_int > 0) {
            serial_number = work_int;
	    LCD_STATUS_REVERT_BEEP;
          }
          break;
        #endif //OPTION_SAVE_MEMORY_NANOKEYER

        case PS2_G_CTRL :
          configuration.keyer_mode = BUG;
          LCD_CENTER_PRINT_TIMED("Bug", 0, default_display_msg_delay);
          config_dirty = 1;
          break;

        #ifdef FEATURE_HELL
        case PS2_H_CTRL :       
          if (char_send_mode == CW) {
            char_send_mode = HELL;
            beep();
          } else {
            char_send_mode = CW;
            beep();
          }
          break;
        #endif //FEATURE_HELL

        case PS2_I_CTRL :
          if (key_tx) {
            key_tx = 0;
            LCD_CENTER_PRINT_TIMED("TX Off", 0, default_display_msg_delay);
            
          } else {
            key_tx = 1;
            LCD_CENTER_PRINT_TIMED("TX On", 0, default_display_msg_delay);
          }
          break;

        #ifdef FEATURE_FARNSWORTH
        case PS2_M_CTRL:         
          NOTIFY_USER_BOOP_BEEP("Farnsworth WPM", 0, default_display_msg_delay);
          work_int = ps2_keyboard_get_number_input(3,-1,1000);
          if (work_int > -1) {
            configuration.wpm_farnsworth = work_int;
	    LCD_STATUS_REVERT_BEEP;
            config_dirty = 1;
          }
          
          break;
          #endif //FEATURE_FARNSWORTH

        case PS2_N_CTRL :
          if (configuration.paddle_mode == PADDLE_NORMAL) {
            configuration.paddle_mode = PADDLE_REVERSE;
            LCD_CENTER_PRINT_TIMED("Paddle Reverse", 0, default_display_msg_delay);
          } else {
            configuration.paddle_mode = PADDLE_NORMAL;
            LCD_CENTER_PRINT_TIMED("Paddle Normal", 0, default_display_msg_delay);
          }
          config_dirty = 1;
          break;

        case PS2_O_CTRL :
          if ((configuration.sidetone_mode == SIDETONE_ON) || (configuration.sidetone_mode == SIDETONE_PADDLE_ONLY)){
            configuration.sidetone_mode = SIDETONE_OFF;
            LCD_CENTER_PRINT_TIMED("Sidetone Off", 0, default_display_msg_delay);
          } else {
            LCD_CENTER_PRINT_TIMED("Sidetone On", 0, default_display_msg_delay);
            configuration.sidetone_mode = SIDETONE_ON;
          }
          config_dirty = 1;
         break;

        case PS2_T_CTRL :
          #ifdef FEATURE_MEMORIES
          repeat_memory = 255;
          #endif
          if (keyboard_tune_on) {
            tx_and_sidetone_key(0,MANUAL_SENDING);
            keyboard_tune_on = 0;
	    LCD_STATUS_REVERT;
          } else {
            LCD_CENTER_PRINT_TIMED("Tune", 0, default_display_msg_delay);
            tx_and_sidetone_key(1,MANUAL_SENDING);
            keyboard_tune_on = 1;
          }
          break;

        case PS2_U_CTRL :
          if (ptt_line_activated) {
            manual_ptt_invoke = 0;
            ptt_unkey();
	    LCD_STATUS_REVERT;
          } else {
            LCD_CENTER_PRINT_TIMED("PTT Invoke", 0, default_display_msg_delay);
            manual_ptt_invoke = 1;
            ptt_key();
          }
          break;

        case PS2_W_CTRL :
          NOTIFY_USER_BOOP_BEEP("WPM Adjust", 0, default_display_msg_delay);
          work_int = ps2_keyboard_get_number_input(3,0,1000);
          if (work_int > 0) {
            speed_set(work_int);
	    LCD_STATUS_REVERT_BEEP;
            config_dirty = 1;
          }
          break;

        case PS2_F1_CTRL :
          switch_to_tx_silent(1);
          LCD_CENTER_PRINT_TIMED("TX 1", 0, default_display_msg_delay);
          break;

        case PS2_F2_CTRL :
          if ((ptt_tx_2) || (tx_key_line_2)) {
            switch_to_tx_silent(2);          
            LCD_CENTER_PRINT_TIMED("TX 2", 0, default_display_msg_delay);
          }
          break;
        #ifndef OPTION_SAVE_MEMORY_NANOKEYER
        case PS2_F3_CTRL :
          if ((ptt_tx_3)  || (tx_key_line_3)) {
            switch_to_tx_silent(3);                       
            LCD_CENTER_PRINT_TIMED("TX 3", 0, default_display_msg_delay);
          }
          break;

        case PS2_F4_CTRL :
          if ((ptt_tx_4)  || (tx_key_line_4)) {
            switch_to_tx_silent(4);   
            LCD_CENTER_PRINT_TIMED("TX 4", 0, default_display_msg_delay);
          }
          break;

        case PS2_F5_CTRL :
          if ((ptt_tx_5)  || (tx_key_line_5)) {
            switch_to_tx_silent(5);  
            LCD_CENTER_PRINT_TIMED("TX 5", 0, default_display_msg_delay);
          }
          break;

        case PS2_F6_CTRL :
          if ((ptt_tx_6)  || (tx_key_line_6)) {
            switch_to_tx_silent(6);
            LCD_CENTER_PRINT_TIMED("TX 6", 0, default_display_msg_delay);
          }
          break;
        #endif //OPTION_SAVE_MEMORY_NANOKEYER

        #ifdef FEATURE_AUTOSPACE
        case PS2_Z_CTRL:
          if (configuration.autospace_active) {
            configuration.autospace_active = 0;
            config_dirty = 1;
            LCD_CENTER_PRINT_TIMED("Autospace Off", 0, default_display_msg_delay);
          } else {
            configuration.autospace_active = 1;
            config_dirty = 1;
            LCD_CENTER_PRINT_TIMED("Autospace On", 0, default_display_msg_delay);
          }
          break;
        #endif

        default :
          if ((keystroke > 31) && (keystroke < 255 /*123*/)) {
            if (ps2_prosign_flag) {
              add_to_send_buffer(SERIAL_SEND_BUFFER_PROSIGN);
              ps2_prosign_flag = 0;
            }
            keystroke = uppercase(keystroke);
            add_to_send_buffer(keystroke);
            #ifdef FEATURE_MEMORIES
            repeat_memory = 255;
            #endif
          }
          break;
      }
    } else {

    }
  } //while ((keyboard.available()) && (play_memory_prempt == 0))
    
    
    
  #else //FEATURE_MEMORIES --------------------------------------------------------------------



  while (keyboard.available()) {
 
    // read the next key
    keystroke = keyboard.read();
    
    #ifdef FEATURE_SLEEP
    last_activity_time = millis(); 
    #endif //FEATURE_SLEEP

    if (ps2_keyboard_mode == PS2_KEYBOARD_NORMAL) {
      switch (keystroke) {
        case PS2_PAGEUP : sidetone_adj(20); break;
        case PS2_PAGEDOWN : sidetone_adj(-20); break;
        case PS2_RIGHTARROW : adjust_dah_to_dit_ratio(int(configuration.dah_to_dit_ratio/10)); break;
        case PS2_LEFTARROW : adjust_dah_to_dit_ratio(-1*int(configuration.dah_to_dit_ratio/10)); break;
        case PS2_UPARROW : speed_set(configuration.wpm+1); break;
        case PS2_DOWNARROW : speed_set(configuration.wpm-1); break;
        case PS2_HOME :
          configuration.dah_to_dit_ratio = initial_dah_to_dit_ratio;
          key_tx = 1;
          config_dirty = 1;
          #ifdef OPTION_MORE_DISPLAY_MSGS
          LCD_CENTER_PRINT_TIMED("Default ratio", 0, default_display_msg_delay);
          SERVICE_DISPLAY();
          #endif           
          break;
        case PS2_TAB :
          if (pause_sending_buffer) {
            pause_sending_buffer = 0;
            #ifdef OPTION_MORE_DISPLAY_MSGS
            LCD_CENTER_PRINT_TIMED("Resume", 0, default_display_msg_delay);
            #endif                 
          } else {
            pause_sending_buffer = 1;
            LCD_CENTER_PRINT_TIMED("Pause", 0, default_display_msg_delay);
          }
        break;  // pause

        case PS2_SCROLL :   // Prosign next two characters
          ps2_prosign_flag = 1;
          #ifdef OPTION_MORE_DISPLAY_MSGS
          LCD_CENTER_PRINT_TIMED("Prosign", 0, default_display_msg_delay);
          #endif          
          break;

        #ifdef FEATURE_MEMORIES
        case PS2_F1 : ps2_usb_keyboard_play_memory(0); break;
        case PS2_F2 : ps2_usb_keyboard_play_memory(1); break;
        case PS2_F3 : ps2_usb_keyboard_play_memory(2); break;
        case PS2_F4 : ps2_usb_keyboard_play_memory(3); break;
        case PS2_F5 : ps2_usb_keyboard_play_memory(4); break;
        case PS2_F6 : ps2_usb_keyboard_play_memory(5); break;
        case PS2_F7 : ps2_usb_keyboard_play_memory(6); break;
        case PS2_F8 : ps2_usb_keyboard_play_memory(7); break;
        case PS2_F9 : ps2_usb_keyboard_play_memory(8); break;
        case PS2_F10 : ps2_usb_keyboard_play_memory(9); break;
        case PS2_F11 : ps2_usb_keyboard_play_memory(10); break;
        case PS2_F12 : ps2_usb_keyboard_play_memory(11); break;
        case PS2_F1_ALT : if (number_of_memories > 0) {repeat_memory_msg(0);} break;
        case PS2_F2_ALT : if (number_of_memories > 1) {repeat_memory_msg(1);} break;
        case PS2_F3_ALT : if (number_of_memories > 2) {repeat_memory_msg(2);} break;
        case PS2_F4_ALT : if (number_of_memories > 3) {repeat_memory_msg(3);} break;
        case PS2_F5_ALT : if (number_of_memories > 4) {repeat_memory_msg(4);} break;
        case PS2_F6_ALT : if (number_of_memories > 5) {repeat_memory_msg(5);} break;
        case PS2_F7_ALT : if (number_of_memories > 6) {repeat_memory_msg(6);} break;
        case PS2_F8_ALT : if (number_of_memories > 7) {repeat_memory_msg(7);} break;
        case PS2_F9_ALT : if (number_of_memories > 8) {repeat_memory_msg(8);} break;
        case PS2_F10_ALT : if (number_of_memories > 9) {repeat_memory_msg(9);} break;
        case PS2_F11_ALT : if (number_of_memories > 10) {repeat_memory_msg(10);} break;
        case PS2_F12_ALT : if (number_of_memories > 11) {repeat_memory_msg(11);} break;
        #endif

        case PS2_DELETE : if (send_buffer_bytes > 0) { send_buffer_bytes--; } break;
        case PS2_ESC :  // clear the serial send buffer and a bunch of other stuff
          if (manual_ptt_invoke) {
            manual_ptt_invoke = 0;
            ptt_unkey();
          }
          if (keyboard_tune_on) {
            tx_and_sidetone_key(0,MANUAL_SENDING);
            keyboard_tune_on = 0;
          }
          if (pause_sending_buffer) {
            pause_sending_buffer = 0;
          }
          clear_send_buffer();
          #ifdef FEATURE_MEMORIES
          //clear_memory_button_buffer();
          play_memory_prempt = 1;
          repeat_memory = 255;
          #endif
          LCD_CENTER_PRINT_TIMED("Abort", 0, default_display_msg_delay);
          break;

        #ifdef FEATURE_MEMORIES
        case PS2_F1_SHIFT  :
          ps2_keyboard_program_memory(0);
          break;

        case PS2_F2_SHIFT  :
          ps2_keyboard_program_memory(1);
          break;

        case PS2_F3_SHIFT  :
          ps2_keyboard_program_memory(2);
          break;

        case PS2_F4_SHIFT  :
          ps2_keyboard_program_memory(3);
          break;

        case PS2_F5_SHIFT  :
          ps2_keyboard_program_memory(4);
          break;

        case PS2_F6_SHIFT  :
          ps2_keyboard_program_memory(5);
          break;

        case PS2_F7_SHIFT  :
          ps2_keyboard_program_memory(6);
          break;

        case PS2_F8_SHIFT  :
          ps2_keyboard_program_memory(7);
          break;

        case PS2_F9_SHIFT  :
          ps2_keyboard_program_memory(8);
          break;

        case PS2_F10_SHIFT  :
          ps2_keyboard_program_memory(9);
          break;

        case PS2_F11_SHIFT  :
          ps2_keyboard_program_memory(10);
          break;

        case PS2_F12_SHIFT  :
          ps2_keyboard_program_memory(11);
          break;
        #endif //FEATURE_MEMORIES

        case PS2_INSERT :   // send serial number and increment
          put_serial_number_in_send_buffer();
          serial_number++;
          break;

        case PS2_END :      // send serial number no increment
          put_serial_number_in_send_buffer();
          break;

        case PS2_BACKSPACE_SHIFT :    // decrement serial number
          serial_number--;
          LCD_CENTER_PRINT_TIMED("Serial: " + String(serial_number), 0, default_display_msg_delay);
          break;

        case PS2_LEFT_ALT :
          #ifdef DEBUG_PS2_KEYBOARD
          debug_serial_port->println("PS2_LEFT_ALT\n");
          #endif
          break;

        case PS2_A_CTRL :
          configuration.keyer_mode = IAMBIC_A;
          LCD_CENTER_PRINT_TIMED("Iambic A", 0, default_display_msg_delay);

          config_dirty = 1;
          break;

        case PS2_B_CTRL :
          configuration.keyer_mode = IAMBIC_B;
          LCD_CENTER_PRINT_TIMED("Iambic B", 0, default_display_msg_delay);
          config_dirty = 1;
          break;

        case PS2_D_CTRL :
          configuration.keyer_mode = ULTIMATIC;
          LCD_CENTER_PRINT_TIMED("Ultimatic", 0, default_display_msg_delay);
          config_dirty = 1;
          break;

        case PS2_E_CTRL :
          NOTIFY_USER_BOOP_BEEP("Enter Serial #", 0, default_display_msg_delay);
          work_int = ps2_keyboard_get_number_input(4,0,10000);
          if (work_int > 0) {
            serial_number = work_int;
	    LCD_STATUS_REVERT_BEEP;
          }
          break;

        case PS2_G_CTRL :
          configuration.keyer_mode = BUG;
          LCD_CENTER_PRINT_TIMED("Bug", 0, default_display_msg_delay);
          config_dirty = 1;
          break;

        case PS2_H_CTRL :
          #ifdef FEATURE_HELL
          if (char_send_mode == CW) {
            char_send_mode = HELL;
            beep();
          } else {
            char_send_mode = CW;
            beep();
          }
          #endif //FEATURE_HELL
          break;

        case PS2_I_CTRL :
          if (key_tx) {
            key_tx = 0;
            LCD_CENTER_PRINT_TIMED("TX Off", 0, default_display_msg_delay);
          } else {
            key_tx = 1;
            LCD_CENTER_PRINT_TIMED("TX On", 0, default_display_msg_delay);
          }
          break;

        case PS2_M_CTRL:
          #ifdef FEATURE_FARNSWORTH
          NOTIFY_USER_BOOP_BEEP("Farnsworth WPM", 0, default_display_msg_delay);
          work_int = ps2_keyboard_get_number_input(3,-1,1000);
          if (work_int > -1) {
            configuration.wpm_farnsworth = work_int;
	    LCD_STATUS_REVERT_BEEP;
            config_dirty = 1;
          }
          #endif
          break;

        case PS2_N_CTRL :
          if (configuration.paddle_mode == PADDLE_NORMAL) {
            configuration.paddle_mode = PADDLE_REVERSE;
            LCD_CENTER_PRINT_TIMED("Paddle Reverse", 0, default_display_msg_delay);
          } else {
            configuration.paddle_mode = PADDLE_NORMAL;
            LCD_CENTER_PRINT_TIMED("Paddle Normal", 0, default_display_msg_delay);
          }
          config_dirty = 1;
          break;

        case PS2_O_CTRL :
          if ((configuration.sidetone_mode == SIDETONE_ON) || (configuration.sidetone_mode == SIDETONE_PADDLE_ONLY)){
            configuration.sidetone_mode = SIDETONE_OFF;
            LCD_CENTER_PRINT_TIMED("Sidetone Off", 0, default_display_msg_delay);
          } else {
            LCD_CENTER_PRINT_TIMED("Sidetone On", 0, default_display_msg_delay);
            configuration.sidetone_mode = SIDETONE_ON;
          }
          config_dirty = 1;
         break;

        case PS2_T_CTRL :
          #ifdef FEATURE_MEMORIES
          repeat_memory = 255;
          #endif
          if (keyboard_tune_on) {
            tx_and_sidetone_key(0,MANUAL_SENDING);
            keyboard_tune_on = 0;
	    LCD_STATUS_REVERT;
          } else {
            LCD_CENTER_PRINT_TIMED("Tune", 0, default_display_msg_delay);
            tx_and_sidetone_key(1,MANUAL_SENDING);
            keyboard_tune_on = 1;
          }
          break;

        case PS2_U_CTRL :
          if (ptt_line_activated) {
            manual_ptt_invoke = 0;
            ptt_unkey();
	    LCD_STATUS_REVERT;
          } else {
            LCD_CENTER_PRINT_TIMED("PTT Invoke", 0, default_display_msg_delay);
            manual_ptt_invoke = 1;
            ptt_key();
          }
          break;

        case PS2_W_CTRL :
          NOTIFY_USER_BOOP_BEEP("WPM Adjust", 0, default_display_msg_delay);
          work_int = ps2_keyboard_get_number_input(3,0,1000);
          if (work_int > 0) {
            speed_set(work_int);
	    LCD_STATUS_REVERT_BEEP;
            config_dirty = 1;
          }
          break;

        case PS2_F1_CTRL :
          //current_ptt_line = ptt_tx_1;
          //current_tx_key_line = tx_key_line_1;
          switch_to_tx_silent(1);
          LCD_CENTER_PRINT_TIMED("TX 1", 0, default_display_msg_delay);
          break;

        case PS2_F2_CTRL :
          if ((ptt_tx_2) || (tx_key_line_2)) {
            switch_to_tx_silent(2);
            //current_ptt_line = ptt_tx_2;
            //current_tx_key_line = tx_key_line_2;           
            LCD_CENTER_PRINT_TIMED("TX 2", 0, default_display_msg_delay);
          }
          break;

        case PS2_F3_CTRL :
          if ((ptt_tx_3)  || (tx_key_line_3)) {
            switch_to_tx_silent(3);
            //current_ptt_line = ptt_tx_3;
            //current_tx_key_line = tx_key_line_3;                       
            LCD_CENTER_PRINT_TIMED("TX 3", 0, default_display_msg_delay);
          }
          break;

        case PS2_F4_CTRL :
          if ((ptt_tx_4)  || (tx_key_line_4)) {
            switch_to_tx_silent(4);
            //current_ptt_line = ptt_tx_4;
            //current_tx_key_line = tx_key_line_4;     
            LCD_CENTER_PRINT_TIMED("TX 4", 0, default_display_msg_delay);
          }
          break;

        case PS2_F5_CTRL :
          if ((ptt_tx_5)  || (tx_key_line_5)) {
            switch_to_tx_silent(5);
            //current_ptt_line = ptt_tx_5;
            //current_tx_key_line = tx_key_line_5;  
            LCD_CENTER_PRINT_TIMED("TX 5", 0, default_display_msg_delay);
          }
          break;

        case PS2_F6_CTRL :
          if ((ptt_tx_6)  || (tx_key_line_6)) {
            switch_to_tx_silent(6);
            //current_ptt_line = ptt_tx_6;
            //current_tx_key_line = tx_key_line_6; 
            LCD_CENTER_PRINT_TIMED("TX 6", 0, default_display_msg_delay);
          }
          break;

        #ifdef FEATURE_AUTOSPACE
        case PS2_Z_CTRL:
          if (configuration.autospace_active) {
            configuration.autospace_active = 0;
            config_dirty = 1;
            LCD_CENTER_PRINT_TIMED("Autospace Off", 0, default_display_msg_delay);
          } else {
            configuration.autospace_active = 1;
            config_dirty = 1;
            LCD_CENTER_PRINT_TIMED("Autospace On", 0, default_display_msg_delay);
          }
          break;
        #endif

        default :
          if ((keystroke > 31) && (keystroke < 255 /*123*/)) {
            if (ps2_prosign_flag) {
              add_to_send_buffer(SERIAL_SEND_BUFFER_PROSIGN);
              ps2_prosign_flag = 0;
            }
            keystroke = uppercase(keystroke);
            add_to_send_buffer(keystroke);
            #ifdef FEATURE_MEMORIES
            repeat_memory = 255;
            #endif
          }
          break;
      }
    } else {

    }
  } //while (keyboard.available())
  #endif //FEATURE_MEMORIES
}
#endif //FEATURE_PS2_KEYBOARD

//-------------------------------------------------------------------------------------------------------
#if (defined(FEATURE_PS2_KEYBOARD) || defined(FEATURE_USB_KEYBOARD)) && defined(FEATURE_MEMORIES)
void ps2_usb_keyboard_play_memory(byte memory_number){

  if (memory_number < number_of_memories) {
    add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
    add_to_send_buffer(memory_number);
    #ifdef FEATURE_MEMORIES
    repeat_memory = 255;
    #endif //FEATURE_MEMORIES
  }
}
#endif  //defined(FEATURE_PS2_KEYBOARD) || defined(FEATURE_USB_KEYBOARD)
//-------------------------------------------------------------------------------------------------------
#if defined(FEATURE_PS2_KEYBOARD) && defined(FEATURE_MEMORIES)
void ps2_keyboard_program_memory(byte memory_number)
{

  char keystroke;
  byte looping = 1;
  byte error = 0;
  int temp_memory_index = 0;
  byte temp_memory[(memory_end(memory_number)-memory_start(memory_number) + 1)];
  int x;
  String keyboard_string;
  String lcd_string = "Program Memory";

  if (memory_number > (number_of_memories - 1)) {
    boop();
    return;
  }
  
  #ifdef FEATURE_DISPLAY
  if (memory_number < 9) {
    lcd_string.concat(' ');
  }
  lcd_string.concat(memory_number+1);
  LCD_CENTER_PRINT_TIMED(lcd_string, 0, default_display_msg_delay);
  #else
  boop_beep();
  #endif
  repeat_memory = 255;
  while (looping) {
    while (keyboard.available() == 0) {
      if (keyer_machine_mode == KEYER_NORMAL) {          // might as well do something while we're waiting
        check_paddles();
        service_dit_dah_buffers();
      }
    }
    keystroke = keyboard.read();
    #ifdef DEBUG_PS2_KEYBOARD
    debug_serial_port->println(keystroke,DEC);
    #endif
    if (keystroke == 13) {        // did we get a carriage return?
      looping = 0;
    } else {
      if (keystroke == PS2_BACKSPACE) {
        if (temp_memory_index) {
          temp_memory_index--;
          #ifdef FEATURE_DISPLAY
          keyboard_string = keyboard_string.substring(0,keyboard_string.length()-1);
          LCD_CENTER_PRINT_TIMED(keyboard_string, 1, default_display_msg_delay);
          #endif
        }
      } else {
        if (keystroke == PS2_ESC) {
          looping = 0;
          error = 1;
        } else {
          keystroke = uppercase(keystroke);
          #ifdef FEATURE_DISPLAY
          keyboard_string.concat(char(keystroke));
          if (keyboard_string.length() > LCD_COLUMNS) {
            LCD_CENTER_PRINT_TIMED(keyboard_string.substring((keyboard_string.length()-LCD_COLUMNS)), 1, default_display_msg_delay);
          } else {         
            LCD_CENTER_PRINT_TIMED(keyboard_string, 1, default_display_msg_delay);
          }
          #endif
          temp_memory[temp_memory_index] = keystroke;
          temp_memory_index++;
          if (temp_memory_index > (memory_end(memory_number)-memory_start(memory_number))) {
            looping = 0;
          }
        }
      }
    }
  }  //while (looping)

  if (error) {
      LCD_STATUS_REVERT_BOOP;
  } else {
    for (x = 0;x < temp_memory_index;x++) {  // write to memory
      EEPROM.write((memory_start(memory_number)+x),temp_memory[x]);
      if ((memory_start(memory_number) + x) == memory_end(memory_number)) {    // are we at last memory location?
        x = temp_memory_index;
      }
    }
    // write terminating 255
    EEPROM.write((memory_start(memory_number)+x),255);
    NOTIFY_USER_BEEP("Done", 0, default_display_msg_delay);
  }
}
#endif

//-------------------------------------------------------------------------------------------------------

#ifdef FEATURE_PS2_KEYBOARD

int ps2_keyboard_get_number_input(byte places,int lower_limit, int upper_limit)
{

  byte looping = 1;
  byte error = 0;
  byte numberindex = 0;
  int numbers[6];
  char keystroke;
  String keyboard_string;

  #ifdef FEATURE_MEMORIES
  repeat_memory = 255;
  #endif

  while (looping) {
    if (keyboard.available() == 0) {        // wait for the next keystroke
      if (keyer_machine_mode == KEYER_NORMAL) {          // might as well do something while we're waiting
        check_paddles();
        service_dit_dah_buffers();
        SERVICE_SEND_BUFFER(PRINTCHAR);

        check_ptt_tail();
	CHECK_POTENTIOMETER();
        CHECK_ROTARY_ENCODER();
      }
    } else {
      keystroke = keyboard.read();
      if ((keystroke > 47) && (keystroke < 58)) {    // ascii 48-57 = "0" - "9")
        numbers[numberindex] = keystroke;
        numberindex++;
        #ifdef FEATURE_DISPLAY
        keyboard_string.concat(String(keystroke-48));
        LCD_CENTER_PRINT_TIMED(keyboard_string, 1, default_display_msg_delay);
        #endif                     
        if (numberindex > places){
            looping = 0;
            error = 1;
        }
      } else {
        if (keystroke == PS2_BACKSPACE) {
          if (numberindex) {
            numberindex--;
            #ifdef FEATURE_DISPLAY
            keyboard_string = keyboard_string.substring(0,keyboard_string.length()-1);
            LCD_CENTER_PRINT_TIMED(keyboard_string, 1, default_display_msg_delay);
            #endif             
          }
        } else {
          if (keystroke == PS2_ENTER) {   // carriage return - get out
            looping = 0;
          } else {                 // bogus input - error out
            looping = 0;
            error = 1;
          }
        }
      }
    }
  }
  if (error) {
    boop();
    return(-1);
  } else {
    int y = 1;
    int return_number = 0;
    for (int x = (numberindex - 1); x >= 0 ; x = x - 1) {
      return_number = return_number + ((numbers[x]-48) * y);
      y = y * 10;
    }
    if ((return_number > lower_limit) && (return_number < upper_limit)) {
      return(return_number);
    } else {
      boop();
      return(-1);
    }
  }
}
#endif

//--------------------------------------------------------------------- 

#if defined(FEATURE_PS2_KEYBOARD) && defined(OPTION_PS2_KEYBOARD_RESET)
void ps2int_write() {

  // code contributed by Bill, W9BEL
  //----- Called from initialize_ps2_keyboard to reset Mini KBD ---------
  // The ISR for the external interrupt in read mode


  uint8_t buffer[45];
  uint8_t head, tail, writeByte = 255;
  uint8_t curbit = 0, parity = 0, ack =0;
 
  if(curbit < 8) {
    if(writeByte & 1) {
      parity ^= 1;
      digitalWrite(ps2_keyboard_data, HIGH);
    } else
      digitalWrite(ps2_keyboard_data, LOW);
    writeByte >>= 1;
  } else if(curbit == 8) { // parity
    if(parity)
      digitalWrite(ps2_keyboard_data, LOW);
    else
      digitalWrite(ps2_keyboard_data, HIGH);
  } else if(curbit == 9) { // time to let go
    pinMode(ps2_keyboard_data, INPUT); // release line
    digitalWrite(ps2_keyboard_data, HIGH); // pullup on
  } else { // time to check device ACK and hold clock again
    //holdClock();
    digitalWrite(ps2_keyboard_clock, LOW); // pullup off
    pinMode(ps2_keyboard_clock, OUTPUT); // pull clock low
    ack = !digitalRead(ps2_keyboard_data);
  }
  curbit++;
}
#endif 

//--------------------------------------------------------------------- 
void initialize_ps2_keyboard(){

  #ifdef OPTION_PS2_KEYBOARD_RESET             // code contributed by Bill, W9BEL
  attachInterrupt(1, ps2int_write, FALLING);
  digitalWrite(ps2_keyboard_data, LOW); // pullup off
  pinMode(ps2_keyboard_data, OUTPUT); // pull clock low
  delay(200);
  #endif //OPTION_PS2_KEYBOARD_RESET


  keyboard.begin(ps2_keyboard_data, ps2_keyboard_clock);
}

#endif //FEATURE_PS2_KEYBOARD
