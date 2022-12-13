#include <Arduino.h>
#include <Wire.h>//I2C
#include <LiquidCrystal_I2C.h>//LCD
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include "SPIFFS.h" //Manejar archivos
#include <WiFiClientSecure.h>
#include <PubSubClient.h> //Libreria MQTT
// Crear el objeto lcd  dirección  0x3F y 16 columnas x 2 filas
LiquidCrystal_I2C lcd(0x3F, 16, 2);
/*Tiempo de espoxicion POR TIPO DE PIEL MÁS COMUNES tipo III (Blanca)- tipo IV(Morenos claro)-tipo V(Moreno oscuro)*/
String III;
String IV;
String V;

/*Sensor Uv*/
int UVOUT = 34;   // Output from the sensor
int REF_3V3 = 32; // 3.3V power on the ESP32 board
/*Sensor Tem Hum*/
#define DHT_SENSOR_PIN 14 // ESP32 pin GIOP14 connected to DHT11 sensor
#define DHT_SENSOR_TYPE DHT11 // Sensor de tem y hum
DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

// Puntero de credenciales WiFI
const char *ssid = "UNIIoT";
const char *password = "12345678";
const char *mqtt_server = "a3put2nuwxtsf7-ats.iot.sa-east-1.amazonaws.com";
const int mqtt_port = 8883;

String Read_rootca;
String Read_cert;
String Read_privatekey;
//********************************
#define BUFFER_LEN 256
long lastMsg = 0;
char msg[BUFFER_LEN];
int value = 0;
byte mac[6];
char mac_Id[18];
int count = 1;
//*********************************
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Coneccion al Wifi
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Conectando...");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi FIEE_IoT conectado");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
}

// confirmaicon de mensaje
//***********************************************************
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Mensaje Recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Reconeccion  ante una interupcion
void reconect()
{
  // Loop para reconccion
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Creando un ID como ramdon
    String clientId = "ESP32- ";
    clientId += String(random(0xffff), HEX);
    // Intentando conectarse
    if (client.connect(clientId.c_str()))
    {
      Serial.println("conectada");

      // conectando, publicando un payload...
      client.publish("ei_out", "hello word");

      //... y suscribiendo
      client.subscribe("ei_in");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("Esperando 5 segundos");

      // Tiempo muerto de 5 segundoa
      delay(5000);
    }
  }
}
//***********************************************************

