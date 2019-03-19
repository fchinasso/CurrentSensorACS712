#include <Time.h>
#include <TimeLib.h>
#include <ArduinoJson.h>
#include <sensorCorrente.h>
#include <SoftwareSerial.h>

sensorCorrente s01(A3, T30A, 'A', '1'); //objets for each current sensor
sensorCorrente s02(A4, T30A, 'A', '2');
sensorCorrente s03(A5, T30A, 'A', '3');

char IdEquipamento[] = "fcc1"; //equipment ID
unsigned long timestamp;

int intervalo = 2; //interval between each measure (seconds)
float tempo = 0.5; //interval between each send (minutes)

SoftwareSerial mySerial(10, 11); // Serial comunication pins

//Lib of Strings for AT Commands
char s0[] = "ATE0";
char s1[] = "AT";
char s2[] = "AT+CSQ";
char s3[] = "AT+CSTT=\"smart.m2m.vivo.com.br\",\"vivo\",\"vivo\"";
char s4[] = "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80";
char s5[] = "AT+CIPSEND=90";
char s6[] = "GET /update?api_key=JLLRIEA9Y8VCK0II&field1=";
char s7[] = "AT+CIPCLOSE";
char s8[] = "AT+CIPSHUT";
char s9[] = "AT+CCLK?";

char j1[] = "AT+SAPBR=3,1,\"APN\",\"smart.m2m.vivo.com.br\"";
char j2[] = "AT+SAPBR=3,1,\"USER\",\"vivo\"";
char j3[] = "AT+SAPBR=3,1,\"PWD\",\"vivo\"";
char j4[] = "AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"";
char j5[] = "AT+SAPBR=1,1";
char j6[] = "AT+SAPBR=2,1";
char j7[] = "AT+SAPBR=0,1";

char h1[] = "AT+HTTPINIT";
char h2[] = "AT+HTTPPARA=\"CID\",1";
char h3[] = "AT+HTTPPARA=\"URL\",\"http://eletry.sumersoft.com:4443/endpoint/v1/metering\"";
char h4[] = "AT+HTTPPARA=\"CONTENT\",\"application/json\"";
char h5[] = "AT+HTTPDATA=";
char h6[] = "AT+HTTPACTION=1";
char h7[] = "AT+HTTPREAD";
char h8[] = "AT+HTTPTERM";
char h9[] = "AT+HTTPSSL=0";

char r1[] = "OK";
char r2[] = "CONNECT";
char r3[] = "CLOSE";
char r4[] = "DOWNLOAD";


void setup() {


  Serial.begin(9600);
  mySerial.begin(9600);     // Starts Serial Communication


  /*s01.calibrar();
    s02.calibrar();
    s03.calibrar();

    s01.printCalibrar();
    s02.printCalibrar();
    s03.printCalibrar();*/

  intervalo = intervalo * 1000; // converts intervals to milliseconds
  tempo = tempo * 1000 * 60;

  pinMode(9, OUTPUT);//GSM module Reset pin
  digitalWrite(9, LOW);
  resetGSM();
  delay(10000);

  //connects to GSM Network
  int check = gsmConnectRoutine();
  int count = 0;


  while (check == 0) {
    check = gsmConnectRoutine();
    count ++;
    if (count > 6) {

      resetGSM();
      count = 0;
      delay(10000);
    }
  }

}

void loop() {


  uint32_t inicio = millis();
  uint32_t timer = millis();

  int N = 0;
  bool flag = false;

  float I1 = 0;
  float I2 = 0;
  float I3 = 0;


  for (;;) {            //main routine

    if (millis() - timer >= intervalo ) {

      I1 += s01.medir(); //gets a measure for each current sensor on the predetermined interval
      I2 += s02.medir();
      I3 += s03.medir();

      timer = millis();
      N++;
    }
    if (millis() - inicio >= tempo) { //stars the routine for JSON packet sent
      I1 = I1 / N;
      I2 = I2 / N;
      I3 = I3 / N;

      HTTPpostRoutine(I1, I2, I3);

      /*flag = enviar(I1, I2, I3);
        if (flag == true)
        Serial.println(String("ENVIADO!"));
        else {
        Serial.println(String("***********ATENCAO********"));
        Serial.println(String("******Falha no envio******"));
        }*/


      I1 = 0;
      I2 = 0;
      I3 = 0;
      N = 0;

      flag = false;

      inicio = millis();
      timer = millis();

    }
  }
}

