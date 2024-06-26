#include <SoftwareSerial.h>

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
#include <EEPROM.h>
//sender phone number with country code
//const String PhoneNo = "+919344281866";
const int totalPhoneNo = 5;
//String phoneNo[totalPhoneNo] = {"+919344281866","+919688010099","+919688010088","+918675112280","+918883304655"};
String phoneNo[totalPhoneNo] = {"","","","",""};
int offsetPhone[totalPhoneNo] = {0,13,26,39,52};
String tempPhone = "";
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

//GSM Module RX pin to Arduino 3
//GSM Module TX pin to Arduino 2
#define rxPin 12
#define txPin 13
SoftwareSerial sim800(rxPin,txPin);
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
#define RELAY_1 8
#define RELAY_2 9
#define RELAY_3 10
#define RELAY_4 11
#define RELAY_5 3

#define R_phase_pin A1
#define Y_phase_pin A2
#define B_phase_pin A3
#define motor1_pin A4
#define compressor1_pin A5
#define water_level_pin A6

boolean STATE_RELAY_1 = 0;
boolean STATE_RELAY_2 = 0;
boolean STATE_RELAY_3 = 0;
boolean STATE_RELAY_4 = 0;
boolean STATE_RELAY_5 = 0;
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
String smsStatus,senderNumber,receivedDate,msg;
boolean isReply = false;
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
String dtmf_cmd;
boolean is_call = false;

//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

boolean DEBUG_MODE = 1;

/***************************
 * setup function
 **************************/
void setup() {
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  Serial.begin(115200);
  Serial.println("Arduino serial initialize");
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  sim800.begin(9600);
  Serial.println("SIM800L software serial initialize");
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
   ////////////////////////////////////////////PIN FOR R PHASE,Y PHASE,B PHASE ////////////////
  pinMode(R_phase_pin, INPUT_PULLUP);
  pinMode(Y_phase_pin, INPUT_PULLUP);
  pinMode(B_phase_pin, INPUT_PULLUP);
///////////////////////////////////////////PIN FOR MOTOR STATE ////////////////
  pinMode(motor1_pin, INPUT_PULLUP);
  pinMode(compressor1_pin, INPUT_PULLUP);
  pinMode(water_level_pin, INPUT_PULLUP);

  pinMode(RELAY_1, OUTPUT); //Relay 1
  pinMode(RELAY_2, OUTPUT); //Relay 2
  pinMode(RELAY_3, OUTPUT); //Relay 3
  pinMode(RELAY_4, OUTPUT); //Relay 4
  pinMode(RELAY_5, OUTPUT); //Relay 5

  digitalWrite(RELAY_1, HIGH);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);
  digitalWrite(RELAY_5, LOW);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  Serial.println("List of Registered Phone Numbers");
  for (int i = 0; i < totalPhoneNo; i++){
    phoneNo[i] = readFromEEPROM(offsetPhone[i]);
    if(phoneNo[i].length() != 13)
      {phoneNo[i] = "";Serial.println(String(i+1)+": empty");}
    else
      {Serial.println(String(i+1)+": "+phoneNo[i]);}
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  delay(7000);
  sim800.print("AT+CMGF=1\r"); //SMS text mode
  delay(1000);
  sim800.println("AT+DDET=1"); //Enable DTMF
  delay(500);
  sim800.println("AT+CLIP=1"); //Enable Caller ID
  delay(500);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}

/***************************
 * Loop Function
 **************************/
