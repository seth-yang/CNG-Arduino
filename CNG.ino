#include <OneWire.h>
#include <DallasTemperature.h>
#include <dht11.h>
#include <IRremote.h>
#include "Lib_CNG.h"

#define FLAG_LED  13

CNG_Class CNG;
void setup () {
    Serial.begin (9600);
    CNG.init ();

    pinMode (FLAG_LED, OUTPUT);
    digitalWrite (FLAG_LED, LOW);
}

void loop () {
    CNG.readCommand ();
    CNG.doWork ();
}
