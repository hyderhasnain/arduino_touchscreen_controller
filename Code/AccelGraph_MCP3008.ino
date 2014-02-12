//----------Touchscreen Controller Accel XYZ Graphing Program----------\\
//----------------Hyder Hasnain; Started July 27, 2013-----------------\\
//------------------Also records data on microSD card------------------\\
//-------------!!!CODE FOR SD CARD TAKES UP A LOT OF SPACE...----------\\
//-------------...SO IT IS COMMENTED-OUT IN THIS VERSION...------------\\
//---------------...TO MAKE ROOM FOR TOUCHSCREEN CODE!!!---------------\\
//--------!!!IF YOU WANT SD LOGGING, REMOVE TOUCHSCREEN CODE!!!--------\\

//--------ALL THE EXTRA COMMENTED CODE IS EXTRA, IF YOU WANT IT--------\\

//--------------------------------NOTES--------------------------------\\
//TFT Characters are 6x8 pixels
//tft.fillRect(x, y, width, height, color);

//--------------------------------LIBRARIES----------------------------\\

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_GFX.h> 
#include <Adafruit_TFTLCD.h> 
#include <TouchScreen.h>
//#include <SD.h>
#include <Time.h>
#include <DS3231RTC.h>
#include <EasyTransfer.h>
#include <MCP3008.h>

//-------------------------------DEFINITIONS---------------------------\\

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
//!!!***!!!
#define LCD_RESET A3 //!!!change in your own code to something else!!!; I don't use this personally, so mine doesn't matter!!!
//!!!***!!!
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 115 //min/max values for touchscreen
#define TS_MINY 124 //!!!THIS IS UNIQUE FOR EVERY TOUCHSCREEN!!!
#define TS_MAXX 914 
#define TS_MAXY 934

#define MINPRESSURE 10 //min/max pressure for touchscreen
#define MAXPRESSURE 1000

#define ADC_CS_PIN A1
#define ADC_CLOCK_PIN 13
#define ADC_MOSI_PIN 11
#define ADC_MISO_PIN 12

MCP3008 adc(ADC_CLOCK_PIN, ADC_MOSI_PIN, ADC_MISO_PIN, ADC_CS_PIN);

EasyTransfer ETSerial;

struct SEND_DATA_STRUCTURE // Set of data used in EasyTransfer communication
{
  int xAng;
  int yAng;
  int zAng;
  int xDeg;
  int yDeg;
  //int zDeg;
  float rX;
  float rY;
  //float rZ;
  float rRadius;
  int rDegree;
  int buttonState;
};

int rX;
int rY;
int rZ;

SEND_DATA_STRUCTURE accelData;

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345); //assign ID to our accelerometer

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET); //define LCD connection pins
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 372); //define touchscreen connection pins

//-------------------------------VARIABLES-----------------------------\\

int x_pos; //position along the graph x axis
float y_pos_x; //current graph y axis position of X value
float y_pos_x_old = 120; //old y axis position of X value
float y_pos_y; //current graph y axis position of Y value
float y_pos_y_old = 120; //old y axis position of Y value
float y_pos_z; //current graph y axis position of Z value
float y_pos_z_old = 120; //old y axis position of Z value
byte x_scale = 1; //scale of graph x axis, controlled by touchscreen buttons
byte y_scale = 1;
int accel_x; //the XYZ values stored on the SD card
int accel_y; 
int accel_z;
byte displayHour;
byte displayMinute;
byte displaySecond;
byte displayAMPM = 0;
int displayADC1;
int displayADC2;
int displayADC3;
int displayADC4;
int displayADC5;
int displayADC6;
int displayADC7;
int displayADC8;

byte SD_CS = 10; //SPI CS pin for SD card

//---------------------------------SETUP-------------------------------\\

void setup()
{

  Serial.begin(57600); //for debugging

  //begin the LCD
  tft.reset();
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFTLCD_BLACK);


  //sync time with RTC
  setSyncProvider(RTC.get);

  if(!accel.begin()) //begin accel, check if it's working
  {
    tft.setCursor(150, 50); 
    tft.setTextColor(TFTLCD_WHITE);
    tft.print("No ADXL345 detected");
    while(1);
  }

  accel.setRange(ADXL345_RANGE_2_G); //set resolution of accelerometer

  /*
  //begin SD card
   pinMode(SD_CS, OUTPUT);
   SD.begin(SD_CS);  
   
   //Place a new header on the log file each time the program runs
   File logFile = SD.open("LOG.csv", FILE_WRITE);
   if (logFile)
   {
   logFile.println(", , , ,");
   String header = "Time, X, Y, Z";
   logFile.println(header);
   logFile.close();
   }
   */

  tftDrawGraphObjects(); //draw graph objects
  tftDrawColorKey();
  tftDrawXScaleButtons();
  tftDrawYScaleButtons();



}

