/*  
 *  A2600 Paddles/Spinners USB Adapter
 *  (C) Alexey Melnikov 
 *   
 *  Based on project by Mikael Norrgård <mick@daemonbite.com>
 *  
 *  GNU GENERAL PUBLIC LICENSE
 *  Version 3, 29 June 2007
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  
 */


/* Original code from Alexey Melnikov
 * https://github.com/MiSTer-devel/Retro-Controllers-USB-MiSTer/blob/master/PaddleTwoControllersUSB/
 * 
 * Modified by sonik-br
 * Uses DigitalIO lib for faster pin access
 * Uses SSD1306Ascii and SoftWire for OLED display
 * Added button debounce
 * Added RetroZord's negCon mode (RZordPsWheel)
 * Added mouse mode
 * Added oled display
 * Added a hidden 2p pong game (by wotblitza) (uses Adafruit SSD1306 and GFX libs)
 * 
 * Mode selection during boot by holding buttons:
 * [Mode] high and [Config] high: MiSTer-S1 Spinner (2x) (falls to DEVICEMODE_DEFAULT)
 * [Mode] high and [Config] low: NeGcon (2x)
 * [Mode] low and [Config] high: Mouse (2x) as X
 * [Mode] low and [Config] low: Mouse (1x), paddle 1 as X, paddle 2 as Y
 * 
 * Press and release [Config] a bunch of times durig boot to access the pong game :)
 * 
 * Changing [Mode] button state during runtime will trigger a hardware reset (after 1 second).
 * Can be used to change the output mode without requiring to disconnect/connect the usb cable.
 * 
 * Pressing [Config] mode during runtime will show [CONFIG] on the oled display.
 * While in this mode, main button press on each pad will change mouse multiplier value.
 * Secondary button will toggle mouse value inversion.
 * Press [Config] again to exit config mode.
 * 
 * Inverted mouse was added to play Tempest on MiSTer's psx core.
 * As the game uses both X and Y axis, the two encoders can be used to outbut as single mouse.
 * I recommend to set multiplier at 10 for this game
 */

// SoftWire config
//also on defined on:
//Adafruit_SSD1306.h
//Adafruit_I2CDevice.h
//Adafruit_GrayOLED.h
//Adafruit_I2CDevice.h

#define SCL_PIN 4// 8
#define SCL_PORT PORTB
#define SDA_PIN 5// 9
#define SDA_PORT PORTB

#define I2C_FASTMODE 1
#define I2C_TIMEOUT 1000
#define I2C_PULLUP 1

//--------------

#include "src/SoftWire/SoftWire.h" //https://github.com/felias-fogg/SoftI2CMaster
#include "src/SSD1306Ascii/SSD1306Ascii.h"
#include "src/SSD1306Ascii/SSD1306AsciiWire.h"
#include "src/DigitalIO/DigitalIO.h"
#include "ZordButton.h"
#include "src/ArduinoJoystickLibrary/Joystick.h"
#include "src/ArduinoJoystickLibrary/MiSTerS1Spinner.h"
#include "src/ArduinoJoystickLibrary/Joy1.h"
#include "src/ArduinoJoystickLibrary/MouseRelative1.h"
#include <avr/wdt.h>

// SPINNER:
// Any spinner should work. Original A2600 driving controller has very low resolution, so it's better to upgrade it with some 3rd party spinner
// component with at least 80 ppr. Spinner components with clicking mechanism is not recommended as it won't be smooth.

// Joystick Port 1
// Arduino Pro Micro     Driving #1
// ---------------------------------
// TXO(1)  PD3           EncA
// RXI(0)  PD2           EncB
// 6       PD7           button 1
// 14      PB3           button 2
// VCC                   VCC
// GND                   GND
//
// Joystick Port 2
// Arduino Pro Micro     Driving #2
// ---------------------------------
// 2    PD1              EncA
// 7    PE6              EncB
// 15   PB1              button 1
// 16   PB2              button 2
// VCC                   VCC
// GND                   GND
//
// Mode change button
// Arduino Pro Micro     Button or Switch
// ---------------------------------
// 3       PD0           Mode Change
// GND                   GND
//
// Config button
// Arduino Pro Micro     Button or Switch
// ---------------------------------
// 10       PB6          Enter/Exit config
// GND                   GND
//
// Display i2c
// Arduino Pro Micro     Display
// ---------------------------------
// 8       PB4           SCL
// 9       PB5           SDA
// VCC                   VCC
// GND                   GND
//
// Note: spinners pins must support interrupts!
//