int gsmConnectRoutine() {

  int count = 0;
  int check = 0;

  //GSM  Network routine

  check = sendATcommand(j1, r1, 400, 0);// AT+SAPBR=3,1,"APN","smart.m2m.vivo.com.br"

  if (check == 0) {
    return 0;
  }

  delay(1000);


  check = sendATcommand(j2, r1, 400, 0);//AT+SAPBR=3,1,"USER","vivo"

  if (check == 0) {
    return 0;
  }

  delay(1000);


  check = sendATcommand(j3, r1, 400, 0); //AT+SAPBR=3,1,"PWD","vivo"

  if (check == 0) {
    return 0;
  }

  delay(1000);
  count = 0;

  check = sendATcommand(j4, r1, 400, 0); //AT+SAPBR=3,1,"CONTYPE","GPRS"

  if (check == 0) {
    return 0;
  }

  delay(1000);
  count = 0;

  while (sendATcommand(j5, r1, 5000, 0) == 0) //AT+SAPBR=1,1
  {
    count++;

    delay(3000);
    sendATcommand(j7, r1, 5000, 0);
    if (count > 2) {
      return 0;
    }
  }
  return 1;

}

int HTTPpostRoutine(float I1, float I2, float I3) {

   //HTTP post routine


  //GET timestamp
  String timestampJSON;
  String timestampS;
  sendATcommand(s9, r1, 500, 1);
  timestampS = String(timestamp);
  timestampJSON = timestampS + '0' + '0' + '0';

  //Serialize Json
  const size_t capacity = 2 * JSON_OBJECT_SIZE(3) + 4 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(7);
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.createObject();
  root["meterId"] = IdEquipamento;
  root["provider"] = "TRAJETO";
  root["epochTimestamp"] = timestampJSON;

  JsonObject& value = root.createNestedObject("value");

  JsonObject& value_voltage = value.createNestedObject("voltage");
  value_voltage["a"] = 125.8;
  value_voltage["b"] = 126.3;
  value_voltage["c"] = 127.1;

  JsonObject& value_current = value.createNestedObject("current");
  value_current["a"] = 0.035;
  value_current["b"] = 0.045;
  value_current["c"] = 0.031;

  JsonObject& value_power = value.createNestedObject("power");
  value_power["total"] = 0;
  value_power["a"] = 0;
  value_power["b"] = 0;
  value_power["c"] = 0;

  JsonObject& value_activePower = value.createNestedObject("activePower");
  value_activePower["total"] = 0;
  value_activePower["a"] = 0;
  value_activePower["b"] = 0;
  value_activePower["c"] = 0;

  JsonObject& value_reactivePower = value.createNestedObject("reactivePower");
  value_reactivePower["total"] = 0;
  value_reactivePower["a"] = 0;
  value_reactivePower["b"] = 0;
  value_reactivePower["c"] = 0;

  JsonObject& value_combinedActivePower = value.createNestedObject("combinedActivePower");
  value_combinedActivePower["total"] = 0;
  value_combinedActivePower["powerPoint"] = 0;
  value_combinedActivePower["powerPeak"] = 0;
  value_combinedActivePower["powerLevel"] = 0;
  value_combinedActivePower["valleyCharge"] = 0;
  value["lineFrequency"] = 60;


  value_current["a"] = I1;
  value_current["b"] = I2;
  value_current["c"] = I3;



  //GET size of Json
  int comprimentoJson = root.measureLength();
  char str[30];

  //update HTTPDATA with sizeof JSON
  sprintf(str, "AT+HTTPDATA=%d,2000", comprimentoJson);

//checks IP, with IP can't be confirmed, it reconnects via gsmConnectRoutine
  int a = sendATcommand(j6, r1, 200, 2);
  int count = 0;
  if (a == 0) {
    while (a == 0) {
      gsmConnectRoutine();
      a = sendATcommand(j6, r1, 200, 2);
      count++;
      if (count > 2) {
        resetGSM();
        count = 0;
        delay(10000);
      }
    }
  }

  //HTTP routine
  a = sendATcommand(h1, r1, 200, 0);//AT+HTTPINIT

  if (a == 0) {
    delay(1000);
    sendATcommand(h8, r1, 500, 0); //AT+HTTPTERM
    sendATcommand(h1, r1, 200, 0);//AT+HTTPINIT

  }
  delay(1000);
  sendATcommand(h9, r1, 200, 0); //AT+HTTPSSL=0
  delay(100);
  sendATcommand(h2, r1, 200, 0); //AT+HTTPPARA="CID",1
  delay(500);
  sendATcommand(h3, r1, 200, 0); //AT+HTTPPARA="URL","devsumersoft.dyndns.org:8580/endpoint/v1/metering"
  delay(1000);
  sendATcommand(h4, r1, 200, 0); //AT+HTTPPARA="CONTENT","application/json"
  delay(1000);
  sendATcommand(str, r4, 200, 0);;//AT+HTTPDATA=

  root.printTo(mySerial);


  delay(6000);
  //sendATcommand(h7, r4, 4000, 0);  //AT+HTTPREAD


  sendATcommand(h6, r1, 6000, 0); //AT+HTTPACTION=1


  delay(10000);

  a = sendATcommand(h8, r1, 2000, 0); //AT+HTTPTERM
  if (a == 0) {
    resetGSM();
  }








}

