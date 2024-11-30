#include <WiFiNINA.h>   
#include <PubSubClient.h>
#include <utility/wifi_drv.h>   // library to drive to RGB LED on the MKR1010
#include "arduino_secrets.h" 

#define soundPin A0
#define LDR A1

const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;
int status = WL_IDLE_STATUS;

//Light 
const int startLightIndex = 40;
const int endLightIndex = 42;
const int totalLightNum = 3;
//smooth sound sesnor data 
int lastLightNum = endLightIndex - startLightIndex;
const int sampleSize = 10;
int samples[sampleSize];
int sampleIndex = 0;


//WIFI connect 
WiFiServer server(80);
WiFiClient wificlient;

WiFiClient mkrClient;
PubSubClient client(mkrClient);


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

  sendmqtt_voice();

} 

void sendmqtt_voice(){
  int soundLevel = 1023;//analogRead(soundPin);
  int lightLevel = analogRead(LDR);
  
 
  
  samples[sampleIndex] = soundLevel; // store in sample array 
  sampleIndex = (sampleIndex + 1) % sampleSize; // update sample 

  int averageValue = 0;
  for (int i = 0; i < sampleSize; i++) {
    averageValue += samples[i];
  }
  averageValue /= sampleSize;

  int lightNum = map(averageValue,0,1023,1,totalLightNum);
  int lightlBrightness = map(lightLevel,0,1023,20,119);
  Serial.print("Sound level: ");
  Serial.println(averageValue);
  Serial.print("Light level: ");
  Serial.println(lightLevel);
  char mqtt_message[100];
  if (lastLightNum > lightNum){
      for(int i = startLightIndex; i < lastLightNum - lightNum ; i++){
        clearAllColor(startLightIndex + i);
      }
  }
  for(int i = 0; i <lightNum; i++){
    setbrightness(endLightIndex - i, lightlBrightness);
    setOneLight(endLightIndex - i);
  }

  lastLightNum = lightNum;
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

 void clearAllColor(int num){
  char mqtt_message[100];
  char mqtt_topic[100];
  sprintf(mqtt_topic, "student/CASA0014/light/%d/all/",num);
  sprintf(mqtt_message, "{\"method\": \"clear\"}" );
    //sprintf(mqtt_message, "{\"method\": \"allrandom\"}");
    Serial.println(mqtt_topic);
    Serial.println(mqtt_message);
     if (client.publish(mqtt_topic, mqtt_message)) {
        Serial.println("Message published");
    } else {
        Serial.println("Failed to publish message");
    }
    delay(100);
 }


  void setOneLight(int num){
  char mqtt_message[100];
  char mqtt_topic[100];
  sprintf(mqtt_topic, "student/CASA0014/light/%d/pixel/",num);
  for(int i = 0; i < 12; i++){
      // send a message to update the light
      char mqtt_message[100];
      sprintf(mqtt_message, "{\"pixelid\": %d, \"R\": 255, \"G\": 0, \"B\": 0, \"W\": 20}", i);
      Serial.println(mqtt_topic);
      Serial.println(mqtt_message);
      if (client.publish(mqtt_topic, mqtt_message, false)) {
        Serial.println("Message published");
      } else {
        Serial.println("Failed to publish message");
      }
  }
    delay(100);
 }

 void setbrightness(int lightID, int brightness){
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
    delay(100);
 }