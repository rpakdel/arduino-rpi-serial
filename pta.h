#pragma once

#include <Arduino.h>

struct PTA
{
    float pressure;
    float temperature;
    float altitude;
};

void PrintPTA(PTA& pta, Print& print)
{
    print.print("Pressure:    ");
    print.print(pta.pressure);
    print.println(" hPa");
    
    print.print("Temperature: ");
    print.print(pta.temperature);
    print.println(" C");


    print.print("Altitude:    ");
    print.print(pta.altitude);
    print.println(" m");
    print.println();
}
