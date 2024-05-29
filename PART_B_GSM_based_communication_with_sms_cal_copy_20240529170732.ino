#include <SoftwareSerial.h>

//sender phone number with country code
const String PHONE = "+919344281866";

String smsStatus,senderNumber,receivedDate,msg,buff;
boolean isReply = false;
String dtmf_cmd;
boolean is_call = false;

#define rxPin 2
#define txPin 3
SoftwareSerial sim800L(rxPin,txPin); 
 
#define relay6_pin 8
#define relay7_pin 9
#define relay8_pin 10
#define relay9_pin 11

boolean relay6_state = 1;
boolean relay7_state = 1;
boolean relay8_state = 1;
boolean relay9_state = 1;

void setup(){
 
  pinMode(relay6_pin, OUTPUT);
  pinMode(relay7_pin, OUTPUT);
  pinMode(relay8_pin, OUTPUT);
  pinMode(relay9_pin, OUTPUT);

 Serial.begin(115200);

 sim800L.begin(9600);
 sim800L.print("AT+CMGF=1\r");
 delay(1000);

  sim800L.println("AT+DDET=1"); //Enable DTMF
  delay(500);

  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";
   
}
void loop(){

  //**************
  while(sim800L.available()){
    buff = sim800L.readString();
    handle_sim800_response();
  }
  //**************
  while(Serial.available())  {
    sim800L.println(Serial.readString());
  }
  //**************

  listen_push_buttons();
}

void handle_sim800_response(){

  Serial.println(buff);
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    if(is_call == true){
      //HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
      if(int index = buff.indexOf("+DTMF:") > -1 ){
        index = buff.indexOf(":");
        dtmf_cmd = buff.substring(index+1, buff.length());
        dtmf_cmd.trim();
        Serial.println("dtmf_cmd: "+dtmf_cmd);

        if(dtmf_cmd == "6") control_relay(6);
        else if(dtmf_cmd == "7") control_relay(7);
        else if(dtmf_cmd == "8") control_relay(8);
        else if(dtmf_cmd == "9") control_relay(9);

      }    
      //HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
      if(buff.indexOf("NO CARRIER") > -1){
        sim800L.println("ATH");
        is_call = false;
      }
      //HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
    }
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    else if(buff.indexOf("RING") > -1)
    {
      delay(2000);
      sim800L.println("ATA");
      is_call = true;
    }
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    else if(buff.indexOf("+CMTI") > -1)
    {
        //get newly arrived sms memory location and store it in temp
        unsigned int index = buff.indexOf(",");
        String temp = buff.substring(index+1, buff.length()); 
        temp = "AT+CMGR=" + temp + "\r"; 
        //get the message stored at memory location "temp"
        sim800L.println(temp);
    }
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM         
    else if(buff.indexOf("+CMGR") > -1){
      extractSms();
      
      if(msg == "6") control_relay(6);
      else if(msg == "7") control_relay(7);
      else if(msg == "8") control_relay(8);
      else if(msg == "9") control_relay(9);

      else if(msg.indexOf(".status") > -1) send_relay_status(msg.substring(0,1));
      else if(msg.indexOf("del all") > -1) delete_all_sms();
    }
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM 
}

void extractSms(){
     unsigned int len, index;
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    //Remove sent "AT Command" from the response string.
    index = buff.indexOf("\r");
    buff.remove(0, index+2);
    buff.trim();
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    //Remove response "AT Command" from the response string.
    index = buff.indexOf(":");
    buff.substring(0, index);
    buff.remove(0, index+2);
   //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    index = buff.indexOf(",");
    smsStatus = buff.substring(1, index-1); 
    buff.remove(0, index+2);
    
    senderNumber = buff.substring(0, 13);
    buff.remove(0,19);
   
    receivedDate = buff.substring(0, 20);
    buff.remove(0,buff.indexOf("\r"));
    buff.trim();
    
    index =buff.indexOf("\n\r");
    buff = buff.substring(0, index);
    buff.trim();
    msg = buff;
    buff = "";
    msg.toLowerCase();
}

