/*****************************************************************************
  RPR-0521RS.cpp

 Copyright (c) 2016 ROHM Co.,Ltd.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
******************************************************************************/
//#include <avr/pgmspace.h>
//#include <Arduino.h>
//#include <Wire.h>

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "../syssvc/serial.h"
#include "../syssvc/syslog.h"
#include "kernel_cfg.h"

#include "i2c_api.h"
//#include "i2c_setting.h"

#include <RPR-0521RS.h>


RPR0521RS::RPR0521RS(PinName _sda, PinName _scl) :
sda(_sda), scl(_scl), mbed_i2c(_sda, _scl)
{
}

int RPR0521RS::init(void)
{
	unsigned char rc;
  unsigned char reg;
  unsigned char index;
  unsigned char als_gain_table[] = {1, 2, 64, 128};
  unsigned short als_meas_time_table[] = {0,0,0,0,0,100,100,100,100,100,400,400,50,0,0,0};
  char cmd[2];
   char buf[2];
   int result;

   //wire の初期化相当の処理
   i2c_init(&_i2c_t, sda, scl);
   i2c_slave_mode(&_i2c_t, false);

//  rc = read(RPR0521RS_SYSTEM_CONTROL, &reg, sizeof(reg));
    result = read2(RPR0521RS_SYSTEM_CONTROL, &reg, sizeof(reg));
    //    cmd[0] = RPR0521RS_SYSTEM_CONTROL;
//  ret = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, cmd, 1, 1);
//  ret = mbed_i2c.read(RPR0521RS_DEVICE_ADDRESS << 1, (char *)&reg, 1);
//  reg = (unsigned char)buf[0];
  if (result < 0) {
//    Serial.println(F("Can't access RPR0521RS"));
	syslog(LOG_ERROR, "Can't access RPR0521RS");
    return (result);
  }else{
		syslog(LOG_ERROR, "Can access RPR0521RS");
  }
  syslog(LOG_ERROR, "reg raw value:%d", reg);
  reg &= 0x3F;
//  Serial.print(F("RPR0521RS Part ID Register Value = 0x"));
//  Serial.println(reg, HEX);
  syslog(LOG_ERROR, "RPR0521RS Part ID Register Value = 0x%x", reg);

  if (reg != RPR0521RS_PART_ID_VAL) {
//    Serial.println(F("Can't find RPR0521RS"));
		syslog(LOG_ERROR, "Can't find RPR0521RS");
    return (result);
  }

//  rc = read(RPR0521RS_MANUFACT_ID, &reg, sizeof(reg));
  result = read2(RPR0521RS_MANUFACT_ID, &reg, sizeof(reg));
//  cmd[0] = RPR0521RS_MANUFACT_ID;
//  ret = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, cmd, 1, 1);
//  ret = mbed_i2c.read(RPR0521RS_DEVICE_ADDRESS << 1, buf, 1);
  if (result < 0) {
//    Serial.println(F("Can't access RPR0521RS"));
		syslog(LOG_ERROR, "Can't access RPR0521RS");
    return (result);
  }
//  reg = (unsigned char)buf[0];
//  Serial.print(F("RPR0521RS MANUFACT_ID Register Value = 0x"));
//  Serial.println(reg, HEX);
  syslog(LOG_ERROR, "RPR0521RS MANUFACT_ID Register Value = 0x%x", reg);

  if (reg != RPR0521RS_MANUFACT_ID_VAL) {
//    Serial.println(F("Can't find RPR0521RS"));
		syslog(LOG_ERROR, "Can't find RPR0521RS");
    return (result);
  }

//  reg = RPR0521RS_ALS_PS_CONTROL_VAL;
//  cmd[0] = RPR0521RS_ALS_PS_CONTROL;
//  cmd[1] = RPR0521RS_ALS_PS_CONTROL_VAL;
//  rc = write(RPR0521RS_ALS_PS_CONTROL, &reg, sizeof(reg));
  result = write2(RPR0521RS_ALS_PS_CONTROL, &reg, sizeof(reg));
//  result = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, cmd, 2, 0);
  if (result < 0) {
//    Serial.println(F("Can't write RPR0521RS ALS_PS_CONTROL register"));
	syslog(LOG_ERROR, "Can't write RPR0521RS ALS_PS_CONTROL register");
    return (result);
  }

//  rc = read(RPR0521RS_PS_CONTROL, &reg, sizeof(reg));
    result = read2(RPR0521RS_PS_CONTROL, &reg, sizeof(reg));
//  cmd[0] = RPR0521RS_PS_CONTROL;
//  result = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, cmd, 1, 1);
//  result = mbed_i2c.read(RPR0521RS_DEVICE_ADDRESS << 1, buf, 1);
  if (result != 0) {
//    Serial.println(F("Can't read RPR0521RS PS_CONTROL register"));
		syslog(LOG_ERROR, "Can't read RPR0521RS PS_CONTROL register");
    return (result);
  }

  reg |= RPR0521RS_PS_CONTROL_VAL;
//  rc = write(RPR0521RS_PS_CONTROL, &reg, sizeof(reg));
    result = write2(RPR0521RS_PS_CONTROL, &reg, sizeof(reg));
//  cmd[0] = RPR0521RS_PS_CONTROL;
//  cmd[0] |= RPR0521RS_PS_CONTROL_VAL;
//  result = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, cmd, 2, 0);
//  result = mbed_i2c.read(RPR0521RS_DEVICE_ADDRESS << 1, buf, 1);
  if (result != 0) {
//    Serial.println(F("Can't write RPR0521RS PS_CONTROL register"));
		syslog(LOG_ERROR, "Can't write RPR0521RS PS_CONTROL register");
  }

  reg = RPR0521RS_MODE_CONTROL_VAL;
//  rc = write(RPR0521RS_MODE_CONTROL, &reg, sizeof(reg));
    result = write2(RPR0521RS_MODE_CONTROL, &reg, sizeof(reg));
//    cmd[0] = RPR0521RS_MODE_CONTROL;
//    cmd[1] = RPR0521RS_MODE_CONTROL_VAL;
//    result = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, cmd, 2, 0);
  if (result != 0) {
//    Serial.println(F("Can't write RPR0521RS MODE CONTROL register"));
		syslog(LOG_ERROR, "Can't write RPR0521RS MODE CONTROL register");
    return (result);
  }

  reg = RPR0521RS_ALS_PS_CONTROL_VAL;
  index = (reg >> 4) & 0x03;
  _als_data0_gain = als_gain_table[index];
  index = (reg >> 2) & 0x03;
  _als_data1_gain = als_gain_table[index];

  index = RPR0521RS_MODE_CONTROL_VAL & 0x0F;
  _als_measure_time = als_meas_time_table[index];

  //設定値のチェック
//#define RPR0521RS_MODE_CONTROL                     (0x41)
//#define RPR0521RS_ALS_PS_CONTROL                   (0x42)
//#define RPR0521RS_PS_CONTROL                       (0x43)
//  cmd[0] = RPR0521RS_MODE_CONTROL;
  result = read2(RPR0521RS_MODE_CONTROL, &reg, sizeof(reg));
//  result = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, cmd, 1, 0);
//  result = mbed_i2c.read(RPR0521RS_DEVICE_ADDRESS << 1, buf, 1);
  syslog(LOG_ERROR, "RPR0521RS_MODE_CONTROL=%x", reg);
//  cmd[0] = RPR0521RS_ALS_PS_CONTROL;
  result = read2(RPR0521RS_ALS_PS_CONTROL, &reg, sizeof(reg));
//  result = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, cmd, 1, 0);
//  result = mbed_i2c.read(RPR0521RS_DEVICE_ADDRESS << 1, buf, 1);
  syslog(LOG_ERROR, "RPR0521RS_ALS_PS_CONTROL=%x", reg);
//  cmd[0] = RPR0521RS_PS_CONTROL;
  result = read2(RPR0521RS_PS_CONTROL, &reg, sizeof(reg));
//  result = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, cmd, 1, 0);
//  result = mbed_i2c.read(RPR0521RS_DEVICE_ADDRESS << 1, buf, 1);
  syslog(LOG_ERROR, "RPR0521RS_PS_CONTROL=%x", reg);


syslog(LOG_ERROR, "init ok!");
  return result;
}

