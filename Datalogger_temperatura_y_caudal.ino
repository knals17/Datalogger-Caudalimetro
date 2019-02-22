#include <Wire.h>
#include <SPI.h>

#include "RTClib.h"
#include <SD.h>
File Archivo;

volatile int NumPulsos; //variable para la cantidad de pulsos recibidos
volatile int NumPulsos2;
int PinSensor = 2;    //Sensor conectado en el pin 2
int PinSensor2 = 3;    //Sensor conectado en el pin 2
float factor_conversion=7.5; //para convertir de frecuencia a caudal

byte diferenteSegundo;



// variables para fecha y hora
int hora;
int minutos;
int segundos;
int dia;
int mes;
int ano;

const int ledr   = 3;
const int sensor = 0;
const int SDCARD = 4;
long milivolts;
long temp;

RTC_DS1307 RTC;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Comenzando la comunicación con la tarjeta SD");
  pinMode(4, OUTPUT);                     //Definimos el pin donde esta conectado el SS(slave Select) del modulo SD como una salida. (si no lo hacemos el lector SD del modulo Ethernet Shield no funcionara).
  //pinMode(10, OUTPUT);                    //Definimos el pin donde esta conectado el SS(slave Select) del modulo ethernet como una salida. (si no lo hacemos el Ethernet Shield no funcionara).
  //digitalWrite(4, HIGH);                  // Poniendo este pin en alto se desactiva el lector SD del ethernet shield.
  //digitalWrite(10, HIGH);
  pinMode(6,OUTPUT);

  

  if (!SD.begin(SDCARD)) {
    Serial.println("Se ha producido un fallo al iniciar la comunicación");
    digitalWrite(5, HIGH);
    return;
  }
  Serial.println("Se ha iniciado la comunicación correctamente");
  Wire.begin();
  RTC.begin();
  //RTC.adjust(DateTime(__DATE__, __TIME__));
  // pinMode(ledr,OUTPUT);

  Archivo = SD.open("datos.txt", FILE_WRITE); //FILE_WRITE

  //Se escribe información en el documento de texto datos.txt.
  if (Archivo) {
    Serial.println("SD OK!");
    digitalWrite (6, HIGH);
    delay(500);
    digitalWrite (6, LOW);
    delay(500);
    Archivo.close();
  }
  //En caso de que haya habido problemas abriendo datos.txt, se muestra por pantalla.
  else {
    Serial.println("ERROR AL ABRIR EL ARCHIVO");
  }
  attachInterrupt(0,ContarPulsos,RISING); //(Interrupcion 0(Pin2),funcion,Flanco de subida)
  attachInterrupt(1,ContarPulsos2,RISING); //(Interrupcion 0(Pin2),funcion,Flanco de subida)

}

void loop() {
 

  DateTime now = RTC.now();

  Serial.print(now.year(), DEC);
  ano = now.year();
  Serial.print('/');
  Serial.print(now.month(), DEC);
  mes = now.month();
  Serial.print('/');
  Serial.print(now.day(), DEC);
  dia = now.day();
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  hora = now.hour();
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  minutos = now.minute();
  Serial.print(':');
  Serial.print(now.second(), DEC);
  segundos = now.second();
  Serial.println(" ");

   float frecuencia=ObtenerFrecuencia() * 2; //obtenemos la Frecuencia de los pulsos en Hz
  float frecuencia2=ObtenerFrecuencia2() * 2; //obtenemos la Frecuencia de los pulsos en Hz

  
  Serial.print ("FrecuenciaPulsos: "); 
  Serial.print (frecuencia,0); 
  Serial.print ("Hz FrecuenciaPulsos2: "); 
  Serial.print (frecuencia2,0); 
  Serial.println ("Hz"); 
  
  //---------GUARDADO DE DATOS-------------------------------
  if (diferenteSegundo != segundos ) { //segundos == 0 || segundos == 10 || segundos == 20 || segundos == 30 || segundos == 40  || segundos == 50) {
    diferenteSegundo = segundos;
    //Se comprueba que el archivo se ha abierto correctamente y se procede a escribir en él.
    Archivo = SD.open("datos.txt", FILE_WRITE); //FILE_WRITE

    //Se escribe información en el documento de texto datos.txt.
    if (Archivo) {
      Archivo.print(ano);
      Archivo.print("/");
      Archivo.print(mes);
      Archivo.print("/");
      Archivo.print(dia);
      Archivo.print(",");
      
      Archivo.print(hora);
      Archivo.print(":");
      Archivo.print(minutos);
      Archivo.print(":");
      Archivo.print(segundos);
      Archivo.print(",");
      
      Archivo.print(frecuencia);
      Archivo.print(",");
      Archivo.print(frecuencia2);
      
      Archivo.println();
      //Se cierra el archivo para almacenar los datos.
      Archivo.close();
      //Se muestra por el monitor que los datos se han almacenado correctamente.
      Serial.println("Todos los datos fueron almacenados");
      digitalWrite (6, HIGH);
      delay(10);
      digitalWrite (6, LOW);
    }
    //En caso de que haya habido problemas abriendo datos.txt, se muestra por pantalla.
    else {
      Serial.println("El archivo datos.txt no se abrió correctamente");
    }
  }
}

//---Función que se ejecuta en interrupción---------------
void ContarPulsos ()
{ 
  NumPulsos++;  //incrementamos la variable de pulsos
} 

void ContarPulsos2 ()
{ 
  NumPulsos2++;  //incrementamos la variable de pulsos
} 

//---Función para obtener frecuencia de los pulsos--------
int ObtenerFrecuencia() 
{
  int frecuencia;
  NumPulsos = 0;   //Ponemos a 0 el número de pulsos
  //interrupts();    //Habilitamos las interrupciones
  delay(500);   //muestra de 1 segundo
  //noInterrupts(); //Desabilitamos las interrupciones
  frecuencia=NumPulsos; //Hz(pulsos por segundo)
  return frecuencia;
}

int ObtenerFrecuencia2() 
{
  int frecuencia2;
  NumPulsos2 = 0;   //Ponemos a 0 el número de pulsos
  //interrupts();    //Habilitamos las interrupciones
  delay(500);   //muestra de 1 segundo
  //noInterrupts(); //Desabilitamos las interrupciones
  frecuencia2=NumPulsos2; //Hz(pulsos por segundo)
  return frecuencia2;
}

