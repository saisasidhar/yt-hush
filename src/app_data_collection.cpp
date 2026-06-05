#ifdef APP_DATA_COLLECTION

#include <Arduino.h>
#include <WiFi.h>
#include <Seeed_Arduino_SSCMA.h>
#include <WebServer.h>
#include <string>

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

bool app_running = false;
struct BurstCaptureState
{
    bool active = false;
    uint32_t end_time_ms = 0;
};
BurstCaptureState burst;

void index_handler()
{
    const char* html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>yt-hush | data collection</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            padding: 20px;
            margin: 0;
            background: #f5f5f5;
        }

        #status {
        min-height: 60px;
        margin-bottom: 20px;
        padding: 12px;

        font-size: 1.4rem;
        font-weight: bold;
        text-align: center;

        color: #ffffff;
        background: #444;
        border-radius: 12px;
        }

        #status:empty {
            background: transparent;
            padding: 12px;
        }

        button {
            display: block;
            width: 100%;
            min-height: 70px;
            margin-bottom: 20px;
            font-size: 1.5rem;
            font-weight: bold;
            border: none;
            border-radius: 12px;
            cursor: pointer;
        }

        .capture {
            background: #007bff;
            color: white;
        }

        .start {
            background: #28a745;
            color: white;
        }

        .stop {
            background: #dc3545;
            color: white;
        }
    </style>
</head>

<body>
    <div id="status"></div>

    <button class="start" onclick="call('/start')">
        Start Acquisition
    </button>

    <button class="stop" onclick="call('/stop')">
        Stop Acquisition
    </button>

    <button class="capture" onclick="call('/capture')">
        Burst Capture
    </button>

<script>
async function call(endpoint) {
    try {
        const res = await fetch(endpoint);
        const data = await res.json();

        showMessage(data.message || "OK");
    } catch (e) {
        showMessage("Request failed");
    }
}

function showMessage(msg) {
    const el = document.getElementById("status");
    el.textContent = msg;

    setTimeout(() => {
        el.textContent = "";
    }, 3000);
}
</script>

</body>
</html>
)rawliteral";

    server.send(200, "text/html", html);
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
    static int led_blink_ms = 250;
    if (app_running) {
        uint32_t now = millis();
        if (burst.active) {
            delay(1000-led_blink_ms);
            if (now > burst.end_time_ms)
                burst.active = false;
        } else {
            delay(5000-led_blink_ms);
        }
        digitalWrite(LED_BUILTIN, LOW);
        if (!AI.invoke(1, false, true)){
            log_i("[%s]Captured Image %d\n", burst.active ? "*": " ", ++count);
        }
        delay(led_blink_ms);
        digitalWrite(LED_BUILTIN, HIGH);
    }
    
}

#endif