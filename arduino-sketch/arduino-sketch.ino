unsigned long oldTime = 0;
unsigned long = currentTime;
const long gap = 86400000;


void setup() {


 
 
}

void loop() {
  currentTime = millis();
  if(currentTime - oldTime > gap){



    currentTime = oldTime;
    

  
  }
}
