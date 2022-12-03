#include <FastLED.h>

#define LED_PIN     6

// Information about the LED strip itself
#define NUM_LEDS    61 // actually 60, but one is redundant after glue the whole strip
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
#define BRIGHTNESS  128

#define FRAMES_PER_SECOND  30
CRGB temperatures[10] = {Candle, Tungsten40W, Tungsten100W, 
                  Halogen, CarbonArc, HighNoonSun, 
                  DirectSunlight, OvercastSky, ClearBlueSky, 
                  0x000000};

int i = 0;
void loop()
{
  Serial.println(i);
  fill_solid( leds, NUM_LEDS, CRGB(255,255,255));
  leds[0] = 0x000000; //one LED is redundant, just paint it black. Too lazy to fix LED strip
  FastLED.setTemperature( temperatures[i] );


  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // switch between temps
  EVERY_N_SECONDS(5) {i = (i + 1) % ARRAY_SIZE(temperatures);}
}

void setup() {
  delay( 3000 ); // power-up safety delay
  // It's important to set the color correction for your LED strip here,
  // so that colors can be more accurately rendered through the 'temperature' profiles
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  Serial.begin(9600);
}

