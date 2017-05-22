/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   sesnorboardconnector.h
 * Author: andreaswassmer
 *
 * Created on 4. Mai 2017, 16:19
 */

#ifndef SENSORBOARDCONNECTOR_H
#define SENSORBOARDCONNECTOR_H

#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fstream>
#include <string>
#include <sstream>

#include "mcp3008Spi.h"

std::string readSensors();
int sampleSound(mcp3008Spi &bus, int delay, int channel);



#endif /* SENSORBOARDCONNECTOR_H */

