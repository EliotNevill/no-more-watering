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
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);

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

time_t HMTime(int h, int m) {
  time_t hmtime ;
  hmtime = h  * 3600 +  m * 60 ;
  return hmtime;
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
    bool isWatering = false;
    int alarmId, alarmEndId;
    bool _enabled = false;
    Pump ( int pNum)
      :  pumpNumber(pNum) {
    }
    ~Pump(){
      Serial.println("DESTROYED");
    }
    void setWateringTime(int h, int m) {
      Alarm.write(alarmId , HMTime(h, m) );
    }
    void toggle() {
      if (_enabled) {
        _enabled = false;
        Alarm.disable(alarmId);
        stopWatering();
      } else if (!_enabled) {
        _enabled = true;
        Alarm.enable(alarmId);
      }
    }
    bool enabled() const {
      return _enabled;
    }
    void startWatering() {
      isWatering = true;
    }
    void stopWatering() {
      isWatering = false;
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
  void write_consec(int* a, int v){
    EEPROM.write(*a , v);
    (*a)++;
  }
  void read_consec(int* a, int* v){
   *v = EEPROM.read(*a);
   (*a)++;
  }
    unsigned long _wateringTime;
};

//Pump objects
Pump pm_1(1);
Pump pm_2(2);
Pump pm_3(3);
Pump pm_4(4);
vector<Pump> pumps;

int b;

// Renderer
class MyRenderer : public MenuComponentRenderer
{

  public:
    virtual void render(Menu const & menu) const
    {
      lcd.clear();
      lcd.setCursor(0, 0);

      if (_sleep) {

        lcd.setCursor(0, 0);
        lcd.print("NMW");
        print_time(hour(), minute(), 11, 0);
        for( b = 0; b < 105; b++){
            lcd.setCursor(b,0);
        for (int i = 0; i <= numPumps; i++) {
          
          
          
        lcd.print("    P");
        lcd.print(i);
        lcd.print('=');  
        lcd.print(pumps[i].enabled()? "ON" : "OFF");
        lcd.print(", DUR=");
        print_time_no_pos(pumps[i].duration_h, pumps[i].duration_m);
        lcd.print(", Time=");
        print_time_no_pos(pumps[i].time_h, pumps[i].time_m);
        Alarm.delay(150);
        
        
  }
        }
        
        

      } else {
        if (strlen(menu.get_name()) == 0 )
          lcd.print("Main Menu");
        else
          lcd.print(menu.get_name());

        lcd.setCursor(0, 1);
        menu.get_current_component()->render(*this);
      }


    }

    void sleep(bool sleep) {
      _sleep = sleep;
    }
    void cursorHighlight(int x, int y) {
      cursorHighlight_x = x;
      cursorHighlight_y = y;
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
      String menuItemName = menu_item.get_name();
      lcd.print(menuItemName);
      if (menuItemName == "Toggle") {
        lcd.setCursor(13, 1);
        lcd.print(pumps[currentPump].enabled() ? "Off" : "On");
      } else if (menuItemName == "Duration") {
        print_time(pumps[currentPump].duration_h, pumps[currentPump].duration_m, 11, 1);
      } else if (menuItemName == "Time") {
        print_time(pumps[currentPump].time_h, pumps[currentPump].time_m, 11, 1);
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
    int cursorHighlight_x, cursorHighlight_y;
};
MyRenderer my_renderer;

// Menu callback functions
void on_click_s(MenuItem* p_menu_item) {
  if(pumps[currentPump].enabled){
   lcd.print("d=");
   print_time_no_pos(pumps[currentPump].duration_h, pumps[currentPump].duration_m);
   lcd.print(" T=");
   print_time_no_pos(pumps[currentPump].time_h, pumps[currentPump].time_m);
  }else{ 
   lcd.print("OFF");
  }
void on_click_to(MenuItem* p_menu_item) {
  if (p_menu_item->get_name() == "Toggle") {
    pumps[currentPump].toggle();
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


//Pump alarm callback functions
void pump_alarm_1() {
  pumps[0].startWatering();
  Alarm.write(pumps[0].alarmEndId, HMTime(pumps[0].duration_h, pumps[0].duration_m));
}
void pump_alarm_2() {
  pumps[1].startWatering();
  Alarm.write(pumps[1].alarmEndId, HMTime(pumps[1].duration_h, pumps[1].duration_m));
}
void pump_alarm_3() {
  pumps[2].startWatering();
  Alarm.write(pumps[2].alarmEndId, HMTime(pumps[2].duration_h, pumps[2].duration_m));
}
void pump_alarm_4() {
  pumps[3].startWatering();
  Alarm.write(pumps[3].alarmEndId, HMTime(pumps[3].duration_h, pumps[3].duration_m));
}
void pump_end_alarm_1() {
  pumps[0].stopWatering();
}
void pump_end_alarm_2() {
  pumps[1].stopWatering();
}
void pump_end_alarm_3() {
  pumps[2].stopWatering();
}
void pump_end_alarm_4() {
  pumps[3].stopWatering();
}


// Menu variables
MenuSystem ms(my_renderer);
Menu mm();
Menu pump_menu_1("Pump 1");
Menu pump_menu_2("Pump 2");
Menu pump_menu_3("Pump 3");
Menu pump_menu_4("Pump 4");
Menu sm("Settings");
MenuItem pm_stat("Status", &on_click_s);
Menu pm_set("Settings");
MenuItem pm_set_to("Toggle", &on_click_to);
MenuItem pm_set_d("Duration", &on_click_d);
MenuItem pm_set_ti("Time", &on_click_ti);
MenuItem pm_set_sa("Save", &on_click_sa);


//Serial Handerler for inputs
//TODO replace with buttons
void serialHandler() {
  char inChar;
  if ((inChar = Serial.read()) > 0) {
    oldTime = now();

    switch (inChar) {
      case 'w': // Previus item
        if (TimeAdjustor::HMSselector == 0) {
          ms.prev();
        } else {
          TimeAdjustor::increaseHMS();
        }
        ms.display();
        break;
      case 's': // Next item
        if (TimeAdjustor::HMSselector == 0) {
          ms.next();
        }
        else
        { TimeAdjustor::decreaseHMS();
        }
        ms.display();
        break;
      case 'a': // Back presed
        if (TimeAdjustor::HMSselector == 0)
        {
          ms.back();
        }
        else
        {
          TimeAdjustor::backHMS();
        }
        ms.display();
        break;
      case 'd': // Select presed
        if (TimeAdjustor::HMSselector == 0)
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
    Serial.println(TimeAdjustor::HMSselector);
    //Sets the current pump
    String menuName = ms.get_current_menu()->get_name();
    if (menuName == "Pump 1") {
      currentPump = 1;
    } else if (menuName == "Pump 2") {
      currentPump = 2;
    } else if (menuName == "Pump 3") {
      currentPump = 3;
    } else if (menuName == "Pump 4") {
      currentPump = 4;
    }

  }


}

void addSubMenus(Menu* m){
    ms.get_root_menu().add_menu(m);
    m->add_item(&pm_stat);
    m->add_menu(&pm_set);
}
void setup() {

  Serial.begin(9600);
  
  lcd.begin(16, 2);
  setTime(8, 29, 0, 1, 1, 11);
  oldTime = now();

  
  

  //Add pumps to the array
  pumps.push_back(pm_1);
  pumps.push_back(pm_2);
  pumps.push_back(pm_3);
  pumps.push_back(pm_4);
  
  //Read pump values from memory
  for (int i = 0; i <= numPumps; i++) {
  pumps[i].readState();
  }

  
  //Create alarms for pumps
  pumps[0].alarmId = Alarm.alarmRepeat(HMTime(pumps[0].time_h, pumps[0].time_m), &pump_alarm_1);
  pumps[1].alarmId = Alarm.alarmRepeat(HMTime(pumps[1].time_h, pumps[1].time_m), &pump_alarm_2);
  pumps[2].alarmId = Alarm.alarmRepeat(HMTime(pumps[2].time_h, pumps[2].time_m), &pump_alarm_3);
  pumps[3].alarmId = Alarm.alarmRepeat(HMTime(pumps[3].time_h, pumps[3].time_m), &pump_alarm_4);

  //The alarm to stop the water flowing
  pumps[0].alarmEndId = Alarm.timerOnce(HMTime(pumps[0].duration_h, pumps[0].duration_m), &pump_end_alarm_1);
  pumps[1].alarmEndId = Alarm.timerOnce(HMTime(pumps[1].duration_h, pumps[1].duration_m), &pump_end_alarm_2);
  pumps[2].alarmEndId = Alarm.timerOnce(HMTime(pumps[2].duration_h, pumps[2].duration_m), &pump_end_alarm_3);
  pumps[3].alarmEndId = Alarm.timerOnce(HMTime(pumps[3].duration_h, pumps[3].duration_m), &pump_end_alarm_4);

  //Disabling all alarms by default
  for (int i = 0; i <= numPumps; i++) {
    Alarm.disable(pumps[i].alarmId);
    Alarm.disable(pumps[i].alarmEndId);
  }

  //Add the settings to the pumps
  pm_set.add_item(&pm_set_to);
  pm_set.add_item(&pm_set_d);
  pm_set.add_item(&pm_set_ti);
  pm_set.add_item(&pm_set_sa);
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


  //Sleep Clock
  if ( (now() - oldTime) > 3) {
    my_renderer.sleep(true);
  } else {
    my_renderer.sleep(false);
  }
}

