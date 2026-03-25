# Anti-Tamper Access Control System

Arduino-based security system with RFID authentication, keypad entry, and tamper detection using accelerometer.

## Features

- **RFID Authentication**: Primary security layer using RC522 module
- **Keypad Entry**: Secondary authentication with 4x4 matrix keypad
- **Tamper Detection**: ADXL345 accelerometer monitors physical tampering
- **Servo Gate Control**: Automated gate/door mechanism
- **Audio Feedback**: Buzzer provides status notifications
- **ESP32 Integration**: WiFi connectivity and Telegram alerts (optional)

## Hardware Requirements

- Arduino Uno/Nano
- RC522 RFID Module
- 4x4 Matrix Keypad
- ADXL345 Accelerometer
- Servo Motor
- Buzzer
- ESP32 (for WiFi features)

## Pin Configuration

| Component | Pin |
|-----------|-----|
| Keypad Rows | D6-D9 |
| Keypad Cols | D2-D5 |
| RFID SS | D10 |
| RFID RST | A2 |
| Servo | A1 |
| Buzzer | A0 |
| Accelerometer | A4 (SDA), A5 (SCL) |

## Setup

1. **Hardware Assembly**: Connect components according to pin configuration
2. **Configuration**: 
   - Copy `config.h.example` to `config.h` and set your passcode
   - Copy `espconfig.h.example` to `espconfig.h` and configure WiFi/Telegram settings
3. **RFID Setup**: Update `AUTH_UID` in main sketch with your card's UID
4. **Upload**: Flash the appropriate sketch to your Arduino/ESP32

## Security Features

- Dual authentication (RFID + Keypad)
- Real-time tamper monitoring
- Access logging
- Remote alerts via Telegram
- Time-based access control

## File Structure

```
├── Safety/           # Main Arduino sketch
├── esp/             # ESP32 variant with WiFi
├── config.h         # Hardware configuration (not tracked)
└── espconfig.h      # WiFi/API credentials (not tracked)
```

## Configuration Files

**Important**: Configuration files containing sensitive data are excluded from version control. Create your own:

- `config.h` - Hardware pins and passcode
- `espconfig.h` - WiFi credentials and API keys

## Usage

1. System starts in locked state
2. Present authorized RFID card
3. Enter 4-digit passcode followed by '#'
4. Gate opens for 2 seconds then closes
5. System resets for next use

## Status Indicators

- **Single beep**: RFID verified
- **Double beep**: Access granted
- **Triple beep**: Access denied
- **Rapid beeps**: Tamper alarm
