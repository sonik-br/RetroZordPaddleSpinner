#include "MouseRelative1.h"

MouseRel1_::MouseRel1_(const char* serial, const uint8_t reportId, const uint8_t totalControllers) :
  Joystick_(serial, totalControllers)
{
  _GamepadReport.id = reportId;

  // Build Joystick HID Report Description
  
  uint8_t tempHidReportDescriptor[50];
  uint8_t hidReportDescriptorSize = 0;

  _hidReportSize = sizeof(MouseRelReport1);


  // USAGE_PAGE (Generic Desktop)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

  // USAGE (Mouse)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;

  // COLLECTION (Application)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0xa1;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

  // REPORT_ID (Default: 3)
  if (_useComposite) {
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x85;
    tempHidReportDescriptor[hidReportDescriptorSize++] = reportId;
  } else {
    _hidReportSize -=1;
  }

  //Buttons

  // USAGE_PAGE (Button)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;

  // USAGE_MINIMUM (Button 1)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x19;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

  // USAGE_MAXIMUM (Button 8)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x29;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x08;

  // LOGICAL_MINIMUM (0)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

  // LOGICAL_MAXIMUM (1)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

  // REPORT_SIZE (1)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

  // REPORT_COUNT (8)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x08;

  // INPUT (Data,Var,Abs)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;

  // End Buttons

  // USAGE_PAGE (Generic Desktop)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

  // USAGE (Pointer)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

  // COLLECTION (Physical)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0xA1;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

  // USAGE (X)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x30;

  // USAGE (Y)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x31;

  // LOGICAL_MINIMUM (-128)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x80;

  //LOGICAL_MAXIMUM (127)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x7F;

  // REPORT_SIZE (8)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x08;

  // REPORT_COUNT (2)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;
          
  // INPUT Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0x06;
  
  // END_COLLECTION (Physical)
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0;

  // END_COLLECTION
  tempHidReportDescriptor[hidReportDescriptorSize++] = 0xc0;


  // Create a copy of the HID Report Descriptor template that is just the right size
  uint8_t* customHidReportDescriptor = new uint8_t[hidReportDescriptorSize];
  memcpy(customHidReportDescriptor, tempHidReportDescriptor, hidReportDescriptorSize);

  // Register HID Report Description
  DynamicHIDSubDescriptor* node = new DynamicHIDSubDescriptor(customHidReportDescriptor, hidReportDescriptorSize, false);
  _endpointPool[_endpointIndex]->AppendDescriptor(node);
}

void MouseRel1_::setButton(const uint8_t index, const bool value) {
  if(bitRead(_GamepadReport.buttons, index) != value)
    _stateChanged = true;
  if(value)
    _GamepadReport.buttons |= 1 << index;
  else
    _GamepadReport.buttons &= ~(1 << index);
}

void MouseRel1_::setXAxis(const int8_t value) {
  if (_GamepadReport.x != value)
    _stateChanged = true;
  _GamepadReport.x = value;
}
void MouseRel1_::setYAxis(const int8_t value) {
  if (_GamepadReport.y != value)
    _stateChanged = true;
  _GamepadReport.y = value;
}

void MouseRel1_::sendState() {
  if (_useComposite)
    _endpointPool[_endpointIndex]->SendReport(&_GamepadReport, _hidReportSize);
  else
    _endpointPool[_endpointIndex]->SendReport((uint8_t*)&_GamepadReport+1, _hidReportSize);
  _stateChanged = false;
}
  
void MouseRel1_::resetState() {
  _GamepadReport.buttons = 0;
  _GamepadReport.x = 0;
  _GamepadReport.y = 0;
  _stateChanged = true;
}
