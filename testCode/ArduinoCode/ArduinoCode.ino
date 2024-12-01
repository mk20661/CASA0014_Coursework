#include <WiFiNINA.h>   
#include <PubSubClient.h>
#include <utility/wifi_drv.h>   // library to drive to RGB LED on the MKR1010
#include "arduino_secrets.h" 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
/*
**** please enter your sensitive data in the Secret tab/arduino_secrets.h
**** using format below

#define SECRET_SSID "ssid name"
#define SECRET_PASS "ssid password"
#define SECRET_MQTTUSER "user name - eg student"
#define SECRET_MQTTPASS "password";
 */
const int A_pin = 2;
const int B_pin = 3;
const int Button_pin = 4;

int menuIndex = 0;                // Current index of the menu
int lastA = LOW;                  // Last state of the A pin
int menuItems = 3;                // Number of menu items
const char* menu[] = {"Single Mode", "Ring Mode", "Select Numbers"}; // Menu options for currencies

  
const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;
int status = WL_IDLE_STATUS;     // the Wifi radio's status
unsigned long lastDebounceTime = 0;          // Debounce timer for rotation
const unsigned long debounceDelay = 10;      // Debounce delay for rotation
unsigned long buttonLastDebounceTime = 0;    // Debounce timer for button
const unsigned long buttonDebounceDelay = 50; // Debounce delay for button

int lastButtonState = HIGH;   

WiFiServer server(80);
WiFiClient wificlient;

WiFiClient mkrClient;
PubSubClient client(mkrClient);


void setup() {
  // Start the serial monitor to show output
  Serial.begin(115200);
  delay(1000);

  // WiFi.setHostname("Lumina ucjtdjw");
  // startWifi();
  // client.setServer(mqtt_server, mqtt_port);
  // Serial.println("setup complete");
}

void loop() {
  checkRotation();  // Check if the rotary encoder was rotated
  checkButton();    // Check if the button was pressed
  // we need to make sure the arduino is still connected to the MQTT broker
  // otherwise we will not receive any messages
  // if (!client.connected()) {
  //   reconnectMQTT();
  // }

  // // we also need to make sure we are connected to the wifi
  // // otherwise it will be impossible to connect to MQTT!
  // if (WiFi.status() != WL_CONNECTED){
  //   startWifi();
  // }

  // check for messages from the broker and ensuring that any outgoing messages are sent.
  // client.loop();
  // clearAllColor();
  Serial.println("sent a message");
  delay(10000);
  
}

// void sendmqtt(char mqtt_topic){

//   // send a message to update the light
//   char mqtt_message[100];
//   sprintf(mqtt_message, "{\"pixelid\": %d, \"R\": 0, \"G\": 255, \"B\": 128, \"W\": 200}", 2);
//   Serial.println(mqtt_topic);
//   Serial.println(mqtt_message);
  

//   if (client.publish(mqtt_topic, mqtt_message)) {
//     Serial.println("Message published");
//   } else {
//     Serial.println("Failed to publish message");
//   }

// }

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
  // is looking for UCL_IoT and a back up network (usually a home one!)
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    // loop through all the networks and if you find UCL_IoT or the backup - ssid1
    // then connect to wifi
    Serial.print("Trying to connect to: ");
    Serial.println(ssid);
    for (int i = 0; i < n; ++i){
      String availablessid = WiFi.SSID(i);
      // Primary network
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
          break; // Exit the loop if connected
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
  Serial.println("iFi connected");
  Serial.print("IP address: ");
  Serial.printlWn(WiFi.localIP());

}

void reconnectMQTT() {
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  } else {
    //Serial.println(WiFi.localIP());
  }

  // Loop until we're reconnected
  while (!client.connected()) {    // while not (!) connected....
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "LuminaSelector";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // ... and subscribe to messages on broker
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, int length) {
  // Handle incoming messages
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

 void clearAllColor(){
  char mqtt_message[100];
  char mqtt_topic[] = "student/CASA0014/light/14/all/";
  sprintf(mqtt_message, "{\"method\":\"clear\"}" );
    //sprintf(mqtt_message, "{\"method\": \"allrandom\"}");
    Serial.println(mqtt_topic);
    Serial.println(mqtt_message);
     if (client.publish(mqtt_topic, mqtt_message)) {
        Serial.println("Message published");
  } else {
        Serial.println("Failed to publish message");
    }
 }

 void checkButton() {
  int buttonState = digitalRead(Button_pin);
  if (buttonState != lastButtonState && (millis() - buttonLastDebounceTime) > buttonDebounceDelay) {
    buttonLastDebounceTime = millis();
    if (buttonState == LOW) {
      Serial.print("Selected: ");
      Serial.println(menu[menuIndex]);
      displaySelection(menu[menuIndex]);
      // Get the selected currency's exchange rate
    }
  }
  lastButtonState = buttonState;
}

 void checkRotation() {
  int currentA = digitalRead(A_pin);
  if (currentA != lastA && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    int currentB = digitalRead(B_pin);
    // Update menu index based on rotation direction
    menuIndex += (currentA == HIGH) ? (currentB == LOW ? 1 : -1) : 0;
    if (menuIndex < 0) menuIndex = menuItems - 1; // Loop back to last item
    if (menuIndex >= menuItems) menuIndex = 0;    // Loop back to first item
    displayMenu();  // Update the displayed menu
  }
  lastA = currentA;
}

// Display menu items on the OLED screen
void displayMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Display a title for the menu
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Select Currency:");

  // Print each menu item with a cursor ">" next to the selected item
  for (int i = 0; i < menuItems; i++) {
    display.print(i == menuIndex ? "> " : "  ");
    display.println(menu[i]);
  }
  display.display();
}

// Display the selected menu item and its exchange rate
void displaySelection(const char* selection) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Selected: ");
  display.println(selection);
  display.display();
}
