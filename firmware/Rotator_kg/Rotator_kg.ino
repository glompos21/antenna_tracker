
//Includes
#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <LSM303.h>

#include "timer.h"
#include "mot.h"


// #include "esp_bt_main.h"
// #include "esp_bt_device.h"
// #include "BluetoothSerial.h"

// ////////Bluetooth
// #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
// #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
// #endif
// BluetoothSerial SerialBT;
// ////////!Bluetooth

Preferences preferences;
LSM303 compass;

#define SerialPort Serial  //Please uncomment this line to use the USB port.
//#define SerialPort Serial1        //Please uncomment this line to use the TTL port.
#define WINDUP_LIMIT 450  //Sets the total number of degrees azimuth rotation in any direction before resetting to zero
//Motor pins - Don't change
const int azFwdPin = 25;
const int azRevPin = 26;
const int elFwdPin = 32;
const int elRevPin = 33;

// //Motor drive gains. These set the amount of motor drive close to the set point
// const int azGain = 25;  //Azimuth motor gain
// const int elGain = 25;  //Elevation motor gain
// //Filter constants
// const float azAlpha = 0.25;    //Alpha value for AZ motor filter: Decrease to slow response time and reduce motor dither.
// const float elAlpha = 0.25;    //Alpha value for EL motor filter: Decrease to slow response time and reduce motor dither.

//Modes
enum Modes { tracking,
             monitoring,
             calibrating,
             debugging,
             pausing,
             manual };  //Rotator controller modes

//Global variables
float az;        //Antenna azimuth
float el;        //Antenna elevation
String line;     //Command line
float azSet;     //Antenna azimuth set point
float elSet;     //Antenna elevation set point
float azLast;    //Last antenna azimuth reading
float elLast;    //Last antenna element reading
float azWindup;  //Antenna windup angle from startup azimuth position
float azOffset;  //Antenna azimuth offset for whole revolutions
bool windup;     //Antenna windup condition
float azError;   //Antenna azimuth error
float elError;   //Antenna elevation error
Modes mode;      //Rotator mode
int calibration_counter = 0;
float magnetic_declination = 0;
//Objects

//Motor driver object: Mot xxMot(Driver-Type, Filter-Alpha, Gain, Fwd-Pin, Rev/Dir-Pin)
Mot azMot(-1, -1, azFwdPin, azRevPin);  //AZ motor driver object
Mot elMot(-1, -1, elFwdPin, elRevPin);  //EL motor driver object

//Non-blocking Timer object
Timer t1(100);


//Functions
void reset(bool getCal) {
  //Reset the rotator, initialize its variables and optionally get the stored calibration
  azSet = 0.0;
  elSet = 0.0;
  line = "";
  azLast = 0.0;
  elLast = 0.0;
  azWindup = 0.0;
  azOffset = 0.0;
  mode = tracking;
  windup = false;
  // if (getCal) restore();
  azError = 0.0;
  elError = 0.0;
  t1.reset(100);
  printCal();
}

float diffAngle(float a, float b) {
  //Calculate the acute angle between two angles in -180..180 degree format
  float diff = a - b;
  if (diff < -180) diff += 360;
  if (diff > 180) diff -= 360;
  return diff;
}

void save() {
  // magnetic_declination
  preferences.begin("calibration", false);  // namespace "calibration", RW mode
  // Save the calibration data structure to preferences
  // preferences.putBytes("lsm_cal", &lsm.cal, sizeof(lsm.cal));
  preferences.end();
}

void restore() {
  preferences.begin("calibration", true);  // namespace "calibration", read-only mode
  // preferences.getBytes("lsm_cal", &lsm.cal, sizeof(lsm.cal));
  preferences.end();
}


void setLSMCalData() {
  compass.m_min = (LSM303::vector<int16_t>){ -444, -475, -1455 };
  compass.m_max = (LSM303::vector<int16_t>){ +824, +486, -641 };
}


