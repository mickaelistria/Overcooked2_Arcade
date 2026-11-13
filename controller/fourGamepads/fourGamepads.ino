#include "Adafruit_TinyUSB.h"
#define PLAYER_COUNT 4
#define GAMEPAD_COUNT 2 // 2 split gamepads
#if CFG_TUD_HID < GAMEPAD_COUNT
	#error "Requires 2 HID instances support. See https://github.com/adafruit/Adafruit_TinyUSB_Arduino/commit/b75604f794acdf88daad310dd75d3a0724129056"
#endif 

#define REPORT_ID_GAMEPAD 1
#define REPORT_ID_KEYBOARD 2
// NB NB!!! Select "Adafruit TinyUSB" for USB stack

uint8_t const desc_hid_report[] = {
	TUD_HID_REPORT_DESC_GAMEPAD(HID_REPORT_ID(REPORT_ID_GAMEPAD)),
	TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KEYBOARD))
};

class PlayerState {
private:
	bool chopping = false,
		washing = false,
		throwing = false,
		extinguishing = false,
		handlingOrReleasing = false,
		movingUp = false,
		movingDown = false,
		movingLeft = false,
		movingRight = false;
	const pin_size_t pinUp, pinDown, pinLeft, pinRight,
		pinChopping, pinWashing, pinExtinguishing, pinThrowing, pinHandlingOrReleasing;
	PinStatus previousChoppingPinState = HIGH, previousWashingPinState = HIGH;
	unsigned long chopEvents[3] = {0, 0, 0};
	unsigned long washEvents[3] = {0, 0, 0};
	bool hasChanges = false;
	bool threeHits(unsigned long events[]) {
		unsigned long min = std::min(events[0], std::min(events[1], events[2]));
		unsigned long max = std::max(events[0], std::max(events[1], events[2]));
		Serial.print(events[0]); Serial.print(" / "); Serial.print(events[1]); Serial.print(" / "); Serial.print(events[2]); Serial.println("");
		return min > 0 && max > 0 // three hits
			&& max - min < 1500 // in less than 1500 milliseconds
			&& millis() - max < 200; // less than 200ms ago;
	}
	void insertEvent(unsigned long events[]) {
		int indexMin = 0;
		unsigned long currentMin = events[0];
		for (int i = 0; i < 3; i++) {
			if (events[i] < currentMin) {
				currentMin = events[i];
				indexMin = i;
			}
		}
		events[indexMin] = millis();
	}
	unsigned long lastEvent(unsigned long events[]) {
		return max(events[0], max(events[1], events[2]));
	}
public:
	PlayerState(pin_size_t _pinUp, pin_size_t _pinDown, pin_size_t _pinLeft, pin_size_t _pinRight, pin_size_t _pinChopping, pin_size_t _pinWashing, pin_size_t _pinThrowing, pin_size_t _pinExtinguishing, pin_size_t _pinHandlingOrReleasing) :
		pinUp(_pinUp), pinDown(_pinDown), pinLeft(_pinLeft), pinRight(_pinRight),
		pinChopping{_pinChopping}, pinWashing(_pinWashing), pinExtinguishing(_pinExtinguishing), pinThrowing(_pinThrowing),
		pinHandlingOrReleasing(_pinHandlingOrReleasing), hasChanges(true) {
		pinMode(pinUp, INPUT_PULLUP);
		pinMode(pinDown, INPUT_PULLUP);
		pinMode(pinLeft, INPUT_PULLUP);
		pinMode(pinRight, INPUT_PULLUP);
		pinMode(pinChopping, INPUT_PULLUP);
		pinMode(pinWashing, INPUT_PULLUP);
		pinMode(pinThrowing, INPUT_PULLUP);
		pinMode(pinExtinguishing, INPUT_PULLUP);
		pinMode(pinHandlingOrReleasing, INPUT_PULLUP);
	}
	bool isChopping() { return chopping; }
	bool isWashing() { return washing; }
	bool isThrowing() { return throwing; }
	bool isExtinguishing() { return extinguishing; }
	bool isHandlingOrReleasing() { return handlingOrReleasing; }
	bool isMovingUp() { return movingUp; } // consider merging those 4 as direction
	bool isMovingDown() { return movingDown; }
	bool isMovingLeft() { return movingLeft; }
	bool isMovingRight() { return movingRight; }
	/**
	 * @return true if state changed upon update
	 */
	bool update() {
		hasChanges = false;
		// High speed chopping: ~6 hits per second, 1 hit every 150 ms
		if (digitalRead(pinChopping) == LOW) { // touch
			if (previousChoppingPinState == HIGH) { // new touch
				//Serial.println("New touch");
				if (millis() - lastEvent(chopEvents) > 150) { // after some delay (to ignore on touch contacts)
					insertEvent(chopEvents);
					if (!chopping && threeHits(chopEvents)) {
						chopping = true;
						//Serial.println("Chopping");
						hasChanges = true;
					}
				}
			}
			previousChoppingPinState = LOW;
		} else { // released, no touch
			/*if (previousChoppingPinState == LOW) {
				Serial.println("Release");
			}*/
			if (chopping && millis() - lastEvent(chopEvents) > 150) { // no touch for last 50ms
				chopping = false;
				hasChanges = true;
				
			}
			previousChoppingPinState = HIGH;
		}
		if (digitalRead(pinWashing) == LOW) { // touch
			if (previousWashingPinState == HIGH) { // new touch
				if (millis() - lastEvent(washEvents) > 150) { // after some delay (to ignore on touch contacts)
					insertEvent(washEvents);
					if (!washing && threeHits(washEvents)) {
						washing = true;
						hasChanges = true;
					}
				}
			}
			previousWashingPinState = LOW;
		} else {
			if (washing && millis() - lastEvent(washEvents) > 150) { // no touch for last 50ms
				washing = false;
				hasChanges = true;
				
			}
			previousWashingPinState = HIGH;
		}
		if (digitalRead(pinExtinguishing) != (extinguishing ? LOW : HIGH)) {
			extinguishing = !extinguishing;
			hasChanges = true;
		}
		if (digitalRead(pinThrowing) != (throwing ? LOW : HIGH)) {
			throwing = !throwing;
			hasChanges = true;
		}
		if (digitalRead(pinHandlingOrReleasing) != (handlingOrReleasing ? LOW : HIGH)) {
			handlingOrReleasing = !handlingOrReleasing;
			hasChanges = true;
		}
		if (digitalRead(pinLeft) != (movingLeft ? LOW : HIGH)) {
			movingLeft = !movingLeft;
			hasChanges = true;
		}
		if (digitalRead(pinRight) != (movingRight ? LOW : HIGH)) {
			movingRight = !movingRight;
			hasChanges = true;
		}
		if (digitalRead(pinUp) != (movingUp ? LOW : HIGH)) {
			movingUp = !movingUp;
			hasChanges = true;
		}
		if (digitalRead(pinDown) != (movingDown ? LOW : HIGH)) {
			movingDown = !movingDown;
			hasChanges = true;
		}
		return hasChanges;
	}
};

