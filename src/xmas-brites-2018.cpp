/*
 * This is a minimal example, see extra-examples.cpp for a version
 * with more explantory documentation, example routines, how to
 * hook up your pixels and all of the pixel types that are supported.
 *
 */
#include "application.h"
#include <neopixel.h>

// Need to define things before using them:
void draw();
void hsvtorgb(unsigned char *r, unsigned char *g, unsigned char *b, unsigned char h, unsigned char s, unsigned char v);

SYSTEM_MODE(AUTOMATIC);
//SYSTEM_THREAD(ENABLED); // This seems fragile currently

SerialLogHandler logHandler(LOG_LEVEL_TRACE);

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D2
// #define PIXEL_COUNT 27 // short strand on top of monitor
#define PIXEL_COUNT 300 // Full strand
#define PIXEL_TYPE WS2813

// Easier to toggle features per platform:
#if PLATFORM_ID == 12 // Argon
#elif PLATFORM_ID == 13 // Boron
#elif PLATFORM_ID == 14 // Xenon
#define SUB_GW_MSG // Receive GW -> Node pings (for Children in mesh)
#define NODE_PING_ENABLE // Send Node -> GW pings (for Children in mesh)

// #define D7_FPS
#define D7_GW_MSG
#endif

// Expose this thing to the internet:
int apiCommand(String command);

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Main Game Loop Timer:
Timer timer(16, draw); // 33 ~ 30fps (30/1000); 16 ~= 60fps
uint16_t currentLedIndex = 0;
bool ledState = false;
bool pinState = false;

#ifdef NODE_PING_ENABLE
// Handle Device Name being sent from cloud
bool nameKnown = false;
String deviceName = "Waiting for Cloud...";
void handler(const char *topic, const char *data) {
    Serial.println("received " + String(topic) + ": " + String(data));
    deviceName = String(data);
    nameKnown = true;
}
#endif // NODE_PING_ENABLE

#ifdef SUB_GW_MSG
// Handle mesh messages
bool meshStatus = false;
void gatewayMsgHandler(const char *event, const char *data) {
  meshStatus = !meshStatus;
  digitalWrite(D7, meshStatus);
}

#endif // SUB_GW_MSG

// Animation stuff
uint32_t frame = 0;

// HSV Color stuff
uint8_t r, g, b, sat = 255, val = 255;
uint8_t hue = 0;

// old
void rainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);
uint8_t speed = 10;

void setup()
{
    pinMode(D7, OUTPUT); // D7 will be used either for FPS output, or GW message state
    
#ifdef NODE_PING_ENABLE
    // If we're pinging the GW, we would like to send a name back:
	Particle.subscribe("particle/device/name", handler);
#endif // NODE_PING_ENABLE

#ifdef SUB_GW_MSG
    // IMPORTANT: In THREADED mode, we MUST wait for the mesh to be ready or we SOS 10.
    waitUntil(Mesh.ready);
    Mesh.subscribe("gw-msg", gatewayMsgHandler);
#endif // SUB_GW_MSG

    // Expose the command api to the world
    Particle.function("CMD", apiCommand);
    
    // Configure the RGB LED strip:
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    
    // Start the main draw loop:
    timer.start();
}

void loop()
{
    // uint16_t i;
    // for(i=0; i<strip.numPixels(); i++) {
    //   strip.setPixelColor(i, strip.Color(255, 255, 255));
    // }
    // strip.show();
    // // delay(1);
    
    // for(i=0; i<strip.numPixels(); i++) {
    //   strip.setPixelColor(i, strip.Color(0, 0, 0));
    // }
    // strip.show();
    
    // delay(1);
    
//   rainbow(speed);
  
    // strip.setPixelColor(0, strip.Color(255, 255, 255));
    // strip.show();
    // delay(500);
    // strip.setPixelColor(0, strip.Color(0, 255, 0));
    // strip.show();
    // delay(500);
    // strip.setPixelColor(0, strip.Color(0, 0, 255));
    // strip.show();
    // delay(500);
    
#ifdef NODE_PING_ENABLE
    // Request name from cloud ever 10 seconds:
    static uint32_t nextPublish = millis();
    if ((int32_t)(millis() - nextPublish) > 0) {
        nextPublish += 10 * 1000;
        Particle.publish("particle/device/name");
    }
    
    // Ping the GW every 100ms:
    static uint32_t nextPing = millis();
    if ((int32_t)(millis() - nextPing) > 0) {
        nextPing += 100;
        if (nameKnown) {
            Mesh.publish("node-msg", deviceName);
        }
    }
#endif
}

