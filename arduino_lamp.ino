#include <FastLED.h>
#include <EncButton.h>

// PIN used
#define LED_PIN     6
#define CLK_PIN     2
#define DT_PIN      3
#define SW_PIN      4

// Information about the LED strip itself
#define NUM_LEDS    61 // actually 60, but one is redundant after glue the whole strip
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB

// other stuff
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#define BRIGHTNESS  10
#define FRAMES_PER_SECOND  60

EncButton<EB_CALLBACK, CLK_PIN, DT_PIN, SW_PIN> enc;

CRGB leds[NUM_LEDS];
CRGB temperatures[10] = {Candle, Tungsten40W, Tungsten100W, 
                  Halogen, CarbonArc, HighNoonSun, 
                  DirectSunlight, OvercastSky, ClearBlueSky, 
                  0x000000};

uint8_t temp_num = 0; // number of palette in color temperature array
uint8_t brightness = 64;

void setup() {
  delay( 3000 ); // power-up safety delay
  // It's important to set the color correction for your LED strip here,
  // so that colors can be more accurately rendered through the 'temperature' profiles
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( brightness );
  Serial.begin(9600);
  enc.attach(TURN_HANDLER, encTurn);   // attach handler for turning rotary encoder
  enc.attach(CLICK_HANDLER, encClick);   // attach handler for clicking rotary encoder

  attachInterrupt(0, isr, CHANGE); // attach interrupts
  attachInterrupt(1, isr, CHANGE); // in main loop we have delays => turning events without interrupts are unstable
}

void encTurn() {
  if (enc.left())
    if (brightness <= 4)
      brightness = 0;
    else
      brightness-=4;
  if (enc.right()) 
    if (brightness >= 251)
      brightness = 255;
    else
      brightness+=4;
}

void encClick() {
  temp_num = (temp_num + 1) % ARRAY_SIZE(temperatures);
}

void isr() {
  enc.tickISR();
}

void loop()
{
  enc.tick();   // tick in main loop as is in library example
  //Serial.println(temp_num);
  //Serial.print("brightness: ");
  //Serial.println(brightness);

  fill_solid( leds, NUM_LEDS, CRGB(255,255,255));
  leds[0] = 0x000000; //one LED is redundant, just paint it black. Too lazy to fix LED strip
  FastLED.setBrightness( brightness );
  FastLED.setTemperature( temperatures[temp_num] );

  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND);
}



