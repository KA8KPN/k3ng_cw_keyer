#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#ifdef FEATURE_POTENTIOMETER
void initialize_potentiometer(void);
void check_potentiometer(void);
byte pot_value_wpm(void);

extern byte pot_wpm_high_value;
extern byte last_pot_wpm_read;
extern int pot_full_scale_reading;
extern byte pot_wpm_low_value;

#define INITIALIZE_POTENTIOMETER() initialize_potentiometer()
#define CHECK_POTENTIOMETER()      check_potentiometer()
#define POT_VALUE_WPM()            pot_value_wpm()


#else // !FEATURE_POTENTIOMETER
#define INITIALIZE_POTENTIOMETER()
#define CHECK_POTENTIOMETER()
#define POT_VALUE_WPM()
#endif // !FEATURE_POTENTIOMETER



#endif // POTENTIOMETER_H
