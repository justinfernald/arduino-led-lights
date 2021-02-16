#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ESP8266_RAW_PIN_ORDER

#include <FastLED.h>
#include <IRremote.h>

#define NUM_LEDS 150

class Color
{
public:
  int hue = 0;
  int sat = 255;
  int val = 255;
  Color() {}
  Color(int hue, int sat, int val)
  {
    this->hue = hue;
    this->sat = sat;
    this->val = val;
  }

  CRGB toCRGB()
  {
    return CHSV(hue, sat, val);
  }
};

CRGB leds[NUM_LEDS];

bool on = true;
bool patternMode = false; // set back to true
bool paused = false;
bool reversed = false;
bool breatheMode = true; // set back to false
bool partyACTIVATED = false;

Color favColors[] = {Color(133, 255, 255), Color(0, 0, 255), Color(193, 255, 255), Color(21, 255, 255), Color(217, 255, 255), Color(96, 255, 255)}; // blue, white, purple, orange, pink, red
const int favColorsSize = 6;
int favColorIndex = 5;
Color currentColor = favColors[favColorIndex];

const int favPatternsSize = 2;
int favPatternIndex = 0;

int frameCount = 0;
double speedUp = 1;
double brightness = 0.5;

int pressedCount = 0;
int lastPressed = 0;
long lastPressedTime = millis();

const int delayInterval = 500;

const int ledPin = 4;      //marked as D2 on the board
                           // ground // G
const int irPowerPin = 13; // R
const int irDataPin = A0;  // Y

IRrecv receiver(irDataPin);
decode_results output;

void setup()
{
  Serial.begin(115200);
  FastLED.addLeds<WS2812B, ledPin, RGB>(leds, NUM_LEDS);
  pinMode(irPowerPin, OUTPUT);
  digitalWrite(irPowerPin, HIGH);
  pinMode(irDataPin, INPUT);
  receiver.enableIRIn();
}

bool show = true;

CRGB color;

void loop()
{
  double inputValue = frameCount * speedUp * (reversed ? -1 : 1);
  if (patternMode)
  {
    switch (favPatternIndex)
    {
    case 0: // rainbow
      for (int i = 0; i < NUM_LEDS; i++)
      {
        Color newColor(mod(currentColor.hue + (int)(10 * (inputValue + i)), 256), currentColor.sat, currentColor.val * brightness);
        leds[i] = newColor.toCRGB();
      }
      break;
    case 1: // random back and forth
      if (mod((int)inputValue, 300) < 150)
      {
        if (mod((int)inputValue, 300) == 0)
        {
          color = CHSV(random(255), currentColor.sat, (int)(255 * brightness));
        }
        leds[mod((int)inputValue, 150)] = color;
        //    leds[x % 150] = CRGB(random(255), random(255), random(255));
      }
      else
      {
        if (mod((int)inputValue, 300) == 150)
        {
          color = CHSV(random(255), currentColor.sat, (int)(255 * brightness));
        }
        leds[149 - (mod((int)inputValue, 150))] = color;
        //    leds[149 - (x % 150)] = CRGB(random(255), random(255), random(255));
      }
      break;
    case 2: // random bouncing bunch (meet in middle)

      break;
    case 3: // center color trails

      break;
    case 4: // blue cyan trails

      break;
    }
  }
  else if (partyACTIVATED)
  {
  }
  else
  {
    if (breatheMode)
    {
      for (int i = 0; i < NUM_LEDS; i++)
      {
        Color newColor(currentColor.hue, (int)(currentColor.sat * ((sin(0.1 * inputValue + 0.1 * i) + 1) / 2)), (int)(currentColor.val * brightness * ((sin(0.1 * inputValue + 0.1 * i) + 1) / 2)));
        leds[i] = newColor.toCRGB();
      }
    }
    else
    {
      for (int i = 0; i < NUM_LEDS; i++)
      {
        Color newColor(currentColor.hue, currentColor.sat, currentColor.val * brightness);
        leds[i] = newColor.toCRGB();
      }
    }
  }

  FastLED.show();

  frameCount++;

  delay(35);

  irBoy();
}

void deSet()
{
  patternMode = false;
  paused = false;
  reversed = false;
  breatheMode = false;
  partyACTIVATED = false;
  speedUp = 1;
  frameCount = 0;
}

