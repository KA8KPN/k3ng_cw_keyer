#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef FEATURE_DISPLAY

#include "keyer_settings.h"

enum lcd_statuses {LCD_CLEAR, LCD_REVERT, LCD_TIMED_MESSAGE, LCD_SCROLL_MSG};
#define default_display_msg_delay 1000

void service_send_buffer(byte no_print);
void service_display(void);
void display_scroll_print_char(char charin);
void lcd_center_print_timed(String lcd_print_string, byte row_number, unsigned int duration);
void lcd_center_print_timed_wpm(void);
void lcd_clear(void);
void initialize_display(void);

#define SERVICE_SEND_BUFFER(x) service_send_buffer(x)
#define SERVICE_DISPLAY()      service_display()
#define DISPLAY_SCROLL_PRINT_CHAR(x)    display_scroll_print_char(x)
#define LCD_CENTER_PRINT_TIMED(a, b, c) lcd_center_print_timed(a, b, c)
#define NOTIFY_USER_BOOP_BEEP(a, b, c) lcd_center_print_timed(a, b, c)
#define NOTIFY_USER_BOOP(a, b, c) lcd_center_print_timed(a, b, c)
#define NOTIFY_USER_BEEP(a, b, c) lcd_center_print_timed(a, b, c)
#define LCD_CENTER_PRINT_TIMED_WPM() lcd_center_print_timed_wpm()
#define INITIALIZE_DISPLAY() initialize_display()
#define LCD_STATUS_REVERT_BEEP lcd_status = LCD_REVERT
#define LCD_STATUS_REVERT lcd_status = LCD_REVERT
#define LCD_STATUS_REVERT_BOOP lcd_status = LCD_REVERT
#define LCD_CLEAR_DISPLAY() lcd_clear()

extern byte lcd_status;
extern unsigned long lcd_timed_message_clear_time;
extern byte lcd_previous_status ;
extern byte lcd_scroll_buffer_dirty;
extern String lcd_scroll_buffer[LCD_ROWS];
extern byte lcd_scroll_flag;
extern byte lcd_paddle_echo;
extern byte lcd_send_echo;

#else // !FEATURE_DISPLAY

#define SERVICE_SEND_BUFFER(x)
#define SERVICE_DISPLAY()
#define DISPLAY_SCROLL_PRINT_CHAR(x)
#define LCD_CENTER_PRINT_TIMED(a, b, c)
#define LCD_CENTER_PRINT_TIMED_WPM()
#define INITIALIZE_DISPLAY()
#define NOTIFY_USER_BOOP_BEEP(a, b, c) boop_beep()
#define NOTIFY_USER_BOOP(a, b, c) boop()
#define NOTIFY_USER_BEEP(a, b, c) beep()
#define LCD_STATUS_REVERT_BEEP    beep()
#define LCD_STATUS_REVERT
#define LCD_STATUS_REVERT_BOOP boop()
#define LCD_CLEAR_DISPLAY()

#endif // !FEATURE_DISPLAY

#endif // DISPLAY_H
