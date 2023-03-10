/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <Arduino_JSON.h>
#include <esp_now.h>
String json;
StaticJsonDocument<500> doc;
JsonObject root;
JsonObject usuarios;

#define SERVICE_UUID "ffe0"  // UART service UUID a5f81d42-f76e-11ea-adc1-0242ac120002
#define CHARACTERISTIC_UUID_RX "ffe1"
#define CHARACTERISTIC_UUID_TX "ffe2"

typedef struct struct_message {
    int msj;
} struct_message;

// Create a struct_message called myData
struct_message myData;

const int LED = 2;
  const int rele = 33;
unsigned long r = 0;
int contador = 0;
unsigned long resta = 0;
unsigned long resta2 = 0;
unsigned long tiempo = 5000UL;
unsigned long tiempo2 = 5000UL;
bool bandera;
String nombre1;
String nombre2;
int scanTime = 1;  //In seconds
int reset = 0;
BLEScan *pBLEScan;
bool device_found;

String Adresse = "A0:DE:0F:9C:8A:0B";

void create_user_json() {
  root = doc.to<JsonObject>();
  usuarios = root["usuarios"].to<JsonObject>();

  ////-------ACÁ SE ASIGNAN LOS DISPOSITIVOS--------------------

  usuarios["Beacon JC"] = "ESP JC";
  usuarios["HUAWEI WATCH FIT-A0B"] = "Reloj JC";
  usuarios["HUAWEI WATCH GT 2-B88"]="Reloj Alejandro";
 // usuarios["AIRCALL SV"] = "Aircall ofi";
 // usuarios["FOOTCALL SV"] = "Footcall acceso";
  serializeJsonPretty(doc, Serial);
  serializeJsonPretty(doc, json);
}



class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    //    Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    // if(advertisedDevice.getName().c_str() == "HUAWEI WATCH GT 2-B88"){
    //        Serial.print("ADDRESS:");
    //        Serial.print(advertisedDevice.getAddress().toString().c_str());
    //        Serial.print("--");
    //        Serial.print("RSSI:");
    //        Serial.print(advertisedDevice.getRSSI());
    //        Serial.print("--");
    //        Serial.print("NAME:");
    //        Serial.println(advertisedDevice.getName().c_str());
    //  }
    //     if(advertisedDevice.getAddress().toString().c_str() == "d0:b4:5d:fd:ab:88"){
    //      Serial.print("---- FOUND ADDRESS OF THE ONE ---");
    //        Serial.println(advertisedDevice.getAddress().toString().c_str());
    //        Serial.print("RSSI:");
    //        Serial.println(advertisedDevice.getRSSI());
    //      }

    //1) con alguna app de escaneo encontrar el uid de tu celular y reemplazar getname por getaddress
    //if(advertisedDevice.getAddress() == "ED:4D:45:ed:45"){
    //2) hacer las pruebas pero con un ESP de bolsillo. AIRCALL (inveestigar que es un beacon)
    //3) ver si funciona en el mismo codigo que el tarjetero es decir que el tarjetero escanee pero tambien reciba BLE
    //modificar el tiempo de escaeno, si no funciona, dejar por separado como otro producto snifferBLE cuando sienta
    //envie un comando al tarjetero para abrir puerta
    //la idea es que se llegue a la puerta y esta abra sola al leer el UID del celular o del beacon
    //4) si se logra lo anterior mirar la forma de agregar UIDS o nombres a un JSON para agregrle mas UIDS
    //algo como lo del medidor (condigura credenciales por BLE y luego WIFI) entra a un modo config
    //donde permite agregar UID por medio de BLE

    nombre1 = (advertisedDevice.getName().c_str());
    if (nombre1 == "null" || nombre1 == "") {
      Serial.println("este BLE no tiene nombre");
    } else {
      Serial.print("Nombre de BLE: ");
      Serial.println(nombre1);
      Serial.println(advertisedDevice.getRSSI());
      chequeo(nombre1);
    }
 //  advertisedDevice.getRSSI() < -75 &&    
    if ( advertisedDevice.getRSSI()  > -77 && bandera == true) {
      Serial.println("USUARIO DENTRO DEL AREA DE LA PUERTA");
      Serial.println(advertisedDevice.getRSSI());
      Serial.println("ACTIVANDO RELÉ");
      digitalWrite(rele, HIGH);
      digitalWrite(LED, HIGH);
      delay(2000);
      digitalWrite(rele, LOW);
      bool bandera2 = false;
      while (bandera2 == false) {
        Serial.println("APAGANDO RELÉ Y PAUSANDO SCAN");
        digitalWrite(rele, LOW);
        contador++;
        delay(1000);
        Serial.println(contador);
        if (contador == 15) {
          digitalWrite(LED, LOW);
          bandera2 = true;
        }
      }
      ESP.restart();
    }
  }
};

void chequeo(String nombre1) {
  Serial.print("Entre al chequeo");
  Serial.println(nombre1);
  String nombre2 = usuarios[nombre1];
  Serial.print("Nombre del dispositivo: ");
  Serial.println(nombre2);
  if (nombre2 == "null") {
    Serial.println("usuario no existe");
   // ESP.restart();
  } else {
    Serial.println("USUARIO SI EXISTE");
    bandera = true;
  }
}

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
      Serial.println("***");
      Serial.print("Received Value: ");

      for (int i = 0; i < rxValue.length(); i++) {
        Serial.print(rxValue[i]);
      }

      Serial.println();

      if (rxValue.find("1") != -1) {
        Serial.println("ACTIVANDO RELÉ Y REINICIANDO ESP");
        digitalWrite(rele, HIGH);
        delay(1500);
        digitalWrite(rele, LOW);
        digitalWrite(LED, HIGH);
        delay(500);
        digitalWrite(LED, LOW);
        ESP.restart();
      }
    }
  }
};

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Mensaje: ");
  Serial.println(myData.msj);
  Serial.println();
  Serial.println("ACTIVANDO RELÉ Y REINICIANDO ESP");
  digitalWrite(rele, HIGH);
  delay(1200);
  digitalWrite(rele, LOW);
  delay(500);
 ESP.restart();
  
}

void setup() {
  Serial.begin(115200); 
  WiFi.mode(WIFI_STA);
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  
  Serial.println("Scanning...");
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(rele, OUTPUT);
  digitalWrite(rele, LOW);
  
      // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

   // Create the BLE Device
  BLEDevice::init("PV SV");  // Give it a name
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  create_user_json();
 

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY);

  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

}

void loop() {
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  int deviceCount = foundDevices.getCount();
  Serial.print("Devices found -------------- > ");
  Serial.println(deviceCount);
  Serial.println("Scan done!");
  pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory
  delay(1000);
  
 
 reset++;
 Serial.println(reset);
 delay(250);
 if (reset >= 60){
   Serial.println("Reseteando dispositivo");
   ESP.restart();    
 }
  
}