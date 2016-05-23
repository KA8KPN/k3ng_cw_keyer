#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#ifdef FEATURE_ROTARY_ENCODER
void initialize_rotary_encoder(void);
void check_rotary_encoder(void);
#else // !FEATURE_ROTARY_ENCODER
#define initialize_rotary_encoder()
#define check_rotary_encoder()
#endif // !FEATURE_ROTARY_ENCODER


#endif // ROTARY_ENCODER_H
