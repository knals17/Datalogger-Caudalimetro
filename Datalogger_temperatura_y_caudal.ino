#include <Wire.h>
#include <SPI.h>

#include "RTClib.h"
#include <SD.h>
#include <math.h>

File Archivo;
//-------------------------Caudalimetro--------------------------------------
volatile int NumPulsos; //variable para la cantidad de pulsos recibidos
volatile int NumPulsos2;
int tiempo_Lectura = 500;
int PinSensor = 2;    //Sensor conectado en el pin 2
int PinSensor2 = 3;    //Sensor conectado en el pin 2

float factor_conversion = 11;  //7.5; //para convertir de frecuencia a caudal

float Q1;
float Q2;
float volumen1 = 0;
float volumen2 = 0;
//-------------------------------NTC-----------------------------------------
const int Rc = 10000; //valor de la resistencia
const int Vcc = 5;

float A = 1.11492089e-3;
float B = 2.372075385e-4;
float C = 6.954079529e-8;

float temperatura_C1;
float temperatura_C2;

float K = 2.5; //factor de disipacion en mW/C
//-------------------------------Presion-------------------------------------
const int sensorPre = A2;
float presion_Mpa;

//---------------------------Otros-------------------------------------------

byte diferenteSegundo;
byte sdStatus;


//--------------------variables para fecha y hora----------------------------
int hora;
int minutos;
int segundos;
int dia;
int mes;
int ano;

const int ledr   = 3;
const int sensor = 0;
const int SDCARD = 4;
const int ledV = 8;
const int ledR = 9;

long milivolts;
long temp;
byte nuevo = 0;

RTC_DS1307 RTC;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Comenzando la comunicación con la tarjeta SD");
  pinMode(4, OUTPUT);                     //Definimos el pin donde esta conectado el SS(slave Select) del modulo SD como una salida. (si no lo hacemos el lector SD del modulo Ethernet Shield no funcionara).
  pinMode(ledV, OUTPUT);
  pinMode(ledR, OUTPUT);
  //digitalWrite(4, HIGH);                  // Poniendo este pin en alto se desactiva el lector SD del ethernet shield.
  //digitalWrite(10, HIGH);
  pinMode(6, OUTPUT);

  if (!SD.begin(SDCARD)) {
    Serial.println("Se ha producido un fallo al iniciar la comunicación");
    digitalWrite(ledR, HIGH);
    sdStatus = 1;
    return;
  } else {
    Serial.println("Se ha iniciado la comunicación correctamente");
    digitalWrite(ledV, HIGH);
    Wire.begin();
    RTC.begin();
    //RTC.adjust(DateTime(__DATE__, __TIME__));
    // pinMode(ledr,OUTPUT);
  }

  if (SD.exists("datos.csv") == false ) {
     nuevo =1;
  }

  Archivo = SD.open("datos.csv", FILE_WRITE); //FILE_WRITE

  //Se escribe información en el documento de texto datos.csv.
  if (Archivo) {
    Serial.println("SD OK!");
    digitalWrite (6, HIGH);
    delay(500);
    digitalWrite (6, LOW);
    delay(500);
    Archivo.close();
  }
  //En caso de que haya habido problemas abriendo datos.csv, se muestra por pantalla.
  else {
    Serial.println("ERROR AL ABRIR EL ARCHIVO");
  }
  attachInterrupt(0, ContarPulsos, RISING); //(Interrupcion 0(Pin2),funcion,Flanco de subida)
  attachInterrupt(1, ContarPulsos2, RISING); //(Interrupcion 0(Pin2),funcion,Flanco de subida)

}

