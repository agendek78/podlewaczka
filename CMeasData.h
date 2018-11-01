/*
 * CMeasData.h
 *
 *  Created on: Aug 19, 2018
 *      Author: andy
 */

#ifndef CMEASDATA_H_
#define CMEASDATA_H_

#include <stdint.h>
#include <vector>
#include <WString.h>

typedef enum
{
  MeasType_TEMP,
  MeasType_PRESS,
  MeasType_HUM,
  MeasType_LUX,
  MeasType_BATT
} MeasType_t;

typedef struct __attribute__((packed))
{
  uint32_t ts;
  int16_t  temp;
  int16_t  vbatt;
  uint16_t press;
  uint32_t lux;
  uint8_t  hum;
} MeasPoint_t;

typedef std::vector<MeasPoint_t>::const_iterator  DataIt_t;

class CMeasData
{
  uint32_t size;
  std::vector<MeasPoint_t>  data;
  String    file;

public:
  CMeasData(int size, const char *i_file);
  virtual ~CMeasData();

  void AddPoint(MeasPoint_t& p);
  void GetMeasJSON(MeasType_t t, String& json);
  void CalcMeanMeas(MeasPoint_t& out);

  void Load();
  void Save();
};

#endif /* CMEASDATA_H_ */
