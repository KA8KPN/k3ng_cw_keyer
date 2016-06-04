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


#else // !FEATURE_POTENTIOMETER
#define initialize_potentiometer()
#define check_potentiometer()
#define pot_value_wpm()
#endif // !FEATURE_POTENTIOMETER



#endif // POTENTIOMETER_H
