/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "sensorboardconnector.h"

using namespace std;

// the numbers represent the channel of the mcp3008 to which the sensor is
// connected to. If you use a different layout change the values accordingly.

typedef enum {
    SENSOR_BRIGHTNESS = 0,
    SENSOR_AIRQUALITY = 1,
    SENSOR_TEMPERATURE = 2,
    SENSOR_LOUDNESS = 3
} sensor_t;

#define NUM_OF_CHANNELS 4
#define SLEEP_S 300

std::string readSensors() {
    mcp3008Spi a2d("/dev/spidev0.0", SPI_MODE_0, 1000000, 8);

    int measurement = 1;
    int a2dVal = 0;
    unsigned char rawdata[3];
    int values[NUM_OF_CHANNELS];
    time_t tstamp = 0;

    ostringstream message;

    //while (true) {

    for (int channel = 0; channel < NUM_OF_CHANNELS; channel++) {
        if (channel == SENSOR_LOUDNESS)
            values[channel] = sampleSound(a2d, 5, SENSOR_LOUDNESS);
        else {

            rawdata[0] = 1; //  first byte transmitted -> start bit
            rawdata[1] = 0b10000000 | (((channel & 7) << 4)); // second byte transmitted -> (SGL/DIF = 1, D2=D1=D0=0)
            rawdata[2] = 0; // third byte transmitted....don't care

            a2d.spiWriteRead(rawdata, sizeof (rawdata));

            a2dVal = 0;
            a2dVal = (rawdata[1] << 8) & 0b1100000000; //merge data[1] & data[2] to get result
            a2dVal |= (rawdata[2] & 0xff);
            values[channel] = a2dVal;
        }
    }
    // create the json string
    tstamp = time(NULL);
    message << "{\"tstamp\":\"" << tstamp << "\",";

    message << "\"brightness\":\"" << values[0] << "\",";
    message << "\"pollution\":\"" << values[1] << "\",";
    message << "\"temperature\":\"" << values[2] << "\",";
    message << "\"noise\":\"" << values[3] << "\"}";

    message << endl;
    //}
    return message.str();
}

int sampleSound(mcp3008Spi &bus, int delay, int channel) {
    unsigned char data[3];
    int value = 0;
    int a2dVal = 0;

    struct timeval tval_before, tval_after, tval_result;
    gettimeofday(&tval_before, NULL);

    do {
        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);

        data[0] = 1; //  first byte transmitted -> start bit
        data[1] = 0b10000000 | (((channel & 7) << 4)); // second byte transmitted -> (SGL/DIF = 1, D2=D1=D0=0)
        data[2] = 0; // third byte transmitted....don't care

        bus.spiWriteRead(data, sizeof (data));

        a2dVal = 0;
        a2dVal = (data[1] << 8) & 0b1100000000; //merge data[1] & data[2] to get result
        a2dVal |= (data[2] & 0xff);

        value += a2dVal;
    } while (tval_result.tv_usec <= delay);
    //printf("tval_result = %ld\n", tval_result.tv_usec);
    return value;
}

