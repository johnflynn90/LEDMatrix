#include <Tlc5940.h>
#include <Time.h>

#define REFRESH_RATE     10
#define MAX_BRIGHTNESS   4095

#define NUM_STATES       9

#define RED_PIN          2
#define GREEN_PIN        3
#define BLUE_PIN         4

/* TODO:
  1. Make matrix sleep and wake up periodically.
  2. Make brightness adjustable.
*/

//int last_touch_time; // In minutes of the day.

int blue_toggled = 0;
int green_toggled = 0;
int red_toggled = 0;

// This maps virtual pins to phycial pins. Adjust this depending
// on the phisical architecture of a matrix.
int pixel_map[75] = 
{
  66, 65, 64, 60, 61, 62, 44, 45, 46, 28, 29, 30, 12, 13, 14,
  72, 71, 70, 57, 58, 59, 41, 42, 43, 25, 26, 27, 9, 10, 11,
  75, 74, 73, 54, 55, 56, 38, 39, 40, 22, 23, 24, 6, 7, 8,
  69, 68, 67, 51, 52, 53, 35, 36, 37, 19, 20, 21, 3, 4, 5,
  78, 77, 76, 48, 49, 50, 32, 33, 34, 16, 17, 18, 0, 1, 2
};

class Color {
  public:
  // Color properties.
  int ID;
  int state;
  int max_brightness;
  int cycles_elapsed;
  
  // Data.
  int position;
  long int brightness;
  long int direction;
  int sweep_x;
  int sweep_y;
  
  // RandomTwinkle
  long int all_led_brightness[25];
  long int all_led_direction[25];
  
  // Methods.
  int Tick(int period);
  void Reset(int new_brightness);
  int SetPixel(int x, int y, int value);
  
  // Test modes.
  void LEDSweep();
  
  // Modes.
  void LEDIdle();
  void LEDSolid();
  void LEDRandomTwinkle(int on_period, int new_led_interval);
  void LEDPulse();
  void LEDZoom();
  void LEDX();
  // void RandomTwinkle();
  void LEDBen();
  
  
public:
  // Interface.
  Color(int ID_in) : ID(ID_in), state(0), cycles_elapsed(0), max_brightness(MAX_BRIGHTNESS) {}
  void Update();
  void SetState(int state_in);
  void ToggleState();
};

Color Red(0), Green(1), Blue(2);

void setup()
{ 
  //Serial.begin(9600);
  pinMode(RED_PIN, INPUT);
  pinMode(GREEN_PIN, INPUT);
  pinMode(BLUE_PIN, INPUT);
  Red.SetState(0);
  Green.SetState(0);
  Blue.SetState(2);
  
  //last_touch_time = 60*hour() + minute();
  Tlc.init(0); // initialise TLC5940 and set all channels off
}

void loop()
{ 
  delay(REFRESH_RATE);
  
  // Read states of push buttons for blue, green and red.
  if (digitalRead(BLUE_PIN) == 0 && !blue_toggled) {
    blue_toggled = 1;
    Blue.ToggleState();
  }
  else if(digitalRead(BLUE_PIN) == 1) {
    blue_toggled = 0;
  }
  
  if (digitalRead(GREEN_PIN) == 0 && !green_toggled) {
    green_toggled = 1;
    Green.ToggleState();
  }
  else if(digitalRead(GREEN_PIN) == 1) {
    green_toggled = 0;
  }
  
  if (digitalRead(RED_PIN) == 0 && !red_toggled) {
    red_toggled = 1;
    Red.ToggleState();
  }
  else if(digitalRead(RED_PIN) == 1) {
    red_toggled = 0;
  }
  
  // Update each color.
  Green.Update();
  Red.Update(); 
  Blue.Update();
  
  Tlc.update();
}

// Returns true when a given interval of time has passed.
int Color::Tick(int period)
{
  if(cycles_elapsed * REFRESH_RATE / period > 0) {
    cycles_elapsed = 0;
    return 1;
  }
  else {
    cycles_elapsed++;
    return 0;
  }
}

