

#ifndef BATTERY_H_
#define BATTERY_H_

#include <stdint.h>

class Battery
{
public:
    // Set Methods
    void setVoltage(float);
    void setCurrent(float);
    void setTemp(float);
    void setPower(float);
    void setBattNum(uint16_t);

    // Get Methods
    float getVoltage();
    float getCurrent();
    float getTemp();
    float getPower();
    uint16_t getBattNum();

    Battery(uint16_t);        // Constructor
    // Battery(const Battery &); // Copy Constructor
    // ~Battery(void);               // Destructor

private:
    float voltage,
        current,
        temperature,
        power;
    uint16_t batteryNumber;
    
};

#endif