void loop() {
  if (sdStatus == 0) {
    float frecuencia = ObtenerFrecuencia() * 2; //obtenemos la Frecuencia de los pulsos en Hz
    float frecuencia2 = ObtenerFrecuencia2() * 2; //obtenemos la Frecuencia de los pulsos en Hz


    Q1 = (frecuencia / factor_conversion) * 0.0167; //Conversion de frecuencia a L/seg
    Q2 = (frecuencia2 / factor_conversion) * 0.0167; //Conversion de frecuencia a L/seg

    volumen1 += Q1;
    volumen2 += Q2;

    temperatura_C1 = lecturaNTC(0); //Lectura de Sonda NTC 1
    temperatura_C2 = lecturaNTC(1); //Lectura de Sonda NTC 2

    presion_Mpa = lecturaPresion(); //Lectura de Sensor de Presion

    serialOut(frecuencia, frecuencia2, Q1, Q2, temperatura_C1, temperatura_C2, presion_Mpa, volumen1, volumen2); //Funcion de Muestreo de datos por Monitor serie

    //---------GUARDADO DE DATOS-------------------------------
    if (diferenteSegundo != segundos ) {
      diferenteSegundo = segundos;
      //Se comprueba que el archivo se ha abierto correctamente y se procede a escribir en él.
      Archivo = SD.open("datos.csv", FILE_WRITE); //FILE_WRITE

      //Se escribe información en el documento de texto datos.csv.
      if (Archivo) {
        if (nuevo == 1) {
          Serial.println("Iniciado Archivo!");
          Archivo.print("Fecha");
          Archivo.print(";");

          Archivo.print("Hora");
          Archivo.print(";");

          Archivo.print("L/s (1)");
          Archivo.print(";");

          Archivo.print("L/s (2)");
          Archivo.print(";");

          Archivo.print("Temp (1)");
          Archivo.print(";");

          Archivo.print("Temp (2)");
          Archivo.print(";");

          Archivo.print("Presion");
          Archivo.print(";");

          Archivo.print("Volumen (1); Volumen (2) ");
          Archivo.print(";");
         
          Archivo.println();

          nuevo = 0;
        }
        Archivo.print(ano);
        Archivo.print("/");
        Archivo.print(mes);
        Archivo.print("/");
        Archivo.print(dia);
        Archivo.print(";");

        Archivo.print(hora);
        Archivo.print(":");
        Archivo.print(minutos);
        Archivo.print(":");
        Archivo.print(segundos);
        Archivo.print(";");

        Archivo.print(Q1);
        Archivo.print(";");
        Archivo.print(Q2);
        Archivo.print(";");
        
        Archivo.print(temperatura_C1);
        Archivo.print(";");
        
        Archivo.print(temperatura_C2);
        Archivo.print(";");
        
        Archivo.print(presion_Mpa);
        Archivo.print(";");

        Archivo.print(volumen1);
        Archivo.print(";");
        Archivo.print(volumen2);
        Archivo.print(";");

        Archivo.println();

        Archivo.close(); //Se cierra el archivo para almacenar los datos.

        digitalWrite (6, HIGH);
        delay(10);
        digitalWrite (6, LOW);
      }
      //En caso de que haya habido problemas abriendo datos.csv, se muestra por pantalla.
      else {
        Serial.println("El archivo datos.csv no se abrió correctamente");
      }
    }
  } else {
    digitalWrite(ledR, HIGH);
    delay(500);
    digitalWrite(ledR, LOW);
    delay(500);
    Serial.println("ERROR: SD no encontrada");
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
  delay(tiempo_Lectura);   //muestra de 1 segundo
  //noInterrupts(); //Desabilitamos las interrupciones
  frecuencia = NumPulsos; //Hz(pulsos por segundo)
  return frecuencia;
}

int ObtenerFrecuencia2()
{
  int frecuencia2;
  NumPulsos2 = 0;   //Ponemos a 0 el número de pulsos
  //interrupts();    //Habilitamos las interrupciones
  delay(tiempo_Lectura);   //muestra de 1 segundo
  //noInterrupts(); //Desabilitamos las interrupciones
  frecuencia2 = NumPulsos2; //Hz(pulsos por segundo)
  return frecuencia2;
}

float lecturaNTC(byte sensor) {
  float raw = analogRead(sensor);
  float V =  raw / 1024 * Vcc;

  float R = (Rc * V ) / (Vcc - V);


  float logR  = log(R);
  float R_th = 1.0 / (A + B * logR + C * logR * logR * logR );

  float kelvin = R_th - V * V / (K * R) * 1000;
  float celsius = kelvin - 273.15;

  return celsius;
}

float lecturaPresion() {
  int raw = analogRead(sensorPre);
  float Mpa = map(raw, 102, 921, 0, 1200);
  Mpa = Mpa / 1000;

  return Mpa;
}

float serialOut(float hz, float hz2, float lts1, float lts2, float t1, float t2, float p, float v1, float v2) {
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
  Serial.print(" ");

  Serial.print ("   F1: ");
  Serial.print (hz);
  Serial.print ("Hz   F2: ");
  Serial.print (hz2);
  Serial.print ("Hz   ");

  Serial.print ("Q1: ");
  Serial.print (lts1);
  Serial.print ("Lts   Q2: ");
  Serial.print (lts2);
  Serial.print ("Lts   ");

  Serial.print("T1: ");
  Serial.print(t1);
  Serial.print("C° ");

  Serial.print("T2: ");
  Serial.print(t2);
  Serial.print("C° ");

  Serial.print("P: ");
  Serial.print(p);
  Serial.print("Mpa ");

  Serial.print("V1: ");
  Serial.print(v1);
  Serial.print("lts ");

  Serial.print("V2: ");
  Serial.print(v2);
  Serial.print("lts\n");
}

