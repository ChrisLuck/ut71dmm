///////////////////////////////////////////////////////////////////////////////
//
// Uni-T UT71C DMM-Software
// Copyright (C) 2014 Christian Lück
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
///////////////////////////////////////////////////////////////////////////////

#include "ut71.h"
#include <stdio.h>
#include <exception>
#include <cstdlib>
#include <sys/time.h>
#include <time.h>

//the rows in rangelut correspond to the the following multimeter-functions:
//0 : AC Voltage (mV)  (may be wrong, doesn't exist on my UT71C)
//1 : DC Voltage
//2 : AC Voltage
//3 : DC Voltage (mV)
//4 : Resistance
//5 : Capacitance
//6 : Temperature Celsius
//7 : Current (uA)
//8 : Current (mA)
//9 : Current (A)
//10: Beep
//11: Diode
//12: Freqency
//13: Temperature Farenheit
//14: unknown (doesn't exist?)
//15: 4-20mA loop current
//16: Duty cycle (this mode is fake. The received data from the meter doesnt
//                differentiate between freq and duty mode.
//                Workaround: look on sign-bit.
//
//(row:function    column:range)
const char* ut71::rangelut[17][8]  = 
{
{"400mV", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"},
{"ERR", "4V", "40V", "400V", "1000V", "ERR", "ERR", "ERR"},
{"ERR", "4V", "40V", "400V", "1000V", "ERR", "ERR", "ERR"},
{"400mV", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"},
{"ERR", "400", "4k", "40k", "400k", "4M", "40M", "ERR"},
{"ERR", "40n", "400n", "4u", "40u", "400u", "4m", "40m"},
{"1000", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"},
{"400u", "4000u", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"},
{"40m", "400m", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"},
{"ERR", "10", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"},
{"400", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"},
{"4", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"},
{"40", "400", "4k", "40k", "400k", "4M", "40M", "400M"},
{"1832", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"},
{"ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"},
{"100", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"},
{"100", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR", "ERR"}
};

//the position of the comma sign is not sent explicitely by the meter.
//the position of the comma depends on the range the meter is in, so the
//comma can be "reconstructed" with the following multiplicators:
//(row:function    column:range)
const long double ut71::multlut[17][8]  = 
{
{1e-5l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l},
{1.0l, 1e-4l, 1e-3l, 1e-2l, 1e-1l, 1.0l, 1.0l, 1.0l},
{1.0l, 1e-4l, 1e-3l, 1e-2l, 1e-1, 1.0l, 1.0l, 1.0l},
{1e-5l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l},
{1.0l, 1e-2l, 1e-1l, 1.0l, 1e1l, 1e2l, 1e3l, 1.0l},
{1.0l, 1e-12l, 1e-11l, 10e-10l, 1e-9l, 1e-8l, 1e-7l, 1e-6l},
{1e-1l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l},
{1e-8l, 1e-7l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l},
{1e-6l, 1e-5l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l},
{1.0l, 1e1l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l},
{1e-2l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l},
{1e-4l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l},
{1e-3l, 1e-2l, 1e-1l, 1.0l, 1e1l, 1e2l, 1e3l, 1e4l},
{1e-1l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l},
{1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l},
{1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l},
{1e-2l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l, 1.0l}
};

const char* ut71::functionlut[] = 
{"mV_AC", "V_DC", "V_AC", "mV_DC", "Ohm", "Farad", "Deg_C", 
"uA", "mA", "A", "Beep", "Diode", "Freq", "Deg_F",
"unknown", "4-20mA_loop", "Duty"};

const char* ut71::acdcmodelut[] = {"DC", "AC", "unknown", "AC+DC"};

const char* ut71::rangemodelut[] = {"manual", "auto"};


ut71::ut71() {}

ut71::~ut71() {}


bool ut71::check(unsigned char * data) {
	if(data[9] != 0x0D)
		return false;
	if(data[10] != 0x8A)
		return false;

	return true;
}

void ut71::parse(unsigned char * data) {
	char digits[6];
	digits[0] = (data[0] & 0x0F) + '0';
	digits[1] = (data[1] & 0x0F) + '0';
	digits[2] = (data[2] & 0x0F) + '0';
	digits[3] = (data[3] & 0x0F) + '0';
	digits[4] = (data[4] & 0x0F) + '0';
	digits[5] ='\0';

	this->digits = atoi(digits);

	//extract sign
	if (data[8] & 0x04){
		this->digits *= -1;
		this->sign = NEG;
	}
	else
		this->sign = POS;

	//extract AC/DC/AC+DC setting
	this->acdcmode = (acdcmodeenum)(data[7] & 0x0F);

	//extract rangemode (manual/auto)
	this->rangemode = (rangemodeenum)(data[8] & 0x01);

	//extract mode
	this->function = (functionenum)(data[6] & 0x0F);
	//freq- and duty-function both have the same this->function value
	//at this point.
	//they can only be differentiated by the value of the sign-bit:
	if(this->function == FREQ && this->sign == NEG){
		this->function = DUTY;
		this->rangemode = MANUAL;
	}

	//extract range
	this->rangeindex = data[5] & 0x0F;

	//calc value
	long double multi = this->multlut[this->function][this->rangeindex];
	this->value = ((long double)this->digits) * multi;


	//print it, comma separated
	fprintf(stdout, "%s, ", this->functionlut[this->function]);
	fprintf(stdout, "%s, ", this->acdcmodelut[this->acdcmode]);
	fprintf(stdout, "%s, ", this->rangemodelut[this->rangemode]);
	fprintf(stdout, "%s, ", this->rangelut[this->function][this->rangeindex]);
	fprintf(stdout, "%d, ", this->digits);
	fprintf(stdout, "%Lf;\n", this->value);
        fflush(stdout);
}