void printCal(void) {
  char buffer[200];  // Ensure this is large enough for your full output string

  SerialPort.println(buffer);
}
void printMon(float az, float el, float azSet, float elSet, float azWindup, float azError, float elError) {
  //Print the monitor data
  SerialPort.print("printMon, az:");
  SerialPort.print(az, 2);
  SerialPort.print(",azSet:");
  SerialPort.print(azSet, 2);
  SerialPort.print(",azError:");
  SerialPort.print(azError, 2);
  SerialPort.print(",el:");
  SerialPort.print(el, 2);
  SerialPort.print(",elSet:");
  SerialPort.print(elSet, 2);
  SerialPort.print(",elError:");
  SerialPort.print(elError, 2);
  SerialPort.print(",azWindup:");
  SerialPort.print(azWindup, 2);
  SerialPort.print(",windup:");
  SerialPort.println(windup);
}

void printAzEl() {
  //Print the rotator feedback data in Easycomm II format
  SerialPort.print("AZ");
  SerialPort.print((az < 0) ? (az + 360) : az, 1);
  SerialPort.print(" EL");
  SerialPort.print(el, 1);
  SerialPort.print("\n");
}

void calibrate() {
}

void getWindup(bool *windup, float *azWindup, float *azOffset, float *azLast, float *elLast, float az, float elSet) {
  //Get the accumulated windup angle from the home position (startup or last reset position) and set the windup state if greater than the limit.
  //Get the raw difference angle between the current and last azimuth reading from the sensor
  float azDiff = az - *azLast;

  //Detect crossing South: azDiff jumps 360 for a clockwise crossing or -360 for an anticlockwise crossing
  //Increment the azimuth offset accordingly
  if (azDiff < -180) *azOffset += 360;
  if (azDiff > 180) *azOffset -= 360;

  //Save the current azimuth reading for the next iteration
  *azLast = az;

  //Compute the azimuth wind-up angle, i.e. the absolute number of degrees from the home position
  *azWindup = az + *azOffset;

  //Detect a windup condition where the antenna has rotated more than 450 degrees from home
  if (abs(*azWindup) > WINDUP_LIMIT) *windup = true;  //Set the windup condition - it is reset later when the antenna nears home

  //Perform the anti-windup procedure at the end of each pass - This is overkill unless you absolutely don't want anti-windup during a pass
  //  if (elSet <= 0)
  //    if (elLast > 0)
  //      if (mode == tracking) {
  //        *windup = true;
  //      }

  //Save the current elevation reading for the next iteration
  *elLast = elSet;
}


void getAzElError(float *azError, float *elError, bool *windup, float *azSet, float *elSet, float az, float el) {
  //Compute the azimuth and elevation antenna pointing errors, i.e. angular offsets from set positions
  //Compute the azimuth antenna pointing error: Normally via the shortest path; opposite if windup detected.
  if (*windup) {  //Check for a windup condition
    //To unwind the antenna set an azError in the appropriate direction to home
    *azError = constrain(azWindup, -180, 180);  //Limit the maximum azimuth error to -180..180 degrees
    //Cancel the windup condition when the antenna is within 180 degrees of home (Actually 175 degrees to avoid rotation direction ambiguity)
    //Set a zero home position by default, but return azumith control to the computer if still connected
    if (abs(*azError) < 175) *windup = false;  //Cancel windup and permit computer control
  } else {
    //Compute the normal azimuth antenna pointing error when there is no windup condition
    *azError = diffAngle(az, *azSet);
  }

  //Compute the elevation antenna pointing error
  *elError = diffAngle(el, *elSet);
}

