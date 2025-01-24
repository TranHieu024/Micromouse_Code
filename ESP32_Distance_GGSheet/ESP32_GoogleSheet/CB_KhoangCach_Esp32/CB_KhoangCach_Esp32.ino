unsigned long Time;
const int trig = 18; 
const int echo = 5; 
long thoigian; 
float khoangcach; 


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Time = millis();
  pinMode(trig, OUTPUT); //Chân trig xuất tín hiệu
  pinMode(echo, INPUT); //Chân echo nhận tín hiệu

  Serial.println("DO KHOANG CACH");
}
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

void loop() {
  // put your setup code here, to run once:
  float k = Distance();
  if ((millis() - Time) >= 1000)
  {
    Serial.println("Khoang cach la: " + String(k));
  }


}
