#include <avr/pgmspace.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


Adafruit_PCD8544 display = Adafruit_PCD8544(8, 7, 5, 4, 3);

#define AXIS_Y A0
#define AXIS_X A1
#define BUTTON A2



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



class Pad {

  public:
    int x, y;
    const static int size = 10;
    const static int step = 2;
    Adafruit_PCD8544 *display;

  public:
    Pad(Adafruit_PCD8544 *display, uint16_t x, uint16_t y){
      this->x = x;
      this->y = y;
      this->display = display;
    }
    void move_up(){
      if (this->y - this->step >= 0){
        this->display->drawLine(this->x, this->y, this->x, this->y+this->size, WHITE);
        this->y-=step;
        this->draw();
      }
    }

    void move_down(){
      if (this->y + this->step + this->size <= 48){
        this->display->drawLine(this->x, this->y, this->x, this->y+this->size, WHITE);
        this->y+=step;
        this->draw();
      }
    }

    void draw(int color){
      this->display->drawLine(this->x, this->y, this->x, this->y+this->size, color); 
    }

    void draw(){
      this->draw(BLACK);
    }

};

class Pixel {

  public:
    uint16_t x, y;
    Adafruit_PCD8544 *display;


  public:
    Pixel(Adafruit_PCD8544 *display,  uint16_t x, uint16_t y){
     this->x = x;
     this->y = y;
     this->display = display;
    }


    void draw(int color){
      this->display->drawPixel(this->x, this->y, color);
      this->display->drawPixel(this->x+1, this->y, color);
      this->display->drawPixel(this->x, this->y+1, color);
      this->display->drawPixel(this->x+1, this->y+1, color);
    }

    void draw(){
      this->draw(BLACK);
      this->draw(BLACK);
    }


    void move(uint16_t x, uint16_t y){
      this->draw(WHITE);
      this->x = x;
      this->y = y;
      this->draw(BLACK);
    }

    void move_right(){
      this->move(this->x+1, this->y);
    }

    void move_left(){
      this->move(this->x-1, this->y);
    }
    
    void move_up(){
      this->move(this->x, this->y-1);
    }

    void move_down(){
      this->move(this->x, this->y+1);
    }

};


#define DIR_UP  1
#define DIR_DOWN 2
#define DIR_RIGHT 3
#define DIR_LEFT 4

class Ball: public Pixel {

  private:
    TimedExecution *speed;
    uint16_t direction_h;
    uint16_t direction_v;

  public:

    Ball (TimedExecution *speed, Adafruit_PCD8544 *display, uint16_t x, uint16_t y): Pixel(display, x, y){
      this->speed = speed;
      this->direction_h = DIR_LEFT;
      this->direction_v = DIR_UP;
    }

    void move(){
      if (this->speed->expired()){
        if (this->direction_v == DIR_UP){
          this->move_up();
        }else {
          this->move_down();
        }

        if (this->direction_h == DIR_LEFT){
          this->move_left();
        }else {
          this->move_right();
        }

        /* Bottom/Up Limit */
        if (this->y+1 == this->display->height() || this->y-1 == 0 ){
          this->reflect_up_down();
          return;
        }
        
      }
    }

    bool is_out(){
      int left_out = (int)this->x < (int)-1;
      int right_out = (int)this->x > this->display->width();
      return left_out || right_out;
    }

    void reflect_left_right(){
      if (this->direction_h == DIR_RIGHT){
        this->direction_h = DIR_LEFT;
      }else {
        this->direction_h = DIR_RIGHT;
      }
    }

    void reflect_up_down(){
      if (this->direction_v == DIR_DOWN){
        this->direction_v = DIR_UP;
      }else {
        this->direction_v = DIR_DOWN;
      }
    }

    bool colides(Pad pad){
      if ((this->x-1 == pad.x && this->direction_h == DIR_LEFT) && (this->y >= pad.y && this->y <= (pad.y + pad.size)) ){
        return true;
      }
      return false;
    }
};




TimedExecution speed = TimedExecution(100);
TimedExecution ball_speed = TimedExecution(50);

Ball ball = Ball(&ball_speed, &display, (uint16_t) 10, (uint16_t) 40);
Pad player1 = Pad(&display, 0, 26);
Pad player2 = Pad(&display, display.width()-1, 26);

void setup()
{
  Serial.begin(9600);
  display.begin();
  display.clearDisplay();
  display.display();

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);

  player1.draw();
  player2.draw();
  ball.draw();
  display.display();
  speed.reset();
}

void loop()
{
  
  uint16_t axis_x = analogRead(AXIS_X);
  uint16_t axis_y = analogRead(AXIS_Y);
  uint16_t button = analogRead(BUTTON);

  if (speed.expired()){
    if (axis_y > 600){
      player1.move_down();
      player1.draw();
    }

    if (axis_y < 450){
      player1.move_up();
      player1.draw();
    }
  }

  ball.move();
  display.display();

  if (ball.colides(player1) || ball.colides(player2)){
    Serial.println("Colides!");
    ball.reflect_left_right();
  }

  if (ball.is_out()){
    display.setCursor(10, 20);
    display.println("GAME OVER");
    display.display();
    ball.x = 60;
    ball.y = 20;
    delay(1500);
    display.clearDisplay();
    player1.draw();
    player2.draw();
    ball.draw();
  }


  display.display();
}
