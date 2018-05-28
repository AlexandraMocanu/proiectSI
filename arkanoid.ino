// library SPFD5408 
#include <SPFD5408_Adafruit_GFX.h>    // Core graphics library
#include <SPFD5408_Adafruit_TFTLCD.h> // Hardware-specific library
#include <SPFD5408_TouchScreen.h>     // Touch library

// game files

#include <block.h>
#include <globals.h>
// #include <"box.h">
// #include <"ball.h">
// #include <"game.h">

// Calibrates value
#define SENSIBILITY 300
#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define XM A1 // must be an analog pin, use "An" notation!
#define YP A2 // must be an analog pin, use "An" notation!
#define XP 7 // can be a digital pin
#define YM 6 // can be a digital pin

//With calibration
//short TS_MINX=168;
//short TS_MINY=157;
//short TS_MAXX=961;
//short TS_MAXY=929;

//second
short TS_MINX=153;
short TS_MINY=923;
short TS_MAXX=933;
short TS_MAXY=223;


TouchScreen ts = TouchScreen(XP, YP, XM, YM, SENSIBILITY);

// LCD Pin
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4 // Optional : otherwise connect to Arduino's reset pin

// Init LCD

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// lastTouch

TSPoint lastTouch;

// hope it ain't burnin

void(* resetFunc) (void) = 0; 

//-- Setup

int frameStart, frameTime, frameCount;
int frameRate = 60;
int MS_PER_FRAME = 1000 / frameRate;

void setup(void) {

  Serial.begin(9600);

  tft.reset();
  tft.begin(0x9341); //hardcoded due to idk
  
  tft.setRotation(3); // Need for the Mega, please changed for your choice or rotation initial

  width = tft.width() - 1;
  height = tft.height() - 1;

  // Debug
  
  Serial.println(F("TFT LCD test"));
  Serial.print("TFT size is "); 
  Serial.print(tft.width()); 
  Serial.print("x"); 
  Serial.println(tft.height());
  // Border

  drawBorder();
  
  // Initial screen
  
  tft.setCursor(65, 50);
  tft.setTextSize (4);
  tft.setTextColor(RED);
  tft.println("Arkanoid");
  
  tft.setCursor(70, 100);
  tft.setTextSize (2);
  tft.setTextColor(BLACK);
  tft.println("Touch to start!");
  
  tft.setCursor(150, 210);
  tft.setTextSize (1);
  tft.setTextColor(BLACK);
  tft.println("Made by: Cazacu Robert");
  tft.setCursor(203, 220);
  tft.println("Mocanu Alexandra");
  
  lastTouch = waitOneTouch();
  drawGameBorder();
  gameSetUp();
}

// -- Loop


void loop()
{
//  //ok, fix this FPS thinggy :)
//  int frameStart = millis();
  
  processInput();
  preUpdateCleanup();
  update();
  render();
//  Serial.println(++frameCount);
  
//  frameTime = millis() - frameStart;
//  if (MS_PER_FRAME > frameTime) {
//    Serial.println("Needs delay");
//    delay(MS_PER_FRAME - frameTime);
//  }
}


void update() {
  if (life_count <= 0)
    resetFunc();
  ball -> move();
}

TSPoint tsp;
void processInput() {
  tsp = currentTouch();
  if( abs(lastTouch.x - tsp.x) > 10 ) {
    deleteOldBar();
    lastTouch = tsp;
    showTouched(lastTouch);
    bar_dx = bar_x;
    bar_x = getNewBar();
    bar_dx = -(bar_dx - bar_x) / abs(bar_dx - bar_x);
  } else {
    bar_dx = 0;
  }
}

void preUpdateCleanup() {
  deleteOldBall();
}

void render() {
  for(uint32_t i = 0; i < box_count; i++)
    if( box[i] -> needsRedraw && !box[i] -> dead)
      box[i] -> draw(tft);
  bar_draw();
  ball -> draw(tft);
  drawLifes();
}