void processPosition() {
  //Perform the main operation of positioning the rotator under different modes
  //Read the accelerometer and magnetometer

  compass.read();
  az = compass.heading() + magnetic_declination;
  el = (atan2(compass.a.x, compass.a.z) * -180.0) / M_PI;

  switch (mode) {
    case debugging:
      // printDebug();  //Print the raw sensor data for debug purposes
      SerialPort.print(az);
      SerialPort.print(",");
      SerialPort.println(el);
      break;
    case calibrating:
      calibrate();  //Process calibration data
      break;
    case pausing:
      azMot.halt();  //Stop the AZ motor
      elMot.halt();  //Stop the EL motor
      azError = 0;
      elError = 0;
      break;
    case manual:
      printMon(az, el, azSet, elSet, azWindup, azError, elError);
      break;
    default:
      if (azSet == 0 && elSet == 0) {
        azError = 0;
        elError = 0;
      } else {
        // getWindup(&windup, &azWindup, &azOffset, &azLast, &elLast, az, elSet);   //Get the AZ windup angle and windup state
        // if (mode == demonstrating) getAzElDemo(&azSet, &elSet, &azInc, &elInc);  //Set the AZ and EL automatically if in demo mode
        getAzElError(&azError, &elError, &windup, &azSet, &elSet, az, el);  //Get the antenna pointing error
      }
      if (mode == monitoring) printMon(az, el, azSet, elSet, azWindup, azError, elError);  //Print the data if in monitor mode
  }
  if (isnan(azError)) azError = 0;
  if (isnan(elError)) elError = 0;
}

void processMotors() {
  //Drive the motors to reduce the azimuth and elevation error to zero
  azMot.drive(azError);
  elMot.drive(elError);
}

void processUserCommands(String line) {
  //Process user commands
  //User command type 1: r, b, m, c, a, d, s, d, h, p or e<decl> followed by a carriage return
  //User command type 2: <az> <el> followed by a carriage return
  String param;    //Parameter value
  int firstSpace;  //Position of the first space in the command line
  int secondSpace;
  String azString;
  String elString;
  char command = line.charAt(0);  //Get the first character
  switch (command) {              //Process type 1 user commands
    case 'r':                     //Reset command
      SerialPort.println("Reset in progress");
      reset(true);  //Reset the rotator and load calibration from EEPROM
      SerialPort.println("Reset complete");
      break;
    case 'b':  //Debug command
      SerialPort.println("Debugging in progress: Press 'a' to abort");
      mode = debugging;
      t1.reset(100);
      break;
    case 'm':  //Monitor command
      SerialPort.println("Monitoring in progress: Press 'a' to abort");
      mode = monitoring;
      t1.reset(100);
      break;
    case 'c':  //Calibrate command
      SerialPort.println("Calibration in progress: Press 'a' to abort or 's' to save");
      reset(false);  //Reset the rotator, but don't load calibration from EEPROM
      mode = calibrating;
      t1.reset(50);
      break;
    case 'a':  //Abort command
      mode = tracking;
      t1.reset(100);
      reset(true);
      SerialPort.println("Function aborted");
      break;
    case 'e':                     //Magnetic declination command
      param = line.substring(1);  //Get the second parameter
      magnetic_declination = param.toFloat();
      break;
    case 's':  //Save command
      if (mode == calibrating) {
        save();
        reset(true);
        SerialPort.println("Calibration saved");
        printCal();
      } else {
        SerialPort.println("Not at Calibration mode");
      }
      break;
    case 'g':  // Go to
      mode = manual;
      firstSpace = line.indexOf(' ');              // Index of first space
      param = line.substring(firstSpace + 1);      // Get substring after 'g '
      secondSpace = param.indexOf(' ');            // Find space between az and el
      azString = param.substring(0, secondSpace);  // Extract az value
      azError = azString.toFloat();                // Parse azimuth

      elString = param.substring(secondSpace + 1);  // Extract el value
      elError = elString.toFloat();                 // Parse elevation
      break;

    case 'h':  //Help command
      SerialPort.println("Commands:");
      SerialPort.println("az el -(0..360 0..90)");
      SerialPort.println("r -Reset");
      SerialPort.println("eNN.N -MagDecl");
      SerialPort.println("c -Calibrate");
      SerialPort.println("s -Save");
      SerialPort.println("a -Abort");
      SerialPort.println("b -Debug");
      SerialPort.println("m -Monitor");
      SerialPort.println("p -Pause");
      SerialPort.println("g +-az +-el - Go to +-az +-el ie g 1 -3 -> move az: +1 el:-3");
      break;
    case 'p':  //Pause command
      if (mode == pausing) {
        mode = tracking;
      } else {
        mode = pausing;
        SerialPort.println("Paused");
      }
      break;
    default:  //Process type 2 user commands
      mode = tracking;
      firstSpace = line.indexOf(' ');          //Get the index of the first space
      param = line.substring(0, firstSpace);   //Get the first parameter
      azSet = param.toFloat();                 //Get the azSet value
      param = line.substring(firstSpace + 1);  //Get the second parameter
      elSet = param.toFloat();                 //Get the elSet value
  }
}
// printMon: nan,nan,0,0,nan,0,nan,nan
void processEasycommCommands(String line) {
  //Process Easycomm II rotator commands
  //Easycomm II position command: AZnn.n ELnn.n UP000 XXX DN000 XXX\n
  //Easycomm II query command: AZ EL \n
  String param;                    //Parameter value
  int firstSpace;                  //Position of the first space in the command line
  int secondSpace;                 //Position of the second space in the command line
  if (line.startsWith("AZ EL")) {  //Query command received
    printAzEl();                   //Send the current Azimuth and Elevation
  } else {
    if (line.startsWith("AZ")) {                            //Position command received: Parse the line.
      firstSpace = line.indexOf(' ');                       //Get the position of the first space
      secondSpace = line.indexOf(' ', firstSpace + 1);      //Get the position of the second space
      param = line.substring(2, firstSpace);                //Get the first parameter
      azSet = param.toFloat();                              //Set the azSet value
      if (azSet > 180) azSet = azSet - 360;                 //Convert 0..360 to -180..180 degrees format
      param = line.substring(firstSpace + 3, secondSpace);  //Get the second parameter
      elSet = param.toFloat();                              //Set the elSet value
    }
  }
}

