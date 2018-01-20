#include <SoftwareSerial.h>

char inchar; // Will hold the incoming character from the GSM shield
SoftwareSerial SIM900(7, 8);

const int nAuthorized = 2;
const String authorized[nAuthorized] = {"1234567890"}; // Add phone numbers to authorize
const String defaultNr = authorized[0];

int pin = 13;

void setup()
{
  
  powerUpOrDown();

  Serial.begin(19200);
  // set up the digital pins to control
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);

  // wake up the GSM shield
  SIM900.begin(19200);
  delay(1000);
  SIM900.println("AT+IPR=19200");
  delay(5000);
  SIM900.println("AT+COPS=0");
  delay(100);
  registerToNetwork();
 
  SIM900.println("AT+CMGF=1"); // set SMS mode to text
  delay(100);
  SIM900.println("AT+CMGF=?");
  delay(100);
  SIM900.println("AT+CFUN=1");
  delay(100);
  SIM900.println("AT+CMGD=1,4"); // delete all SMS
  delay(100);
  SIM900.println("AT+CNMI=2,2,0,0,0");  // blurt out contents of new SMS upon receipt to the GSM shield's serial out
  delay(100);
  SIM900.println("AT+CPIN?");
  delay(100);
  Serial.println("Ready...");

  sendSMS("Successfully connected to network", defaultNr);
}

void registerToNetwork()
{
  int retries = 125;
  int backoffms = 3000;

  SIM900.println("AT+CFUN=1");
  delay(5000);
  SIM900.println("AT+CPIN=\"0000\"");
  delay(5000);
  for (int i=0; i < retries; i++)
  {
    SIM900.println("AT+CREG?");
    delay(1000);
    String msg = readSIM900();
    Serial.println(msg);
    if(msg.substring(21, 22) == "1"){
      Serial.println("Registered");
      return;
    }
    else{
      Serial.println("Not registered");
      delay(backoffms);
    }
  }
  Serial.println("Max retries exceeded...");
  delay(100);
  powerUpOrDown();
  exit(1);
}

void loop()
{
  delay(1000);

  String msg = readSIM900();
  String phoneNr = msg.substring(9, 21);

  //If a character comes in from the cellular module...
  if(msg.substring(2,6) == "+CMT")
  {
    delay(10);

    String phoneNr = msg.substring(9, 21);
    Serial.println(phoneNr);
    if(!isAuthorized(phoneNr)){
      Serial.println("Phone number not authorized");
      return;
    }
    Serial.println("Phone number authorized");

    String cmd = msg.substring(50, 51);
    Serial.println(cmd);
    if(cmd == "A"){
      digitalWrite(pin, HIGH);
      delay(100);
      sendSMS("Switch on", phoneNr);
    }
    else if(cmd == "B"){
      digitalWrite(pin, LOW);
      delay(100);
      sendSMS("Switch off", phoneNr);
    }
    else{
      sendSMS("Unrecognised command (A = on, B = off)", phoneNr);
    }
    delay(1000);
    SIM900.println("AT+CMGD=1,4"); // delete all SMS
  }
}

boolean isAuthorized(String phoneNr)
{
  for (int i=0; i <= nAuthorized; i++)
  {
    if(phoneNr == authorized[i])
    {
      return true;
    }
  }
  return false;
}

String readSIM900()
{
    String buffer;

    while (SIM900.available())
    {
        delay(10);
        char c = SIM900.read();
        buffer.concat(c);
    }

    return buffer;
}

void sendSMS(String msg, String nr)
{
  Serial.println("Sending Text...");
  SIM900.println("AT+CMGF=1"); // Set the shield to SMS mode
  delay(100);
  // send sms message, the phone number needs to include the country code e.g. if a U.S. phone number such as (540) 898-5543 then the string must be:
  // +15408985543
  SIM900.println("AT+CMGS = \"" + nr + "\"");
  delay(100);
  SIM900.println(msg); //the content of the message
  delay(100);
  SIM900.print((char)26);//the ASCII code of the ctrl+z is 26 (required according to the datasheet)
  delay(100);
  SIM900.println();
  Serial.println("Text Sent.");
}

void powerUpOrDown()
{
    pinMode(9, OUTPUT);
    digitalWrite(9,LOW);
    delay(1000);
    digitalWrite(9,HIGH);
    delay(2000);
    digitalWrite(9,LOW);
    delay(3000);
}
