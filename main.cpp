///////////////////////////////////////////////////////////////////////////////
//
// Uni-T UT71C DMM-Software
// Copyright (C) 2014 Christian Lück
// 
// Based on:
//  he2325u.cpp - UT61E / HOITEK HE2325U USB interface SW
//  from Rainer Wetzel (c) 2011 (diyftw.de)
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

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include "hidapi.h"
#include "ut71.h"

// Headers needed for sleeping.
#include <unistd.h>

int main(int argc, char* argv[])
{
    int res;
    unsigned char buf[256];
    hid_device *handle=0;
    int dev_cnt = 0;

    ut71 myUT71;


    struct hid_device_info *devs, *cur_dev;
    

    if(argc==1) //if no params are supplied list available devs
    {
        devs = hid_enumerate(0x1a86, 0xe008);
        cur_dev = devs;
        while(cur_dev){cur_dev = cur_dev->next; dev_cnt++; }
        printf("[!] found %i devices:\n",dev_cnt);

        cur_dev = devs; 
        while (cur_dev){
            printf("%s\n", cur_dev->path);
            cur_dev = cur_dev->next;
        }

        hid_free_enumeration(devs);
    }


    if(argc>1) // use the supplied device (if any)
    {
        handle = hid_open_path(argv[1]);
    }   

    // Open the device using the VID, PID,
    // and optionally the Serial number (NULL for the hoitek chip).
    if(handle==0) handle = hid_open(0x1a86, 0xe008, NULL); // 1a86 e008

    if (!handle) {
        printf("unable to open device\n");
        return 1;
    }


    // Set the hid_read() function to be non-blocking.
    //hid_set_nonblocking(handle, 1);

    memset(buf,0,sizeof(buf));
    
    unsigned int bps = 19200;
    // Send a Feature Report to the device (set baudrate)
    buf[0] = bps;
    buf[1] = bps>>8;
    buf[2] = bps>>16;
    buf[3] = bps>>24;
    res = hid_send_feature_report(handle, buf, 4); // 4 bytes
    if (res < 0)
        printf("Unable to send a feature report.\n");

    memset(buf,0,sizeof(buf));

    int j=0;
    unsigned char recent = 0x0D;
    #define PACKET_SIZE 11
    unsigned char packet[PACKET_SIZE];
    memset(packet,0,sizeof(packet));

    printf("-data start-\n");

    usleep(1000);

    do
    {

        res = 0;
        while (res == 0){
            res = hid_read(handle, buf, sizeof(buf));
            if (res == 0)
                printf("waiting...\n");
            if (res < 0)
                printf("Unable to read()\n");
        }

        int len=buf[0] & 0x07; //extract length

        if(len==1){

            //print received data for debugging:
            //printf("%x ", buf[1]&0x0F);
            //printf("%x ", buf[1]);
            //fflush(stdout);

            //reconstruct packet
            packet[j] = buf[1];

            if(j-1 < PACKET_SIZE)
                j++;

            //sync packet (ends with the sequence 0x0D 0x0A)
            if(buf[1] == 0x8A && recent == 0x0D){
                j=0;
                if(myUT71.check(packet))
                    myUT71.parse(packet);
            }

            recent = buf[1];
        }

    }while(res>=0);

    hid_close(handle);

    return 0;
}
