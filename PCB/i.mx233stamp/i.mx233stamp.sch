EESchema Schematic File Version 2  date Tue 20 Sep 2011 03:32:09 PM COT
LIBS:con-jack
LIBS:adm3101e
LIBS:microsd
LIBS:transistor-npn
LIBS:ipc-7351-transistor
LIBS:switch-misc
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:micron_ddr_512Mb
LIBS:iMX23
LIBS:sdmmc
LIBS:usbconn
LIBS:fsusb20
LIBS:r_pack2
LIBS:pasives-connectors
LIBS:EEPROM
LIBS:PWR
LIBS:m25p32
LIBS:PROpendous-cache
LIBS:w_analog
LIBS:gl850g
LIBS:srf2012
LIBS:rclamp0502b
LIBS:mcp130
LIBS:ABM8G
LIBS:usb_a
LIBS:Reset
LIBS:i.mx233stamp-cache
EELAYER 25  0
EELAYER END
$Descr A4 11700 8267
encoding utf-8
Sheet 1 4
Title ""
Date "20 sep 2011"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	3000 4050 3150 4050
Wire Wire Line
	3000 3850 3150 3850
Wire Wire Line
	3000 3550 3150 3550
Wire Wire Line
	4750 1650 5250 1650
Wire Wire Line
	4750 2250 5250 2250
Wire Wire Line
	5250 1950 4750 1950
Wire Wire Line
	5250 1750 4750 1750
Wire Bus Line
	5250 2400 4750 2400
Wire Wire Line
	4750 3450 5250 3450
Wire Wire Line
	5250 3200 4750 3200
Wire Wire Line
	4750 2950 5250 2950
Wire Wire Line
	4750 4050 5250 4050
Wire Wire Line
	4750 3800 5250 3800
Wire Wire Line
	4750 3600 5250 3600
Wire Wire Line
	4750 2850 5250 2850
Wire Wire Line
	4750 2750 5250 2750
Wire Wire Line
	4750 2650 5250 2650
Wire Wire Line
	4750 3700 5250 3700
Wire Wire Line
	4750 3950 5250 3950
Wire Wire Line
	4750 3100 5250 3100
Wire Wire Line
	5250 3350 4750 3350
Wire Wire Line
	4750 2050 5250 2050
Wire Bus Line
	4750 2500 5250 2500
Wire Wire Line
	4750 1850 5250 1850
Wire Wire Line
	4750 2150 5250 2150
Wire Wire Line
	3150 3450 3000 3450
Wire Wire Line
	3000 3750 3150 3750
Wire Wire Line
	3000 3950 3150 3950
$Sheet
S 2150 3250 850  850 
U 4E3BEC51
F0 "USB" 60
F1 "USB.sch" 60
F2 "USB_DM0" T R 3000 3550 60 
F3 "USB_DP0" T R 3000 3450 60 
F4 "USB_P4" B R 3000 3750 60 
F5 "USB_M4" B R 3000 3850 60 
F6 "USM_P3" B R 3000 3950 60 
F7 "USB_M3" B R 3000 4050 60 
$EndSheet
$Sheet
S 5250 1600 1150 2500
U 4D30B991
F0 "DDR" 60
F1 "DDR1.sch" 60
F2 "DDR_A[0..12]" I L 5250 2400 60 
F3 "DDR_DQ[0..15]" B L 5250 2500 60 
F4 "DDR_BA0" I L 5250 3950 60 
F5 "DDR_BA1" I L 5250 4050 60 
F6 "DDR_CE0" I L 5250 2650 60 
F7 "DDR_CASN" I L 5250 2850 60 
F8 "DDR_CKE" I L 5250 3800 60 
F9 "DDR_CLKN" I L 5250 3700 60 
F10 "DDR_CLK" I L 5250 3600 60 
F11 "DDR_DQM0" I L 5250 3350 60 
F12 "DDR_DQM1" I L 5250 3450 60 
F13 "DDR_DQS0" I L 5250 3100 60 
F14 "DDR_DQS1" I L 5250 3200 60 
F15 "DDR_RASN" I L 5250 2950 60 
F16 "DDR_WEN" I L 5250 2750 60 
F17 "GPMI_D04" I L 5250 2050 60 
F18 "GPMI_D02" I L 5250 1950 60 
F19 "GPMI_WRN" I L 5250 2150 60 
F20 "GPMI_RDY1" I L 5250 2250 60 
F21 "GPMI_D01" I L 5250 1850 60 
F22 "GPMI_D00" I L 5250 1750 60 
F23 "GPMI_D03" I L 5250 1650 60 
$EndSheet
$Sheet
S 3150 1600 1600 2500
U 4D30AC69
F0 "i.MX233" 60
F1 "i.MX233.sch" 60
F2 "DDR_A[0..12]" O R 4750 2400 60 
F3 "DDR_DQ[0..15]" B R 4750 2500 60 
F4 "DDR_WEN" O R 4750 2750 60 
F5 "DDR_RASN" O R 4750 2950 60 
F6 "DDR_DQS1" O R 4750 3200 60 
F7 "DDR_DQS0" O R 4750 3100 60 
F8 "DDR_DQM1" O R 4750 3450 60 
F9 "DDR_DQM0" O R 4750 3350 60 
F10 "DDR_CLK" O R 4750 3600 60 
F11 "DDR_CLKN" O R 4750 3700 60 
F12 "DDR_CKE" O R 4750 3800 60 
F13 "DDR_CASN" O R 4750 2850 60 
F14 "DDR_CE0" O R 4750 2650 60 
F15 "DDR_BA1" O R 4750 4050 60 
F16 "DDR_BA0" O R 4750 3950 60 
F17 "GPMI_D04" O R 4750 2050 60 
F18 "GPMI_WRN" O R 4750 2150 60 
F19 "GPMI_RDY1" O R 4750 2250 60 
F20 "GPMI_D00" O R 4750 1750 60 
F21 "GPMI_D01" O R 4750 1850 60 
F22 "GPMI_D02" O R 4750 1950 60 
F23 "GPMI_D03" O R 4750 1650 60 
F24 "USB_DM" B L 3150 3550 60 
F25 "USB_DP" B L 3150 3450 60 
F26 "USB_M3" B L 3150 4050 60 
F27 "USM_P3" B L 3150 3950 60 
F28 "USB_M4" B L 3150 3850 60 
F29 "USB_P4" B L 3150 3750 60 
$EndSheet
$EndSCHEMATC
