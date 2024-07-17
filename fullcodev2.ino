#include <Arduino.h>
#include <Wiegand.h>
#include <EEPROM.h>

// Pin definitions
#define WIEGAND_PIN_D0 16
#define WIEGAND_PIN_D1 17
#define RFID_LED_PIN 18
#define MODE_SWITCH_PIN 19
#define CLEAR_DATA_PIN 21
#define PULSE_PIN 22
#define BUZZER_PIN 23

// Forward declaration of classes
class RfidDatabase;
class WiegandManager;

// Global instances
RfidDatabase rfidDatabase(100, 0);
WiegandManager wiegandManager(WIEGAND_PIN_D0, WIEGAND_PIN_D1);

// Operation modes
enum OperationMode {
  VERIFICATION_MODE,
  REGISTRATION_MODE,
  CLEAR_MODE
};

OperationMode currentMode = VERIFICATION_MODE;
bool lastModeSwitchState = LOW;
volatile bool accessGranted = false;

// RfidDatabase class for managing RFID database
class RfidDatabase {
public:
  RfidDatabase(int maxSize, int initialSize) : maxSize(maxSize), initialSize(initialSize) {}

  void begin() {
    EEPROM.begin(dbSize());
  }

  bool insert(uint32_t id, const char* name) {
    // Implementation to insert into database
    return true;  // Replace with actual logic
  }

  bool contains(uint32_t id) {
    // Implementation to check if ID is in database
    return true;  // Replace with actual logic
  }

  void remove(uint32_t id) {
    // Implementation to remove ID from database
  }

  uint8_t count() {
    // Implementation to count number of IDs in database
    return 0;  // Replace with actual logic
  }

  uint8_t maxNameLength() {
    return 10;  // Example max name length, replace with actual logic
  }

  uint32_t dbSize() {
    return maxSize * sizeof(uint32_t) + initialSize;
  }

  void clearDatabase() {
    Serial.println("Clearing all data from database...");

    uint8_t count = this->count();
    for (int i = 0; i < count; i++) {
      uint32_t id;
      if (getId(0, id)) {
        remove(id);
      }
    }

    Serial.println("Database cleared.");

    digitalWrite(BUZZER_PIN, LOW);
    delay(750);
    digitalWrite(BUZZER_PIN, HIGH);
  }

private:
  int maxSize;
  int initialSize;

  bool getId(int index, uint32_t& id) {
    // Implementation to get ID from database by index
    return true;  // Replace with actual logic
  }

  bool getName(int index, char* name) {
    // Implementation to get name from database by index
    return true;  // Replace with actual logic
  }
};

// WiegandManager class for handling Wiegand protocol
class WiegandManager {
public:
  WiegandManager(int pinD0, int pinD1) : pinD0(pinD0), pinD1(pinD1) {}

  void begin() {
    wiegand.onReceive(receivedData, "Card read: ");
    wiegand.onReceiveError(receivedDataError, "Card read error: ");
    wiegand.onStateChange(stateChanged, "State changed: ");
    wiegand.begin(Wiegand::LENGTH_ANY, true);

    pinMode(pinD0, INPUT);
    pinMode(pinD1, INPUT);

    attachInterrupt(digitalPinToInterrupt(pinD0), pinStateChanged, CHANGE);
    attachInterrupt(digitalPinToInterrupt(pinD1), pinStateChanged, CHANGE);
  }

  void flush() {
    wiegand.flush();
  }

  void setPin0State(bool state) {
    wiegand.setPin0State(state);
  }

  void setPin1State(bool state) {
    wiegand.setPin1State(state);
  }

  void onReceive(void (*callback)(uint8_t*, uint8_t, const char*)) {
    receivedCallback = callback;
  }

  void onReceiveError(void (*callback)(Wiegand::DataError, uint8_t*, uint8_t, const char*)) {
    errorCallback = callback;
  }

private:
  Wiegand wiegand;
  static void receivedData(uint8_t* data, uint8_t bits, const char* message) {
    if (receivedCallback) {
      receivedCallback(data, bits, message);
    }
  }

  static void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
    if (errorCallback) {
      errorCallback(error, rawData, rawBits, message);
    }
  }

  static void stateChanged(bool plugged, const char* message) {
    Serial.print(message);
    Serial.println(plugged ? "CONNECTED" : "DISCONNECTED");
  }

  static void pinStateChanged() {
    if (wiegandManager) {
      wiegandManager->wiegand.setPin0State(digitalRead(wiegandManager->pinD0));
      wiegandManager->wiegand.setPin1State(digitalRead(wiegandManager->pinD1));
    }
  }

  static void (*receivedCallback)(uint8_t*, uint8_t, const char*);
  static void (*errorCallback)(Wiegand::DataError, uint8_t*, uint8_t, const char*);

  int pinD0;
  int pinD1;

public:
  static void setWiegandManager(WiegandManager* manager) {
    wiegandManager = manager;
  }
};

void setup() {
  Serial.begin(9600);

  rfidDatabase.begin();
  wiegandManager.begin();

  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(CLEAR_DATA_PIN, INPUT_PULLUP);
  pinMode(PULSE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RFID_LED_PIN, OUTPUT);

  digitalWrite(PULSE_PIN, LOW);
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(RFID_LED_PIN, HIGH);
}

void loop() {
  checkModeSwitch();
  checkClearData();
  wiegandManager.flush();

  if (accessGranted) {
    accessGranted = false;
    grantAccess();
  }

  delay(100);
}

void checkModeSwitch() {
  bool currentModeSwitchState = digitalRead(MODE_SWITCH_PIN);
  if (currentModeSwitchState != lastModeSwitchState) {
    delay(50); // Debounce
    currentModeSwitchState = digitalRead(MODE_SWITCH_PIN);
    if (currentModeSwitchState != lastModeSwitchState) {
      toggleMode();
      lastModeSwitchState = currentModeSwitchState;
    }
  }
}

void checkClearData() {
  if (digitalRead(CLEAR_DATA_PIN) == LOW) {
    rfidDatabase.clearDatabase();
    delay(500); // Debounce
  }
}

void toggleMode() {
  currentMode = (currentMode == VERIFICATION_MODE) ? REGISTRATION_MODE : VERIFICATION_MODE;

  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, LOW);
    delay(50);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
  }

  Serial.println(currentMode == VERIFICATION_MODE ? "Verification mode activated." : "Registration mode activated.");
}

void grantAccess() {
  digitalWrite(RFID_LED_PIN, LOW);
  for (int i = 0; i < 1; i++) {
    digitalWrite(BUZZER_PIN, LOW);
    delay(250); // Buzzer on time
    digitalWrite(BUZZER_PIN, HIGH);
    delay(125); // Buzzer off time
  }

  digitalWrite(RFID_LED_PIN, HIGH);
}

