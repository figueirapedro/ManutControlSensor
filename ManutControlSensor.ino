#include <WiFi.h> //Lib WiFi
#include <FirebaseESP32.h>

#include "time.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define pinoSinal 32 // PINO ANALÓGICO UTILIZADO PELO MÓDULO
#define pinoLed 5 //PINO DIGITAL UTILIZADO PELO LED

//SSID e senha da rede WiFi onde o esp32 irá se conectar 
#define SSID "Whopper"
#define PASSWORD "19651968"
#define DATABASE_URL "manutcontrol-fc8ae-default-rtdb.firebaseio.com" // URL da base de dados fornecido pelo Firebase para a conexão http
#define API_KEY "AIzaSyC0XqyJUQg51YnhUAYaMrahkrPYue5JNsQ"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "pedro.figueirab@hotmail.com"
#define USER_PASSWORD "senha123"

FirebaseData fbdo;
FirebaseConfig config;
FirebaseAuth auth;
FirebaseJson json;

// Variable to save USER UID
String uid;

// Database main path (to be updated in setup with the user UID)
String databasePath;

const char* ntpServer = "pool.ntp.org";

// Database child nodes
String vibPath = "/vibration";
String timePath = "/timestamp";

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 1000;

int timestamp;

/*
// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}
*/

void setup() {
  // put your setup code here, to run once:
  Serial.begin (115200);
  pinMode(pinoSinal, INPUT); //DEFINE O PINO COMO ENTRADA
  pinMode(pinoLed, OUTPUT); //DEFINE O PINO COMO SAÍDA
  digitalWrite(pinoLed, LOW); //LED INICIA DESLIGADO

  WiFi.begin (SSID, PASSWORD);
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  
 // Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

    Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }

  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  databasePath = "/UsersData/" + uid + "/readings";
}

void loop() {

  float vibration = analogRead(pinoSinal);

    if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    //Get current timestamp
    timestamp = millis()/1000;
    Serial.print ("time: ");
    Serial.println (timestamp);

    String parentPath= databasePath + "/" + String(timestamp);

    Serial.println ();

    Serial.print (" \ vib: ");
    Serial.println (String(vibration,2));
    json.set(vibPath.c_str(), String(vibration,2));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
  }

  if(vibration > 10) { //SE A LEITURA DO PINO FOR MAIOR QUE 10, FAZ
    digitalWrite(pinoLed, HIGH); //ACENDE O LED
  } else { //SENÃO
    digitalWrite(pinoLed, LOW); //APAGA O LED
  }
}