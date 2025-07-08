//LIBRARIES
#include <Wire.h>              //I2C Driver
#include <Adafruit_GFX.h>      //Graphics library
#include <Adafruit_SSD1306.h>  //SSD1306 OLED Driver

//OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define NUMFLAKES 2  // Number of snowflakes in the animation example
#define LOGO_HEIGHT 50
#define LOGO_WIDTH 50
#define XPOS 0  // Indexes into the 'icons' array in function below
#define YPOS 1
#define DELTAY 2
#define displayTime 200