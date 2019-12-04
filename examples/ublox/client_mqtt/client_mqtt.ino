#define CELLMODEM_MODEL UBLOX
#define CELLMODEM_USE_ARDUINO_CLIENT
#include "CellularModem.h"
#include "MQTT.h"

#ifdef ARDUINO_SAMD_MKRGSM1400
  #define modemSerial       SerialGSM
  
  #define MODEM_ON_OFF_PIN            -1
  #define MODEM_RESET_PIN             GSM_RESETN
  #define MODEM_STATUS_PIN            -1

  #define MODEM_DTR_PIN               GSM_DTR
  #define MODEM_CTS_PIN               GSM_CTS
  #define MODEM_DCD_PIN               -1
  #define MODEM_RTS_PIN               GSM_RTS
  #define MODEM_RI_PIN                -1

  #define debugSerial       Serial

  #define SERIAL_BAUDRATE   115200
#else
  #warning "User should define his own pins"
#endif

#define TOGGLE_LED    LED_BUILTIN

CellularModem modem(modemSerial, MODEM_ON_OFF_PIN, MODEM_RESET_PIN, MODEM_STATUS_PIN); 
CellularModemClient modemClient(modem);
MQTTClient mqttClient;

const char * modemApn = "orangeworld";
const char * modemUsername = "";
const char * modemPassword = "";

const char * server = "antaresserver.dynu.net";
//const int port = 80;

void connect() {
  if(!modem.networkOn()) {
    return;
  }
   
   //modemClient.setTCPMode(1);
  

  if(!modem.isGPRSConnected()) {
    modem.attachGPRS("orangeworld", nullptr, nullptr);

    while (!mqttClient.connect("arduino", "lab", "lab1234")) {
      debugSerial.print(".");
      delay(5000);
    }
  
    debugSerial.println("\nconnected!");

    if (!mqttClient.connected()) {
      debugSerial.println("Not Connected");
    } 
    
    mqttClient.subscribe("/led");
  }
}

void messageReceived(MQTTClient *client, char topic[], char payload[], int payload_length) {
  Serial.print("Incoming: ");
  Serial.print(topic);
  Serial.print(" ");
  Serial.println(payload);

  if(strstr(topic, "led") && payload_length == 1) {
    int value = atoi(payload);

    if(value == 0) {
      digitalWrite(TOGGLE_LED, LOW);
    }
    else if(value == 1) {
      digitalWrite(TOGGLE_LED, HIGH);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  modemSerial.begin(SERIAL_BAUDRATE);
  debugSerial.begin(SERIAL_BAUDRATE);

  pinMode(TOGGLE_LED, OUTPUT);
  digitalWrite(TOGGLE_LED, LOW);
    
  modem.init();
  modem.setUartPins(UartPins_t{MODEM_DTR_PIN, MODEM_DCD_PIN, MODEM_CTS_PIN, MODEM_RTS_PIN, MODEM_RI_PIN});
  modem.setDebugSerial(debugSerial);

  
  delay(1000);
  debugSerial.println("Start");
  mqttClient.begin("antaresserver.dynu.net", 1883, modemClient);
  mqttClient.onMessageAdvanced(messageReceived);
  connect();

}

void loop() {
  // put your main code here, to run repeatedly:
  static unsigned long lastMillis = 0;

  modem.poll(250);
  if(!mqttClient.loop()) {
    debugSerial.print("MQTT Error: ");
    debugSerial.println(mqttClient.lastError());
    while(1);
  }

  /*if (!mqttClient.connected()) {
    connect();
  }*/

  // publish a message roughly every second.
  if (millis() - lastMillis > 30000) {
    lastMillis = millis();
    mqttClient.publish("/hello", "world");
  }
}
