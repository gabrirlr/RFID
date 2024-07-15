#include <Wiegand.h>

#define MAX_CARDS 100

// Estrutura para armazenar informações do cartão RFID
struct RFIDCard {
  uint32_t cardNumber;
};

// Classe para gerenciar o armazenamento dos cartões RFID
class RFIDStorage {
public:
  RFIDStorage();
  bool isCardRegistered(uint32_t cardNumber);
  void addCardToDatabase(uint32_t cardNumber);
  void printDatabase();

private:
  RFIDCard cardDatabase[MAX_CARDS];
  int cardCount;
};

// Implementação dos métodos da classe RFIDStorage
RFIDStorage::RFIDStorage() : cardCount(0) {}

bool RFIDStorage::isCardRegistered(uint32_t cardNumber) {
  for (int i = 0; i < cardCount; i++) {
    if (cardDatabase[i].cardNumber == cardNumber) {
      return true;
    }
  }
  return false;
}

void RFIDStorage::addCardToDatabase(uint32_t cardNumber) {
  if (cardCount < MAX_CARDS) {
    cardDatabase[cardCount].cardNumber = cardNumber;
    cardCount++;
    Serial.println("Card added to database.");
  } else {
    Serial.println("Database is full.");
  }
}

void RFIDStorage::printDatabase() {
  for (int i = 0; i < cardCount; i++) {
    Serial.print("Card ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(cardDatabase[i].cardNumber);
  }
}

// Classe para gerenciar a leitura dos cartões RFID
class RFIDReader {
public:
  RFIDReader(uint8_t pinD0, uint8_t pinD1);
  void begin();
  bool available();
  uint32_t getCode();
  int getWiegandType();

private:
  WIEGAND wg;
};

// Implementação dos métodos da classe RFIDReader
RFIDReader::RFIDReader(uint8_t pinD0, uint8_t pinD1) {
  wg.begin(pinD0, pinD1);
}

void RFIDReader::begin() {
  wg.begin();
}

bool RFIDReader::available() {
  return wg.available();
}

uint32_t RFIDReader::getCode() {
  return wg.getCode();
}

int RFIDReader::getWiegandType() {
  return wg.getWiegandType();
}

// Instâncias das classes RFIDStorage e RFIDReader
RFIDStorage storage;
RFIDReader reader(16, 17); // Defina os pinos de RX2 e TX2 para o leitor Wiegand

void setup() {
  Serial.begin(9600); // Comunicação serial para debug
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // Comunicação serial para o leitor Wiegand usando RX2 e TX2

  reader.begin();
}

void loop() {
  if (reader.available()) {
    uint32_t cardNumber = reader.getCode();
    Serial.print("Wiegand HEX = ");
    Serial.print(cardNumber, HEX);
    Serial.print(", DECIMAL = ");
    Serial.print(cardNumber);
    Serial.print(", Type W");
    Serial.println(reader.getWiegandType());

    if (!storage.isCardRegistered(cardNumber)) {
      storage.addCardToDatabase(cardNumber);
    } else {
      Serial.println("Card is already registered.");
    }
  }
}
