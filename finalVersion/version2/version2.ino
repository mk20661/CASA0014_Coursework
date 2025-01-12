#include <WiFiNINA.h>   
#include <PubSubClient.h>
#include <utility/wifi_drv.h>
#include "arduino_secrets.h" 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>


const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;
int status = WL_IDLE_STATUS;

//configuration LDR
#define LDR A0

//configuration sound level sensor
#define SoundSensorPin A1
#define VREF  5.0
#define SampleSize 10
int Samples[SampleSize];
int sampleIndex = 0;
float soundValue = 0;
int averageValue = 0;

//LED setting
#define NUM_LEDs 12
int numLEDs = 0;

//OLED setting
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//Rotary Encoder Setting
const int A_pin = 2;
const int B_pin = 3;
const int Button_pin = 4;

//menu
int menuIndex = 0;
int colorMenuIndex = 0;          
const char* colorMenu[6] = {"Red", "Green", "Blue", "Yellow", "White","Purple"};
int colorMenuItems = 6;
int selectedColor[3] = {0, 0, 0};  
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 10;
unsigned long buttonLastDebounceTime = 0;
const unsigned long buttonDebounceDelay = 50;
int lastButtonState = HIGH;
int lastA = LOW; 

WiFiServer server(80);
WiFiClient wificlient;

WiFiClient mkrClient;
PubSubClient client(mkrClient);

char mqtt_topic[] = "student/CASA0014/light/7/pixel/";
char mqtt_topic_brightness[] = "student/CASA0014/light/7/brightness/";

void setup() {
  // Start the serial monitor to show output
  Serial.begin(115200);
  delay(1000);

  //oled
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED fail"));
    while (true);
  }
  display.clearDisplay();
  display.display();

  //encoder
  pinMode(A_pin, INPUT);
  pinMode(B_pin, INPUT);
  pinMode(Button_pin, INPUT_PULLUP);

  //wifi
  WiFi.setHostname("Lumina ucjtdjw");
  startWifi();
  client.setServer(mqtt_server, mqtt_port);
  Serial.println("setup complete");
  displayColorMenu();
}

void loop() {
  checkRotation();
  checkButton();
  if (!client.connected()) {
    reconnectMQTT();
  }

  // we also need to make sure we are connected to the wifi
  // otherwise it will be impossible to connect to MQTT!
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }

  // check for messages from the broker and ensuring that any outgoing messages are sent.
  client.loop();

  sendmqtt();
  delay(100);
} 

void sendmqtt(){
  int orginalValue = analogRead(SoundSensorPin) / 1024.0 * VREF;
  soundValue = orginalValue * 50.0;

  Samples[sampleIndex] = soundValue; // store in sample array 
  sampleIndex = (sampleIndex + 1) % SampleSize; // update sample
  for (int i = 0; i < SampleSize; i++) {
    averageValue += Samples[i];
  }
  averageValue /= SampleSize;
  Serial.print("Sound value: ");
  Serial.println(averageValue);
  numLEDs = map(averageValue,30,150,0,NUM_LEDs);
  updateLED(numLEDs);

  int lightLevel = analogRead(LDR);
  Serial.print("Light level: ");
  Serial.println(lightLevel);
  int lightBrightness = map(lightLevel,0,1023,0,119);
  setbrightness(lightBrightness);
}

void startWifi(){
    
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // Function for connecting to a WiFi network
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    Serial.print("Trying to connect to: ");
    Serial.println(ssid);
    for (int i = 0; i < n; ++i){
      String availablessid = WiFi.SSID(i);
      if (availablessid.equals(ssid)) {
        Serial.print("Connecting to ");
        Serial.println(ssid);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
          delay(600);
          Serial.print(".");
        }
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connected to " + String(ssid));
          break;
        } else {
          Serial.println("Failed to connect to " + String(ssid));
        }
      } else {
        Serial.print(availablessid);
        Serial.println(" - this network is not in my list");
      }

    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}

void reconnectMQTT() {
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "LuminaSelector";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void updateLED(int LEDNums){
  char mqtt_message[100];
  for (int i = 0; i < NUM_LEDs; i++) {
    if (i < LEDNums) {
      sprintf(mqtt_message, "{\"pixelid\": %d, \"R\": %d, \"G\": %d, \"B\": %d, \"W\": 0}", i,selectedColor[0],selectedColor[1],selectedColor[2]);
      Serial.println(mqtt_topic);
      Serial.println(mqtt_message);

      if (client.publish(mqtt_topic, mqtt_message, false)) {
        Serial.println("Message published");
      } else {
        Serial.println("Failed to publish message");
      }
    } else {
      sprintf(mqtt_message, "{\"pixelid\": %d, \"R\": 0, \"G\": 0, \"B\": 0, \"W\": 0}", i);
      Serial.println(mqtt_topic);
      Serial.println(mqtt_message);
      if (client.publish(mqtt_topic, mqtt_message, false)) {
        Serial.println("Message published");
      } else {
        Serial.println("Failed to publish message");
      }
    }
  }
}

void displayColorMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Select a Colour");
  for (int i = 0; i < colorMenuItems; i++) {
    display.print(i == colorMenuIndex ? "> " : "  ");
    display.println(colorMenu[i]);
  }
  display.display();
}

void selectColor(int index) {
  switch (index) {
    case 0: selectedColor[0] = 255; selectedColor[1] = 0;   selectedColor[2] = 0; break; // Red
    case 1: selectedColor[0] = 0;   selectedColor[1] = 255; selectedColor[2] = 0; break; // Green
    case 2: selectedColor[0] = 0;   selectedColor[1] = 0;   selectedColor[2] = 255; break; // Blue
    case 3: selectedColor[0] = 255; selectedColor[1] = 255; selectedColor[2] = 0; break; // Yellow
    case 4: selectedColor[0] = 255; selectedColor[1] = 255; selectedColor[2] = 255; break; // White
    case 5: selectedColor[0] = 255; selectedColor[1] = 0; selectedColor[2] = 255; break; // Purple
  }
  displaySelection(colorMenu[index]);
}

void displaySelection(const char* selection) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Selected:");
  display.println(selection);
  display.display();
}

void checkRotation() {
  int currentA = digitalRead(A_pin);
  if (currentA != lastA && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    int currentB = digitalRead(B_pin);
    colorMenuIndex += (currentA == HIGH) ? (currentB == LOW ? 1 : -1) : 0;
    colorMenuIndex = (colorMenuIndex + colorMenuItems) % colorMenuItems;
    displayColorMenu();
  }
  lastA = currentA;
}

void checkButton() {
  int buttonState = digitalRead(Button_pin);
  if (buttonState != lastButtonState && (millis() - buttonLastDebounceTime) > buttonDebounceDelay) {
    buttonLastDebounceTime = millis();
    if (buttonState == LOW) {
      selectColor(colorMenuIndex);
    }
  }
  lastButtonState = buttonState;
}

void setbrightness(int brightness){
  char mqtt_message[100];
  sprintf(mqtt_message, "{\"brightness\": \"%d\"}", brightness);
    Serial.println(mqtt_topic_brightness);
    Serial.println(mqtt_message);
     if (client.publish(mqtt_topic_brightness, mqtt_message)) {
        Serial.println("Message published");
    } else {
        Serial.println("Failed to publish message");
    }
}
