#ifndef _PINS_H_
#define _PINS_H_

// pin definations
/*
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */
#define ONE_WIRE_BUS         2             // typical to DS18B20 sensor
#define IR_LED               3             // REQUIRED.
#define DOOR_SENSOR_BUS      4             // door sensor
#define DHT11_BUS            5             // The DHT11's port
#define RECEIVER_BUS         6             // IR Remote Receiver port
#define FAN_BUS              7             // fans
#define LIGHT_BUS            8             // lights
#define LOCK_BUS             9             // locks
#define SDA_PIN             10             // pin 10~13 connect to RC522, see the table above for detail
#define RST_PIN             A0             // in this case, we connect RC522 pin RST to A0
#define MQ_BUS              A1             // The MQ-Serial sensor's port
#define BEEPER              A2             // The buzzer
#define OK_BUS              A4             // When the android connect me, turn this led on
#define ERROR_BUS           A5             // If the android is not connected, turn this led on

#endif //_PINS_H_