void loop() {
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
while(sim800.available()){
  parseData(sim800.readString());
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
while(Serial.available())  {
  sim800.println(Serial.readString());
}
//MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
} //main loop ends

/***************************
 * parseData function:
 * this function parse the incomming command such as CMTI or CMGR etc.
 * if the sms is received. then this function read that sms and then pass 
 * that sms to "extractSms" function. Then "extractSms" function divide the
 * sms into parts. such as sender_phone, sms_body, received_date etc.
 **************************/
void parseData(String buff){
  Serial.println(buff);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
  if(buff.indexOf("RING") > -1)
  {
    boolean flag = 0;
    //------------------------------------------
    //+CLIP: "03001234567",129,"",0,"",0
    if(buff.indexOf("+CLIP:")){
     unsigned int index, index1;
     
      index = buff.indexOf("\"");
      index1 = buff.indexOf("\"", index+1);
      //get phone like this format 3001234567
      String temp = buff.substring(index+2, index1);
      temp.trim();
      debugPrint("meee: "+temp);
      //Serial.println(temp.length());
      if(temp.length() == 13){
        //number formate xxx-yyy-zzzzzzz
        //+923001234567
        flag = comparePhone(temp);
      }
      //------------------------------------------
      else if(temp.length() == 10){
        //number formate yyyy-zzzzzzz
        //3001234567
        flag = compareWithoutCountryCode(temp); 
      }
    }
    //------------------------------------------
    if(flag == 1){
      //delay(6000);
      sim800.println("ATA");
      delay(500);
      sim800.println("AT+VTD=500;");
      is_call = true;
    }
    else{
      debugPrint("The phone number is not registered.");
      //is_call = false;
    }
    return;
  }
  //HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
  else if(is_call == true){
    //----------------------------------------------------------------
    if(int index = buff.indexOf("+DTMF:") > -1 )
    {
      index = buff.indexOf(":");
      dtmf_cmd = buff.substring(index+1, buff.length());
      dtmf_cmd.trim();
      debugPrint("DTMF Command: "+dtmf_cmd);
      controlDTMF(dtmf_cmd);
      return;
    }
    //----------------------------------------------------------------
    if(buff.indexOf("NO CARRIER") > -1)
    {
      sim800.println("ATH");
      is_call = false;
      return;
    }
    //----------------------------------------------------------------
  }
  //HHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  
  unsigned int len, index;
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Remove sent "AT Command" from the response string.
  index = buff.indexOf("\r");
  buff.remove(0, index+2);
  buff.trim();
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  
  if(buff != "OK"){
    index = buff.indexOf(":");
    String cmd = buff.substring(0, index);
    cmd.trim();
    
    buff.remove(0, index+2);
    
    if(cmd == "+CMTI"){
      //get newly arrived memory location and store it in temp
      index = buff.indexOf(",");
      String temp = buff.substring(index+1, buff.length()); 
      temp = "AT+CMGR=" + temp + "\r"; 
      //get the message stored at memory location "temp"
      sim800.println(temp); 
    }
    else if(cmd == "+CMGR"){
      extractSms(buff);
      //----------------------------------------------------------------------------
      if(msg.equals("r") && phoneNo[0].length() != 13) {
        writeToEEPROM(offsetPhone[0],senderNumber);
        phoneNo[0] = senderNumber;
        String text = "Number is Registered: ";
        text = text + senderNumber;
        debugPrint(text);
        Reply("Number is Registered", senderNumber);
      }
      //----------------------------------------------------------------------------
      if(comparePhone(senderNumber)){
        doAction(senderNumber);
      }
      //----------------------------------------------------------------------------
    }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  }
  else{
  //The result of AT Command is "OK"
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  
}


/***************************
 * extractSms function:
 * This function divide the sms into parts. such as sender_phone, sms_body, 
 * received_date etc.
 **************************/
void extractSms(String buff){
   unsigned int index;
   
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

    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
    String tempcmd = msg.substring(0, 3);
    if(tempcmd.equals("r1=") || tempcmd.equals("r2=") ||
       tempcmd.equals("r3=") || tempcmd.equals("r4=") ||
       tempcmd.equals("r5=")){
        
        tempPhone = msg.substring(3, 16);
        msg = tempcmd;
        //debugPrint(msg);
        //debugPrint(tempPhone);
    }
    //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
       
}

/***************************
 * doAction function:
 * Performs action according to the received sms
 **************************/
void doAction(String phoneNumber){
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  if(msg == "motor1on"){  
    if(digitalRead(R_phase_pin) && digitalRead(Y_phase_pin) && digitalRead(B_phase_pin) == HIGH){
    digitalWrite(RELAY_1, LOW);
    delay(100);
    digitalWrite(RELAY_1, HIGH);
    STATE_RELAY_1 = 1;
    debugPrint("Relay 1 is ON");

    digitalWrite(RELAY_4, LOW);
    STATE_RELAY_4 = 0;
    debugPrint("Relay 4 is OFF");

    Reply("MOTOR 1=ON COMPRESSOR 1= OFF", phoneNumber);
  }
  else{
    Reply("NO 3 PHASE SUPPLY", phoneNumber);
  }
  }
  else if(msg == "motor1off"){
    digitalWrite(RELAY_2, LOW);
    delay(100);
    digitalWrite(RELAY_2, HIGH);
    STATE_RELAY_2 = 0;
    debugPrint("Relay 2 is OFF");
    Reply("MOTOR 1= OFF", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "compressor1on"){ 
    if(digitalRead(R_phase_pin) && digitalRead(Y_phase_pin) && digitalRead(B_phase_pin) == HIGH){
     delay(10); 
    digitalWrite(RELAY_2, LOW);
    delay(100);
    digitalWrite(RELAY_2, HIGH);
    STATE_RELAY_2 = 0;
    debugPrint("Relay 2 is OFF");
    digitalWrite(RELAY_3, LOW);
    delay(100);
    digitalWrite(RELAY_3, HIGH);
    STATE_RELAY_3 = 1;
    debugPrint("Relay 3 is ON");
    Reply("COMPRESSOR 1= ON MOTOR 1=OFF ", phoneNumber);
  }
  else{
    Reply("NO 3 PHASE SUPPLY", phoneNumber);
  }
  }
  else if(msg == "compressor1off"){
    digitalWrite(RELAY_4, LOW);
    STATE_RELAY_4 = 0;
    debugPrint("Relay 4 is OFF");
    Reply("COMPRESSOR 1= OFF", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "lighton"){  
    digitalWrite(RELAY_5, HIGH);
    STATE_RELAY_5 = 1;
    debugPrint("Relay 5 is ON");
    Reply("Relay 5 is ON", phoneNumber);
  }
  else if(msg == "lightoff"){
    digitalWrite(RELAY_5, LOW);
    STATE_RELAY_5 = 0;
    debugPrint("Relay 5 is OFF");
    Reply("light is OFF", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "stat=0"){
    if(digitalRead(R_phase_pin) && digitalRead(Y_phase_pin) && digitalRead(B_phase_pin) == HIGH){
    delay(10);
    Reply("HAVING 3 PHASE SUPPLY", phoneNumber);
  }
    else{
    Reply("NO 3 PHASE SUPPLY ", phoneNumber);
    }
  }
  else if(msg == "stat=1"){
    if(digitalRead(motor1_pin) == HIGH){
    delay(10);
     Reply("MOTOR1 ON", phoneNumber);
  }
  else{
    Reply("MOTOR1 OFF", phoneNumber);
  }
  }
  else if(msg == "stat=2"){
    if(digitalRead(compressor1_pin) == HIGH){
    delay(10);
    Reply("compressor1 ON", phoneNumber);
  }
  else{
    Reply("compressor1 OFF", phoneNumber);
  }
  }
  else if(msg == "stat=3"){
   if(digitalRead(water_level_pin) == HIGH){
    delay(10);
    Reply("WATER LEVEL IS LOW", phoneNumber);
  }
  else{
    Reply("WATER LEVEL IS HIGH", phoneNumber);
  }
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "r2="){  
      writeToEEPROM(offsetPhone[1],tempPhone);
      phoneNo[1] = tempPhone;
      String text = "Phone2 is Registered: ";
      text = text + tempPhone;
      debugPrint(text);
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "r3="){  
      writeToEEPROM(offsetPhone[2],tempPhone);
      phoneNo[2] = tempPhone;
      String text = "Phone3 is Registered: ";
      text = text + tempPhone;
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "r4="){  
      writeToEEPROM(offsetPhone[3],tempPhone);
      phoneNo[3] = tempPhone;
      String text = "Phone4 is Registered: ";
      text = text + tempPhone;
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "r5="){  
      writeToEEPROM(offsetPhone[4],tempPhone);
      phoneNo[4] = tempPhone;
      String text = "Phone5 is Registered: ";
      text = text + tempPhone;
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "list"){  
      String text = "";
      if(phoneNo[0])
        text = text + phoneNo[0]+"\r\n";
      if(phoneNo[1])
        text = text + phoneNo[1]+"\r\n";
      if(phoneNo[2])
        text = text + phoneNo[2]+"\r\n";
      if(phoneNo[3])
        text = text + phoneNo[3]+"\r\n";
      if(phoneNo[4])
        text = text + phoneNo[4]+"\r\n";
        
      debugPrint("List of Registered Phone Numbers: \r\n"+text);
      Reply(text, phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=1"){  
      writeToEEPROM(offsetPhone[0],"");
      phoneNo[0] = "";
      Reply("Phone1 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=2"){  
      writeToEEPROM(offsetPhone[1],"");
      phoneNo[1] = "";
      debugPrint("Phone2 is deleted.");
      Reply("Phone2 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=3"){  
      writeToEEPROM(offsetPhone[2],"");
      phoneNo[2] = "";
      debugPrint("Phone3 is deleted.");
      Reply("Phone3 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=4"){  
      writeToEEPROM(offsetPhone[3],"");
      phoneNo[3] = "";
      debugPrint("Phone4 is deleted.");
      Reply("Phone4 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(msg == "del=5"){  
      writeToEEPROM(offsetPhone[4],"");
      phoneNo[4] = "";
      debugPrint("Phone5 is deleted.");
      Reply("Phone5 is deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM  
  if(msg == "del=all"){  
      writeToEEPROM(offsetPhone[0],"");
      writeToEEPROM(offsetPhone[1],"");
      writeToEEPROM(offsetPhone[2],"");
      writeToEEPROM(offsetPhone[3],"");
      writeToEEPROM(offsetPhone[4],"");
      phoneNo[0] = "";
      phoneNo[1] = "";
      phoneNo[2] = "";
      phoneNo[3] = "";
      phoneNo[4] = "";
      offsetPhone[0] = "";
      offsetPhone[1] = "";
      offsetPhone[2] = "";
      offsetPhone[3] = "";
      offsetPhone[4] = "";
      debugPrint("All phone numbers are deleted.");
      Reply("All phone numbers are deleted.", phoneNumber);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";
  tempPhone = "";
}
/***************************
 * Reply function
 * Send an sms
 **************************/
void Reply(String text, String Phone)
{
    return;
    sim800.print("AT+CMGF=1\r");
    delay(1000);
    sim800.print("AT+CMGS=\""+Phone+"\"\r");
    delay(1000);
    sim800.print(text);
    delay(100);
    sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(1000);
    Serial.println("SMS Sent Successfully.");
    /*********************
     * only upto 70 messages can be stored in sim800l memory
     * after the memory is full, then no new sms will be received
     * until you free up the sms memory by deleting some sms.
     * that's why the below written command deletes all the sms
     * from the memory automatically. 
     *********************/
    //sim800.print("AT+CMGD=1,4");
}
/***************************
 * writeToEEPROM function:
 * Store registered phone numbers in EEPROM
 **************************/
void writeToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = 13; //strToWrite.length();
  //EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    //meee
    //Serial.print(addrOffset + i);
    //Serial.println(strToWrite[addrOffset + i]);
    EEPROM.write(addrOffset + i, strToWrite[i]);
  }
}
/***************************
 * readFromEEPROM function:
 * Store phone numbers in EEPROM
 **************************/
String readFromEEPROM(int addrOffset)
{
  int len = 13;
  char data[len + 1];
  for (int i = 0; i < len; i++)
  {
    data[i] = EEPROM.read(addrOffset + i);
  }
  data[len] = '\0';
  return String(data);
}
/***************************
 * comparePhone function:
 * compare phone numbers stored in EEPROM
 **************************/
boolean comparePhone(String number)
{
  boolean flag = 0;
  //--------------------------------------------------
  for (int i = 0; i < totalPhoneNo; i++){
    phoneNo[i] = readFromEEPROM(offsetPhone[i]);
    if(phoneNo[i].equals(number)){
      //Serial.println(phoneNo[i]);
      flag = 1;
      break;
    }
  }
  //--------------------------------------------------
  return flag;
}

/***************************
 * compareWithoutCountryCode function:
 * compare phone numbers stored in EEPROM
 **************************/
boolean compareWithoutCountryCode(String number)
{
  boolean flag = 0;
  //--------------------------------------------------
  for (int i = 0; i < totalPhoneNo; i++){
    phoneNo[i] = readFromEEPROM(offsetPhone[i]);
    //remove first 3 digits (country code)
    phoneNo[i].remove(0,3);
    Serial.println("meee1: "+phoneNo[i]);
    if(phoneNo[i].equals(number)){
      //Serial.println(phoneNo[i]);
      flag = 1;
      break;
    }
  }
  //--------------------------------------------------
  return flag;
}
/***************************
 * dtmfControl function:
 * compare phone numbers stored in EEPROM
 **************************/
void controlDTMF(String dtmf_code){
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  if(dtmf_code == "1"){
    if(digitalRead(R_phase_pin) && digitalRead(Y_phase_pin) && digitalRead(B_phase_pin) == HIGH){
    delay(10);
    digitalWrite(RELAY_1, LOW);
    delay(100);
    digitalWrite(RELAY_1, HIGH);
    debugPrint("Relay 1 is ON");
    sim800.println("AT+VTS=0;");
    }
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(dtmf_code == "2"){
    digitalWrite(RELAY_2, LOW);
    delay(100);
    digitalWrite(RELAY_2, HIGH);
    STATE_RELAY_2 = 0;
    debugPrint("Relay 2 is OFF");
    delay(1000);
    sim800.println("AT+VTS=0;");
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(dtmf_code == "3"){
    digitalWrite(RELAY_3, LOW);
    delay(100);
    digitalWrite(RELAY_3, HIGH);
    STATE_RELAY_3 = 1;
    debugPrint("Relay 3 is ON");
    delay(1000);
    sim800.println("AT+VTS=0;");
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(dtmf_code == "4"){
     digitalWrite(RELAY_4, LOW);
    STATE_RELAY_4 = 0;
    debugPrint("Relay 4 is OFF");
    delay(1000);
    sim800.println("AT+VTS=0;");
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(dtmf_code == "5"){
    digitalWrite(RELAY_5, HIGH);
    STATE_RELAY_5 = 1;
    debugPrint("Relay 5 is ON");
     delay(1000);
    sim800.println("AT+VTS=0;");
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(dtmf_code == "6"){
     digitalWrite(RELAY_5, LOW);
    STATE_RELAY_5 = 1;
    debugPrint("Relay 5 is ON");
    delay(1000);
    sim800.println("AT+VTS=0;");
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  else if(dtmf_code == "7"){
    digitalWrite(RELAY_1, HIGH);
    delay(1000);
    digitalWrite(RELAY_2, HIGH);
    delay(500);
    digitalWrite(RELAY_3, HIGH);
    delay(1000);
    digitalWrite(RELAY_4, HIGH);
    STATE_RELAY_1 == 1;
    STATE_RELAY_2 == 1;
    STATE_RELAY_3 == 1;
    STATE_RELAY_4 == 1;
    debugPrint("All relays are OFF");
    sim800.println("AT+VTS=0;");
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}

/***************************
 * debugPrint function:
 * compare phone numbers stored in EEPROM
 **************************/
 void debugPrint(String text){
  if(DEBUG_MODE == 1)
    Serial.println(text);
 }