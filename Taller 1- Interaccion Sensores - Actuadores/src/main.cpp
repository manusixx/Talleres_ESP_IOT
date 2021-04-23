#include <Arduino.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>

// Información del Canal y Campos de ThingSpeak
char thingSpeakAddress[] = "api.thingspeak.com";
unsigned long channelID = 1368221;
char *readAPIKey = (char *)"KCKCJYUL35N5SUKK";
char *writeAPIKey = (char *)"2O5RBDM44AMWVU3Q";
const unsigned long postingInterval = 20L * 1000L;
unsigned int dataFieldOne = 1;   // Campo para escribir el estado de la Temperatura
unsigned int dataFieldTwo = 2;   // Campo para escribir el estado del Bombillo
unsigned int dataFieldThree = 3; // Campo para escribir el estado del Ventilador
unsigned int dataFieldFour = 4;  // Campo para escribir el Tiempo

//Inicialización de variables
int lectura = 0;
float temperatura = 0.0;
const int temperaturapin = A0;
const int bombillopin = LED_BUILTIN;
boolean estadoventilador = false; //false = apagado
boolean estadobombillo = false;   //false = apagado

//------------------------- Activar WIFI ESP8266 -----------------------
char ssid[] = "manusixx";
char password[] = "d4rkm4st3r";
WiFiClient client; //Cliente Wifi para ThingSpeak
//-------------------------- Fin Configuración WIFI ESP8266 --------------

void setup()
{
    //----------- Comando para Conectarse a la WIFI el ESP8266 ---------
    Serial.println("Conectandose a la WIFI!");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi conectada");
    Serial.println(WiFi.localIP());
    //----------- Fin de conección ESP8266 -----------------------------
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    //************ Conectar Cliente ThinkSpeak *******
    ThingSpeak.begin(client);
    //************ Fin Conectar Cliente ThingSpeak ***
}

// Toma de lectura del sensor
int readSensor()
{
    lectura = analogRead(temperaturapin);
    Serial.print("El valor de la lectura capturado es: ");
    Serial.println(lectura);
    return lectura;
}

//convertir a lectura analoga que va de 0 a 1023  a voltage que va de 0 a 3V
float calcTemperature(int lectura)
{
    float voltage = lectura * (3.3 / 1024.0);
    Serial.print("El valor del voltaje es: ");
    Serial.println(voltage);
    //Convertir a grados celcius
    temperatura = (voltage * 100) - 50;
    Serial.print("El valor en celcius es: ");
    Serial.println(temperatura);
    return temperatura;
}

void tmpAlert(float temperatura)
{
    if (temperatura >= 35.05)
    {
        Serial.println("Alerta temperatura mayor a 35.05 grados celcius");
        digitalWrite(bombillopin, LOW);
        estadoventilador = true; //false = apagado
        estadobombillo = true;   //false = apagado
        delay(1000);
    }
    else
    {
        Serial.println("Temperatura normal");
        digitalWrite(bombillopin, HIGH);
        estadoventilador = false; //false = apagado
        estadobombillo = false;   //false = apagado
        delay(10);
    }
}

// Use this function if you want to write a single field
int writeTSData(long TSChannel, unsigned int TSField, float data)
{
    int writeSuccess = ThingSpeak.writeField(TSChannel, TSField, data, writeAPIKey); // Write the data to the channel
    if (writeSuccess)
    {
        //lcd.setCursor(0, 1);
        //lcd.print("Send ThinkSpeak");
        Serial.println(String(data) + " written to Thingspeak.");
    }

    return writeSuccess;
}

//use this function if you want multiple fields simultaneously
int write2TSData(long TSChannel, unsigned int TSField1,
                 float field1Data, unsigned int TSField2, long field2Data,
                 unsigned int TSField3, long field3Data,
                 unsigned int TSField4, long field4Data)
{

    ThingSpeak.setField(TSField1, field1Data);
    ThingSpeak.setField(TSField2, field2Data);
    ThingSpeak.setField(TSField3, field3Data);
    ThingSpeak.setField(TSField4, field4Data);

    int printSuccess = ThingSpeak.writeFields(TSChannel, writeAPIKey);
    return printSuccess;
}

//metodo repetitivo
unsigned long lastConnectionTime = 0;
long lastUpdateTime = 0;

void loop()
{
    if (millis() - lastUpdateTime >= postingInterval)
    {
        lastUpdateTime = millis();
        lectura = readSensor();
        temperatura = calcTemperature(lectura);
        tmpAlert(temperatura);

        //Enviar los Datos a ThinkSpeak
        write2TSData(channelID, dataFieldOne, temperatura,
                     dataFieldTwo, estadobombillo, dataFieldThree, estadoventilador,
                     dataFieldFour, millis());
    }
    //  delay(1000);
}