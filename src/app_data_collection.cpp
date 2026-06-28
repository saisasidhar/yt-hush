#ifdef APP_DATA_COLLECTION

#include <Arduino.h>
#include <WiFi.h>
#include <Seeed_Arduino_SSCMA.h>
#include <WebServer.h>
#include <string>

#include "USB.h"
#include "USBHIDKeyboard.h"

#include "index_html.h"

#define DEBUG_PORT Serial
static const char* TAG = "yt-hush";

#ifdef ESP32
#include <HardwareSerial.h>
HardwareSerial atSerial(0);
#else
#define atSerial Serial1
#endif

SSCMA AI;

WebServer server(80);

USBHIDKeyboard Keyboard;
#define TV_SELECT KEY_RETURN
#define TV_UP     KEY_UP_ARROW
#define TV_DOWN   KEY_DOWN_ARROW
#define TV_LEFT   KEY_LEFT_ARROW
#define TV_RIGHT  KEY_RIGHT_ARROW
#define TV_MUTE   KEY_F8

bool app_running = false;
struct BurstCaptureState
{
    bool active = false;
    uint32_t end_time_ms = 0;
};
BurstCaptureState burst;

void index_handler()
{
    server.send(200, "text/html", index_html);
}

void capture_handler()
{
    if (burst.active)
    {
        log_i("Burst capture already active. Ignoring");
        server.send(200, "application/json", "{\"message\":\"Capture already active\"}");
        return;
    }

    uint32_t now = millis();
    burst.active = true;
    burst.end_time_ms = now + 42000;

    server.send(200, "application/json", "{\"message\":\"Ok sir, clicking away...\"}");
}

void start_handler()
{
    app_running = true;
    server.send(200, "application/json", "{\"message\":\"Acquisition started\"}");
}

void stop_handler()
{
    app_running = false;
    server.send(200, "application/json", "{\"message\":\"Acquisition stopped\"}");
}

void app_setup()
{
    Serial.begin(115200);
    delay(5000);
    Serial.setDebugOutput(true);
    USB.begin();
    Keyboard.begin();
    log_i("USB HID Keyboard initialized");

    log_i("*** yt-hush ***");
    log_i("Data Collection Firmware");

    if(strcmp(WIFI_SSID, "NOT_SET") == 0 || strcmp(WIFI_PASS, "NOT_SET") == 0) {
        log_w("WiFi credentials are not configured properly. Not proceeding");
        while(1) { delay(1000); } 
    }
    log_i("Connecting to \'%s\' with \'%s\'\n", WIFI_SSID, WIFI_PASS);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    log_i("WiFi connected");
    log_i("IP Address: %s", WiFi.localIP().toString().c_str());

    server.on("/", HTTP_GET, index_handler);
    server.on("/capture", HTTP_GET, capture_handler);
    server.on("/start", HTTP_GET, start_handler);
    server.on("/stop", HTTP_GET, stop_handler);
    server.on("/up", HTTP_GET, []() {
        log_i("Sending UP key");
        Keyboard.press(TV_UP);
        Keyboard.releaseAll();
        server.send(200, "application/json", "{\"message\":\"OK\"}");
    });
    server.on("/down", HTTP_GET, []() {
        log_i("Sending DOWN key");
        Keyboard.press(TV_DOWN);
        Keyboard.releaseAll();
        server.send(200, "application/json", "{\"message\":\"OK\"}");
    });
    server.on("/right", HTTP_GET, []() {
        log_i("Sending RIGHT key");
        Keyboard.press(TV_RIGHT);
        Keyboard.releaseAll();
        server.send(200, "application/json", "{\"message\":\"OK\"}");
    });
    server.on("/left", HTTP_GET, []() {
        log_i("Sending LEFT key");
        Keyboard.press(TV_LEFT);
        Keyboard.releaseAll();
        server.send(200, "application/json", "{\"message\":\"OK\"}");
    });
    server.on("/select", HTTP_GET, []() {
        log_i("Sending SELECT key");
        Keyboard.press(TV_SELECT);
        Keyboard.releaseAll();
        server.send(200, "application/json", "{\"message\":\"OK\"}");
    });
    server.on("/mute", HTTP_GET, []() {
        log_i("Sending MUTE key");
        Keyboard.press(TV_MUTE);
        Keyboard.releaseAll();
        server.send(200, "application/json", "{\"message\":\"OK\"}");
    });
    server.begin();
    log_i("Web server started");

    AI.begin(&atSerial);
    log_i("Connected to Grove AI Vision 2");
    AI.save_jpeg();
    log_i("Configured to save JPEG on invoke");

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    log_i("Setup Complete");
}

void app_loop()
{
    server.handleClient();
    static int count = 0;
    static uint32_t next_capture_ms = 0;
    if (app_running) {
        uint32_t now = millis();
        if (now > next_capture_ms) {
            digitalWrite(LED_BUILTIN, LOW);
            if (!AI.invoke(1, false, true)){
                log_i("[%s]Captured Image %d at %d\n", burst.active ? "*": " ", ++count, now);
            }
            log_i("[%s]Captured Image %d at %d\n", burst.active ? "*": " ", ++count, now);
            digitalWrite(LED_BUILTIN, HIGH);
            if (burst.active) {
                next_capture_ms = now + (1000);
                if (now > burst.end_time_ms)
                    burst.active = false;
            } else {
                next_capture_ms = now + (5000);
            }
        }
    }
    delay(50);
}

#endif