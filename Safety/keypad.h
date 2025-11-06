#ifndef KEYPAD_H
#define KEYPAD_H

#include <Arduino.h>
#include "config.h"

class Keypad {
private:
    byte rowPins[ROWS];
    byte colPins[COLS];
    char keys[ROWS][COLS];

public:
    // Constructor
    inline Keypad() {
        memcpy(rowPins, ROW_PINS, sizeof(rowPins));
        memcpy(colPins, COL_PINS, sizeof(colPins));
        memcpy(keys, KEYS, sizeof(keys));
    }

    // Initialize pin modes
    inline void begin() {
        for (byte r = 0; r < ROWS; r++) {
            pinMode(rowPins[r], OUTPUT);
            digitalWrite(rowPins[r], HIGH);
        }
        for (byte c = 0; c < COLS; c++) {
            pinMode(colPins[c], INPUT_PULLUP);
        }
    }

    // Scan and return key pressed (or '\0' if none)
    inline char getKey() {
        for (byte r = 0; r < ROWS; r++) {
            digitalWrite(rowPins[r], LOW);

            for (byte c = 0; c < COLS; c++) {
                if (digitalRead(colPins[c]) == LOW) {
                    delay(20); // debounce
                    while (digitalRead(colPins[c]) == LOW); // wait for release
                    digitalWrite(rowPins[r], HIGH);
                    return keys[r][c];
                }
            }

            digitalWrite(rowPins[r], HIGH);
        }
        return '\0'; // no key pressed
    }

    // Wait for 4 keys and check if they match the stored passcode
    inline bool checkPasscode() {
        char entered[5] = {0};
        byte count = 0;

        Serial.println("Enter 4-digit code:");

        while (count < 4) {
            char key = getKey();
            if (key) {
                if (key == '*' || key == '#') continue; // ignore control keys
                entered[count++] = key;
                Serial.print('*'); // hide actual input
            }
        }

        entered[4] = '\0'; // null terminate

        // Compare entered code to stored passcode
        if (strcmp(entered, PASSCODE) == 0) {
            Serial.println("\nAccess Granted!");
            return true;
        } else {
            Serial.println("\nAccess Denied!");
            return false;
        }
    }
};

#endif
