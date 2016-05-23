// TODO:  Put this in a file that everybody can include
#ifndef HARDWARE_CUSTOM
  #include "keyer_features_and_options.h"
#endif

#ifdef FEATURE_POTENTIOMETER

#include <stdio.h>
#include <Arduino.h>

#include "potentiometer.h"
#include "keyer_pin_settings.h"
#include "keyer_settings.h"

#include "config.h"

void speed_set(int wpm_set);  // TODO -- put this in a header file

int pot_full_scale_reading = default_pot_full_scale_reading;
byte pot_wpm_low_value;

//--------------------------------------------------------------------- 
void initialize_potentiometer(){

    pinMode(potentiometer,INPUT);
    pot_wpm_high_value = initial_pot_wpm_high_value;
    last_pot_wpm_read = pot_value_wpm();
    configuration.pot_activated = 1;
}
  

//-------------------------------------------------------------------------------------------------------

void check_potentiometer()
{
  #ifdef DEBUG_LOOP
  debug_serial_port->println(F("loop: entering check_potentiometer")); 
  #endif
    
  if (configuration.pot_activated || potentiometer_always_on) {
    byte pot_value_wpm_read = pot_value_wpm();
    if ((abs(pot_value_wpm_read - last_pot_wpm_read) > potentiometer_change_threshold)) {
      #ifdef DEBUG_POTENTIOMETER
        debug_serial_port->print(F("check_potentiometer: speed change: "));
        debug_serial_port->print(pot_value_wpm_read);
        debug_serial_port->print(F(" analog read: "));
        debug_serial_port->println(analogRead(potentiometer));
      #endif
      speed_set(pot_value_wpm_read);
      last_pot_wpm_read = pot_value_wpm_read;
      #ifdef FEATURE_WINKEY_EMULATION
        if ((primary_serial_port_mode == SERIAL_WINKEY_EMULATION) && (winkey_host_open)) {
          winkey_port_write(((pot_value_wpm_read-pot_wpm_low_value)|128));
          winkey_last_unbuffered_speed_wpm = configuration.wpm;
        }
      #endif
      #ifdef FEATURE_SLEEP
        last_activity_time = millis(); 
      #endif //FEATURE_SLEEP
    }
  }
}

//-------------------------------------------------------------------------------------------------------
byte pot_value_wpm()
{
  int pot_read = analogRead(potentiometer);
  byte return_value = map(pot_read, 0, pot_full_scale_reading, pot_wpm_low_value, pot_wpm_high_value);
  return return_value;

}

//-------------------------------------------------------------------------------------------------------

#endif // FEATURE_POTENTIOMETER

