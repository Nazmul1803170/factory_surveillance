//Required Library
#include <DHT.h> //Required library for DHT sensor
#include <MQ2.h> //Required library for MQ2 sensor
#include <Wire.h> //Required library for wired connection
#include <ESP8266WiFi.h> //Required library for wifi connection
#include <WiFiManager.h>  //Required library for wifi management
#include <ArduinoJson.h> //Sometimes required for compiling
#include <FirebaseArduino.h> //Required library for firebase connection

//Firebase auth token
#define firebase_host_url "factory-surveillance-df7a6-default-rtdb.firebaseio.com"
#define firebase_secrect_code "fZWqCU9SiAzGF0LeKbBRLYogv8uP41PBHS5nSsNA"


//Pin Defination
#define error_led D5
#define wifi_connected_led D6
#define firebase_status_led D7

#define mq2_analog_input_pin A0

#define dht_sensor_pin D1
#define dht_sensor_type DHT11 //For DHT-11 sensor

//Creating DHT object
DHT dht(dht_sensor_pin, dht_sensor_type);
//Creating MQ2 object
MQ2 mq2(mq2_analog_input_pin);


//---------------------Function for Led Management------------------------------------//

void led_status(int error, int wifi, int firebase){
  digitalWrite(error_led, error);
  digitalWrite(wifi_connected_led, wifi);
  digitalWrite(firebase_status_led, firebase);
}

//Create wifimanager object
WiFiManager wifiManager;

//Function for establish connection with Wifi
void wifi_connection(){
  led_status(1, 0, 0);
  wifiManager.autoConnect("Factory_Surveillance");  
  Serial.println("Connected.....");
}
//Function for reset or erase saved AP from wifimanager
void reset_wifi(){
  wifiManager.resetSettings();
  led_status(1, 0, 0);
  wifi_connection();
}

//---------------------------Function for checking wifi connected or not--------------------------
void check_wifi(){
  if(WiFi.status() == WL_CONNECTED){
    digitalWrite(wifi_connected_led, 1);
    digitalWrite(error_led, 0);
  }
  else{
    digitalWrite(wifi_connected_led, 0);
    digitalWrite(error_led, 1);
  }
}

//------------Function for checking firebase is connected or not----------------//
void check_firebase(){
  String deviceStatus = Firebase.getString("deviceStatus");
  if(deviceStatus == "100"){
    Firebase.setInt("deviceStatus",200);
  }
  int firebase_status;
  if(Firebase.failed()){
    Serial.println(Firebase.error());
    firebase_status = 200;
  }
  
  firebase_status = Firebase.getInt("okStatus");
  if(firebase_status == 100){
    Serial.println("Successfully connected with Firebase.");
    digitalWrite(firebase_status_led, 1);
    digitalWrite(error_led, 0);
  }
  else{
    Serial.println("Failed to connect Firebase.");
    digitalWrite(firebase_status_led, 0);
    digitalWrite(error_led, 1);
  }
}


void setup(){
  //All pin mode setup
  pinMode(error_led, OUTPUT);
  pinMode(wifi_connected_led, OUTPUT);
  pinMode(firebase_status_led, OUTPUT);
  
  Serial.begin(115200);

  dht.begin();
  mq2.begin();

  wifi_connection();
  Firebase.begin(firebase_host_url, firebase_secrect_code);
}

void loop(){
  check_wifi();
  check_firebase();
  
  // Wait a few seconds between measurements.
  delay(2000);
  //-------------DHT Sensor-------------------//
  //Read Humidity
  float humidity = dht.readHumidity();
  //Read temperature as Celsius (the default)
  float tempc = dht.readTemperature();
  //Read temperature as Fahrenheit (isFahrenheit = true)
  float tempf = dht.readTemperature(true);

  //Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(tempc) || isnan(tempf)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  //Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(tempf, humidity);
  //Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(tempc, humidity, false);

  //Print all measurement by DHT sensor in Serial Monitor
  Serial.print(F("\nHumidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(tempc);
  Serial.print(F("°C "));
  Serial.print(tempf);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));


  //-------------MQ2 Sensor-------------------//
  //float* measured_values= mq2.read(true);
  //Measure LPG in ppm unit
  int lpg = mq2.readLPG();
  //Measure CO in ppm unit
  int co = mq2.readCO();
  //Measure SMOKE in ppm unit
  int smoke = mq2.readSmoke();

  //Print all measurement by MQ2 sensor in Serial Monitor
  Serial.print(F("LPG: "));
  Serial.print(lpg);
  Serial.print(F(" ppm  CO: "));
  Serial.print(co);
  Serial.print(F(" ppm  SMOKE: "));
  Serial.print(smoke);
  Serial.print(" ppm");

  //All measurement send in Firebase
  Firebase.setString("humidity",(String)humidity);
  Firebase.setString("tempc",(String)tempc);
  Firebase.setString("tempf",(String)tempf);
  Firebase.setString("hic",(String)hic);
  Firebase.setString("hif",(String)hif);
  Firebase.setString("lpg",(String)lpg);
  Firebase.setString("co",(String)co);
  Firebase.setString("smoke",(String)smoke);

}
