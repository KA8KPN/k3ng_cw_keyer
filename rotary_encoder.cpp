#include "keyer_features_and_options.h"

#include <stdio.h>
#include <Arduino.h>

#include "rotary_encoder.h"
#include "keyer_pin_settings.h"

void speed_change(int change);  // TODO -- put this in a header file

#ifdef FEATURE_ROTARY_ENCODER
  #ifdef OPTION_ENCODER_HALF_STEP_MODE      // Use the half-step state table (emits a code at 00 and 11)
  const unsigned char ttable[6][4] = {
    {0x3 , 0x2, 0x1,  0x0}, {0x23, 0x0, 0x1, 0x0},
    {0x13, 0x2, 0x0,  0x0}, {0x3 , 0x5, 0x4, 0x0},
    {0x3 , 0x3, 0x4, 0x10}, {0x3 , 0x5, 0x3, 0x20}
  };
  #else                                      // Use the full-step state table (emits a code at 00 only)
  const unsigned char ttable[7][4] = {
    {0x0, 0x2, 0x4,  0x0}, {0x3, 0x0, 0x1, 0x10},
    {0x3, 0x2, 0x0,  0x0}, {0x3, 0x2, 0x1,  0x0},
    {0x6, 0x0, 0x4,  0x0}, {0x6, 0x5, 0x0, 0x10},
    {0x6, 0x5, 0x4,  0x0},
  };
  #endif //OPTION_ENCODER_HALF_STEP_MODE 
  unsigned char state = 0;
  #define DIR_CCW 0x10                      // CW Encoder Code (do not change)
  #define DIR_CW 0x20                       // CCW Encoder Code (do not change)


void initialize_rotary_encoder(){  
  
    pinMode(rotary_pin1, INPUT);
    pinMode(rotary_pin2, INPUT);
    #ifdef OPTION_ENCODER_ENABLE_PULLUPS
      digitalWrite(rotary_pin1, HIGH);
      digitalWrite(rotary_pin2, HIGH);
    #endif //OPTION_ENCODER_ENABLE_PULLUPS
}


void check_rotary_encoder(){

  static unsigned long timestamp[5];

  unsigned char pinstate = (digitalRead(rotary_pin2) << 1) | digitalRead(rotary_pin1);
  state = ttable[state & 0xf][pinstate];
  unsigned char result = (state & 0x30);
      
  if (result) {                                    // If rotary encoder modified  
    timestamp[0] = timestamp[1];                    // Encoder step timer
    timestamp[1] = timestamp[2]; 
    timestamp[2] = timestamp[3]; 
    timestamp[3] = timestamp[4]; 
    timestamp[4] = millis();
    
    unsigned long elapsed_time = (timestamp[4] - timestamp[0]); // Encoder step time difference for 10's step
 
    if (result == DIR_CW) {                      
      if (elapsed_time < 250) {speed_change(2);} else {speed_change(1);};
    }
    if (result == DIR_CCW) {                      
      if (elapsed_time < 250) {speed_change(-2);} else {speed_change(-1);};
    }
    
  } // if (result)

  
  
}
#endif //FEATURE_ROTARY_ENCODER