// Reset all lights of this color.
void Color::Reset(int new_brightness)
{
  int i;
  for(i = 0; i < 25; i ++) {
    Tlc.set(pixel_map[3*i + ID], new_brightness);
  }
}

// Set a pixel to a certain brightness.
int Color::SetPixel(int x, int y, int value)
{
  Tlc.set(pixel_map[15*y + 3*x + ID], value);
}

// Update an effect.
void Color::Update()
{ 
  switch(state)
  {
  case 0:
    Reset(0);
    break;
  //case 1:
  //  LEDIdle();
    break;
  case 1:
   /// LEDSweep();
    LEDSolid();
    break;
  case 2:
    LEDRandomTwinkle(100, 50);
    break;
  case 3:
    LEDPulse();
    break;
  case 4:
    LEDZoom();
    break;
  case 5:
    LEDX();
    break;
  case 6:
    LEDRandomTwinkle(15, 5);
    break;
  case 7:
    LEDBen();
    break;
  }
}

// Sets the state of a color to an effect.
void Color::SetState(int state_in)
{
  state = state_in;
  max_brightness = MAX_BRIGHTNESS;
}

// Cycle through the available states.
void Color::ToggleState()
{
  int new_state = (state < (NUM_STATES-1)) ? state+1 : 0;
  
  SetState(new_state);
}

// NOT IMPLEMENTED: Sets the display to a low-power mode.
void Color::LEDIdle()
{
  max_brightness = MAX_BRIGHTNESS / 2;
  LEDRandomTwinkle(100, 200);
}

// Turn on a light, one at a time.
void Color::LEDSweep()
{
   Reset(0);
   SetPixel(sweep_x, sweep_y, 1000);

   if(sweep_x >= 4) {
     sweep_y = (sweep_y + 1) % 5;
     sweep_x = 0;
   }
   else {
     sweep_x++;
   }
}

// Randomly twinkle lights. Creates a random number of new lights
// every interval and randomly places them.
void Color::LEDRandomTwinkle(int on_period, int new_led_interval)
{
  int i;
  Reset(0);
  
  if(Tick(new_led_interval)) {
    int i;
    int contestants = random(8); // Spawn 0 to 7 new lights.
    
    for(i = 0; i < contestants; i++) {
      int winner = random(25);
      if(all_led_brightness[winner] == 0) {
        all_led_brightness[winner] = 1;
        all_led_direction[winner] = 1;
      }
    }
  }
  
  for(i = 0; i < 25; i++) {
    if(all_led_brightness[i] == 0) {
      continue;
    }
    
    all_led_brightness[i] += all_led_direction[i] * max_brightness * REFRESH_RATE / on_period;
    
    if(all_led_brightness[i] >= max_brightness) {
      all_led_direction[i] = -1;
      all_led_brightness[i] = max_brightness;
    }
    else if(all_led_brightness[i] <= 0) {
      all_led_direction[i] = 1;
      all_led_brightness[i] = 0;
    }
    
    Tlc.set(pixel_map[3*i + ID], all_led_brightness[i]);
  }
}

// Puse all of the LED's together.
void Color::LEDPulse()
{ 
  brightness += direction * max_brightness * REFRESH_RATE / 1000;
  
  if(brightness >= max_brightness) {
    direction = -1;
    brightness = max_brightness;
  }
  else if(brightness <= 0) {
    direction = 1;
    brightness = 0;
  }
  
  Reset(brightness);
}