void processCommands(void) {
  //Process incoming data from the control computer
  //User commands are entered by the user and are terminated with a carriage return
  //Easycomm commands are generated by a tracking program and are terminated with a line feed
  while (SerialPort.available() > 0) {
    char ch = SerialPort.read();  //Read a single character from the serial buffer
    switch (ch) {
      case 13:                      //Carriage return received
        processUserCommands(line);  //Process user commands
        line = "";                  //Command processed: Clear the command line
        break;
      case 10:                          //Line feed received
        processEasycommCommands(line);  //Process Easycomm commands
        line = "";                      //Command processed: Clear the command line
        break;
      default:       //Any other character received
        line += ch;  //Add this character to the command line
        break;
    }
  }


  // while (SerialBT.available() > 0) {
  //   char ch = SerialBT.read();  //Read a single character from the serial buffer
  //   switch (ch) {
  //     case 13:                      //Carriage return received
  //       processUserCommands(line);  //Process user commands
  //       line = "";                  //Command processed: Clear the command line
  //       break;
  //     case 10:                          //Line feed received
  //       processEasycommCommands(line);  //Process Easycomm commands
  //       line = "";                      //Command processed: Clear the command line
  //       break;
  //     default:       //Any other character received
  //       line += ch;  //Add this character to the command line
  //       break;
  //   }
  // }
}


// void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
//   switch (event) {
//     case ESP_SPP_SRV_OPEN_EVT:
//       SerialPort.println("BT Connected");
//       // beep(40, 5);
//       break;
//     case ESP_SPP_CLOSE_EVT:
//       SerialPort.println("BT Disconnected");
//       // beep(400, 2);
//       break;
//     default:
//       break;
//   }
// }

