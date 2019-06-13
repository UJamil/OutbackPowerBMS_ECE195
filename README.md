# OutbackPowerBMS_ECE195
Firmware for the ARM-based Battery Monitoring System developed for Outback Power

# Code Documentation

## Introduction

The Outback Power Battery Monitoring System Prototype designed by Seattle University’s ECE 19.5 features firmware for an ARM M4 microprocessor (ST Microelectronics STM32F439ZI) based on the ARM mbed libraries and operating system. The firmware controls the data flow of the system, in which it is responsible for gathering, calculating, storing, and transmitting the data. Refer to diagram A for a flowchart of the data.

## Battery.h Library

Since the data for each battery requires individual monitoring, a library called Battery was developed to utilize a Battery class object to hold the data relevant to each battery. The Battery object stores the voltage, current, temperature, and power which is updated by the ADC. Additionally, the battery object holds power calculations in a dynamic array, which is used to calculate the daily average and then logged to an SD card. The Battery library is defined in the header file Battery.h and implementation file Battery.cpp.
 
## Analog-to-Digital Conversion over SPI

To gather data, the microprocessor communicates over SPI protocol to a Maxim MAX1228 10-channel 12-bit Analog to Digital Converter (ADC) integrated circuit. Using the mbed SPI library, the ARM M4 is set to Master, while the ADC is set to Slave mode. The mbed SPI object is setup to use the ARM pins PA_7 for Master-out-Slave-in (MOSI), PA_6 for Master-in-Slave-out (MISO), PA_5 for the clock signal (SCLK), and PA_4 for the chip select (SSEL). 

### Setup

The MAX1228 expects two setup bytes, the first for setting up the registers, and the second for selecting differential measurements. In reference to the datasheet, the setup bytes are selected to be ”0x76” and “0x00”, meaning that the setup register is selected, external timing over the CNVST pin is enabled, the internal 4.096V reference disabled (an external 5V reference is used instead), and the last two bits are to setup the unipolar registers to single-ended (which is set in the second byte). 

### Conversion

To begin conversion, the hex value “0x80” was sent to the ADC, which selected channel 0. Each subsequent channel was selected by adding a hex byte (0x08) to the initial value of “0x80” so that it would cycle through each of the nine connected channels from “0x80”, “0x88”, “0x90”… and so on until “0xC0” where it would return to the first channel. 

## Storage and Data Logging

Storing the data also uses the SPI communication protocol to communicate with a flash memory IC or SD card. The mbed SDBlockDevice object was used to interface with and SD card to log the power calculations from each battery. Since SD cards also use SPI, the SDBlockDevice is initialized with the microcontroller’s pins PB_7 for MOSI, PC_2 for MISO, PB_13 for SCLK, and PB_12 for SSEL. 

## Communication

### CAN Bus

To transmit the data to other devices, the CAN bus was setup using the mbed CAN library. The CAN messages contain the data formatted in strings to represent the measurement data. Since the CAN message format limits the DATA to 8 bytes, the format was carefully selected to send only the most relevant data. The message format was designed to include the battery number (BAT), measurement type [TYP] and a floating-point of measurement data that included a sign and a value range from tenths to hundreds. For example, a typical message would be the string “21-250.4” which is representative of a current measurement of -250.4 A from battery 2. A battery is assigned a number, in this case 1 to 3 to the integer value batteryNumber. Each measurement is defined at the top of the main implementation file, with voltage measurements being type 0, current measurements 1, temperature measurements 2, and instantaneous power being 3. 

### RS-485 Serial

[to be implemented]
 
## Development Notes

The firmware for the project is hosted here on this GitHub private repository: 

### Development Environment

The software tools used to develop the firmware included Microsoft’s Visual Studio Code, a lightweight source-code editor, and the open-source integrated development environment (IDE) extension PlatformIO. This extension supports a variety of different boards and offers several libraries for development. This project utilizes ARM’s mbed framework since it is compatible with the STM32F439 microprocessor used on the development board and provides examples and functionality that makes adding project features easier. 
ARM also provides an online compiler as well as an offline IDE.  

### Development Board

During the initial prototyping phase, the ST Microelectronics STM32 Nucleo-144 board with STM32F429ZI MCU was used due to its features and functionality. These include all of the communication capabilities used, such as CAN, SPI, and RS-485. 