class PlayerControlMapping {
public:
	PlayerControlMapping(bool _isLeftJoystick, hid_gamepad_button_bm_t _actionButton, hid_gamepad_button_bm_t _handleReleaseButton, uint8_t _extraKeyChopping, uint8_t _extraKeyWashing, uint8_t _extraKeyExtinguishing, uint8_t _extraKeyThrowing) :
		isLeftJoystick(_isLeftJoystick),
		actionButton(_actionButton),
		handleOrReleaseButton(_handleReleaseButton),
		extraKeyChopping(_extraKeyChopping),
		extraKeyWashing(_extraKeyWashing),
		extraKeyExtinguishing(_extraKeyExtinguishing),
		extraKeyThrowing(_extraKeyThrowing) {
	}
	const bool isLeftJoystick;
	const hid_gamepad_button_bm_t actionButton,
		handleOrReleaseButton;
	const uint8_t extraKeyChopping,
		extraKeyExtinguishing,
		extraKeyThrowing,
		extraKeyWashing;
};

#define MAX_KEYS 6
class Controller {
private:
	inline static uint8_t counter = 1;
	Adafruit_USBD_HID controller;
	hid_gamepad_report_t gamepadReport;
	uint8_t pressedKeys[MAX_KEYS] = {0, 0, 0, 0, 0, 0};

