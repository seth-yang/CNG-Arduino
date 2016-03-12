#include <Arduino.h>
#include "Lib_CNG.h"
#include "card.h"

CNG_Class CNG;

void setup () {
    Serial.begin (9600);
    CNG.init ();

    init_card ();

    pinMode (BEEPER, OUTPUT);
    digitalWrite (BEEPER, LOW);
}

void loop () {
    CNG.readCommand ();
    CNG.doWork ();

    read_sector ();
}