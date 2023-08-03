//HEADER FILE FOR GAUSS RIFLE
//All coil stages will share the same charging and discharging pins to conserve GPIO
//this pin will then switch an optocoupler leading to multiple relays for individual switching 

#ifndef _GAUSS_RIFLE_3_COIL_H
#define _GAUSS_RIFLE_3_COIL_H

//Include Arduino library
#include <Arduino.h>

//Define Input Pins
#define charge_switch 36 //GPIO36, no internal pullup or pulldown resistor
#define discharge_switch 39 //GPIO39, no internal pullup or pulldown resistor
#define initial_charge_switch 34 //GPIO34, no internal pullup or pulldown resistor
#define reset 35 //GPIO35, no internal pullup or pulldown resistor
#define safety 32 //GPIO32
#define trigger 33 //GPIO33
#define coil_1_IR 25 //GPIO25
#define coil_2_IR 26 //GPIO26
#define coil_3_IR 27 //GPIO27

//Define Output Pins
#define green_light 16 //GPIO16
#define yellow_light 4 //GPIO4
#define red_light 13 //GPIO13
#define boost_xf_relay 23 //GPIO23
#define hv_charging_relay 22 //GPIO22
#define discharging_relay 21 //GPIO21
#define coil_1_SCR_relay 19 //GPIO19
#define coil_2_SCR_relay 18 //GPIO18
#define coil_3_SCR_relay 17 //GPIO17

//Define other constants
#define max_time_between_coils 5000

//Declare Global variables for interrupt to change
extern bool charging_state;
extern bool discharging_state;
extern bool fire_state;
extern bool warning_state;
extern bool emergency_state;
extern bool reset_state;
extern bool coil_1_IR_state;
extern bool coil_2_IR_state;
extern bool coil_3_IR_state;
extern bool projectile_stuck;

//Variables to keep track of the timing of recent interrupts, used for software button debouncing
extern unsigned long button_time;
extern unsigned long last_button_time; 

//Variables for tracking coil fire stages
extern unsigned long coil_fire_time;
extern unsigned long time_since_coil_fire;

//Define enum for coil firing sequence, states: pre fire = gun ready to be fired (starting with coil 1)
//Preceding states are when projectile reaches that coil so it can be shut off and the next turned on
typedef enum coil_gun_fire_states{pre_fire, coil_1_fire, coil_2_fire, coil_3_fire};

//Declare functions
void charge_interrupt();
void discharge_interrupt();
void trigger_firing_interrupt();
void coil_1_interrupt();
void coil_2_interrupt();
void coil_3_interrupt();
void firing_timer_ISR();

#endif

//End of header file
