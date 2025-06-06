# FRG-100CTL
An ESP32 based hardware controller with Nextion display for the Yaesu FRG-100 receiver.

![FRG100-CTL](https://github.com/user-attachments/assets/46b08680-8708-494b-9c8a-1ac31a9b92f1)


This is a controller for the Yaesu FRG-100 receiver.  As a stand alone radio I felt it lacked frequency agility, and this controller provides that functionality.

## ESP32 Programming
The controller is based on the ESP32 and uses a 3.5" Nextion display.  To program the ESP32 you will need to install the Arduino IDE.  The Arduino IDE can be downloaded here: https://www.arduino.cc/en/software/ Once installed you will also need to add the Easy Nextion library through the Library Manager in the IDE.  Installaton instructions can be found here: https://github.com/Seithan/EasyNextionLibrary.  From the Library Manager choose the Easy Nextion Library and click on install.

![EasyNextion](https://github.com/user-attachments/assets/03fbc209-d9d0-411a-84db-3e5e98209fa4)


The EasyNextion library as installed is limited to 50 functions.  I have modified two of the library files **trigger.h** and **calltriggers.cpp** and you need to replace the origionals with these.

Under Windows you will find these files at c:\users\<name>\Documents\Arduino\Libraries\Easy_Nextion_Library\src

For MacOS it is located under Users/<user>/Documents/Arduino/Libraries/Easy_Nextion_Library/src

You will also need to add the ESP32 board and files. 

![ESP_BoardManager](https://github.com/user-attachments/assets/9c6e06e3-7c43-4ac5-aa96-7dcc969c8481)

To compile the program and upload it to the ESP32 create a folder below the Arduino folder named FRG-100CTL (..\Documents\Arduino\FRG-100CTL).  Then copy the file named FRG-100CTL.ini to that directory.

## Display Programming
You will need to program your Nextion display with the screen file that matches your Nextion model number.  The file contains all the screens and button actions.  To load the file copy the .tft file to a FAT32 formatted micro SD card.  It must be the only file on the SD card.  Insert the card, apply power to the board using the Nextion supplied micro USB adapter.  It will load and display a success message.  Remove power, remove the SD card and the display will be ready.

## PCB
I've created a simple PCB for this project.  The associated files are:

  - FRG-100CTL_Gerbers.zip      These are the Gerber files required to manufacture the PCB
  - FRG-100CTL_Schematic.pdf    The schematic diagram.
  - FRG-100CTL_BOM.pdf          Parts list for the PCB
## Enclosure   
I created a 3D printed case for this project.  Interested viewers can find details here: https://www.thingiverse.com/thing:7029759