//------------------------------MAIN PROGRAM--------------------------\\

void loop()
{
  //---------MAIN 'FOR' LOOP! THIS IS WHERE ALL THE ACTION HAPPENS! HAS TO BE FAST!!!!!---------\\


  for (x_pos = (11 + x_scale); x_pos <= 320; x_pos += x_scale) //go along every point on the x axis and do something, start over when finished
  {

    Point p = ts.getPoint(); //get touch point!
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.height()); //remap p.x and p.y to usable values
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.width(), 0);

    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) //if pressed enough
    {
      //!!!!!!!! X SCALE BUTTONS !!!!!!!!
      if ( (p.y >= 210 && p.y <= 230) && (p.x >= 0 && p.x <= 20) ) //if x "-" button pressed!
      {
        if (x_scale > 1) 
        {
          x_scale --; //decrement x scale
          delay(30);
          tft.fillRect(256, 21, 6, 8, TFTLCD_BLACK); 
          tft.setCursor(254, 21); 
          tft.setTextSize(1); 
          tft.setTextColor(TFTLCD_WHITE); 
          tft.print(x_scale); //print the current x scale value
          break; //restart 'for' loop
        }
      }
      if ( (p.y >= 239 && p.y <= 259) && (p.x >= 0 && p.x <= 20) ) //if x "+" button pressed!
      {
        if (x_scale < 6)
        {
          x_scale ++; //increment x scale
          delay(30);
          tft.fillRect(256, 21, 6, 8, TFTLCD_BLACK); 
          tft.setCursor(254, 21); 
          tft.setTextSize(1); 
          tft.setTextColor(TFTLCD_WHITE); 
          tft.print(x_scale); //print the current x scale value
          break; //restart 'for' loop
        }
      }
      //!!!!!!!! Y SCALE BUTTONS !!!!!!!!
      if ( (p.y >= 266 && p.y <= 286) && (p.x >= 0 && p.x <= 20) ) //if x "-" button pressed!
      {
        if (y_scale > 1) 
        {
          y_scale --; //decrement x scale
          delay(30);
          tft.fillRect(312, 21, 6, 8, TFTLCD_BLACK); 
          tft.setCursor(311, 21); 
          tft.setTextSize(1); 
          tft.setTextColor(TFTLCD_WHITE); 
          tft.print(y_scale); //print the current y scale value
          break; //restart 'for' loop
        }
      }
      if ( (p.y >= 295 && p.y <= 315) && (p.x >= 0 && p.x <= 20) ) //if x "+" button pressed!
      {
        if (y_scale < 8)
        {
          y_scale ++; //increment x scale
          delay(30);
          tft.fillRect(312, 21, 6, 8, TFTLCD_BLACK); 
          tft.setCursor(310, 21); 
          tft.setTextSize(1); 
          tft.setTextColor(TFTLCD_WHITE); 
          tft.print(y_scale); //print the current y scale value
          break; //restart 'for' loop
        }
      } 
    }

    //get accel data
    sensors_event_t event;
    accel.getEvent(&event);
    //store accel data, convert
    /* accel_x = event.acceleration.x; //values to store onto SD card, these can only be ints, not floats, so we need separate variables!
     accel_y = event.acceleration.y;
     accel_z = event.acceleration.z; */
    y_pos_x = ((-event.acceleration.x * (y_scale * 10)) + 120); //values to use when displaying on LCD, these are floats, for more precision!
    y_pos_y = ((-event.acceleration.y * (y_scale * 10)) + 120); // 120 is axis, so value is displacement from axis, scaled for better visisility!
    y_pos_z = ((-event.acceleration.z * (y_scale * 10)) + 120);

    /* //------------------------LCD DISPLAY CODE----------------------\\
     //display accel data to LCD
     tft.setTextColor(TFTLCD_WHITE); tft.setTextSize(1);
     tft.setCursor(200, 30); 
     tft.print("X New:"); tft.print(event.acceleration.x);
     tft.setCursor(200, 38);
     tft.print("Y New:"); tft.print(event.acceleration.y);
     tft.setCursor(200, 46);
     tft.print("Z New:"); tft.print(event.acceleration.z);
     */

    /*
    //CODE FOR PLOTTING POINTS
     //plot "X" value
     tft.drawPixel(x_pos, y_pos_x, TFTLCD_GREEN); //plot single point 
     tft.drawFastVLine(x_pos+1, y_pos_x-3, 6, TFTLCD_GREEN); //draw 'cursor' in front of point
     //plot "Y" value
     tft.drawPixel(x_pos, y_pos_y, TFTLCD_PINK);
     tft.drawFastVLine(x_pos+1, y_pos_y-3, 6, TFTLCD_PINK);
     //plot "Z" value
     tft.drawPixel(x_pos, y_pos_z, TFTLCD_RED);
     tft.drawFastVLine(x_pos+1, y_pos_z-3, 6, TFTLCD_RED);
     delay(150);
     //clear 'cursor' in front of point
     tft.drawFastVLine(x_pos+1, y_pos_x-3, 6, TFTLCD_BLACK);
     tft.drawFastVLine(x_pos+1, y_pos_y-3, 6, TFTLCD_BLACK);
     tft.drawFastVLine(x_pos+1, y_pos_z-3, 6, TFTLCD_BLACK);
     */

    /*
    //CODE FOR PLOTTING CIRCLES
     //plot "X" value
     tft.drawCircle(x_pos, y_pos_x, 1, TFTLCD_GREEN);
     //plot "Y" value
     tft.drawCircle(x_pos, y_pos_y, 1, TFTLCD_PINK);
     //plot "Z" value
     tft.drawCircle(x_pos, y_pos_z, 1, TFTLCD_RED);
     delay(50);
     */

    //CODE FOR PLOTTING CONTINUOUS LINES!!!!!!!!!!!!
    //Plot "X" value
    tft.drawLine(x_pos - x_scale, y_pos_x_old, x_pos, y_pos_x, TFTLCD_GREEN);
    //Plot "Y" value
    tft.drawLine(x_pos - x_scale, y_pos_y_old, x_pos, y_pos_y, TFTLCD_RED);
    //Plot "Z" value
    tft.drawLine(x_pos - x_scale, y_pos_z_old, x_pos, y_pos_z, TFTLCD_BLUE);

    //Draw preceding black 'boxes' to erase old plot lines, !!!WEIRD CODE TO COMPENSATE FOR BUTTONS AND COLOR KEY SO 'ERASER' DOESN'T ERASE BUTTONS AND COLOR KEY!!!
    if ((x_pos >= 198) && (x_pos <= 320)) //above x axis
    {
      tft.fillRect(x_pos+1, 28, 10, 92, TFTLCD_BLACK); //compensate for buttons!
    }
    else
    {
      tft.fillRect(x_pos+1, 0, 10, 120, TFTLCD_BLACK); //don't compensate for buttons!
    }
    if ((x_pos >= 254) && (x_pos <= 320)) //below x axis
    {
      tft.fillRect(x_pos+1, 121, 10, 88, TFTLCD_BLACK);
    }
    else
    {
      tft.fillRect(x_pos+1, 121, 10, 119, TFTLCD_BLACK);
    }

    tftDisplayTime();

    if ( (y_pos_x == 120) || (y_pos_y == 120) || (y_pos_z == 120) )
    {
      tft.drawFastHLine(10, 120, 310, TFTLCD_WHITE); // x axis
    }

    y_pos_x_old = y_pos_x; //set old y pos values to current y pos values 
    y_pos_y_old = y_pos_y;
    y_pos_z_old = y_pos_z;

    //tft.fillRect(230, 30, 50, 24, TFTLCD_BLACK); //clear displayed accel data from LCD

    //tft.fillRect(230, 200, 70, 8, TFTLCD_BLACK); //clear displayed time

    /*
    //Record XYZ data to SD card
     String DataString = String(hour()) + ":" + String(minute()) + ":" + String(second()) + " , " + String(accel_x) + " , " + String(accel_y) + " , " + String(accel_z);
     File logFile = SD.open("LOG.csv", FILE_WRITE); //open log file on SD card
     if (logFile)
     {
     logFile.println(DataString); //print log date to file
     logFile.close(); //close log file
     }
     */
    /*
    float rX = (event.acceleration.x / 10);
     float rY = (event.acceleration.y / 10);
     float rZ = (event.acceleration.z / 10);
     Serial.print("xValue= "); Serial.print(rX);
     Serial.print(" yValue= "); Serial.print(rY);
     Serial.print(" zValue= "); Serial.println(rZ);
     //delay(50);
     */

    //accelData.rX = rX;
    //accelData.rY = rY;
    //accelData.rZ = rZ;
    //ETSerial.sendData();
    //delay(50);

    int Ch1Val = adc.readADC(0); int Ch2Val = adc.readADC(1); int Ch3Val = adc.readADC(2); int Ch4Val = adc.readADC(3);
    int Ch5Val = adc.readADC(4); int Ch6Val = adc.readADC(5); int Ch7Val = adc.readADC(6); int Ch8Val = adc.readADC(7);

    if (displayADC1 != Ch1Val) {
      tft.fillRect(25, 220, 30, 8, TFTLCD_BLACK); }

    if (displayADC2 != Ch2Val) {
      tft.fillRect(55, 220, 30, 8, TFTLCD_BLACK); }

    if (displayADC3 != Ch3Val) {
      tft.fillRect(85, 220, 30, 8, TFTLCD_BLACK); }

    if (displayADC4 != Ch4Val) {
      tft.fillRect(115, 220, 30, 8, TFTLCD_BLACK); }

    if (displayADC5 != Ch5Val) {
      tft.fillRect(145, 220, 30, 8, TFTLCD_BLACK); }

    if (displayADC6 != Ch6Val) {
      tft.fillRect(175, 220, 30, 8, TFTLCD_BLACK); }

    if (displayADC7 != Ch7Val) {
      tft.fillRect(205, 220, 30, 8, TFTLCD_BLACK); }

    if (displayADC8 != Ch8Val) {
      tft.fillRect(235, 220, 30, 8, TFTLCD_BLACK); }   

    tft.setTextColor(TFTLCD_WHITE); 
    tft.setTextSize(1);
    tft.setCursor(25, 220); tft.print(Ch1Val); tft.print(" ");
    tft.setCursor(55, 220); tft.print(Ch2Val); tft.print(" ");
    tft.setCursor(85, 220); tft.print(Ch3Val); tft.print(" ");
    tft.setCursor(115, 220); tft.print(Ch4Val); tft.print(" ");
    tft.setCursor(145, 220); tft.print(Ch5Val); tft.print(" ");
    tft.setCursor(175, 220); tft.print(Ch6Val); tft.print(" ");
    tft.setCursor(205, 220); tft.print(Ch7Val); tft.print(" ");
    tft.setCursor(235, 220); tft.print(Ch8Val);
    
    displayADC1 = Ch1Val; displayADC2 = Ch2Val; displayADC3 = Ch3Val; displayADC4 = Ch4Val;
    displayADC5 = Ch5Val; displayADC6 = Ch6Val; displayADC7 = Ch7Val; displayADC8 = Ch8Val;
    
    delay(5);

    /*
   Serial.print("L Joystick U/D: "); Serial.print(Ch1Val);
     Serial.print("  L Joystick R/L: "); Serial.print(Ch2Val);
     Serial.print("  R Joystick U/D: "); Serial.print(Ch3Val);
     Serial.print("  L Joystick U/D: "); Serial.print(Ch4Val);
     Serial.print("  L Joystick Button: "); Serial.print(Ch5Val);
     Serial.print("  L Joystick Button: "); Serial.print(Ch6Val);
     Serial.print("  Light Sensor: "); Serial.print(Ch7Val);
     Serial.print("  Battery: "); Serial.println(Ch8Val);
     delay(5);
     */

  }

  tft.fillRect(208, 0, 112, 28, TFTLCD_BLACK); //erase XY buttons and any lines behind them
  tft.fillRect(254, 208, 66, 32, TFTLCD_BLACK); //erase time and color key and any stray lines behind them

  tftDrawXScaleButtons(); //redraw stuff
  tftDrawYScaleButtons();
  tftDrawColorKey();
  tftDrawGraphObjects();

  //Completely clear out old plot points/lines
  //tft.fillRect(21, 0, 299, 119, TFTLCD_BLACK); 
  //tft.fillRect(21, 111, 299, 119, TFTLCD_BLACK); 


}

