#include <NashGPS.h>

// TODO
// - parse other sentences, GPRMC, and GPGSA - finish parsing GPGGA
// - make get functions, to make updating things work
// - maybe implement timing stuff, or keep general need to decide or make two versions
// - put time in info struct
// - actually use more parameters, especially time, altitude etc
// -

#define _GPGGA "$GPGGA"
#define _GNGGA "$GNGGA"
#define _GPRMC "$GPRMC"
#define _GNRMC "$GNRMC"
#define _GPGSA "$GPGSA"
#define _GNGSA "$GNGSA"

bool NashGPS::compareCodes(const char* array2, int numToCompare) // compares the first term in nmea sentences
{
    for (int i = 0; i < numToCompare; i++)
    {
        if (sentence[i] != array2[i])
        {
            return false;
        }
    }
    return true;
}

int NashGPS::feed(uint8_t c) // feed characters from nmea stream into object to be split into sentences and passed along
{
    if (c == '$')
    {
        sentenceLen = sentenceIndex + 1;
        sentenceType = getSentenceType(); // get type of sentence
        parseSentence(sentenceType); // parse sentence

        sentenceLen = 0;
        sentenceIndex = 0;
        sentence[sentenceIndex] = c;
        sentenceIndex++;
        return 0; // got sentence
    }
    else
    {
        sentence[sentenceIndex] = c;
        sentenceIndex++;
        return 1; // no sentence
    }
}

int NashGPS::parseSentence(sentenceType_t sentType)
{
    // this is where to start! do switch statement for each type then parse!
    switch (sentType)
    {
        case (GPGGA): // has location and time and fix info and all the good stuff
            parseGPGGA();
            updateGPGGA();
            return 0;
            break;

        case(GPRMC): // has all the other stuff and satellites
            parseGPRMC();
            updateGPRMC();
            return 0;
            break;

        case (GPGSA):
            return 0;

        case(OTHER):
        case(INVALID):
            // do nothing
            return 1;
            break;
    }

    return 0; // success
}

sentenceType_t NashGPS::getSentenceType()
{
    if (sentence[0] != '$')
    {
        return INVALID;
    }

    if (compareCodes(_GPGGA, 6) || compareCodes(_GNGGA, 6))
    {
        return GPGGA;
    }

    if (compareCodes(_GPRMC, 6) || compareCodes(_GNRMC, 6))
    {
        return GPRMC;
    }

    if (compareCodes(_GPGSA, 6) || compareCodes(_GNGSA, 6))
    {
        return GPGSA;
    }

    else
    {
        return OTHER;
    }
}

