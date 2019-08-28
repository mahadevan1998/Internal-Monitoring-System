#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <PubSubClient.h>
#include <SparkFun_ADXL345.h>
#include <SimpleDHT.h>


#define WIFISSID "Subramanian" // Put your WifiSSID here
#define PASSWORD "subramanian1998" // Put your wifi password here
#define TOKEN "BBFF-Asmmr1qeqbx3b2qWhj8IxUZ1sdDrMy" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "asdfgh ;lkjhj" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
unsigned long last = 0;

/****************************************
 * SERVER DETAILS
 ****************************************/
#define x_axis "xdisplay" // Assing the variable label
#define y_axis "ydisplay" // Assing the variable label
#define z_axis "zdisplay" // Assing the variable label
#define temp1  "temp1"
#define temp2  "temp2"

//#define x_axis_SUBSCRIBE "switch" // Assing the variable label
#define DEVICE_LABEL "ims1" // Assig the device label

/*********************************************
 * PIN DECLARATIONS AND OBJECTS INITIALISATION
 *********************************************/
int dataPinSensor1 = D3;
int dataPinSensor2 = D7;
SimpleDHT22 dht22_1(dataPinSensor1);
SimpleDHT22 dht22_2(dataPinSensor2);
ADXL345 adxl = ADXL345();                                           //it should be a random and unique ascii string and different from all other devices
int i = 0;

char mqttBroker[]  = "things.ubidots.com";
char payload[100];
char topic[150];
char topicSubscribe[100];
// Space to store values to send
char stx_sensor[10];
char sty_sensor[10];
char stz_sensor[10];
char temp1_sensor[10];
char temp2_sensor[10];

/****************************************
 * Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);



void reconnect() {
  // Loop until we're reconnected
  
  int i=0;
  while((!client.connected())&& i<=50) {
      Serial.println("Attempting MQTT connection...");
      i++;
     }
        // Attemp to connect
  if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
      //client.subscribe(topicSubscribe);
    }
  else
     Serial.println("Failed to connect");
  }

/*void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  if (message == "0") {
    digitalWrite(relay, HIGH);
  } else {
    digitalWrite(relay, LOW);
  }
  
  Serial.write(payload, length);
  Serial.println();
}*/

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  WiFi.begin(WIFISSID, PASSWORD);
   i=0;
  while ((WiFi.status() != WL_CONNECTED)&& (i<=50)) {
    Serial.print(".");
    i++;
    delay(500);
    }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  //client.setCallback(callback);

  //sprintf(topicSubscribe, "/v1.6/devices/%s/%s/lv", DEVICE_LABEL, x_axis_SUBSCRIBE);
  
  //client.subscribe(topicSubscribe);
  acc_setup();
}

void loop() {
  if (!client.connected()) {
    client.subscribe(topicSubscribe);   
    Serial.println(topicSubscribe);
    reconnect();
  }

  orient();
 
 if ((millis() - last) >= (60*1000UL)) {
    temp();
    delay(2000);
    last = millis();
    }


client.loop();

}

