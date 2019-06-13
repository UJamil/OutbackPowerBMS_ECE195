

#include <Battery.h>

Battery::Battery(uint16_t battNum)
{
    voltage = 0.0f;
    current = 0.0f;
    temperature = 0.0f;
    power = 0.0f;
    batteryNumber = battNum;
}

// Battery::Battery(const Battery &)
// {
// }

// Battery::~Battery()
// {
// }

void Battery::setVoltage(float V)
{
    voltage = V;
}
void Battery::setCurrent(float C)
{
    current = C;
}
void Battery::setTemp(float T)
{
    temperature = T;
}
void Battery::setBattNum(uint16_t battNum)
{
    batteryNumber = battNum;
}

float Battery::getVoltage()
{
    return voltage;
}
float Battery::getCurrent()
{
    return current;
}
float Battery::getTemp()
{
    return temperature;
}
float Battery::getPower()
{
    power = voltage * current;
    return power;
}
uint16_t Battery::getBattNum()
{
    return batteryNumber;
}