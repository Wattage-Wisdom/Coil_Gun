//Code for esp32 to track speed of projectile

//It is expected that when the IR beam is powered on, the reciever will transmit high on the ouput
//This high output will always be on, then when the projectile crosses the beam the transmitter will go low for a short second

#define distance 0.1 //distance in meters between IR sensors
#define IR_sensor_1 25 //GPIO25
#define IR_sensor_2 26 //GPIO26

bool new_time = false;
double start_time = 0;
double end_time = 0;
double velocity = 0;
double button_time_1 = 0;
double last_button_time_1 = 0;
double button_time_2 = 0;
double last_button_time_2 = 0;


void start_timer_interrupt();
void end_timer_interrupt();


void setup() {
  
pinMode(IR_sensor_1, INPUT_PULLDOWN);
pinMode(IR_sensor_2, INPUT_PULLDOWN);

attachInterrupt(IR_sensor_1, start_timer_interrupt, FALLING); //start timer
attachInterrupt(IR_sensor_2, end_timer_interrupt, FALLING); //end timer

Serial.begin(9600);

}

void loop() {

  if(new_time == true){

    //Code to calculate veloctiy
    velocity = (1000000*distance)/(end_time-start_time); //calculates the velocity in m/s
    //Print the new velocity to the serial monitor
    Serial.print("New velocity is: ");
    Serial.print(velocity);
    Serial.print(" m/s\n");
    //Print new time to the LCD, ADD LATER
   
    new_time = false;
  }
  //End of main loop
    
  }


//Start timer for counting
void IRAM_ATTR start_timer_interrupt(){
  button_time_1 = millis(); 
  if (button_time_1 - last_button_time_1 > 250){ 
       start_time = micros();
       last_button_time_1 = button_time_1;
}
}

//End timer
void IRAM_ATTR end_timer_interrupt(){
  button_time_2 = millis(); 
  if (button_time_2 - last_button_time_2 > 250){ 
       end_time = micros();
       new_time = true;
       last_button_time_2 = button_time_2;
}
}

//End of code
