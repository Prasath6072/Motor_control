#include <SoftwareSerial.h>

//sender phone number with country code
const String PHONE = "+919344281866";
//const String PHONE2 = "+919715179920";

String smsStatus,senderNumber,receivedDate,msg,buff;
boolean isReply = false;
String dtmf_cmd;
boolean is_call = false;

#define rxPin 12
#define txPin 13
SoftwareSerial sim800L(rxPin,txPin); 

#define button1_pin 7
#define button2_pin 6
#define button3_pin 5
#define button4_pin 4

#define relay1_pin 8
#define relay2_pin 9
#define relay3_pin 10
#define relay4_pin 11
#define relay5_pin 3

#define R_phase_pin A1
#define Y_phase_pin A2
#define B_phase_pin A3
#define motor1_pin A4
#define compressor1_pin A5
#define water_level_pin A6

boolean relay1_state = 0;
boolean relay2_state = 0;
boolean relay3_state = 0;
boolean relay4_state = 0;
boolean relay5_state = 0;

void setup()
{
  ///////////////////////////////////////pin for push buttion /////////////////////////
  pinMode(button1_pin, INPUT_PULLUP);
  pinMode(button2_pin, INPUT_PULLUP);
  pinMode(button3_pin, INPUT_PULLUP);
  pinMode(button4_pin, INPUT_PULLUP);
////////////////////////////////////////////PIN FOR R PHASE,Y PHASE,B PHASE ////////////////
  pinMode(R_phase_pin, INPUT_PULLUP);
  pinMode(Y_phase_pin, INPUT_PULLUP);
  pinMode(B_phase_pin, INPUT_PULLUP);
///////////////////////////////////////////PIN FOR MOTOR STATE ////////////////
  pinMode(motor1_pin, INPUT_PULLUP);
  pinMode(compressor1_pin, INPUT_PULLUP);

  pinMode(water_level_pin, INPUT_PULLUP);
    
  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);
  pinMode(relay3_pin, OUTPUT);
  pinMode(relay4_pin, OUTPUT);
  pinMode(relay5_pin, OUTPUT);

  digitalWrite(relay1_pin, HIGH);
  digitalWrite(relay2_pin, HIGH);
  digitalWrite(relay3_pin, HIGH);
  digitalWrite(relay4_pin, HIGH);
  digitalWrite(relay5_pin, LOW);

 Serial.begin(115200);
 sim800L.begin(9600);

 sim800L.print("AT+CMGF=1\r");
 delay(1000);

  sim800L.println("AT+DDET=1"); //Enable DTMF
  delay(500);
  Serial.println("Device geat ready");

  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";

}

void loop()
{
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

  // listen_ir();
   
  listen_push_buttons();

}

