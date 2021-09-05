#include <Arduino.h>
#include <Wire.h>
#include <IRremote.h>
#include <RTClib.h>
#include <EEPROM.h>

#define DELTA_EMERGENCY_TIME 30

#define HORA_LIGAR 18
#define MINUTO_LIGAR 00
#define SEGUNDO_LIGAR 00

#define HORA_DESLIGAR 23
#define MINUTO_DESLIGAR 20
#define SEGUNDO_DESLIGAR 00

#define CODE_ON 0x20DF8E71
#define CODE_OFF 0x20DF4EB1

#define ATUADOR_PIN 8
#define IR_PIN 11
//#define DEBUG 1
//#define AJUST_TIME 1

struct EEPromTime{
    int hour;
    int minute;
    int second;
};

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
        #ifdef DEBUG
            Serial.println("ligou rele");
        #endif

        digitalWrite(pin, HIGH);
    }

    void off() const {
        #ifdef DEBUG
            Serial.println("Desligou rele");
        #endif
    
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
byte emergencyMode = 0;

bool isInTriggerTime(){
    now = RTC_DS3231::now();
    return now.hour() >= ligar.hora && (now.hour() <= desligar.hora && now.minute() < desligar.minuto);
}

EEPromTime getEEPromTime(){

    EEPromTime time{
        EEPROM.read(0),
        EEPROM.read(1),
        EEPROM.read(2),
    };

    return time;
}

bool isEmergencyMode(){
    now = RTC_DS3231::now();
    EEPromTime time = getEEPromTime();

    #ifdef DEBUG
        Serial.println("EEPROM HOUR: " + String(time.hour));
        Serial.println("EEPROM MINUTE: " + String(time.minute));
        Serial.println("EEPROM SECOND: " + String(time.second));
    #endif  

    return now.hour() == time.hour && now.minute() == time.minute && (now.second() - time.second) <= DELTA_EMERGENCY_TIME;
}

void saveLastPowerOn(){
    now = RTC_DS3231::now();

    EEPROM.write(0, now.hour());
    EEPROM.write(1, now.minute());
    EEPROM.write(2, now.second());
}

void emergencyModeActions(){

    #ifdef DEBUG
        Serial.println("Rele ligado!");
    #endif

    rele.on();
    delay(200);
    rele.off();
    delay(200);
    rele.on();
    saveLastPowerOn();
}

void setup(){

    #ifdef DEBUG
        Serial.begin(9600);
    #endif

    if(!rtc.begin()){
        while(1);
    }

    #ifdef AJUST_TIME
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    #endif

    if(isEmergencyMode()){
        emergencyMode = 1;
        emergencyModeActions();

        #ifdef DEBUG
            Serial.println("EMERGENCY MODE!!");
        #endif

    }else{

        #ifdef DEBUG
            Serial.println("NORMAL MODE!!");
        #endif

        saveLastPowerOn();

        irReceiver.enableIRIn();

        if(isInTriggerTime() && rele.isOff()){
            rele.on();
        }else if(!isInTriggerTime() && rele.isOn()){
            rele.off();
        }

        delay(100);

    }
}

bool isTimeOf(timeTrigger* trigger){
    now = RTC_DS3231::now();
    return trigger->segundo == now.second() && trigger->minuto == now.minute() && trigger->hora == now.hour();
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

    while(emergencyMode){}

    now = RTC_DS3231::now();

    #ifdef DEBUG
        Serial.println(String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()));
    #endif

    readIr();

    if(isTimeOf(&ligar)){
        rele.on();
    }

    if(isTimeOf(&desligar)){
        rele.off();
    }

    delay(20);
}