void drawLifes() {
  tft.setCursor(10, 6);
  tft.setTextSize (1);
  tft.setTextColor(WHITE);
  tft.println("Lifes: ");

  tft.fillRect(50, 6, 5, 5, BLACK);
  tft.setCursor(50, 6);
  tft.println(life_count);
}

void deleteOldBar() {
  tft.fillRect(bar_x, bar_y, bar_l, bar_t, BLACK);
}

void deleteOldBall() {
   tft.fillCircle(ball -> x, ball -> y, ball -> r, BLACK);
}

void bar_draw() {
  tft.fillRect(bar_x, bar_y, bar_l, bar_t, RED);
}

void gameSetUp() {
    ball = new Ball();
    box = new Block*[box_count];
    for(uint32_t st = 0; st < box_count; st++) {
      box[st] = new Block();
    }
    
   for(uint32_t p = 0; p < box_row; p++){
      for(uint32_t q = 0; q < box_column; q++)
      {
        box[p*box_column + q]->x = offsetX + box[p*box_column + q]->l * q;
        box[p*box_column + q]->y = offsetY + box[p*box_column + q]->t * p;
      }
    }
}

int getNewBar() {
  return clamp( 5 - ((26 * (lastTouch.y - 900)) / 69),
    5,
    264);
}

int clamp (int x, int left, int right) {
  if (x < left) x = left;
  if (x > right) x = right;
  return x;
}

TSPoint currentTouch() {
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT); //Pins configures again for TFT control
  pinMode(YP, OUTPUT);

 if ((p.z > MINPRESSURE ) && (p.z < MAXPRESSURE))
  return p;
 else
  return lastTouch;
}

// wait 1 touch to return the point 

TSPoint waitOneTouch() {
  
  TSPoint p;
  
  do {
    p= ts.getPoint(); 
  
    pinMode(XM, OUTPUT); //Pins configures again for TFT control
    pinMode(YP, OUTPUT);
  
  } while((p.z < MINPRESSURE )|| (p.z > MAXPRESSURE));
  
  return p;
}


void drawGameBorder () {

  uint16_t width = tft.width() - 1;
  uint16_t height = tft.height() - 1;
  uint8_t border = 5;

  tft.fillScreen(RED);
  tft.fillRect(border, border, (width - border * 2), (height - border + 1), BLACK);
  
}


// Draw a border

void drawBorder () {

  uint16_t width = tft.width() - 1;
  uint16_t height = tft.height() - 1;
  uint8_t border = 10;

  tft.fillScreen(RED);
  tft.fillRect(border, border, (width - border * 2), (height - border * 2), WHITE);
  
}

// Show the coordinates     

void showTouched(TSPoint p) {

  uint8_t w = 40; // Width
  uint8_t h = 10; // Heigth
  uint8_t x = (width - (w*2)); // X
  uint8_t y = 11; // Y
  
  tft.fillRect(x, y, w*2, h, WHITE); // For cleanup

  tft.drawRect(x, y, w, h, RED); // For X
  tft.drawRect(x+w+2, y, w*2, h, RED); // For Y

  tft.setTextColor(BLACK);
  tft.setCursor(x+2, y + 1);
  tft.print("X: ");
  showValue(p.x);
  
  tft.setCursor(x+2+w+2, y + 1);
  tft.print("Y: ");
  showValue(p.y);

}

// Show a value of TSPoint

void showValue (uint16_t value) {

  if (value < 10)
    tft.print("00");
  if (value < 100)
    tft.print("0");
    
  tft.print(value);
  
}

// Map the coordinate X
  
uint16_t mapXValue(TSPoint p) {

  uint16_t x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());

  //Correct offset of touch. Manual calibration
  //x+=1;
  
  return x;

}

// Map the coordinate Y

uint16_t mapYValue(TSPoint p) {

  uint16_t y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

  //Correct offset of touch. Manual calibration
  //y-=2;

  return y;
}


