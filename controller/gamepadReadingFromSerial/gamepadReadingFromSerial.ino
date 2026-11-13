#include "Adafruit_TinyUSB.h"
#define PLAYER_COUNT 4
#define GAMEPAD_COUNT 2 // 2 split gamepads
#if CFG_TUD_HID < GAMEPAD_COUNT
	#error "Requires 2 HID instances support. See https://github.com/adafruit/Adafruit_TinyUSB_Arduino/commit/b75604f794acdf88daad310dd75d3a0724129056"
#endif 

uint8_t const desc_hid_report[] = {
	TUD_HID_REPORT_DESC_GAMEPAD()
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
	bool washEnabled = false,
		chopEnabled = false,
		extinguishEnabled = false;
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
	void resetEnabled() {
		chopEnabled = false;
		washEnabled = false;
		extinguishEnabled = false;
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
		enableOtherActions();
	}
	bool isChopping() { return chopEnabled && chopping; }
	bool isWashing() { return washEnabled && washing; }
	bool isThrowing() { return !(chopEnabled || washEnabled || extinguishEnabled) && throwing; }
	bool isExtinguishing() { return extinguishEnabled && extinguishing; }
	bool isHandlingOrReleasing() { return handlingOrReleasing; }
	bool isMovingUp() { return movingUp; } // consider merging those 4 as direction
	bool isMovingDown() { return movingDown; }
	bool isMovingLeft() { return movingLeft; }
	bool isMovingRight() { return movingRight; }
	void enableOtherActions() {
		chopEnabled = false;
		washEnabled = false;
		extinguishEnabled = false;
	}
	void enableChop() {
		enableOtherActions();
		chopEnabled = true;
	}
	void enableWash() {
		enableOtherActions();
		washEnabled = true;
	}
	void enableExtinguish() {
		enableOtherActions();
		extinguishEnabled = true;
	}
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
	PlayerControlMapping(bool _isLeftJoystick, hid_gamepad_button_bm_t _actionButton, hid_gamepad_button_bm_t _handleReleaseButton) :
		isLeftJoystick(_isLeftJoystick),
		actionButton(_actionButton),
		handleOrReleaseButton(_handleReleaseButton) {
	}
	const bool isLeftJoystick;
	const hid_gamepad_button_bm_t actionButton,
		handleOrReleaseButton;
};

class Controller {
private:
	inline static uint8_t counter = 1;
	Adafruit_USBD_HID controller;
	hid_gamepad_report_t gamepadReport;

	PlayerControlMapping *leftControlMapping, *rightControlMapping;
	PlayerState *leftState, *rightState;
	void sendReport() {
		if (controller.ready()) {
			delay(10);
			controller.sendReport(0, &gamepadReport, sizeof(gamepadReport));
		}
	}
	void apply(PlayerControlMapping *mapping, PlayerState *state) {
		if (mapping == NULL || state == NULL) {
			return;
		}
		if (state->isChopping()) {
			pressButton(mapping->actionButton);
		}
		if (state->isWashing()) {
			pressButton(mapping->actionButton);
		}
		if (state->isExtinguishing()) {
			pressButton(mapping->actionButton);
		}
		if (state->isThrowing()) {
			pressButton(mapping->actionButton);
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
			gamepadReport.buttons = 0;
			gamepadReport.x = gamepadReport.y = gamepadReport.rx = gamepadReport.ry = 0;
			apply(leftControlMapping, leftState);
			apply(rightControlMapping, rightState);
			sendReport();
		}
	}
};

Controller* controllers[GAMEPAD_COUNT] { NULL, NULL };

PlayerState* players[PLAYER_COUNT] { NULL, NULL, NULL, NULL };
void setup() {
	Serial.begin(9600);
	// Manual begin() is required on core without built-in support e.g. mbed rp2040
	if (!TinyUSBDevice.isInitialized()) {
		TinyUSBDevice.begin(0);
	}
	PlayerState *player1State = new PlayerState(2, 3, 4, 5, 6, 7, 8, 9, 10);
	PlayerControlMapping *unsplitPlayerMapping = new PlayerControlMapping(true, GAMEPAD_BUTTON_2, GAMEPAD_BUTTON_0);
	PlayerControlMapping *leftMapping = new PlayerControlMapping(true, GAMEPAD_BUTTON_TL2, GAMEPAD_BUTTON_4);
	PlayerState *player2State = new PlayerState(11, 12, 13, 14, 15, 16, 17, 18, 19);
	PlayerControlMapping *rightMapping = new PlayerControlMapping(false, GAMEPAD_BUTTON_TR2, GAMEPAD_BUTTON_5);
	controllers[0] = new Controller(unsplitPlayerMapping, player1State, NULL, NULL);
	controllers[1] = new Controller(unsplitPlayerMapping, player2State, NULL, NULL);
	players[0] = player1State;
	players[1] = player2State;
	// If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
	if (TinyUSBDevice.mounted()) {
		TinyUSBDevice.detach();
		delay(10);
		TinyUSBDevice.attach();
	}

	//Serial.println("Overcooked!2 Custom Arcade Controller - ready");
}

PlayerState *currentlyModifiedPlayer = NULL;
unsigned long lastPing = 0;
void loop() {
	if (millis() - lastPing > 5000) {
		Serial.println("ping");
		lastPing = millis();
	}
  while (Serial.available() > 0) {
    int byte = Serial.read();
		Serial.print("received "); Serial.println(byte);
		Serial.flush();
    if (byte >= '0' && byte <= '9') {
      int playerNumber = byte - '0';
      if (playerNumber < PLAYER_COUNT) {
    	  currentlyModifiedPlayer = players[playerNumber];
      }
    } else if (currentlyModifiedPlayer != NULL)  {
      switch (byte) {
        case 'C': // chop
          currentlyModifiedPlayer->enableChop();
					break;
        case 'W': // wash
          currentlyModifiedPlayer->enableWash();
					break;
        case 'E': // extinguish
					currentlyModifiedPlayer->enableExtinguish();
          break;
        case 'N': // Nothing
					currentlyModifiedPlayer->enableOtherActions();
          break;
      }
      currentlyModifiedPlayer = NULL;
    }
  }
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
