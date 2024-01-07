// #define MOTOR 23
// #define DELAY_TIME 5000
#define PIN1 16
#define PIN2 17
#define PIN7 5

#include <WiFi.h>
#include <MQTT.h>
#include <RTClib.h>
#include <LCD_I2C.h>

LCD_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;



char daysOfWeek[7][12] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

const char ssid[] = "@JumboPlusIoT";
const char pass[] = "nice123456";

const char mqtt_broker[]="test.mosquitto.org";
const char mqtt_topic[]="group_6/command";
const char mqtt_client_id[]="arduino_group_6"; // must change this string to a unique value
int MQTT_PORT=1883;

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;
bool isReverse = false;
bool isSpin = false;
unsigned long long int Time=0;

void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect(mqtt_client_id)) {  
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnected!");

  client.subscribe(mqtt_topic);
  // client.unsubscribe("/hello");
}
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
  if(payload == "on"){
    digitalWrite(PIN1,HIGH);
    client.publish("group_6/log", "Motor is on.");
    
    if(!isSpin){
      isSpin = true;
      client.publish("group_6/command", "reverse");
      client.publish("group_6/command", "reverse");
    }
    
  }else if(payload == "reverse"){
    if(isReverse){
      digitalWrite(PIN2,HIGH);
      digitalWrite(PIN7,LOW);
      //payload = "";
      isReverse = !isReverse;
    }else{
      digitalWrite(PIN2,LOW);
      digitalWrite(PIN7,HIGH);
      //payload = "";
      isReverse = !isReverse;
    }
  }else if(payload == "off"){
    digitalWrite(PIN1,LOW);
    client.publish("group_6/log", "Motor is off.");
    isSpin = false;
  }

}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.begin(ssid, pass);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported
  // by Arduino. You need to set the IP address directly.

  if (! rtc.begin()) {
    Serial.println("RTC module is NOT found");
    Serial.flush();
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  lcd.begin();
  lcd.backlight();
  
  pinMode(PIN1,OUTPUT);
  pinMode(PIN2,OUTPUT);
  pinMode(PIN7,OUTPUT);
  
  
  
  client.begin(mqtt_broker, MQTT_PORT, net); 
  client.onMessage(messageReceived);
  digitalWrite(PIN1,LOW);
  digitalWrite(PIN2,LOW);
  digitalWrite(PIN7,LOW);
 
connect();
}

void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }
  if(isSpin){
    if(isReverse && Time%1000==0){
    lcd.clear();
    lcd.print("Backward");
    lcd.setCursor(0,1);
    lcd.print("Duration :");
    lcd.print(Time/1000);
    }else if(!isReverse && Time%1000==0){
    lcd.clear();
    lcd.print("Forward");
    lcd.setCursor(0,1);
    lcd.print("Duration :");
    lcd.print(Time/1000);
    }
  }else{
    lcd.clear();
    lcd.print("Stop");
    Time=0;
  }
  
  Time+=10;
  
}