void irBoy()
{
  while (!IrReceiver.isIdle())
    ;
  if (IrReceiver.decode())
  {

    // 69 70 71
    // 68 64 67
    // 07 21 09
    // 22 25 13
    // 12 24 94
    // 08 28 90
    // 66 82 74
    IrReceiver.resume();
    if (IrReceiver.decodedIRData.command == 0)
    {
      Serial.print(".");
      pressedCount = 0;
      lastPressed = 0;
      return;
    }
    if (IrReceiver.decodedIRData.command == lastPressed)
    {
      pressedCount++;
    }
    else
    {
      pressedCount = 0;
    }

    lastPressed = IrReceiver.decodedIRData.command;
    lastPressedTime = millis();

    Serial.println("--------------------------");
    //        Serial.print("Value: ");
    //        Serial.println(IrReceiver.decodedIRData.decodedRawData);
    //        Serial.println(IrReceiver.decodedIRData.command);
    switch (IrReceiver.decodedIRData.command)
    {
    case 69: // power : toggle on / off
      if (pressedCount != 0)
        return;
      on = !on;
      break;
    case 70: // vol up : brightness up
      brightness += 0.05;
      if (brightness > 1)
        brightness = 1;
      break;
    case 71: // func / stop : PARTY
      deSet();
      partyACTIVATED = true;
      break;
    case 68: // backward : reverse
      if (pressedCount != 0)
        return;
      reversed = true;
      break;
    case 64: // play / pause : freeze
      if (pressedCount != 0)
        return;
      paused = !paused;
      break;
    case 67: // forward : de-reverse
      if (pressedCount != 0)
        return;
      reversed = false;
      break;
    case 7: // down : decrease speed
      speedUp -= 0.1;
      if (speedUp < 0.05)
        speedUp = 0.05;
      break;
    case 21: // vol down : brightness down
      brightness -= 0.05;
      if (brightness < 0)
        brightness = 0;
      break;
    case 9: // up : increase speed
      speedUp += 0.1;
      if (speedUp > 3)
        speedUp = 3;
      break;
    case 22: // 0 : prev pattern
      if (pressedCount != 0)
        return;
      if (patternMode)
      {
        favPatternIndex = (favPatternIndex + favPatternsSize - 1) % favPatternsSize;
      }
      deSet();
      patternMode = true;
      break;
    case 25: // eq : toggle color breathe
      if (pressedCount != 0)
        return;
      if (!patternMode && !partyACTIVATED)
      {
        breatheMode = !breatheMode;
      }
      else
        deSet();
      break;
    case 13: // st/rept : next pattern
      if (pressedCount != 0)
        return;
      if (patternMode)
      {
        favPatternIndex = (favPatternIndex + 1) % favPatternsSize;
      }
      deSet();
      patternMode = true;
      break;
    case 12: // 1 : color or pattern

      break;
    case 24: // 2 : color or pattern

      break;
    case 94: // 3 : color or pattern

      break;
    case 8: // 4 : down sat
            //      if (!patternMode && !partyACTIVATED)
            //      {
      currentColor.sat -= 9;
      if (currentColor.sat < 0)
        currentColor.sat = 0;
      //      }
      //      deSet();
      break;
    case 28: // 5 : prev color
      if (pressedCount != 0)
        return;
      if (!patternMode && !partyACTIVATED)
      {
        favColorIndex = (favColorIndex + favColorsSize - 1) % favColorsSize;
        currentColor = favColors[favColorIndex];
        deSet();
        break;
      case 90: // 6 : up sat
               //      if (!patternMode && !partyACTIVATED)
               //      {
        currentColor.sat += 9;
        if (currentColor.sat > 255)
          currentColor.sat = 255;
        //      }
        //      deSet();
        break;
      case 66: // 7 : down hue
               //      if (!patternMode && !partyACTIVATED)
               //      {
        currentColor.hue -= 3;
        if (currentColor.hue < 0)
          currentColor.hue = 0;
        //      }
        //      deSet();
        break;
      case 82: // 8 : next color
        if (pressedCount != 0)
          return;
        if (!patternMode && !partyACTIVATED)
        {
          favColorIndex = (favColorIndex + 1) % favColorsSize;
          currentColor = favColors[favColorIndex];
        }
        deSet();
        break;
      case 74: // 9 : up hue
               //      if (!patternMode && !partyACTIVATED)
               //      {
        currentColor.hue += 3;
        if (currentColor.hue > 255)
          currentColor.hue = 255;
        //      }
        //      deSet();
        break;
      }
      printStatus();
    }
    else if (millis() - lastPressedTime > delayInterval)
    {
      pressedCount = 0;
      lastPressed = 0;
    }
  }

  void printStatus()
  {
    Serial.print("Lights On: ");
    Serial.println(on ? "On" : "Off");

    Serial.print("Pattern Mode: ");
    Serial.println(patternMode ? "On" : "Off");

    Serial.print("Party Mode: ");
    Serial.println(partyACTIVATED ? "PAARRTTTYYY" : "no party :(");

    Serial.print("Paused status: ");
    Serial.println(paused ? "Paused" : "Playing");

    Serial.print("Breathe Mode: ");
    Serial.println(breatheMode ? "Breathing" : "Off");

    Serial.print("Reversed: ");
    Serial.println(reversed ? "Reversed" : "Forward");

    Serial.print("Brightness: ");
    Serial.println(brightness);

    Serial.print("Speed up:");
    Serial.println(speedUp);

    Serial.print("Color Index: ");
    Serial.println(favColorIndex);

    Serial.print("Pattern Index: ");
    Serial.println(favPatternIndex);

    Serial.print("Current Color: ");
    Serial.print(currentColor.hue);
    Serial.print(" ");
    Serial.print(currentColor.sat);
    Serial.print(" ");
    Serial.println(currentColor.val);
  }

  int mod(int value, int m)
  {
    int mod = value % m;
    if (mod < 0)
      mod += m;
    return mod;
  }