unsigned char RPR0521RS::get_rawpsalsval(unsigned char *data)
{
	unsigned char rc;
	char cmd[2];
	char buf[6];

	syslog(LOG_ERROR, "get_rawpsalsval called");
  cmd[0] = RPR0521RS_PS_DATA_LSB;
  rc = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, cmd, 1, 1);
  rc = mbed_i2c.read(RPR0521RS_DEVICE_ADDRESS << 1, buf, 6);
  data[0] = (unsigned char)buf[0];
  data[1] = (unsigned char)buf[1];
  data[2] = (unsigned char)buf[2];
  data[3] = (unsigned char)buf[3];
  data[4] = (unsigned char)buf[4];
  data[5] = (unsigned char)buf[5];
syslog(LOG_ERROR, "get_rawpsalsval val=%d, %d, %d, %d, %d, %d", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
//  rc = read(RPR0521RS_PS_DATA_LSB, data, 6);
  if (rc != 0) {
//    Serial.println(F("Can't get RPR0521RS PS/ALS_DATA value"));
		syslog(LOG_ERROR, "Can't get RPR0521RS PS/ALS_DATA value");
  }else{
		syslog(LOG_ERROR, "get_rawpsalsval ok");
  }

  return (rc);
}

unsigned char RPR0521RS::get_psalsval(unsigned short *ps, float *als)
{
  unsigned char rc;
  unsigned char val[6];
  unsigned short rawps;
  unsigned short rawals[2];

	syslog(LOG_ERROR, "get_psalsval called");
  rc = get_rawpsalsval(val);
  if (rc != 0) {
    return (rc);
  }

  rawps     = ((unsigned short)val[1] << 8) | val[0];
  rawals[0] = ((unsigned short)val[3] << 8) | val[2];
  rawals[1] = ((unsigned short)val[5] << 8) | val[4];

  *ps  = rawps;
  *als = convert_lx(rawals);

  return (rc);
}

