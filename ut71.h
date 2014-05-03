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

#ifndef ut71_H_
#define ut71_H_


class ut71 {
public:
    ut71();
    virtual ~ut71();

    bool check(unsigned char * data);
    void parse(unsigned char * data);


private:
    //look up tables
    static const long double multlut[][8];
    static const char* rangelut[][8]; 
    static const char* rangemodelut[];
    static const char* functionlut[];
    static const char* acdcmodelut[];

    //digits shown on display, without commma
    int digits;

    //measured value in V,A,Hz,Ohm.. ("milli" or "kilo" included in value)
    long double value;

    //points to current range (use rangelut to decode)
    unsigned int rangeindex;

    // UT71C: display shows "OL"
    bool overload;

    // UT71C: 4000count or 40000count
    bool lowcountmode;

    // UT71C: function, selected with rotary-switch and blue button
    enum functionenum {MVAC, VDC, VAC, MVDC, R, CAP, CTEMP, UA, MA, A,
        BEEP, DIODE, FREQ, FTEMP, UNKNOWN1, LOOPCUR, DUTY} function;

    // UT71C: yellow button
    enum acdcmodeenum {DC, AC, UNKNOWN2, ACDC} acdcmode;

    enum signenum {NEG, POS} sign;
    enum rangemodeenum {MANUAL, AUTO} rangemode;
};

#endif /* ut71_H_ */
