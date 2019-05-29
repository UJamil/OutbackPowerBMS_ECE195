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

// Function prototypes
void setupADC(SPI *); // Setup and initialize SPI communication for ADC
void print_ADC_value(char *, int, int);   // Console print hex values from ADC, for debugging
float extract_float(char *); // Convert raw binary char values to floats

// Global object declarations
SPI maximADC(PA_7, PA_6, PA_5, PA_4); // SPI1_MOSI, SPI1_MISO, SPI1_SCLK, SPI1_SSEL
SDBlockDevice sd(PA_7, PA_6, PA_5, PA_4); // SPI2_MOSI, SPI2_MISO, SPI2_SCLK, SPI2_SSEL
CAN can1(PD_0, PD_1); // CAN1_RD, CAN1_TD

int main()
{
  setupADC(&maximADC);

  char conv_req = MESSAGE_INIT;
  char adc_response[RX_BUFFER_SIZE];
  float channel_value[NUM_CHANNELS];
  char can_message[8];

  while (1)
  {
    maximADC.lock();
    for (int i = 0; i < NUM_CHANNELS; i++) // Iterate through each ADC Channel
    {
      maximADC.write(&conv_req, 2, adc_response, RX_BUFFER_SIZE);
      float adc_value_f = (extract_float(adc_response)) / 1000;
      channel_value[i] = adc_value_f;

      printf("%f", adc_value_f);
      printf("\n");

      conv_req += 0x08; // increment to next channel
    }

    for (int j = 0; j < NUM_CHANNELS; j += 3) // Voltage Format
    {
      channel_value[j] = 25.386f * channel_value[j] - 5.2756f;
    }
    for (int k = 1; k < NUM_CHANNELS; k += 3) // Current Format
    {
      channel_value[k] = (channel_value[k] - 2.5039f) * 200;
    }
    for (int l = 2; l < NUM_CHANNELS; l += 3) // Temperature Format
    {
      channel_value[l] = channel_value[l]; // TODO: Change
    }

    int counter = 0;
    for (int i = 1; i <= 3; i++)
    {
      for (int j = 1; j <= 3; j++)
      {
        snprintf(can_message, 8, "%d%d%3.1f", i, j, channel_value[counter]);
        if (can1.write(CANMessage(12, can_message, 8)))
        {
          printf("%s\n", can_message);
        }
        counter++;
        if (counter == 9)
        {
          counter = 0;
        }
        wait_ms(20);
      }
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

  maximADC->write(0x76);  // select setup register, external timing (CNVST), internal reference off (external single ended 5V), unipolar setup
  maximADC->write(0x00);  // set all ADC channels to unipolar single-ended
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
float extract_float(char *input)
{
  uint16_t adc_value = ((char)input[2] + ((char)input[1] << 8));
  float adc_value_f = 1.2 * adc_value;
  return adc_value_f;
}