	PlayerControlMapping *leftControlMapping, *rightControlMapping;
	PlayerState *leftState, *rightState;
	void sendReport() {
		if (controller.ready()) {
			Serial.print("key"); Serial.println(pressedKeys[0]);
			Serial.print("buttons"); Serial.println(gamepadReport.buttons);
			controller.keyboardReport(REPORT_ID_KEYBOARD, 0, pressedKeys);
			delay(10);
			controller.sendReport(REPORT_ID_GAMEPAD, &gamepadReport, sizeof(gamepadReport));
		}
	}
	void apply(PlayerControlMapping *mapping, PlayerState *state) {
		if (mapping == NULL || state == NULL) {
			return;
		}
		if (state->isChopping()) {
			pressButton(mapping->actionButton);
			if (mapping->extraKeyChopping != 0) {
				pressKey(mapping->extraKeyChopping);
			}
		}
		if (state->isWashing()) {
			pressButton(mapping->actionButton);
			if (mapping->extraKeyWashing != 0) {
				pressKey(mapping->extraKeyWashing);
			}
		}
		if (state->isExtinguishing()) {
			pressButton(mapping->actionButton);
			if (mapping->extraKeyExtinguishing != 0) {
				pressKey(mapping->extraKeyExtinguishing);
			}
		}
		if (state->isThrowing()) {
			pressButton(mapping->actionButton);
			if (mapping->extraKeyThrowing != 0) {
				pressKey(mapping->extraKeyThrowing);
			}
		}
		if (state->isHandlingOrReleasing()) {
			pressButton(mapping->handleOrReleaseButton);
		}
		if (mapping->isLeftJoystick) {
			setLeftJoystick(state->isMovingUp(), state->isMovingDown(), state->isMovingLeft(), state->isMovingRight());
		} else {
			setRightJoystick(state->isMovingUp(), state->isMovingDown(), state->isMovingLeft(), state->isMovingRight());
		}
	}
	void pressButton(hid_gamepad_button_bm_t button) {
		gamepadReport.buttons |= button;
	}
	void pressKey(uint8_t key) {
		for (int i = 0; i < MAX_KEYS; i++) {
			if (pressedKeys[i] == 0) {
				pressedKeys[i] = key;
				return;
			}
			if (pressedKeys[i] == key) {
				return;
			}
		}
	}
	void setLeftJoystick(bool up, bool down, bool left, bool right) {
		gamepadReport.y = (up ? 127 : (down ? -127 : 0));
		gamepadReport.x = (right ? 127 : (left ? -127 : 0));
	}
	void setRightJoystick(bool up, bool down, bool left, bool right) {
		gamepadReport.z = (up ? 127 : (down ? -127 : 0));
		gamepadReport.rz = (right ? 127 : (left ? -127 : 0));
	}
public:
	Controller(PlayerControlMapping *_leftMapping, PlayerState *_leftState, PlayerControlMapping *_rightMapping, PlayerState *_rightState) :
		controller{Adafruit_USBD_HID{desc_hid_report, sizeof(desc_hid_report), HID_ITF_PROTOCOL_NONE, 2, false}},
		leftControlMapping(_leftMapping),
		leftState(_leftState),
		rightControlMapping(_rightMapping),
		rightState(_rightState),
		gamepadReport(hid_gamepad_report_t(0, 0, 0, 0, 0, 0, 0, 0)) {
		controller.setStringDescriptor("Overcooked!2 Gamepad #" + counter++);
		controller.begin();
	}
	void update() {
		bool dirty = false;
		//Serial.println("Update");
		if (leftState != NULL) {
			dirty = (dirty || leftState->update());
		}
		if (rightState != NULL) {
			dirty = (dirty || rightState->update());
		}
		if (dirty) {
			for (int i = 0; i < MAX_KEYS; i++) {
				pressedKeys[i] = 0;
			}
			gamepadReport.buttons = 0;
			gamepadReport.x = gamepadReport.y = gamepadReport.rx = gamepadReport.ry = 0;
			apply(leftControlMapping, leftState);
			apply(rightControlMapping, rightState);
			sendReport();
		}
	}
};

