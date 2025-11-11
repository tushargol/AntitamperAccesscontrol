#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <Arduino.h>
#include <Wire.h>
#include "espconfig.h"

class Accelerometer {
private:
    float baseX, baseY, baseZ;

public:
    inline void begin() {
        Wire.begin(SDA_PIN, SCL_PIN);
        delay(100);

        Serial.println(F(" Scanning for ADXL345..."));
        Wire.beginTransmission(ADXL345_ADDRESS);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            Serial.print(F(" ADXL345 found at I2C address: 0x"));
            Serial.println(ADXL345_ADDRESS, HEX);
        } else {
            Serial.print(F(" ADXL345 not detected at 0x"));
            Serial.println(ADXL345_ADDRESS, HEX);
            Serial.println(F("Check wiring (VCC, GND, SDA, SCL, CS HIGH, SDO GND)."));
        }

        // Enable measurement mode
        Wire.beginTransmission(ADXL345_ADDRESS);
        Wire.write(0x2D);  // Power control register
        Wire.write(0x08);  // Set measurement mode
        Wire.endTransmission();

        delay(100);
        calibrate();
    }

    inline void readRaw(int16_t &x, int16_t &y, int16_t &z) {
        Wire.beginTransmission(ADXL345_ADDRESS);
        Wire.write(0x32);  // Data start register
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)ADXL345_ADDRESS, (uint8_t)6);

        if (Wire.available() == 6) {
            x = Wire.read() | (Wire.read() << 8);
            y = Wire.read() | (Wire.read() << 8);
            z = Wire.read() | (Wire.read() << 8);
        }
    }

    inline void readAcceleration(float &xg, float &yg, float &zg) {
        int16_t x, y, z;
        readRaw(x, y, z);
        // Each LSB = 3.9 mg @ ±2g full resolution
        xg = x * 0.0039;
        yg = y * 0.0039;
        zg = z * 0.0039;
    }

    inline void calibrate() {
        Serial.println(F("📏 Calibrating accelerometer... Hold still!"));
        delay(2000);

        float xg, yg, zg;
        readAcceleration(xg, yg, zg);
        baseX = xg;
        baseY = yg;
        baseZ = zg;

        Serial.println(F("✅ Calibration complete."));
        Serial.print(F("Base values → X: ")); Serial.print(baseX, 3);
        Serial.print(F(" | Y: ")); Serial.print(baseY, 3);
        Serial.print(F(" | Z: ")); Serial.println(baseZ, 3);
    }

    inline bool isTampered(float threshold = 0.3) {
        float xg, yg, zg;
        readAcceleration(xg, yg, zg);

        float dx = fabs(xg - baseX);
        float dy = fabs(yg - baseY);
        float dz = fabs(zg - baseZ);

        if (dx > threshold || dy > threshold || dz > threshold) {
            Serial.println(F("⚠️ Tampering detected!"));
            return true;
        }
        return false;
    }

    inline void printAcceleration(bool showMagnitude = true) {
        float xg, yg, zg;
        readAcceleration(xg, yg, zg);

        Serial.print(F("X: "));
        Serial.print(xg, 3);
        Serial.print(F(" g | Y: "));
        Serial.print(yg, 3);
        Serial.print(F(" g | Z: "));
        Serial.print(zg, 3);
        Serial.print(F(" g"));

        if (showMagnitude) {
            float mag = sqrt(xg * xg + yg * yg + zg * zg);
            Serial.print(F(" | |A| = "));
            Serial.print(mag, 3);
            Serial.print(F(" g"));
        }
        Serial.println();
    }
};

#endif
