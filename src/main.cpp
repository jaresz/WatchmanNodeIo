#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>

#define BUTTON_PIN  0
#define LED_PIN 15
#define LIGHT_OFF 0
#define LIGHT_TURNING_ON 1
#define LIGHT_ON 2
#define LIGHT_TURNING_OFF 3
#define NUMBER_OF_WIFI_NETWORKS 2

#include "wifi-config.h"


WiFiServer server(80);

int buttonState = HIGH;
int lastButtonState = HIGH;
int ledState = LOW;
int ledStripPin[2] = { 14, 5 };
int ledStripNumpixels[2] = { 5, 5 };
int lightState[2] = { LIGHT_OFF, LIGHT_OFF }; // 0 - off, 1 - turning on, 2 - on, 3 - turning off
int pirPin[2] = { 16, 4 };
int selectedPixelNumber[2] = { 0, 0 };

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50; // the debounce time; increase if the output flickers
unsigned long lastPir1Time = 0;
unsigned long lastMakeLight = 0;

Adafruit_NeoPixel pixel[2] =
		{ Adafruit_NeoPixel(ledStripNumpixels[0], ledStripPin[0]),
				Adafruit_NeoPixel(ledStripNumpixels[1], ledStripPin[1]) };

void makeLight() {
	if (millis() - lastMakeLight < 100)
		return;
	lastMakeLight = millis();
	for (int lightNumber = 0; lightNumber < 2; lightNumber++) {

		if (LIGHT_TURNING_ON == lightState[lightNumber]) {
			selectedPixelNumber[lightNumber]++;
			if (selectedPixelNumber[lightNumber]
					> ledStripNumpixels[lightNumber]) {
				lightState[lightNumber] = LIGHT_ON;
				selectedPixelNumber[lightNumber] = 0;
			} else {
				pixel[lightNumber].setPixelColor(
						selectedPixelNumber[lightNumber],
						pixel[lightNumber].Color(0, 0, 0));

				pixel[lightNumber].setPixelColor(
						selectedPixelNumber[lightNumber],
						pixel[lightNumber].Color(200, 255, 31));
				pixel[lightNumber].show();
			}
		} else if (LIGHT_TURNING_OFF == lightState[lightNumber]) {
			selectedPixelNumber[lightNumber]++;
			if (selectedPixelNumber[lightNumber]
					> ledStripNumpixels[lightNumber]) {
				lightState[lightNumber] = LIGHT_OFF;
				selectedPixelNumber[lightNumber] = 0;
			} else {
				pixel[lightNumber].setPixelColor(
						selectedPixelNumber[lightNumber],
						pixel[lightNumber].Color(0, 0, 0));
				pixel[lightNumber].show();
			}
		}

	}
}