void listen_push_buttons(){
    // if(digitalRead(button1_pin) == LOW){
    //   delay(200);
    //   control_relay(1);
    // }
    // else if (digitalRead(button2_pin) == LOW){
    //   delay(200);
    //   control_relay(2);
    // }
    // else if (digitalRead(button3_pin) == LOW){
    //   delay(200);
    //   control_relay(3);
    // }
    // else if (digitalRead(button4_pin) == LOW){
    //   delay(200);
    //   control_relay(4);
    // }
      if (digitalRead(button5_pin) == LOW){
      delay(200);
      control_relay(5);
    }
     else if (digitalRead(button6_pin) == LOW){
      delay(200);
      control_relay(6);
    }
     else if (digitalRead(button7_pin) == LOW){
      delay(200);
      control_relay(7);
    }
     else if (digitalRead(button8_pin) == LOW){
      delay(200);
      control_relay(8);
    }
    
}
void control_relay(int relay){
  //------------------------------------------------
  if(relay == 1){
    relay1_state = !relay1_state;
    digitalWrite(relay1_pin, relay1_state);
    Serial.print("RelayState = ");
    Serial.println(relay1_state);
    delay(50);
    relay1_state = !relay1_state;
    digitalWrite(relay1_pin, relay1_state);
  }
  //------------------------------------------------
  // else if(relay == 2){
  //   relay2_state = !relay2_state;
  //   digitalWrite(relay2_pin, relay2_state);
  //   delay(50);
  //   relay2_state = !relay2_state;
  //   digitalWrite(relay2_pin, relay2_state);
  // }
  // //------------------------------------------------
  // else if(relay == 3){
  //   relay3_state = !relay3_state;
  //   digitalWrite(relay3_pin, relay3_state);
  //   delay(50);
  // }
  // //------------------------------------------------
  // else if(relay == 4){
  //   relay4_state = !relay4_state;
  //   digitalWrite(relay4_pin, relay4_state);
  //   delay(50);
  // }
  // //------------------------------------------------
  //  else if(relay == 5){
  //   relay5_state = !relay5_state;
  //   digitalWrite(relay5_pin, relay5_state);
  //   delay(50);
  // }
  // //------------------------------------------------
  else if(relay == 6){
    relay6_state = !relay6_state;
    digitalWrite(relay6_pin, relay6_state);
    delay(50);
  }
  //------------------------------------------------
  else if(relay == 7){
    relay7_state = !relay7_state;
    digitalWrite(relay7_pin, relay7_state);
    delay(50);
  }
  //------------------------------------------------
  else if(relay == 8){
    relay8_state = !relay8_state;
    digitalWrite(relay8_pin, relay8_state);
    delay(50);
  }
  //------------------------------------------------
  else if(relay == 9){
    relay9_state = !relay9_state;
    digitalWrite(relay9_pin, relay9_state);
    delay(50);
  }
  //------------------------------------------------
    
}

void send_relay_status(String relay)
{
  Serial.println("Relay Number: "+relay);
  //(relay1_state):"ON"?"OFF"
  String sms_text = "";
  //------------------------------------------------
  if(relay == "1"){
    sms_text = (relay1_state) ? "ON" : "OFF";
    sms_text = "Relay 1 is " + sms_text;
  }
  //------------------------------------------------
  // else if(relay == "2"){
  //   sms_text = (relay2_state) ? "ON" : "OFF";
  //   sms_text = "Relay 2 is " + sms_text;
  // }
  // //------------------------------------------------
  // else if(relay == "3"){
  //   sms_text = (relay3_state) ? "ON" : "OFF";
  //   sms_text = "Relay 3 is " + sms_text;
  // }
  // //------------------------------------------------
  // else if(relay == "4"){
  //   sms_text = (relay4_state) ? "ON" : "OFF";
  //   sms_text = "Relay 4 is " + sms_text;
  // }
  // //------------------------------------------------  
  // else if(relay == "5"){
  //   sms_text = (relay5_state) ? "ON" : "OFF";
  //   sms_text = "Relay 5 is " + sms_text;
  // }
  //------------------------------------------------  
  else if(relay == "6"){
    sms_text = (relay6_state) ? "ON" : "OFF";
    sms_text = "Relay 6 is " + sms_text;
  }
  //------------------------------------------------  
  else if(relay == "7"){
    sms_text = (relay7_state) ? "ON" : "OFF";
    sms_text = "Relay 7 is " + sms_text;
  }
  //------------------------------------------------  
  else if(relay == "8"){
    sms_text = (relay8_state) ? "ON" : "OFF";
    sms_text = "Relay 8 is " + sms_text;
  }
  //------------------------------------------------  
  else if(relay == "9"){
    sms_text = (relay9_state) ? "ON" : "OFF";
    sms_text = "Relay 9 is " + sms_text;
  }
  //------------------------------------------------  
    
  Reply(sms_text);
}

void delete_all_sms()
{
  sim800L.println("AT+CMGD=1,4");
  delay(5000);
  while(sim800L.available()){
    String response = sim800L.readString();
    if(response.indexOf("OK") > -1 ){
      Reply("All sms are deleted");
    } else {
      Reply(response);
    }
  }
}

void Reply(String text)
{
  Serial.println(text);
    sim800L.print("AT+CMGF=1\r");
    delay(1000);
    sim800L.print("AT+CMGS=\""+PHONE+"\"\r");
    delay(1000);
    sim800L.print(text);
    delay(100);
    sim800L.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(1000);
    Serial.println("SMS Sent Successfully.");

    msg = "";
}