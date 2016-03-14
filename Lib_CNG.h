#ifndef _CNG_H_
#define _CNG_H_

#include "pins.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <dht11.h>
#include <IRremote.h>

#define MAX_LENGTH         6


/* The heartbeat command */
#define CMD_HELLO          'H'
/* Command to set something */
#define CMD_SET            'S'
/* Reset the SOC */
#define CMD_RESET          'R'
#define CMD_SEND_DATA      'D'
#define CMD_READ_STATE     'V'

#define TYPE_DATA_TIMEOUT  'D'
#define TYPE_HELLO_TIMEOUT 'H'
#define TYPE_MODE          'M'

#define MODE_LEARN         'L'
#define MODE_SILENT        'S'

#define TARGET_REMOTE      'R'
#define TARGET_FAN         'F'
#define TARGET_LOCK        'L'
#define TARGET_LIGHT       'T'
#define TARGET_DOOR        'D'

#define B2L(buff)          ((uint32_t)((buff)[0] & 0xff) << 24 | \
                                      ((buff)[1] & 0xff) << 16 | \
                                      ((buff)[2] & 0xff) << 8  | \
                                      ((buff)[3] & 0xff))

/* +++++++++++++++++++++ common functions ++++++++++++++++ */
void readAndSendState () {
    
}

inline void beep (int beeper) {
    for (int i = 0; i < 100; i ++) {
        digitalWrite (beeper, HIGH);
        delayMicroseconds (200);
        digitalWrite (beeper, LOW);
        delayMicroseconds (200);
    }
    delay (200);
}

inline void raise_event (char target, char value) {
    Serial.print ("{E:{T:\"");
    Serial.print (target);
    Serial.print ("\",D:\"");
    Serial.print (value);
    Serial.println ("\"}}");
    Serial.flush ();
}

/* +++++++++++++++++++++ Main class ++++++++++++++++++++ */
class CNG_Class {
private:
    long    touch,                              // sensor data touch timestamp
            hello_touch,                        // hello touch timestamp
            data_timeout = 5000,                // sensor data timeout
            hello_timeout = 5000;               // hello timeout
    int pos = 0,                                // command read position
        fail_count = 3,                         // bluetooth connect fail count
        mode = MODE_SILENT,                     // IR Control mode
        door_value, v1, mismatch_count = 0;     // door sensor variables
    char command[MAX_LENGTH];

    OneWire *oneWire;
    DallasTemperature *sensor;
    dht11 DHT11;
    IRrecv *receiver;
    IRsend sender;
    decode_results results;
    
    void sendSensorData (long now);
    float readHumidity ();
    void checkStatus (long now);
    void process (const char cmd, const char type, const char *buff);
    void set (char target, const char *buff);
    void sendData (char target, int value);
    void learn ();
    void checkEvent ();
public :
    CNG_Class ();
    ~CNG_Class ();
    void init ();
    void readCommand ();
    void doWork ();
};

/* +++++++++++++++++ implementations +++++++++++++++++++ */
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
    pinMode (FAN_BUS, OUTPUT);
    pinMode (DOOR_SENSOR_BUS, INPUT_PULLUP);

    digitalWrite (ERROR_BUS, HIGH);
    digitalWrite (OK_BUS, LOW);
    
    touch = hello_touch = millis ();
    door_value = digitalRead (DOOR_SENSOR_BUS);
    receiver -> enableIRIn ();
    sensor -> begin ();
}

/**
 * Read commands from BT Port
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
//#ifdef DEBUG
        Serial.print ("cmd     = ");
        Serial.println (cmd);
        Serial.print ("type    = ");
        Serial.println (type);
        for (int i = 0; i < 4; i ++) {
            Serial.print ("buff[");
            Serial.print (i);
            Serial.print ("] = ");
            Serial.println (buff[i], HEX);
        }
        Serial.println ();
//#endif
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
//        float hum, temp;
//        read (&temp, &hum);
        int smoke = analogRead (MQ_BUS);
        Serial.print ("{D:");
        Serial.print ("{T:");
        Serial.print (sensor->getTempCByIndex (0));
//        Serial.print (temp);
        Serial.print (",H:");
//        Serial.print (hum);
        Serial.print (humidity);
        Serial.print (",S:");
        Serial.print (smoke);
        Serial.println ("}}");
        touch = millis ();
    }
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
            set (type, buff);
            break;
        }
        case CMD_SEND_DATA : {
            uint32_t data = B2L (buff);
            sendData (type, data);
            break;
        }
        case CMD_READ_STATE :
            readAndSendState ();
            break;
        default :
            break;
    }
}

void CNG_Class::set (char target, const char *buff) {
    switch (target) {
        case TYPE_DATA_TIMEOUT : {
            uint32_t n = B2L (buff);
            data_timeout = n * 1000;
            break;
        }
        case TYPE_HELLO_TIMEOUT : {
            uint32_t n = B2L (buff);
            hello_timeout = n * 1000;
            break;
        }
        case TYPE_MODE :
            mode = buff[0];
            raise_event (target, (char) mode);
            break;
        default :
            break;
    }
}

void CNG_Class::sendData (char target, int value) {
    switch (target) {
        case TARGET_FAN :
            digitalWrite (FAN_BUS, value);
            raise_event (target, value ? 'U' : 'd');
            break;
        case TARGET_LOCK :
            digitalWrite (LOCK_BUS, value);
            raise_event (target, value ? 'U' : 'd');
            beep (BEEPER);
            break;
        case TARGET_REMOTE :
            break;
        case TARGET_LIGHT :
            digitalWrite (LIGHT_BUS, value);
            raise_event (target, value ? 'U' : 'd');
            break;
        case TARGET_DOOR :
            if (value) { // android auth success. open the door and beep
                digitalWrite (LOCK_BUS, HIGH);
                raise_event (target, 'U');
                beep (BEEPER);
            } else {
                for (int i = 0; i < 3; i ++) {
                    beep (BEEPER);
                }
            }
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
            if (results.value != -1) {
                Serial.print ("{I:{C:");
                Serial.print (results.value);
                Serial.print (",T:");
                Serial.print (results.decode_type);
                Serial.println ("}}");
            }
            receiver->resume ();
        }
    }
}

void CNG_Class::checkEvent () {
    int value = digitalRead (DOOR_SENSOR_BUS);
    if (value != door_value) {
        if (mismatch_count >= 5) {
/*
            Serial.print ("{E:{T:\"D\",D:\"");
            Serial.print (value == HIGH ? "C" : "O");
            Serial.println ("\"}}");
*/
            raise_event ('D', value == HIGH ? 'C' : 'O');
            door_value = value;
            v1 = door_value;
            mismatch_count = 0;
        } else {
            mismatch_count++;
        }
    }
}
#endif
