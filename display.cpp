// TODO:  Put this in a file that everybody can include
#ifndef HARDWARE_CUSTOM
  #include "keyer_features_and_options.h"
#endif

#include "keyer_dependencies.h"

#ifdef FEATURE_DISPLAY

#include <stdio.h>
#include <Arduino.h>

#include "display.h"
#include "keyer.h"

#include "config.h"

byte lcd_status = LCD_CLEAR;
unsigned long lcd_timed_message_clear_time = 0;
byte lcd_previous_status = LCD_CLEAR;
byte lcd_scroll_buffer_dirty = 0;
String lcd_scroll_buffer[LCD_ROWS];
byte lcd_scroll_flag = 0;
byte lcd_paddle_echo = 1;
byte lcd_send_echo = 1;


#if defined(FEATURE_LCD_4BIT) || defined(FEATURE_LCD1602_N07DH)
  #include <LiquidCrystal.h>
#endif

#if defined(FEATURE_LCD_ADAFRUIT_I2C) || defined(FEATURE_LCD_ADAFRUIT_BACKPACK) || defined(FEATURE_LCD_YDv1) || defined(FEATURE_LCD_SAINSMART_I2C)
  #include <Wire.h>
#endif

#if defined(FEATURE_LCD_YDv1)
  #include <LiquidCrystal_I2C.h>
#endif

#if defined(FEATURE_LCD_ADAFRUIT_I2C)
  #include <Adafruit_MCP23017.h>
  #include <Adafruit_RGBLCDShield.h>
#endif

#if defined(FEATURE_LCD_ADAFRUIT_BACKPACK)
  #include <Adafruit_LiquidCrystal.h>
#endif

#if defined(FEATURE_LCD_SAINSMART_I2C)
  #include <LiquidCrystal_I2C.h>
#endif //FEATURE_SAINSMART_I2C_LCD  

#if defined(FEATURE_LCD_4BIT) || defined(FEATURE_LCD1602_N07DH)
  LiquidCrystal lcd(lcd_rs, lcd_enable, lcd_d4, lcd_d5, lcd_d6, lcd_d7);
#endif

#if defined(FEATURE_LCD_ADAFRUIT_I2C)
  Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
#endif

#if defined(FEATURE_LCD_ADAFRUIT_BACKPACK)
  Adafruit_LiquidCrystal lcd(0);
#endif

#if defined(FEATURE_LCD_SAINSMART_I2C)
  // #define I2C_ADDR      0x27
  // #define BACKLIGHT_PIN 3
  // #define En_pin        2
  // #define Rw_pin        1
  // #define Rs_pin        0
  // #define D4_pin        4
  // #define D5_pin        5
  // #define D6_pin        6
  // #define D7_pin        7
  // LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin, BACKLIGHT_PIN, POSITIVE);  
  LiquidCrystal_I2C lcd(0x27,20,4);
#endif //FEATURE_SAINSMART_I2C_LCD    

#if defined(FEATURE_LCD_YDv1)
  //LiquidCrystal_I2C lcd(0x38);
  LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // for FEATURE_LCD_YDv1; set the LCD I2C address needed for LCM1602 IC V1
#endif


//-------------------------------------------------------------------------------------------------------

void service_display() {

  #ifdef DEBUG_LOOP
  debug_serial_port->println(F("loop: entering service_display"));
  #endif    

  byte x = 0;

  if (lcd_status == LCD_REVERT) {
    lcd_status = lcd_previous_status;
    switch (lcd_status) {
      case LCD_CLEAR: lcd_clear(); break;
      case LCD_SCROLL_MSG:
         lcd.clear();
         for (x = 0;x < LCD_ROWS;x++){
           //clear_display_row(x);
           lcd.setCursor(0,x);
           lcd.print(lcd_scroll_buffer[x]);
         }         
         lcd_scroll_flag = 0; 
         lcd_scroll_buffer_dirty = 0;         
         break;
    }
  } else {
    switch (lcd_status) {
      case LCD_CLEAR : break;
      case LCD_TIMED_MESSAGE:
        if (millis() > lcd_timed_message_clear_time) {
          lcd_status = LCD_REVERT;
        }
      case LCD_SCROLL_MSG:
        if (lcd_scroll_buffer_dirty) { 
          if (lcd_scroll_flag) {
            lcd.clear();
            lcd_scroll_flag = 0;
          }         
          for (x = 0;x < LCD_ROWS;x++){
            //clear_display_row(x);
            lcd.setCursor(0,x);
            lcd.print(lcd_scroll_buffer[x]);
          }
          lcd_scroll_buffer_dirty = 0;
        }
      break;
    }
  }

}


