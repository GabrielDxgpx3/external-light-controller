#include <Arduino.h>
#include <Wire.h>
#include <IRremote.h>
#include <RTClib.h>

#define HORA_LIGAR 15
#define MINUTO_LIGAR 40
#define SEGUNDO_LIGAR 00

#define HORA_DESLIGAR 15
#define MINUTO_DESLIGAR 52
#define SEGUNDO_DESLIGAR 00

#define CODE_ON 0x20DF8E71
#define CODE_OFF 0x20DF4EB1

#define ATUADOR_PIN 8
#define IR_PIN 11

struct timeTrigger{
    int hora;
    int minuto;
    int segundo;
};

struct Atuador{
    const int pin;

    Atuador(int pin) : pin(pin){
        pinMode(pin, OUTPUT);
    }

    void on() const {
        //Serial.println("ligou rele");
        digitalWrite(pin, HIGH);
    }

    void off() const {
        //Serial.println("Desligou rele");
        digitalWrite(pin, LOW);
    }

    bool isOn() const {
        return digitalRead(pin);
    }

    bool isOff() const {
        return !digitalRead(pin);
    }
};

timeTrigger desligar{
    HORA_DESLIGAR,
    MINUTO_DESLIGAR,
    SEGUNDO_DESLIGAR
};

timeTrigger ligar{
    HORA_LIGAR,
    MINUTO_LIGAR,
    SEGUNDO_LIGAR
};

Atuador rele{
    ATUADOR_PIN
};

RTC_DS3231 rtc;
IRrecv irReceiver(IR_PIN);
decode_results iRresults;
DateTime now;

bool isInTriggerTime(){
    now = RTC_DS3231::now();
    return now.hour() >= ligar.hora && (now.hour() <= desligar.hora && now.minute() < desligar.minuto);
}

void setup(){
    //Serial.begin(9600);
    irReceiver.enableIRIn();

    if(!rtc.begin()){
        while(1);
    }

    if(isInTriggerTime() && rele.isOff()){
        rele.on();
    }else if(!isInTriggerTime() && rele.isOn()){
        rele.off();
    }

    delay(100);
}

bool isTimeOf(timeTrigger* trigger){
    now = RTC_DS3231::now();
    return trigger->segundo == now.second() && trigger->minuto == now.minute() && trigger->hora == now.hour();;
}

void readIr(){
    if(irReceiver.decode(&iRresults)){

        switch(iRresults.value){
            case CODE_ON:
                rele.on();
                break;
            case CODE_OFF:
                rele.off();
                break;
        }

        irReceiver.resume();
    }
}

void loop(){
    now = RTC_DS3231::now();

    //Serial.println(String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()));

    readIr();

    if(isTimeOf(&ligar)){
        rele.on();
    }

    if(isTimeOf(&desligar)){
        rele.off();
    }

    delay(20);
}