#include <avr/pgmspace.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


Adafruit_PCD8544 display = Adafruit_PCD8544(8, 7, 5, 4, 3);

void plot_char_pointer(uint16_t base_address){
  int x = 0, y = 0;
  char c = 0;
  for (int i=0; i < 4032; i++){
    c = pgm_read_byte_near(base_address+i);

    if (c == '\0'){
      break;
    }

    if (c == '1'){
      display.drawPixel(x, y, BLACK);
    }
    x++;

    if (x % 84 == 0){
      x = 0;
      y++;
    }
  }
  //display.display();
}

void display_logo(uint16_t base_address){
  display.clearDisplay(); 
  plot_char_pointer(base_address);
  display.display();  
  delay(3000);
}

#define AXIS_Y A0
#define AXIS_X A1
#define BUTTON A2


class Pixel {

  private:
    uint16_t x, y;


  public:
    Pixel(uint16_t x, uint16_t y){
     this->x = x;
     this->y = y;
    }


    void draw(Adafruit_PCD8544 display, int color){
      display.drawPixel(this->x, this->y, color);
      display.drawPixel(this->x+1, this->y, color);
      display.drawPixel(this->x, this->y+1, color);
      display.drawPixel(this->x+1, this->y+1, color);
    }

    void draw(Adafruit_PCD8544 display){
      this->draw(display, BLACK);
      this->draw(display, BLACK);
    }


    void move(Adafruit_PCD8544 display, uint16_t x, uint16_t y){
      //display.drawPixel(this->x, this->y, WHITE);
      this->draw(display, WHITE);
      this->x = x;
      this->y = y;
      display.drawPixel(this->x, this->y, BLACK);
    }

    void move_right(Adafruit_PCD8544 display){
      this->move(display, this->x+1, this->y);
    }

    void move_left(Adafruit_PCD8544 display){
      this->move(display, this->x-1, this->y);
    }
    
    void move_up(Adafruit_PCD8544 display){
      this->move(display, this->x, this->y-1);
    }

    void move_down(Adafruit_PCD8544 display){
      this->move(display, this->x, this->y+1);
    }

};


class TimedExecution {

  private:
    unsigned long last_run;
    uint16_t delay;

  public:
    TimedExecution(uint16_t minimum_delay){
      this->delay = minimum_delay;
    }
    TimedExecution(){
      this->delay = 250;
    }

    void set_delay(uint16_t d){
      this->delay = d;
    }

    uint16_t get_delay(){
      return this->delay;    
    }

    bool expired(){
      unsigned long now = millis();

      if (now - last_run >= delay){
        this->last_run = now;
        return true;
      }
      return false;
    }

    void reset(){
      this->last_run = millis();
    }

};

Pixel p = Pixel(0, 0);
TimedExecution speed = TimedExecution(100);

void setup()
{
  Serial.begin(9600);
  display.begin();
  display.clearDisplay();
  display.display();

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  p.draw(display);
  display.display();
  speed.reset();
}

void loop()
{
  
  uint16_t axis_x = analogRead(AXIS_X);
  uint16_t axis_y = analogRead(AXIS_Y);
  uint16_t button = analogRead(BUTTON);

  if (speed.expired()){
    if (axis_x > 600){
      p.move_right(display);
      p.draw(display);
    }

    if (axis_x < 450){
      p.move_left(display);
      p.draw(display);
    }

    if (axis_y > 600){
      p.move_down(display);
      p.draw(display);
    }

    if (axis_y < 450){
      p.move_up(display);
      p.draw(display);
    }

    display.display();
  }

}
