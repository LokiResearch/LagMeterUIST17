# Description

## electronicBoard

Contains the Eagle files to replicate the lagmeter shield for Arduino Leonardo.

A [shopping basket](http://www.digikey.fr/short/qtt791) with the list of components to buy is ready for Digikey (also available as a [CSV file](electronicBoard/digikey.csv)).


## LagmeterLeonardo

Contains the Arduino code to run on an Arduino Leonardo board with the above shield.

Requires to install [HID-Project](https://github.com/NicoHood/HID/) first.

## TurboHID

Contains C++ source code to communicate with the Arduino Leonardo using raw HID.

You have to install [libpointing](http://libpointing.org) first (used for the timestamps).

To compile the library run ```qmake && make```

On Windows, the library is provided in the external folder.

## BasicLogger

Code for logging data coming from the Arduino

Successfully compile using Qt 5.15.1 MSCV2015 64 bits in release mode.

## ReplicationCppQt && ReplicationCppGLUT

Code for producing tables 4, 5 & 6 in the paper. Compile using ```qmake && make```

Edit ```config.cfg``` to provide the URI for your mouse.

Directly pass the config file as a parameter for the GLUT application and use ```-c config.cfg``` for the Qt application.

## parser

Python code to parse the log files and compute stats.

Example: ```./parse-log --start piezo yourlogfile```

You have to correct the reported values according to what is described in the paper: "Intermediate latency values were computed by subtracting the host computer piezo time from the considered event time (libpointing, system, toolkit, repaint or screen), adding 5.7 ms and further adding 0.7 ms for the initial communication between the Arduino and the host computer." (page 5). Read the paper for more details.

## FlashApps

Code for producing Table 7 in the paper.

## UISTdemo

Android code used for the UIST demo showing the measure of latency while the user is interacting with an application. The lagmeter is connected to the Android device.

