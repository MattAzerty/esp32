//FOR ROOMSYSTEM APP

//--LIB--
#include <FirebaseESP32.h>    //FIREBASE
#include <WiFi.h>             //WIFI SUPPORT
#include <Preferences.h>      //SAVED CONSTANT
#include "BluetoothSerial.h"  //BT SUPPORT
#include "DHT.h"              //HUMIDITY DHT11 SENSOR
#include <NTPClient.h>        //FOR TIMESTAMP
#include <WiFiUdp.h>          //FOR TIMESTAMP
#include "heltec.h"           //FOR DISPLAY
// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
#include <esp_system.h>  //FOR RESTART

//--SHARED DATA--

//FIREBASE
#define FIREBASE_HOST "*************************************"
#define FIREBASE_Authorization_key "******************"
#define API_KEY "**************"
#define USER_EMAIL "*****"
#define USER_PASSWORD "*****"
FirebaseData firebaseData;
FirebaseData stream;
FirebaseJson json;
FirebaseJson json2;
// Define the FirebaseAuth data for authentication data
FirebaseAuth auth;
// Define the FirebaseConfig data for config data
FirebaseConfig config;
bool isUserConnected = false;  //CHECK IF USER CONNECTED ON FIREBASE

//DHT11 sensor
#define DHTPIN 17  //FOR DHT11 CONNECTION
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

//TIME
WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "pool.ntp.org");
//WiFiClient wifiClient;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

unsigned long initialTime;
const unsigned long uploadInterval = 15000;    //msec
const unsigned long histUpInterval = 21600000;  //msec

unsigned long lastUploadTime = 0;
unsigned long lastUploadHistTime = 21600005;
//unsigned long readStreamPrevMillis;

String ssids_array[50];
String network_string;
String connected_string;
const char *pref_ssid = "";
const char *pref_pass = "";
const char *pref_uid = "";
String client_wifi_ssid;
String client_wifi_password;
String client_firebase_uid;
const char *bluetooth_name = "ESP32-BT";
long start_wifi_millis;
long wifi_timeout = 15000;
bool wifi_connect = false;
enum wifi_setup_stages { NONE,
                         BT_INI,
                         SCAN_START,
                         SCAN_COMPLETE,
                         SSID_ENTERED,
                         WAIT_PASS,
                         PASS_ENTERED,
                         WAIT_UID,
                         UID_ENTERED,
                         WAIT_CONNECT,
                         LOGIN_FAILED };
enum wifi_setup_stages wifi_stage = NONE;
BluetoothSerial SerialBT;
Preferences preferences;


void setup()  //-------------SETUP------------------------
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  //digitalWrite(LED,LOW);

  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);

  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "Starting...");
  Heltec.display->display();

  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(true);

  dht.begin();  //Start DHT11 acquisition

  preferences.begin("wifi_access", false);
  if (!init_wifi()) {  // Connect to Wi-Fi fails
    Heltec.display->drawString(0, 10, "Connecting...Failed");
    Heltec.display->drawString(0, 20, "Device in Bluetooth:");
    Heltec.display->drawString(0, 30, "---Check your phone---");
    Heltec.display->display();
    SerialBT.register_callback(callback);
    SerialBT.begin(bluetooth_name);
  } else {
    SerialBT.flush();
    SerialBT.disconnect();
    SerialBT.end();
  }


}  //END OF SETUP

bool init_wifi() {
  String temp_pref_ssid = preferences.getString("pref_ssid");
  String temp_pref_pass = preferences.getString("pref_pass");
  pref_ssid = temp_pref_ssid.c_str();
  pref_pass = temp_pref_pass.c_str();
  Serial.println(pref_ssid);
  Serial.println(pref_pass);
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  start_wifi_millis = millis();
  WiFi.begin(pref_ssid, pref_pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start_wifi_millis > wifi_timeout) {
      WiFi.disconnect(true, true);
      wifi_connect = false;
      return false;
    }
  }
  
  SerialBT.println("--END OF BT COM--");
  delay(1000);
  SerialBT.flush();
  SerialBT.disconnect();
  SerialBT.end();

  /* Assign the database URL and database secret(required) */
  config.database_url = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_Authorization_key;

  // Optional, set AP reconnection in setup()
  //Firebase.reconnectWiFi(true);

  config.timeout.wifiReconnect = 10 * 1000;
  config.timeout.rtdbStreamReconnect = 1 * 1000;
  config.timeout.serverResponse = 10 * 1000;
  config.timeout.rtdbStreamError = 5 * 1000;

  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);
  Firebase.setReadTimeout(stream, 1000);
  String userUID = preferences.getString("pref_uid");
  if (!Firebase.beginStream(stream, "/UsersData/HTSensor/" + userUID + "/isUserConnected")) {
    // Could not begin stream connection, then print out the error detail
    Serial.println(stream.errorReason());
  }
