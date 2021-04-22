#include <Arduino.h>

/**
 * uso del sensor de temperatura
 * cuando el sensor detecta más de 10 grados enciende
 */

//Inicialización de variables
int lectura = 0;
float temperatura = 0.0;
const int temperaturapin = A0;
const int bombillopin = LED_BUILTIN;   

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
}

// Toma de lectura del sensor
int readSensor (){
    lectura = analogRead(temperaturapin);
    Serial.print("El valor de la lectura capturado es: " );
    Serial.println(lectura);
    return lectura;
}

//convertir a lectura analoga que va de 0 a 1023  a voltage que va de 0 a 3V
float calcTemperature(int lectura){   
    float voltage = lectura * (3.3 /1024.0) ;
    Serial.print("El valor del voltaje es: " );
    Serial.println(voltage);
    //Convertir a grados celcius
    temperatura = (voltage * 100)-50;
    Serial.print("El valor en celcius es: " );
    Serial.println(temperatura);
    return temperatura;
}

void tmpAlert(float temperatura){
 if (temperatura >= 35.05  ) {
        Serial.println("Alerta temperatura mayor a 35.05 grados celcius");
        digitalWrite(bombillopin, LOW);
 }
 else
 {
     Serial.println("Temperatura normal");
     digitalWrite(bombillopin, HIGH);
 }
}

void loop() {
    lectura = readSensor ();
    temperatura = calcTemperature(lectura);   
    tmpAlert(temperatura);   
    delay(1000);
}