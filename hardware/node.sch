EESchema Schematic File Version 4
LIBS:bluepill_water-cache
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 2
Title "RF Node"
Date "2018-12-14"
Rev ""
Comp ""
Comment1 "Generic layout for a node"
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L bluepill_breakouts:NRF24L01_Breakout-Bluepill-bluepill_ups-rescue-bluepill_ups-rescue U?
U 1 1 5C14BC20
P 2150 4150
AR Path="/5C14BC20" Ref="U?"  Part="1" 
AR Path="/5C14BAF9/5C14BC20" Ref="U1"  Part="1" 
F 0 "U1" H 2628 4128 50  0000 L CNN
F 1 "NRF24L01_Breakout" H 2628 4037 50  0000 L CNN
F 2 "SIP8" H 2300 4750 50  0001 L CIN
F 3 "http://www.nordicsemi.com/eng/content/download/2730/34105/file/nRF24L01_Product_Specification_v2_0.pdf" H 2150 4050 50  0001 C CNN
	1    2150 4150
	-1   0    0    -1  
$EndComp
Wire Wire Line
	4800 3950 2650 3950
Wire Wire Line
	2650 3850 4800 3850
Wire Wire Line
	2650 4050 4800 4050
Wire Wire Line
	4800 4150 2650 4150
Wire Wire Line
	2650 4250 4800 4250
Wire Wire Line
	2650 4350 4800 4350
$Comp
L power:GNDREF #PWR?
U 1 1 5C14BC2D
P 2150 4800
AR Path="/5C14BC2D" Ref="#PWR?"  Part="1" 
AR Path="/5C14BAF9/5C14BC2D" Ref="#PWR0102"  Part="1" 
F 0 "#PWR0102" H 2150 4550 50  0001 C CNN
F 1 "GNDREF" H 2155 4627 50  0001 C CNN
F 2 "" H 2150 4800 50  0001 C CNN
F 3 "" H 2150 4800 50  0001 C CNN
	1    2150 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	2150 4750 2150 4800
Text Label 3250 3850 0    50   ~ 0
SP1_CS
Text Label 3250 3950 0    50   ~ 0
SPI1_SCK
Text Label 3250 4050 0    50   ~ 0
SPI1_MISO
Text Label 3250 4150 0    50   ~ 0
SPI1_MOSI
Text Label 3250 4250 0    50   ~ 0
NRF_IRQ
Text Label 3250 4350 0    50   ~ 0
NRF_CE
$Comp
L power:+3.3V #PWR?
U 1 1 5C14BC4E
P 2150 3300
AR Path="/5C14BC4E" Ref="#PWR?"  Part="1" 
AR Path="/5C14BAF9/5C14BC4E" Ref="#PWR0106"  Part="1" 
F 0 "#PWR0106" H 2150 3150 50  0001 C CNN
F 1 "+3.3V" H 2165 3473 50  0000 C CNN
F 2 "" H 2150 3300 50  0001 C CNN
F 3 "" H 2150 3300 50  0001 C CNN
	1    2150 3300
	1    0    0    -1  
$EndComp
Wire Wire Line
	2150 3300 2150 3550
$Comp
L power:+3.3V #PWR?
U 1 1 5C14BC55
P 4400 4750
AR Path="/5C14BC55" Ref="#PWR?"  Part="1" 
AR Path="/5C14BAF9/5C14BC55" Ref="#PWR0107"  Part="1" 
F 0 "#PWR0107" H 4400 4600 50  0001 C CNN
F 1 "+3.3V" V 4415 4878 50  0000 L CNN
F 2 "" H 4400 4750 50  0001 C CNN
F 3 "" H 4400 4750 50  0001 C CNN
	1    4400 4750
	0    -1   -1   0   
$EndComp
$Comp
L power:GNDREF #PWR?
U 1 1 5C14BC5C
P 4650 5050
AR Path="/5C14BC5C" Ref="#PWR?"  Part="1" 
AR Path="/5C14BAF9/5C14BC5C" Ref="#PWR0108"  Part="1" 
F 0 "#PWR0108" H 4650 4800 50  0001 C CNN
F 1 "GNDREF" H 4655 4877 50  0001 C CNN
F 2 "" H 4650 5050 50  0001 C CNN
F 3 "" H 4650 5050 50  0001 C CNN
	1    4650 5050
	1    0    0    -1  
