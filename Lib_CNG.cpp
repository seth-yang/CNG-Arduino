#include <OneWire.h>
#include <DallasTemperature.h>
#include <dht11.h>
#include <IRremote.h>
#include "Lib_CNG.h"

CNG_Class::CNG_Class () {
	oneWire  = new OneWire (ONE_WIRE_BUS);
	sensor   = new DallasTemperature (oneWire);
	receiver = new IRrecv (RECEIVER_BUS);
}

CNG_Class::~CNG_Class () {
	delete sensor;
	delete oneWire;
	delete receiver;
}

void CNG_Class::init () {
    pinMode (ERROR_BUS, OUTPUT);
    pinMode (OK_BUS, OUTPUT);
    pinMode (DOOR_SENSOR_BUS, INPUT_PULLUP);

    digitalWrite (ERROR_BUS, HIGH);
    digitalWrite (OK_BUS, LOW);
	touch = hello_touch = millis ();
	door_value = digitalRead (DOOR_SENSOR_BUS);

	sensor -> begin ();
}

/**
 * Read commands from Serial Port
 */
void CNG_Class::readCommand () {
    while (Serial.available ()) {
        char ch = Serial.read ();
        if (ch >= 0) {
            command [pos ++] = ch;
        }
    }

    if (pos >= MAX_LENGTH) {
        char  cmd  = command [0];
        char  type = command [1];
        char *buff = command + 2;

        process (cmd, type, buff);
        memset (command, 0, MAX_LENGTH);
        pos = 0;
    }
}

/**
 * Send data to bluetooth
 */
void CNG_Class::sendSensorData (long now) {
    if (now - touch > data_timeout) {
        sensor->requestTemperatures ();
        float humidity = readHumidity ();
        int smoke = analogRead (MQ_BUS);
        Serial.print ("{\"D\":");
        Serial.print ("{\"T\":");
        Serial.print (sensor->getTempCByIndex (0));
        Serial.print (",\"H\":");
        Serial.print (humidity);
        Serial.print (",\"S\":");
        Serial.print (smoke);
        Serial.println ("}}");
        touch = millis ();
    }
}

/**
 * Convert 4 bytes to an integer
 */
int CNG_Class::byteToInt (const char *bytes, int start) {
    return (bytes [start    ] & 0xff) << 24 |
           (bytes [start + 1] & 0xff) << 16 |
           (bytes [start + 2] & 0xff) << 8  |
           (bytes [start + 3] & 0xff);
}

float CNG_Class::readHumidity () {
    int chk = DHT11.read (DHT11_BUS);
    if (chk == DHTLIB_OK) {
        return (float) DHT11.humidity;
    }

    return -1.0f;
}

void CNG_Class::checkStatus (long now) {
    if (now - hello_touch > hello_timeout) {
        if (fail_count > 0) {
            fail_count--;
            hello_touch = millis ();
        }

        if (fail_count <= 0) {
            digitalWrite (OK_BUS, LOW);
            digitalWrite (ERROR_BUS, HIGH);
        }
    }
}

void CNG_Class::process (const char cmd, const char type, const char *buff) {
    switch (cmd) {
        case CMD_HELLO : {
            fail_count = 3;
            hello_touch = millis ();
            digitalWrite (OK_BUS, HIGH);
            digitalWrite (ERROR_BUS, LOW);
            break;
        }
        case CMD_RESET :
            ;
            break;
        case CMD_SET : {
            int n = byteToInt (buff, 0) * 1000;
            set (type, n);
            break;
        }
        case CMD_SEND_DATA : {
            int data = byteToInt (buff, 0) * 1000;
            sendData (type, data);
            break;
        }
        default :
            break;
    }
}

void CNG_Class::set (char target, int value) {
    switch (target) {
        case TYPE_DATA_TIMEOUT :
            data_timeout = value;
            break;
        case TYPE_HELLO_TIMEOUT :
            hello_timeout = value;
            break;
        case TYPE_MODE :
            mode = value;
            break;
        default :
            break;
    }
}

void CNG_Class::sendData (char target, int value) {
    switch (target) {
        case TARGET_FAN :
            digitalWrite (FAN_BUS, value);
            break;
        case TARGET_LOCK :
            digitalWrite (LOCK_BUS, value);
            break;
        case TARGET_REMOTE :
            break;
        case TARGET_LIGHT :
            digitalWrite (LIGHT_BUS, value);
            break;
        default :
            break;
    }
}

void CNG_Class::doWork () {
    long now = millis ();
    checkStatus (now);
    sendSensorData (now);
    learn ();
    checkEvent ();
}

void CNG_Class::learn () {
    if (mode == MODE_LEARN) {
        if (receiver->decode (&results)) {
            Serial.print ("{\"C\":{\"C\":");
            Serial.print (results.value);
            Serial.println ("}}");
        }
    }
}

void CNG_Class::checkEvent () {
    int value = digitalRead (DOOR_SENSOR_BUS);
    if (value != door_value) {
        if (mismatch_count >= 5) {
            if (value == HIGH) {
                door_value = 1;
                Serial.println ("{\"E\":{\"D\":\"C\"}}");
                // door is closed.
            } else {
                door_value = 0;
                Serial.println ("{\"E\":{\"D\":\"O\"}}");
                // door is opened.
            }
            v1 = door_value;
            mismatch_count = 0;
        } else {
            mismatch_count++;
        }
    }
}