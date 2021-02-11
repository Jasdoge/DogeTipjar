# DogeTipjar
A fancy dog-based internet currency tipjar

# Parts needed

* 1000 uF Capacitor (suggested 10V+)
* [ESP32 Dev board](https://www.aliexpress.com/item/32866131814.html) - You can use any ESP32 board style, but the 3D printed case was designed for that particular style
* [DFPlayer Mini](https://www.aliexpress.com/item/32657798948.html)
* SD Card for your DFPlayer
* [Small speaker](https://www.aliexpress.com/item/32901680862.html)
* [TFT Touch Screen](https://www.aliexpress.com/item/32960934541.html) - The one I used is using the ILI9341 for control and 320x240 resolution. You can use other styles if you want to edit the images & code.
* Wires

# Schematics


# How to program

1. [Install Arduino](https://www.arduino.cc/)
2. [Install ESP32 board](https://github.com/espressif/arduino-esp32)
3. [Install the ESP32 filesystem uploader](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/)
4. Install the following libraries using the Arduino library manager:
	* TFT_eSPI. This one needs you to edit a User_Setup.h library file for your board. There's an example User_Setup.h in the Templates folder setup for the ILI9341 screen that I use in this project. For more info [see the library](https://github.com/Bodmer/TFT_eSPI). 
	* QRCode
	* ArduinoJson
	* ArduinoWebsockets
	* DFPlayerMini_Fast
5. Set your board appropriately in Arduino. I use ESP32 Dev module, No OTA (2MB App/2MB FATFS)
6. Open the TipJar.ino file in Arduino.
7. Plug in your arduino to your computer and select the com port in the tools menu.
8. Go to Tools -> ESP32 Sketch data upload. This should upload all the image files and font to your ESP32.
9. Audio: Format your SD card to FAT32.
10. Find the audio files in the Audio folder. The files need to be pasted in a specific order. Look at Instructions.txt in the Audio folder for the correct sequence.
11. Plug the SD card into the DFPlayer.
12. Compile and upload by hitting the Arrow button in Arduino.


# Case

The STL folder has the 3d prints. It's very simple and requires you to use hot glue to attach the screen. Feel free to make a better one!