void acc_setup(){
  
 
  
  adxl.powerOn();                     // Power on the ADXL345

  adxl.setRangeSetting(16);           // Give the range settings
                                      // Accepted values are 2g, 4g, 8g or 16g
                                      // Higher Values = Wider Measurement Range
                                      // Lower Values = Greater Sensitivity

  //adxl.setSpiBit(0);                  // Configure the device to be in 4 wire SPI mode when set to '0' or 3 wire SPI mode when set to 1
                                      // Default: Set to 1
                                      // SPI pins on the ATMega328: 11, 12 and 13 as reference in SPI Library 
   
  adxl.setActivityXYZ(1, 0, 0);       // Set to activate movement detection in the axes "adxl.setActivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
  adxl.setActivityThreshold(75);      // 62.5mg per increment   // Set activity   // Inactivity thresholds (0-255)
 
  adxl.setInactivityXYZ(1, 0, 0);     // Set to detect inactivity in all the axes "adxl.setInactivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
  adxl.setInactivityThreshold(75);    // 62.5mg per increment   // Set inactivity // Inactivity thresholds (0-255)
  adxl.setTimeInactivity(10);         // How many seconds of no activity is inactive?

  adxl.setTapDetectionOnXYZ(0, 0, 1); // Detect taps in the directions turned ON "adxl.setTapDetectionOnX(X, Y, Z);" (1 == ON, 0 == OFF)
 
  // Set values for what is considered a TAP and what is a DOUBLE TAP (0-255)
  adxl.setTapThreshold(50);           // 62.5 mg per increment
  adxl.setTapDuration(15);            // 625 μs per increment
  adxl.setDoubleTapLatency(80);       // 1.25 ms per increment
  adxl.setDoubleTapWindow(200);       // 1.25 ms per increment
 
  // Set values for what is considered FREE FALL (0-255)
  adxl.setFreeFallThreshold(7);       // (5 - 9) recommended - 62.5mg per increment
  adxl.setFreeFallDuration(30);       // (20 - 70) recommended - 5ms per increment
 
  // Setting all interupts to take place on INT1 pin
  //adxl.setImportantInterruptMapping(1, 1, 1, 1, 1);     // Sets "adxl.setEveryInterruptMapping(single tap, double tap, free fall, activity, inactivity);" 
                                                        // Accepts only 1 or 2 values for pins INT1 and INT2. This chooses the pin on the ADXL345 to use for Interrupts.
                                                        // This library may have a problem using INT2 pin. Default to INT1 pin.
  
  // Turn on Interrupts for each mode (1 == ON, 0 == OFF)
  adxl.InactivityINT(1);
  adxl.ActivityINT(1);
  adxl.FreeFallINT(1);
  adxl.doubleTapINT(1);
  adxl.singleTapINT(1);
  }



  

  void orient()
  {
    // Accelerometer Readings
  int x,y,z;   
  adxl.readAccel(&x, &y, &z);         // Read the accelerometer values and store them in variables declared above x,y,z
  Serial.print(x);
  Serial.print(", ");
  Serial.print(y);
  Serial.print(", ");
  Serial.println(z); 
  

  
  dtostrf(x, 4, 2, stx_sensor);
  dtostrf(y, 4, 2, sty_sensor);
  dtostrf(z, 4, 2, stz_sensor);
  
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", x_axis); // Adds the variable label
  
  sprintf(payload, "%s {\"value\": %s}}", payload, stx_sensor); // Adds the value
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  //delay(500);
  
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", y_axis); // Adds the variable label
  
  
  sprintf(payload, "%s {\"value\": %s}}", payload, sty_sensor); // Adds the value
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  //delay(500);
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", z_axis); // Adds the variable label
  
  sprintf(payload, "%s {\"value\": %s}}", payload, stz_sensor); // Adds the value
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  //delay(1000);

  }


  void temp()
  {
 
  float temperature1 = 0;
  float humidity1 = 0;
  int err1 = SimpleDHTErrSuccess;
  if ((err1 = dht22_1.read2(&temperature1, &humidity1, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 ONE failed, err="); Serial.println(err1);delay(2000);
    return;
  }
  
  Serial.print("Sample OK: ONE ");
  Serial.print((float)temperature1); Serial.print(" *C, ");
  Serial.print((float)humidity1); Serial.println(" RH%");

  dtostrf(temperature1, 4, 2, temp1_sensor);

  // Reading data from sensor 2...
  // ============================
  Serial.println("Getting data from sensor 2...");

  //delay(2500);

  float temperature2 = 0;
  float humidity2= 0;
  int err2 = SimpleDHTErrSuccess;
  if ((err2 = dht22_2.read2(&temperature2, &humidity2, NULL)) != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 TWO failed, err="); Serial.println(err2);delay(2000);
    return;
  }
  
  Serial.print("Sample OK: TWO");
  Serial.print((float)temperature2); Serial.print(" *C, ");
  Serial.print((float)humidity2); Serial.println(" RH%");

  delay(1500);
  

  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", temp1); // Adds the variable label
  
  sprintf(payload, "%s {\"value\": %s}}", payload, temp1_sensor); // Adds the value
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  
  

  dtostrf(temperature2, 4, 2, temp2_sensor);

  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", temp2); // Adds the variable label
  
  sprintf(payload, "%s {\"value\": %s}}", payload, temp2_sensor); // Adds the value
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  

  // DHT11 sampling rate is 1HZ.
  //delay(1500);

 }
