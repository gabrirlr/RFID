#include <Wiegand.h>

#define MAX_CARDS 100
#define READ_PIN 13
#define REGISTER_PIN 12
#define DELETE_PIN 14
#define PULSE_PIN 17

class Database {
public:
    Database() : cardCount(0) {}

    bool isCardRegistered(uint32_t cardNumber) {
        for (int i = 0; i < cardCount; i++) {
            if (cardDatabase[i].cardNumber == cardNumber) {
                return true;
            }
        }
        return false;
    }

    void addCard(uint32_t cardNumber) {
        if (cardCount < MAX_CARDS) {
            cardDatabase[cardCount].cardNumber = cardNumber;
            cardCount++;
            Serial.println("Card added to database.");
        } else {
            Serial.println("Database is full.");
        }
    }

    void clearDatabase() {
        cardCount = 0;
        Serial.println("Database cleared.");
    }

private:
    struct RFIDCard {
        uint32_t cardNumber;
    };

    RFIDCard cardDatabase[MAX_CARDS];
    int cardCount;
};

class Reader {
public:
    Reader() {
        wg.begin();
    }

    bool available() {
        return wg.available();
    }

    uint32_t getCardNumber() {
        return wg.getCode();
    }

private:
    WIEGAND wg;
};

class PulseSender {
public:
    PulseSender(uint8_t pin) : pulsePin(pin) {
        pinMode(pulsePin, OUTPUT);
        digitalWrite(pulsePin, LOW);
    }

    void sendPulse(uint32_t duration) {
        digitalWrite(pulsePin, HIGH);
        delay(duration);
        digitalWrite(pulsePin, LOW);
    }

private:
    uint8_t pulsePin;
};

Database db;
Reader reader;
PulseSender pulseSender(PULSE_PIN);

void setup() {
    Serial.begin(9600);
    pinMode(REGISTER_PIN, INPUT_PULLUP);
    pinMode(DELETE_PIN, INPUT_PULLUP);
}

void loop() {
    bool isRegistering = !digitalRead(REGISTER_PIN);
    bool isDeleting = !digitalRead(DELETE_PIN);

    if (isDeleting) {
        db.clearDatabase();
        delay(1000); // Debounce
    } else if (isRegistering) {
        if (reader.available()) {
            uint32_t cardNumber = reader.getCardNumber();
            Serial.print("Registering card number: ");
            Serial.println(cardNumber);
            db.addCard(cardNumber);
        }
    } else {
        if (reader.available()) {
            uint32_t cardNumber = reader.getCardNumber();
            Serial.print("Reading card number: ");
            Serial.println(cardNumber);
            if (db.isCardRegistered(cardNumber)) {
                Serial.println("Card is registered. Sending pulse.");
                pulseSender.sendPulse(500);
            } else {
                Serial.println("Card is not registered.");
            }
        }
    }
}

