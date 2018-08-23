/*
 * CIrrigation.cpp
 *
 *  Created on: Aug 19, 2018
 *      Author: andy
 */

#include "CIrrigation.h"

#include <Arduino.h>

#define TIME_CHECK_RANGE  10UL
#define TIME_IN_RANGE(time, checkTime)  ((time) >= (checkTime)) && ((time) <= (checkTime + TIME_CHECK_RANGE))
#define PODLEWANIE_LEN  (1 * MINUTE)
#define IRR_TANK_USAGE  (5)

using namespace std;

CIrrigation::CIrrigation(int tankCount)
{
  irrTankStatus = vector<uint8_t>(tankCount);
  irrCurrState = IrrState_WAITING;
  irrCurrTime = 0;
  irrCurrCh = 0;
  for(int i = 0; i < tankCount; i++)
  {
    irrTankStatus[i] = 100;
  }
}

CIrrigation::~CIrrigation()
{
}

inline String numeLead(int num)
{
  if (num < 10)
  {
    return "0"+String(num);
  }
  else
  {
    return String(num);
  }
}

void CIrrigation::GetStatusJSON(String& json)
{
  json += "{\"ts\":[";

  for(uint32_t i = 0; i < irrTankStatus.size(); i++)
  {
    if (i > 0)
    {
      json += ",";
    }
    json += irrTankStatus[i];
  }
  json += "],\"g\":[";

  for(IrrEventIt_t it = irrEvents.begin(); it != irrEvents.end(); it++)
  {
    if (it != irrEvents.begin())
    {
      json += ",";
    }
    json = json + "{\"t\":\"" + numeLead((*it).time / (HOUR));
    json = json + ":" + numeLead(((*it).time % HOUR) / MINUTE);
    json = json + "\",\"st\":" + (int)(*it).state;
    json += ",\"en\":[";
    for(uint32_t i = 0; i < (*it).chEn.size(); i++)
    {
      if (i > 0)
      {
        json += ",";
      }
      json += (*it).chEn[i] ? 1 : 0;
    }
    json += "]}";
  }
  json += "]}";

  Serial.println(json);
}

void CIrrigation::AddChannel(uint8_t i_out, uint8_t i_tank)
{
  IrrChannel_t ch = {i_out, i_tank, IrrState_WAITING};
  irrChannels.push_back(ch);
  digitalWrite(i_out, HIGH);
}

void CIrrigation::AddTime(uint32_t t)
{
  IrrEvent_t  ev = {t, IrrState_WAITING, vector<bool>(irrChannels.size())};

  for(uint32_t i = 0; i < ev.chEn.size(); i++)
  {
    ev.chEn[i] = true;
  }
  irrEvents.push_back(ev);
}

void CIrrigation::startCh(uint8_t chIdx)
{
  digitalWrite(irrChannels[chIdx].output, LOW);
  irrChannels[chIdx].state = IrrState_ONGOING;
  Serial.print("Rozpoczęcie podlewania na kanale ");Serial.println(chIdx);
}

void CIrrigation::stopCh(uint8_t chIdx)
{
  digitalWrite(irrChannels[chIdx].output, HIGH);
  irrChannels[chIdx].state = IrrState_WAITING;
  Serial.print("Zakończenie podlewania na kanale ");Serial.println(chIdx);
}

void CIrrigation::updateTankState(uint8_t i_tank, uint8_t i_val)
{
  if (irrTankStatus[i_tank] >= i_val)
  {
    irrTankStatus[i_tank] -= i_val;
  }
}

void CIrrigation::irrigate(uint32_t i_time)
{
  for(IrrEventIt_t it = irrEvents.begin(); it != irrEvents.end(); it++)
  {
    if ((*it).state == IrrState_ONGOING)
    {
      while(1)
      {
        if ((*it).chEn[irrCurrCh] == true)
        {
          IrrState_t state = irrChannels[irrCurrCh].state;

          if (state == IrrState_WAITING)
          {
            irrCurrTime = i_time;
            startCh(irrCurrCh);
            break;
          }
          else if (state == IrrState_ONGOING)
          {
            if ((irrCurrTime + PODLEWANIE_LEN) <= i_time)
            {
              stopCh(irrCurrCh);
              updateTankState(irrChannels[irrCurrCh].tankIdx, IRR_TANK_USAGE);
            }
            else
            {
              break;
            }
          }
        }

        irrCurrCh++;
        if (irrCurrCh >= irrChannels.size())
        {
          (*it).state = IrrState_DONE;
          irrCurrCh = 0;
          if ((it + 1) == irrEvents.end())
          {
            irrCurrState = IrrState_DONE;
          }
          else
          {
            irrCurrState = IrrState_WAITING;
          }
          break;
        }
      }
      break;
    }
  }
}

void CIrrigation::SetEvChannel(uint32_t i_ev, uint32_t i_ch, bool i_en)
{
  irrEvents[i_ev].chEn[i_ch] = i_en;
}

void CIrrigation::DoWork(uint32_t i_currTime)
{
  if (i_currTime < 60 && irrCurrState == IrrState_DONE)
  {
    //reset
    for(IrrEventIt_t it = irrEvents.begin(); it != irrEvents.end(); it++)
    {
      (*it).state = IrrState_WAITING;
    }
    irrCurrState = IrrState_WAITING;
    return;
  }

  if (irrCurrState == IrrState_WAITING)
  {
    for(IrrEventIt_t it = irrEvents.begin(); it != irrEvents.end(); it++)
    {
      uint32_t evTime = (*it).time;

      if (TIME_IN_RANGE(i_currTime, evTime))
      {
        (*it).state = IrrState_ONGOING;
        irrCurrState = IrrState_ONGOING;
        irrCurrTime = i_currTime;
        irrCurrCh = 0;
        Serial.print("Podlewanie dla ev: "); Serial.println((*it).time);
        irrigate(i_currTime);
        break;
      }
    }
  }
  else
  {
    irrigate(i_currTime);
  }
}