//-------------------------------FUNCTIONS---------------------------\\

void printDigits(int digits)  // utility function for digital clock display: prints preceding colon and leading 0
{
  tft.print(":");
  if(digits < 10)
    tft.print('0');
  tft.print(digits);
}

void tftDrawXScaleButtons()
{
  //draw touch controls to change x axis resolution
  tft.fillRect(210, 0, 20, 20, TFTLCD_WHITE); //draw "-" box
  tft.fillRect(239, 0, 20, 20, TFTLCD_WHITE); //draw "+" box
  tft.setCursor(215, 3); 
  tft.setTextColor(TFTLCD_GREEN); 
  tft.setTextSize(2); 
  tft.print("-"); //print "-"
  tft.setCursor(244, 3); 
  tft.print("+"); 
  tft.drawFastVLine(234, 0, 20, TFTLCD_WHITE);
  tft.setCursor(208, 21); 
  tft.setTextColor(TFTLCD_WHITE); 
  tft.setTextSize(1); 
  tft.print("X Scale:"); 
  tft.print(x_scale);
}

void tftDrawYScaleButtons()
{
  //draw touch controls to change x axis resolution
  tft.fillRect(266, 0, 20, 20, TFTLCD_WHITE); //draw "-" box
  tft.fillRect(295, 0, 20, 20, TFTLCD_WHITE); //draw "+" box
  tft.setCursor(271, 3); 
  tft.setTextColor(TFTLCD_RED); 
  tft.setTextSize(2); 
  tft.print("-"); //print "-"
  tft.setCursor(300, 3); 
  tft.print("+");
  tft.drawFastVLine(290, 0, 20, TFTLCD_WHITE);  
  tft.setCursor(265, 21); 
  tft.setTextColor(TFTLCD_WHITE); 
  tft.setTextSize(1); 
  tft.print("Y Scale:"); 
  tft.print(y_scale);
}


