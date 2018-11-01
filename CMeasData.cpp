/*
 * CMeasData.cpp
 *
 *  Created on: Aug 19, 2018
 *      Author: andy
 */

#include "CMeasData.h"
#include <FS.h>

using namespace std;

CMeasData::CMeasData(int i_size, const char *i_file)
{
  size = i_size;
  file = i_file;
}

CMeasData::~CMeasData()
{
}

void CMeasData::AddPoint(MeasPoint_t& p)
{
  if (data.size() >= size)
  {
    data.erase(data.begin());
  }
  data.push_back(p);
}

void CMeasData::GetMeasJSON(MeasType_t t, String& json)
{
  json = "\"ts\":[";

  for (DataIt_t it = data.begin(); it != data.end(); it++)
  {
    if (it != data.begin())
    {
      json += ",";
    }
    json += (*it).ts;
  }

  json += "],\"vals\":[";
  for (DataIt_t it = data.begin(); it != data.end(); it++)
  {
    if (it != data.begin())
    {
      json += ",";
    }
    switch(t)
    {
      case   MeasType_TEMP:
        json += (*it).temp / 10.0f;
        break;
      case MeasType_PRESS:
        json += (*it).press / 10.0f;
        break;
      case MeasType_HUM:
        json += (*it).hum;
        break;
      case MeasType_LUX:
        json += (*it).lux;
        break;
      case MeasType_BATT:
        json += (*it).vbatt;
        break;
    }
  }

  json += "]";
}

void CMeasData::CalcMeanMeas(MeasPoint_t& out)
{
  int32_t temp = 0, hum = 0, press = 0;

  for (DataIt_t it = data.begin(); it != data.end(); it++)
  {
    temp += (*it).temp;
    hum += (*it).hum;
    press += (*it).press;
  }

  out.hum = hum / data.size();
  out.press = press / data.size();
  out.temp = temp / data.size();
}

void CMeasData::Load()
{
  File f = SPIFFS.open(file, "r");
  if(f)
  {
    MeasPoint_t m;

    while(f.read((uint8_t *)&m, sizeof(m)) > 0)
    {
      AddPoint(m);
    }

    f.close();
  }
}

void CMeasData::Save()
{
  File f = SPIFFS.open(file, "w+");
  if(f)
  {
    MeasPoint_t m;

    for (DataIt_t it = data.begin(); it != data.end(); it++)
    {
      m = (*it);
      f.write((const uint8_t *)&m, sizeof(m));
    }

    f.close();
  }
}
