#include <array>
#include <functional>

#include <Arduino.h>

#include <WiFi.h>
#include <esp_http_server.h>

#include <FastLED.h>

FASTLED_USING_NAMESPACE

static constexpr auto NUM_LEDS = 100;

std::array<CRGB, NUM_LEDS> leds;

void addGlitter(fract8 chanceOfGlitter)
{
    if(random8() < chanceOfGlitter)
        leds[random16(leds.size())] += CRGB::White;
}

static uint8_t gHue = 0;

static const std::array<std::function<void()>, 7> patterns {
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

static auto iter = patterns.begin();

static bool rotatePattern = true;

void nextPattern()
{
    iter++;
    if (iter == patterns.end())
        iter = patterns.begin();
}

static esp_err_t index_handler(httpd_req_t *req)
{

    static const char *index1 = "<html>"
                               "<head>"
                               "</head>"
                               "<body>"
                               "<h1>Total Entchen-Control 1.0</h1>"
                               "<p>"
                               "<a href=\"cmd?var=nextPattern&val=\">Next pattern</a> ";

    static const char *turnOn = "<a href=\"cmd?var=rotatePattern&val=true\">Enable pattern rotate</a> ";
    static const char *turnOff = "<a href=\"cmd?var=rotatePattern&val=false\">Disable pattern rotate</a>";

    static const char *index2 = "</p>"
                               "<form action=\"cmd\" method=\"GET\">"
                               "<input type=\"hidden\" name=\"var\" value=\"setPattern\" />"
                               "<select name=\"val\" size=\"7\">"
                               "<option value=\"0\">rainbow</option>"
                               "<option value=\"1\">rainbowWithGlitter</option>"
                               "<option value=\"2\">confetti</option>"
                               "<option value=\"3\">sinelon</option>"
                               "<option value=\"4\">juggle</option>"
                               "<option value=\"5\">bpm</option>"
                               "<option value=\"6\">Fire2012</option>"
                               "</select>"
                               "<button type=\"submit\">submit</button>"
                               "</form>"
                               "</body>"
                               "</html>";

    String str = index1;
    if (rotatePattern)
        str += turnOff;
    else
        str += turnOn;
    str += index2;

    return httpd_resp_send(req, str.c_str(), str.length());
}

static esp_err_t status_handler(httpd_req_t *req)
{
    return httpd_resp_send(req, "statu2", 6);
}

static esp_err_t cmd_handler(httpd_req_t *req)
{
    char*  buf;
    size_t buf_len;
    char variable[32] = {0,};
    char value[32] = {0,};

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = (char*)malloc(buf_len);
        if(!buf){
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) == ESP_OK &&
                httpd_query_key_value(buf, "val", value, sizeof(value)) == ESP_OK) {
            } else {
                free(buf);
                httpd_resp_send_404(req);
                return ESP_FAIL;
            }
        } else {
            free(buf);
            httpd_resp_send_404(req);
            return ESP_FAIL;
        }
        free(buf);
    } else {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    if (!strcmp(variable, "nextPattern"))
    {
        nextPattern();
        return httpd_resp_send(req, "ok1", 3);
    }
    else if (!strcmp(variable, "setPattern"))
    {
        const auto val = atoi(value);
        if (val >= patterns.size())
            return httpd_resp_send_500(req);

        iter = patterns.begin() + val;

        return httpd_resp_send(req, "ok2", 3);
    }
    else if (!strcmp(variable, "rotatePattern"))
    {
        rotatePattern = !strcmp(value, "true");
        return httpd_resp_send(req, "ok3", 3);
    }

    return httpd_resp_send_500(req);
}

void setup()
{
    WiFi.begin("realraum", "r3alraum");

    FastLED.addLeds<WS2811, 16, RGB>(&leds[0], leds.size()).setCorrection(TypicalLEDStrip);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t webserver;

    {
        const esp_err_t error = httpd_start(&webserver, &config);
        if (error != ESP_OK)
        {
            Serial.println("could not start webserver");
            Serial.println(error);
        }
    }

    for (const httpd_uri_t &uri : {
         httpd_uri_t { "/",        HTTP_GET, index_handler,   NULL },
         httpd_uri_t { "/status",  HTTP_GET, status_handler,  NULL },
         httpd_uri_t { "/cmd",     HTTP_GET, cmd_handler,     NULL }
    })
    {
        const esp_err_t error = httpd_register_uri_handler(webserver, &uri);
        if (error != ESP_OK)
        {
            Serial.print("error registering uri ");
            Serial.println(uri.uri);
            Serial.println(error);
        }
    }
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
}