$EndComp
Wire Wire Line
	4800 4850 4650 4850
Wire Wire Line
	4650 4850 4650 4950
Wire Wire Line
	4800 4950 4650 4950
Connection ~ 4650 4950
Wire Wire Line
	4650 4950 4650 5050
Wire Wire Line
	4800 3450 4300 3450
Wire Wire Line
	4800 3550 4300 3550
Text HLabel 4300 3450 0    50   Input ~ 0
ADC_CH0
Text HLabel 4300 3550 0    50   Input ~ 0
ADC_CH1
NoConn ~ 6550 3750
NoConn ~ 6550 3850
NoConn ~ 6550 3950
NoConn ~ 6550 4050
NoConn ~ 6550 4150
NoConn ~ 6550 4250
NoConn ~ 6550 4550
NoConn ~ 6550 4650
NoConn ~ 6550 4750
NoConn ~ 6550 4850
NoConn ~ 6550 4950
NoConn ~ 4800 4650
NoConn ~ 4800 4550
NoConn ~ 4800 4450
NoConn ~ 4800 3750
NoConn ~ 4800 3650
NoConn ~ 4800 3350
NoConn ~ 4800 3250
NoConn ~ 4800 3150
NoConn ~ 4800 3050
NoConn ~ 5600 1850
NoConn ~ 5700 1850
Wire Wire Line
	6550 4350 7400 4350
Wire Wire Line
	6550 4450 7400 4450
Text HLabel 7400 4450 2    50   Output ~ 0
UART1_TX
$Comp
L bluepill_breakouts:BluePill_STM32F103C U?
U 1 1 5C14BC04
P 5650 3750
AR Path="/5C14BC04" Ref="U?"  Part="1" 
AR Path="/5C14BAF9/5C14BC04" Ref="U2"  Part="1" 
F 0 "U2" H 5675 2278 50  0000 C CNN
F 1 "BluePill_STM32F103C" H 5675 2187 50  0000 C CNN
F 2 "DIP40" H 5700 2150 50  0001 C CNN
F 3 "www.rogerclark.net" H 5650 2250 50  0001 C CNN
	1    5650 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	4800 4750 4400 4750
NoConn ~ 6550 3050
Wire Wire Line
	6550 3250 6900 3250
Wire Wire Line
	6900 3250 6900 2300
$Comp
L power:+5V #PWR0123
U 1 1 5C13D36B
P 6900 2300
F 0 "#PWR0123" H 6900 2150 50  0001 C CNN
F 1 "+5V" H 6915 2473 50  0000 C CNN
F 2 "" H 6900 2300 50  0001 C CNN
F 3 "" H 6900 2300 50  0001 C CNN
	1    6900 2300
	1    0    0    -1  
$EndComp
$Comp
L power:GNDREF #PWR?
U 1 1 5C16DC3A
P 6700 3150
AR Path="/5C16DC3A" Ref="#PWR?"  Part="1" 
AR Path="/5C14BAF9/5C16DC3A" Ref="#PWR?"  Part="1" 
F 0 "#PWR?" H 6700 2900 50  0001 C CNN
F 1 "GNDREF" H 6705 2977 50  0001 C CNN
F 2 "" H 6700 3150 50  0001 C CNN
F 3 "" H 6700 3150 50  0001 C CNN
	1    6700 3150
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6700 3150 6550 3150
Text HLabel 7400 4350 2    50   Input ~ 0
UART1_RX
NoConn ~ 6550 3550
NoConn ~ 6550 3650
Wire Wire Line
	6550 3350 7650 3350
Wire Wire Line
	6550 3450 7650 3450
Text HLabel 7650 3350 2    50   Output ~ 0
PB9
Text HLabel 7650 3450 2    50   Output ~ 0
PB8
$EndSCHEMATC
