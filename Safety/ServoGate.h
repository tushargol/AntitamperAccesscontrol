#ifndef SERVOGATE_H
#define SERVOGATE_H

#include <Arduino.h>
#include <Servo.h>
#include "config.h"

class ServoGate {
private:
    Servo servo;
    int openAngle;
    int closedAngle;

public:
    inline ServoGate(int closedPos = 0, int openPos = 90)
        : closedAngle(closedPos), openAngle(openPos) {}

    inline void begin() {
        servo.attach(SERVO_PIN);
        servo.write(closedAngle);
        delay(300);
    }

    inline void openGate() {
        Serial.println("Gate Opening...");
        for (int pos = closedAngle; pos <= openAngle; pos++) {
            servo.write(pos);
            delay(10); // smooth motion
        }
        delay(1000); // hold open
    }

    inline void closeGate() {
        Serial.println("Gate Closing...");
        for (int pos = openAngle; pos >= closedAngle; pos--) {
            servo.write(pos);
            delay(10);
        }
        delay(300);
    }

    inline void unlockSequence() {
        openGate();
        delay(2000);  // keep gate open for 2 seconds
        closeGate();
        Serial.println("Gate Locked.");
    }
};

#endif