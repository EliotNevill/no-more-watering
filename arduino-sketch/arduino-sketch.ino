#include <MenuSystem.h>
#include <LiquidCrystal.h>
#include <EEPROM.h>
#include <TimeAlarms.h>
#include <Time.h>
#include <TimeLib.h>

//LCD init
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


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

  virtual void render_menu_item(MenuItem const & menu_item) const
  {
    lcd.print(menu_item.get_name());
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
};
MyRenderer my_renderer;

// Menu callback function
void on_1s(MenuItem* p_menu_item){}
void on_1t(MenuItem* p_menu_item){}
void on_1d(MenuItem* p_menu_item){}
void on_1f(MenuItem* p_menu_item){}

// Menu variables
MenuSystem ms(my_renderer);
Menu mm("Main Menu");
Menu pm_1("Pump 1");
Menu pm_2("Pump 2");
Menu pm_3("Pump 3");
Menu pm_4("Pump 4");
Menu sm("Settings");

MenuItem pm_1_stat("Status",&on_1s);
Menu pm_1_set("Settings");
MenuItem pm_1_set_t("Toggle",&on_1t);
MenuItem pm_1_set_d("Duration",&on_1d);
MenuItem pm_1_set_f("Frequency",&on_1f);












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
}

void setup() {
  
  Serial.begin(9600);
  lcd.begin(16, 2);

  ms.get_root_menu().add_menu(&pm_1);
  pm_1.add_item(&pm_1_stat);
  pm_1.add_menu(&pm_1_set);
  pm_1_set.add_item(&pm_1_set_t);
  pm_1_set.add_item(&pm_1_set_d);
  pm_1_set.add_item(&pm_1_set_f);

  ms.get_root_menu().add_menu(&pm_2);
  ms.get_root_menu().add_menu(&pm_3);
  ms.get_root_menu().add_menu(&pm_4);
  ms.get_root_menu().add_menu(&sm);
  
  ms.display();

  // setup lcdz
  // setup buttons, inputs outputs
  // set time

}

void loop() {
  serialHandler();
}
//This is a comment