Controller* controllers[GAMEPAD_COUNT] { NULL, NULL };

// Those symbols are matching what's in the Overcooked!2 Controller Mod
#define PLAYER1_CHOPPING_KEY HID_KEY_1
#define PLAYER2_CHOPPING_KEY HID_KEY_2
#define PLAYER3_CHOPPING_KEY HID_KEY_3
#define PLAYER4_CHOPPING_KEY HID_KEY_4
#define PLAYER1_WASHING_KEY HID_KEY_A
#define PLAYER2_WASHING_KEY HID_KEY_B
#define PLAYER3_WASHING_KEY HID_KEY_C
#define PLAYER4_WASHING_KEY HID_KEY_D
#define PLAYER1_EXTINGUISHING_KEY HID_KEY_W
#define PLAYER2_EXTINGUISHING_KEY HID_KEY_X
#define PLAYER3_EXTINGUISHING_KEY HID_KEY_Y
#define PLAYER4_EXTINGUISHING_KEY HID_KEY_Z
#define PLAYER1_THROWING_KEY 0
#define PLAYER2_THROWING_KEY 0
#define PLAYER3_THROWING_KEY 0
#define PLAYER4_THROWING_KEY 0

void setup() {
	Serial.begin(115200);
	// Manual begin() is required on core without built-in support e.g. mbed rp2040
	if (!TinyUSBDevice.isInitialized()) {
		TinyUSBDevice.begin(0);
	}
	PlayerState *leftState = new PlayerState(2, 3, 4, 5, 6, 7, 8, 9, 10);
	PlayerControlMapping *unsplitPlayer1Mapping = new PlayerControlMapping(true, GAMEPAD_BUTTON_WEST, GAMEPAD_BUTTON_SOUTH, PLAYER1_CHOPPING_KEY, PLAYER1_WASHING_KEY, PLAYER1_EXTINGUISHING_KEY, PLAYER1_THROWING_KEY);
	PlayerControlMapping *leftMapping = new PlayerControlMapping(true, GAMEPAD_BUTTON_TL2, GAMEPAD_BUTTON_TL, PLAYER1_CHOPPING_KEY, PLAYER1_WASHING_KEY, PLAYER1_EXTINGUISHING_KEY, PLAYER1_THROWING_KEY);
	PlayerState *rightState = new PlayerState(11, 12, 13, 14, 15, 16, 17, 18, 19);
	PlayerControlMapping *rightMapping = new PlayerControlMapping(false, GAMEPAD_BUTTON_TR2, GAMEPAD_BUTTON_TR, PLAYER2_CHOPPING_KEY, PLAYER2_WASHING_KEY, PLAYER2_EXTINGUISHING_KEY, PLAYER2_THROWING_KEY);
	controllers[0] = new Controller(unsplitPlayer1Mapping, leftState, NULL, NULL);
	// If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
	if (TinyUSBDevice.mounted()) {
		TinyUSBDevice.detach();
		delay(10);
		TinyUSBDevice.attach();
	}

	//Serial.println("Overcooked!2 Custom Arcade Controller - ready");
}

void loop() {
	#ifdef TINYUSB_NEED_POLLING_TASK
	// Manual call tud_task since it isn't called by Core's background
	TinyUSBDevice.task();
	#endif

	// not enumerated()/mounted() yet: nothing to do
	if (!TinyUSBDevice.mounted()) {
		return;
	}

	for (int i = 0; i < GAMEPAD_COUNT; i++) {
		if (controllers[i] != NULL) {
			controllers[i]->update();
		}
	}
}
