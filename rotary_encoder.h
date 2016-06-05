#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#ifdef FEATURE_ROTARY_ENCODER
void initialize_rotary_encoder(void);
void check_rotary_encoder(void);

#define INITIALIZE_ROTARY_ENCODER() initialize_rotary_encoder()
#define CHECK_ROTARY_ENCODER() check_rotary_encoder()

#else // !FEATURE_ROTARY_ENCODER
#define INITIALIZE_ROTARY_ENCODER()
#define CHECK_ROTARY_ENCODER()
#endif // !FEATURE_ROTARY_ENCODER


#endif // ROTARY_ENCODER_H
