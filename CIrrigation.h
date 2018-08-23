/*
 * CIrrigation.h
 *
 *  Created on: Aug 19, 2018
 *      Author: andy
 */

#ifndef CIRRIGATION_H_
#define CIRRIGATION_H_

#include <vector>
#include <stdint.h>
#include <WString.h>

#define MINUTE  60UL
#define HOUR    (60UL * MINUTE)
#define DAY     (24UL * HOUR)
#define MAKE_TIME(h, m, s)  (h * HOUR + m * MINUTE + s)

typedef enum
{
  IrrState_WAITING,
  IrrState_DONE,
  IrrState_ONGOING
} IrrState_t;

typedef struct
{
  uint32_t          time;
  IrrState_t        state;
  std::vector<bool> chEn;
} IrrEvent_t;

typedef struct
{
  uint8_t     output;
  uint8_t     tankIdx;
  IrrState_t  state;
} IrrChannel_t;

typedef std::vector<IrrEvent_t>::iterator           IrrEventIt_t;
typedef std::vector<IrrChannel_t>::const_iterator   IrrChannelIt_t;

class CIrrigation
{
  std::vector<IrrEvent_t>   irrEvents;
  std::vector<IrrChannel_t> irrChannels;
  std::vector<uint8_t>      irrTankStatus;
  IrrState_t                irrCurrState;
  uint32_t                  irrCurrTime;
  uint32_t                  irrCurrCh;

  void irrigate(uint32_t i_time);
  void startCh(uint8_t chIdx);
  void stopCh(uint8_t chIdx);

  void updateTankState(uint8_t i_tank, uint8_t i_val);
public:
  CIrrigation(int tankCount);
  virtual ~CIrrigation();

  void AddChannel(uint8_t i_out, uint8_t i_tank);
  void AddTime(uint32_t t);

  uint32_t GetEventCount() { return irrEvents.size();};
  uint32_t GetChannelCount() { return irrChannels.size();};

  void SetEvChannel(uint32_t i_ev, uint32_t i_ch, bool i_en);

  void DoWork(uint32_t i_currTime);

  void GetStatusJSON(String& json);
};

#endif /* CIRRIGATION_H_ */
