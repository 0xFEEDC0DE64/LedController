#pragma once

#include <WiFiClient.h>

#include <lib8tion.h>

class ControlClient {
public:
    ControlClient(const char *name, bool &power) :
        m_name(name),
        m_power(power)
    {}

    void handleClient()
    {
        if(!m_client.connected())
        {
            if (m_client.connect(host, 1234))
            {
                m_client.println(m_name);
                sendStatus();
            }
        }

        if(m_client.connected())
        {
            while(m_client.available())
            {
                char c(m_client.read());

                switch(c) {
                case '1': on(); break;
                case '0': off(); break;
                case 't': toggle(); break;
                case 's': sendStatus(); break;
                case 'r': m_client.println("rebooting"); ESP.restart(); break;
                default: Serial.print("Unknown command: "); Serial.println(c);
                }
            }
        }
    }

    void on() { m_power = true; sendStatus(); }
    void off() { m_power = false; sendStatus(); }
    void toggle() { m_power = !m_power; sendStatus(); }
    bool status() { return m_power; }
    void sendStatus() { m_client.println(status() ? "on" : "off"); }

private:
    static constexpr auto host = "192.168.0.2";

    WiFiClient m_client;
    const char *m_name;
    bool &m_power;
};