// void printDeviceAddress() {
//   const uint8_t *point = esp_bt_dev_get_address();
//   for (int i = 0; i < 6; i++) {
//     char str[3];
//     sprintf(str, "%02X", (int)point[i]);
//     SerialPort.print(str);
//     if (i < 5) {
//       SerialPort.print(":");
//     }
//   }
//   SerialPort.print("\n");
// }

void testMotor() {
  int delay_ms = 5000;
  int _drive = 40.;
  read_print_sensor();
  azMot.drive(_drive);
  delay(delay_ms);
  read_print_sensor();
  azMot.drive(-1 * _drive);
  delay(delay_ms);
  azMot.drive(0.0);
  read_print_sensor();
  elMot.drive(_drive);
  delay(delay_ms);
  read_print_sensor();
  elMot.drive(-1 * _drive);
  delay(delay_ms);
  elMot.drive(0.0);
}

void read_print_sensor() {
  compass.read();
  //Get the antenna AZ and EL
  az = compass.heading() + magnetic_declination;
  el = (atan2(compass.a.x, compass.a.z) * -180.0) / M_PI;
  SerialPort.print("az:");

  SerialPort.print(az);
  SerialPort.print(",");
  SerialPort.print(",el:");
  SerialPort.println(el);
}

void setup() {
  //Initialize the system
  //Reset the rotator and load configuration from EEPROM
  reset(true);
  //Initialize the serial port
  SerialPort.begin(9600);

  // SerialBT.register_callback(callback);
  // SerialBT.begin("Glompos21_ESP32_Rotor");
  // SerialPort.print("\nYou can pair bluetooth on:");
  // printDeviceAddress();
  SerialPort.println("\nGlompos21_ESP32_Rotor");
  //Initialize the sensor
  Wire.begin();
  compass.init();
  compass.enableDefault();
  azMot.halt();
  elMot.halt();
  setLSMCalData();
  read_print_sensor();
  SerialPort.println("starting loop...");
}


void loop() {
  //Repeat continuously
  processCommands();             //Process commands from the control computer
  t1.execute(&processPosition);  //Process position only periodically
  processMotors();               //Process motor drive

  // testMotor();
}









// void printCal(void) {
//   //Print the calibration data
//   SerialPort.print("md:");
//   SerialPort.print(lsm.cal.md, 1);
//   SerialPort.print(",");
//   SerialPort.print("me.i:");
//   SerialPort.print(lsm.cal.me.i, 1);
//   SerialPort.print(",");
//   SerialPort.print("me.j:");
//   SerialPort.print(lsm.cal.me.j, 1);
//   SerialPort.print(",");
//   SerialPort.print("me.k:");
//   SerialPort.print(lsm.cal.me.k, 1);
//   SerialPort.print(",");
//   SerialPort.print("ge.i:");
//   SerialPort.print(lsm.cal.ge.i, 1);
//   SerialPort.print(",");
//   SerialPort.print("ge.j:");
//   SerialPort.print(lsm.cal.ge.j, 1);
//   SerialPort.print(",");
//   SerialPort.print("ge.k:");
//   SerialPort.print(lsm.cal.ge.k, 1);
//   SerialPort.print(",");
//   SerialPort.print("ms.i:");
//   SerialPort.print(lsm.cal.ms.i, 1);
//   SerialPort.print(",");
//   SerialPort.print("ms.j:");
//   SerialPort.print(lsm.cal.ms.j, 1);
//   SerialPort.print(",");
//   SerialPort.print("ms.k:");
//   SerialPort.print(lsm.cal.ms.k, 1);
//   SerialPort.print(",");
//   SerialPort.print("gs.i:");
//   SerialPort.print(lsm.cal.gs.i, 1);
//   SerialPort.print(",");
//   SerialPort.print("gs.j:");
//   SerialPort.print(lsm.cal.gs.j, 1);
//   SerialPort.print(",");
//   SerialPort.print("gs.k:");
//   SerialPort.println(lsm.cal.gs.k, 1);
// }