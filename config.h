#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

// Variables and stuff
struct config_t {
  unsigned int wpm;
  byte paddle_mode;
  byte keyer_mode;
  byte sidetone_mode;
  unsigned int hz_sidetone;
  unsigned int dah_to_dit_ratio;
  byte pot_activated;
  byte length_wordspace;
  byte autospace_active;
  unsigned int wpm_farnsworth;
  byte current_ptt_line;
  byte current_tx;
  byte weighting;
  unsigned int memory_repeat_time;
  byte dit_buffer_off;
  byte dah_buffer_off;
  byte cmos_super_keyer_iambic_b_timing_percent;
  byte cmos_super_keyer_iambic_b_timing_on;
  uint8_t ip[4];
  uint8_t gateway[4];  
  uint8_t subnet[4]; 
  uint8_t link_send_ip[4][FEATURE_INTERNET_LINK_MAX_LINKS];
  uint8_t link_send_enabled[FEATURE_INTERNET_LINK_MAX_LINKS];
  int link_send_udp_port[FEATURE_INTERNET_LINK_MAX_LINKS];
  int link_receive_udp_port;
  uint8_t link_receive_enabled;
};

extern struct config_t configuration;

#endif // CONFIG_H_INCLUDED
