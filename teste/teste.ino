#include <sensorCorrente.h>
#include "SoftwareSerial.h"


sensorCorrente s01(A3,T30A,'D','1'); //declarcao de objetos para cada fase
sensorCorrente s02(A4,T30A,'D','2');
sensorCorrente s03(A5,T30A,'D','3');




int intervalo = 2; //intervalo entre  cada medição
float tempo = 5; //tempo para calculo da média

SoftwareSerial mySerial(10, 11); // Declaramos os pinos RX(10) e TX(11) que vamos a usar na comunicacao Serial

char s0[] = "ATE0";
char s1[] = "AT";
char s2[] = "AT+CSQ";
char s3[] = "AT+CSTT=\"smart.m2m.vivo.com.br\",\"vivo\",\"vivo\"";
char s4[] = "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80";
char s5[] = "AT+CIPSEND=90";
char s6[] = "GET /update?api_key=JLLRIEA9Y8VCK0II&field1=";

char s7[] = "AT+CIPCLOSE";
char s8[] = "AT+CIPSHUT";

char r1[] = "OK";
char r2[] = "CONNECT";
char r3[] = "CLOSE";





void setup() {

pinMode(9, OUTPUT);//Pino de reset
digitalWrite(9, HIGH);
Serial.begin(9600);
mySerial.begin(9600);     // Iniciando comunicação serial
delay(1000);

/*s01.calibrar();
s02.calibrar();
s03.calibrar();

s01.printCalibrar();
s02.printCalibrar();
s03.printCalibrar();*/

intervalo=intervalo*1000;
tempo=tempo*1000*60;
reset();


}

void loop() {

uint32_t inicio = millis();
uint32_t timer = millis();

int N = 0;
bool flag = false;

float I1=0;
float I2=0;
float I3=0;


    while( millis() - inicio < tempo){

        if(millis() - timer >= intervalo ){

          I1+=s01.medir();
          I2+=s02.medir();
          I3+=s03.medir();

          timer = millis();
          N++;
        }

     }

      I1 = I1/N;
      I2 = I2/N;
      I3 = I3/N;


    flag = enviar(I1,I2,I3);
      if(flag==true)
        Serial.println(String("ENVIADO!"));
      else{
      Serial.println(String("***********ATENCAO********"));
      Serial.println(String("******Falha no envio******"));
}
}


bool enviar(float fase1,float fase2,float fase3) {

  int count = 0;
  bool flag = false;

  char f1[15];
  char f2[15];
  char f3[15];

  dtostrf(fase1, 7, 2,f1); //converte o float em char
  dtostrf(fase2, 7, 2,f2);
  dtostrf(fase2, 7, 2,f3);


  char * str = (char *) malloc(1 + strlen(s6) + 3*strlen(f1) + 2*strlen("&field2="));
  snprintf(str,100,"%s%s%s%s%s%s",s6,f1,"&field2=",f2,"&field3=",f3);



  while (flag == false) {//verifica se realizou

    if (flag == false) {
      flag = RotinaGSM(str);
      count++;
    }
    if (count > 3){
       free(str);
      return false;
    }
    if (flag == 1){
      free(str);
      return true;
    }
  }
}


int RotinaGSM(char valor[]) {

  int cont = 0;


  while (sendATcommand(s1, r1, 200) == 0)  // AT
  {
    sendATcommand(s1, r1, 200);
  }
  delay(1000);


  while (sendATcommand(s8, r1, 200) == 0)  // CIPSHUT
  {
    delay(1000);
    sendATcommand(s8, r1, 200);
  }
  delay(2000);

  while (sendATcommand(s3, r1, 200) == 0) //AT+CSTT=\"smart.m2m.vivo.com.br\",\"vivo\",\"vivo\"
  {
    delay(1000);
    sendATcommand(s3, r1, 200);
  }
  delay(2000);

  while (sendATcommand(s8, r1, 200) == 0)  // CIPSHUT
  {
    delay(1000);
    sendATcommand(s8, r1, 200);
  }
  delay(2000);

  while (sendATcommand(s4, r2, 5000) == 0) // AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80
  {
    delay(1000);
    sendATcommand(s4, r2, 5000);
    cont++;
    if (cont > 4){
    reset();
      return 0 ;
    }
  }
  cont = 0;
  delay(2000);

  sendATcommand(s5, r1, 2000);      // AT+CIPSEND=
  delay(2000);

  sendATcommand(valor, r1, 2000);      //GET /update?api_key=9EZ3TG966X160QBN&field1=corrente
  delay(2000);

  while (sendATcommand(s7, r3, 200) == 0) //AT+CIPCLOSE
  {
    sendATcommand(s7, r3, 200);
    cont++;
     if (cont > 4){
       reset();
      return 0 ;
    }
  }

  return 1;


}
int8_t sendATcommand(char* ATcommand, char* expected_answer1, unsigned int timeout) {
  int i;
  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    //Inicializa a string

  delay(100);

  while ( mySerial.available() > 0) mySerial.read();   // Clean the input buffer

  mySerial.println(ATcommand);


  x = 0;
  previous = millis();

  // this loop waits for the answer
  do {
    if (mySerial.available() != 0) {
      response[x] = mySerial.read();
      x++;
      // check if the desired answer is in the response of the module
      if (strstr(response, expected_answer1) != NULL)
      {
        answer = 1;
      }
    }
    // Waits for the asnwer with time out
  }
  while ((answer == 0) && ((millis() - previous) < timeout));
  Serial.println(response);
  return answer;
}
void reset() //Reinicia o GSM
  {
    digitalWrite(9, 0);
    delay(2000);
    digitalWrite(9, 1);

  }