int RotinaGSM(char valor[]) {

  int count = 0;
  int check;


  check = sendATcommand(s8, r1, 200, 0); // CIPSHUT

  while (check == 0)
  {
    count++;
    check = sendATcommand(s8, r1, 200, 0);
    if (count > 4)
      return 0;
  }
  delay(2000);
  count = 0;


  check = sendATcommand(s3, r1, 200, 0); //AT+CSTT=\"smart.m2m.vivo.com.br\",\"vivo\",\"vivo\"
  while (check == 0)
  {
    count++;
    check = sendATcommand(s3, r1, 200, 0);
    if (count > 4)
      return 0;
  }
  delay(2000);
  count = 0;

  check = sendATcommand(s8, r1, 200, 0); // CIPSHUT
  while (check == 0)
  {
    count++;
    check = sendATcommand(s8, r1, 200, 0);
    if (count > 4)
      return 0;
  }
  delay(2000);
  count = 0;


  check = sendATcommand(s4, r2, 5000, 0); // AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80
  delay(2000);

  sendATcommand(s5, r1, 2000, 0);     // AT+CIPSEND=
  delay(2000);

  sendATcommand(valor, r1, 2000, 0);     //GET /update?api_key=9EZ3TG966X160QBN&field1=corrente
  delay(2000);
  count = 0;

  while (sendATcommand(s7, r3, 200, 0) == 0) //AT+CIPCLOSE
  {
    count++;
    sendATcommand(s7, r3, 200, 0);
    if (count > 4) {
      return 0 ;
    }
  }

  return 1;


}
//function that receives a command and checks if the response is expected
int8_t sendATcommand(char* ATcommand, char* expected_answer1, unsigned int timeout, int flag) {
  int i;
  uint8_t x = 0,  answer = 0;
  char response[100];
  unsigned long previous;

  memset(response, '\0', 100);    //initializes string

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

  //for timestamp calculation (only when required)
  if (flag == 1) {


    int mes = dateToDecimal(&response[22]);
    int dia = dateToDecimal(&response[25]);
    int hora = dateToDecimal(&response[28]);
    int minuto = dateToDecimal(&response[31]);
    int segundo = dateToDecimal(&response[34]);
    int ano = dateToDecimal(&response[19]);



    /*Serial.println(String("HORA:") + tm_hour);
      Serial.println(String("MINUTO:") + tm_min);
      Serial.println(String("SEGUNDO:") + tm_sec);
      Serial.println(String("DIA DO ANO:") + tm_yday);
      Serial.println(String("ANO:") + tm_year);
      Serial.println();
      delay(1000);*/

    setTime(hora+3, minuto, segundo, dia, mes, ano);

    timestamp = now();




  }
  //checks IP via at+sapbr=2,1
  if (flag == 2) {
    int i = 0;

    /* while (i < 50) {
       Serial.println(i + String("=") + response[i]);
       i++;
      }*/

    if (response[25] - '0' == 1) {
      return 1;
    }

    if (response[25] - '0' == 3)
      return 0;

  }
  return answer;

}

void resetGSM() //GSM reset
{
  Serial.println(String("Resetando....."));
  digitalWrite(9, 0);
  delay(2000);
  digitalWrite(9, 1);



}

bool enviar(float fase1, float fase2, float fase3) {

  int count = 0;
  int flag = 0;

  char f1[15];
  char f2[15];
  char f3[15];

  dtostrf(fase1, 7, 2, f1); //converte o float em char
  dtostrf(fase2, 7, 2, f2);
  dtostrf(fase2, 7, 2, f3);


  char * str = (char *) malloc(1 + strlen(s6) + 3 * strlen(f1) + 2 * strlen("&field2="));
  snprintf(str, 100, "%s%s%s%s%s%s", s6, f1, "&field2=", f2, "&field3=", f3);



  while (flag == 0) {//verifica se realizou

    if (flag == 0) {
      flag = RotinaGSM(str);
      count++;
    }
    if (count > 3) {
      free(str);
      resetGSM();
      return false;
    }
    if (flag == 1) {
      free(str);
      return true;
    }
  }
}
//converts an set of chars to int
int dateToDecimal(char * date) {
  int value = ((date[0] - '0') * 10 + (date[1] - '0'));
  return value;
}
