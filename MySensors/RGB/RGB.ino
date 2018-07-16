/**
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   version 2 as published by the Free Software Foundation.

   LED STRIP sketch for Mysensors
 *******************************

   REVISION HISTORY
   1.0
     Based on the example sketch in mysensors
   1.1
     fadespeed parameter (send as V_VAR1 message)
     HomeAssistant compatible (send status to ack)
   1.2
     OTA support
   1.3
     Power-on self test
   1.4
     Bug fix
   1.5
     Other default values
   1.6
     Repeater feature
   1.7
     Multitasking. Alarm, Relax and normal modes.
*/
//#define MY_NODE_ID            3
#define MY_OTA_FIRMWARE_FEATURE
//#define MY_REPEATER_FEATURE
#define MY_NODE_ID AUTO

#define MY_RADIO_NRF24

#define MY_DEBUG

#include <MySensors.h>

#define CHILD_ID_LIGHT 1

#define SN "LED Strip"
#define SV "1.7"

MyMessage lightMsg(CHILD_ID_LIGHT, V_LIGHT);
MyMessage rgbMsg(CHILD_ID_LIGHT, V_RGB);
MyMessage dimmerMsg(CHILD_ID_LIGHT, V_DIMMER);
MyMessage msgRelay1(CHILD_ID_LIGHT + 1, V_LIGHT);
MyMessage msgRelay2(CHILD_ID_LIGHT + 2, V_LIGHT);

int current_r = 255;
int current_g = 255;
int current_b = 255;
int target_r = 255;
int target_g = 255;
int target_b = 255;
int save_r;
int save_g;
int save_b;


float delta_r = 0.0;
float delta_g = 0.0;
float delta_b = 0.0;

char rgbstring[] = "ffffff";

int on_off_status = 0;
int dimmerlevel = 100;
int fadespeed = 20;
unsigned long last_update = 0;
int tick_length = 10;
int fade_step = 0;

int program_timer;
int program_cycle;

#define REDPIN 6
#define GREENPIN 5
#define BLUEPIN 3

#define LIGHT_NORMAL 0
#define LIGHT_FADING 1

#define PROGRAM_NORMAL 0
#define PROGRAM_ALARM 1
#define PROGRAM_RELAX 2

int light_mode = LIGHT_NORMAL;
int program_mode = PROGRAM_NORMAL;

#define RELAX_SPEED 50
#define MAX_CYCLES_RELAX 7
const int program_param_RELAX[MAX_CYCLES_RELAX][3] = {
  {255, 32, 0},
  {255, 32, 16},
  {255, 16, 32},
  {255, 128, 0},
  {255, 32, 00},
  {255, 32, 32},
  {255, 0, 32}
};

#define RELAY1PIN 8
#define RELAY2PIN 7

#define RELAY_ON 0  // GPIO value to write to turn on attached relay
#define RELAY_OFF 1 // GPIO value to write to turn off attached relay


void setup()
{
  // Fix the PWM timer. Without this the LEDs will flicker.
  TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM00);

  // Output pins
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(RELAY1PIN, OUTPUT);
  pinMode(RELAY2PIN, OUTPUT);
  digitalWrite(RELAY1PIN,  RELAY_OFF);
  digitalWrite(RELAY2PIN,  RELAY_OFF);
  send(msgRelay1.set(false ), true);
  send(msgRelay2.set(false ), true);
}

void presentation()
{
  // Send the Sketch Version Information to the Gateway
  sendSketchInfo(SN, SV);
  present(CHILD_ID_LIGHT, S_RGB_LIGHT);
  present(CHILD_ID_LIGHT + 1, S_BINARY);
  present(CHILD_ID_LIGHT + 2, S_BINARY);
}

void selftest() {
  on_off_status = 1;
  current_r = 255;
  current_g = 0;
  current_b = 0;
  set_hw_status();
  wait(100);
  current_r = 0;
  current_g = 255;
  set_hw_status();
  wait(100);
  current_g = 0;
  current_b = 255;
  set_hw_status();
  wait(100);
  current_r = 255;
  current_g = 255;
  set_hw_status();
  wait(100);
  on_off_status = 0;
}


void loop()
{
  static bool first_message_sent = false;
  if ( first_message_sent == false ) {
    selftest();
    set_hw_status();
    send_status(1, 1, 1);
    first_message_sent = true;
  }

  unsigned long now = millis();

  if (now - last_update > tick_length) {
    last_update = now;

    if (light_mode == LIGHT_FADING) {
      calc_fade();
    }

    if (program_mode > PROGRAM_NORMAL) {
      handle_program();
    }

  }
  set_hw_status();

}

