#ifndef NASHGPS_H
#define NASHGPS_H

// This library/component should be general and work anywhere - may make fork to work with ESP to do cool timing things, may also be able to do this without using ESP-IDF functions

// Updating of Info struct is now only reset when update status is polled

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

double convertLatDeg(uint8_t buf[], uint8_t N);
double convertLonDeg(uint8_t buf[], uint8_t W);

enum sentenceType_t
{
    INVALID,
    GPGGA,
    GPRMC,
    GPGSA,
    OTHER
};

struct gpsInfo_s
{
    double lat;
    double lon;
    double alt;
    float speed; // this is going to be in mph
    bool fix;
    int fixType;
    bool updated;

    double timeRaw;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    uint16_t milliseconds;

    uint16_t dateRaw;
    uint8_t day;
    uint8_t month;
    uint8_t year;
};


// the length of the buffers could be tuned to use less memory, but its only a few bytes so not worried
struct bytesGPGGA_s
{
    uint8_t time[12];
    uint8_t lat[12];
    uint8_t N;
    uint8_t lon[12];
    uint8_t W;
    uint8_t fix;
    uint8_t sats[3];
    uint8_t hdop[4];
    uint8_t alt[6];
    uint8_t altUnit;
    uint8_t gSep[6];
    uint8_t gSepUnit;
    uint8_t checkSum[4];
};

struct bytesGPRMC_s
{
    uint8_t time[12];
    uint8_t validity;
    uint8_t lat[12];
    uint8_t N;
    uint8_t lon[12];
    uint8_t W;
    uint8_t speed[6];
    uint8_t course[6];
    uint8_t date[6];
    uint8_t degVar[6];
    uint8_t degVarW;
};

class NashGPS
{
    public:

    int feed(uint8_t c); // feed nmea characters to be sorted and parsed

    double getLat(); // return lattitude - done
    double getLon(); // return longitude - done
    double getAlt(); // return altitude
    float getSpeed(); // return speed with specified units, -- will add feature for unit specifying later

    double getTimeRaw(); // return full time - hhmmss.sss
    uint8_t getHours();
    uint8_t getMinutes();
    uint8_t getSeconds();
    uint16_t getMilliseconds();

    uint16_t getDateRaw(); // return full date - ddmmyy
    
    // these do not cause info to be put into non updated state
    bool hasFix(); // true for fix, false fo no fix - done
    int getFixType(); // 0 for no fix, 2 for 2D fix, 3 for 3D fix
    bool isUpdated(); // tells if data has been updated since anything has been retrieved

    private:

    //struct holding relevent data for info
    gpsInfo_s gpsInfo;
    
    // temp structs/buffers for parsing
    bytesGPGGA_s bytesGPGGA; 
    bytesGPRMC_s bytesGPRMC;
    uint8_t sentence[256];
    uint8_t sentenceLen = 0;

    // variables to parse and manage sentences -- should probably init these values somewhere else or something
    sentenceType_t sentenceType = OTHER;
    int sentenceIndex = 0;
    int termNum = 0;
    int termIndex = 0;

    // methods for parsing sentences
    bool compareCodes(const char* array2, int numToCompare); // compares first 6 characters in sentence to NMEA codes
    int parseSentence(sentenceType_t sentType);
    sentenceType_t getSentenceType();

    void parseGPGGA(); // fill bytesGPGGA with sentence parameters
    void updateGPGGA(); // update gps info based on GPGGA sentence
    void updateTimeGPGGA(gpsInfo_s* info); // update time related stuff from GGA sentence
    void clearBytesGPGGA(); // empties out GPGGA parse array to avoid overlapping and wrong values

    void parseGPRMC(); // fill bytesGPRMC with sentence parameters
    void updateGPRMC(); // update gps info based on GPRMC sentence
    void clearBytesGPRMC(); // empties out GPGGA parse array to avoid overlapping and wrong values

};

#endif