//-------------------------------------------------------------------------------------------------------

void display_scroll_print_char(char charin){
  
  static byte column_pointer = 0;
  static byte row_pointer = 0;
  static byte holding_space = 0;
  byte x = 0;

  #ifdef DEBUG_DISPLAY_SCROLL_PRINT_CHAR
  debug_serial_port->print(F("display_scroll_print_char: "));
  debug_serial_port->write(charin);
  debug_serial_port->print(" ");
  debug_serial_port->println(charin);
  #endif //DEBUG_DISPLAY_SCROLL_PRINT_CHAR

  #ifdef OPTION_DISPLAY_NON_ENGLISH_EXTENSIONS
  switch (charin){
    case 220: charin = 0;break; // U_umlaut  (D, ...)
    case 214: charin = 1;break; // O_umlaut  (D, SM, OH, ...)
    case 196: charin = 2;break; // A_umlaut  (D, SM, OH, ...)
    case 198: charin = 3;break; // AE_capital (OZ, LA)
    case 216: charin = 4;break; // OE_capital (OZ, LA)
    case 197: charin = 6;break; // AA_capital (OZ, LA, SM)
    case 209: charin = 7;break; // N-tilde (EA) 
  }
  #endif //OPTION_DISPLAY_NON_ENGLISH_EXTENSIONS

  if (lcd_status != LCD_SCROLL_MSG) {
    lcd_status = LCD_SCROLL_MSG;
    lcd.clear();
  } 

  if (charin == ' '){
    holding_space = 1;
    return;
  }

  if (holding_space){   // ok, I admit this is a hack.  Hold on to spaces and don't scroll until the next char comes in...
    if (column_pointer > (LCD_COLUMNS-1)) {
      row_pointer++;
      column_pointer = 0;
      if (row_pointer > (LCD_ROWS-1)) {
        for (x = 0; x < (LCD_ROWS-1); x++) {
          lcd_scroll_buffer[x] = lcd_scroll_buffer[x+1];
        }
        lcd_scroll_buffer[x] = "";     
        row_pointer--;
        lcd_scroll_flag = 1;
      }    
    } 
    if (column_pointer > 0){ // don't put a space in the first column
      lcd_scroll_buffer[row_pointer].concat(' ');
      column_pointer++;
    }
    holding_space = 0;
  }

  

  if (column_pointer > (LCD_COLUMNS-1)) {
    row_pointer++;
    column_pointer = 0;
    if (row_pointer > (LCD_ROWS-1)) {
      for (x = 0; x < (LCD_ROWS-1); x++) {
        lcd_scroll_buffer[x] = lcd_scroll_buffer[x+1];
      }
      lcd_scroll_buffer[x] = "";     
      row_pointer--;
      lcd_scroll_flag = 1;
    }    
  } 
  lcd_scroll_buffer[row_pointer].concat(charin);
  column_pointer++;
  

  lcd_scroll_buffer_dirty = 1; 
}


//-------------------------------------------------------------------------------------------------------
void lcd_clear() {

  lcd.clear();
  lcd_status = LCD_CLEAR;

}

//-------------------------------------------------------------------------------------------------------
void lcd_center_print_timed(String lcd_print_string, byte row_number, unsigned int duration)
{
  if (lcd_status != LCD_TIMED_MESSAGE) {
    lcd_previous_status = lcd_status;
    lcd_status = LCD_TIMED_MESSAGE;
    lcd.clear();
  } else {
    clear_display_row(row_number);
  }
  lcd.setCursor(((LCD_COLUMNS - lcd_print_string.length())/2),row_number);
  lcd.print(lcd_print_string);
  lcd_timed_message_clear_time = millis() + duration;
}

