#pragma once

const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>yt-hush | data collection</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta charset="UTF-8">
    <style>
        body {
            font-family: Arial, sans-serif;
            padding: 20px;
            margin: 0;
            background: #f5f5f5;
        }

        #status, #countdown{
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

        #status:empty, #countdown:empty{
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
        
        .remote {
            max-width: 320px;
            margin: 20px auto;
        }

        .nav-grid {
            display: grid;
            grid-template-columns: repeat(3, 1fr);
            gap: 10px;
        }

        .nav-btn {
            min-height: 70px;
            font-size: 1.5rem;
            font-weight: bold;
            border: none;
            border-radius: 12px;
            background: #666;
            color: white;
        }

        .ok-btn {
            background: #28a745;
        }

        .mute-btn {
            background: #ff9800;
            margin-top: 20px;
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

    <div id="countdown"></div>

    <div class="remote">
        <div class="nav-grid">
            <div></div>
            <button class="nav-btn" onclick="simpleCall('/up')">▲</button>
            <div></div>

            <button class="nav-btn" onclick="simpleCall('/left')">◀</button>
            <button class="nav-btn ok-btn" onclick="simpleCall('/select')">🔘</button>
            <button class="nav-btn" onclick="simpleCall('/right')">▶</button>

            <div></div>
            <button class="nav-btn" onclick="simpleCall('/down')">▼</button>
            <div></div>
        </div>

        <button class="nav-btn mute-btn" onclick="simpleCall('/mute')">🔇</button>
    </div>

<script>
let countdownTimer = null;

async function call(endpoint) {
    try {
        const res = await fetch(endpoint);
        const data = await res.json();

        if (endpoint === "/capture") {
            showCountdown(42);
        } else {
            showMessage(data.message || "OK");

            if (endpoint === "/stop") {
                clearCountdown();
            }
        }
    } catch (e) {
        showMessage("Request failed");
    }
}

async function simpleCall(endpoint) {
    try {
        await fetch(endpoint);
    } catch (e) {
        console.error("Request failed", e);
    }
}

function showMessage(msg) {
    const el = document.getElementById("status");
    el.textContent = msg;
}

function clearCountdown() {
    if (countdownTimer) {
        clearInterval(countdownTimer);
        countdownTimer = null;
    }

    document.getElementById("countdown").textContent = "";
}

function showCountdown(count) {
    clearCountdown();

    const el = document.getElementById("countdown");
    el.textContent = count;

    countdownTimer = setInterval(() => {
        count--;

        if (count <= 0) {
            clearCountdown();
        } else {
            el.textContent = count;
        }
    }, 1000);
}
</script>

</body>
</html>
)rawliteral";