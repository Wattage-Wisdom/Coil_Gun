//Main code for the weapon system, includes setup and main loop

//Include associated header file, cpp file will be included automatically
#include "Gauss_Rifle_3_Coil.h"

//Instantiate enum for state of coil firing sequence 
coil_gun_fire_states firing_state;

//Declare a pointer for firing timer
hw_timer_t *firing_timer = NULL;

//Setup loop
void setup() {

//Set variables in their desired initial state
firing_state = pre_fire;
unsigned long coil_fire_time = 0;
unsigned long time_since_coil_fire = 0;

//Setup Inputs
//The following have no internal pullup or pulldown resistors, make sure to include them in circuitry
pinMode(charge_switch, INPUT);
pinMode(discharge_switch, INPUT);
pinMode(initial_charge_switch, INPUT);
pinMode(reset, INPUT);

//Following inputs have internal resistors
pinMode(safety, INPUT_PULLDOWN);
pinMode(trigger, INPUT_PULLDOWN);
pinMode(coil_1_IR, INPUT_PULLDOWN);
pinMode(coil_2_IR, INPUT_PULLDOWN);
pinMode(coil_3_IR, INPUT_PULLDOWN);

//Setup Outputs
pinMode(green_light, OUTPUT);
pinMode(yellow_light, OUTPUT);
pinMode(red_light, OUTPUT);
pinMode(boost_xf_relay, OUTPUT);
pinMode(hv_charging_relay, OUTPUT);
pinMode(discharging_relay, OUTPUT);
pinMode(coil_1_SCR_relay, OUTPUT);
pinMode(coil_2_SCR_relay, OUTPUT);
pinMode(coil_3_SCR_relay, OUTPUT);

//Setup Serial Communication
Serial.begin(9600);

//Declare interrupts
attachInterrupt(charge_switch, charge_interrupt, RISING);//charging circuit interrupt
attachInterrupt(discharge_switch, discharge_interrupt, RISING);//discharging circuit interrupt
attachInterrupt(trigger, trigger_firing_interrupt, RISING);//trigger pulled fire gun interrupt
attachInterrupt(coil_1_IR, coil_1_interrupt, FALLING);//projectile reach coil 1 interrupt
attachInterrupt(coil_2_IR, coil_2_interrupt, FALLING);//projectile reach coil 2 interrupt
attachInterrupt(coil_3_IR, coil_3_interrupt, FALLING);//projectile reach coil 3 interrupt

//Setup timer and interrupt
    firing_timer = timerBegin(0, 80000, true); //prescalar set so every tick equal 1ms
    timerAttachInterrupt(firing_timer, &firing_timer_ISR, true);
    timerAlarmWrite(firing_timer, 50000, true);
    timerAlarmEnable(firing_timer);
    timerStop(firing_timer);
    timerRestart(firing_timer);

//Turn green LED on as the board has now been intialized
digitalWrite(green_light, HIGH);

//End of setup code
}

