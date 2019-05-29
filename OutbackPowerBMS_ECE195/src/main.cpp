/*
    Filename: main.cpp
    Author: Usman J.
    Purpose: Firmware for Battery Monitoring System,
        designed and developed for Outback Power
    Description:
    Notes: Communication over SPI with the Maxim MAX1228 12-bit 10-channel ADC
    Credits: Developed using ARM mbed platform: https://os.mbed.com/
*/

// Libraries
#include <mbed.h>
#include <SDBlockDevice.h>

// Definitions
#define NUM_CHANNELS 9
#define RX_BUFFER_SIZE 4
#define MESSAGE_INIT 0x80
#define VOLTAGE_MEAS 0
#define CURRENT_MEAS 1
#define TEMP_MEAS 2

// Function prototypes
void setupADC(SPI *);                   // Setup and initialize SPI communication for ADC
void print_ADC_value(char *, int, int); // Console print hex values from ADC, for debugging
void sendCAN(CAN *, char *); // Send CAN messages
float extractFloat(char *); // Convert raw binary char values to floats
float powerInst(float, float);

// Global object declarations
SPI maximADC(PA_7, PA_6, PA_5, PA_4);       // SPI1_MOSI, SPI1_MISO, SPI1_SCLK, SPI1_SSEL
SDBlockDevice sd(PB_7, PC_2, PB_13, PB_12); // SPI2_MOSI, SPI2_MISO, SPI2_SCLK, SPI2_SSEL
CAN can1(PD_0, PD_1);                       // CAN1_RD, CAN1_TD

int main()
{
  setupADC(&maximADC);

  char conv_req = MESSAGE_INIT;
  char adc_response[RX_BUFFER_SIZE];
  float channel_value[NUM_CHANNELS];
  char can_message[8];
  float instant_power = 0;

  //float *powerCalcs = NULL; // Create a dynamic array for power calculations

  while (1)
  {
    maximADC.lock();
    for (int channel = 0; channel < NUM_CHANNELS; channel++) // Iterate through each ADC Channel (0-8)
    {
      maximADC.write(&conv_req, 2, adc_response, RX_BUFFER_SIZE);
      float adc_value_f = (extractFloat(adc_response)) / 1000;
      channel_value[channel] = adc_value_f;

      // weird debugging thing idk
      printf("%f", adc_value_f);
      printf("\n");

      // move to function for threading
      if (channel % 3 == 0) // Voltage measurements on CH 0, 3, 6
      {
        channel_value[channel] = 25.386f * channel_value[channel] - 5.2756f; // Voltage formatting
      }
      else if (channel % 3 == 1) // Current measurements on CH 1, 4, 7
      {
        channel_value[channel] = (channel_value[channel] - 2.5039f) * 200;             // Current formatting
        instant_power = powerInst(channel_value[channel - 1], channel_value[channel]); // Calculate instantaneus power
      }
      else if (channel % 3 == 2) // Temperature measurements on CH 2, 5, 8
      {
        channel_value[channel] = channel_value[channel]; // Temperature formatting (change to lookup/dictionary)
      }

      snprintf(can_message, 8, "%d%d%3.1f", channel, channel % 3, channel_value[channel]); // CH, TYP, VAL format for CAN string
      sendCAN(&can1,  can_message);
      // move to function for threading
      // if (can1.write(CANMessage(12, can_message, 8)))
      // {
      //   printf("%s\n", can_message);
      // }

      conv_req += 0x08; // increment to next channel
    }

    maximADC.unlock();
    conv_req = MESSAGE_INIT; // Reset to channel 0
    wait_ms(1000);
  }
}

void setupADC(SPI *maximADC)
{
  maximADC->lock();
  maximADC->format(8, 0);       // 8-bit frame, pol = phase
  maximADC->frequency(1000000); // 1MHz clock frequency, looks unstable if lower?

  // maximADC.write(0x10);        // Reset
  // maximADC.write(0x00);        // Buffer

  maximADC->write(0x76); // select setup register, external timing (CNVST), internal reference off (external single ended 5V), unipolar setup
  maximADC->write(0x00); // set all ADC channels to unipolar single-ended
  maximADC->unlock();
}

// Print voltage values
void print_ADC_value(char *adc_response, int size, int ch) // for debugging, print hex values
{
  printf("ch%d: ", ch);
  for (int i = 0; i < size; i++)
    printf("%d", adc_response[i]);
  printf("\n");
  return;
}

// Helper Function
float extractFloat(char *input)
{
  uint16_t adc_value = ((char)input[2] + ((char)input[1] << 8));
  float adc_value_f = 1.2 * adc_value;
  return adc_value_f;
}

float powerInst(float voltage, float current)
{
  float instant_power = voltage * current;
  return instant_power;
}

void sendCAN(CAN *can1, char * msg)
{
  if (can1->write(CANMessage(12, msg, 8)))
  {
    printf("%s\n", msg); //test
  }
}