void handleOTA() {
	// Port defaults to 8266
	// ArduinoOTA.setPort(8266);

	// Hostname defaults to esp8266-[ChipID]
	ArduinoOTA.setHostname("Rovero");

	// No authentication by default
	// ArduinoOTA.setPassword((const char *)"123");

	ArduinoOTA.onStart([]() {
		Serial.println("Start");
	});
	ArduinoOTA.onEnd([]() {
		Serial.println("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
		else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
		else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
		else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
		else if (error == OTA_END_ERROR) Serial.println("End Failed");
	});
	ArduinoOTA.begin();
}

void connectWifi() {
	WiFi.mode(WIFI_STA);
	uint8_t wifiStatus;
	int wifiNetNumber = 0;
	do {
		Serial.print("WiFi Connecting to ");
		Serial.print(wifiNetNumber);
		Serial.print(". ");
		Serial.print(wifiSsids[wifiNetNumber]);
		WiFi.begin(wifiSsids[wifiNetNumber], wifiPasswords[wifiNetNumber]);
		Serial.print('.');
		pixel[0].setPixelColor(0, pixel[0].Color(31, 0, 0));
		pixel[0].show();
		Serial.print('.');
		wifiNetNumber++;
		Serial.print('.');

		wifiStatus = WiFi.waitForConnectResult();
		Serial.print("wifiStatus: ");
		Serial.println(wifiStatus);
		if (wifiStatus
				!= WL_CONNECTED&& wifiNetNumber >= NUMBER_OF_WIFI_NETWORKS) {
			Serial.print("wifiNetNumber: ");
			Serial.print(wifiNetNumber);
			Serial.println(" Connection Failed! Rebooting...");
			ESP.restart();
		}
	} while (wifiStatus != WL_CONNECTED);
	Serial.println(" Connected! ");

	pixel[0].setPixelColor(0, pixel[0].Color(0, 0, 127));
	pixel[0].show();
	delay(100);
	pixel[0].setPixelColor(0, pixel[0].Color(0, 0, 31));
	pixel[0].show();
	handleOTA();
	Serial.println("Ready");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
}


void setup() {
	Serial.begin(115200);

	pixel[0].begin(); // This initializes the NeoPixel library.
	pixel[0].clear();

	lastDebounceTime = millis();
	Serial.println("Booting");

	pinMode(BUTTON_PIN, INPUT);
	pinMode(pirPin[0], INPUT);
	pinMode(pirPin[1], INPUT);
	digitalWrite(pirPin[0], LOW);
	digitalWrite(pirPin[1], LOW);
	pinMode(LED_PIN, OUTPUT);

	connectWifi();
	// Start the server
	server.begin();
	Serial.println("Server started");

	// Print the IP address
	Serial.print("Use this URL to connect: ");
	Serial.print("http://");
	Serial.print(WiFi.localIP());
	Serial.println("/");

	pinMode(ledStripPin[0], OUTPUT);
	pinMode(ledStripPin[1], OUTPUT);

	delay(2);

	digitalWrite(LED_PIN, HIGH);

	for (int i = 1; i < ledStripNumpixels[0]; i++) {

		Serial.print(i);
		Serial.println("-setPixelColor");
		//if (i>0) pixel[0].setPixelColor(i-1, pixel[0].Color(0,0,0));
		pixel[0].setPixelColor(i, pixel[0].Color(1, 0, 0));
		pixel[0].show();
		delay(100);
		pixel[0].setPixelColor(i, pixel[0].Color(1, 0, 1));
		pixel[0].show();
		delay(100);
		pixel[0].setPixelColor(i, pixel[0].Color(1, 0, 2));
		pixel[0].show(); // This sends the updated pixel color to the hardware.
		delay(100);
		pixel[0].setPixelColor(i, pixel[0].Color(0, 0, 0));
	}
	delay(100);
	pixel[0].show();
	Serial.println("pixend");
	digitalWrite(LED_PIN, LOW);
}

void srvPage(WiFiClient client, int lightState[2], int ledState) {
	// Return the response
	client.println("HTTP/1.1 200 OK");
	client.println("Content-Type: text/html");
	client.println(""); //  do not forget this one
	client.println("<!DOCTYPE HTML>");
	client.println("<html>");
	client.println("<head>");
	client.print("<title>");
	client.print("Watchman ");
	for (int nr = 0; nr < 2; nr++) {
		if (nr > 0)
			client.print(",");

		client.print(nr);
		client.print(":");
		client.print(lightState[nr]);
	}
	client.print("</title>");
	client.println(
			"<meta name=\"viewport\" content=\"width=device-width, user-scalable=no\" />");
	//client.println("<meta http-equiv=\"refresh\" content=\"5\">");
	client.print("<link rel='stylesheet' ");
	client.print(
			"href='https://stackpath.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css' ");
	client.print(
			"integrity='sha384-Vkoo8x4CGsO3+Hhxv8T/Q5PaXtkKtu6ug5TOeNV6gBiFeWPGFN9MuhOf23Q9Ifjh' ");
	client.print(" crossorigin='anonymous'>");
	client.println("<style> body {  width: 100%;   } </style>");
	client.println("</head>");
	client.println("<body>");
	for (int nr = 0; nr < 2; nr++) {
		client.print(nr);
		client.print(" lightState = ");
		if (LIGHT_OFF == lightState[nr]) {
			client.print(" OFF ");
		} else if (LIGHT_TURNING_ON == lightState[nr]) {
			client.print(" LIGHT_TURNING_ON ");
		} else if (LIGHT_ON == lightState[nr]) {
			client.print(" LIGHT_ON ");
		} else if (LIGHT_TURNING_OFF == lightState[nr]) {
			client.print(" LIGHT_TURNING_OFF ");
		}

		client.println("<br>");
	}
	client.println(
			"<div style=\"text-align:right; background-color: #ffffA0;\">sek:");
	client.println(millis() / 1000);
	client.println("</div>");
	client.println("<br><br>");
	client.println("<div class=\"nocontainer\">");
	client.println("<div class=\"row\">");
	client.println(" <div class=\"col-md-6\">");
	client.println(
			"<a href=\"/LED=ON\"><button class=\"btn btn-success btn-block\">Turn On </button></a>");
	client.println(" </div>");
	client.println(" <div class=\"col-md-6\">");
	client.println(
			"<a href=\"/LED=OFF\"\"><button class=\"btn btn-danger btn-block\">Turn Off </button></a><br />");
	client.println(" </div>");
	client.println("</div>");
	client.println("</body>");
	client.println("</html>");
}

void srvStatus(WiFiClient client, int lightState[2], int ledState) {
	// Return the response
	client.println("HTTP/1.1 200 OK");
	client.println("Content-Type: application/json");
	client.println("");

	client.println("{");
	client.println("name:\"state\"");
	for (int nr = 0; nr < 2; nr++) {
		client.print(nr);
		client.print(":");
		client.print(lightState[nr]);
		client.println(",");
	}
	client.println("}");
}


void loop() {

	for (int lightNumber = 0; lightNumber < 2; lightNumber++) {
		if (HIGH == digitalRead(pirPin[lightNumber])) {
			if (millis() - lastPir1Time > 3000 && millis() > 1000) {
				Serial.print(millis());
				Serial.print(" pirPin[ ");
				Serial.print(lightNumber);
				Serial.print("] ON ");
				Serial.println(digitalRead(pirPin[0]));
				lastPir1Time = millis();
				lightState[lightNumber] = LIGHT_TURNING_ON;
			}
		} else if (LOW == digitalRead(pirPin[0])) {
			if (millis() - lastPir1Time > 8000) {
				Serial.print(millis());
				Serial.print(" pirPin[ ");
				Serial.print(lightNumber);
				Serial.println("] OFF ");
				if (lightState[lightNumber] != LIGHT_TURNING_OFF
						&& lightState[lightNumber] != LIGHT_OFF) {
					lastPir1Time = millis();
					lightState[lightNumber] = LIGHT_TURNING_OFF;
				}
			}
		}
	}
	makeLight();

	int reading = digitalRead(BUTTON_PIN);
	if (reading != lastButtonState) {
		// reset the debouncing timer

		lastDebounceTime = millis();
	}

	if ((millis() - lastDebounceTime) > debounceDelay) {
		// whatever the reading is at, it's been there for longer than the debounce
		// delay, so take it as the actual current state:

		// if the button state has changed:
		if (reading != buttonState) {
			buttonState = reading;
			Serial.print("Kontaktron");
			Serial.println(reading);
			// only toggle the LED if the new button state is HIGH
			if (buttonState == HIGH) {
				ledState = !ledState;
			}
		}
	}

	// set the LED:
	digitalWrite(LED_PIN, ledState);

	// save the reading. Next time through the loop, it'll be the lastButtonState:
	lastButtonState = reading;

	ArduinoOTA.handle();
	// Check if a client has connected

	WiFiClient client = server.available();
	if (!client) {
		return;
	}

	// Wait until the client sends some data
	Serial.println("new client");
	while (!client.available()) {
		delay(1);
	}

	// Read the first line of the request
	String request = client.readStringUntil('\r');
	Serial.println(request);
	client.flush();

	// Match the request

	if (request.indexOf("/LED=ON") != -1) {
		digitalWrite(LED_PIN, HIGH);
		for (int i = 1; i < ledStripNumpixels[0]; i++) {
			pixel[0].setPixelColor(i, pixel[0].Color(255, 0, 0));
		}
		pixel[0].show();

	}
	if (request.indexOf("/LED=OFF") != -1) {
		digitalWrite(LED_PIN, LOW);
		for (int i = 1; i < ledStripNumpixels[0]; i++) {
			pixel[0].setPixelColor(i, pixel[0].Color(0, 0, 0));
		}
		pixel[0].show();
	}

	// Set LED_PIN according to the request
	digitalWrite(LED_PIN, ledState);

	if (request.indexOf("/status") != -1 || request.indexOf(".json") != -1) {
		// Return the response
		srvStatus(client, lightState, ledState);
	} else {
		srvPage(client, lightState, ledState);
	}
	delay(1);
	Serial.println("Client disonnected");
	Serial.println("");

}