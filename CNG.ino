#include <OneWire.h>
#include <DallasTemperature.h>
#include <dht11.h>
#include <IRremote.h>
#include "Lib_CNG.h"

CNG_Class CNG;
void setup () {
    Serial.begin (9600);
    CNG.init ();
}

void loop () {
    CNG.readCommand ();
    CNG.doWork ();
}

