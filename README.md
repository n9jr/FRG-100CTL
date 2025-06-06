# FRG-100CTL
An ESP32 based hardware controller with Nextion display for the Yaesu FRG-100 receiver.

This is a controller for the Yaesu FRG-100 receiver.  As a stand alone radio I felt it lacked frequency agility, and this controller provides that functionality.

The controller is based on the ESP32 and uses a 3.5" Nextion display.  To program the ESP32 you will need to install the Arduino IDE.  You will also need to add the EasyNextion library through the Library Manager in the IDE.  Installaton instructions can be found here: https://github.com/Seithan/EasyNextionLibrary.  From the Library Manager choose the Easy Nextion Library and click on install.

![EasyNextion](https://github.com/user-attachments/assets/03fbc209-d9d0-411a-84db-3e5e98209fa4)


The EasyNextion library as installed is limited to 50 functions.  I have modified two of the library files trigger.h and calltriggers.cpp and you need to replace the origionals with these.

Under Windows you will find the files at c:\users\<name>\Documents\Arduino\Libraries\Easy_Nextion_Library\src
For MacOS it is located under Users/<user>/Documents/Arduino/Libraries/Easy_Nextion_Library/src

You will also need to add the ESP32 board and files. 

![ESP_BoardManager](https://github.com/user-attachments/assets/9c6e06e3-7c43-4ac5-aa96-7dcc969c8481)





FRG-100CTL.ino            The Arduino sketch for use in the Arduino IDE to compile the program and upload to the ESP32.

PCB related files:

FRG-100CTL_Gerbers.zip    Gerber files for fabricating the PCB.

FRG-100CTL_Schematic.pdf  Circuit schematic diagram.

FRG-100CTL_BOM.pdf        Bill of Materials

EasyNextion Files:

trigger.h

calltriggers.h