// Create a manual zoom effect.
void Color::LEDZoom()
{
  if(!Tick(150)) {
    return;
  }
  
  Reset(0);
  brightness = max_brightness;

  switch(position)
  {
  case 0:
    SetPixel(2, 2, max_brightness);
    break;
  case 1:
    SetPixel(1, 1, max_brightness);
    SetPixel(1, 2, max_brightness);
    SetPixel(1, 3, max_brightness);
    SetPixel(2, 1, max_brightness);
    SetPixel(2, 3, max_brightness);
    SetPixel(3, 1, max_brightness);
    SetPixel(3, 2, max_brightness);
    SetPixel(3, 3, max_brightness);
    break;
  case 2:
    SetPixel(0, 0, max_brightness);
    SetPixel(0, 1, max_brightness);
    SetPixel(0, 2, max_brightness);
    SetPixel(0, 3, max_brightness);
    SetPixel(0, 4, max_brightness);
    SetPixel(1, 0, max_brightness);
    SetPixel(1, 4, max_brightness);
    SetPixel(2, 0, max_brightness);
    SetPixel(2, 4, max_brightness);
    SetPixel(3, 0, max_brightness);
    SetPixel(3, 4, max_brightness);
    SetPixel(4, 0, max_brightness);
    SetPixel(4, 1, max_brightness);
    SetPixel(4, 2, max_brightness);
    SetPixel(4, 3, max_brightness);
    SetPixel(4, 4, max_brightness);
    break;
  case 3:
    // Be blank.
    break;
  }
  
  position = (position < 3) ? position+1 : 0;
}

// Create a rotating X.
void Color::LEDX()
{
  if(!Tick(100)) {
    return;
  }
  
  Reset(0);
  brightness = max_brightness;

  switch(position)
  {
  case 0:
    SetPixel(2, 0, max_brightness);
    SetPixel(2, 1, max_brightness);
    SetPixel(2, 2, max_brightness);
    SetPixel(2, 3, max_brightness);
    SetPixel(2, 4, max_brightness);
    SetPixel(0, 2, max_brightness);
    SetPixel(1, 2, max_brightness);
    SetPixel(3, 2, max_brightness);
    SetPixel(4, 2, max_brightness);
    break;
  case 1:
    SetPixel(2, 2, max_brightness);
    SetPixel(1, 0, max_brightness);
    SetPixel(0, 3, max_brightness);
    SetPixel(3, 4, max_brightness);
    SetPixel(4, 1, max_brightness);
    break;
  case 2:
    SetPixel(0, 0, max_brightness);
    SetPixel(1, 1, max_brightness);
    SetPixel(2, 2, max_brightness);
    SetPixel(3, 3, max_brightness);
    SetPixel(4, 4, max_brightness);
    SetPixel(0, 4, max_brightness);
    SetPixel(1, 3, max_brightness);
    SetPixel(4, 0, max_brightness);
    SetPixel(3, 1, max_brightness);
    break;
  case 3:
    SetPixel(2, 2, max_brightness);
    SetPixel(0, 1, max_brightness);
    SetPixel(3, 0, max_brightness);
    SetPixel(4, 3, max_brightness);
    SetPixel(1, 4, max_brightness);
    break;
  }
  
  position = (position < 3) ? position+1 : 0;
}

// Create a zooming diamond.
void Color::LEDBen()
{
  if(!Tick(100)) {
    return;
  }
  
  Reset(0);
  brightness = max_brightness;

  switch(position)
  {
  case 0:
    SetPixel(2, 2, max_brightness);
    
    break;
  case 1:
    SetPixel(2, 2, max_brightness);

    break;
  case 2:
    SetPixel(2, 1, max_brightness);
    SetPixel(1, 2, max_brightness);
    SetPixel(2, 3, max_brightness);
    SetPixel(3, 2, max_brightness);
    break;
  case 3:
    SetPixel(2, 0, max_brightness);
    SetPixel(1, 1, max_brightness);
    SetPixel(2, 0, max_brightness);
    SetPixel(0, 2, max_brightness);
    SetPixel(1, 3, max_brightness);
    SetPixel(2, 4, max_brightness);
    SetPixel(3, 3, max_brightness);
    SetPixel(4, 2, max_brightness);
    SetPixel(3, 1, max_brightness);
    break;
  }
  
  position = (position < 3) ? position+1 : 0;
}

// Turns all LED's on with max brightness.
void Color::LEDSolid()
{
  Reset(max_brightness);
}