unsigned char RPR0521RS::check_near_far(unsigned short data)
{
  if (data >= RPR0521RS_NEAR_THRESH) {
    return (RPR0521RS_NEAR_VAL);
  } else {
    return (RPR0521RS_FAR_VAL);
  }
}

float RPR0521RS::convert_lx(unsigned short *data)
{
  float lx;
  float d0, d1, d1_d0;

  if (_als_data0_gain == 0) {
    return (RPR0521RS_ERROR);
  }

  if (_als_data1_gain == 0) {
    return (RPR0521RS_ERROR);
  }

  if (_als_measure_time == 0) {
    return (RPR0521RS_ERROR);
  } else if (_als_measure_time == 50) {
    if ((data[0] & 0x8000) == 0x8000) {
      data[0] = 0x7FFF;
    }
    if ((data[1] & 0x8000) == 0x8000) {
      data[1] = 0x7FFF;
    }
  }

  d0 = (float)data[0] * (100 / _als_measure_time) / _als_data0_gain;
  d1 = (float)data[1] * (100 / _als_measure_time) / _als_data1_gain;

  if (d0 == 0) {
    lx = 0;
    return (lx);
  }

  d1_d0 = d1 / d0;

  if (d1_d0 < 0.595) {
    lx = (1.682 * d0 - 1.877 * d1);
  } else if (d1_d0 < 1.015) {
    lx = (0.644 * d0 - 0.132 * d1);
  } else if (d1_d0 < 1.352) {
    lx = (0.756 * d0 - 0.243 * d1);
  } else if (d1_d0 < 3.053) {
    lx = (0.766 * d0 - 0.25 * d1);
  } else {
    lx = 0;
  }

  return (lx);
}


int RPR0521RS::write2(unsigned char command, unsigned char *data, unsigned char size)
{
	int result = -1;
	result = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, (const char *)&command, size, 0);
	return result;
}

