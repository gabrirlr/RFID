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
void setup(){

}
void loop(){
  
}