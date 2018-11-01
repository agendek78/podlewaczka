/*
 * CBattMon.cpp
 *
 *  Created on: Oct 25, 2018
 *      Author: andy
 */

#include "CBattMon.h"
#include <Arduino.h>

#define DIV_R1  (1010.0)
#define DIV_R2  (300.0)
#define BATT_MIN_VOLT 3000
#define BATT_MAX_VOLT 4200

using namespace std;

static uint8_t sigmoidal(uint16_t voltage, uint16_t minVoltage, uint16_t maxVoltage)
{
  uint8_t result = 105 - (105 / (1 + pow(1.724 * (voltage - minVoltage)/(maxVoltage - minVoltage), 5.5)));
  return result >= 100 ? 100 : result;
}

float CBattMon::vbattGet(void)
{
  float ret = analogRead(m_pin);

  Serial.print("adc read: "); Serial.println(ret);

  return (ret * 0.947 * (DIV_R1 + DIV_R2) / DIV_R2) / 1024.0;
}

float CBattMon::calcMean()
{
  float ret = 0.0f;

  for(auto i : m_buffer)
  {
    ret += i;
  }

  return ret / m_buffer.size();
}

CBattMon::CBattMon(uint8_t i_pin, uint32_t i_bufferLen) :
  m_bufferLen(i_bufferLen), m_pin(i_pin)
{
}

CBattMon::~CBattMon()
{
}

int16_t CBattMon::DoMeas()
{
  float currVal = vbattGet();

  m_buffer.push_back((int16_t)(currVal * 1000.0));

  if (m_buffer.size() > m_bufferLen)
  {
    m_buffer.erase(m_buffer.begin());
  }

  return currVal;
}

int16_t CBattMon::GetMeanValue()
{
  return calcMean();
}

uint8_t CBattMon::GetBattLevel()
{
  return GetBattLevel(calcMean());
}

uint8_t CBattMon::GetBattLevel(int16_t i_battVolt)
{
  return sigmoidal(i_battVolt, BATT_MIN_VOLT, BATT_MAX_VOLT);
}
