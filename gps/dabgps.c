//
// Ed's interface to standard gpsd daemon
// Collects position information fast enough for gpsd
// Stores it in DAB radio shared memory for dab logger
// which doesn't schedule often enough to stop gpsd buffer from overflowing
//

#include <stdio.h>
#include <gps.h>
#include <math.h>
#include <strings.h>
#include <unistd.h>

#include "../types.h"
#include "../shm.h"

#define GPS_TIMEOUT 5
#define GPSD_REQD_VERSION 11

dabShMemType *dabShMem;

void invalidateGpsInfo(gpsInfoType *gpsInfo)
{
    shmLock();
    bzero(gpsInfo, sizeof(gpsInfoType));
    shmFree();
}

int main(int argc, char *argv[])
{
    struct gps_data_t gps_data;
    struct tm *tmPtr;
    char timeStr[80];
    gpsInfoType *gpsInfo;
    long gpsTimeout;

    fprintf(stderr, "%d\n", getpid());
    setvbuf(stdout, NULL, _IOLBF, 0);

    printf("DAB receiver GPS engine\n");

    dabShMem = shmAttach();
    gpsInfo = &(dabShMem -> gpsInfo);
    invalidateGpsInfo(gpsInfo);

    printf("Using GPSD library version %d.%d\n", GPSD_API_MAJOR_VERSION,
                                                     GPSD_API_MINOR_VERSION);

    if(GPSD_API_MAJOR_VERSION != GPSD_REQD_VERSION)
    {
        printf("Wrong gpsd version - needs V%d.X\n", GPSD_REQD_VERSION);
        return 1;
    }

    gpsTimeout = GPS_TIMEOUT * 1000000;
    while(1)
    {
        invalidateGpsInfo(gpsInfo);

        if(gps_open("localhost", "2947", &gps_data) != 0)
        {
            printf("GPS open error\n");
            return 1;
        }

        gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

        while(gps_waiting(&gps_data, gpsTimeout))
        {
            if(gps_read(&gps_data, NULL, 0) == -1)
            {
                printf("GPS read error\n");
                break;
            }

            if((MODE_SET & gps_data.set) != MODE_SET)
            {
                printf("No GPS mode\n");
                continue;
            }

            if(gps_data.fix.mode < 2 || gps_data.fix.mode > 4)
            {
                printf("Invalid GPS mode\n");
                gps_data.fix.mode = 0;
            }

            if(isfinite(gps_data.fix.latitude) &&
                isfinite(gps_data.fix.longitude))
            {
                shmLock();
                gpsInfo -> latitude = gps_data.fix.latitude;
                gpsInfo -> longitude = gps_data.fix.longitude;
                gpsInfo -> altitude = gps_data.fix.altHAE;
                gpsInfo -> speed = gps_data.fix.speed;
                gpsInfo -> seconds = gps_data.fix.time.tv_sec;
                gpsInfo -> fix = gps_data.fix.mode;
                shmFree();

                tmPtr = localtime(&(gpsInfo -> seconds));
                strftime(timeStr, 80, "%d %B %Y, %H:%M:%S", tmPtr);
                printf("%dD fix at %s\n", gpsInfo -> fix, timeStr);

                printf("  Latitude: %0.6f  Longitude: %0.6f\n",
                                 gpsInfo -> latitude, gpsInfo -> longitude);
                printf("  Altitude: %0.3f m  Speed: %0.3f m/s\n",
                                 gpsInfo -> altitude, gpsInfo -> speed);
            }
            else
            {
                printf("Invalid GPS position\n");

                invalidateGpsInfo(gpsInfo);
            }
        }

        printf("GPS timeout\n");

        gps_stream(&gps_data, WATCH_DISABLE, NULL);
        gps_close(&gps_data);
    }

    return 0;
}