//Firebase.setStreamCallback(stream, streamCallback, streamTimeoutCallback);

  // Initialize NTP client
  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  initialTime = timeClient.getEpochTime();
  // Convert epoch time to tm struct
  struct tm *timeinfo = localtime((const time_t *)&initialTime);
  // Get formatted date and time
  char formattedTime[32];
  strftime(formattedTime, 32, "%Y:%m:%d-%H:%M:%S", timeinfo);

  Heltec.display->clear();
  Heltec.display->drawString(0, 0, "CONNECTED");
  Heltec.display->display();
  wifi_connect = true;
  return true;
}

void scan_wifi_networks() {
  WiFi.mode(WIFI_STA);
  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0) {
    SerialBT.println("no networks found");
  } else {
    SerialBT.println();
    SerialBT.print(n);
    SerialBT.println(" networks found");
    delay(1000);
    for (int i = 0; i < n; ++i) {
      ssids_array[i + 1] = WiFi.SSID(i);
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.println(ssids_array[i + 1]);
      network_string = i + 1;
      network_string = network_string + ": " + WiFi.SSID(i) + " (Strength:" + WiFi.RSSI(i) + ")";
      SerialBT.println(network_string);
    }
    wifi_stage = SCAN_COMPLETE;
  }
}

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)  //Callback for BT interractions
{

  if (event == ESP_SPP_SRV_OPEN_EVT) {
    wifi_stage = BT_INI;
  }

  if (event == ESP_SPP_DATA_IND_EVT && wifi_stage == WAIT_UID) {  // data from phone is userUID
    client_firebase_uid = SerialBT.readString();
    client_firebase_uid.trim();
    wifi_stage = UID_ENTERED;
  }

  if (event == ESP_SPP_DATA_IND_EVT && wifi_stage == SCAN_COMPLETE) {  // data from phone is SSID
    int client_wifi_ssid_id = SerialBT.readString().toInt();
    client_wifi_ssid = ssids_array[client_wifi_ssid_id];
    wifi_stage = SSID_ENTERED;
  }

  if (event == ESP_SPP_DATA_IND_EVT && wifi_stage == WAIT_PASS) {  // data from phone is password
    client_wifi_password = SerialBT.readString();
    client_wifi_password.trim();
    wifi_stage = PASS_ENTERED;
  }
}

