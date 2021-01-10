#ifndef TIMECONTROLLER_H
#define TIMECONTROLLER_H

#define ACTION_ON_RELE
#define ACTION_OFF_RELE

#define ACTION_CONFIGURE_TIME



#include <IRemote/src/IRremote.h>

class TimeController {

public:
    TimeController(int);
    bool init();

private:
    IRrecv* irrecv;
    decode_results results;

    bool isInConfTime;


    void onRele();
    void configureTime();
    void offRele();

    int CheckIfReciveCommand();





};


#endif
