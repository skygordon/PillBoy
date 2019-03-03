/***************************************************
  This is our GFX example for the Adafruit ILI9341 TFT FeatherWing
  ----> http://www.adafruit.com/products/3315

  Check out the links above for our tutorials and wiring diagrams

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

#include <SPI.h>
//#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

//FOR SMS Capability/////////////////////////
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <string.h>  //used for some string handling and processing.
#include <math.h>
char network[] = "MIT";  //SSID for 6.08 Lab
char password[] = ""; //Password for 6.08 Lab
//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int POSTING_PERIOD = 6000; //periodicity of getting a number fact.
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response
const char message[100] = "You are running low on your medication. Remember to get a refill!";
////////////////////////////////////////////

#define STMPE_CS 16
#define TFT_CS   0
#define TFT_DC   15
#define SD_CS    2

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 20
#define TS_MAXX 3500
#define TS_MINY 40
#define TS_MAXY 3800
 
  int xcoord = 0;
  int ycoord = 0;
  bool pressed = false;
  int prior10 = 0;
  int prior9 = 0;
  int prior8 = 0;
  int prior7 = 0;
  int prior6 = 0;
  int prior5 = 0;
  int prior4 = 0;
  int prior3 = 0;
  int prior2 = 0;
  int prior1 = 0;
  int current = 0;
  uint8_t state;  //system state for screen changes
  int pillamount;
  int pillamountmaybe;
  int taken = 0;
  
// for the text reminder
  int reminder;

 // for the photonresistor
  int photocellPin = 0;     // the cell and 10K pulldown are connected to a0
  int photocellReading;     // the analog reading from the analog resistor divider
  uint8_t bottlestate;
  float oldvalue;
  int lightread;
  unsigned long timer;

  float step(float input, float alpha){
      float output = (alpha*oldvalue)+((1-alpha)*input);
      oldvalue = output;
      return output;
    };

 // for timer
 unsigned long starttime;  //used for storing millis() readings.
 int hr;
  
void setup() {
  Serial.begin(115200);
  delay(10);
  // FOR SMS
WiFi.begin(network,password); //attempt to connect to ESP8266WiFi
  uint8_t count = 0; //count used for ESP8266WiFi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count<12) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(3000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
 
  Serial.println("FeatherWing TFT");
  if (!ts.begin()) {
    Serial.println("Couldn't start touchscreen controller");
    while (1);
  }
  Serial.println("Touchscreen started");
  starttime = millis();
  hr = 0;
  tft.begin();
  tft.setRotation(1);
  state = 0;
  bottlestate = 0;
  startText();
  reminder = 0;
  
  }

  
void loop() {

//for time ahahahahahhhhahahahHAHAHAHAhahhahaa LOVE my LIFE this is so great no sweat
if (millis()-starttime >= 3600000) {
  hr = hr+1;
  starttime = millis();
  }
if (hr >= 24) {
  hr = 0;
  taken = 0;
  starttime = millis();
  }
  
//for reminder
if ((taken==1) && (reminder==0) && (pillamount <= 5)) {
  reminder = reminder + 1;
  textmessage();
  // send reminder text
  }

//for the light stuff
  photocellReading = analogRead(photocellPin);  
  Serial.print("Analog reading = ");
  Serial.print(lightread);     // the raw analog reading

  lightread = step(photocellReading, 0.9);
  bottle();

//touchscreen pressed? sensing
if (ts.touched()) {
    pressed = true;
    } else {
      pressed = false;
    }
    
 TS_Point p = ts.getPoint();
  // Scale from ~0->4000 to tft.width using the calibration #'s
  xcoord = int(map(p.x, TS_MINX, TS_MAXX, 0, tft.width()));
  ycoord = int(map(p.y, TS_MINY, TS_MAXY, 0, tft.height()));

  
  screenswitch();

 Serial.print("X = "); Serial.print(xcoord);
 Serial.print("\tY = "); Serial.print(ycoord);  
 Serial.print("\tPressure = "); Serial.println(p.z);  

}
  

void screenswitch(){
  switch(state){
    case 0: //Home Screen
    {
      if (pressed){

        if ((290<xcoord) && (xcoord<320) && (110<ycoord) && (ycoord<130)) {
          pillamount = pillamount-1;
          startText();
          delay(500);
      
          } else if ((290<xcoord) && (xcoord<320) && (185<ycoord) && (ycoord<210)) {
            pillamount = pillamount+1;
            startText();
            delay(500);
            
          } else if ( (170<xcoord) && (xcoord<235) && (125<ycoord) && (ycoord<195)) { 
        state = 1;
        RefillScreen();
        delay(1000);
        }
     }
  }
      break; 
    case 1: //Refill Screen
    {
      if ((pressed) && (130<xcoord) && (xcoord<220) && (25<ycoord) && (ycoord<52)) {
        pillamountmaybe = 30;
        state = 2;
        finaleditScreen();
        }
      if ((pressed) && (130<xcoord) && (xcoord<220) && (99<ycoord) && (ycoord<130)) {
        pillamountmaybe = 60;
        state = 2;
        finaleditScreen();
        }
      if ((pressed) && (130<xcoord) && (xcoord<220) && (178<ycoord) && (ycoord<215)) {
        pillamountmaybe = 90;
        state = 2;
        finaleditScreen();
        }
  }
     break;
     case 2: //Checking to make sure they want to edit pill amount
     { if ((pressed) && (235<xcoord) && (xcoord<300) && (67<ycoord) && (ycoord<115)) {
      pillamount = pillamountmaybe;
      state = 0;
      startText();
         }
      if ((pressed) && (235<xcoord) && (xcoord<300) && (160<ycoord) && (ycoord<210)) {
      state = 0;
      startText();
         }
      }
      
      
      }}


void bottle(){
  switch(bottlestate){
    case 0: {//Home Screen
      if (lightread > 1000) {
        timer = millis();
        bottlestate = 1;
    } 
    }
      break;
    case 1: {
      if ((lightread>1000) && (millis()-timer>3000)) {
        Serial.print("to CASE 2");
        reminder = 0;
        taken = 1;
        pillamount = pillamount-1;
        bottlestate = 2;
        state = 0;
        startText();
        timer = millis();
        } else if ((lightread<1000) && (millis()-timer>3000)) {
          bottlestate = 0;
          }
      }
      break;
    case 2: {
      if  ((lightread<900) && (millis()-timer>5000)) {
        bottlestate = 0;
        delay(2000);
        }
      }
      break;
  }}

unsigned long textmessage() {
  char body[100] = "";
      sprintf(body,"text=%s",message);//generate body, posting to User, steps variable value
      int body_len = strlen(body); //calculate body length (for header reporting)
      sprintf(request_buffer,"POST http://608dev.net/sandbox/sc/caseyjoh/makemitserver.py HTTP/1.1\r\n");
      strcat(request_buffer,"Host: 608dev.net\r\n");
      strcat(request_buffer,"Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
      strcat(request_buffer,"\r\n"); //new line from header to body
      strcat(request_buffer,body); //body
      strcat(request_buffer,"\r\n"); //header
      Serial.println(request_buffer);
      do_http_request("608dev.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);
      Serial.println(response_buffer); //viewable in Serial Terminal
     }
    
unsigned long startText() {
  tft.fillScreen(ILI9341_BLACK);
  tft.fillRect(140, 110, 140, 50, ILI9341_WHITE);
  tft.setCursor(150, 125);
  tft.setTextColor(ILI9341_BLUE);  tft.setTextSize(3);
  tft.println("Refill?");
  tft.fillRect(140, 180, 60, 50, ILI9341_CYAN);
  tft.fillRect(220, 180, 60, 50, ILI9341_CYAN);
  tft.setCursor(155, 185);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(6);
  tft.println("-");
  tft.setCursor(235, 185);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(6);
  tft.println("+");
  
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("");
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(2);
  tft.println("Good Morning!");
  tft.println("");
  
  if (taken == 0) {
  tft.setTextColor(ILI9341_MAGENTA);
  tft.setTextSize(3);
  tft.println("You Haven't Taken Your Meds Today");
  } else {
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(3);
  tft.println("You Have Taken");
  tft.println(" Your Meds Today!");
    }

  
  tft.setCursor(0, 100);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  tft.println("Amount");
  tft.setCursor(0, 130);
  tft.println("Left:");
  tft.println("");
  tft.setTextSize(5);
  tft.println(pillamount);
}


unsigned long RefillScreen() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("");
  tft.setTextColor(ILI9341_WHITE); tft.setTextSize(3);
  tft.println("Refill Amount?");
  tft.fillRect(10, 75, 90, 90, ILI9341_CYAN);
  tft.fillRect(115, 75, 90, 90, ILI9341_CYAN);
  tft.fillRect(220, 75, 90, 90, ILI9341_CYAN);
  tft.setCursor(33, 105);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(4);
  tft.println("30");
  tft.setCursor(138, 105);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(4);
  tft.println("60");
  tft.setCursor(243, 105);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(4);
  tft.println("90");
  }

unsigned long finaleditScreen() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(4);
  tft.println("Are You Sure");
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("");
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(4);
  tft.println("the Pill");
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("");
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(4);
  tft.println("Amount is");
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(1);
  tft.println("");
  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(5);
  tft.println(pillamountmaybe);
  tft.fillRect(70, 150, 90, 75, ILI9341_CYAN);
  tft.fillRect(200, 150, 90, 75, ILI9341_CYAN);
  tft.setCursor(80, 175);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(4);
  tft.println("Yes");
  tft.setCursor(225, 175);
  tft.setTextColor(ILI9341_BLACK);  tft.setTextSize(4);
  tft.println("No");
}
 
