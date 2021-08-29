#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "wiset-f358c-default-rtdb.firebaseio.com"  //firebase 아이디
#define FIREBASE_AUTH "uJ0wl5QTbb8N3zCt1uIojuPteFwLtoDOlV5mSVTS" //firebase 비밀번호
#define WIFI_SSID "SK_WiFiGIGA2E0B" //wifi ssid
#define WIFI_PASSWORD "1710011743"  //wifi 비밀번호

FirebaseData firebaseData;
FirebaseJson json;


//비접촉온도센서
#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//MPU6050(기울기 센서)
#include<Wire.h>
const int MPU_addr=0x68; 
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;


//사운드센서
int led = 12;
int threshold = 80; 
int volume;

//온습도센서
#include <DHT.h>
#include<LiquidCrystal.h>
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
int t;
int h;

void printResult(FirebaseData &data);
void setup(){

  //비접촉온도센서
   Serial.begin(9600);
   Serial.println("Adafruit MLX90614 test");  
   mlx.begin();  

  //MPU6050(기울기 센서)
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  
  Wire.write(0);     
  Wire.endTransmission(true);


  //온습도센서
   dht.begin();

  //사운드센서
  pinMode(led, OUTPUT);  
  
  //connect to wifi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println();
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
 
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
 
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setBSSLBufferSize(1024, 1024);
  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(1024);
  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}

void loop(){

  //비접촉온도센서
   Serial.print("센서 주변의 온도 = "); 
   Serial.print(mlx.readAmbientTempC()); 
   Serial.print("*C\t센서가 감지한 주변의 온도 = "); 
   Serial.print(mlx.readObjectTempC()); 
   Serial.println("*C");
   Serial.print("센서 주변의 온도 = ");
   Serial.print(mlx.readAmbientTempF()); 
   Serial.print("*F\t센서가 감지한 주변의 온도 = "); 
   Serial.print(mlx.readObjectTempF()); 
   Serial.println("*F");
   Serial.println();
   delay(100);

   //MPU6050(기울기 센서: 가속도 3축 + 자이로 2축 + 온도 1축)
   Wire.beginTransmission(MPU_addr);
   Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
   Wire.endTransmission(false);
   Wire.requestFrom(MPU_addr,14,true);  
   
   //AcX,AcY,AcZ = 가속도 센서
   AcX=Wire.read()<<8|Wire.read();  
   AcY=Wire.read()<<8|Wire.read();  
   AcZ=Wire.read()<<8|Wire.read();  

   //Tmp = 온도
   Tmp=Wire.read()<<8|Wire.read();  

   //GyX,GyY,GyZ = 자이로 센서(각속도 측정)
   GyX=Wire.read()<<8|Wire.read(); 
   GyY=Wire.read()<<8|Wire.read(); 
   GyZ=Wire.read()<<8|Wire.read();  
   
   Serial.print("가속도 AcX = "); Serial.print(AcX);
   Serial.print(" | 가속도 AcY = "); Serial.print(AcY);
   Serial.print(" | 가속도 AcZ = "); Serial.print(AcZ);
   Serial.print(" | 온도 Tmp = "); Serial.print(Tmp/340.00+36.53);  
   Serial.print(" | 각속도 GyX = "); Serial.print(GyX);
   Serial.print(" | 각속도 GyY = "); Serial.print(GyY);
   Serial.print(" | 각속도 GyZ = "); Serial.println(GyZ);
   delay(333);
   

  //사운드센서
   volume = analogRead(A1); 
   Serial.print("사운드 = ");
   Serial.println(volume);
   delay(100);
 
  if(volume>=threshold){
    digitalWrite(led, HIGH); 
  }  
  else{
    digitalWrite(led, LOW); 
  }

  
  //온습도센서
   t = dht.readTemperature();
   h = dht.readHumidity();
   Serial.print("온도= ");
   Serial.print(t);
   Serial.print(" °C");
   Serial.print(", 습도= ");
   Serial.print(h);
   Serial.println(" %");
   delay(100);

  Serial.println(t,h);
   if (isnan(h) || isnan(t)) {                                                // Check if any reads failed and exit early (to try again).
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  
  String fireTemp = String(t);
  String fireHumid = String(h); 
  Firebase.setString("/sensor/humy", fireHumid);                                  //setup path and send readings
  Firebase.setString("/sensor/temp", fireTemp);
  delay(1000);


 //오류시 
 if (Firebase.failed()) {
      Serial.print("pushing String failed:");
      Serial.println(Firebase.error());  
      return;
  }
   delay(3000);
}
 
