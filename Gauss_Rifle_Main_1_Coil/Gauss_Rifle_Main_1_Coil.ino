//Main code for the weapon system, includes setup and main loop

//Include associated header file, cpp file will be included automatically
#include "Gauss_Rifle_1_Coil.h"

//Instantiate enum for state of coil firing sequence 
coil_gun_fire_states firing_state;

//Setup loop
void setup() {

firing_state = pre_fire;

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

//Turn green LED on as the board has now been intialized
digitalWrite(green_light, HIGH);

//DEBUG
Serial.print("Board has been setup\n");

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
    digitalWrite(yellow_light, HIGH);
    warning_state = true;
    Serial.print("Discharge switch is on or already in warning/emergecny state, charging circuit cannot be turned on\n");
  }

  //Now that relays have been closed in the proper order the charging can commence
    delay(10000); //RC circuit time constant is XXXXX and so circuit will charge for XXXXX or XXXX time constants
    digitalWrite(hv_charging_relay, LOW);
    delay(2000); //time for hv relay to disconnect before turning high voltage circuit side off to 0V
    digitalWrite(boost_xf_relay, LOW);
    charging_state = false;//Circuit has been charged so turn off the charging state
  //End of charging state
  }


//Dischage swithc has been flipped, discharge the capacitors
while(discharging_state == true){
  digitalWrite(discharging_relay, HIGH);
  delay(60000);//wait ample time for the capacitors to discharge safely
  digitalWrite(discharging_relay, LOW);
  discharging_state = false;

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
      charging_state == false && discharging_state == false && emergency_state == false){
        //All safety conditions are met, fire coil 1 and begin the firing process
        digitalWrite(coil_1_SCR_relay, HIGH);
        firing_state = coil_1_fire;
      }

      else{//Certain safety precautions are not met
        digitalWrite(yellow_light, HIGH);
        Serial.print("Trigger was pulled without all safety conditions met\n");
        fire_state=false;
        warning_state = true;
      }
    break; //break the pre-fire state

    //End of switch mode for 1 coil

    
    //Waiting for projectile to reach coil 1
    case coil_1_fire:
    while(coil_1_IR_state == false){//while loop to hold until projectile reaches coil 1
      Serial.print("in coil loop 1\n");
    }
    //Pojectile reached coil 1, turn off coil 1 and turn coil 2 on
    digitalWrite(coil_1_SCR_relay, LOW);
    fire_state=false;
    firing_state = pre_fire;
    coil_1_IR_state = false;
    break; //break coil 1 fire state
    
    //End of firing state for 1 coil
    
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
  
  if(digitalRead(reset) == HIGH && digitalRead(safety) == HIGH && digitalRead(trigger) == LOW && digitalRead(charge_switch) == LOW && digitalRead(discharge_switch) == LOW){
  //All switches are flipped including the reset switch, begin the reset process, mark reset state by all lighrs being on
  digitalWrite(red_light, HIGH);
  digitalWrite(yellow_light, HIGH);
  digitalWrite(green_light, HIGH);
  reset_state = true;
  delay(500); //delay for debounce and to wait before resetting
  }

  //State so that reset switch must be turned on and off before resetting the machine
  if(reset_state == true && digitalRead(reset) == LOW){
    warning_state = false;
    emergency_state = false;
    digitalWrite(red_light, LOW);
    digitalWrite(yellow_light, LOW);
    digitalWrite(green_light, HIGH);
    reset_state = false;
    fire_state = pre_fire; //Reset fire state so it does not accidentilly trigger a coil to fire
  }

}//End of the reset code


}//End of main loop code

//Add debounce for reset state
//Trigger cycle fix
