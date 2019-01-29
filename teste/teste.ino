#include <ArduinoJson.h>
#include <sensorCorrente.h>
#include <SoftwareSerial.h>

sensorCorrente s01(A3, T30A, 'D', '1'); //objetcs for each phase to be measured
sensorCorrente s02(A4, T30A, 'D', '2');
sensorCorrente s03(A5, T30A, 'D', '3');


long timestamp;
int intervalo = 2; //interval between each measure (seconds)
float tempo = 0.3; //interval between each send (minutes)

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
char h3[] = "AT+HTTPPARA=\"URL\",\"devsumersoft.dyndns.org:8580/endpoint/v1/metering\"";
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

  intervalo = intervalo * 1000;
  tempo = tempo * 1000 * 60;

  pinMode(9, OUTPUT);//GSM module Reset pin
  digitalWrite(9, HIGH);
  resetGSM();
  delay(5000);

 //connects to GSM Network
  int check = gsmConnectRoutine();


  while (check == 0) {
    delay(3000);
    resetGSM();
    delay(3000);
    check = gsmConnectRoutine();
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

//main loop, measures and then send the values
  for (;;) {

    if (millis() - timer >= intervalo ) {

      I1 += s01.medir();
      I2 += s02.medir();
      I3 += s03.medir();

      timer = millis();
      N++;
    }
    if (millis() - inicio >= tempo) {
      I1 = I1 / N;
      I2 = I2 / N;
      I3 = I3 / N;

 //after the elapsed time the measures are sent
      HTTPpostRoutine(I1, I2, I3);

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

//Routine for GSM Network connection
int gsmConnectRoutine() {

  int count = 0;

  //GSM routine

  while (sendATcommand(j1, r1, 400, 0) == 0) // AT+SAPBR=3,1,"APN","smart.m2m.vivo.com.br"
  {
    sendATcommand(j1, r1, 400, 0);
    count ++;
    if (count > 4)
      return 0;
  }
  delay(1000);
  count = 0;

  while (sendATcommand(j2, r1, 400, 0) == 0) //AT+SAPBR=3,1,"USER","vivo"
  {
    count ++;
    sendATcommand(j1, r1, 400, 0);
    if (count > 4)
      return 0;
  }
  delay(1000);
  count = 0;

  while (sendATcommand(j3, r1, 2400, 0) == 0) //AT+SAPBR=3,1,"PWD","vivo"
  {
    count++;
    sendATcommand(j3, r1, 400, 0);
    if (count > 4)
      return 0;
  }
  delay(1000);
  count = 0;

  while (sendATcommand(j4, r1, 400, 0) == 0) //AT+SAPBR=3,1,"CONTYPE","GPRS"
  {
    count++;
    sendATcommand(j4, r1, 400, 0);
    if (count > 5)
      return 0;
  }
  delay(1000);
  count = 0;

  while (sendATcommand(j5, r1, 6000, 0) == 0) //AT+SAPBR=1,1
  {
    count++;

    sendATcommand(j7, r1, 6000, 0);
    if (count > 2) {
      return 0;
    }
  }
  return 1;

}

//routine for HTTP Post method
void HTTPpostRoutine(float I1, float I2, float I3) {

  //Initializes JSON to be sent


  const size_t capacity = 2 * JSON_OBJECT_SIZE(3) + 4 * JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(7);
  DynamicJsonBuffer jsonBuffer(capacity);

  JsonObject& root = jsonBuffer.createObject();
  root["meterId"] = "lulalibre";
  root["provider"] = "TRAJETO";
  root["epochTimestamp"] = "1546617761492";

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

//updates Json with the measured values
  value_current["a"] = I1;
  value_current["b"] = I2;
  value_current["c"] = I3;


  //GET timestamp
  sendATcommand(s9, r1, 500, 1);
  root["epochTimestamp"] = timestamp;

  //GET size of Json
  int comprimentoJson = root.measureLength();
  char str[30];

  //update HTTPDATA with sizeof JSON
  sprintf(str, "AT+HTTPDATA=%d,2000", comprimentoJson);
  Serial.println(String("***************") + timestamp + String("*************************"));



  //HTTP routine
  sendATcommand(h1, r1, 200, 0); //AT+HTTPINIT
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
  sendATcommand(h6, r1, 200, 0); //AT+HTTPACTION=1
  delay(8000);
  sendATcommand(h8, r1, 500, 0);

}

int RotinaGSM(char valor[]) {

  int cont = 0;


  while (sendATcommand(s1, r1, 200, 0) == 0) // AT
  {
    sendATcommand(s1, r1, 200, 0);
  }
  delay(1000);


  while (sendATcommand(s8, r1, 200, 0) == 0) // CIPSHUT
  {
    delay(1000);
    sendATcommand(s8, r1, 200, 0);
  }
  delay(2000);

  while (sendATcommand(s3, r1, 200, 0) == 0) //AT+CSTT=\"smart.m2m.vivo.com.br\",\"vivo\",\"vivo\"
  {
    delay(1000);
    sendATcommand(s3, r1, 200, 0);
  }
  delay(2000);

  while (sendATcommand(s8, r1, 200, 0) == 0) // CIPSHUT
  {
    delay(1000);
    sendATcommand(s8, r1, 200, 0);
  }
  delay(2000);

  while (sendATcommand(s4, r2, 5000, 0) == 0) // AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80
  {
    delay(1000);
    sendATcommand(s4, r2, 5000, 0);
    cont++;
    if (cont > 4) {
      resetGSM();
      return 0 ;
    }
  }
  cont = 0;
  delay(2000);

  sendATcommand(s5, r1, 2000, 0);     // AT+CIPSEND=
  delay(2000);

  sendATcommand(valor, r1, 2000, 0);     //GET /update?api_key=9EZ3TG966X160QBN&field1=corrente
  delay(2000);

  while (sendATcommand(s7, r3, 200, 0) == 0) //AT+CIPCLOSE
  {
    sendATcommand(s7, r3, 200, 0);
    cont++;
    if (cont > 4) {
      resetGSM();
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

  //for timestamp calculation (only when required)
  if (flag == 1) {

    int tm_year = (dateToDecimal(&response[19]) + 2000) - 1900;
    int mes = dateToDecimal(&response[22]);
    int dia = dateToDecimal(&response[25]);
    int tm_hour = dateToDecimal(&response[28]);
    int tm_min = dateToDecimal(&response[31]);
    int tm_sec = dateToDecimal(&response[34]);
    int tm_yday;


    switch (mes) {

      case 1:
        tm_yday = dia;
        break;
      case 2:
        tm_yday = 31 + dia;
        break;
      case 3:
        tm_yday = 60 + dia;
        break;
      case 4:
        tm_yday = 91 + dia;
        break;
      case 5:
        tm_yday = 121 + dia;
        break;
      case 6:
        tm_yday = 152 + dia;
        break;
      case 7:
        tm_yday = 182 + dia;
        break;
      case 8:
        tm_yday = 213 + dia;
        break;
      case 9:
        tm_yday = 244 + dia;
        break;
      case 10:
        tm_yday = 274 + dia;
        break;
      case 11:
        tm_yday = 305 + dia;
        break;
      case 12:
        tm_yday = 335 + dia;
        break;
    }
    /*  Serial.println(String("HORA:") + tm_hour);
      Serial.println(String("MINUTO:") + tm_min);
      Serial.println(String("SEGUNDO:") + tm_sec);
      Serial.println(String("DIA DO ANO:") + tm_yday);
      Serial.println();
      delay(1000);*/

    timestamp = tm_sec + tm_min * 60 + tm_hour * 3600 + tm_yday * 86400 +
                (tm_year - 70) * 31536000 + ((tm_year - 69) / 4) * 86400 -
                ((tm_year - 1) / 100) * 86400 + ((tm_year + 299) / 400) * 86400;

    // Serial.println(timestamp);
  }
  return answer;


}
//GSM module reset
void resetGSM()
{
  digitalWrite(9, 0);
  delay(2000);
  digitalWrite(9, 1);

}

bool enviar(float fase1, float fase2, float fase3) {

  int count = 0;
  bool flag = false;

  char f1[15];
  char f2[15];
  char f3[15];

  dtostrf(fase1, 7, 2, f1); //converte o float em char
  dtostrf(fase2, 7, 2, f2);
  dtostrf(fase2, 7, 2, f3);


  char * str = (char *) malloc(1 + strlen(s6) + 3 * strlen(f1) + 2 * strlen("&field2="));
  snprintf(str, 100, "%s%s%s%s%s%s", s6, f1, "&field2=", f2, "&field3=", f3);



  while (flag == false) {//verifica se realizou

    if (flag == false) {
      flag = RotinaGSM(str);
      count++;
    }
    if (count > 3) {
      free(str);
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
