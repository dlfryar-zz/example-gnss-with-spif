/* mbed Microcontroller Library
 * Copyright (c) 2017 u-blox
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "gnss.h"

#define CHECK_TALKER(s) ((buffer[3] == s[0]) && (buffer[4] == s[1]) && (buffer[5] == s[2]))

// LEDs
DigitalOut ledRed(LED1, 1);
DigitalOut ledGreen(LED2, 1);
DigitalOut ledBlue(LED3, 1);

Thread spi_thread;
extern void thread_spi();
#define LOCATION_THREAD_SLEEP 5

/* This example program for the u-blox C030 and C027 boards instantiates
 * the gnss interface and waits for time/position to be received from a satellite.
 * Progress may be monitored with a serial terminal running at 9600 baud.
 * The LED on the C030 board will turn green when this program is
 * operating correctly, pulse blue when a time reading has been received,
 * pulse white when GNSS position has been received or turn red if there is
 * a failure.
 * On the C027 and C030 boards the green/red (respectively) LED near the
 * GNSS module will flash as the module achieves a fix.
 */

int main()
{
    spi_thread.start(thread_spi);

#ifdef TARGET_NRF52840_DK
    GnssI2C gnss(I2C_SDA0, I2C_SCL0);
#else
    GnssI2C gnss(I2C_SDA, I2C_SCL);
#endif

    int gnssReturnCode;
    int length;
    char buffer[256];

    printf("Starting up...\n");
    if (gnss.init())
    {
        printf("Waiting for GNSS to receive something...\n");
        while (1)
        {
            gnssReturnCode = gnss.getMessage(buffer, sizeof(buffer));
            if (gnssReturnCode > 0)
            {

                ledGreen = 0;
                ledBlue = 1;
                ledRed = 1;
                length = LENGTH(gnssReturnCode);

                printf("NMEA: %.*s\n", length - 2, buffer);

                if ((PROTOCOL(gnssReturnCode) == GnssParser::NMEA) && (length > 6))
                {
                    // Talker is $GA=Galileo $GB=Beidou $GL=Glonass $GN=Combined $GP=GNSS
                    if ((buffer[0] == '$') || buffer[1] == 'G')
                    {
                        if (CHECK_TALKER("GLL"))
                        {
                            double latitude = 0, longitude = 0;
                            char ch;

                            if (gnss.getNmeaAngle(1, buffer, length, latitude) &&
                                gnss.getNmeaAngle(3, buffer, length, longitude) &&
                                gnss.getNmeaItem(6, buffer, length, ch) && (ch == 'A'))
                            {
                                ledBlue = 0;
                                ledRed = 0;
                                ledGreen = 0;

                                printf("\nGNSS: location is %.5f %.5f.\n\n", latitude, longitude);
                                printf("I am here: https://maps.google.com/?q=%.5f,%.5f\n\n",
                                       latitude, longitude);
                            }
                        }
                        else if (CHECK_TALKER("GGA") || CHECK_TALKER("GNS"))
                        {
                            double altitude = 0;
                            const char *timeString = NULL;

                            // Altitude
                            if (gnss.getNmeaItem(9, buffer, length, altitude))
                            {
                                printf("\nGNSS: altitude is %.1f m.\n", altitude);
                            }

                            // Time
                            timeString = gnss.findNmeaItemPos(1, buffer, buffer + length);
                            if (timeString != NULL)
                            {
                                ledBlue = 0;
                                ledRed = 1;
                                ledGreen = 1;

                                printf("\nGNSS: time is %.6s.\n\n", timeString);
                            }
                        }
                        else if (CHECK_TALKER("VTG"))
                        {
                            double speed = 0;

                            // Speed
                            if (gnss.getNmeaItem(7, buffer, length, speed))
                            {
                                printf("\nGNSS: speed is %.1f km/h.\n\n", speed);
                            }
                        }
                    }
                }
                wait(LOCATION_THREAD_SLEEP);
            }
        }
    }
    else
    {
        printf("Unable to initialize GNSS.\n");
    }

    ledRed = 0;
    ledGreen = 1;
    ledBlue = 1;
    printf("Should never get here.\n");
    MBED_ASSERT(false);
}

// End Of File