///////////////// Customizable settings /////////////////////////

// Spinner pulses per revolution
// For arduino shield spinner: 20
#define SPINNER_PPR 20

// Comment it to disable paddle emulation by spinner
#define PADDLE_EMU

// Optional parameter. Leave it commented out.
//#define SPINNER_SENSITIVITY 1

// Set it to 1 if you want only single input device.
#define DEV_NUM 2


#define DEBOUNCETIME 8 //millis

/////////////////////////////////////////////////////////////////

// pins map
#define ENC_P1_0 1
#define ENC_P1_1 0
#define BTN_P1_0 6
#define BTN_P1_1 14

#define ENC_P2_0 2
#define ENC_P2_1 7
#define BTN_P2_0 15
#define BTN_P2_1 16

#define BTN_MODE_CHANGE 3
#define BTN_CONFIG 10

const uint8_t encpin[2][2] = {{ENC_P1_0,ENC_P1_1},{ENC_P2_0,ENC_P2_1}}; // rotary encoder
//const int8_t dbtnpin[2]   = {BTN_P1_0,BTN_P2_0};        // spinner button (primary button)
//const int8_t nbtnpin[2]   = {BTN_P1_1,BTN_P2_1};        // negcon's second button

DigitalPin<ENC_P1_0> encoderP1_0;
DigitalPin<ENC_P1_1> encoderP1_1;
//DigitalPin<BTN_P1_0> btnP1_0;
//DigitalPin<BTN_P1_1> btnP1_1;

DigitalPin<ENC_P2_0> encoderP2_0;
DigitalPin<ENC_P2_1> encoderP2_1;
//DigitalPin<BTN_P2_0> btnP2_0;
//DigitalPin<BTN_P2_1> btnP2_1;

uint8_t defaultMultiplier = 1;
uint8_t multiplier[] = {defaultMultiplier, defaultMultiplier};//multiplier value
const uint8_t buttonCount = 6;
const uint8_t buttonPins[buttonCount] = {BTN_P1_0, BTN_P2_0, BTN_P1_1, BTN_P2_1, BTN_CONFIG, BTN_MODE_CHANGE};
ArcadePad* btnDebounce;


////////////////////////////////////////////////////////

#ifndef SPINNER_SENSITIVITY
  #if SPINNER_PPR < 50
    #define SPINNER_SENSITIVITY 1
  #else
    #define SPINNER_SENSITIVITY 2
  #endif
#endif


bool mouseInverse = false;
enum DeviceEnum {
  DEVICEMODE_DEFAULT = 0,
  DEVICEMODE_NEGCON,
  DEVICEMODE_MOUSE,
  DEVICEMODE_MOUSE_XY //both spinners controls a single mouse
};

DeviceEnum currentMode = DEVICEMODE_DEFAULT;

//uint8_t btnModeInitialState = LOW;

//Display config
#define I2C_DISPLAY_ADDRESS 0x3C // 0X3C+SA0 - 0x3C or 0x3D
SSD1306AsciiWire display;

Joystick_* Gamepad[DEV_NUM];

uint16_t drvpos[2];
//int16_t drvX[2] = {0,0};

#define SP_MAX ((SPINNER_PPR*4*270UL)/360)
const uint16_t sp_max = SP_MAX;
int32_t sp_clamp[2] = {SP_MAX/2,SP_MAX/2};

JogconReport1 rep;
const int16_t sp_step = (SPINNER_PPR*10)/(20*SPINNER_SENSITIVITY);

#include "PongGame.h"
bool isPongMode = false;

void configure()
{
  //Check mode button

  //if(digitalRead(BTN_P1_0) == LOW || digitalRead(BTN_P2_0) == LOW) {//button 0
  //if(digitalRead(BTN_CONFIG) == LOW) {//button mode
  if(btnDebounce->state(buttonCount-1) == LOW) { //MOUSE MODES
    currentMode = (btnDebounce->state(buttonCount-2) == HIGH) ? DEVICEMODE_MOUSE : DEVICEMODE_MOUSE_XY;
  //} else if(digitalRead(BTN_MODE_CHANGE) == LOW) {
  } else if(btnDebounce->state(buttonCount-2) == LOW) { //JOYSTICK MODES
    currentMode = DEVICEMODE_NEGCON;
  } //else default mode
  //else {
  //  currentMode = DEVICEMODE_DEFAULT;
  //}
}