//-------------------------------------------------------------------------------------------------------

void clear_display_row(byte row_number)
{
  for (byte x = 0; x < LCD_COLUMNS; x++) {
    lcd.setCursor(x,row_number);
    lcd.print(" ");
  }
}

//-------------------------------------------------------------------------------------------------------
void lcd_center_print_timed_wpm()
{
    #if defined(OPTION_ADVANCED_SPEED_DISPLAY)
      lcd_center_print_timed(String(configuration.wpm) + " wpm - " + (configuration.wpm*5) + " cpm ", 0, default_display_msg_delay);
      lcd_center_print_timed(String(1200/configuration.wpm) + ":" + (((1200/configuration.wpm)*configuration.dah_to_dit_ratio)/100) + "ms 1:" + (float(configuration.dah_to_dit_ratio)/100.00), 1, default_display_msg_delay);
    #else
      lcd_center_print_timed(String(configuration.wpm) + " wpm", 0, default_display_msg_delay);
    #endif
}

//--------------------------------------------------------------------- 

void initialize_display(){
    #if defined(FEATURE_LCD_SAINSMART_I2C)
      lcd.begin();
      lcd.home();
    #else
      lcd.begin(LCD_COLUMNS, LCD_ROWS);
    #endif
    #ifdef FEATURE_LCD_ADAFRUIT_I2C
      lcd.setBacklight(lcdcolor);
    #endif //FEATURE_LCD_ADAFRUIT_I2C

    #ifdef FEATURE_LCD_ADAFRUIT_BACKPACK
      lcd.setBacklight(HIGH);
    #endif


    #ifdef OPTION_DISPLAY_NON_ENGLISH_EXTENSIONS  // OZ1JHM provided code, cleaned up by LA3ZA
      // Store bit maps, designed using editor at http://omerk.github.io/lcdchargen/


      byte U_umlaut[8] =   {B01010,B00000,B10001,B10001,B10001,B10001,B01110,B00000}; // 'Ü'  
      byte O_umlaut[8] =   {B01010,B00000,B01110,B10001,B10001,B10001,B01110,B00000}; // 'Ö'  
      byte A_umlaut[8] =   {B01010,B00000,B01110,B10001,B11111,B10001,B10001,B00000}; // 'Ä'    
      byte AE_capital[8] = {B01111,B10100,B10100,B11110,B10100,B10100,B10111,B00000}; // 'Æ' 
      byte OE_capital[8] = {B00001,B01110,B10011,B10101,B11001,B01110,B10000,B00000}; // 'Ø' 
      byte empty[8] =      {B00000,B00000,B00000,B00000,B00000,B00000,B00000,B00000}; // empty 
      byte AA_capital[8] = {B00100,B00000,B01110,B10001,B11111,B10001,B10001,B00000}; // 'Å'   
      byte Ntilde[8] =     {B01101,B10010,B00000,B11001,B10101,B10011,B10001,B00000}; // 'Ñ' 

      
      
      //     upload 8 charaters to the lcd
      lcd.createChar(0, U_umlaut); //     German
      lcd.createChar(1, O_umlaut); //     German, Swedish
      lcd.createChar(2, A_umlaut); //     German, Swedish 
      lcd.createChar(3, AE_capital); //   Danish, Norwegian
      lcd.createChar(4, OE_capital); //   Danish, Norwegian
      lcd.createChar(5, empty); //        For some reason this one needs to display nothing - otherwise it will display in pauses on serial interface
      lcd.createChar(6, AA_capital); //   Danish, Norwegian, Swedish
      lcd.createChar(7, Ntilde); //       Spanish
      lcd.clear(); // you have to ;o)
    #endif //OPTION_DISPLAY_NON_ENGLISH_EXTENSIONS


    lcd_center_print_timed("K3NG Keyer",0,4000);
}


#endif // FEATURE_DISPLAY
