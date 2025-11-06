#ifndef RFIDMODULE_H
#define RFIDMODULE_H

#include "Config.h"
#include <SPI.h>
#include <MFRC522.h>

class RFIDModule {
public:
  RFIDModule(uint8_t ssPin = RFID_SS_PIN, uint8_t rstPin = RFID_RST_PIN)
      : _ssPin(ssPin), _rstPin(rstPin), _mfrc(ssPin, rstPin), _lastUID("") {}

  inline void begin(long baud = 9600) {
    Serial.begin(baud);
    SPI.begin();
    _mfrc.PCD_Init();
    Serial.println("RFIDModule initialized. Approximate your card to the reader...");
    Serial.println();
  }

  inline bool pollCard() {
    if (!_mfrc.PICC_IsNewCardPresent()) return false;
    if (!_mfrc.PICC_ReadCardSerial()) return false;
    _lastUID = formatUID();
    return true;
  }

  inline String getUID() const { return _lastUID; }

  inline bool verifyCard(const String &expectedUID) const {
    String a = normalize(_lastUID);
    String b = normalize(expectedUID);
    return a.equalsIgnoreCase(b);
  }

private:
  uint8_t _ssPin;
  uint8_t _rstPin;
  MFRC522 _mfrc;
  String _lastUID;

  inline String formatUID() {
    String s = "";
    for (byte i = 0; i < _mfrc.uid.size; i++) {
      if (_mfrc.uid.uidByte[i] < 0x10)
        s += " 0";
      else
        s += " ";
      char buf[3];
      sprintf(buf, "%02X", _mfrc.uid.uidByte[i]);
      s += String(buf);
    }
    s.trim();
    s.toUpperCase();
    return s;
  }

  inline static String normalize(String s) {
    s.trim();
    s.toUpperCase();
    String out = "";
    bool lastSpace = false;
    for (unsigned int i = 0; i < s.length(); i++) {
      char c = s.charAt(i);
      if (isspace(c)) {
        if (!lastSpace) {
          out += ' ';
          lastSpace = true;
        }
      } else {
        out += c;
        lastSpace = false;
      }
    }
    out.trim();
    return out;
  }
};

#endif // RFIDMODULE_H
