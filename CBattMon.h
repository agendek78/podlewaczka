/*
 * CBattMon.h
 *
 *  Created on: Oct 25, 2018
 *      Author: andy
 */

#ifndef CBATTMON_H_
#define CBATTMON_H_

#include <stdint.h>
#include <vector>

class CBattMon
{
  std::vector<int16_t> m_buffer;
  uint32_t  m_bufferLen;
  uint8_t   m_pin;

  float vbattGet();
  float calcMean();

public:
  CBattMon(uint8_t i_pin, uint32_t i_bufferLen);
  virtual ~CBattMon();

  /**
   * Returns current battery voltage and adds result
   * to the mean buffer.
   */
  int16_t DoMeas();

  int16_t GetMeanValue();

  uint8_t GetBattLevel();
  uint8_t GetBattLevel(int16_t i_battVolt);

};

#endif /* CBATTMON_H_ */
