/* -*- mode: c++ -*-
 *
 *
 * Authors: Géry Casiez
 *
 * Copyright © Inria
 *
 */

#include "TurboHID.h"
#include <pointing/pointing.h>
#include <fstream>
#include <chrono>
#include <thread>

std::ostream *logstream = 0 ;

void turboHIDCallback(void */*context*/, TimeStamp::inttime timestamp, int nbBytesReceived, unsigned char* data) {
    std::stringstream ss; 
    for (int i = 0; i < nbBytesReceived; i++) {
        ss << (int)data[i] << " ";
    }

    *logstream << timestamp << " leonardo " << nbBytesReceived << " " << ss.str() << std::endl ;
}

int main(int /*argc*/, char** /*argv*/) {
    try {
    	TurboHID turboHID;

        std::string logfile = "";//cfg.get<std::string>("meter-logfile") ;
        if (logfile.empty())
            logstream = &std::cout ;
        else
            logstream = new std::ofstream(logfile.c_str()) ;
        turboHID.startThread();
        turboHID.setTurboHIDCallback(turboHIDCallback, NULL);

        while(true) {
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
        	//usleep(1000);
        }

        turboHID.stopThread();
    }  catch (std::exception const& e) {
        std::cerr << "error : " << e.what() << std::endl;
    }
	return 0;
}