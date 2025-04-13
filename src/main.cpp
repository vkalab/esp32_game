#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

// SSID a heslo pro Access Point
const char* ssid = "game";
const char* password = "game";

const byte DNS_PORT = 53;
DNSServer dnsServer;

// Vytvoření web serveru na portu 80
WebServer server(80);

// HTML stránka s vloženým CSS a JavaScript kódem
// Tato stránka implementuje mobilní hru „klikni na tlačítko“ s 30 sekundovým odpočtem.
// Hráč musí za těchto 30 sekund stihnout alespoň 50 kliknutí. Pokud se mu to podaří, objeví
// se tajný kód. Pokud ne, zobrazí se zpráva "Byli jsi příliš pomalý".
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Tap Game</title>
  <style>
    body {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      height: 100vh;
      margin: 0;
      font-family: Arial, sans-serif;
      background-color: #f9f9f9;
    }
    #gameContainer {
      position: relative;
      width: 90%;
      max-width: 400px;
      height: 70vh;
      border: 2px solid #ccc;
      background-color: #fff;
      border-radius: 10px;
      overflow: hidden;
    }
    #tapButton {
      position: absolute;
      padding: 15px 25px;
      font-size: 18px;
      border: none;
      background-color: #008CBA;
      color: #fff;
      border-radius: 5px;
      cursor: pointer;
    }
    #timer {
      font-size: 24px;
      margin: 20px;
    }
    #secret {
      font-size: 28px;
      margin-top: 20px;
      color: green;
      display: none;
      font-weight: bold;
    }
  </style>
</head>
<body>
  <div id="timer">Time: 30</div>
  <div id="gameContainer">
    <button id="tapButton">Tap me!</button>
  </div>
  <div id="secret"></div>
  <script>
    var timeLeft = 30;
    var clickCount = 0; // Počítadlo kliknutí
    var tapButton = document.getElementById("tapButton");
    var timerDisplay = document.getElementById("timer");
    var secretDisplay = document.getElementById("secret");
    var gameContainer = document.getElementById("gameContainer");
    var timerInterval;

    // Spustí odpočítávání hry
    function startGame() {
      timerInterval = setInterval(function() {
        timeLeft--;
        timerDisplay.textContent = "Time: " + timeLeft;
        if (timeLeft <= 0) {
          clearInterval(timerInterval);
          endGame();
        }
      }, 1000);
    }

    // Funkce ukončující hru: zkontroluje počet kliknutí a zobrazí odpovídající zprávu
    function endGame() {
      tapButton.disabled = true;
      if (clickCount >= 50) {
        secretDisplay.textContent = "Tajný kód: J07ECRF";
      } else {
        secretDisplay.textContent = "Byli jsi příliš pomalý";
      }
      secretDisplay.style.display = "block";
    }

    // Pohne tlačítkem na náhodnou pozici v rámci herního kontejneru
    function moveButton() {
      var containerRect = gameContainer.getBoundingClientRect();
      var btnRect = tapButton.getBoundingClientRect();
      // Zajistí, že tlačítko zůstane celé viditelné uvnitř kontejneru
      var maxX = containerRect.width - btnRect.width;
      var maxY = containerRect.height - btnRect.height;
      var randomX = Math.floor(Math.random() * maxX);
      var randomY = Math.floor(Math.random() * maxY);
      tapButton.style.left = randomX + "px";
      tapButton.style.top = randomY + "px";
    }

    // Při kliknutí: pokud čas ještě běží, inkrementujeme počet kliknutí a přesuneme tlačítko
    tapButton.addEventListener("click", function() {
      if (timeLeft > 0) {
        clickCount++;
        moveButton();
      }
    });

    // Po načtení stránky se hra spustí a tlačítko se náhodně umístí
    window.onload = function() {
      moveButton();
      startGame();
    };
  </script>
</body>
</html>
)rawliteral";

// Funkce, která zpracuje všechny HTTP požadavky odesláním captive portal stránky
void handleRoot() {
  server.send_P(200, "text/html", htmlPage);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Spuštění WiFi access pointu
  Serial.println("Starting Access Point...");
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Spuštění DNS serveru pro přesměrování DNS požadavků
  dnsServer.start(DNS_PORT, "*", myIP);

  // Definice tras pro web server
  server.on("/", handleRoot);
  // Handler pro všechny ostatní URL požadavky
  server.onNotFound(handleRoot);

  // Spuštění web serveru
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Zpracování DNS požadavků (pro captive portal přesměrování)
  dnsServer.processNextRequest();
  // Zpracování příchozích HTTP požadavků
  server.handleClient();
}