void tftDrawColorKey()
{
  //Display color key
  tft.setTextSize(1); 
  tft.setTextColor(TFTLCD_WHITE);
  tft.fillRect(280, 216, 15, 8, TFTLCD_GREEN); 
  tft.setCursor(296, 216); 
  tft.print(" - X");
  tft.fillRect(280, 224, 15, 8, TFTLCD_RED); 
  tft.setCursor(296, 224); 
  tft.print(" - Y");
  tft.fillRect(280, 232, 15, 8, TFTLCD_BLUE); 
  tft.setCursor(296, 232); 
  tft.print(" - Z");  
}

void tftDrawGraphObjects()
{
  //draw the graph objects
  tft.fillRect(11, 5, x_scale+1, 120, TFTLCD_BLACK);
  tft.fillRect(11, 121, x_scale+1, 119, TFTLCD_BLACK);
  tft.drawFastVLine(10, 5, 230, TFTLCD_WHITE); // y axis
  tft.drawFastHLine(10, 120, 310, TFTLCD_WHITE); // x axis
  tft.setTextColor(TFTLCD_YELLOW); 
  tft.setTextSize(1); // set parameters for y axis labels
  tft.setCursor(3, 116); 
  tft.print("0");  // "0" at center of ya axis
  tft.setCursor(3, 6); 
  tft.print("+"); // "+' at top of y axis
  tft.setCursor(3, 228); 
  tft.print("-"); // "-" at bottom of y axis

}

