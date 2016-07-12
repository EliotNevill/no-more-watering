#include <StandardCplusplus.h>
#include <system_configuration.h>
#include <unwind-cxx.h>


#include <utility.h>
#include <vector>
#include <sstream>


#include <MenuSystem.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <TimeAlarms.h>
#include <Time.h>
#include <TimeLib.h>

using namespace std;
typedef void (*Pointer)();

//LCD init
const int numPumps = 4;
int currentPump;
long oldTime;
int current_h, current_m, current_s;
int currentStatus =0;
const  int default_day = 5;
const  int default_month = 7;
const int default_year = 46;
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);
byte offChar[8] = {
B01110,
B00100,
B11111,
B11111,
B00011,
B00000,
B00000,
B00000
};byte onChar1[8] = {
B01110,
B00100,
B11111,
B11111,
B00011,
B00010,
B00001,
B00010
};
byte onChar2[8] = {
B01110,
B00100,
B11111,
B11111,
B00011,
B00001,
B00010,
B00001
};
byte crossChar[8] = {
  B10001,
  B10001,
  B01010,
  B00100,
  B00100,
  B01010,
  B10001,
  B10001,
};
byte emptyPipe00[8] = {
  B00000,
  B11111,
  B00000,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
};
byte emptyPipe10[8] = {
  B00000,
  B11111,
  B10000,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
};
byte emptyPipe20[8] = {
  B00000,
  B11111,
  B10100,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
};byte emptyPipe30[8] = {
  B00000,
  B11111,
  B10101,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte pipes[]= {emptyPipe00[8],emptyPipe10[8],emptyPipe20[8],emptyPipe30[8]};

//This class is for adjusting time
class TimeAdjustor {
  public:
    static int HMSselector;
    static void forwardsHMS() {
      if (HMSselector < 3)
        HMSselector++;
    }
    static  void backHMS() {
      if (HMSselector > 0)
        HMSselector--;
    }
    static void increaseHMS() {
      if (HMSselector == 1) {
        (*hours) ++;
      } else if (HMSselector == 2) {
        (*minutes)++;
      } else if (HMSselector == 3) {
        (*seconds)++;
      }
      timeFixer();
    }
    static  void decreaseHMS() {
      if (HMSselector == 1) {
        (*hours)--;
      } else if (HMSselector == 2) {
        (*minutes)--;
      } else if (HMSselector == 3) {
        (*seconds)--;
      }
      timeFixer();
    }
    static void setHMS(int* h, int* m, int* s) {
      hours = h;
      minutes = m;
      seconds = s;
    }



  private:
    static int mod(int a, int b) {
      return (a % b + b) % b;
    }
    static void timeFixer() {
      if (*seconds > 59 || *seconds < 0) {
        *seconds = mod(*seconds , 60);
      }
      if (*minutes > 59 || *minutes < 0) {
        *minutes = mod(*minutes, 60);
      }
      if (*hours > 23 || *hours < 0) {
        *hours = mod(*hours , 24);
      }

    }

    static int * hours, * minutes, * seconds;
};
int TimeAdjustor::HMSselector = 0;
int* TimeAdjustor::hours;
int* TimeAdjustor::minutes;
int* TimeAdjustor::seconds;

time_t HMSTime(int h, int m, int s) {
TimeElements tm;
tm.Second =  s;
tm.Minute = m; 
tm.Hour   = h;  
tm.Day    = default_day;      
tm.Month  = default_month;  
tm.Year   = default_year;   
  return makeTime(tm);
}
long HMStoS(int h, int m, int s){
  long sec = ((long)h)*3600 + ((long)m)*60 + (long)s;
 /*
 Serial.print(h);
 Serial.print(':');
 Serial.print(m);
 Serial.print(':');
 Serial.println(s);
 */
 return (long) sec;

}
  void write_consec(int* a, int v){
    EEPROM.write(*a , v);
    (*a)++;
  }
  void read_consec(int* a, int* v){
   *v = EEPROM.read(*a);
   (*a)++;
  }
void saveSettings(){
  int address = 0;
  write_consec(&address, hour());
  write_consec(&address, minute());
  write_consec(&address, second());
}
void loadSettings(){
  int address = 0;
  read_consec(&address,&current_h);
  read_consec(&address,&current_m);
  read_consec(&address,&current_s);
  setTime(HMSTime(current_h, current_m, current_s));
}
  
//Pump
class Pump  {
  public:
    const int pumpNumber;
    int duration_h;
    int duration_m;
    int duration_s;
    int time_h;
    int time_m;
    int time_s;
    long time_to_water;
    bool isWatering = false;
    AlarmId alarmId = 0, alarmEndId = 0;
    bool _enabled = false;
    Pump ( int pNum)
      :  pumpNumber(pNum) {
    }
    ~Pump(){
      Serial.println("DESTROYED");
    }

    void toggle() {
      if (_enabled) {
        _enabled = false;
      } else if (!_enabled) {
        _enabled = true;
      }
    }
    bool enabled() const {
      return _enabled;
    }
    void saveState(){
    int address = pumpNumber * 8;
    write_consec(&address,duration_h);
    write_consec(&address,duration_m);
    write_consec(&address,duration_s);
    write_consec(&address,time_h);
    write_consec(&address,time_m);
    write_consec(&address,time_s);
    write_consec(&address,_enabled);
    }
    void readState(){
    int address = pumpNumber * 8;
    read_consec(&address,&duration_h);
    read_consec(&address,&duration_m);
    read_consec(&address,&duration_s);
    read_consec(&address,&time_h);
    read_consec(&address,&time_m);
    read_consec(&address,&time_s);
    int a;
    read_consec(&address,&a);
    _enabled = a==1?true:false;
    }
    
  private:
};

//Pump objects
Pump pm_1(1);
Pump pm_2(2);
Pump pm_3(3);
Pump pm_4(4);
vector<Pump> pumps;


// Renderer
class MyRenderer : public MenuComponentRenderer
{

  public:
    virtual void render(Menu const & menu) const
    {
      lcd.clear();
      lcd.noCursor();
      lcd.setCursor(0, 0);

      if (_sleep) {
        lcd.setCursor(0, 0);
        lcd.print("NMW");
        print_time(hour(), minute(), 4, 0);
        lcd.setCursor(9,0);
        for(int i; i<4; i++){
        lcd.write(get_pump_glyph(pumps[i].isWatering, pumps[i].enabled()));

        }

        if(scrollTim > 30){
          p++;
          scrollTim=0;
        }  
        if(p > 3){
          p = 0;
        }
 
        lcd.setCursor(14,0);
        lcd.print("P");
        lcd.print(pumps[p].pumpNumber);
        lcd.setCursor(0,1);  
        lcd.print("D=");
        print_time_no_pos(pumps[p].duration_h, pumps[p].duration_m);
        lcd.print(" T=");
        print_time_no_pos(pumps[p].time_h, pumps[p].time_m);
        scrollTim++;
        lcd.cursor();
        lcd.setCursor(9 + p,0);
        
      }else if(currentStatus != 0){
      //
      //
      //Enter Stats screen here
      lcd.leftToRight();
      lcd.setCursor(0,0);
      lcd.print('P');
      lcd.print(currentStatus);
      if(pumps[currentStatus-1].isWatering){
        for( int i=0; i <12 ; i ++ ){
          lcd.write(byte (7));
          
        }
      }else{
      Serial.println(pumps[currentStatus-1].time_to_water);
     pipeLength = 11-(pumps[currentStatus-1].time_to_water/7854);
     Serial.println(pipeLength); 
     for(int f = 0; f < pipeLength; f++){
      lcd.write(byte(7));
     }
     int whichChar = pumps[currentStatus-1].time_to_water%7854/2618;
    
    Serial.println(pumps[currentStatus-1].time_to_water%7854);
     lcd.write(byte(7-whichChar));
     Serial.println(whichChar);
     
   
     
     for(int f =0; f< 10 - pipeLength; f++){
      lcd.write(byte(4));
     }
      }
      
     
     

     


      
      lcd.write(get_pump_glyph(pumps[currentStatus-1].isWatering, pumps[currentStatus-1].enabled()));
      
      
      lcd.setCursor(0,1);  
        lcd.print("D=");
        print_time_no_pos(pumps[currentStatus-1].duration_h, pumps[currentStatus-1].duration_m);
        lcd.print(" T=");
        print_time_no_pos(pumps[currentStatus-1].time_h, pumps[currentStatus-1].time_m);


        lcd.cursor();
        lcd.setCursor(((pumps[currentStatus-1].time_to_water%7854)%2618)/261,0);

        

        
      



        
      }else {
        if (strlen(menu.get_name()) == 0 )
          lcd.print("Main Menu");
        else
          lcd.print(menu.get_name());

        lcd.setCursor(0, 1);
        menu.get_current_component()->render(*this);
      }


    }
    byte get_pump_glyph(bool state , bool isOn) const{
      if(state){
        if( t > 10){
        flickr = !flickr;
        t = 0;
        }else{
          t++;
        }
        
        return (flickr) ? byte(2): byte (1);

      }else{if(isOn){

        return byte(0);

      }else{
        return byte(3);
      }
      
        
      }
    }
    void sleep(bool sleep) {
      _sleep = sleep;
    }


    //PN contributed to this function
    void print_time(int h, int m, int x, int y) const {
      lcd.setCursor(x, y);
      if (h < 10)
        lcd.print('0');
      lcd.print(h);
      lcd.print(':');
      if (m < 10)
        lcd.print('0');
      lcd.print(m);
    }
     void print_time_no_pos(int h, int m) const {
      if (h < 10)
        lcd.print('0');
      lcd.print(h);
      lcd.print(':');
      if (m < 10)
        lcd.print('0');
      lcd.print(m);
    }

    


    virtual void render_menu_item(MenuItem const & menu_item) const
    {
       //Serial.println(menu_item.has_focus()?"Yes":"No");
      String menuItemName = menu_item.get_name();
      lcd.print(menuItemName);
      if (menuItemName == "Toggle") {
        lcd.setCursor(13, 1);
        lcd.print(pumps[currentPump].enabled() ? "Off" : "On");
      } else if (menuItemName == "Duration") {
        print_time(pumps[currentPump].duration_h, pumps[currentPump].duration_m, 11, 1);
        lcd.cursor();
        if(TimeAdjustor::HMSselector != 0)
        lcd.setCursor(12+(3*(TimeAdjustor::HMSselector-1)),1);
      } else if (menuItemName == "Time") {
        print_time(pumps[currentPump].time_h, pumps[currentPump].time_m, 11, 1);
        lcd.cursor();
        if(TimeAdjustor::HMSselector != 0)
        lcd.setCursor(12+(3*(TimeAdjustor::HMSselector-1)),1);
      }else if(menuItemName == "Clock Time"){
        print_time(current_h,current_m,11,1);
        lcd.cursor();
        if(TimeAdjustor::HMSselector != 0)
        lcd.setCursor(12+(3*(TimeAdjustor::HMSselector-1)),1);
      }

    }

    virtual void render_back_menu_item(BackMenuItem const & menu_item) const
    {
      lcd.print(menu_item.get_name());
    }

    virtual void render_numeric_menu_item(NumericMenuItem const & menu_item) const
    {
      lcd.print(menu_item.get_name());
    }

    virtual void render_menu(Menu const & menu) const
    {
      lcd.print(menu.get_name());
    }
  private:
    bool _sleep = false;

   mutable bool flickr = false;
   mutable int t = 0;
   mutable int scrollTim;
   mutable int p = 1;
   mutable int pipeLength;
   mutable int whichChar;

};

MyRenderer my_renderer;

// Menu callback functions
void on_click_s(MenuItem* p_menu_item) {
 if (p_menu_item->get_name() == "Status") {
  currentStatus = pumps[currentPump].pumpNumber;
 }


}

void on_click_to(MenuItem* p_menu_item) {
  if (p_menu_item->get_name() == "Toggle") {
    pumps[currentPump].toggle();
    pumps[currentPump].saveState();
  }
}
void on_click_d(MenuItem* p_menu_item) {
  if (p_menu_item->get_name() == "Duration") {
    TimeAdjustor::setHMS(&pumps[currentPump].duration_h , &pumps[currentPump].duration_m , &pumps[currentPump].duration_s);
    TimeAdjustor::forwardsHMS();
  }
}
void on_click_ti(MenuItem* p_menu_item) {
  if (p_menu_item->get_name() == "Time") {
    TimeAdjustor::setHMS(&pumps[currentPump].time_h , &pumps[currentPump].time_m , &pumps[currentPump].time_s);
    TimeAdjustor::forwardsHMS();
  }
}
void on_click_sa(MenuItem* p_menu_item){
   pumps[currentPump].saveState();
}
void on_click_lo(MenuItem* p_menu_item){
  pumps[currentPump].readState();
}
void on_click_set_ct(MenuItem* p_menu_item){
  if (p_menu_item->get_name() == "Clock Time") {
  current_h = hour();
  current_m = minute();
  current_s = second();
    TimeAdjustor::setHMS(&current_h , &current_m, &current_s);
    TimeAdjustor::forwardsHMS();
  }
}
void on_click_set_sa(MenuItem* p_menu_item){
  setTime(HMSTime(current_h, current_m, current_s));
  saveSettings();
}




// Menu variables
MenuSystem ms(my_renderer);
Menu mm();
Menu pump_menu_1("Pump 1");
Menu pump_menu_2("Pump 2");
Menu pump_menu_3("Pump 3");
Menu pump_menu_4("Pump 4");
Menu sm("Settings");
MenuItem sm_t("Clock Time", &on_click_set_ct);
MenuItem sm_sa("Save", &on_click_set_sa);
MenuItem pm_stat("Status", &on_click_s);
Menu pm_set("Settings");
MenuItem pm_set_to("Toggle", &on_click_to);
MenuItem pm_set_d("Duration", &on_click_d);
MenuItem pm_set_ti("Time", &on_click_ti);
MenuItem pm_set_sa("Save", &on_click_sa);
MenuItem pm_set_lo("Load", &on_click_lo);


//Serial Handerler for inputs
//TODO replace with buttons
const int sw1Pin=14;
const int sw2Pin=15;
const int sw3Pin=16;
const int sw4Pin=17;
int sw1State = 0;
int sw2State = 0;
int sw3State = 0;
int sw4State = 0;


int i;

int prevSw[]= {0,0,0,0};

void serialHandler() {
   sw1State = digitalRead(sw1Pin);
  sw2State = digitalRead(sw2Pin);
  sw3State = digitalRead(sw3Pin);
  sw4State = digitalRead(sw4Pin);

 int switches[]={sw1State,sw2State,sw3State,sw4State};
 
 
for( i = 0; i < 4; i++){
  
  if (switches[i] == HIGH ){
  if (switches[i] != prevSw[i]) {

    Serial.print(i);
    prevSw[i] = switches[i];
    oldTime = now();

    switch (i) {
      case 0: // Previus item
        if (TimeAdjustor::HMSselector == 0) {
          ms.prev();
        } else {
          TimeAdjustor::increaseHMS();
        }
        ms.display();
        break;
      case 1: // Next item
        if (TimeAdjustor::HMSselector == 0) {
          ms.next();
        }
        else
        { TimeAdjustor::decreaseHMS();
        }
        ms.display();
        break;
      case 2: // Back presed
            if(currentStatus != 0){
        currentStatus = 0;
      }
        else if (TimeAdjustor::HMSselector == 0)
        {
          ms.back();
        }
        else
        {
          TimeAdjustor::backHMS();
        }
        ms.display();
        break;
      case 3: // Select presed
      if(currentStatus != 0){
        
      }
       else if (TimeAdjustor::HMSselector == 0)
        {  
          ms.select();
        }
        else
        {
          TimeAdjustor::forwardsHMS();
        }
        ms.display();
        break;
      case '?':
      default:
        break;
        

    }
   // Serial.println(TimeAdjustor::HMSselector);
    //Sets the current pump
    String menuName = ms.get_current_menu()->get_name();
    if (menuName == "Pump 1") {
      currentPump = 0;
    } else if (menuName == "Pump 2") {
      currentPump = 1;
    } else if (menuName == "Pump 3") {
      currentPump = 2;
    } else if (menuName == "Pump 4") {
      currentPump = 3;
    }

  }
  }else{prevSw[i] = 0;
  
    
  }
}


}
void alarmHandler(){
  for (int i = 0; i < numPumps; i++) {
    long currentsec = HMStoS(hour(),minute(),second());
    long alarmsec = HMStoS(pumps[i].time_h , pumps[i].time_m ,pumps[i].time_s );
    long durationsec = HMStoS(pumps[i].duration_h , pumps[i].duration_m ,pumps[i].duration_s );
    long ttw = alarmsec - currentsec;
    pumps[i].time_to_water = (ttw > 0 )? ttw : ttw+86400;
    if((currentsec-alarmsec) >= 0 && (currentsec - alarmsec) <= durationsec){
      pumps[i].isWatering =true;
    }else{
      pumps[i].isWatering =false;
    }
  }
}

void addSubMenus(Menu* m){
    ms.get_root_menu().add_menu(m);
    m->add_item(&pm_stat);
    m->add_menu(&pm_set);
}



void setup() {
pinMode(sw1Pin,INPUT);
  pinMode(sw2Pin,INPUT);
  pinMode(sw3Pin,INPUT);
  pinMode(sw4Pin,INPUT);
  Serial.begin(9600);
  lcd.begin(16, 2);

  lcd.createChar(0,offChar);
    lcd.createChar(1,onChar1);
      lcd.createChar(2,onChar2);
      lcd.createChar(3,crossChar);
        lcd.createChar(4,emptyPipe00);
          lcd.createChar(5,emptyPipe10);
            lcd.createChar(6,emptyPipe20);
              lcd.createChar(7,emptyPipe30);
      
     
        
      
     
      
  loadSettings(); 
  current_h = hour();
  current_m = minute();
  current_s = second();
  oldTime = now();

  
  

  //Add pumps to the array
  pumps.push_back(pm_1);
  pumps.push_back(pm_2);
  pumps.push_back(pm_3);
  pumps.push_back(pm_4);
  
  //Read pump values from memory
  for (int i = 0; i < numPumps; i++) {
  pumps[i].readState();
  }
  
  //Add the settings to the pumps
  sm.add_item(&sm_t);
  sm.add_item(&sm_sa);
  pm_set.add_item(&pm_set_to);
  pm_set.add_item(&pm_set_d);
  pm_set.add_item(&pm_set_ti);
  pm_set.add_item(&pm_set_sa);
  pm_set.add_item(&pm_set_lo);
  addSubMenus(&pump_menu_1);
  addSubMenus(&pump_menu_2);
  addSubMenus(&pump_menu_3);
  addSubMenus(&pump_menu_4);
  ms.get_root_menu().add_menu(&sm);

  //start displaying
  ms.display();


}
int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void loop() {
  serialHandler();
  ms.display();
  Alarm.delay(50);
   //Serial.print(pumps[0].time_to_water);
 /*  Serial.print(' ');
   Serial.print(HMStoS(hour(),minute(),second()));
   Serial.println(' ');
   time_t tta =  Alarm.read(pumps[0].alarmId) - HMStoS(hour(),minute(),second()) ;
  Serial.println(tta);
  */
  alarmHandler();
  //Sleep Clock
  if ( (now() - oldTime) > 3) {
    my_renderer.sleep(true);
  } else {
    my_renderer.sleep(false);
  }
}

