#include <avr/pgmspace.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <SoftwareSerial.h>
#include <math.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 7, 8);

#define AXIS_Y A0
#define AXIS_X A1
#define BUTTON A2

#define BANNER_HEIGHT 9


SoftwareSerial bt_player2(A4, A5); // RX, TX


#define DIRECTIONAL_P1 49
#define BUTTON_P1 54

#define DIRECTIONAL_P2 50
#define BUTTON_P2 56

#define KEYDOWN 68
#define KEYUP 85


int number_of_digits(uint16_t n){
  return n > 0 ? (int) log10 ((double) n) + 1 : 1;
}

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



class Pong{

  private:
    Adafruit_PCD8544 *display;
    TimedExecution blink_on;
    bool color;

    void print_pushstart(int x, int y, int color){
      this->display->setTextColor(color);
      this->display->setTextSize(1);
      this->display->setCursor(x, y);
      this->display->println("PUSH");
      this->display->setCursor(x-3, y+8);
      this->display->println("START");
    }

  public:
    Pong(Adafruit_PCD8544 *display){
      this->display = display;
      this->blink_on.set_delay(500);
      this->blink_on.reset();
      this->color = true;
    }

  void splash(bool p1_entered, bool p2_entered){
    this->display->setCursor(20, 0);
    this->display->setTextColor(BLACK);
    this->display->setTextSize(2);
    this->display->println("PONG");

    if (this->blink_on.expired()){
      if (this->color){
        if (!p1_entered){
          this->print_pushstart(5, 30, BLACK); // player 1
        }
        if (!p2_entered){
          this->print_pushstart(55, 30, BLACK); // player 2
        }
      
      }else {
        this->print_pushstart(5, 30, WHITE);
        this->print_pushstart(55, 30, WHITE);
      }
      this->color = !this->color;
    }
  }
};


class Pad {

  public:
    int x, y;
    const static int size = 10;
    const static int step = 2;
    Adafruit_PCD8544 *display;
    int pin_x, pin_y;
    uint16_t min, max;
    int data[3];
    bool start_pressed;
    HardwareSerial *hw_serial;
    SoftwareSerial *sw_serial;

    
    void init_values(Adafruit_PCD8544 *display, uint16_t x, uint16_t y){
      this->x = x;
      this->y = y;
      this->display = display;
      this->start_pressed = false;
    }

    Pad(Adafruit_PCD8544 *display, uint16_t x, uint16_t y, SoftwareSerial *serial_line){
      this->init_values(display, x, y);
      this->sw_serial = serial_line;
    }

    Pad(Adafruit_PCD8544 *display, uint16_t x, uint16_t y, HardwareSerial *serial_line){
      this->init_values(display, x, y);
      this->hw_serial = serial_line;
    }

    void move_up(){
      if (this->y - this->step >= BANNER_HEIGHT){
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
    
    void process_data(int *data){
      if ((data[0] == DIRECTIONAL_P1) && (data[1] == 51) && (data[2] == KEYDOWN)){
        this->move_up();
      }

      if ((data[0] == DIRECTIONAL_P1) && (data[1] == 52) && (data[2] == KEYDOWN)){
        this->move_down();
      }

      if (data[0] == BUTTON_P1 && data[1] == 67 && data[2] == KEYDOWN){
        this->start_pressed = true;
      }

      this->draw();
    }

    void process_input_player(){

      if (sw_serial){
        if (sw_serial->available()){
          data[0] = sw_serial->read();
          data[1] = sw_serial->read();
          data[2] = sw_serial->read();
        }
      }

      if (hw_serial){
        if (hw_serial->available()){
          data[0] = hw_serial->read();
          data[1] = hw_serial->read();
          data[2] = hw_serial->read();
        }
      }

      this->process_data(data);
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
        if (this->y+1 == this->display->height() || this->y-1 == BANNER_HEIGHT ){
          this->reflect_up_down();
          return;
        }

      }
    }

    int is_out(){
      int left_out = (int)this->x < (int)-1;
      int right_out = (int)this->x > this->display->width();
      if (left_out){
        return -1;
      }
      if (right_out){
        return 1;
      }
      return 0;
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
      if (
          ((this->x-1 == pad.x && this->direction_h == DIR_LEFT)  
          || (this->x+2 == pad.x && this->direction_h == DIR_RIGHT)) 
          && (this->y >= pad.y && this->y <= (pad.y + pad.size)) ){

        return true;
      }
      return false;
    }
};

class Placar{