void receive(const MyMessage &message)
{
  int val;

  if (message.type == V_RGB) {
    Serial.print( "V_RGB: " );
    Serial.println(message.data);
    long number = (long) strtol( message.data, NULL, 16);

    // Save old value
    strcpy(rgbstring, message.data);

    // Split it up into r, g, b values
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;

    init_fade(fadespeed, r, g, b);
    send_status(0, 0, 1);

  } else if (message.type == V_LIGHT || message.type == V_STATUS) {
    if (message.sensor == CHILD_ID_LIGHT) {
      Serial.print( "V_LIGHT: " );
      Serial.println(message.data);
      val = atoi(message.data);
      if (val == 0 or val == 1) {
        on_off_status = val;
        send_status(1, 0, 0);
      }
    } else if (message.sensor == CHILD_ID_LIGHT + 1) {
      digitalWrite(RELAY1PIN, message.getBool() ? RELAY_ON : RELAY_OFF);
    }
    else if (message.sensor == CHILD_ID_LIGHT + 2) {
      digitalWrite(RELAY2PIN, message.getBool() ? RELAY_ON : RELAY_OFF);
    }

  } else if (message.type == V_DIMMER || message.type == V_PERCENTAGE) {
    Serial.print( "V_DIMMER: " );
    Serial.println(message.data);
    val = atoi(message.data);
    if (val >= 0 and val <= 100) {
      dimmerlevel = val;
      send_status(0, 1, 0);
    }

  } else if (message.type == V_VAR1 ) {
    Serial.print( "V_VAR1: " );
    Serial.println(message.data);
    val = atoi(message.data);
    if (val >= 0 and val <= 2000) {
      fadespeed = val;
    }

  } else if (message.type == V_VAR2 ) {
    Serial.print( "V_VAR2: " );
    Serial.println(message.data);
    val = atoi(message.data);
    if (val == PROGRAM_NORMAL) {
      stop_program();
    } else if (val == PROGRAM_ALARM || val == PROGRAM_RELAX) {
      init_program(val);
    }

  } else {
    Serial.println( "Invalid command received..." );
    return;
  }

}

void set_rgb(int r, int g, int b) {
  analogWrite(REDPIN, r);
  analogWrite(GREENPIN, g);
  analogWrite(BLUEPIN, b);
}

void init_program(int program) {
  program_mode = program;
  program_cycle = 0;
  save_rgb();

  if (program == PROGRAM_ALARM) {
    light_mode = LIGHT_NORMAL;
    current_r = 255;
    current_g = 255;
    current_b = 255;
    program_timer = 50;
  } else if (program == PROGRAM_RELAX) {
    program_timer = 300;
    init_fade(fadespeed,
              program_param_RELAX[program_cycle][0],
              program_param_RELAX[program_cycle][1],
              program_param_RELAX[program_cycle][2]);
  }
}

void handle_program() {
  program_timer--;
  if (program_mode == PROGRAM_ALARM) {
    if (program_timer == 0) {
      program_timer = 50;
      if (program_cycle == 0) {
        program_cycle = 1;
        current_r = 0;
        current_g = 0;
        current_b = 0;
      } else {
        program_cycle = 0;
        current_r = 255;
        current_g = 255;
        current_b = 255;
      }
    }
  } else if (program_mode == PROGRAM_RELAX) {
    if (light_mode == LIGHT_NORMAL) {
      program_cycle = (program_cycle + 1) % MAX_CYCLES_RELAX;
      Serial.print("Next cycle step ");
      Serial.println(program_cycle);
      init_fade(fadespeed * RELAX_SPEED,
                program_param_RELAX[program_cycle][0],
                program_param_RELAX[program_cycle][1],
                program_param_RELAX[program_cycle][2]);

    }

  }
}

void stop_program() {
  restore_rgb();
  light_mode = LIGHT_NORMAL;
  program_mode = PROGRAM_NORMAL;
}

void save_rgb() {
  save_r = current_r;
  save_g = current_g;
  save_b = current_b;
}

void restore_rgb() {
  current_r = save_r;
  current_g = save_g;
  current_b = save_b;
}
void init_fade(int t, int r, int g, int b) {
  Serial.print( "Init fade" );
  light_mode = LIGHT_FADING;
  target_r = r;
  target_g = g;
  target_b = b;
  fade_step = t;
  delta_r = (target_r - current_r) / float(fade_step);
  delta_g = (target_g - current_g) / float(fade_step);
  delta_b = (target_b - current_b) / float(fade_step);
}

void calc_fade() {
  if (fade_step > 0) {
    fade_step--;
    current_r = target_r - delta_r * fade_step;
    current_g = target_g - delta_g * fade_step;
    current_b = target_b - delta_b * fade_step;
  } else {
    Serial.println( "Normal mode" );
    light_mode = LIGHT_NORMAL;
  }
}

void set_hw_status() {
  int r = on_off_status * (int)(current_r * dimmerlevel / 100.0);
  int g = on_off_status * (int)(current_g * dimmerlevel / 100.0);
  int b = on_off_status * (int)(current_b * dimmerlevel / 100.0);

  set_rgb(r, g, b);

}


void send_status(int send_on_off_status, int send_dimmerlevel, int send_rgbstring) {
  if (send_rgbstring) send(rgbMsg.set(rgbstring));
  if (send_on_off_status) send(lightMsg.set(on_off_status));
  if (send_dimmerlevel) send(dimmerMsg.set(dimmerlevel));
}
