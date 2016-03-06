#ifndef _CNG_H_
#define _CNG_H_

#include <OneWire.h>
#include <DallasTemperature.h>
#include <dht11.h>
#include <IRremote.h>
#include <SoftwareSerial.h>

#define ONE_WIRE_BUS       2
#define IR_LED             3             // REQUIRED.
/**
 * When the android connect me, turn this led on
 */
#define OK_BUS            A4
/**
 * If the android is not connected, turn this led on
 */
#define ERROR_BUS         A5
/**
 * The DHT11's port
 */
#define DHT11_BUS          5

#define RECEIVER_BUS       6

#define FAN_BUS            7
#define LIGHT_BUS          8
#define LOCK_BUS           9
#define DOOR_SENSOR_BUS   10

/**
 * The MQ-Serial sensor's port
 */
#define MQ_BUS             A0
/*
#define TIMEOUT            1000
#define HELLO_TIMEOUT      1000
*/
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

#define B2L(buff)          ((uint32_t)((buff)[0] & 0xff) << 24 | \
                                      ((buff)[1] & 0xff) << 16 | \
                                      ((buff)[2] & 0xff) << 8  | \
                                      ((buff)[3] & 0xff))

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
//	int byteToInt (const char *bytes, int start);
	float readHumidity ();
//    void read (float *temperature, float *humidity);
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

void readAndSendState ();
#endif
