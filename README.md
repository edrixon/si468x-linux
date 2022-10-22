DAB radio using DABshield Arduino board running on a Raspberry Pi

Shield is connected to Raspberry Pi SPI interface and a couple of GPIO pins.

Uses gpsd for coverage logging.

si468x.c and si468x.h are library based on software provided for the DAB shield board.  Changed to run under Linux, written in C instead of C++.
Supports only DAB functionality of Si468x chips.

Radio operates in one of three modes
(1) Simple radio, tuned to a service on an ensemble.  Audio via icecast/darkice over network stream on web server.
(2) Band logging.  When idle, scans band and logs any active ensembles it finds
(3) Coverage.  When idle, periodically logs GPS position, RSSI, CNR, SNR and FIC quality to CSV file for processing and display with Google Maps.

*Control engine manages the hardware and maintains a shared memory segment.
*web server allows web access to radio for tuning and streaming of received audio
*telnet server allows remote control and configuration using command line

