#include <Wiegand.h>
#include <EEPROM.h>
#include "RfidDb.h"

// Definições dos pinos conectados aos sinais D0 e D1 do Wiegand
#define WIEGAND_PIN_D0 16
#define WIEGAND_PIN_D1 17
#define RFID_LED_PIN 18           // Pino para o LED do leitor RFID
#define MODE_SWITCH_PIN 19        // Pino para alternar entre os modos (HIGH = Registro, LOW = Verificação)
#define CLEAR_DATA_PIN 21         // Pino para limpar todos os dados do banco de dados (LOW = Limpar)
#define PULSE_PIN 22
#define BUZZER_PIN 23             // Pino para o buzzer

// Objeto que gerencia o protocolo Wiegand
Wiegand wiegand;
RfidDb rfidDatabase = RfidDb(100, 0);  // Banco de dados RFID com capacidade ajustada

enum OperationMode {
  VERIFICATION_MODE,
  REGISTRATION_MODE,
  CLEAR_MODE
};

OperationMode currentMode = VERIFICATION_MODE;  // Modo inicial: Verificação
bool lastButtonState = LOW;   // Estado do botão na última verificação
volatile bool accessGranted = false; // Flag para sinalizar acesso concedido

void setup() {
  Serial.begin(9600);

  EEPROM.begin(rfidDatabase.dbSize());  // Inicializa a EEPROM no ESP32
  rfidDatabase.begin();

  // Inicializar leitores Wiegand
  wiegand.onReceive(receivedData, "Cartão lido: ");
  wiegand.onReceiveError(receivedDataError, "Erro na leitura do cartão: ");
  wiegand.onStateChange(stateChanged, "Estado alterado: ");
  wiegand.begin(Wiegand::LENGTH_ANY, true);

  setupPins();
  attachInterrupts();
  pinStateChanged();  // Enviar estado inicial dos pinos para a biblioteca Wiegand
}

void loop() {
  checkModeSwitch();
  checkClearData();
  wiegand.flush();

  if (accessGranted) {
    accessGranted = false; // Reset flag
    grantAccess();         // Ações de acesso concedido
  }

  delay(100);
}

void setupPins() {
  pinMode(WIEGAND_PIN_D0, INPUT);
  pinMode(WIEGAND_PIN_D1, INPUT);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  pinMode(CLEAR_DATA_PIN, INPUT_PULLUP);
  pinMode(PULSE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RFID_LED_PIN, OUTPUT);

  digitalWrite(PULSE_PIN, LOW);
  digitalWrite(BUZZER_PIN, HIGH);
  digitalWrite(RFID_LED_PIN, HIGH);
}

void attachInterrupts() {
  attachInterrupt(digitalPinToInterrupt(WIEGAND_PIN_D0), pinStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(WIEGAND_PIN_D1), pinStateChanged, CHANGE);
}

void checkModeSwitch() {
  bool buttonState = digitalRead(MODE_SWITCH_PIN);
  if (buttonState != lastButtonState) {
    delay(50); // Debounce
    buttonState = digitalRead(MODE_SWITCH_PIN);
    if (buttonState != lastButtonState) {
      toggleMode();
      lastButtonState = buttonState;
    }
  }
}

void checkClearData() {
  if (digitalRead(CLEAR_DATA_PIN) == LOW) {
    clearDatabase();
    delay(500); // Debounce
  }
}

void pinStateChanged() {
  wiegand.setPin0State(digitalRead(WIEGAND_PIN_D0));
  wiegand.setPin1State(digitalRead(WIEGAND_PIN_D1));
}

void stateChanged(bool plugged, const char* message) {
  Serial.print(message);
  Serial.println(plugged ? "CONECTADO" : "DESCONECTADO");
}

void receivedData(uint8_t* data, uint8_t bits, const char* message) {
  Serial.print(message);
  Serial.print(bits);
  Serial.print(" bits / ");

  uint32_t id = extractId(data, bits);
  Serial.print("ID: ");
  Serial.println(id);

  handleData(id);

  dumpState();
}

void receivedDataError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
  Serial.print(message);
  Serial.print(Wiegand::DataErrorStr(error));
  Serial.print(" - Dados brutos: ");
  Serial.print(rawBits);
  Serial.print(" bits / ");

  printRawData(rawData, rawBits);
}

uint32_t extractId(uint8_t* data, uint8_t bits) {
  uint32_t id = 0;
  uint8_t bytes = (bits + 7) / 8;
  for (int i = 0; i < bytes; i++) {
    id = (id << 8) | data[i];
  }
  return id;
}

void handleData(uint32_t id) {
  switch (currentMode) {
    case VERIFICATION_MODE:
      if (rfidDatabase.contains(id)) {
        accessGranted = true;
        Serial.println("Cartão encontrado no banco de dados.");
      } else {
        Serial.println("Cartão não encontrado no banco de dados.");
      }
      break;

    case REGISTRATION_MODE:
      if (rfidDatabase.insert(id, "Cartao")) {
        Serial.println("Cartão registrado com sucesso!");
      } else {
        Serial.println("Falha ao registrar o cartão.");
      }
      break;
  }
}

void printRawData(uint8_t* rawData, uint8_t rawBits) {
  uint8_t bytes = (rawBits + 7) / 8;
  for (int i = 0; i < bytes; i++) {
    Serial.print(rawData[i] >> 4, 16);
    Serial.print(rawData[i] & 0xF, 16);
  }
  Serial.println();
}

void dumpState() {
  uint8_t count = rfidDatabase.count();
  Serial.print("count = ");
  Serial.print(count);
  Serial.print(" [");

  for (int i = 0; i < count; i++) {
    uint32_t id;
    char name[rfidDatabase.maxNameLength()];
    if (rfidDatabase.getId(i, id)) {
      Serial.print(id);
      if (rfidDatabase.getName(i, name)) {
        Serial.print(":");
        Serial.print(name);
      }
      if (i < count - 1) {
        Serial.print(", ");
      }
    }
  }
  Serial.println("]");
}

void toggleMode() {
  currentMode = (currentMode == VERIFICATION_MODE) ? REGISTRATION_MODE : VERIFICATION_MODE;

  for (int i = 0; i < 3; i++) {
    digitalWrite(BUZZER_PIN, LOW);
    delay(50);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
  }

  Serial.println(currentMode == VERIFICATION_MODE ? "Modo de verificação ativado." : "Modo de registro ativado.");
}

void sendPulse(int pin) {
  digitalWrite(pin, HIGH);
  delay(500);
  digitalWrite(pin, LOW);
}

void clearDatabase() {
  Serial.println("Limpando todos os dados do banco de dados...");

  uint8_t count = rfidDatabase.count();
  for (int i = 0; i < count; i++) {
    uint32_t id;
    if (rfidDatabase.getId(0, id)) {
      rfidDatabase.remove(id);
    }
  }

  Serial.println("Banco de dados limpo.");

  digitalWrite(BUZZER_PIN, LOW);
  delay(750);
  digitalWrite(BUZZER_PIN, HIGH);
}

void grantAccess() {
  digitalWrite(RFID_LED_PIN, LOW);
  for (int i = 0; i < 1; i++) {
    digitalWrite(BUZZER_PIN, LOW);
    delay(250); // Tempo que o buzzer fica ligado
    digitalWrite(BUZZER_PIN, HIGH);
    delay(125); // Tempo que o buzzer fica desligado
  }

  digitalWrite(RFID_LED_PIN, HIGH);
}