void tftDisplayTime()
{
  //Display current time    
  if (displayHour != hour()) //if hour has updated
  { 
    tft.fillRect(254, 208, 18, 8, TFTLCD_BLACK); //clear out old hour
    if (hour() > 12) //since RTC reports time in 24 hour format, if greater than 12...
    {
      displayHour = hour() - 12; //set for PM
    }
    if (hour() >= 12)
    {
      displayAMPM = 1; //set for PM
    }
    if (hour() <= 12)
    {
      displayHour = hour(); //update hour //set for AM
      displayAMPM = 0;
    }
  }

  if (displayMinute != minute()) //if minute has updated
  { 
    tft.fillRect(266, 208, 18, 8, TFTLCD_BLACK); //clear out old minute
    displayMinute = minute(); //update minute
  }

  if (displaySecond != second()) //if second has updated
  {
    tft.fillRect(284, 208, 18, 8, TFTLCD_BLACK); //clear out old second
    displaySecond = second(); //update second
    tft.drawFastVLine(x_pos, 117, 6, TFTLCD_PINK);
  }

  tft.setCursor(254, 208); 
  tft.setTextColor(TFTLCD_WHITE); 
  tft.setTextSize(1); //Set text position, color
  tft.print(displayHour); 
  printDigits(displayMinute); 
  printDigits(displaySecond);
  if (displayAMPM == 0) //if AM
  {
    tft.print(" AM");
  }
  if (displayAMPM == 1) //if PM
  {
    tft.print(" PM");
  }
}

