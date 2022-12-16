/*
  A simple Pong game:
  https://create.arduino.cc/projecthub/wotblitza/pong-with-oled-ssd1306-joystick-and-buzzer-58c423
 */

#include "src/Adafruit_GFX_Library/Adafruit_GFX.h"
#include "src/Adafruit_SSD1306/Adafruit_SSD1306.h"


const unsigned long PADDLE_RATE = 45;
const unsigned long BALL_RATE = 1;
const uint8_t PADDLE_HEIGHT = 12;
const uint8_t MAX_SPEED_COUNTER = 3;
const uint8_t MAX_HIT_COUNTER = 5;
int rightScore = 0;
int leftScore = 0;
int maxScore = 8;
//int BEEPER = 12;
bool resetBall = true;
uint8_t speedCounter = 0;
uint8_t hitCounter = 0;
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306* gamedisplay;

void drawCourt();
void drawScore();
void gameOver();
void incrementBallSpeed();

uint8_t ball_x = 64, ball_y = 32;
int8_t ball_dir_x = 0, ball_dir_y = 0;
unsigned long ball_update;

unsigned long paddle_update;

const uint8_t PLAYER_LEFT_X = 22;
int8_t player_left_y = 26;

const uint8_t PLAYER_RIGHT_X = 105;
int8_t player_right_y = 6;

bool player_left_isCpu = true;
bool player_right_isCpu = true;

int spinner_left = 0;
int spinner_right = 0;

void pongSetup() {
    gamedisplay = new Adafruit_SSD1306(128, 64, &Wire, -1);
    gamedisplay->begin(SSD1306_SWITCHCAPVCC, I2C_DISPLAY_ADDRESS);
    gamedisplay->display();
    unsigned long start = millis();
    //pinMode(BEEPER, OUTPUT);
    //pinMode(SW_pin, INPUT);
    //pinMode(RESET_BUTTON, INPUT_PULLUP);
    //digitalWrite(SW_pin, HIGH);
    gamedisplay->clearDisplay();
    while(millis() - start < 1000);//show adafruid splash

    gamedisplay->fillScreen(BLACK);
    gamedisplay->setCursor(38,0);
    gamedisplay->setTextColor(WHITE);
    gamedisplay->setTextSize(2);
    gamedisplay->print(F("PONG"));
    gamedisplay->display();
    while(millis() - start < 2000);//show PONG
    gamedisplay->fillScreen(BLACK);
    
    drawCourt();
    drawScore();  
    //while(millis() - start < 2000);

    gamedisplay->display();

    ball_update = millis();
    paddle_update = ball_update;
}