void handle_sim800_response()
{
  Serial.println(buff);
    //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    if(is_call == true){
      //HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
      if(int index = buff.indexOf("+DTMF:") > -1 ){
        index = buff.indexOf(":");
        dtmf_cmd = buff.substring(index+1, buff.length());
        dtmf_cmd.trim();
        Serial.println("dtmf_cmd: "+dtmf_cmd);

        if(dtmf_cmd == "1")      control_relay(1);
        else if(dtmf_cmd == "2") control_relay(2);
        else if(dtmf_cmd == "3") control_relay(3);
        else if(dtmf_cmd == "4") control_relay(4);
        else if(dtmf_cmd == "5") control_relay(5);
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
     
      if(msg == "motor1on")      control_relay(1);
      else if(msg == "motor1off") control_relay(2);

      else if(msg == "compressor1on") control_relay(3);
      else if(msg == "compressor1off") control_relay(4);
      else if(msg == "light") control_relay(5);
      
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void listen_push_buttons(){

     if(digitalRead(button1_pin) == LOW){
        Serial.println(relay4_state);
        digitalWrite(relay4_pin,LOW);
        delay (100);
        digitalWrite(relay4_pin,HIGH);
        delay(100);

        Serial.println(relay1_state);
        digitalWrite(relay1_pin,LOW);
        delay (100);
        digitalWrite(relay1_pin,HIGH);
        delay(100);
    }
   else if(digitalRead(button2_pin) == LOW){
     
        Serial.println(relay2_state);
        digitalWrite(relay2_pin,LOW);
        delay (100);
        digitalWrite(relay2_pin,HIGH);
        delay(100);
    }
    else if(digitalRead(button3_pin) == LOW){
        Serial.println(relay2_state);
        digitalWrite(relay2_pin,LOW);
        delay (100);
        digitalWrite(relay2_pin,HIGH);
        delay(100);

        Serial.println(relay3_state);
        digitalWrite(relay3_pin,LOW);
        delay (100);
        digitalWrite(relay3_pin,HIGH);
        delay(100);
    }
    else if (digitalRead(button4_pin) == LOW){
        Serial.println(relay4_state);
        digitalWrite(relay4_pin,LOW);
        delay (100);
        digitalWrite(relay4_pin,HIGH);
        delay(100);
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void control_relay(int relay){
  //------------------------------------------------
   String sms_text = "";
  if(relay == 1){
     if(digitalRead(R_phase_pin) && digitalRead(Y_phase_pin) && digitalRead(B_phase_pin) == HIGH){
    delay(10);
    Serial.println(relay4_state);
    digitalWrite(relay4_pin,LOW);
    delay (100);
    digitalWrite(relay4_pin,HIGH);
    delay(50);

    Serial.println(relay1_state);
    digitalWrite(relay1_pin,LOW);
    delay (100);
    digitalWrite(relay1_pin,HIGH);
    delay(50);
  
     sms_text = "MOTOR 1= ON COMPRESSOR 1= OFF";///////////////////CODE FOR REPLAY MESSAGE ///////////////////
  }
  else{
     sms_text = "NO 3 PHASE SUPPLY";
  }
  }

  //------------------------------------------------

   if(relay == 2){
  
    Serial.println(relay2_state);
    digitalWrite(relay2_pin,LOW);
    delay (100);
    digitalWrite(relay2_pin,HIGH);
    delay(50);
    sms_text = "MOTOR 1= OFF";
  }
  //------------------------------------------------
   if(relay == 3){
  
    if(digitalRead(R_phase_pin) && digitalRead(Y_phase_pin) && digitalRead(B_phase_pin) == HIGH){
    delay(10);
    Serial.println(relay2_state);
    digitalWrite(relay2_pin,LOW);
    delay (100);
    digitalWrite(relay2_pin,HIGH);
    delay(50);

    Serial.println(relay3_state);
    digitalWrite(relay3_pin,LOW);
    delay (100);
    digitalWrite(relay3_pin,HIGH);
    delay(50);

    sms_text = "COMPRESSOR 1= ON  MOTOR 1= OFF ";
    }
    else{
     sms_text = "NO 3 PHASE SUPPLY";
  } 
  }
  //------------------------------------------------

  if(relay == 4){
 

    Serial.println(relay4_state);
    digitalWrite(relay4_pin,LOW);
    delay (100);
    digitalWrite(relay4_pin,HIGH);
    delay(50);
    sms_text = "COMPRESSOR 1= OFF";
  }
  //------------------------------------------------
  else if(relay == 5){
    relay5_state = !relay5_state;
    digitalWrite(relay5_pin, relay5_state);
    Serial.print("RelayState = ");
    delay(50);
    sms_text = (relay5_state) ? "OFF" : "ON";
    sms_text = "LIGHT is " + sms_text;

  }
  //------------------------------------------------

   Reply(sms_text);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void send_relay_status(String relay)
{
  Serial.println("Relay Number: "+relay);
  //(relay1_state):"ON"?"OFF"
  String sms_text = "";
  //------------------------------------------------
   if(relay == "0"){ 
    if(digitalRead(R_phase_pin) && digitalRead(Y_phase_pin) && digitalRead(B_phase_pin) == HIGH){
    delay(10);
    sms_text = "HAVING 3 PHASE SUPPLY";
   }
    else{
    sms_text = "NO 3 PHASE SUPPLY";
    }
  }
//------------------------------------------------ 
  if(relay == "1"){
    if(digitalRead(motor1_pin) == HIGH){
    delay(10);
    sms_text = "MOTOR1 ON";
  }
  else{
    sms_text = "MOTOR1 OFF";
    }
  }
  //------------------------------------------------
  if(relay == "2"){
    if(digitalRead(compressor1_pin) == HIGH){
    delay(10);
    sms_text = "compressor1 ON";
  }
  else{
    sms_text = "compressor1 OFF";
    }
  }
  //------------------------------------------------
   if(relay == "3"){
   if(digitalRead(water_level_pin) == HIGH){
    delay(10);
    sms_text = "WATER LEVEL IS LOW";
  }
  else{
    sms_text = "WATER LEVEL IS HIGH";
    }
  }
  //------------------------------------------------
   if(relay == "4"){
    sms_text = (relay4_state) ? "ON" : "OFF";
    sms_text = "Relay 4 is " + sms_text;
  }
  //------------------------------------------------ 
  else if(relay == "5"){
    sms_text = (relay5_state) ? "ON" : "OFF";
    sms_text = "Relay 5 is " + sms_text;
  }
  //------------------------------------------------ 
   
  Reply(sms_text);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Reply(String text)
{
  Serial.println(text);
    sim800L.print("AT+CMGF=1\r");
    delay(1000);
    sim800L.print("AT+CMGS=\""+PHONE+"\"\r");
   
    delay(1000);
    sim800L.print(text);
    delay(1000);
    sim800L.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(500);
    Serial.println("SMS Sent Successfully.");

    msg = "";
}