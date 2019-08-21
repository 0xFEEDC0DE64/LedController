#include <array>
#include <functional>

#include <Arduino.h>

constexpr bool useRemoteServer = false;

#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
#elif ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#else
#error "unknown board"
#endif

#include "ledcontroller.h"
#include "controlclient.h"

constexpr auto WIFI_SSID = "Camp2019-things";
constexpr auto WIFI_PASSWD = "camp2019";

LedController ledController;

#ifdef ESP32
WebServer server(80);
#elif ESP8266
ESP8266WebServer server(80);
#else
#error "unknown board"
#endif

bool power = true;
bool rotatePattern = true;

ControlClient client("daniel_ledstrip", power);

void setup()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    WiFi.begin(WIFI_SSID, WIFI_PASSWD);

    server.on("/", HTTP_GET, []()
    {
        server.sendHeader("Connection", "close");

        server.send(200, "text/html",
                    "<!doctype html>"
                    "<html lang=\"en\">"
                        "<head>"
                            "<!-- Required meta tags -->"
                            "<meta charset=\"utf-8\">"
                            "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">"

                            "<!-- Bootstrap CSS -->"
                            "<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\" integrity=\"sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T\" crossorigin=\"anonymous\">"

                            "<style>"
                                ".hidden {"
                                    "display: none;"
                                "}"
                            "</style>"

                            "<title>LED-Control</title>"
                        "</head>"
                        "<body>"
                            "<div class=\"container\">"
                                "<h1>LED-Control</h1>"
                                "<p>"
                                    "<a href=\"setPower?val=true\" id=\"enablePower\" class=\"softLink\">Turn LEDs on</a> "
                                    "<a href=\"setPower?val=false\" id=\"disablePower\" class=\"softLink\">Turn LEDs off</a> "
                                    "<a href=\"nextPattern\" id=\"nextPattern\" class=\"softLink\">Next pattern</a> "
                                    "<a href=\"setPatternRotate?val=true\" id=\"enableRotate\" class=\"softLink\">Enable automatic pattern rotate</a> "
                                    "<a href=\"setPatternRotate?val=false\" id=\"disableRotate\" class=\"softLink\">Disable automatic pattern rotate</a>"
                                "</p>"

                                "<select id=\"patternSelect\"></select>"

                                "<input type=\"range\" min=\"0\" max=\"255\" id=\"brightness\" />"
                            "</div>"

                            "<script src=\"https://code.jquery.com/jquery-3.3.1.slim.min.js\" integrity=\"sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo\" crossorigin=\"anonymous\"></script>"
                            "<script>"
                                "jQuery(document).ready(function($){"
                                    "$('.softLink').click(function(){"
                                        "var xhr = new XMLHttpRequest();"
                                        "xhr.open('GET', this.getAttribute('href'));"
                                        "xhr.send();"

                                        "return false;"
                                    "});"

                                    "$('#patternSelect').change(function(){"
                                        "var xhr = new XMLHttpRequest();"
                                        "xhr.open('GET', 'setPattern?val=' + this.selectedIndex);"
                                        "xhr.send();"
                                    "});"

                                    "$('#brightness').change(function(){"
                                        "var xhr = new XMLHttpRequest();"
                                        "xhr.open('GET', 'setBrightness?val=' + this.value);"
                                        "xhr.send();"
                                    "});"

                                    "var refreshStatus = function() {"
                                        "var xhr = new XMLHttpRequest();"
                                        "xhr.open('GET', 'status');"
                                        "xhr.onload = function() {"
                                            "var parsed = JSON.parse(xhr.responseText);"

                                            "function setHidden(element, hidden) {"
                                                "if (hidden) {"
                                                    "element.addClass('hidden');"
                                                "} else {"
                                                    "element.removeClass('hidden');"
                                                "}"
                                            "}"

                                            "setHidden($('#enablePower'), parsed.power);"
                                            "setHidden($('#disablePower'), !parsed.power);"
                                            "setHidden($('#nextPattern'), !parsed.power);"
                                            "setHidden($('#enableRotate'), !parsed.power || parsed.autoRotate);"
                                            "setHidden($('#disableRotate'), !parsed.power || !parsed.autoRotate);"
                                            "setHidden($('#patternSelect'), !parsed.power);"
                                            "setHidden($('#brightness'), !parsed.power);"

                                            "var select = $('#patternSelect');"
                                            "select.empty();"
                                            "select.attr('size', parsed.patterns.length);"

                                            "for (var pattern of parsed.patterns) {"
                                                "select.append($('<option>')"
                                                    ".text(pattern.name)"
                                                    ".attr('selected', pattern.selected)"
                                                ");"
                                            "}"

                                            "$('#brightness').val(parsed.brightness)"
                                        "};"
                                        "xhr.send();"
                                    "};"

                                    "refreshStatus();"

                                    "setInterval(refreshStatus, 1000);"
                                "});"
                            "</script>"
                        "</body>"
                    "</html>"
                    );
    });

