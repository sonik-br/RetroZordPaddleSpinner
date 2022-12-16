/*
  Derived class
  16 buttons
  Hat
  If joystick: analog relative spinner and paddle
*/

#ifndef MISTERS1SPINNER_h
#define MISTERS1SPINNER_h

#include "Joystick.h"

typedef struct {
  union 
  {
    struct {
      bool  b0:  1;
      bool  b1:  1;
      bool  b2:  1;
      bool  b3:  1;
      bool  b4:  1;
      bool  b5:  1;
      bool  b6:  1;
      bool  b7:  1; 
    };
    uint8_t buttons;
  };

  int8_t spinner;
  int8_t paddle;
} JogconReport1;

class MisterSpinner1_ : public Joystick_
{
  private:
    JogconReport1 _GamepadReport;
    uint8_t _hidReportSize;

  public:
    MisterSpinner1_(const char* serial, const uint8_t totalControllers);
    void setButton(const uint8_t index, const bool value);
    void setSpinner(const int8_t value);
    void setPaddle(const int8_t value);
    void sendState();
    void resetState();
};

#endif // MISTERS1SPINNER_h
