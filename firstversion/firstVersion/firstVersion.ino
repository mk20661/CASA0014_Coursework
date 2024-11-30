#include <WiFiNINA.h>   
#include <PubSubClient.h>
#include <utility/wifi_drv.h>   // library to drive to RGB LED on the MKR1010
#include "arduino_secrets.h" 

#define LDR A1
#define soundPin A0

const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;
int status = WL_IDLE_STATUS;


WiFiServer server(80);
WiFiClient wificlient;

WiFiClient mkrClient;
PubSubClient client(mkrClient);

const int sampleSize = 10;
int samples[sampleSize];
int sampleIndex = 0;

const int lightID = 15;



void setup() {
  // Start the serial monitor to show output
  Serial.begin(115200);
  delay(1000);

  WiFi.setHostname("Lumina ucjtdjw");
  startWifi();
  client.setServer(mqtt_server, mqtt_port);
  Serial.println("setup complete");
}

void loop() {
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

} 
int lastPxielNum = 0;
void sendmqtt(){
  int lightLevel = analogRead(LDR);
  int soundLevel = analogRead(soundPin);
  Serial.print("Light level: ");
  Serial.println(lightLevel);
  int lightlBrightness = map(lightLevel,0,1023,0,119);
  setbrightness(lightlBrightness);
  samples[sampleIndex] = soundLevel; // store in sample array 
  sampleIndex = (sampleIndex + 1) % sampleSize; // update sample 

  int averageValue = 0;
  for (int i = 0; i < sampleSize; i++) {
    averageValue += samples[i];
  }
  averageValue /= sampleSize;
  int r,g,b;
  int pixelNum = map(averageValue,0,1023,1,12);
 if (averageValue <= 200) {
    r = 255;//white
    g = 255;
    b = 255;
 }else if (averageValue > 200 && averageValue <= 400){
    r = 0;//green
    g = 255;
    b = 0;
 }else if (averageValue >400 && averageValue <= 600){
    r = 0;//blue
    g = 0;
    b = 255;
 }else if (averageValue >600 && averageValue <= 800){
    r = 255;//yellow
    g = 255;
    b = 0;
 }else{
    r = 255;//red
    g = 0;
    b = 0;
 }
  Serial.print("Sound level: ");
  Serial.println(averageValue);
  char mqtt_message[100];
  if (lastPxielNum > pixelNum){
    for(int i = 11; i > (lastPxielNum - pixelNum);i-- ){
        clearOnePxile(i);
    }
  }

  for(int i = 0; i < pixelNum; i++){
    setOnePxiel(i,r,g,b);
  }
  // strcat(mqtt_message, "]}");
  // Serial.println(mqtt_topic_all);
  // Serial.println(mqtt_message);

  // if (client.publish(mqtt_topic_all, mqtt_message)) {
  //   Serial.println("Message published");
  // } else {
  //   Serial.println("Failed to publish message");
  // }
  lastPxielNum = pixelNum;
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
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

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

 void clearOnePxile(int num){
  char mqtt_message[100];
  char mqtt_topic[100];
  sprintf(mqtt_topic, "student/CASA0014/light/%d/pixel/",lightID);
  sprintf(mqtt_message, "{\"pixelid\": %d, \"R\": 0, \"G\": 0, \"B\": 0, \"W\": 0}", num);
    //sprintf(mqtt_message, "{\"method\": \"allrandom\"}");
    Serial.println(mqtt_topic);
    Serial.println(mqtt_message);
     if (client.publish(mqtt_topic, mqtt_message)) {
        Serial.println("Message published");
    } else {
        Serial.println("Failed to publish message");
    }
    delay(10);
 }

 void setbrightness(int brightness){
  char mqtt_message[100];
  char mqtt_topic[100];
  sprintf(mqtt_topic, "student/CASA0014/light/%d/brightness/",lightID);
  sprintf(mqtt_message, "{\"brightness\": \"%d\"}", brightness);
    Serial.println(mqtt_topic);
    Serial.println(mqtt_message);
     if (client.publish(mqtt_topic, mqtt_message)) {
        Serial.println("Message published");
    } else {
        Serial.println("Failed to publish message");
    }
    delay(10);
 }

 void setOnePxiel(int num,int R, int G, int B){
  char mqtt_message[100];
  char mqtt_topic[100];
  sprintf(mqtt_topic, "student/CASA0014/light/%d/pixel/",lightID);
  sprintf(mqtt_message, "{\"pixelid\": %d, \"R\": %d, \"G\": %d, \"B\": %d, \"W\": 0}", num,R,G,B);
    Serial.println(mqtt_topic);
    Serial.println(mqtt_message);
     if (client.publish(mqtt_topic, mqtt_message)) {
        Serial.println("Message published");
    } else {
        Serial.println("Failed to publish message");
    }
    delay(10);
 }