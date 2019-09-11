#include "ThingSpeak.h"
#include<WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include<TwitterApi.h>


char ssid[] = "your ssid"  //SSID
char password[] = "your password"; //Password

unsigned long myChannelField = 848037; // Channel ID
const int ChannelField = 1; // Which To Field Write
const char * myWriteAPIKey = "your Thinngspeak API key"; // Write API Key
WiFiClient client;

#define TELEGRAM_BOT_TOKEN "Your boot tele account"; //initialize tele bot for message from
WiFiClientSecure clientTele, clientTwit;

UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, clientTele);

const int trigPin =2;
const int echoPin =5;
const int led1 = 14;
const int led2= 23;
const int led3= 22;
const int led4=21;
const int buzzer = 4;

//variavel for motor stepper
#define IN1  27
#define IN2  26
#define IN3  25
#define IN4  33
//variabel for distance
long duration;
int distance;
//variabel for motor stepper
int Steps = 0;
int k=0;
boolean Direction = true;
unsigned long last_time;
unsigned long currentMillis ;
int steps_left=4095;
long times;
String notif;

String chat_id;
int delayBetweenChecks = 1000; 
unsigned long lastTimeChecked;   //last time messages' scan has been done

unsigned long lightTimerExpires;
boolean lightTimerActive = false;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(10);
  ThingSpeak.begin(client);


  pinMode(trigPin,OUTPUT);
  pinMode(echoPin,INPUT);
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(led3,OUTPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(led4, OUTPUT);

  pinMode(IN1, OUTPUT); 
 pinMode(IN2, OUTPUT); 
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT); 
 
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  bot.longPoll = 60;
}



void handleNewMessages(int numNewMessages) {
    
  for (int i = 0; i < numNewMessages; i++) {
   
       
    // If the type is a "callback_query", a inline keyboard button was pressed
    if (bot.messages[i].type ==  F("callback_query")) {
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;
      Serial.print("Call back button pressed with text: ");
      Serial.println(text);

        Serial.print("nilai d msg ");
        Serial.println(k);
        if (text == F("ON")) {
        digitalWrite(led4, HIGH);
        
        //stepper on
        while(steps_left>0){
            currentMillis = micros();
            if(currentMillis-last_time>=1000){
            stepper(1); 
           

            times=times+micros()-last_time;
            last_time=micros();
            steps_left--;
            }
            }
             bot.sendMessage(chat_id, "Katup Terbuka", "");
            Serial.println("Katup Terbuka!");
            //Serial.println(times);
           
            
            delay(5000);
            Direction=!Direction;
            steps_left=4095;
            digitalWrite(led4, LOW);
            bot.sendMessage(chat_id, "Katup Sudah ditutup kembali", "");
            Serial.println("Katup Sudah ditutup kembali");
            
      } else if (text.startsWith("TIME")) {
        text.replace("TIME", "");
        int timeRequested = text.toInt();
        digitalWrite(led4, HIGH);
        digitalWrite(led3, LOW);
        lightTimerActive = true;
        lightTimerExpires = millis() + (timeRequested * 1000 * 60);
      }
    } else {
      String chat_id = String(bot.messages[i].chat_id);
      String text = bot.messages[i].text;

      if (text == F("/options")) {

        // Keyboard Json is an array of arrays.
        // The size of the main array is how many row options the keyboard has
        // The size of the sub arrays is how many coloums that row has

        // "The Text" property is what shows up in the keyboard
        // The "callback_data" property is the text that gets sent when pressed  
        
        String keyboardJson = F("[[{ \"text\" : \"ON\", \"callback_data\" : \"ON\" }]]");
        keyboardJson += F("[{ \"text\" : \"5 Mins\", \"callback_data\" : \"TIME5\" }, { \"text\" : \"10 Mins\", \"callback_data\" : \"TIME10\" }, { \"text\" : \"20 Mins\", \"callback_data\" : \"TIME20\" }]]");
        bot.sendMessageWithInlineKeyboard(chat_id, "Sadbhs Stars", "", keyboardJson);
      }

      // When a user first uses a bot they will send a "/start" command
      // So this is a good place to let the users know what commands are available
      if (text == F("/start")) {
        bot.sendMessage(chat_id, "/options : returns the inline keyboard\n", "Markdown");
      }
    }
      //}
      
    
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);

  duration=pulseIn(echoPin,HIGH);
  distance=duration*0.034/2;
  Serial.print("Distance: ");
  Serial.println(distance);

  //coding for stepper
  
  
  if (distance >=4 && distance<=6)
  {
    Serial.println("isi masih penuh");
    digitalWrite(led1,HIGH);
    digitalWrite(led2,LOW);
    digitalWrite(led3,LOW);
    digitalWrite(buzzer,LOW);
  }
  else if (distance>=7 && distance <=9){
    Serial.println("isi tinggal setengah");
    digitalWrite(led1,LOW);
    digitalWrite(led2,HIGH);
    digitalWrite(led3,LOW);
    digitalWrite(buzzer,LOW);
  }
  else if(distance>=10 && distance<=16)
  {
    Serial.println("isi tinggal sedikit\n SEGERA ISI!");
    digitalWrite(led1,LOW);
    digitalWrite(led2,LOW);
    digitalWrite(led3,HIGH);
    digitalWrite(buzzer,HIGH);
   
  }
  else{
    Serial.println("ERROR DILUAR JANGKAUAN");
  }


  Serial.print("Jarak saat ini (cm) : ");
  Serial.println(distance);
  Serial.println("Send data to Thingspeak ....");
  Serial.println("\n");

  
  ThingSpeak.writeField(myChannelField, ChannelField, distance, myWriteAPIKey);

  
  if (millis() > lastTimeChecked + delayBetweenChecks)  {
    
    // getUpdates returns 1 if there is a new message from Telegram
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    if (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
    }

    lastTimeChecked = millis();

    if (lightTimerActive && millis() > lightTimerExpires) {
      lightTimerActive = false;
      digitalWrite(led4, LOW);
    }
  }

  delay(1000);
}

void stepper(int xw){
  for (int x=0;x<xw;x++){
switch(Steps){
   case 0:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, HIGH);
   break; 
   case 1:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, HIGH);
   break; 
   case 2:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, LOW);
   break; 
   case 3:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, HIGH);
     digitalWrite(IN4, LOW);
   break; 
   case 4:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
   case 5:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, HIGH);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
     case 6:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
   case 7:
     digitalWrite(IN1, HIGH); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, HIGH);
   break; 
   default:
     digitalWrite(IN1, LOW); 
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
   break; 
}
SetDirection();
}
} 

void SetDirection(){
if(Direction==1){ Steps++;}
if(Direction==0){ Steps--; }
if(Steps>7){Steps=0;}
if(Steps<0){Steps=7; }
}