void pongLoop() {
    bool update = false;
    unsigned long time = millis();

    //static bool up_state = false;
    //static bool down_state = false;

    if(btnDebounce->fell(0))
      player_left_isCpu = false;
    if(btnDebounce->fell(1))
      player_right_isCpu = false;

    if(resetBall)
    {
      if(rightScore == maxScore || leftScore == maxScore)
            {
              gameOver();
            }
      else{      
      gamedisplay->fillScreen(BLACK);
      drawScore();
      drawCourt();       
      ball_x = random(45,50); 
      ball_y = random(23,33);
      do
      {
      ball_dir_x = random(-1,2);
      }while(ball_dir_x==0);

       do
      {
      ball_dir_y = random(-1,2);
      }while(ball_dir_y==0);
      
      
      resetBall=false;
      }
    }

    
    //up_state |= (digitalRead(UP_BUTTON) == LOW);
   // down_state |= (digitalRead(DOWN_BUTTON) == LOW);
   
    if(time > ball_update) {
        int16_t new_x = ball_x + ball_dir_x;
        int8_t new_y = ball_y + ball_dir_y;
        //bool isPaddleHit = false;

        // Check if we hit the vertical walls
        if(new_x <= 0 || new_x >= 127) {
         
          if(new_x <= 0){
            rightScore+=1;
            gamedisplay->fillScreen(BLACK);
            //soundPoint();
            resetBall = true;
            
          }
          else if(new_x >= 127){
            leftScore+=1;
            gamedisplay->fillScreen(BLACK);
            //soundPoint();
            resetBall = true;
          }
            ball_dir_x = -ball_dir_x;
            new_x += ball_dir_x + ball_dir_x;
            speedCounter = 0;
            hitCounter = 0;
        }

        // Check if we hit the horizontal walls.
        if(new_y <= 0 || new_y >= 63) {
            //soundBounce();
            if((ball_dir_y < 0 && new_y <= 0) || (ball_dir_y > 0 && new_y >= 63)) {
              ball_dir_y = -ball_dir_y;
              new_y += ball_dir_y + ball_dir_y;
            }
        }

        // Check if we hit the left paddle
        //if(new_x == PLAYER_LEFT_X && new_y >= player_left_y && new_y <= player_left_y + PADDLE_HEIGHT) {
        if(new_x >= PLAYER_LEFT_X-MAX_SPEED_COUNTER && new_x <= PLAYER_LEFT_X && new_y >= player_left_y && new_y <= player_left_y + PADDLE_HEIGHT) {
            ball_dir_x = -ball_dir_x;
            incrementBallSpeed();
            new_x += ball_dir_x + ball_dir_x;
        }

        // Check if we hit the right paddle
        /*if(new_x == PLAYER_RIGHT_X && new_y >= player_right_y && new_y <= player_right_y + PADDLE_HEIGHT)
        {*/
        if(new_x <= PLAYER_RIGHT_X+MAX_SPEED_COUNTER && new_x >= PLAYER_RIGHT_X && new_y >= player_right_y && new_y <= player_right_y + PADDLE_HEIGHT) {
            ball_dir_x = -ball_dir_x;
            incrementBallSpeed();
            new_x += ball_dir_x + ball_dir_x;
        }

        gamedisplay->drawPixel(ball_x, ball_y, BLACK);
        gamedisplay->drawPixel(new_x, new_y, WHITE);
        ball_x = new_x;
        ball_y = new_y;

        ball_update += BALL_RATE;

        update = true;
    }

    if(time > paddle_update) {
        paddle_update += PADDLE_RATE;

        // CPU paddle
        gamedisplay->drawFastVLine(PLAYER_LEFT_X, player_left_y, PADDLE_HEIGHT, BLACK);
        const uint8_t half_paddle = PADDLE_HEIGHT >> 1;
        if(player_left_isCpu) {
          if(player_left_y + half_paddle > ball_y)
              player_left_y -= speedCounter+1; //1;
          if(player_left_y + half_paddle < ball_y)
              player_left_y += speedCounter+1; //1;
        } else {
          if (spinner_left != 0)
            player_left_y = constrain(player_left_y + spinner_left, 1, 63 - PADDLE_HEIGHT);
          spinner_left = 0;
        }

        if(player_left_y < 1) player_left_y = 1;
        if(player_left_y + PADDLE_HEIGHT > 63) player_left_y = 63 - PADDLE_HEIGHT;
        gamedisplay->drawFastVLine(PLAYER_LEFT_X, player_left_y, PADDLE_HEIGHT, WHITE);

        // Player paddle
        gamedisplay->drawFastVLine(PLAYER_RIGHT_X, player_right_y, PADDLE_HEIGHT, BLACK);
        /*if(analogRead(Y_pin) < 480) {
            player_right_y -= 1;
        }
        if(analogRead(Y_pin) > 510) {
            player_right_y += 1;
        }*/
        if(player_right_isCpu) {
          if(player_right_y + half_paddle > ball_y)
              player_right_y -= speedCounter+1; //1;
          if(player_right_y + half_paddle < ball_y)
              player_right_y += speedCounter+1;// 1;
        } else {
          if (spinner_right != 0)
            player_right_y = constrain(player_right_y + spinner_right, 1, 63 - PADDLE_HEIGHT);
          spinner_right = 0;
        }


        
        //up_state = down_state = false;
        if(player_right_y < 1) player_right_y = 1;
        if(player_right_y + PADDLE_HEIGHT > 63) player_right_y = 63 - PADDLE_HEIGHT;
        gamedisplay->drawFastVLine(PLAYER_RIGHT_X, player_right_y, PADDLE_HEIGHT, WHITE);
    }
    update = true;
    
    if(update){
        drawScore();
        gamedisplay->display();

        if(btnDebounce->state(2) == LOW && btnDebounce->state(3) == LOW)
        {
          gameOver();
        }
    }
}

void drawCourt() {
    gamedisplay->drawRect(0, 0, 128, 64, WHITE);
}
void drawScore() {
  // draw AI and player scores
  gamedisplay->setTextSize(2);
  gamedisplay->setTextColor(WHITE);
  gamedisplay->setCursor(45, 0);
  gamedisplay->println(leftScore);
  gamedisplay->setCursor(75, 0);
  gamedisplay->println(rightScore);
}

void gameOver(){ 
  gamedisplay->fillScreen(BLACK);
  if(leftScore > rightScore)
  {
    gamedisplay->setCursor(15,0);
    gamedisplay->setTextColor(WHITE);
    gamedisplay->setTextSize(2);
    gamedisplay->print(F("LEFT WON"));
  } else if(rightScore > leftScore ){
    gamedisplay->setCursor(10,0);
    gamedisplay->setTextColor(WHITE);
    gamedisplay->setTextSize(2);
    gamedisplay->print(F("RIGHT WON"));
  } else {
    gamedisplay->setCursor(38,0);
    gamedisplay->setTextColor(WHITE);
    gamedisplay->setTextSize(2);
    gamedisplay->print(F("DRAW"));
  }
 delay(200);
 gamedisplay->display();
 delay(2000);
 leftScore = rightScore = 0;
  
  unsigned long start = millis();
  while(millis() - start < 2000);
  ball_update = millis();
  paddle_update = ball_update;
  resetBall=true;
  spinner_left = spinner_right = 0;
  speedCounter = 0;
  hitCounter = 0;
}

void incrementBallSpeed() {
  if(speedCounter < MAX_SPEED_COUNTER) {
    hitCounter++;
    if(hitCounter == MAX_HIT_COUNTER || hitCounter == (MAX_HIT_COUNTER*3) || hitCounter == (MAX_HIT_COUNTER*6)) {
      //hitCounter == 0;
      
      ball_dir_x = ball_dir_x + (ball_dir_x > 0 ? 1 : -1);
      
      if(speedCounter < MAX_SPEED_COUNTER/2)
        ball_dir_y = ball_dir_y + (ball_dir_y > 0 ? 1 : -1);
      
      speedCounter++;
    }
  }
}
