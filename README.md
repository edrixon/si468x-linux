This is my Si468x based DAB receiver for Linux

Built to run on DietPi minimal Linux distro

Hardware is a Raspberry Pi 4, a DABShield radio board, PCF2127 based RTC,
USB sound card, USB GPS  and some LEDs.

DABshield board is designed for an Arduino but is controlled by the RPi i2c
interface in this application.  The IOREF pin on the DABShield is connected
to 3V supply for correct IO levels.

Requires GPSd, Icecast, pigpio and hostapd.

Radio operates in three modes.

First is web controlled receiver with streaming audio.
User can change frequency and service.  Audio is streamed to user with and
Icecast server.  A telnet server is also available for control and setup.
This uses port 5000.

When idle (no telnet or web clients), radio will either scan and log any
ensembles found or it will log signal strength etc of currently selected
multiplex along with position from GPS.  This is intended as a coverage
logger for drive testing of a DAB multiplex.  A KML file can be generated for
Google Earth based on either RSSI or CNR. 

System time is set using NTP and GPS time.

Network access is either with Ethernet or wireless access with hostapd which
starts a wireless access point.  Wireless access can be used to control the
radio with a laptop when mobile.  

Library code is based on the C++ library provided for the DABShield board.
It's changed for Linux, converted to C and only supports DAB functionality.


