
/*
  Acceso con huella dactilar

A. UNO    SENSOR
  5V        V+
  2         TX
  3         RX
  GND       GND

Uno       =   A4 (SDA), A5 (SCL)
Mega2560  =   20 (SDA), 21 (SCL)
*/
 
#include <Adafruit_Fingerprint.h>    // Libreria  para el Sensor de huella dactilar
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
LiquidCrystal_I2C lcd(0x27,20,4);  // configura el LCD para mostrar 16 caracteres en 2 lineas
#include <Keypad.h> //Incluimos la libreria Keypad
const byte FILAS = 4;          // define numero de filas
const byte COLUMNAS = 4;       // define numero de columnas
char keys[FILAS][COLUMNAS] = {    // define la distribucion de teclas
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pinesFilas[FILAS] = {9,8,7,6};         // pines correspondientes a las filas
byte pinesColumnas[COLUMNAS] = {5,4,3,2};  // pines correspondientes a las columnas
Keypad teclado = Keypad(makeKeymap(keys), pinesFilas, pinesColumnas, FILAS, COLUMNAS);  // crea objeto teclado
char TECLA;                        // almacena la tecla presionada
char CLAVE[5];                     // almacena en un array 4 digitos ingresados
char CLAVE_MAESTRA[5] = "1234";    // almacena en un array la contraseña inicial
byte INDICE = 0;                   // indice del array

//Declaración de variables
int estado = 0;                  // 0=cerrado 1=abierto
int relay = 13;
int relayVCC = 31;
int buzzer = 36;
int LCD = 43;
int character = 0;
int activated =0;
char Received = 'c';
char Str[16] = {' ', ' ', ' ', ' ', ' ', ' ', '-', '*', '*', '*', ' ', ' ', ' ', ' ', ' ', ' '};  

#define bluetooth Serial1.

SoftwareSerial mySerial(10, 11);     // Crear Serial para Sensor  Rx, TX del Arduino
// SoftwareSerial portTwo(50, 51);  Crear Serial para Bluetooth  Rx, TX del Arduino
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);  // Crea el objeto Finger comunicacion pin 2 , 3 

void setup(){
  pinMode(relay, OUTPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(relayVCC,OUTPUT);
  pinMode (LCD,OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(3, OUTPUT);
  digitalWrite(relayVCC,HIGH);
  digitalWrite(9, HIGH);
  digitalWrite(LCD, HIGH); 
  lcd.init();// Inicializar el LCD 
  // Mostrar mensaje en el LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("INGRESA HUELLA O");
  lcd.setCursor(0,1);
  lcd.print("LA CLAVE:       ");
  
  Serial.begin(9600); //Inicia la comunicación serial con  el sensor de huellas.
  Serial1.begin(9600); //Inicia la comunicación serial con el bluetooth.
 
  while (!Serial); 
  delay(100);
  Serial.println("Sistema de apertura con huella dactilar");

  finger.begin(57600);  // inicializa comunicacion con sensor a 57600 Baudios
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Detectado un sensor de huella!");
  }else {
    Serial.println("No hay comunicacion con el sensor de huella");
    Serial.println("Revise las conexiones");
    while (1) { delay(1); }
  }
  
  finger.getTemplateCount();
  Serial.print("El sensor contiene "); Serial.print(finger.templateCount); Serial.println(" plantillas");
  Serial.println("Esperando por una huella valida...");
  limpia();

}

void loop(){
  digitalWrite(LCD, HIGH); 

  getFingerprintIDez();
  delay(500);            //retardo de 50 milisegundos entre lecturas

  
  
  TECLA = teclado.getKey();   // obtiene tecla presionada y asigna a variable 
  
  if (TECLA)                  // comprueba que se haya presionado una tecla
  {
    CLAVE[INDICE] = TECLA;    // almacena en array la tecla presionada

    digitalWrite(buzzer, HIGH);
    delay(50);
    digitalWrite(buzzer, LOW);     // apaga el buzzer
    INDICE++;                 // incrementa indice en uno
    lcd.print("*");         // envia al LCD la tecla presionada
      } 
  if(INDICE == 4)             // si ya se almacenaron los 4 digitos
  {
    lcd.clear();            //Limpia el LCD
    if(!strcmp(CLAVE, CLAVE_MAESTRA))    // compara clave ingresada con clave maestra
        abierto();              
    else 
        error();
    INDICE = 0;
  } 
  ///////////////Android OPEN/CLOSE/////////  
  if(Serial1.available()>0)
 { 
    char Received = Serial1.read();

    if (Received == 'o')
    {
      abierto();
      limpia();
    }
 }
}

void abrirPuerta() {
  Serial.println("*** AUTORIZADA *** "); 
  digitalWrite(buzzer,HIGH); //Suena el buzzer
  delay (200);
  digitalWrite(buzzer, LOW); //Apaga el buzzer
  digitalWrite(relay, HIGH);   // Abrir la cerradura
  delay(5000);// Tiempo de la puerta abierta
  digitalWrite(buzzer,HIGH); //Suena el buzzer
  delay (200);
  digitalWrite(buzzer, LOW); //Apaga el buzzer
  digitalWrite(relay, LOW);     // cierra la cerradura
  lcd.clear();
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) { 

    return -1;
  }

  // Si hay coincidencias de huella
  Serial.print("ID #"); Serial.print(finger.fingerID); 
  Serial.print(" coincidencia del "); Serial.println(finger.confidence);
   if(finger.fingerID==1){

     Serial.print("BIENVENIDO MARCOS *** "); 
     abierto();
 
   }else{
   Serial.print("ERORR *** ");}

   
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("INGRESA HUELLA O");
  lcd.setCursor(0,1);
  lcd.print("LA CLAVE: ");
   
  return finger.fingerID; 
}


void error(){   
      lcd.setCursor(0,1);
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);     // apaga el buzzer
      delay(150); 
      digitalWrite(buzzer, HIGH);
      delay(200);
      digitalWrite(buzzer, LOW);     // apaga el buzzer
      delay(150); 
      digitalWrite(buzzer, HIGH);
      delay(250);
      digitalWrite(buzzer, LOW);     // apaga el buzzer
      delay(150); 
      lcd.print("ERROR DE CLAVE    "); 
      delay(1500);
      lcd.clear();
      lcd.print("ACCESO DENEGADO    "); 
      delay(3000);
      limpia();
      
}

void abierto(){  
   if(estado==0){ 
      estado=1;
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Abierto            ");  // imprime en el LCD que esta abierta
      Serial.println(" AUTORIZADA *** "); 
      digitalWrite(buzzer, HIGH);   // Enciende el buzzer 
      delay(200);
      digitalWrite(relay, HIGH);   // Abrir la cerradura
      digitalWrite(buzzer, LOW);     // apaga el buzzer 
      delay(5000);             // Tiempo de la puerta abierta
      digitalWrite(buzzer, HIGH);   // Enciende el buzzer 
      delay (200);
      digitalWrite(buzzer, LOW);     // apaga el buzzer 
      digitalWrite(relay, LOW);     // cierra la cerradura 
      lcd.backlight();
      lcd.setCursor(0,0);
      lcd.print("Acceso        ");
      lcd.setCursor(0,1);
      lcd.print("Autorizado    ");
      delay(5000);
      lcd.clear();
      estado=0; 
   }
   else{
    estado=0;      
   } 
 limpia();
}

///////////////////// limpia //////////////////////////////
void limpia(){  
 lcd.clear();
 lcd.setCursor(0,0);
 lcd.print("INGRESA HUELLA O      ");
 lcd.setCursor(0,1);
 lcd.print("CLAVE: ");
}
