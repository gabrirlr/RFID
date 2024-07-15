#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>

// Definir constantes para Wiegand 26
const int DATA_PIN = 2; // Substituir pelo seu pino de dados
const int CLOCK_PIN = 3; // Substituir pelo seu pino de clock
const int DATA_BIT_COUNT = 26;

// Estrutura de dados do cartão RFID
struct RFIDCard {
  uint32_t cardID; // Armazenar o ID do cartão de 26 bits como um inteiro de 32 bits
  // Outros dados do cartão (por exemplo, proprietário, nível de acesso)
};

// Classe de armazenamento de dados RFID
class RFIDDataStore {
  public:
    RFIDDataStore(int eepromSize) {
      // Inicializar a EEPROM com o tamanho especificado
    }

    void addCard(RFIDCard card) {
      // Armazenar dados do cartão na EEPROM
    }

    RFIDCard getCard(uint32_t cardID) {
      // Recuperar dados do cartão da EEPROM
    }

    // ... outros métodos para atualizar, excluir e gerenciar cartões
};

// Função para ler dados Wiegand 26
uint32_t readWiegandData() {
  // Implementar lógica para ler bits de dados dos pinos de dados e clock
  // Calcular o ID do cartão a partir dos dados lidos
  return cardID;
}

void setup() {
  // Inicializar pinos, leitor RFID e EEPROM
}

void loop() {
  uint32_t cardID = readWiegandData();
  if (cardID != 0) {
    // Processar o ID do cartão (por exemplo, verificar se o cartão está autorizado, armazenar dados do cartão)
  }
}