void NashGPS::parseGPGGA()
{
    clearBytesGPGGA();
    termNum = 0;
    termIndex = 0;
    for (int i = 0; i < sentenceLen; i++)
    {
        if (sentence[i] == ',')
        {
            termNum++;
            termIndex = 0;
        }

        else if (termNum == 1) // pull time out and put into thingy
        {
            bytesGPGGA.time[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 2) // lattitude
        {
            bytesGPGGA.lat[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 3) // North/South
        {
            bytesGPGGA.N = sentence[i];
        }

        else if (termNum == 4) // longitude
        {
            bytesGPGGA.lon[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 5) // West/East
        {
            bytesGPGGA.W = sentence[i];
        }

        else if (termNum == 6) // fix
        {
            bytesGPGGA.fix = sentence[i];
        }

        else if (termNum == 7) // number of satellites
        {
            bytesGPGGA.sats[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 8) // altitude
        {
            bytesGPGGA.alt[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 9) // altitude unit
        {
            bytesGPGGA.altUnit = sentence[i];
        }

        else if (termNum == 10) // gSep
        {
            bytesGPGGA.gSep[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 11) // gsep unit
        {
            bytesGPGGA.gSepUnit = sentence[i];
        }
    }
}

void NashGPS::updateGPGGA()
{
    if (bytesGPGGA.fix == '1' || bytesGPGGA.fix == '2')
    {
        gpsInfo.lat = convertLatDeg(bytesGPGGA.lat, bytesGPGGA.N);
        gpsInfo.lon = convertLonDeg(bytesGPGGA.lon, bytesGPGGA.W);

        updateTimeGPGGA(&gpsInfo);

        gpsInfo.updated = true;
        gpsInfo.fix = true;
        return; // location updated
    }

    updateTimeGPGGA(&gpsInfo);

    gpsInfo.fix = false;
    gpsInfo.fixType = 0; // unknown fix type
    gpsInfo.updated = true;
    return; // location not updated, no fix
    
}

void NashGPS::updateTimeGPGGA()
{
    // this is kinda aids but it should be fast and fine, plus its hidden in a nice function
    gpsInfo.timeRaw = atof((const char*)bytesGPGGA.time);
    uint8_t buf[4];
    buf[2] = '\0';
    buf[3] = '\0';

    // hours
    buf[0] = bytesGPGGA.time[0];
    buf[1] = bytesGPGGA.time[1];
    gpsInfo.hours = atoi((const char*)buf);

    // minutes
    buf[0] = bytesGPGGA.time[2];
    buf[1] = bytesGPGGA.time[3];
    gpsInfo.minutes = atoi((const char*)buf);

    // seconds
    buf[0] = bytesGPGGA.time[4];
    buf[1] = bytesGPGGA.time[5];
    gpsInfo.seconds = atoi((const char*)buf);

    // milliseconds
    buf[0] = bytesGPGGA.time[7];
    buf[1] = bytesGPGGA.time[8];
    buf[2] = bytesGPGGA.time[9];
    gpsInfo.milliseconds = atoi((const char*)buf);
}

void NashGPS::parseGPRMC()
{
    clearBytesGPRMC();
    termNum = 0;
    termIndex = 0;
    for (int i = 0; i < sentenceLen; i++)
    {
        if (sentence[i] == ',')
        {
            termNum++;
            termIndex = 0;
        }

        else if (termNum == 1) // time
        {
            bytesGPRMC.time[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 2) // validity
        {
            bytesGPRMC.validity = sentence[i];
        }

        else if (termNum == 3) // lattitude
        {
            bytesGPRMC.lat[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 4) // north/south
        {
            bytesGPRMC.N = sentence[i];
        }

        else if (termNum == 5) // longitude
        {
            bytesGPRMC.lon[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 6) // west/east
        {
            bytesGPRMC.W = sentence[i];
        }

        else if (termNum == 7) // speed
        {
            bytesGPRMC.speed[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 8) // course
        {
            bytesGPRMC.course[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 9) // date
        {
            bytesGPRMC.date[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 10) // magnetic variation
        {
            bytesGPRMC.degVar[termIndex] = sentence[i];
            termIndex++;
        }

        else if (termNum == 11) // west/east for magnetic variation
        {
            bytesGPRMC.degVarW = sentence[i];
        }
    }
}

void NashGPS::updateGPRMC()
{
    // perhaps check validity, going to leave other stuff to GPGGA
    gpsInfo.speed = atof((const char*)bytesGPRMC.speed) * 1.15; // multiply by 1.15 to get in mph

}

double convertLatDeg(uint8_t buf[], uint8_t N) // convert from format in nmea to double deg
{
    uint8_t degByte[3];
    for (int i = 0; i < 2; i++)
    {
        degByte[i] = buf[i];
    }
    degByte[2] = '\0'; // add null char to stop atof

    uint8_t minByte[8];
    for (int i = 0; i < 7; i++)
    {
        minByte[i] = buf[i+2];
    }
    minByte[7] = '\0';

    double deg = atof((const char*)degByte);
    double min = atof((const char*)minByte);
    double lat = deg + (min/60.0);

    if (N == 'S')
    {
        lat = lat * -1.0;
    }

    return lat;
}

double convertLonDeg(uint8_t buf[], uint8_t W)
{
    uint8_t degByte[4];
    for (int i = 0; i < 3; i++)
    {
        degByte[i] = buf[i];
    }
    degByte[3] = '\0';

    uint8_t minByte[8];
    for (int i = 0; i < 7; i++)
    {
        minByte[i] = buf[i+3];
    }
    minByte[7] = '\0';

    double deg = atof((const char*)degByte);
    double min = atof((const char*)minByte);
    double lon = deg + (min/60.0);

    if (W == 'W')
    {
        lon = lon * -1.0;
    }

    return lon;
}

double NashGPS::getLat()
{
    return gpsInfo.lat;
}

double NashGPS::getLon()
{
    return gpsInfo.lon;
}

float NashGPS::getSpeed()
{
    //gpsInfo.updated = false; // going to leave updating info to GPGGA for now until I know more about the behavior
    return gpsInfo.speed;
}

double NashGPS::getAlt()
{
    return gpsInfo.alt;
}

bool NashGPS::hasFix()
{
    return gpsInfo.fix;
}

bool NashGPS::isUpdated()
{
    bool retVal = gpsInfo.updated;
    gpsInfo.updated = false;
    return retVal;
}

void NashGPS::clearBytesGPGGA()
{
    memset(&bytesGPGGA, 0, sizeof(bytesGPGGA));
}

void NashGPS::clearBytesGPRMC()
{
    memset(&bytesGPRMC, 0, sizeof(bytesGPRMC));
}

double NashGPS::getTimeRaw()
{
    return gpsInfo.timeRaw;
}

uint8_t NashGPS::getHours()
{
    return gpsInfo.hours;
}

uint8_t NashGPS::getMinutes()
{
    return gpsInfo.minutes;
}

uint8_t NashGPS::getSeconds()
{
    return gpsInfo.seconds;
}

uint16_t NashGPS::getMilliseconds()
{
    return gpsInfo.milliseconds;
}