void setup(){
  Serial.begin(9600);
  // initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Intencidad de UV");
  lcd.setCursor(0, 1);
  lcd.print("FIIEE_UNI_IoT");
  
  /*Sensor Uv*/
  pinMode(UVOUT, INPUT);
  pinMode(REF_3V3, INPUT);
  /*Sensor Tem Hum*/
  dht_sensor.begin(); // initialize the DHT sensor

  /*Conexion Aws*/
  setup_wifi();
  delay(1000);
  //****************************
  if (!SPIFFS.begin(true))
  {
    Serial.println("Se ha producido un error al montar SPIFFS");
    return;
  }
  //*****************************
  // Root CA leer archivo
  File file2 = SPIFFS.open("/AmazonRootCA1.pem", "r");
  if (!file2)
  {
    Serial.println("No se pudo abrir el archivo para leerlo");
    return;
  }
  Serial.println("Root CA File Content: ");
  while (file2.available())
  {
    Read_rootca = file2.readString();
    Serial.println(Read_rootca);
  }
  //***********************************
  // Cert leer archivo
  File file4 = SPIFFS.open("/6213aeb45c-certificate.pem.crt", "r");
  if (!file4)
  {
    Serial.println("No se pudo abrir el archivo para leerlo");
    return;
  }
  Serial.println("Cert File Content: ");
  while (file4.available())
  {
    Read_cert = file4.readString();
    Serial.println(Read_cert);
  }

  //*****************************
  // Privatekev leer archivo
  File file6 = SPIFFS.open("/6213aeb45c-private.pem.key", "r");
  if (!file6)
  {
    Serial.println("No se pudo abrir el archivo para leerlo");
    return;
  }
  Serial.println("privateKey Content: ");
  while (file6.available())
  {
    Read_privatekey = file6.readString();
    Serial.println(Read_privatekey);
  }
  //*****************************

  char *pRead_rootca;
  pRead_rootca = (char *)malloc(sizeof(char) * (Read_rootca.length()));
  strcpy(pRead_rootca, Read_rootca.c_str());

  char *pRead_cert;
  pRead_cert = (char *)malloc(sizeof(char) * (Read_cert.length() + 1));
  strcpy(pRead_cert, Read_cert.c_str());

  char *pRead_privatekey;
  pRead_privatekey = (char *)malloc(sizeof(char) * (Read_privatekey.length() + 1));
  strcpy(pRead_privatekey, Read_privatekey.c_str());

  Serial.println("==============================================");
  Serial.println("Certificados que pasan adjuntan al espClient");
  Serial.println();
  Serial.println("Root CA: ");
  Serial.write(pRead_rootca);
  Serial.println("==============================================");
  Serial.println();
  Serial.println("Cert: ");
  Serial.write(pRead_cert);
  Serial.println("==============================================");
  Serial.println();
  Serial.println("privateKey: ");
  Serial.write(pRead_privatekey);
  Serial.println("==============================================");

  espClient.setCACert(pRead_rootca);
  espClient.setCertificate(pRead_cert);
  espClient.setPrivateKey(pRead_privatekey);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  //*****************************

  WiFi.macAddress(mac);
  snprintf(mac_Id, sizeof(mac_Id), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(mac_Id);
  //*****************************
  delay(2000);
}
/*Sensor Uv*/
// Takes an average of readings on a given pin
// Returns the average
int averageAnalogRead(int pinToRead){
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;

  for (int x = 0; x < numberOfReadings; x++)
    runningValue += analogRead(pinToRead);
    runningValue /= numberOfReadings;

  return (runningValue);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

String alerta(float uvIntensity){
  String mensaje;
  if (uvIntensity >= 0 and uvIntensity <=2){
    III = "134";
    IV = "140";
    V = "...";
    mensaje = " Riesgo: BAJO";
  }
  if (uvIntensity > 2 and uvIntensity <= 5){
    III = "66";
    IV = "132";
    V = "140";
    mensaje = " Riesgo: MODERADO";
  }
  if (uvIntensity > 5 and uvIntensity <= 7){
    III = "40";
    IV = "88";
    V = "140";
    mensaje = " Riesgo: ALTO";
  }
  if (uvIntensity > 7 and uvIntensity <= 10){
    III = "30";
    IV = "63";
    V = "124";
    mensaje = " Riesgo: MUY ALTO";
  }
  if (uvIntensity > 11 ){
    III = "31";
    IV = "50";
    V = "94";
    mensaje = " Riesgo: EXTREMO";
  }
  return mensaje;
}



void loop(){
  /*Sensor Uv*/
  int uvLevel = averageAnalogRead(UVOUT);
  int refLevel = averageAnalogRead(REF_3V3);

  // Use the 3.3V power pin as a reference to get a very accurate output value from sensor
  float outputVoltage = 3.3 / refLevel * uvLevel;

  float uvIntensity = mapfloat(outputVoltage/3.0, 0.99, 2.8, 0.0, 15.0); // Convert the voltage to a UV intensity level
  
  /*Sensor Tem Hum*/
  // read humidity
  float humi = dht_sensor.readHumidity();
  // read temperature in Celsius
  float tempC = dht_sensor.readTemperature();
  // read temperature in Fahrenheit
  float tempF = dht_sensor.readTemperature(true);

  // check whether the reading is successful or not
  if (isnan(tempC) || isnan(tempF) || isnan(humi)){
    Serial.println("Failed to read from DHT sensor!");
  }
  else{
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print("%");

    Serial.print("  |  ");

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.print("°C  ~  ");
    Serial.print(tempF);
    Serial.println("°F");
  }

  /*Serial.print("output: ");
  Serial.print(refLevel);

  Serial.print("ML8511 output: ");
  Serial.print(uvLevel);

  Serial.print(" / ML8511 voltage: ");
  Serial.print(outputVoltage);

  Serial.print(" / UV Intensity (mW/cm^2): ");
  Serial.print(uvIntensity);*/

  /***********Mostrar en LCD******************/
  lcd.clear();
  lcd.print("Intencidad de UV");
  lcd.setCursor(0, 1);
  lcd.print(uvIntensity);
  lcd.print(" mW/cm^2");
  delay(2000);
  String mensaje = alerta(uvIntensity);
  lcd.clear();
  lcd.print("Intencidad de UV");
  lcd.setCursor(0, 1);
  lcd.print(mensaje);
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperatura");
  lcd.setCursor(0, 1);
  lcd.print(tempC);
  lcd.print(" C ");
  delay(500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Humedad");
  lcd.setCursor(0, 1);
  lcd.print(humi);
  lcd.print(" % ");

  Serial.println();

  Serial.println();

  // Comprobando conexion y enviando valores
  if (!client.connected())
  {
    reconect();
  }
  client.loop();
  long now = millis();
  if (now - lastMsg > 5000){
    lastMsg = now;
    //==============================================
    String macIdStr = mac_Id;
    String Temprature = String(tempC);
    String Humidity = String(humi);
    String Intensity = String(uvIntensity);
    String mensaje = alerta(uvIntensity);
    snprintf(msg, BUFFER_LEN, "{\"mac_Id\" : \"%s\", \"Temprature\" : %s, \"Humidity\" : \"%s\"}", macIdStr.c_str(), Temprature.c_str(), Humidity.c_str());
    Serial.print("Publish message: ");
    Serial.print(count);
    Serial.println(msg);
    client.publish("SensorTemHum", msg);
    Serial.println();

    snprintf(msg, BUFFER_LEN, "{\"mac_Id\" : \"%s\", \"Intensity\": \"%s\", \"mensaje\": \"%s\", \"III\": \"%s\", \"IV\": \"%s\" , \"V\": \"%s\"}", macIdStr.c_str(), Intensity.c_str(), mensaje.c_str(), III.c_str(), IV.c_str(), V.c_str());
    Serial.print("Publish message: ");
    Serial.print(count);
    Serial.println(msg);
    client.publish("sensorUV", msg);
    count = count + 1;
  }
  delay(500);
}