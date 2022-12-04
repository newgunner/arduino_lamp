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
#define MAX_BRIGHTNESS  255
#define MIN_BRIGHTNESS  0
#define STEP  8
#define FRAMES_PER_SECOND  60

enum menu {
  MENU_MODE = 0,
  MENU_BRIGHT,
  MENU_PALETTE,
  MENU_MAX,
};

enum rotate_direction {
  DIR_LEFT = -1,
  DIR_UNKNOWN = 0,
  DIR_RIGHT = 1,
};

enum mode {
  MODE_LAMP = 0,
  MODE_ANIMATION,
  MODE_ANIMATION_ALL,
  MODE_MAX,
};

EncButton<EB_CALLBACK, CLK_PIN, DT_PIN, SW_PIN> enc;

CRGB leds[NUM_LEDS];
CRGB white_temps[9] = {Candle, Tungsten40W, Tungsten100W, 
                  Halogen, CarbonArc, HighNoonSun, 
                  DirectSunlight, OvercastSky, ClearBlueSky};

uint8_t temp_num = 0; // number of palette in color temperature array
uint8_t rainbow_num = 0; // number of animation in patterns array
uint8_t brightness = 64;
uint8_t current_menu = 0;
uint8_t current_mode = 0;
uint8_t starthue = 0;

// List of patterns. Each is defined as a separate function below.
typedef void (*PatternList[])();
PatternList patterns = { &rainbow, &rainbowWithGlitter, &confetti, &sinelon, &juggle, &bpm };

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
  // change behavior of rotating based on current menu number
  int direction = DIR_UNKNOWN;
  if (enc.left())
    direction = DIR_LEFT;
  if (enc.right())
    direction = DIR_RIGHT;

  switch (current_menu) {
    case MENU_MODE:
      current_mode = (current_mode + 1) % MODE_MAX;
      break;
    case MENU_BRIGHT:
      if ((direction == DIR_LEFT) && (brightness <= (MIN_BRIGHTNESS + STEP)))
        brightness = MIN_BRIGHTNESS;
      else if ((direction == DIR_RIGHT) && (brightness >= (MAX_BRIGHTNESS - STEP)))
        brightness = MAX_BRIGHTNESS;
      else 
        brightness += STEP * direction;
      break;
    case MENU_PALETTE:
      if (current_mode == MODE_LAMP)
        temp_num = (temp_num + direction) % ARRAY_SIZE(white_temps);
      else if (current_mode == MODE_ANIMATION)
        rainbow_num = (rainbow_num + direction) % ARRAY_SIZE(patterns);
      break;
  }
}

void encClick() {
  // switch menu items
  current_menu = (current_menu + 1) % MENU_MAX;
}

void isr() {
  enc.tickISR();
}

void loop()
{
  enc.tick();   // tick in main loop as is in library example
  /*
  Serial.println(temp_num);
  Serial.print("menu: ");
  Serial.println(current_menu);
  Serial.print("mode: ");
  Serial.println(current_mode);
  Serial.print("brightness: ");
  Serial.println(brightness);
  */

  // TODO: replace 'if' to 'switch-case'
  if (current_mode == MODE_LAMP) {
    fill_solid( leds, NUM_LEDS, CRGB(255,255,255));
    leds[0] = 0x000000; // one LED is redundant, just paint it black. Too lazy to fix LED strip
    FastLED.setTemperature( white_temps[temp_num] );
  }
  else if (current_mode == MODE_ANIMATION || current_mode == MODE_ANIMATION_ALL) {
    patterns[rainbow_num]();
    FastLED.setTemperature(UncorrectedTemperature);
  }

  FastLED.setBrightness( brightness );
  FastLED.show();

  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND);

  // slowly move rainbow in strip
  EVERY_N_MILLISECONDS( 20 ) { starthue += 5; }

  // change patterns periodically if mode RAINBOW_ALL set
  EVERY_N_SECONDS( 10 ) {
    if (current_mode == MODE_ANIMATION_ALL)
      rainbow_num = (rainbow_num + 1) % ARRAY_SIZE(patterns);
  } 
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, starthue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( starthue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( starthue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, starthue+(i*2), beat-starthue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