int RPR0521RS::read2(unsigned char command, unsigned char *data, int size)
{
	int result = -1;
	result = mbed_i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, (const char *)&command, 1, 1);
	if(result < 0){
		// 書き込みエラーの場合は中断して戻る
		return result;
	}
	result = mbed_i2c.read(RPR0521RS_DEVICE_ADDRESS << 1, (char *)data, size);
	return result;
}

unsigned char RPR0521RS::write(unsigned char memory_address, unsigned char *data, unsigned char size)
{
  unsigned char rc;

//  Wire.beginTransmission(RPR0521RS_DEVICE_ADDRESS);
  beginTransmission(RPR0521RS_DEVICE_ADDRESS);
//  Wire.write(memory_address);
  wire_write(memory_address);
//  Wire.write(data, size);
  wire_write(data, size);
//  rc = Wire.endTransmission();
  rc = endTransmission();
  return (rc);
}



unsigned char RPR0521RS::read(unsigned char memory_address, unsigned char *data, int size)
{
  unsigned char rc;
  unsigned char cnt;

  syslog(LOG_ERROR, "read called");

  //  Wire.beginTransmission(RPR0521RS_DEVICE_ADDRESS);
  beginTransmission(RPR0521RS_DEVICE_ADDRESS);
//  Wire.write(memory_address);
  wire_write(memory_address);
//  rc = Wire.endTransmission(false);
  rc = endTransmission(false);
  if (rc != 0) {
	syslog(LOG_ERROR, "rc != 0 return");

	  return (rc);
  }else{
		syslog(LOG_ERROR, "rc ok");
  }

//  Wire.requestFrom(RPR0521RS_DEVICE_ADDRESS, size, true);
  requestFrom(RPR0521RS_DEVICE_ADDRESS, size, true);
  cnt = 0;
//  while(Wire.available()) {
  while(available()) {
//	data[cnt] = Wire.read();
    data[cnt] = wire_read();
    cnt++;
  }
  syslog(LOG_ERROR, "read function end");

  return (0);
}

// add

void RPR0521RS::beginTransmission(int _address)
{
    syslog(LOG_ERROR, "beginTransmission called");
//    begin();
    i2c_start(&_i2c_t);
    i2c_byte_write(&_i2c_t, _address << 1);
    syslog(LOG_ERROR, "beginTransmission end");
}

unsigned char RPR0521RS::endTransmission()
{
    return endTransmission(true);
}

unsigned char RPR0521RS::endTransmission(unsigned char stop)
{
    syslog(LOG_ERROR, "endTransmission called");
    if (stop) {
        i2c_stop(&_i2c_t);
    }
    return 0;
}

size_t RPR0521RS::wire_write(unsigned char _data)
{
    syslog(LOG_ERROR, "wire_write(unsigned char _data) called");

    return i2c_byte_write(&_i2c_t, _data);
}

size_t RPR0521RS::wire_write(const unsigned char* _data, size_t _size)
{
    syslog(LOG_ERROR, "wire_write(const unsigned char* _data) called");
    for (size_t i = 0; i < _size; i++) {
        wire_write(_data[i]);
    }
    return _size;
}

unsigned char RPR0521RS::requestFrom(unsigned char _address, unsigned char quantity, unsigned char stop)
{
    return requestFrom(int(_address), int(quantity), int(stop));
}

unsigned char RPR0521RS::requestFrom(int _address, int quantity, int stop)
{

    syslog(LOG_ERROR, "requestFrom called");
// clamp to buffer length
    if (quantity > BUFFER_LENGTH) {
        quantity = BUFFER_LENGTH;
    }

    // perform blocking read into buffer
    int rc = i2c_read(&_i2c_t, _address << 1, (char*)rxBuffer, quantity, stop);

    syslog(LOG_ERROR, "i2c_read result=%d", rc);

    // set rx buffer iterator vars
    rxBufferIndex = 0;
    rxBufferLength = quantity;

    return quantity;
}

int RPR0521RS::available()
{
    return rxBufferLength - rxBufferIndex;
}

int RPR0521RS::wire_read()
{
    int value = -1;

    // get each successive byte on each call
    if (rxBufferIndex < rxBufferLength) {
        value = rxBuffer[rxBufferIndex++];
    }

    return value;
}
