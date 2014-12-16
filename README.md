calibSrim
=========

program to make calibrations based on relativistic kinematics and SRIM data for charged particle losses in material 

Usage:
---------

```
.L nrfit.C+
nrfit("calib.txt")
```


Data input:
-----------

The data are in a text configuration file - e.g. calib.txt

Syntax:

line starting with __#__ is a comment
**BUT**
line starting with __#define__ or __#free__ contains a definition

the main part has the structure

```#-----------------------------------------------------------------------------------------
# k    dk           reaction     medium    Exc   Tbeam thickness angle   deadlay livelay P3  P2  P1   P0
#------------------------------------------------------------------------------------------
```

* _k and dk_ ... are channel and error

* _reaction_ ... the notation is lowercase elementA where the A is a mass number
         ... the reaction is described by a word projetile_target_ejectile_remnant
         ... isotopes must correspond to data prepared from SRIM and to internal mass tables in the source code

* _medium_ ... material of the target
       ... must correspond to data prepared from SRIM

* _Exc._   ... excited state in MeV ... influences kinematics

* _Tbeam_  ... kinetic Energy of the projectile

* _thickness_ ... in microns

* _angle_ ...    beam deviation angle

* _deadlay_ ... dead layer of the detector

* _livelay_ ... detector thickness

* _P3-P0_ ...  polynomial calibration coefficients highest to lowest order

```
#########################
# alfa T1  dE
 872.593    0.047  he4_he4_he4_he4  mylar  0.0    T4     0.0  0.0   de1layau  de1laysi    0.0 0.0 pde1_1 pde1_0
 879.290    0.025  he4_he4_he4_he4  mylar  0.0    T3     0.0  0.0   de1layau  de1laysi    0.0 0.0 pde1_1 pde1_0
 921.394    0.057  he4_he4_he4_he4  mylar  0.0    T2     0.0  0.0   de1layau  de1laysi    0.0 0.0 pde1_1 pde1_0
 927.968    0.025  he4_he4_he4_he4  mylar  0.0    T1     0.0  0.0   de1layau  de1laysi    0.0 0.0 pde1_1 pde1_0

```


the configuration can have variables and eventualy free for fit:

```
#--------------T7--------------
#     40ug
#define var  de7layau=0.02070
#define var  de7laysi=264
#define var  pde7_1=0.006372
#define var  pde7_0=-0.107064
#   BEAM  FROM  MONO  16.18
#define var T=16.42
############################
#free var T=step=0.01
#free var pde1_1=step=0.0001
#free var pde1_0=step=0.0001
```

An example of the result:

![alt text](https://github.com/jaromrax/calibNR/blob/master/calib_complex.jpg "pull graph")

Remarks:
----------

* Use energy in MeV
* different linear calibration coefficients ... data is shown with different color?
