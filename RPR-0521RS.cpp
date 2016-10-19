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

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "../syssvc/serial.h"
#include "../syssvc/syslog.h"
#include "kernel_cfg.h"
#include "i2c_api.h"

#include <RPR-0521RS.h>

RPR0521RS::RPR0521RS(PinName sda, PinName scl) :
_sda(sda), _scl(scl), _i2c(sda, scl)
{
}

int RPR0521RS::init(void)
{
	unsigned char reg;
	unsigned char index;
	unsigned char als_gain_table[] = {1, 2, 64, 128};
	unsigned short als_meas_time_table[] = {0,0,0,0,0,100,100,100,100,100,400,400,50,0,0,0};
	int result;

	i2c_init(&_i2c_t, _sda, _scl);
	i2c_slave_mode(&_i2c_t, false);

	result = read(RPR0521RS_SYSTEM_CONTROL, &reg, sizeof(reg));
	if (result < 0) {
		syslog(LOG_ERROR, "Can't access RPR0521RS");
		return (result);
	}else{
		syslog(LOG_ERROR, "Can access RPR0521RS");
	}
	syslog(LOG_ERROR, "reg raw value:%d", reg);
	reg &= 0x3F;
	syslog(LOG_ERROR, "RPR0521RS Part ID Register Value = 0x%x", reg);

	if (reg != RPR0521RS_PART_ID_VAL) {
		syslog(LOG_ERROR, "Can't find RPR0521RS");
		return (result);
	}

	result = read(RPR0521RS_MANUFACT_ID, &reg, sizeof(reg));
	if (result < 0) {
		syslog(LOG_ERROR, "Can't access RPR0521RS");
		return (result);
	}
	syslog(LOG_ERROR, "RPR0521RS MANUFACT_ID Register Value = 0x%x", reg);

	if (reg != RPR0521RS_MANUFACT_ID_VAL) {
		syslog(LOG_ERROR, "Can't find RPR0521RS");
		return (result);
	}

	result = write(RPR0521RS_ALS_PS_CONTROL, &reg, sizeof(reg));
	if (result < 0) {
		syslog(LOG_ERROR, "Can't write RPR0521RS ALS_PS_CONTROL register");
		return (result);
	}

    result = read(RPR0521RS_PS_CONTROL, &reg, sizeof(reg));
	if (result != 0) {
		syslog(LOG_ERROR, "Can't read RPR0521RS PS_CONTROL register");
		return (result);
	}

	reg |= RPR0521RS_PS_CONTROL_VAL;
	result = write(RPR0521RS_PS_CONTROL, &reg, sizeof(reg));
	if (result != 0) {
		syslog(LOG_ERROR, "Can't write RPR0521RS PS_CONTROL register");
	}

	reg = RPR0521RS_MODE_CONTROL_VAL;
	result = write(RPR0521RS_MODE_CONTROL, &reg, sizeof(reg));
	if (result != 0) {
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
	result = read(RPR0521RS_MODE_CONTROL, &reg, sizeof(reg));
	syslog(LOG_ERROR, "RPR0521RS_MODE_CONTROL=%x", reg);
	result = read(RPR0521RS_ALS_PS_CONTROL, &reg, sizeof(reg));
	syslog(LOG_ERROR, "RPR0521RS_ALS_PS_CONTROL=%x", reg);
	result = read(RPR0521RS_PS_CONTROL, &reg, sizeof(reg));
	syslog(LOG_ERROR, "RPR0521RS_PS_CONTROL=%x", reg);

	syslog(LOG_ERROR, "init ok!");
	return result;
}

unsigned char RPR0521RS::get_rawpsalsval(unsigned char *data)
{
	syslog(LOG_ERROR, "get_rawpsalsval called");
	int result = read(RPR0521RS_PS_DATA_LSB, data, 6);
	if (result != 0) {
		syslog(LOG_ERROR, "Can't get RPR0521RS PS/ALS_DATA value");
	}else{
		syslog(LOG_ERROR, "get_rawpsalsval ok");
	}

	return (result);
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


int RPR0521RS::write(unsigned char command, unsigned char *data, unsigned char size)
{
	int result = _i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, (const char *)&command, size, 0);
	return result;
}

int RPR0521RS::read(unsigned char command, unsigned char *data, int size)
{
	int result = _i2c.write(RPR0521RS_DEVICE_ADDRESS << 1, (const char *)&command, 1, 1);
	if(result != 0){
		// 書き込みエラーの場合は中断して戻る
		return result;
	}
	result = _i2c.read(RPR0521RS_DEVICE_ADDRESS << 1, (char *)data, size);
	return result;
}
