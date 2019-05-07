/*
 * HBWValve.h
 *
 * Created on: 05.05.2019
 * loetmeister.de
 *
 *
 *  15.02.2015
 *      Author: hglaser
 * Eine einfache Ventilsteuerung für mein Thermisches 24V Ventil. Ich habe
 * es mittels eines TIP 220 angesteuert, es geht natürlich auch über ein Relaismodul
 * oder ähnliches. Verwendet wird zur Umrechnung der Ventilstellung die von FHEM gesendet
 * wird, "time proportioning control" eine Art extrem langsames PWM. Bei über 50% schaltet
 * das Ventil zuerst ein, unter 50% zuerst aus. Nach einer Änderung wird die erste
 * Ein- oder Auszeit einmal halbiert
 *
 *
 *25 % ____|----|________|----|_______|
 *     aus  ein  aus      ein  aus...
 *50 % ____|--------|________|--------|____
 *     aus  ein      aus      ein      aus...
 *75 % ----|____|--------|____|--------
 *     ein  aus   ein      aus  ein...
 *
 */
 
#ifndef HBWVAVLE_H_
#define HBWVAVLE_H_


#include "HBWired.h"

#define DEBUG_OUTPUT   // debug output on serial/USB


#define OFF LOW
#define ON HIGH
#define VENTON true
#define VENTOFF false

#define MANUAL 0
#define AUTOMATIC 1

// need to match frame definition in XML:
#define SET_TOGGLE_AUTOMATIC  201
#define SET_MANUAL  203
#define SET_AUTOMATIC 205


// config of one valve channel, address step 7
struct hbw_config_valve {
  uint8_t logging:1;      // +0.0   1=on 0=off
  uint8_t unlocked:1;     // +0.1   0=LOCKED, 1=UNLOCKED
  uint8_t n_inverted:1;   // +0.2   inverted logic (use NO valves, NC is default)
  uint8_t :5;     //fillup //0x..:3-8
  uint16_t send_max_interval;   // Maximum-Sendeintervall
  uint8_t error_pos;
  uint16_t valveSwitchTime;   // Time the valve needs to reach 100% (NC:open or NO:closed state)
// TODO: option for anti stick? valve_protect (e.g. open valves once a week?)
  uint8_t dummy :8;
};


// Class HBWVavle
class HBWValve : public HBWChannel {
  public:
    HBWValve(uint8_t _pin, hbw_config_valve* _config);
    virtual void loop(HBWDevice*, uint8_t channel);
    virtual uint8_t get(uint8_t* data);
    virtual void set(HBWDevice*, uint8_t length, uint8_t const * const data, bool setByPID);
    virtual void set(HBWDevice*, uint8_t length, uint8_t const * const data);
    virtual void afterReadConfig();

    // linked PID channel access
    bool getPidsInAuto();
    void setPidsInAuto(bool newAuto);
    uint8_t getErrorPos();

  private:
    hbw_config_valve* config;
    uint8_t pin;

    uint8_t valveLevel;
    
    // output control
    inline void switchstate(int State);
    uint32_t set_timer(bool firstState, byte status);
    uint32_t set_peakmiddle(uint32_t ontimer, uint32_t offtimer);
    bool first_on_or_off(uint32_t ontimer, uint32_t offtimer);
    int init_new_state();
    uint32_t set_ontimer(uint8_t VentPositionRequested);
    uint32_t set_offtimer(uint32_t ontimer);
    uint32_t outputChangeLastTime;    // last time output state was changed
    uint32_t outputChangeNextDelay;    // time until next state change
    uint32_t onTimer, offTimer;     // current calculated on and of duration
    uint32_t lastSentTime;   // time of last send

    uint32_t lastFeedbackTime;  // when did we send the last feedback?
    uint16_t nextFeedbackDelay; // 0 -> no feedback pending
    
    struct valve_config {
//      uint8_t inAuto  :1; // 1 = automatic ; 0 = manual
//      uint8_t upDown  :1; // Pid regelt hoch oder runter
//      uint8_t status  :1; // outputs on or off?
      uint8_t initDone :1;
      uint8_t firstState:1;
      uint8_t nextState :1;
    } valveConf;

    //boolean initDone;

    union tag_state_flags {
      struct state_flags {
        uint8_t notUsed :4;
        uint8_t upDown  :1; // Pid regelt hoch oder runter
        uint8_t inAuto  :1; // 1 = automatic ; 0 = manual
        uint8_t status  :1; // outputs on or off?
      } element;
      uint8_t byte:8;
    } stateFlags;
    
    
};

#endif /* HBWVAVLE_H_ */
