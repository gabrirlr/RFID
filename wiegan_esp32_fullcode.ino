#include <Wiegand.h>

#define MAX_CARDS 100

struct RFIDCard {
  uint32_t cardNumber;
};

WIEGAND wg;

// Banco de dados simulado (pode ser uma lista ou array)
RFIDCard cardDatabase[MAX_CARDS];
int cardCount = 0;

void setup() {
  Serial.begin(9600);

  // Inicializa o Wiegand com os pinos padrão (D2 e D3 no Arduino Uno)
  // ou utilize wg.begin(pinD0, pinD1) para placas não UNO.
  wg.begin();
}

void loop() {
  // Verifica se um novo cartão foi lido
  if (wg.available()) {
    uint32_t cardNumber = wg.getCode();
    Serial.print("Wiegand HEX = ");
    Serial.print(cardNumber, HEX);
    Serial.print(", DECIMAL = ");
    Serial.print(cardNumber);
    Serial.print(", Type W");
    Serial.println(wg.getWiegandType());

    // Verifica se o cartão está registrado no banco de dados
    if (!isCardRegistered(cardNumber)) {
      // Se não estiver, adiciona o cartão ao banco de dados
      addCardToDatabase(cardNumber);
    } else {
      Serial.println("Card is already registered.");
    }
  }
}

// Função para verificar se um cartão está registrado no banco de dados
bool isCardRegistered(uint32_t cardNumber) {
  for (int i = 0; i < cardCount; i++) {
    if (cardDatabase[i].cardNumber == cardNumber) {
      return true;
    }
  }
  return false;
}

// Função para adicionar um cartão ao banco de dados
void addCardToDatabase(uint32_t cardNumber) {
  if (cardCount < MAX_CARDS) {
    cardDatabase[cardCount].cardNumber = cardNumber;
    cardCount++;
    Serial.println("Card added to database.");
  } else {
    Serial.println("Database is full.");
  }
}
