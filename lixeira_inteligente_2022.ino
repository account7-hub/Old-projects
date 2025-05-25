

//Biblioteca para controle do servo motor
#include <Servo.h>

//Bibliotecas para uso do display LCD
#include <Wire.h>
#include "rgb_lcd.h"

//Bibliotecas para uso da rede e post no Twitter
#include <SPI.h>
#include <Ethernet.h>
#include <Twitter.h>

//Biblioteca de tempo para aquisiçao de data e hora atuais
#include <time.h>


//Definicao dos pinos de ligacao dos sensores Ultrassom
//Ultrassom externo
#define echoUltraF 9
#define trigUltraF 8
//Ultrassom interno
#define echoUltraT 4
#define trigUltraT 5


//Declaracao servo motor
Servo myServo;


int distanciaFrente; //distancia medida pelo ultrassom externo
int distanciaLixo;   //distancia medida pelo ultrasssom interno
int pctLixo = 0;     //armazena porcentagem de lixo

bool tweeta = true;  //flag para controle de tweets

bool flagMusic = true; //flag para o controle do audio


//variaveis para obtencao da data e hora atuais
time_t t = time(NULL);
struct tm tm;

int melody[] = {660, 660, 660, 510, 660, 770, 380}; //notas do audio
int noteDurations[] = {100, 100, 100, 100, 100, 100, 100}; //duracao de cada nota
int pausadepoisdasnotas[] = {150, 300, 300, 100, 300, 550, 575}; //duracao de pausas


//Declaracao do display LCD I2C
rgb_lcd lcd;

void setup() {

  Serial.begin(9600);

  Serial.println("Inicializando...");
  lcd.begin(16, 2);
  initUltrasom(echoUltraF, trigUltraF);
  initUltrasom(echoUltraT, trigUltraT);
  initTampa();

  lcd.setRGB(0, 255, 0);
  lcd.print("@TrashIoT");
  lcd.setCursor(0, 1);
  lcd.print("#intelMAKER");

  Serial.println("Fim inicializacao...");


}

void loop() {

  distanciaLixo = getDistance(echoUltraT, trigUltraT);
  distanciaFrente = getDistance(echoUltraF, trigUltraF);
  Serial.print("Distancia ultrassom frente: ");
  Serial.print(distanciaFrente);
  Serial.print(" Distancia ultrassom tampa: ");
  Serial.println(distanciaLixo);
  Serial.println();

  if (distanciaFrente < 40)
  {
    abreTampa();
    flagMusic = true;
  }
  else
  {
    //
    if(flagMusic)
    {
      playMusic(2);
      flagMusic = false;
    }
    fechaTampa();
    if (distanciaLixo < 3)
      distanciaLixo = 3;
    if (distanciaLixo > 18)
      distanciaLixo = 18;

    pctLixo = (100 * distanciaLixo - 1800) / -15;

    Serial.print(pctLixo);
    Serial.println("% Ocupado");

    if (pctLixo == 100) {
      Serial.println("Lixeira Cheia");
      if (tweeta)
      {
        tm = *localtime(&t);
        char msg[100];
        sprintf(msg, "Lixeira cheia\nData: %04d %02d %02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tweeta = false;
        tweetMessage(msg);
        Serial.println("tweetando");
      }

    }
    else if (pctLixo == 0)
    {
      tweeta = true;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("@TrashIoT");
    lcd.setCursor(0, 1);
    lcd.print(pctLixo);
    lcd.setCursor(3, 1);
    lcd.print("% ocupada");

    lcd.setRGB((255 * pctLixo / 100), (255 - (255 * pctLixo / 100)), 0);

  }

  delay(500);

}

void initUltrasom(int echo, int trig)
{
  pinMode(echo, INPUT);
  pinMode(trig, OUTPUT);
}

//Funcao para obter distancia com o ultrassom
long getDistance(int echo, int trig)
{

  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH);
  long distancia = duration / 29 / 2 ;

  return distancia;
}

//Funcao para inicializacao do servo da tampa
void initTampa()
{
  myServo.attach(3);
}

//Funcao que realiza a abertura da tampa
void abreTampa()//int val=100)
{
  myServo.write(19);
}

//Funcao que realiza o fechmanto da tampa
void fechaTampa()//int val=19)
{
  myServo.write(100);
}

//Funcao responsavel pelo Tweet
void tweetMessage(char *msg) {
  Twitter twitter("4134070685-Roeq5HEFtF7zn69ask1k8tv05EOJu84tc4tJ6Ui");

  //Tweet that sucker!
  twitter.post(msg);
}


//Funcao para tocar a musica
void playMusic(int pin)
{
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(pin, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    //int pauseBetweenNotes = noteDuration * 1.30;

    delay(pausadepoisdasnotas[thisNote]);
    //delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(pin);
  }
}
