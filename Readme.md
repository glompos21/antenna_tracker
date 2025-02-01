# Mostly 3D printed antenna tracker

![expoded_view.png](./images/expoded_view.png)

## Parts
- 2x DC Motor Worm Gear Motor 

![foto](./images/dc_motor.png)
- DC motor driver 
- Main electric board 

- 2x 6707-2rs bearings

![bearing](./images/bearing_6707.png)

- Various M3 bolts TBD
- M3 heat insert

# Tolerance
Print the part tolerance_test_print to test the tolerance of your 3D printer for the interal (VarSet.Base_bearing_inside_diameter_with_tolerance) and external (VarSet.Base_bearing_outside_diameter_with_tolerance) diameter of bearing. 
- Change the VarSet.Base_tolerance_outside_circle if the internal base for the bearing is need adjasment (VarSet.Base_bearing_inside_diameter_with_tolerance). 
![internal_base](./images/test_inside_diameter.png)
- Change the VarSet.Base_tolerance_inside_circle if the exteral base of the bearing need adgasment (VarSet.Base_bearing_outside_diameter_with_tolerance).
![outside_base](./images/test_outside_diameter.png)



## Todo
- add electronics
    - [AntRunner](https://github.com/wuxx/AntRunner): keep the software - change the motors from stepper to dc





## License

Copyright (c) 2024 Kostas Gompakis

Licensed under the MIT license.
