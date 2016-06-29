#include <StandardCplusplus.h>
#include <system_configuration.h>
#include <unwind-cxx.h>
#include <utility.h>
#include <vector>


#include <MenuSystem.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <TimeAlarms.h>
#include <Time.h>
#include <TimeLib.h>

using namespace std;

const int numPumps = 4;
int currentPump;


//LCD init
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);


//Pump
class Pump : public Menu{
    
    
    unsigned long wateringDuration;

  public:
    
    Pump (const char* pName, int pNum) 
    :Menu(pName) {
      pumpNumber = pNum;
    }
    unsigned long wateringTime() {
      return _wateringTime;
    }
    void wateringTime(unsigned long &wateringTime) {
      _wateringTime = wateringTime;
    }
    void toggle(){
      if(_enabled){
        _enabled = false;
      }else if(!_enabled){
        _enabled = true;
      }
    }
    boolean enabled() const{
      return _enabled;
    }
 private:
    unsigned long _wateringTime;
    boolean _enabled = false;
    int pumpNumber;
};

Pump pm_1("Pump 1",1);
Pump pm_2("Pump 2",2);
Pump pm_3("Pump 3",3);
Pump pm_4("Pump 4",4);
//extern Pump* pumps[numPumps] ; //= {pm_1, pm_2, pm_3, pm_4};
static vector<Pump> pumps;



// Renderer
class MyRenderer : public MenuComponentRenderer
{
  
  public:
    virtual void render(Menu const & menu) const
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(menu.get_name());
      lcd.setCursor(0, 1);
      menu.get_current_component()->render(*this);
    }
    void setPumps(vector<Pump> &pumps){
       _pumps = pumps;
    }
    virtual void render_menu_item(MenuItem const & menu_item) const
    {
      String menuItemName = menu_item.get_name();
      if (menuItemName == "Toggle") {
        lcd.print(_pumps[currentPump].enabled()? "Toggle       Off":"Toggle        On");
      }else{
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
     vector<Pump> _pumps;
};
MyRenderer my_renderer;

// Menu callback function
void on_click_s(MenuItem* p_menu_item) {}
void on_click_t(MenuItem* p_menu_item) {
  if (p_menu_item->get_name() == "Toggle") {
    pumps[currentPump].toggle();
  }
  }
void on_click_d(MenuItem* p_menu_item) {}
void on_click_f(MenuItem* p_menu_item) {}

// Menu variables
MenuSystem ms(my_renderer);
Menu mm();

//pumps[2].Pump("Pump 1",1)
/*
*/
Menu sm("Settings");
MenuItem pm_stat("Status", &on_click_s);
Menu pm_set("Settings");
MenuItem pm_set_t("Toggle", &on_click_t);
MenuItem pm_set_d("Duration", &on_click_d);
MenuItem pm_set_f("Frequency", &on_click_f);

void serialHandler() {
  char inChar;
  if ((inChar = Serial.read()) > 0) {
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
  }
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

  /*else if(strcmp(menuName, "Pump 2") == 0) {
    currentPump = 2;
    }else if(strcmp(menuName, "Pump 3") == 0) {
    currentPump = 3;
    }else if(strcmp(menuName, "Pump 4") == 0) {
    currentPump = 4;
    }
  */
}
void setup() {

  Serial.begin(9600);
  lcd.begin(16, 2);
  setTime(8,29,0,1,1,11);
 
  pumps.push_back(pm_1);
  pumps.push_back(pm_2);
  pumps.push_back(pm_3);
  pumps.push_back(pm_4);
  
 /* pumps.push_back(pm_3);
  pumps.push_back(pm_4);
*/
  pm_set.add_item(&pm_set_t);
  pm_set.add_item(&pm_set_d);
  pm_set.add_item(&pm_set_f);

  for (int i = 0; i <= numPumps; i++) {
    ms.get_root_menu().add_menu(&pumps[i]);
    pumps[i].add_item(&pm_stat);
    pumps[i].add_menu(&pm_set);
  }
  ms.get_root_menu().add_menu(&sm);
  ms.display();

  // setup lcdz
  // setup buttons, inputs outputs
  // set time

}

void loop() {
  serialHandler();
  my_renderer.setPumps(pumps);
  Alarm.delay(0);
}
//This is a comment