#if ESP8266
    server.on("/update", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html",
                  "<form method=\"POST\" enctype=\"multipart/form-data\">"
                      "<input type=\"file\" name=\"update\" />"
                      "<button type=\"submit\">Install</button>"
                  "</form>");
    });

    server.on("/update", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }, []() {
        HTTPUpload& upload = server.upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Update: %s\n", upload.filename.c_str());

            uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            if (!Update.begin(maxSketchSpace)) { //start with max available size
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
        yield();
    });
#endif

    server.on("/reboot", HTTP_GET, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/html",
                  "<form method=\"POST\">"
                      "<button type=\"submit\">Perform reboot</button>"
                  "</form>");
    });

    server.on("/reboot", HTTP_POST, []() {
        server.sendHeader("Connection", "close");
        server.send(200, "text/plain", "OK");
        ESP.restart();
    });

    server.on("/status", HTTP_GET, []()
    {
        server.sendHeader("Connection", "close");
        server.sendHeader("Access-Control-Allow-Origin", "*");

        String str = "{";

        str += "\"patterns\":[";
        bool first = true;
        for (auto pattern : ledController.patterns)
        {
            if (first)
                first = false;
            else
                str += ',';

            str += "{\"name\":\"";
            str += pattern->name();
            str += "\",\"selected\":";
            str += (*ledController.iter) == pattern ? "true" : "false";
            str += '}';
        }
        str += "],";

        str += "\"power\":";
        str += power ? "true" : "false";
        str += ",";

        str += "\"autoRotate\":";
        str += rotatePattern ? "true" : "false";
        str += ",";

        str += "\"brightness\":";
        str += String(int(FastLED.getBrightness()));

        str += "}";

        server.send(200, "text/json", str.c_str());
    });

    server.on("/nextPattern", HTTP_GET, []()
    {
        ledController.nextPattern();
        server.send(200, "text/html", "ok");
    });

    server.on("/setPattern", HTTP_GET, []()
    {
        for (int i = 0; i < server.args(); i++)
            if (server.argName(i) == "val")
            {
                const auto val = server.arg(i);

                const auto index = atoi(val.c_str());
                if (index < 0 || index >= ledController.patterns.size())
                {
                    server.send(400, "text/html", "out of range");
                    return;
                }

                ledController.iter = ledController.patterns.begin() + index;

                server.send(200, "text/html", "ok");

                return;
            }

        server.send(400, "text/html", "val missing");

    });

    server.on("/setPower", HTTP_GET, []()
    {
        for (int i = 0; i < server.args(); i++)
            if (server.argName(i) == "val")
            {
                const auto val = server.arg(i);

                if (val != "true" && val != "false")
                {
                    server.send(400, "text/html", "invalid val");
                    return;
                }

                if (val == "true")
                    client.on();
                else
                    client.off();

                server.send(200, "text/html", "ok");

                return;
            }

        server.send(400, "text/html", "val missing");
    });

    server.on("/setPatternRotate", HTTP_GET, []()
    {
        for (int i = 0; i < server.args(); i++)
            if (server.argName(i) == "val")
            {
                const auto val = server.arg(i);

                if (val != "true" && val != "false")
                {
                    server.send(400, "text/html", "invalid val");
                    return;
                }

                rotatePattern = val == "true";

                server.send(200, "text/html", "ok");
            }

        server.send(400, "text/html", "val missing");
    });

    server.on("/setBrightness", HTTP_GET, []()
    {
        for (int i = 0; i < server.args(); i++)
            if (server.argName(i) == "val")
            {
                const auto val = server.arg(i);

                const auto brightness = atoi(val.c_str());
                if (brightness < 0 || brightness > 255)
                {
                    server.send(400, "text/html", "out of range");
                    return;
                }

                FastLED.setBrightness(brightness);

                server.send(200, "text/html", "ok");

                return;
            }

        server.send(400, "text/html", "val missing");
    });

    server.begin();
}

void loop()
{
    EVERY_N_MILLISECONDS(20)
    {
        if (power)
            ledController.run();
        else
            ledController.poweroff();
        FastLED.show();
    }

    if (power && rotatePattern)
    {
        EVERY_N_SECONDS(10)
        {
            ledController.nextPattern();
        }
    }

    if (useRemoteServer)
        client.handleClient();
    server.handleClient();
}