void mode_change_isr()
{
  //detachInterrupt(digitalPinToInterrupt(BTN_MODE_CHANGE));
  for(int idx=0; idx<DEV_NUM; idx++){
    detachInterrupt(digitalPinToInterrupt(encpin[idx][0]));
    detachInterrupt(digitalPinToInterrupt(encpin[idx][1]));
  }

  display.clear();
  display.setCol(10);
  display.println(F("REBOOTING"));
  display.ssd1306WriteCmd(SSD1306_DISPLAYON);

  //reset and send controllers state
  for(int idx=0; idx<DEV_NUM; idx++)
  {
    Gamepad[idx]->resetState();
    Gamepad[idx]->sendState();
    delay(10);
  }

  //reset the arduino
  //wdt_enable(WDTO_1S);
  wdt_enable(WDTO_500MS);
  while(1);
}

void drv_proc(const int8_t idx)
{
  static int8_t prev[2];
  int8_t a;
  int8_t b;
  if(idx == 0)
  {
    a = encoderP1_0;
    b = encoderP1_1;
  }
  else
  {
    a = encoderP2_0;
    b = encoderP2_1;
  }

  int8_t spval = (b << 1) | (b^a);
  int8_t diff = (prev[idx] - spval)&3;

  if(diff == 1) 
  {
    drvpos[idx] += 10;
    if(sp_clamp[idx] < sp_max) sp_clamp[idx]++;
  }
  if(diff == 3) 
  {
    drvpos[idx] -= 10;
    if(sp_clamp[idx] > 0) sp_clamp[idx]--;
  }

  prev[idx] = spval;
}

void drv0_isr()
{
  drv_proc(0);
}

void drv1_isr()
{
  drv_proc(1);
}


void showRetroZord(const bool clearRow)
{
  display.setRow(0);
  if(clearRow)
  {
    display.setCol(0);
    display.clearToEOL();
  }
  display.setCol(10);
  display.println(F("RetroZord"));
}

void showConfig(const bool clearRow)
{
  display.setRow(0);
  if(clearRow)
  {
    display.setCol(0);
    display.clearToEOL();
  }
  display.setCol(28);
  display.println(F("Config"));
  
}

void showCurrentMode(const bool clearRow)
{
  display.setRow(3);
  if(clearRow)
  {
    display.setCol(0);
    display.clearToEOL();
  }
  switch(currentMode)
  {
  case DEVICEMODE_DEFAULT:
    display.setCol(12);
    display.println(F("MiSTer-S1"));
    break;
  case DEVICEMODE_NEGCON:
    display.setCol(28);
    display.println(F("NeGcon"));
    break;
  case DEVICEMODE_MOUSE:
    if(mouseInverse) {
      display.setCol(5);
      display.println(F("Mouse inv."));
    } else {
      display.setCol(35);
      display.println(F("Mouse"));
    }
    break;
  case DEVICEMODE_MOUSE_XY:
    if(mouseInverse) {
      display.setCol(0);
      display.println(F("MouseXY inv."));
    } else {
      display.setCol(20);
      display.println(F("MouseXY"));
    }
    break;
  }
}

void showMultiplier(const bool clearRow)
{
  display.setRow(6);
  if(clearRow) {
    display.setCol(0);
    display.clearToEOL();
  }

  if(currentMode == DEVICEMODE_NEGCON)//negcon
    return;

  for(uint8_t i = 0; i < 2; i++)
  {
    if(i == 0)
      display.setCol(10);
    else
      display.setCol(multiplier[1] > 9 ? 82 : 94);
    display.print(multiplier[i]);display.print(F("X"));
  }
}


