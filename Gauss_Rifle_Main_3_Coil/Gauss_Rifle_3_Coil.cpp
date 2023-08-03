//CPP file for Gauss Rifle to include any excess functions or classes created (possibly move over to more OOP at a later date)

#include "Gauss_Rifle_3_Coil.h"
//Currently only excess functions are interrupts so define them here

//Global Variables
//Assign Global variables for interrupt to change
bool charging_state = false;
bool discharging_state = false;
bool fire_state = false;
bool warning_state = false;
bool emergency_state = false;
bool reset_state = false;
bool coil_1_IR_state = false;
bool coil_2_IR_state = false;
bool coil_3_IR_state = false;
bool projectile_stuck = false;

//Assign variables to keep track of the timing of recent interrupts, used for software button debouncing
unsigned long button_time = 0;  
unsigned long last_button_time = 0; 


//Charge interrupt to charge the capacitors
void IRAM_ATTR charge_interrupt() {//interrupt flag is automatically cleared when routine is entered so does not need to be reset
  //Code is included to handle button debouncing inside of the interrupt loops
  button_time = millis(); 
  if (button_time - last_button_time > 250){ //if interrupt has not been executed in .25 seconds then execute it, otherwise likely button bouncing    
       charging_state = true;
       last_button_time = button_time;
}
}

//Discharge interrupt to discharge the capacitors
void IRAM_ATTR discharge_interrupt() {
  button_time = millis(); 
  if (button_time - last_button_time > 250){ 
       discharging_state = true;
       last_button_time = button_time;
}
}

//Trigger interrupt to fire the weapon
void IRAM_ATTR trigger_firing_interrupt() {
  button_time = millis(); 
  if (button_time - last_button_time > 250){ 
       fire_state = true; 
       last_button_time = button_time;
}  
}

//Projectile reached coil 1 interrupt, stage coils
void IRAM_ATTR coil_1_interrupt(){
  button_time = millis(); 
  if (button_time - last_button_time > 250){ 
       coil_1_IR_state = true;
       last_button_time = button_time;
}
}

//Projectile reached coil 2 interrupt, stage coils
void IRAM_ATTR coil_2_interrupt(){
  button_time = millis(); 
  if (button_time - last_button_time > 250){ 
       coil_2_IR_state = true;
       last_button_time = button_time;
}
}

//Projectile reached coil 3 interrupt, stage coils
void IRAM_ATTR coil_3_interrupt(){
  button_time = millis(); 
  if (button_time - last_button_time > 250){ 
       coil_3_IR_state = true;
       last_button_time = button_time;
}
}

//Timer interrupt if projectile doesn't properly move through the coils
void IRAM_ATTR firing_timer_ISR() {
    projectile_stuck = true;
}

//End of cpp file