  public:
    uint16_t _score_left, _score_right;
    Adafruit_PCD8544 *display;

    Placar(Adafruit_PCD8544 *display){
        this->display = display;
        this->_score_right = 0;
        this->_score_left = 0;
    }

    void reset(){
      this->_score_left = 0;
      this->_score_right = 0;
    }

    void draw(){
      this->display->fillRect(0, 0, this->display->width(), 9, BLACK);
      this->display->setTextSize(1);
      this->display->setCursor(30, 1);
      this->display->setTextColor(WHITE);
      this->display->print("PONG");

      this->display->setCursor(1, 1);
      this->display->print(this->_score_left);
      
      this->display->setCursor(this->display->width() - (6 * number_of_digits(this->_score_right)), 1);
      this->display->print(this->_score_right);

    }

    void score_left(){
      this->_score_left++;
      this->show_score(true);
      this->draw();
    }

    void score_right(){
      this->_score_right++;
      this->show_score(true);
      this->draw();
    }

    void show_score(bool show_goal){
      this->display->setTextColor(BLACK);

      if (show_goal){
        this->display->setTextSize(1);
        this->display->setCursor(15, 10);
        this->display->print("GOOOOL!!!");
      }

      this->display->setTextSize(2);
      this->display->setCursor(5, 20);
      this->display->print(this->_score_left);

      this->display->setCursor(this->display->width() - (12 * number_of_digits(this->_score_right)) - 10, 20);
      this->display->print(this->_score_right);

    }

};


TimedExecution pad_speed = TimedExecution(100);
TimedExecution ball_speed = TimedExecution(30);

Placar placar = Placar(&display);
Ball ball = Ball(&ball_speed, &display, (uint16_t) 42, (uint16_t) 24);
Pad player1 = Pad(&display, 0, 26, &Serial);
Pad player2 = Pad(&display, display.width()-1, 26, &bt_player2);

void setup()
{
  Serial.begin(9600);
  bt_player2.begin(9600);
  display.begin();
  display.clearDisplay();
  display.display();

  player1.draw();
  player2.draw();
  ball.draw();
  player2.draw();
  placar.draw();

  display.clearDisplay();
  display.display();
  delay(2000);
}


Pong pong(&display);

bool show_splash = true;

void loop()
{
  while (show_splash){
    player1.process_input_player();
    player2.process_input_player();
    pong.splash(player1.start_pressed, player2.start_pressed);
    display.display();

    if (player1.start_pressed && player2.start_pressed){
      display.clearDisplay();
      show_splash = false;
      ball.draw();
      placar.draw();
      player1.draw();
      player2.draw();
      display.display();
      delay(3000);
      break;
    }
  }
    
  if (pad_speed.expired()){
    player1.process_input_player();
    player2.process_input_player();
  }

  placar.draw();
  ball.move();

  if (ball.colides(player1) || ball.colides(player2)){
    ball.reflect_left_right();
    ball.move();
  }

  // Player marcou ponto
  int is_out = ball.is_out();
  if (is_out){

    if (is_out < 0){
      placar.score_right();
    }else {
      placar.score_left();
    }

    display.display();  /* Mostra o placar grande */
    ball.x = 42;
    ball.y = 24;
    delay(1500);

    display.clearDisplay();
  }

  // Fim de Jogo, player ganhou.
  if (placar._score_left >= 5 || placar._score_right >= 5){
    placar.show_score(false);
    display.setTextSize(1);
    display.setCursor(10, 10);
    display.print("FIM DE JOGO");
    display.display();
    player1.start_pressed = false;
    while (!player1.start_pressed){
      player1.process_input_player();
      delay(100);
    }
    placar.reset();
    display.clearDisplay();
  }

  display.display();
}