void setup()
{
  //Start debounce lib. This will also put pins as INPUT_PULLUP
  btnDebounce = new ArcadePad(buttonCount, buttonPins, DEBOUNCETIME);

  btnDebounce->begin();
  
  //do some initial debouncing
  for(uint8_t i=0; i<10; i++) {
    btnDebounce->update();
    delay(2);
  }

  //pinMode(BTN_MODE_CHANGE, INPUT_PULLUP);
  //pinMode(BTN_CONFIG, INPUT_PULLUP);
  //pinMode(BTN_P1_0, INPUT_PULLUP);
  //pinMode(BTN_P1_1, INPUT_PULLUP);
  //pinMode(BTN_P2_0, INPUT_PULLUP);
  //pinMode(BTN_P2_1, INPUT_PULLUP);
  //delay(20);
  
  //configure output mode
  configure();
    
  //Create output controllers
  for(int idx=0; idx<DEV_NUM; idx++)
  {
    if(currentMode == DEVICEMODE_NEGCON) {
      Gamepad[idx] = new Joy1_("RZordPsWheel", JOYSTICK_DEFAULT_REPORT_ID + idx, JOYSTICK_TYPE_GAMEPAD, DEV_NUM,
        true,//includeXAxis,
        false,//includeYAxis,
        true,//includeZAxis,
        false,//includeRxAxis,
        false,//includeRyAxis,
        true,//includeThrottle,
        true,//includeBrake,
        true);//includeSteering
    } else if(currentMode == DEVICEMODE_MOUSE || currentMode == DEVICEMODE_MOUSE_XY) {
      Gamepad[idx] = new MouseRel1_("RZordSpinnerMouse", JOYSTICK_DEFAULT_REPORT_ID + idx, DEV_NUM);
    } else {
      Gamepad[idx] = new MisterSpinner1_("MiSTer-S1 Spinner", DEV_NUM);
    }
  }

  //Initialize default state on controllers and send first state
  for(int idx=0; idx<DEV_NUM; idx++)
  {
    Gamepad[idx]->resetState();

    if(currentMode == DEVICEMODE_NEGCON)
    {
      const byte ANALOG_MAX_VALUE = 255U;
      const byte ANALOG_IDLE_VALUE = 128U;
      ((Joy1_*)Gamepad[idx])->setAnalog0(ANALOG_IDLE_VALUE); //x
      ((Joy1_*)Gamepad[idx])->setAnalog1(ANALOG_MAX_VALUE); //z
      ((Joy1_*)Gamepad[idx])->setAnalog2(ANALOG_MAX_VALUE); //I throttle
      ((Joy1_*)Gamepad[idx])->setAnalog3(ANALOG_MAX_VALUE); //II brake
      ((Joy1_*)Gamepad[idx])->setAnalog4(ANALOG_IDLE_VALUE); //paddle
    }

    Gamepad[idx]->sendState();
    
  }

  Wire.begin();
  Wire.setClock(400000L);
  
  display.begin(&Adafruit128x64, I2C_DISPLAY_ADDRESS);
  display.setFont(System5x7);
  display.clear();
  display.setContrast(1);
  display.set2X();

  showRetroZord(false);
  showCurrentMode(false);
  //showMultiplier(false);

  display.setRow(6);
  //display.setCol(2);
  //display.println(F("<STARTING>"));
  display.setCol(0);
  display.println(F("PLEASE WAIT"));
  //display.println(F("<Starting>"));

  //update a few times
  /*for(uint8_t i=0; i<10; i++) {
    delay(1);
    btnDebounce->update();
  }*/

  //delay(3000);
  const unsigned long start = micros();
  uint8_t miniGameCount = 0;
  while((micros()-start) < 2500000UL){//2.5 seconds
    btnDebounce->update();
    if(btnDebounce->fell(buttonCount-1) || btnDebounce->rose(buttonCount-1))
      mode_change_isr();
    if(btnDebounce->fell(buttonCount-2))
      miniGameCount++;
  }
  isPongMode = miniGameCount > 1;

  //Configure rotary encoder pins and setup interrupts
  for(int idx=0; idx<DEV_NUM; idx++)
  {
    pinMode(encpin[idx][0], INPUT_PULLUP);
    pinMode(encpin[idx][1], INPUT_PULLUP);
    //pinMode(dbtnpin[idx],   INPUT_PULLUP);
    //pinMode(nbtnpin[idx],   INPUT_PULLUP);

    drv_proc(idx);
    drvpos[idx] = 0;
    attachInterrupt(digitalPinToInterrupt(encpin[idx][0]), idx ? drv1_isr : drv0_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(encpin[idx][1]), idx ? drv1_isr : drv0_isr, CHANGE);
  }
  //attachInterrupt(digitalPinToInterrupt(BTN_MODE_CHANGE), mode_change_isr, CHANGE);

  //Serial.begin(115200);
  if(isPongMode) {
    pongSetup();
    return;
  }

  display.ssd1306WriteCmd(SSD1306_DISPLAYOFF);
  showConfig(true);
  showMultiplier(true);
}


