#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

class Accelerometer {
private:
    float baseX, baseY, baseZ;

public:
    inline void begin() {
        Wire.begin();
        delay(100);

        // 🔍 Scan for ADXL345
        Serial.println("Scanning for ADXL345...");
        Wire.beginTransmission(ADXL345_ADDRESS);
        uint8_t error = Wire.endTransmission();
        if (error == 0) {
            Serial.print("✅ ADXL345 found at I2C address: 0x");
            Serial.println(ADXL345_ADDRESS, HEX);
        } else {
            Serial.print("❌ ADXL345 not detected at address 0x");
            Serial.println(ADXL345_ADDRESS, HEX);
            Serial.println("Check wiring: VCC, GND, SDA, SCL, CS (HIGH), and SDO.");
        }

        // Enable measurement mode
        Wire.beginTransmission(ADXL345_ADDRESS);
        Wire.write(0x2D);  // Power control
        Wire.write(0x08);  // Measurement mode
        Wire.endTransmission();

        delay(100);
        calibrate();
    }

    inline void readRaw(int16_t &x, int16_t &y, int16_t &z) {
        Wire.beginTransmission(ADXL345_ADDRESS);
        Wire.write(0x32);
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
        xg = x * 0.0039; // 3.9 mg/LSB @ ±2g full resolution
        yg = y * 0.0039;
        zg = z * 0.0039;
    }

    inline void calibrate() {
        float xg, yg, zg;
        readAcceleration(xg, yg, zg);
        baseX = xg;
        baseY = yg;
        baseZ = zg;
        Serial.println("Accelerometer calibrated.");
    }

    inline bool isTampered(float threshold = 0.3) {
        float xg, yg, zg;
        readAcceleration(xg, yg, zg);

        float dx = fabs(xg - baseX);
        float dy = fabs(yg - baseY);
        float dz = fabs(zg - baseZ);

        if (dx > threshold || dy > threshold || dz > threshold) {
            Serial.println("⚠️  Tampering detected!");
            return true;
        }
        return false;
    }

    // 🆕 Print current acceleration readings
    inline void printAcceleration(bool showMagnitude = true) {
        float xg, yg, zg;
        readAcceleration(xg, yg, zg);

        Serial.print("X: ");
        Serial.print(xg, 3);
        Serial.print(" g | Y: ");
        Serial.print(yg, 3);
        Serial.print(" g | Z: ");
        Serial.print(zg, 3);
        Serial.print(" g");

        if (showMagnitude) {
            float magnitude = sqrt(xg * xg + yg * yg + zg * zg);
            Serial.print(" | |A| = ");
            Serial.print(magnitude, 3);
            Serial.print(" g");
        }

        Serial.println();
    }
};

#endif
