# Mostly 3D printed antenna tracker

![expoded_view.png](./images/expoded_view.png)

The main change from previous version is change into double helix.
Max 1 min for traversing 180o for azimuthal so around 6o/sec

## Parts
- 2x DC Motor Worm Gear Motor 

![foto](./images/dc_motor.png)
- DC motor driver 
- Main electric board 

- 2x 6707-2rs bearings

![bearing](./images/bearing_6707.png)

- 8 x M3x6 (I got a M3x10 and file it)
- 12 x M3x10
- 8 x M3x20
- 8 x M3 nut and washer
- 2x M3x6 Screw without Head  
- 2.5mm drill bit. Used to set the hole for the small_spur_gear_double_helix
- 12x M3 heat insert

# Tolerance
Print the part tolerance_test_print to test the tolerance of your 3D printer for the integral (VarSet.Base_bearing_inside_diameter_with_tolerance) and external (VarSet.Base_bearing_outside_diameter_with_tolerance) diameter of bearing. 
- Change the VarSet.Base_tolerance_outside_circle if the internal base for the bearing is need adjacent (VarSet.Base_bearing_inside_diameter_with_tolerance). 
![internal_base](./images/test_inside_diameter.png)
- Change the VarSet.Base_tolerance_inside_circle if the external base of the bearing need adjacent (VarSet.Base_bearing_outside_diameter_with_tolerance).
![outside_base](./images/test_outside_diameter.png)



# Software

Controlling on android: https://github.com/rt-bishop/Look4Sat/

## On Linux
Install:
- gpredict
- HAMLIB https://github.com/Hamlib/Hamlib/wiki/Download#latest-release
Plug the rotator into to a USB port and start the HAMLIB rotator control daemon in a terminal window:
- rotctld -m 202 -r /dev/ttyACM0 -s 9600 -C timeout=500 -vvv
- Where: 202 is the model number for a rotator emulating AMSAT Easycomm II protocol 


## Testing
2 different type of commands
- Easycomm II rotator. Need to be ternamanted with a New Line code.
    - Querey commands send: "AZ EL"
    - Send command: "AZnn.n ELnn.n"
- User commands. NEED to be terminated with Carriege Return (CR)
	- to set to az=100.1 and el=45.4, put on 100.1 45.5<CR>

# L298N Motor Driver Logic Table (1 Motor)

| IN1 | IN2 | Motor State      |
|-----|-----|------------------|
|  1  |  0  | Forward          |
|  0  |  1  | Backward         |
|  1  |  1  | Brake (Short)    |
|  0  |  0  | Stop (Free Run)  |

## Firmware
Based on https://github.com/F4HTB/esp32Rotor
Calibrate the LSM303DLHC sensor: 
- https://www.sarcnet.org/mini-satellite-antenna-rotator-mk1.html -> Calibration Instructions
- https://www.youtube.com/watch?v=oJnpO5Nj7Gc


# Calibration Jig

A calibration jig for the LSM303DLHC. 
See [calibration.md](./firmware/calibration.md) for more info.
You can use a 3d printed [calibration_jig](https://github.com/glompos21/calibration_jig)





## License

Copyright (c) 2024 Kostas Gompakis

Licensed under the MIT license.
