#ifndef CARD_H
#define CARD_H

#include "pins.h"                         // load pin definations
#include <RFID.h>

/**
 * The RC522's key for reading IC card
 */
unsigned char APP_KEY[16] = {              
    0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC,
    0xff, 0x07, 0x80, 0x69,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

RFID rfid (SDA_PIN, RST_PIN);

void init_card () {
    SPI.begin ();
    rfid.init ();
}

/**
 * read block data at position:pos, with the key.
 * if read success, write the data to buff.
 */
void read_sector () {
    rfid.isCard ();
    rfid.readCardSerial ();
    rfid.selectTag (rfid.serNum);
    
    int state = rfid.auth (PICC_AUTHENT1A, 59, APP_KEY, rfid.serNum);
    if (state == MI_OK) {
        unsigned char buff[16];
        if (rfid.read (58, buff) == MI_OK) {
            Serial.print ("{E:{T:\"C\",D:\"");
            for (int i = 0; i < 16; i ++) {
                if (i > 0) Serial.print (" ");
                Serial.print ((buff [i] >> 4) & 0x0f, HEX);
                Serial.print (buff [i] & 0x0f, HEX);
            }
            Serial.println ("\"}}");
        }
    }

    rfid.halt ();
}
#endif // CARD_H
