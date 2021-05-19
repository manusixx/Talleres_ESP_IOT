#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//Estudiante: Manuel Alejandro Pastrana Pardo - Materia IOT - Maestria en Ciencias de la computación
//Inicialización de variables
int lectura = 0;
float temperatura = 0.0;
const int temperaturapin = A0;
const int bombillopin = LED_BUILTIN;
boolean estadoventilador = false; //false = apagado
boolean estadobombillo = false;   //false = apagado
const unsigned long postingInterval = 5L * 1000L;  //Establece cada cuanto se envia a ThingSpeak
unsigned long lastConnectionTime = 0;               //Para controlar el tiempo de generar nueva medición
long lastUpdateTime = 0;  
int nummedicion = 0; 

//************************ Configurar MQTT ************************
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

//Parametros para los mensajes MQTT
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//Broquer MQTT
//const char* mqtt_server = "iot.eclipse.org";
const char * mqtt_server = "192.168.1.13";
//const char * ssid = "pi";
//const char * password = "mpastrana";

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 0 was received as first character
  if ((char)payload[0] == '0')
  {
    digitalWrite(bombillopin, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  }
  else
  {
    digitalWrite(bombillopin, HIGH); // Turn the LED off by making the voltage HIGH
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      if (temperatura > 0)
      {
        snprintf(msg, 75, "%f", temperatura);
        client.publish("temperaturaSalida", msg);
        Serial.println("enviando...");
        Serial.println(msg);
      }
      // ... and resubscribe
      client.subscribe("accionLed");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//********************* FIN Configurarción MQTT ************************

void setup()
{
  //----------- Comando para Conectarse a la WIFI el ESP8266 ---------
  WiFiManager wifiManager;
  // Descomentar para resetear configuración - Hacer el ejercicio con el celular
  // todas las veces.
  wifiManager.resetSettings();

  // Creamos AP y portal para configurar desde el Celular
  wifiManager.autoConnect("ESP8266Temp");
  Serial.println("!Ya estás conectado¡");
  //----------- Fin de conección ESP8266 -----------------------------
  
  //************* Inicializar Servidor MQTT *********************
  //Inicializar el canal MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  //************* FIN Inicializar Servidor MQTT *****************

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}

// Toma de lectura del sensor
int readSensor()
{
  lectura = analogRead(temperaturapin);
  Serial.println(lectura);
  return lectura;
}

//convertir a lectura analoga que va de 0 a 1023  a voltage que va de 0 a 3V
float calcTemperature(int lectura)
{
  float voltage = lectura * (3.3 / 1024.0);
  //Convertir a grados celcius
  temperatura = (voltage * 100) - 50;
  Serial.print("Temperatura sala: ");
  Serial.print(temperatura);
  Serial.print(" °C");
  return temperatura;
}

void tmpAlert(float temperatura)
{
  if (temperatura >= 35.05)
  {
    Serial.println("Alerta temperatura supera el umbral");
    digitalWrite(bombillopin, LOW);
    estadoventilador = true; //false = apagado
    Serial.println("Ventilador encendido");
    estadobombillo = true;   //false = apagado
    Serial.println("Bombillo encendido");
    Serial.println("========================================"); 
    delay(1000);
    
  }
  else
  {
    Serial.println("Temperatura normal");
    digitalWrite(bombillopin, HIGH);
    estadoventilador = false; //false = apagado
    Serial.println("Ventilador apagado");
    estadobombillo = false;   //false = apagado
    Serial.println("Bombillo apagado");
    Serial.println("========================================"); 
    delay(10);
  }
}

void loop()
{
   //*********** Conectarse al servidor MQTT ****************
//Intentarconectarse al servidor MQTT
   if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //*********** FIn conectarse al Servidor MQTT ************

  // Solamente actualiza si el tiempo de publicación es excedido
  if (millis() - lastUpdateTime >=  postingInterval) {
      lastUpdateTime = millis();

      //LeerSensores
       lectura = readSensor();
       temperatura = calcTemperature(lectura);
       tmpAlert(temperatura);
   
      //Imprimir Valores Sensores y Actuadores 
      Serial.print("=========== Medición No.: ");
      Serial.print(nummedicion++);
      Serial.println(" ============");
     
      //Enviar los datos al servidor MQTT
      //Publicar la temperatura
      snprintf (msg, 75, "%f", temperatura);
      Serial.print("Publicando temperatura en el Servidor MQTT: ");
      Serial.println(msg);
      client.publish("temperaturaSalida", msg);

      //Publicar el Estado del Ventilador
      snprintf (msg, 75, "%i", estadoventilador);
      Serial.print("Publicando el estado del ventilador en el Servidor MQTT: ");
      Serial.println(msg);
      client.publish("ventiladorSalida", msg);

       //Publicar el Estado del Bombillo
      snprintf (msg, 75, "%i", estadobombillo);
      Serial.print("Publicando el estado del bombillo en el Servidor MQTT: ");
      Serial.println(msg);
      client.publish("bombilloSalida", msg);
    }
}