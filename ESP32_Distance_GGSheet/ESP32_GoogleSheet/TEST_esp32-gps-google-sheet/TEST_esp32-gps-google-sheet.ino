#include <WiFi.h>
#include <HTTPClient.h>
//----------------------------
//ENTER_GOOGLE_DEPLOYMENT_ID
const char * ssid = "Ca Map Biu";
const char * password = "7979797979";
String GOOGLE_SCRIPT_ID = "AKfycbwJFroE6Ihljzh8ZqEaaq2cmGDrWwg4qK2WmQWjtGAC0SIa6nEvA3xewTttiN4nIboh";
//---------------------------------------------------------------------

const int sendInterval = 2000;

void setup() 
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
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
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    newData = true;
  }

  //If newData is true
  if (newData == true)
  {
    newData = false;
    Serial.println("long1");
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
  bool nho = true;
  if (nho == true)
  {
    write_to_google_sheet("LONG");
  }
  else
  {
    Serial.println("No any valid GPS data.");
  }

}

/************************************************************************************
    loop function starts
 **********************************************************************************/
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
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
