#include <Arduino.h>



struct Button {
	const uint8_t PIN;
	uint32_t numberKeyPresses;
  uint32_t lastPress;
	bool pressed;
};

Button button1 = {18, 0, false};

void IRAM_ATTR isr() {
  if(button1.lastPress + 250 > millis()) {
    return;
  }


	button1.numberKeyPresses++;
  button1.lastPress = millis();
	button1.pressed = true;
}

void setup() {
	Serial.begin(9600);
	pinMode(button1.PIN, INPUT_PULLDOWN);
	attachInterrupt(button1.PIN, isr, FALLING);
}

void loop() {
	if (button1.pressed) {
		Serial.printf("Button has been pressed %u times\n", button1.numberKeyPresses);
		button1.pressed = false;
	}
}