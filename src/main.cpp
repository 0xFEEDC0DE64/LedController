#include <array>
#include <functional>

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <FastLED.h>

#include "ledcontroller.h"

FASTLED_USING_NAMESPACE

constexpr auto WIFI_SSID = "McDonalds Free WiFi 2.4GHz";
constexpr auto WIFI_PASSWD = "Passwort_123";

LedController ledController;

constexpr auto pin = 0;
constexpr auto NUM_LEDS = 100;

std::array<CRGB, NUM_LEDS> leds;

ESP8266WebServer server(80);

uint8_t gHue = 0;

bool rotatePattern = true;

void addGlitter(fract8 chanceOfGlitter)
{
    if(random8() < chanceOfGlitter)
        leds[random16(leds.size())] += CRGB::White;
}

const std::array<std::function<void()>, 7> patterns {
    []()
    {
        fill_rainbow(&leds[0], leds.size(), gHue, 7);
    },
    []()
    {
        fill_rainbow(&leds[0], leds.size(), gHue, 7);
        addGlitter(80);
    },
    []()
    {
        fadeToBlackBy(&leds[0], leds.size(), 10);
        int pos = random16(leds.size());
        leds[pos] += CHSV(gHue + random8(64), 200, 255);
    },
    []()
    {
        fadeToBlackBy(&leds[0], leds.size(), 20);
        int pos = beatsin16(13, 0, leds.size() - 1);
        leds[pos] += CHSV(gHue, 255, 192);
    },
    []()
    {
        fadeToBlackBy(&leds[0], leds.size(), 20);
        byte dothue = 0;
        for(int i = 0; i < 8; i++) {
            leds[beatsin16(i+7, 0, leds.size() - 1)] |= CHSV(dothue, 200, 255);
            dothue += 32;
        }
    },
    []()
    {
        uint8_t BeatsPerMinute = 62;
        CRGBPalette16 palette = PartyColors_p;
        uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
        for(int i = 0; i < leds.size(); i++)
            leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
    },
    []()
    {
      static constexpr auto COOLING = 55;
      static constexpr auto SPARKING = 120;
      static constexpr auto gReverseDirection = false;

      // Array of temperature readings at each simulation cell
      static std::array<byte, NUM_LEDS> heat;

      // Step 1.  Cool down every cell a little
        for( int i = 0; i < leds.size(); i++) {
          heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / leds.size()) + 2));
        }

        // Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for( int k= leds.size() - 1; k >= 2; k--) {
          heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
        }

        // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
        if( random8() < SPARKING ) {
          int y = random8(7);
          heat[y] = qadd8( heat[y], random8(160,255) );
        }

        // Step 4.  Map from heat cells to LED colors
        for( int j = 0; j < leds.size(); j++) {
          CRGB color = HeatColor( heat[j]);
          int pixelnumber;
          if( gReverseDirection ) {
            pixelnumber = (leds.size()-1) - j;
          } else {
            pixelnumber = j;
          }
          leds[pixelnumber] = color;
        }
    }
};

auto iter = patterns.begin();

void nextPattern()
{
    iter++;
    if (iter == patterns.end())
        iter = patterns.begin();
}

void index_handler()
{
    String str = "<html>"
                 "<head>"
                 "</head>"
                 "<body>"
                 "<h1>Total Entchen-Control 1.0</h1>"
                 "<p>"
                 "<a href=\"cmd?var=nextPattern&val=\">Next pattern</a> ";
    if (rotatePattern)
        str += "<a href=\"cmd?var=rotatePattern&val=false\">Disable pattern rotate</a>";
    else
        str += "<a href=\"cmd?var=rotatePattern&val=true\">Enable pattern rotate</a> ";

    const auto index = std::distance(patterns.begin(), iter);

    str += "</p>"
           "<form action=\"cmd\" method=\"GET\">"
           "<input type=\"hidden\" name=\"var\" value=\"setPattern\" />"
           "<select name=\"val\" size=\"7\">"
           "<option value=\"0\"";
    if (index == 0)
        str += " selected";
    str += ">rainbow</option>"
           "<option value=\"1\"";
    if (index == 1)
        str += " selected";
    str += ">rainbowWithGlitter</option>"
           "<option value=\"2\"";
    if (index == 2)
        str += " selected";
    str += ">confetti</option>"
           "<option value=\"3\"";
    if (index == 3)
        str += " selected";
    str += ">sinelon</option>"
           "<option value=\"4\"";
    if (index == 4)
        str += " selected";
    str += ">juggle</option>"
           "<option value=\"5\"";
    if (index == 5)
        str += " selected";
    str += ">bpm</option>"
           "<option value=\"6\"";
    if (index == 6)
        str += " selected";
    str += ">Fire2012</option>"
           "</select>"
           "<button type=\"submit\">submit</button>"
           "</form>"
           "</body>"
           "</html>";

    server.sendHeader("Connection", "close");
    server.send(200, "text/html", str.c_str());
}

void cmd_handler()
{
    server.sendHeader("Connection", "close");

    const String *var = nullptr;
    const String *val = nullptr;

    for (int i = 0; i < server.args(); i++)
    {
        if (server.argName(i) == "var")
            var = &server.arg(i);
        else if (server.argName(i) == "val")
            val = &server.arg(i);
    }

    if (var == nullptr || val == nullptr)
        server.send(400, "text/html", "var or val missing");

    if (*var == "nextPattern")
    {
        nextPattern();
        server.send(200, "text/html", "ok");
        return;
    }
    else if (*var == "setPattern")
    {
        const auto index = atoi(val->c_str());
        if (index < 0 || index >= patterns.size())
        {
            server.send(500, "text/html", "out of range");
            return;
        }

        iter = patterns.begin() + index;

        server.send(200, "text/html", "ok");
        return;
    }
    else if (*var == "rotatePattern")
    {
        rotatePattern = *val == "true";
        server.send(200, "text/html", "ok");
        return;
    }

    server.send(500, "text/html", "unknown var");
}

void setup()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);

    FastLED.addLeds<WS2812, pin, RGB>(&leds[0], leds.size()).setCorrection(TypicalLEDStrip);

    server.on("/", HTTP_GET, index_handler);
    server.on("/cmd", HTTP_GET, cmd_handler);
    server.begin();
}
  
void loop()
{
    EVERY_N_MILLISECONDS(20)
    {
        (*iter)();
        FastLED.show();
    }

    if (rotatePattern)
    {
        EVERY_N_SECONDS(10)
        {
            nextPattern();
        }
    }

    server.handleClient();
}

