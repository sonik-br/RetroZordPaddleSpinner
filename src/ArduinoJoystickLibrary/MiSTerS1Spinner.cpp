#include "MiSTerS1Spinner.h"

static const uint8_t _hidReportDescriptor[] PROGMEM = {
  0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,                       // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,                       // COLLECTION (Application)
    0xa1, 0x00,                     // COLLECTION (Physical)
    
      0x05, 0x09,                   // USAGE_PAGE (Button)
      0x19, 0x01,                   // USAGE_MINIMUM (Button 1)
      0x29, 0x01,                   // USAGE_MAXIMUM (Button 1)
      0x15, 0x00,                   // LOGICAL_MINIMUM (0)
      0x25, 0x01,                   // LOGICAL_MAXIMUM (1)
      0x95, 0x08,                   // REPORT_COUNT (8)
      0x75, 0x01,                   // REPORT_SIZE (1)
      0x81, 0x02,                   // INPUT (Data,Var,Abs)
    
      0x05, 0x01,                   // USAGE_PAGE (Generic Desktop)

      0x09, 0x37,                   // USAGE (Dial)
      0x15, 0x80,                   // LOGICAL_MINIMUM (-128)
      0x25, 0x7F,                   // LOGICAL_MAXIMUM (127)
      0x95, 0x01,                   // REPORT_COUNT (1)
      0x75, 0x08,                   // REPORT_SIZE (8)
      0x81, 0x06,                   // INPUT (Data,Var,Rel)

      0x09, 0x38,                   // USAGE (Wheel)
      0x15, 0x00,                   // LOGICAL_MINIMUM (0)
      0x26, 0xFF, 0x00,             // LOGICAL_MAXIMUM (255)
      0x95, 0x01,                   // REPORT_COUNT (1)
      0x75, 0x08,                   // REPORT_SIZE (8)
      0x81, 0x02,                   // INPUT (Data,Var,Abs)

    0xc0,                           // END_COLLECTION
  0xc0,                             // END_COLLECTION 
};

MisterSpinner1_::MisterSpinner1_(const char* serial, const uint8_t totalControllers) :
  Joystick_(serial, totalControllers)
{

  _hidReportSize = sizeof(JogconReport1);
  
  // Register HID Report Description
  DynamicHIDSubDescriptor* node = new DynamicHIDSubDescriptor(_hidReportDescriptor, sizeof(_hidReportDescriptor), true);
  _endpointPool[_endpointIndex]->AppendDescriptor(node);
}

void MisterSpinner1_::setButton(const uint8_t index, const bool value) {
  if(bitRead(_GamepadReport.buttons, index) != value)
    _stateChanged = true;
  if(value)
    _GamepadReport.buttons |= 1 << index;
  else
    _GamepadReport.buttons &= ~(1 << index);
}

void MisterSpinner1_::setSpinner(const int8_t value) {
  if(_GamepadReport.spinner != value)
    _stateChanged = true;
  _GamepadReport.spinner = value;
}

void MisterSpinner1_::setPaddle(const int8_t value) {
  if(_GamepadReport.paddle != value)
    _stateChanged = true;
  _GamepadReport.paddle = value;
}

void MisterSpinner1_::sendState() {
  _endpointPool[_endpointIndex]->SendReport(&_GamepadReport, _hidReportSize);
  _stateChanged = false;
}
  
void MisterSpinner1_::resetState() {
  _GamepadReport.buttons = 0;
  _GamepadReport.spinner = 0;
  _GamepadReport.paddle = 0;
  _stateChanged = true;
}
