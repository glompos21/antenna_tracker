//
// DOIT ESP32 DEVKIT V1

const int azFwdPin = 25;
const int azRevPin = 26;
const int elFwdPin = 32;
const int elRevPin = 33;

void setup() {
	// Set all the motor control pins to outputs
	pinMode(azFwdPin, OUTPUT);
	pinMode(azRevPin, OUTPUT);
	pinMode(elFwdPin, OUTPUT);
	pinMode(elRevPin, OUTPUT);
	
	// Turn off motors - Initial state
	digitalWrite(azFwdPin, LOW);
	digitalWrite(azRevPin, LOW);
	digitalWrite(elFwdPin, LOW);
	digitalWrite(elRevPin, LOW);
}

void loop() {
	directionControl();
	delay(1000);

}

// This function lets you control spinning direction of motors
void directionControl() {

	// Turn on motor A & B
	digitalWrite(azFwdPin, HIGH);
	digitalWrite(azRevPin, LOW);
	digitalWrite(elFwdPin, HIGH);
	digitalWrite(elRevPin, LOW);
	delay(5000);
	
	// Now change motor directions
	digitalWrite(azFwdPin, LOW);
	digitalWrite(azRevPin, HIGH);
	digitalWrite(elFwdPin, LOW);
	digitalWrite(elRevPin, HIGH);
	delay(5000);
	
	// Turn off motors
	digitalWrite(azFwdPin, LOW);
	digitalWrite(azRevPin, LOW);
	digitalWrite(elFwdPin, LOW);
	digitalWrite(elRevPin, LOW);
}
