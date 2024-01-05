#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <DFRobot_PAJ7620U2.h>
#include <FastLED.h>

DFRobot_PAJ7620U2 paj;
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

#define LIMIT_SWITCH 4
#define BUTTON_PIN 2
#define NEO_A_DATA 3
#define NEO_B_DATA 6
#define BRIGHTNESS 64
#define COLOR_ORDER GRB
#define LED_TYPE WS2812B
#define UPDATES_PER_SECOND 100

CRGB neoAleds[28];
CRGB neoBleds[1];

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

const int in1 = 7;
const int in2 = 8;
const int en = 5;
bool closed = false;
bool opened = true;
bool gesture_ready = false;
bool play_status = true;
bool random_status = false;
bool currentRandomStatus = false;
bool previousRandomStatus = false;
int current_volume = 2;

void setup() {
  FastLED.addLeds<LED_TYPE, NEO_A_DATA, COLOR_ORDER>(neoAleds, 28).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<LED_TYPE, NEO_B_DATA, COLOR_ORDER>(neoBleds, 1).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(BRIGHTNESS);
  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
    
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  myDFPlayer.begin(mySoftwareSerial);
  delay(200);
  myDFPlayer.volume(current_volume);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  pinMode(LIMIT_SWITCH, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(en, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  digitalWrite(en, HIGH);

  myDFPlayer.enableLoopAll();
  myDFPlayer.start();
}


void loop() {
  if (currentRandomStatus != previousRandomStatus){
    if (random_status){
      myDFPlayer.randomAll();
    }
    if (!random_status){
      myDFPlayer.enableLoopAll();
    }
    currentRandomStatus = previousRandomStatus;
    myDFPlayer.start();
  }
  
  if(!gesture_ready){
    neoBleds[0] = CRGB::Black;
    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; 
    
    FillLEDsFromPaletteColors(startIndex);
    
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
  }
  
  if (closed==false && opened && gesture_ready==false){
    sub_close();
    if (digitalRead(LIMIT_SWITCH) == LOW){
      stop_move();
      delay(200);
      sub_open();
      delay(500);
      stop_move();
      closed = true;
      opened = false;
    }
  }
  
  if (digitalRead(BUTTON_PIN) == LOW){
    if (closed && opened==false){
      sub_open();

      for (int i=0; i<28; i++){
        neoAleds[i] = CRGB::Black;
        FastLED.show();
      }

      static uint8_t thrustIndex = 0;
      thrustIndex = thrustIndex + 1; 
      
      uint8_t brightness = 255;
      for (int j=0; j<300; j++){
        neoBleds[0] = ColorFromPalette( currentPalette, thrustIndex, brightness, currentBlending);
        thrustIndex += 3;
      
        FastLED.show();
        FastLED.delay(1000 / UPDATES_PER_SECOND);
      }
      gesture_ready = true;
      stop_move();
      opened = true;
    }
  }
  
  if (gesture_ready){
    paj.begin();
    paj.setGestureHighRate(false); 
    current_volume = myDFPlayer.readVolume();


    DFRobot_PAJ7620U2::eGesture_t gesture = paj.getGesture();
    if(gesture != paj.eGestureNone ){
      if (gesture == paj.eGestureRight || gesture == paj.eGestureClockwise){
        next_track_lights();
        myDFPlayer.next();
      }
      else if (gesture == paj.eGestureLeft || gesture == paj.eGestureAntiClockwise){
        previous_track_lights();
        myDFPlayer.previous();
      }
      else if (gesture == paj.eGestureUp){
        volume_up_lights();
        Serial.println(current_volume);
        switch(current_volume){
          case(0): {
            myDFPlayer.volume(2);
            break;
          }
          case(2): {
            myDFPlayer.volume(6);
            break;
          }
          case(6): {
            myDFPlayer.volume(10);
            break;
          }
          case(10): {
            myDFPlayer.volume(16);
            break;
          }
          case(16): {
            myDFPlayer.volume(21);
            break;
          }
          case(21): {
            myDFPlayer.volume(26);
            break;
          }
          case(26): {
            myDFPlayer.volume(30);
            break;
          }
          case(30): {
            myDFPlayer.volume(30);
            break;
          }
          default:;
        }
      Serial.println(current_volume);
      }
      else if (gesture == paj.eGestureDown){
        volume_down_lights();
        Serial.println(current_volume);
        switch(current_volume){
          case(0): {
            myDFPlayer.volume(0);
            break;
          }
          case(2): {
            myDFPlayer.volume(0);
            break;
          }
          case(6): {
            myDFPlayer.volume(2);
            break;
          }
          case(10): {
            myDFPlayer.volume(6);
            break;
          }
          case(16): {
            myDFPlayer.volume(10);
            break;
          }
          case(21): {
            myDFPlayer.volume(16);
            break;
          }
          case(26): {
            myDFPlayer.volume(21);
            break;
          }
          case(30): {
            myDFPlayer.volume(26);
            break;
          }
          default:;
        }

      }
      else if (gesture == paj.eGestureWaveSlowlyUpDown){
        play_status = !play_status;
        pause_play_lights();
        if (play_status){
          myDFPlayer.start();
        } else {
          myDFPlayer.pause();
        } 
      }
      else if (gesture == paj.eGestureWaveSlowlyForwardBackward){
        random_status = !random_status;
        currentRandomStatus = !previousRandomStatus;
        play_mode_lights();
        myDFPlayer.stop();
      }

      else if (gesture == paj.eGestureWaveSlowlyLeftRight || gesture == paj.eGestureWaveSlowlyDisorder){
        gesture_ready = false;
        closed = false;
        opened = true;
      }
    }
  }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    
    for( int i = 0; i < 28; ++i) {
        neoAleds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}

void pause_play_lights(){
  for (int i=0; i<28; i++){
    neoAleds[i] = CRGB::Black;
    FastLED.show();
  }
  for (int m=0; m<2; m++){
    for (int j=0; j<8; j++){
      neoAleds[j] = CRGB::Cyan;
      FastLED.show();
    }
    FastLED.delay(100);
    for (int i=0; i<28; i++){
      neoAleds[i] = CRGB::Black;
      FastLED.show();
    }
    for (int k=14; k<22; k++){
      neoAleds[k] = CRGB::Cyan;
      FastLED.show();
    }
    FastLED.delay(100);
    for (int i=0; i<28; i++){
      neoAleds[i] = CRGB::Black;
      FastLED.show();
    }
  }
  for (int n=0; n<28; n++){
    neoAleds[n] = CRGB::Cyan;
    FastLED.show();
   }
}

void play_mode_lights(){
  for (int i=0; i<28; i++){
    neoAleds[i] = CRGB::Black;
    FastLED.show();
  }
  
  for (int j=0; j<28; j++){
    neoAleds[j] = CRGB::Cyan;
    FastLED.delay(20);
    FastLED.show();
  }
  FastLED.delay(100);

  for (int i=0; i<28; i++){
    neoAleds[i] = CRGB::Black;
    FastLED.show();
  }
  for (int j=28; j>=0; j--){
    neoAleds[j] = CRGB::Cyan;
    FastLED.delay(20);
    FastLED.show();
  }
  FastLED.delay(100);
  
  for (int i=0; i<28; i++){
    neoAleds[i] = CRGB::Black;
    FastLED.show();
  }
  
  for (int j=0; j<28; j++){
    neoAleds[j] = CRGB::Cyan;
    FastLED.delay(20);
    FastLED.show();
  }
}

void next_track_lights(){
  for (int i=0; i<28; i++){
    neoAleds[i] = CRGB::Black;
    FastLED.show();
  }
  
  for (int j=0; j<28; j++){
    neoAleds[j] = CRGB::Cyan;
    FastLED.delay(20);
    FastLED.show();
  }
}

void previous_track_lights(){
  for (int i=0; i<28; i++){
    neoAleds[i] = CRGB::Black;
    FastLED.show();
  }
  for (int j=28; j>=0; j--){
    neoAleds[j] = CRGB::Cyan;
    FastLED.delay(20);
    FastLED.show();
  }
}

void volume_up_lights(){
  for (int i=0; i<28; i++){
    neoAleds[i] = CRGB::Black;
    FastLED.show();
  }
  for (int j=0; j<8; j++){
    neoAleds[j] = CRGB::Cyan;
    FastLED.show();
  }
  FastLED.delay(200);
  for (int k=8; k<14; k++){
    neoAleds[k] = CRGB::Cyan;
    FastLED.show();
  }
  FastLED.delay(200);
  for (int l=14; l<22; l++){
    neoAleds[l] = CRGB::Cyan;
    FastLED.show();
  }
  FastLED.delay(200);
  for (int m=22; m<28; m++){
    neoAleds[m] = CRGB::Cyan;
    FastLED.show();
  }
  
}

void volume_down_lights(){
  for (int i=0; i<28; i++){
    neoAleds[i] = CRGB::Black;
    FastLED.show();
  }
  for (int j=22; j<28; j++){
    neoAleds[j] = CRGB::Cyan;
    FastLED.show();
  }
  FastLED.delay(200);
  for (int k=14; k<22; k++){
    neoAleds[k] = CRGB::Cyan;
    FastLED.show();
  }
  FastLED.delay(200);
  for (int l=8; l<14; l++){
    neoAleds[l] = CRGB::Cyan;
    FastLED.show();
  }
  FastLED.delay(200);
  for (int m=0; m<8; m++){
    neoAleds[m] = CRGB::Cyan;
    FastLED.show();
  }
}


void sub_close(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
}

void sub_open(){
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
}

void stop_move(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
}