void draw() {
    // This is called every 33ms (close enough to 30 fps (30/1000))
#ifdef D7_FPS
    // Handle Pinout for testing framerate:
    pinState = !pinState;
    if (pinState) {
        digitalWrite(D7, HIGH);
    } else {
        digitalWrite(D7, LOW);
    }
#endif // D7_FPS

        
    // Handle LEDs:
    static uint32_t color;
    
    // All pixels different spot on rainbow
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      hsvtorgb(&r,&g,&b,frame*4+i,sat,val);
      color = strip.Color(r, g, b);
      strip.setPixelColor(i, color);
    }
    strip.show();

    // All pixels the same color:
    //hue += 1;
    //hsvtorgb(&r,&g,&b,hue,sat,val);
    //color = strip.Color(r, g, b);
    //for(uint16_t i=0; i<strip.numPixels(); i++) {
    //  strip.setPixelColor(i, color);
    //}
    //strip.show();

    // Add new pixel every frame:
    //currentLedIndex++;
    //if (currentLedIndex>PIXEL_COUNT) {
    //    currentLedIndex = 0;
    //    ledState = !ledState;
    //    if (ledState) {
    //        color = strip.Color(0, 255, 0);
    //    } else {
    //        color = strip.Color(0, 0, 255);
    //    }
    //}
    //strip.setPixelColor(currentLedIndex, color);
    //strip.show();
    // end new pixel every frame

    // Increment frame counter:
    frame += 1;
}

int apiCommand(String command) {
    int newSpeed = command.toInt();
    speed = newSpeed;
    return speed;
}

void rainbow(uint8_t wait) {
    uint16_t i, j;
    
    for(j=0; j<256; j++) {
        for(i=0; i<strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel((i+j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
    if(WheelPos < 85) {
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
}



// Taken from: http://web.mit.edu/storborg/Public/hsvtorgb.c
// And modified to work correctly. Integer overflows are bullshit.
void hsvtorgb(unsigned char *r, unsigned char *g, unsigned char *b, unsigned char h, unsigned char s, unsigned char v) {
    uint16_t region, fpart, p, q, t;

    if(s == 0) {
        // color is grayscale
        *r = *g = *b = v;
        return;
    }

    // make hue 0-5
    region = h / 43;
    //find remainder part, make it from 0-255
    fpart = (h - (region * 43)) * 6;

    // calculate temp vars, doing integer multiplication
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * fpart) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - fpart)) >> 8))) >> 8;

    // assign temp vars based on color cone region
    switch(region) {
        case 0:
            *r = v; *g = t; *b = p; break;
        case 1:
            *r = q; *g = v; *b = p; break;
        case 2:
            *r = p; *g = v; *b = t; break;
        case 3:
            *r = p; *g = q; *b = v; break;
        case 4:
            *r = t; *g = p; *b = v; break;
        default:
            *r = v; *g = p; *b = q; break;
    }

    return;
}







/*

#include "neopixel.h"

void setup();
void loop();
#line 10 "src/FancyProject.ino"
SYSTEM_MODE(AUTOMATIC);

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_PIN D2
#define PIXEL_COUNT 300
#define PIXEL_TYPE WS2813

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

// Prototypes for local build, ok to leave in for Build IDE
void rainbow(uint8_t wait);
uint32_t Wheel(byte WheelPos);

void setup()
{
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop()
{
  rainbow(0);
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


*/
