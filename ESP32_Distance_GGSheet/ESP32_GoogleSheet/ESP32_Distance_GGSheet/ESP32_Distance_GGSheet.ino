#include <WiFi.h>
#include <HTTPClient.h>
//----------------------------
//ENTER_GOOGLE_DEPLOYMENT_ID
const char * ssid = "Ca Map Biu";
const char * password = "7979797979";
String GOOGLE_SCRIPT_ID = "AKfycbxAAXgiCfUXMR13y-fEbMVL3spZSzlPXuEYaH0yGN9zMywRPi2H4zDg-g8z_RUMoBi9";
//----------------Khai bao cam bien
const int trig = 18; 
const int echo = 5; 
long thoigian; 
unsigned long Time;
float khoangcach; 

const int sendInterval = 2000;

void setup()
{
  Serial.begin(115200);
  Time = millis();
  pinMode(trig, OUTPUT); //Chân trig xuất tín hiệu
  pinMode(echo, INPUT); //Chân echo nhận tín hiệu
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("CONNECTED !!!");
}


/****************loop function starts**********************/
void loop()
{

  boolean newData = false;
  if(millis() - Time >= 100)
  {
    Time = millis();
    if(Distance() != 0)
    {
      newData = true;
    }
  }

  //If newData is true
  if (newData == true)
  {
    newData = false;
    print_speed();
  }
  else
  {
    Serial.println("No new data is received.");
  }
  //delay(sendInterval);
}


void print_speed()
{
  float kc = Distance() ;
  if ( kc != 0)
  {
    Serial.println(kc);
    
    String param;
    param  = "latitude="+String(kc);
    
    Serial.println(param);
    write_to_google_sheet(param); 
  }
  else
  {
    Serial.println("No data.");
  }

}
/************Khoang cach********************/
float Distance()
{
  digitalWrite(trig, 0); //Tắt chân trig
  delayMicroseconds(2);
  digitalWrite(trig, 1); //bật chân trig để phát xung
  delayMicroseconds(10); //Xung có độ rộng là 10 microsecond
  digitalWrite(trig, 0);

  thoigian = pulseIn (echo, HIGH);

  khoangcach = int(thoigian / 2 / 29.412);

  return khoangcach;

}

/******/
void write_to_google_sheet(String params) {
  HTTPClient http;
  String url = "https://script.google.com/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + params;
  //Serial.print(url);
  Serial.println("Postring GPS data to Google Sheet");
  //---------------------------------------------------------------------
  //starts posting data to google sheet
  http.begin(url.c_str());
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);
  //---------------------------------------------------------------------
  //getting response from google sheet
  String payload;
  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("Payload: " + payload);
  }
  //---------------------------------------------------------------------
  http.end();
}
