#include "Adafruit_TinyUSB.h"
#define GAMEPAD_COUNT 2
#if CFG_TUD_HID < GAMEPAD_COUNT
  #error "Requires 2 HID instances support. See https://github.com/adafruit/Adafruit_TinyUSB_Arduino/commit/b75604f794acdf88daad310dd75d3a0724129056"
#endif 

#define REPORT_ID_GAMEPAD 1
#define REPORT_ID_KEYBOARD 2
// NB NB!!! Select "Adafruit TinyUSB" for USB stack


// HID report descriptor using TinyUSB's template
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_ID_GAMEPAD)),
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD))
};

// USB HID object. For ESP32 these values cannot be changed after this declaration
// desc report, desc len, protocol, interval, use out endpoint
Adafruit_USBD_HID usb_hid[GAMEPAD_COUNT] {
  Adafruit_USBD_HID(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false),
  Adafruit_USBD_HID(desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false),
};

// Report payload defined in src/class/hid/hid.h
// - For Gamepad Button Bit Mask see  hid_gamepad_button_bm_t
// - For Gamepad Hat    Bit Mask see  hid_gamepad_hat_t
hid_gamepad_report_t   gamepadReports[2]; // Two gamepad descriptors
// keys being hit
#define MAX_KEYS 6
uint8_t pressedKeys[2][MAX_KEYS];

void setup() {
  // Manual begin() is required on core without built-in support e.g. mbed rp2040
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  usb_hid[0].setStringDescriptor("Kikoo");
  usb_hid[0].begin();
  usb_hid[0].setStringDescriptor("Lol");
  usb_hid[1].begin();

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }
  
  Serial.println("Adafruit TinyUSB HID multi-gamepad example");
}

uint8_t gamepadIndex = 0;

bool isChopping(uint8_t controllerIndex) {
  return random() % 2 == 0;
}

bool wasChopping[2] = { false, false };
int occurrence = 0;
void loop() {
  #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif

  // not enumerated()/mounted() yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }

  Adafruit_USBD_HID controller = usb_hid[gamepadIndex];
  if ( !controller.ready() ) return;
  
  // joueur 

  bool chopping = isChopping(gamepadIndex);
  bool changed = (chopping != wasChopping[gamepadIndex]);
  if (changed) {
    hid_gamepad_report_t gamepadReport = gamepadReports[gamepadIndex];
    uint8_t *keys = pressedKeys[gamepadIndex];
    if (chopping) {
      gamepadReport.buttons = gamepadReport.buttons | GAMEPAD_BUTTON_TL;
      keys[0] = HID_KEY_A + occurrence++;
    } else if (!chopping && (gamepadReport.buttons & GAMEPAD_BUTTON_WEST) != 0) {
      gamepadReport.buttons = gamepadReport.buttons ^ GAMEPAD_BUTTON_WEST;
      keys[0] = 0;
      changed = true;
    }
    wasChopping[gamepadIndex] = chopping;
    controller.keyboardReport(REPORT_ID_KEYBOARD, 0, keys);
    controller.sendReport(REPORT_ID_GAMEPAD, &gamepadReport, sizeof(gamepadReport));
  }
  delay(1000);
  //Serial.print("Testing gamepad nr: ");
  //Serial.println(gamepadIndex);

  // Reset buttons
  //Serial.println("No pressing buttons");
  //gp[gp_i].x       = 0;
  //gp[gp_i].y       = 0;
  //gp[gp_i].z       = 0;
  //gp[gp_i].rz      = 0;
  //gp[gp_i].rx      = 0;
  //gp[gp_i].ry      = 0;
  //gamepadReport.hat     = 0;
  //gamepadReport.buttons = 0;

  //controller.sendReport(REPORT_ID_GAMEPAD, &gamepadReport, sizeof(gamepadReport));
  //delay(2000);

  // Random touch
  //Serial.println("Random touch");
  //gp[gp_i].x       = random(-127, 128);
  //gp[gp_i].y       = random(-127, 128);
  //gp[gp_i].z       = random(-127, 128);
  //gp[gp_i].rz      = random(-127, 128);
  //gp[gp_i].rx      = random(-127, 128);
  //gp[gp_i].ry      = random(-127, 128);
  //gamepadReport.hat     = random(0,      9);
  //gamepadReport.buttons = random(0, 0xffff) & (GAMEPAD_BUTTON_A | GAMEPAD_BUTTON_B | GAMEPAD_BUTTON_X | GAMEPAD_BUTTON_Y);
  //controller.sendReport(REPORT_ID_GAMEPAD, &gamepadReport, sizeof(gamepadReport));
  //delay(2000);

  // select the other gamepad
  gamepadIndex = (gamepadIndex + 1) % GAMEPAD_COUNT;
}
