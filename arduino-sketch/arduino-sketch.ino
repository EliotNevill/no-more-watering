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


time_t HMTime(int h, int m) {
  time_t hmtime ;
  hmtime = h  * 3600 +  m * 60 ;
  return hmtime;
}

//Pump
class Pump : public Menu {

  public:
    const int pumpNumber;
    int duration_h = 0;
    int duration_m = 0;
    boolean isWatering = false;
    int alarmId, alarmEndId;
    boolean _enabled = false;
    Pump (const char* pName, int pNum)
      : Menu(pName) , pumpNumber(pNum) {
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
    boolean enabled() const {
      return _enabled;
    }
    void startWatering() {
      isWatering = true;
    }
    void stopWatering() {
      isWatering = false;
    }
  private:
    unsigned long _wateringTime;
};

//Pump objects
Pump pm_1("Pump 1", 1);
Pump pm_2("Pump 2", 2);
Pump pm_3("Pump 3", 3);
Pump pm_4("Pump 4", 4);
vector<Pump> pumps;


// Renderer
class MyRenderer : public MenuComponentRenderer
{

  public:
    virtual void render(Menu const & menu) const
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      if (_sleep) {
<<<<<<< HEAD
        lcd.setCursor(0, 1);
        lcd.print("Sleeping");
        //   Serial.print(print_time(3,4));
=======
        lcd.print("NMW");
        print_time(hour(),minute(),5,0);
       /* lcd.setCursor(0, 0);
        lcd.print("NMW");
        lcd.setCursor(6,0);
        lcd.print(hour());
        lcd.setCursor(8,0);
        lcd.print(':');
        lcd.setCursor(9,0);
        lcd.print(minute());*/
     //   Serial.print(print_time(3,4));
>>>>>>> origin/master
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
<<<<<<< HEAD


=======
  void print_time(int h,int m, int x, int y) const{
      lcd.leftToRight();
      lcd.setCursor(x,y);
      lcd.print(h);
      lcd.print(':');
      lcd.print(m);
    }
>>>>>>> origin/master
    virtual void render_menu_item(MenuItem const & menu_item) const
    {
      String menuItemName = menu_item.get_name();
      if (menuItemName == "Toggle") {
        lcd.print(pumps[currentPump].enabled() ? "Toggle       Off" : "Toggle        On");
      } else {
        lcd.print(menu_item.get_name());
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
};
MyRenderer my_renderer;

// Menu callback functions
void on_click_s(MenuItem* p_menu_item) {}
void on_click_t(MenuItem* p_menu_item) {
  if (p_menu_item->get_name() == "Toggle") {
    pumps[currentPump].toggle();
  }
}
void on_click_d(MenuItem* p_menu_item) {}
void on_click_f(MenuItem* p_menu_item) {}

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
void pump_end_alarm_1() {pumps[0].stopWatering();}
void pump_end_alarm_2() {pumps[1].stopWatering();}
void pump_end_alarm_3() {pumps[2].stopWatering();}
void pump_end_alarm_4() {pumps[3].stopWatering();}


// Menu variables
MenuSystem ms(my_renderer);
Menu mm();
Menu sm("Settings");
MenuItem pm_stat("Status", &on_click_s);
Menu pm_set("Settings");
MenuItem pm_set_t("Toggle", &on_click_t);
MenuItem pm_set_d("Duration", &on_click_d);
MenuItem pm_set_f("Frequency", &on_click_f);


//Serial Handerler for inputs
//TODO replace with buttons
void serialHandler() {
  char inChar;
  if ((inChar = Serial.read()) > 0) {
    oldTime = now();
    switch (inChar) {
      case 'w': // Previus item
        ms.prev();
        ms.display();
        break;
      case 's': // Next item
        ms.next();
        ms.display();
        break;
      case 'a': // Back presed
        ms.back();
        ms.display();
        break;
      case 'd': // Select presed
        ms.select();
        ms.display();
        break;
      case '?':
      default:
        break;
    }

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
void setup() {

  Serial.begin(9600);
  lcd.begin(16, 2);
  setTime(8, 29, 0, 1, 1, 11);
  oldTime = now();



  //Add pumps to the menu
  pumps.push_back(pm_1);
  pumps.push_back(pm_2);
  pumps.push_back(pm_3);
  pumps.push_back(pm_4);
  //Add the settings to the pumps
  pm_set.add_item(&pm_set_t);
  pm_set.add_item(&pm_set_d);
  pm_set.add_item(&pm_set_f);

  for (int i = 0; i <= numPumps; i++) {
    //Add settings and status to the pumps
    ms.get_root_menu().add_menu(&pumps[i]);
    pumps[i].add_item(&pm_stat);
    pumps[i].add_menu(&pm_set);
  }
  ms.get_root_menu().add_menu(&sm);


  //Create alarms for pumps
  pumps[0].alarmId = Alarm.alarmRepeat(HMTime(0, 0), &pump_alarm_1);
  pumps[1].alarmId = Alarm.alarmRepeat(HMTime(0, 0), &pump_alarm_2);
  pumps[2].alarmId = Alarm.alarmRepeat(HMTime(0, 0), &pump_alarm_3);
  pumps[3].alarmId = Alarm.alarmRepeat(HMTime(0, 0), &pump_alarm_4);

  //The alarm to stop the water flowing
  pumps[0].alarmEndId = Alarm.timerOnce(0, &pump_end_alarm_1);
  pumps[1].alarmEndId = Alarm.timerOnce(0, &pump_end_alarm_2);
  pumps[2].alarmEndId = Alarm.timerOnce(0, &pump_end_alarm_3);
  pumps[3].alarmEndId = Alarm.timerOnce(0, &pump_end_alarm_4);

  //Disabling all alarms by default
    for (int i = 0; i <= numPumps; i++) {
      Alarm.disable(pumps[i].alarmId);
      Alarm.disable(pumps[i].alarmEndId);
    }


  //start displaying
  ms.display();


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