void loop()  //#########################################################################-----LOOP------###############################################################
{
  delay(1000);
  static unsigned long lastMillis = 0;
  static unsigned long long rollovers = 0;

  if (millis() < lastMillis) {
    rollovers++;
  }

  lastMillis = millis();
  unsigned long long elapsedTime = (rollovers * ((unsigned long long)4294967296)) + millis();  //4294967296 is the maximum value that can be stored in a 32-bit unsigned integer data type.


   if (Firebase.ready()) {
    if (!Firebase.readStream(stream))
      Serial.printf("sream read error, %s\n\n", stream.errorReason().c_str());


    if (stream.streamTimeout()) {
      Serial.println("stream timed out, resuming...\n");

      if (!stream.httpConnected())
        Serial.printf("error code: %d, reason: %s\n\n", stream.httpCode(), stream.errorReason().c_str());
    }

    if (stream.streamAvailable()) {
      if (stream.dataTypeEnum() == fb_esp_rtdb_data_type_boolean){isUserConnected = stream.boolData();} 
      Serial.println(isUserConnected);
    }
  } 

  if (wifi_connect) {

    //Case userConnected
    if (Firebase.ready() && isUserConnected && millis() - lastUploadTime >= uploadInterval) {
      lastUploadTime = millis();

      Serial.println("realtime");
      String userUID = preferences.getString("pref_uid");

      json.set("realTimeTEMPERATURE", dht.readTemperature()-2);
      json.set("realTimeHUMIDITY", dht.readHumidity());

      if (!Firebase.updateNodeAsync(stream, "/UsersData/HTSensor/" + userUID, json)) {
        Heltec.display->drawString(10, 25, "                  ");
        Heltec.display->drawString(10, 25, "Error On Firebase");
        Heltec.display->display();
      } else {
        Heltec.display->drawString(10, 25, "                  ");
        Heltec.display->display();
      }
    }

    //Case for histroy data
    if (Firebase.ready() && millis() - lastUploadHistTime >= histUpInterval) {
      lastUploadHistTime = millis();
      uploadHistData(elapsedTime);
    }
  }

  if (!Firebase.ready()) {

    switch (wifi_stage) {
      case BT_INI:
        Heltec.display->clear();
        Heltec.display->drawString(0, 00, "Phone connected...");
        Heltec.display->drawString(0, 10, "Select your SSID:");
        Heltec.display->display();
        SerialBT.println("Ready to receive userUID...");
        wifi_stage = WAIT_UID;
        break;

      case UID_ENTERED:
        preferences.putString("pref_uid", client_firebase_uid);
        SerialBT.println("userUID: " + client_firebase_uid);
        SerialBT.println();
        wifi_stage = SCAN_START;
        break;

      case SCAN_START:
        SerialBT.println("Scanning Wi-Fi networks");
        Serial.println("Scanning Wi-Fi networks");
        scan_wifi_networks();
        SerialBT.println("Please enter the number for your Wi-Fi");
        wifi_stage = SCAN_COMPLETE;
        break;

      case SSID_ENTERED:
        Heltec.display->drawString(0, 20, client_wifi_ssid);
        Heltec.display->display();
        SerialBT.println("Please enter your Wi-Fi password");
        Heltec.display->drawString(0, 40, "Enter password:");
        Heltec.display->display();
        wifi_stage = WAIT_PASS;
        break;

      case PASS_ENTERED:
        Heltec.display->drawString(80, 40, "****");
        Heltec.display->display();
        SerialBT.println("Please wait for Wi-Fi connection...");
        Serial.println("Please wait for Wi_Fi connection...");
        wifi_stage = WAIT_CONNECT;
        preferences.putString("pref_ssid", client_wifi_ssid);
        preferences.putString("pref_pass", client_wifi_password);
        if (init_wifi()) {  // Connected to WiFi
          connected_string = "ESP32 IP: ";
          connected_string = connected_string + WiFi.localIP().toString();
          //SerialBT.println(connected_string);
        } else {  // try again
          wifi_stage = LOGIN_FAILED;
          Heltec.display->clear();
          Heltec.display->drawString(0, 0, "Login failed");
          Heltec.display->drawString(0, 10, "Select your SSID:");
          Heltec.display->display();
        }
        break;

      case LOGIN_FAILED:
        SerialBT.println("Wi-Fi connection failed");
        Serial.println("Wi-Fi connection failed");
        delay(2000);
        wifi_stage = SCAN_START;
        break;
    }
  }

}  //END OF LOOP

//For historyValues
void uploadHistData(unsigned long long elapsedTime) {

  unsigned long long epochTime = initialTime + elapsedTime / 1000;
  // Convert epoch time to tm struct
  struct tm *timeinfo = localtime((const time_t *)&epochTime);
  // Get formatted date and time
  char formattedTime[32];
  strftime(formattedTime, 32, "%Y:%m:%d-%H:%M:%S", timeinfo);
  Serial.println(formattedTime);

  float hum2 = dht.readHumidity();
  float temp2 = dht.readTemperature() - 2;

  Heltec.display->clear();
  Heltec.display->drawString(0, 00, "Temperature:");
  Heltec.display->drawString(70, 00, (String)temp2);
  Heltec.display->drawString(100, 00, "°C");
  Heltec.display->drawString(0, 10, "Humidity:");
  Heltec.display->drawString(70, 10, (String)hum2);
  Heltec.display->drawString(100, 10, "%");
  Heltec.display->drawString(0, 40, "Last time updated:");
  Heltec.display->drawString(0, 50, formattedTime);
  Heltec.display->display();

  String userUID = preferences.getString("pref_uid");
  Serial.println(userUID);

  json2.set("timestamp", formattedTime);
  json2.set("temperature", temp2);
  json2.set("humidity", hum2);

  if (Firebase.ready() && !Firebase.pushJSONAsync(stream, "/UsersData/HTSensor/" + userUID + "/HISTORY", json2)) {
    Serial.println(stream.errorReason().c_str());
    Heltec.display->drawString(10, 25, "                  ");
    Heltec.display->drawString(10, 25, "Error On Firebase");
    Heltec.display->display();
  } else {
    Heltec.display->drawString(10, 25, "                  ");
    Heltec.display->display();
  }
}