//Will run this reapeted loop and execute steps based off the state the weapon is in which is set by interrupts
void loop() {

  //Charge loop to charge the capacitors
  //Charging circuit, designed to put less stress on the high voltage charging relay coil (want smallest potential difference possible when closing)
  while(charging_state == true){
    
    //If initial charge or Capacitors below 200V both sides of hv charge relay are at 0V (or lower than halfway threshold volts) so close this relay first
  if(digitalRead(initial_charge_switch) == HIGH && digitalRead(discharge_switch) == LOW && warning_state == false && emergency_state == false){
    digitalWrite(hv_charging_relay, HIGH);
    delay(1000); //time to let the hv relay close before turning on the high voltage circuit
    digitalWrite(boost_xf_relay, HIGH);
  }

  //If not initial charge or Capacitors above 200V turn on boost xf first so hv charge relay has less potential difference when it closes
  else if(digitalRead(initial_charge_switch) == LOW && digitalRead(discharge_switch) == LOW && warning_state == false && emergency_state == false){
    digitalWrite(boost_xf_relay, HIGH);
    delay(3000);//delay for slightly longer so boost xf has time to power up and output 400V
    digitalWrite(hv_charging_relay, HIGH);
  }
  
  //Discharge switch is on so do not turn on the charging loop
  else{
    digitalWrite(boost_xf_relay, LOW);
    digitalWrite(hv_charging_relay, LOW);
    warning_state = true;
  }

  //Now that relays have been closed in the proper order the charging can commence
  if(warning_state == false && emergency_state == false){
    delay(10000); //RC circuit time constant is XXXXX and so circuit will charge for XXXXX or XXXX time constants
    digitalWrite(hv_charging_relay, LOW);
    delay(2000); //time for hv relay to disconnect before turning high voltage circuit side off to 0V
    digitalWrite(boost_xf_relay, LOW);
    charging_state = false;//Circuit has been charged so turn off the charging state
    discharging_state = false;//Turn off all other states in case triggered during charging process
    fire_state = false;
  }
  //End of charging state
  }


//Dischage swithc has been flipped, discharge the capacitors
while(discharging_state == true && digitalRead(hv_charging_relay) == LOW && digitalRead(boost_xf_relay) == LOW && fire_state == false){
  digitalWrite(discharging_relay, HIGH);
  delay(60000);//wait ample time for the capacitors to discharge safely
  digitalWrite(discharging_relay, LOW);
  discharging_state = false;
  charging_state = false; //turn off all other states in case triggered during the disharging process
  fire_state = false;

//End of discharging state
}



//Trigger has been pulled, fire the weapon if all safety precautions are met
//Once fired, run through each individual coil state
while(fire_state == true){

  //States: pre-fire, coil 1 fire, coil 2 fire, coil 3 fire (reset to pre-fire)
  switch(firing_state){

    //Gun has been primed, fire coil 1 if all safety conditions are met
    case pre_fire:
      if(digitalRead(safety) == LOW && digitalRead(charge_switch) == LOW  && digitalRead(discharge_switch) == LOW && digitalRead(initial_charge_switch) == LOW && 
      charging_state == false && discharging_state == false && warning_state == false && emergency_state == false){
        //All safety conditions are met, fire coil 1 and begin the firing process
        digitalWrite(coil_1_SCR_relay, HIGH);
        firing_state = coil_1_fire;
        timerStart(firing_timer);//engage and restart firing timer to act as a watchdog during firing cycle
      }

      else{//Certain safety precautions are not met
        digitalWrite(yellow_light, HIGH);
        warning_state = true;
        fire_state = false;
      }
    break; //break the pre-fire state
 
    //Waiting for projectile to reach coil 1
    case coil_1_fire:
    while(coil_1_IR_state == false){ //designed to hold processor inside this while loop waiting for projectile to reach coil 1 IR and trigger state
      Serial.print("In coil 1 loop");
    //Code added to make sure weapon cycles through full firing cycle and does not get stuck
    if( projectile_stuck == true){
      digitalWrite(coil_1_SCR_relay, LOW);
      emergency_state = true; //if it has been 10 seconds between initial fire and projectile reaching coil 1 enter emergency state and break loop
      fire_state = false;
      firing_state = pre_fire;
      timerStop(firing_timer);
      timerRestart(firing_timer);
      break;
    }
    }
    if(emergency_state == false){
    //Pojectile reached coil 1, turn off coil 1 and turn coil 2 on
    digitalWrite(coil_1_SCR_relay, LOW);
    digitalWrite(coil_2_SCR_relay, HIGH);
    coil_1_IR_state = false;
    firing_state = coil_2_fire;
    timerRestart(firing_timer);
    }
    break; //break coil 1 fire state

    //Waiting for projectile to reach coil 2
    case coil_2_fire:
    while(coil_2_IR_state == false){//designed to hold processor inside this while loop waiting for projectile to reach coil 2 IR and trigger state 
      Serial.print("In coil 2 loop");
    //Code added to make sure weapon cycles through full firing cycle and does not get stuck
    if(projectile_stuck == true){
      digitalWrite(coil_2_SCR_relay, LOW);
      emergency_state = true; //if it has been 10 seconds between initial fire and projectile reaching coil 2 enter emergency state and break loop
      fire_state = false;
      firing_state = pre_fire;
      timerStop(firing_timer);
      timerRestart(firing_timer);
      break;
    }
    }
    if(emergency_state == false){
    //Pojectile reached coil 2, turn off coil 2 and turn coil 3 on
    digitalWrite(coil_2_SCR_relay, LOW);
    digitalWrite(coil_3_SCR_relay, HIGH);
    coil_2_IR_state = false;
    firing_state = coil_3_fire;
    timerRestart(firing_timer);
    }
    break; //break coil 2 fire state

    case coil_3_fire:
    while(coil_3_IR_state == false){ //designed to hold processor inside this while loop waiting for projectile to reach coil 3 IR and trigger state
      Serial.print("In coil 3 loop");
    //Code added to make sure weapon cycles through full firing cycle and does not get stuck
    if(projectile_stuck == true){
      digitalWrite(coil_3_SCR_relay, LOW);
      emergency_state = true; //if it has been 10 seconds between initial fire and projectile reaching coil 3 enter emergency state and break loop
      fire_state = false;
      firing_state = pre_fire;
      timerStop(firing_timer);
      timerRestart(firing_timer);
      break;
    }
    }
    if(emergency_state == false){
    //Projectile reached coil 3, shutoff coil 3 and reset firing state
    digitalWrite(coil_3_SCR_relay, LOW);
    firing_state = pre_fire;
    coil_3_IR_state = false;
    timerStop(firing_timer);
    timerRestart(firing_timer);
    charging_state = false; //turn off other states in case triggered during firing process
    discharging_state = false;
    //CHANGE THIS TO SOMETHING BECAUSE YOU NEED TO LOAD THE WEAPON
    delay(5000);//there is a 3 second gap to give the operator time to release the trigger otherwise the weapon will fire again
    fire_state = false; //put here in case trigger is pulled during delay
    }
    break; //break coil 3 fire state
   

  }//End of switch
  
  }//End of firing state


//Rifle is in the warning state, must shut everything off to reset
while(warning_state == true || emergency_state == true){

  digitalWrite(green_light, LOW);
  if(warning_state == true){
    digitalWrite(yellow_light, HIGH);
  }
  if(emergency_state == true){
    digitalWrite(red_light, HIGH);
  }
  
  if(digitalRead(reset) == HIGH && digitalRead(safety) == HIGH && digitalRead(trigger) == LOW && digitalRead(charge_switch) == LOW && digitalRead(discharge_switch) == LOW && 
  digitalRead(initial_charge_switch) == LOW){
  //All switches are flipped including the reset switch, begin the reset process, mark reset state by all lighrs being on
  digitalWrite(red_light, HIGH);
  digitalWrite(yellow_light, HIGH);
  digitalWrite(green_light, HIGH);
  reset_state = true;
  delay(500); //delay for debounce and to wait before resetting
  }

  //State so that reset switch must be turned on and off before resetting the machine
  if(reset_state == true && digitalRead(reset) == LOW){
    //Turn off all states to complete total reset
    warning_state = false;
    emergency_state = false;
    projectile_stuck = false;
    charging_state = false;
    discharging_state = false;
    fire_state = false;
    digitalWrite(red_light, LOW);
    digitalWrite(yellow_light, LOW);
    digitalWrite(green_light, HIGH);
    reset_state = false;
    fire_state = pre_fire; //Reset fire state so it does not accidentilly trigger a coil to fire
    timerStop(firing_timer);
    timerRestart(firing_timer);
  }

}//End of the reset code


}//End of main loop codecode