void loop()
{
  static bool isConfigMode = false;
  //static unsigned long multiplierModeLastTime[] = {0, 0};//multiplier mode button holding time
  //static bool btnMultiplierChange[] = {false, false};//holds button state of press/release

  // LEDs off
  TXLED1; //RXLED1;

  btnDebounce->update();//debounce buttons
  
  if(btnDebounce->fell(buttonCount-1) || btnDebounce->rose(buttonCount-1))
  {
    mode_change_isr();
  }

  if(btnDebounce->fell(buttonCount-2))//Config button
  {
    isConfigMode = !isConfigMode;
    display.ssd1306WriteCmd(isConfigMode ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);
  }

  for(uint8_t idx=0; idx<DEV_NUM; idx++)
  {
    const uint8_t btnIdx0 = (idx | 0x0);
    const uint8_t btnIdx1 = (idx | 0x2);

    //if(btnDebounce->heldUntil(btnIdx1, 3000))//button 1 is held activated for 3 seconds
    //if(btnDebounce->held(btnIdx1) && btnDebounce->duration(btnIdx1) > 3000)//button 1 is held activated for 3 seconds
    if(isConfigMode)
    {
      if(btnDebounce->fell(btnIdx0))//button 0 just activated
      {
        multiplier[idx] += 3;
        if(multiplier[idx] > 10) //reached limit. loop to initial value.
          multiplier[idx] = 1;
        //update the display
        showMultiplier(true);
      }
      if(btnDebounce->fell(btnIdx1) && (currentMode == DEVICEMODE_MOUSE || currentMode == DEVICEMODE_MOUSE_XY))//button 1 just activated
      {
        mouseInverse = !mouseInverse;
        showCurrentMode(true);
      }
    } //end if(isConfigMode)
    
    rep.buttons = 0;
    rep.paddle = 0;
    rep.spinner = 0;

    // spinner button
    //if(btnDebounce->held(btnIdx0))
    if(btnDebounce->state(btnIdx0) == LOW)
    {
      rep.b0 = 1;
    }
    
    // negcon button
    if(btnDebounce->state(btnIdx1) == LOW)
    {
      rep.b1 = 1;
    }

#ifdef PADDLE_EMU
    rep.paddle = ((sp_clamp[idx]*255)/sp_max);
#endif

    // spinner rotation
    static uint16_t prev[2] = {0,0};
    int16_t val = ((int16_t)(drvpos[idx] - prev[idx]))/sp_step;
    if(val>127) val = 127; else if(val<-127) val = -127;

    // apply multiplier and limit range
    int16_t mval = val * multiplier[idx];
    if(mval>127) mval = 127; else if(mval<-127) mval = -127;
    rep.spinner = mval;
    prev[idx] += val*sp_step;

    if(currentMode == DEVICEMODE_NEGCON)
    {
      ((Joy1_*)Gamepad[idx])->setButton(0, rep.b1); //Neg A | Paddle B
      ((Joy1_*)Gamepad[idx])->setButton(3, rep.b0); //Neg Start | Paddle A
      ((Joy1_*)Gamepad[idx])->setAnalog0(rep.paddle); //x
      ((Joy1_*)Gamepad[idx])->setAnalog4(rep.paddle); //paddle
    } else if(currentMode == DEVICEMODE_MOUSE) {
      ((MouseRel1_*)Gamepad[idx])->setButton(0, rep.b0); //mouse left
      ((MouseRel1_*)Gamepad[idx])->setButton(1, rep.b1); //mouse right
      ((MouseRel1_*)Gamepad[idx])->setXAxis(mouseInverse ? rep.spinner*-1 : rep.spinner);
    } else if(currentMode == DEVICEMODE_MOUSE_XY) {
      ((MouseRel1_*)Gamepad[0])->setButton(0, rep.b0); //mouse left
      ((MouseRel1_*)Gamepad[0])->setButton(1, rep.b1); //mouse right
      if(idx == 0) // X axis
        ((MouseRel1_*)Gamepad[0])->setXAxis(mouseInverse ? rep.spinner*-1 : rep.spinner);
      else // Y axis
        ((MouseRel1_*)Gamepad[0])->setYAxis(mouseInverse ? rep.spinner*-1 : rep.spinner);
    } else {
      ((MisterSpinner1_*)Gamepad[idx])->setButton(0, rep.b0);
      ((MisterSpinner1_*)Gamepad[idx])->setSpinner(rep.spinner);
      ((MisterSpinner1_*)Gamepad[idx])->setPaddle(rep.paddle);
    }

    // Only report controller state if it has changed
    if(Gamepad[idx]->stateChanged()) {
      Gamepad[idx]->sendState();

      if(isPongMode) {
        //update pong spinner
        if(idx == 0){
          if(rep.spinner != 0)
            spinner_left += rep.spinner;
        } else {
          if(rep.spinner != 0)
            spinner_right += rep.spinner;
        }
      }//end if isPongMode
    }//end if stateChanged

  }//end for


  if(isPongMode)
    pongLoop();
  
}
