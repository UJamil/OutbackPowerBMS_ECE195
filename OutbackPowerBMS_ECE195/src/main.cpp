/*
    Filename: main.cpp
    Author: Usman Jamil
    Collaborators: Abdulaziz Alrakaf
    Purpose: Firmware for Battery Monitoring System,
        designed and developed for Outback Power
    Description:
    Notes: Communication over SPI with the Maxim MAX1228 12-bit 10-channel ADC
    Credits: Developed using ARM mbed platform: https://os.mbed.com/
*/

// Libraries
#include <mbed.h>
#include <SDBlockDevice.h>
#include "Battery.h"

// Definitions
#define NUM_CHANNELS 9
#define RX_BUFFER_SIZE 4
#define MESSAGE_INIT 0x80
#define VOLTAGE_MEAS 0
#define CURRENT_MEAS 1
#define TEMP_MEAS 2
#define INST_POWER 3
#define ADC_MULTIPLIER 1.2f // 5V ref / 2^12 bits = 5V /4096 ~= 1.2mV

// Function prototypes
void setupADC(SPI *);                   // Setup and initialize SPI communication for ADC
void print_ADC_value(char *, int, int); // Console print hex values from ADC, for debugging
void sendCAN(CAN *, Battery *);         // Send CAN messages
void formatData(float *, int);          // Format raw floats to voltage, current, temperature
float extractFloat(char *);             // Convert raw binary char values to floats

// Global object declarations
SPI maximADC(PA_7, PA_6, PA_5, PA_4);       // SPI1_MOSI, SPI1_MISO, SPI1_SCLK, SPI1_SSEL
SDBlockDevice sd(PB_7, PC_2, PB_13, PB_12); // SPI2_MOSI, SPI2_MISO, SPI2_SCLK, SPI2_SSEL
CAN can1(PD_0, PD_1);                       // CAN1_RD, CAN1_TD
Battery BAT_1(1), BAT_2(2), BAT_3(3);       // Battery objects

Serial pc(USBTX, USBRX, "debug", 115200);

Battery Batteries[3] = {BAT_1, BAT_2, BAT_3};

int main()
{
  // Setup
  setupADC(&maximADC);

  // Declare char arrays
  char adc_response[RX_BUFFER_SIZE]; // SPI Read Buffer
  float ADC_meas[NUM_CHANNELS];      // Array of ADC measurements

  // Initialize variables
  char conv_req = MESSAGE_INIT; // Set conversion request to 0x80 for channel 0
  float adc_value_f = 0.0f;
  int channelNum = 0;
  int battIndex = 0;

  //float *powerCalcs = NULL; // Create a dynamic array for power calculations

  while (1)
  {
    maximADC.lock();
    for (channelNum = 0; channelNum < NUM_CHANNELS; channelNum++) // Iterate through each ADC Channel (0-8)
    {
      maximADC.write(&conv_req, 2, adc_response, RX_BUFFER_SIZE);
      adc_value_f = (extractFloat(adc_response)) / 1000;
      ADC_meas[channelNum] = adc_value_f;
      formatData(ADC_meas, channelNum);

      switch (channelNum)
      {
      case 0:
        Batteries[0].setVoltage(ADC_meas[channelNum]);
        break;
      case 1:
        Batteries[0].setCurrent(ADC_meas[channelNum]);
        break;
      case 2:
        Batteries[0].setTemp(ADC_meas[channelNum]);
        break;
      case 3:
        Batteries[1].setVoltage(ADC_meas[channelNum]);
        break;
      case 4:
        Batteries[1].setCurrent(ADC_meas[channelNum]);
        break;
      case 5:
        Batteries[1].setTemp(ADC_meas[channelNum]);
        break;
      case 6:
        Batteries[2].setVoltage(ADC_meas[channelNum]);
        break;
      case 7:
        Batteries[2].setCurrent(ADC_meas[channelNum]);
        break;
      case 8:
        Batteries[2].setTemp(ADC_meas[channelNum]);
        break;
      }

      conv_req += 0x08; // increment to next channel
    }
    maximADC.unlock();

    // printf("%3.1f", BAT_1.getVoltage());

    for (battIndex = 0; battIndex <= 2; battIndex++)
    {
      sendCAN(&can1, &Batteries[battIndex]);
    }

    conv_req = MESSAGE_INIT; // Reset to channel 0
    wait_ms(100);
  }
}

void setupADC(SPI *maximADC)
{
  // Setup ADC registers over SPI
  maximADC->lock();

  maximADC->format(8, 0);       // 8-bit frame, pol = phase
  maximADC->frequency(1000000); // 1MHz clock frequency, looks unstable if lower?
  maximADC->write(0x76);        // select setup register, external timing (CNVST), internal reference off (external single ended 5V), unipolar setup
  maximADC->write(0x00);        // set all ADC channels to unipolar single-ended

  maximADC->unlock();
}

// Print voltage values (for debugging)
void print_ADC_value(char *adc_response, int size, int ch)
{
  printf("ch%d: ", ch);
  for (int i = 0; i < size; i++)
    printf("%d", adc_response[i]);
  printf("\n");
  return;
}

// Helper Function for extracting floats from char
float extractFloat(char *input)
{
  uint16_t adc_value = ((char)input[2] + ((char)input[1] << 8));
  float adc_value_f = ADC_MULTIPLIER * adc_value; 
  return adc_value_f;
}

// Send CAN messages
void sendCAN(CAN *can1, Battery *bat)
{
  char can_message[8];

  // BAT[1], TYP[1], VAL[6] format for CAN string of [8] bytes

  snprintf(can_message, 8, "%d%d%3.1f", bat->getBattNum(), VOLTAGE_MEAS, bat->getVoltage()); // Send Voltage
  printf(can_message);
  can1->write(CANMessage(12, can_message, 8));
  wait_ms(20);

  snprintf(can_message, 8, "%d%d%3.1f", bat->getBattNum(), CURRENT_MEAS, bat->getCurrent()); // Send Current
  can1->write(CANMessage(12, can_message, 8));
  wait_ms(20);

  snprintf(can_message, 8, "%d%d%3.1f", bat->getBattNum(), TEMP_MEAS, bat->getTemp()); // Send Temperature
  can1->write(CANMessage(12, can_message, 8));
  wait_ms(20);

  snprintf(can_message, 8, "%d%d%3.1f", bat->getBattNum(), INST_POWER, bat->getPower()); // Send Power
  can1->write(CANMessage(12, can_message, 8));
  wait_ms(20);
}

// Format data
void formatData(float *channelData, int channelNum)
{
  if (channelNum % 3 == VOLTAGE_MEAS) // Voltage measurements on CH 0, 3, 6
  {
    channelData[channelNum] = 25.386f * channelData[channelNum] - 5.2756f; // Voltage formatting
  }
  else if (channelNum % 3 == CURRENT_MEAS) // Current measurements on CH 1, 4, 7
  {
    channelData[channelNum] = (channelData[channelNum] - 2.5039f) * 200; // Current formatting
  }
  else if (channelNum % 3 == TEMP_MEAS) // Temperature measurements on CH 2, 5, 8
  {
    channelData[channelNum] = channelData[channelNum]; // Temperature formatting (change to lookup/dictionary)
  }
}