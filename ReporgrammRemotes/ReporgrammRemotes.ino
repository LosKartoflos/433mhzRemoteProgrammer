//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <RCSwitch.h>

//setup
LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

const int buttonPinToggle = 3;
const int buttonPinDown = 4;
const int buttonPinUp = 5;
const int buttonPinWholeButton = 6;

const int ledPin = 13;

const int recieveInteruptPint = 0;
const int sendPin = 9;

//Defines how much buzzer and buttons there should be. If it reaches an end it loops through the other
const int maxBuzzerCount = 100;
const int maxButtonCount = 4;


//variables
int buzzerToProgramm = 0;
int buttonToProgramm = 0;

int  btToggleVoltage = 0;
int  btDownVoltage = 0;
int  btUpVoltage = 0;
int  btWholebuttonVoltage = 0;

bool btToggleIsPressed = false;
bool btDownIsPressed = false;
bool btUpIsPressed = false;
bool btWholeIsPressed = false;

unsigned long lastSend = 0;
const unsigned long sendInterval = 100; // sende alle 2000 ms (anpassen)

RCSwitch SwitchRecieve = RCSwitch();
RCSwitch SwitchSend = RCSwitch();


// its Defined by the buzzer and button size. the Maximum digit count of the buzzer and buttons is the size of the code to send
//thefirst digets should be the buzzer index filled up with zeroes if doesn't make up the dezimal. The same for the buttons
//the index starts always at zero because of the modulo which checks that the max number is never overcome.
//E.G. for 100 controlles and 4 Button the code for controler 11 (or 10 if you start counting at zero) and button 3(or C for alphabetical) is: 0102;
int codeToSend = 0;
int lastCodeToSend = 0;

bool sendMode = true;

void setup()
{
  //general
  Serial.begin(9600); 
  pinMode (ledPin, OUTPUT);

  codeToSend = GenerateCode(buzzerToProgramm, buttonToProgramm);

  //LCD
  lcd.init();                      // initialize the lcd 
  lcd.init();
  lcd.backlight();

  lcdPrintCurrentBuzzerAndButton(buzzerToProgramm, buttonToProgramm);  

  //Button
  
  //initialize pins
  pinMode(buttonPinToggle, INPUT_PULLUP);
  pinMode(buttonPinDown, INPUT_PULLUP);
  pinMode(buttonPinUp, INPUT_PULLUP);
  pinMode(buttonPinWholeButton, INPUT_PULLUP);

  //Transmission is activate in the button actions
  EnableTransmitData();

}

void loop()
{
  //buttons voltage
   btToggleVoltage = digitalRead(buttonPinToggle);
   btDownVoltage = digitalRead(buttonPinDown);
   btUpVoltage = digitalRead(buttonPinUp);
   btWholebuttonVoltage = digitalRead(buttonPinWholeButton);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  checkButtonStates();

  //sendeMode
  if(sendMode){
    sendData();

  }
  //recieve Mode
  else if(!sendMode){
    recieveData();
   
  }
  
}

//===================================================
// General Functions
//===================================================

//Sets BuzzerToProgrammToSpecific index. Modulos by max buzzer count
void SetBuzzer(int newBuIndex){

    int index = newBuIndex%maxBuzzerCount;
    if(index < 0)
      index = maxBuzzerCount-1;
    buzzerToProgramm = index; 

    codeToSend = GenerateCode(buzzerToProgramm, buttonToProgramm);
}

//Sets Button to new index. Modulos by max button count. !!!!Doesn't count up or down the Buzzer!!!
void SetButton(int newBtIndex){
    
    int index = newBtIndex%maxButtonCount;
    if(index < 0)
      index = maxButtonCount-1;
    buttonToProgramm = index; 

    codeToSend = GenerateCode(buzzerToProgramm, buttonToProgramm);
}

//Counts the buttonToProgramm one up an if needed the Buzzer at the max button
void CountButtonUp(){

  if(buttonToProgramm >= maxButtonCount-1)
    CountBuzzerUp();
    
  SetButton(buttonToProgramm+1);
}

//Counts up the Buzzer until the maximum is recached. then it starts from zero (due to modulo in SetBuzzer)
void CountBuzzerUp(){
  SetBuzzer(buzzerToProgramm+1);
}

//Counts the buttonToProgramm one down an if needed the Buzzer at the min button
void CountButtonDown(){

  if(buttonToProgramm <= 0)
    CountBuzzerDown();
    
  SetButton(buttonToProgramm-1);
}

//Counts up the Buzzer until the maximum is recached. then it jumps to the highest index (due to modulo in SetBuzzer)
void CountBuzzerDown(){
  SetBuzzer(buzzerToProgramm-1);
}

//generates the code to send to the remote according to "codeToSend" variable description
int GenerateCode(int buzzerIndex, int buttonIndex ){
 // Stellen ermitteln
  int buzzerDigits = String(maxBuzzerCount).length();
  int buttonDigits = String(maxButtonCount).length();

  // Gesamtdigits bestimmen
  int totalDigits = 1 + buzzerDigits + buttonDigits; // +1 wegen führender '1'

  // String zusammenbauen
  String code = "1";

  String buzzerStr = String(buzzerIndex);
  while (buzzerStr.length() < buzzerDigits) {
    buzzerStr = "0" + buzzerStr;
  }

  String buttonStr = String(buttonIndex);
  while (buttonStr.length() < buttonDigits) {
    buttonStr = "0" + buttonStr;
  }

  code += buzzerStr + buttonStr;

  // In int umwandeln und zurückgeben
  return code.toInt();
}

//===================================================
// Transmission
//===================================================

//Recieve 433 mhz Data and show it
void recieveData(){
   if (SwitchRecieve.available()) {
    lcdRecievingMessage(String(SwitchRecieve.getReceivedValue()));
    SwitchRecieve.resetAvailable();
  }
}

//handles the looping thins arround sending the code
void sendData(){
  //LCD update
  if(lastCodeToSend != codeToSend){
    lcdSendingMessage(String(codeToSend));
    lastCodeToSend = codeToSend;
  }

  //send in intervals
  if (millis() - lastSend >= sendInterval) {
    lastSend = millis(); 
    SwitchSend.send(codeToSend, 24);
  }
}

void EnableRecieveData() {
  SwitchRecieve.enableReceive(recieveInteruptPint); // Receiver on interrupt 0 => that is pin #2
}

void DisableRecieveData() {
  SwitchRecieve.disableReceive();
}

void EnableTransmitData() {
  SwitchSend.enableTransmit(sendPin); 
}

void DisableTransmitData() {
  SwitchSend.disableTransmit();
}




//===================================================
// Buttons
//===================================================

//higth is unpressed 
//low is pressed
//(because of internal pullup resistor usage)

//Checks if a button is newly pressed and triggers corresponding action
void checkButtonStates(){
  //Button Toggle
  if (btToggleVoltage == HIGH) {
    if(btToggleIsPressed)
      btToggleIsPressed = false;
    //Serial.println("Button1 not pressed");
  } else {
    //Click
    if(!btToggleIsPressed && btToggleVoltage == LOW) {
      btToggleIsPressed = true;
      btToggleAction();
      //Serial.println("Button1");
    }
  }

//Button down
  if (btDownVoltage == HIGH) {
    if(btDownIsPressed)
      btDownIsPressed = false;
  } else {
    //Click
    if(!btDownIsPressed){
      btDownIsPressed = true;
      btDownAction();
    }
  }

//Button Up
  if (btUpVoltage == HIGH) {
    if(btUpIsPressed)
      btUpIsPressed = false;
  } else {
    //Click
    if(!btUpIsPressed){
      btUpIsPressed = true;
      btUpAction();
    }
  }

//Whole button next
  if (btWholebuttonVoltage == HIGH) {
   if(btWholeIsPressed)
      btWholeIsPressed = false;
    
    //for Visualization
    digitalWrite(ledPin, LOW);
  } else {
    //Click
    if(!btWholeIsPressed){
      btWholeIsPressed = true;
      btWholeButtonAction();
    }
    //for Visualization
    digitalWrite(ledPin, HIGH);
  }
}

//Should Toggle Read or Write mode for the antennas
void btToggleAction(){
  sendMode = !sendMode;
  lcdClearfirstRow();

  if(sendMode){
    DisableRecieveData();
    EnableTransmitData();   
    lcdSendingMessage(String(codeToSend));
  }
  else if(!sendMode){
    DisableTransmitData();
    EnableRecieveData();
    lcdRecievingMessage("xxx");
  }
    
}

//Should count up the BUtton of the Remote and after the forth (#4) button the Buzzer is counted up, too. 
void btUpAction(){

  if(!btWholeIsPressed){
    CountButtonUp();
  }
  else if(btWholeIsPressed){
    CountBuzzerUp();
  }
  lcdPrintCurrentBuzzerAndButton(buzzerToProgramm, buttonToProgramm);
  
}

//Should count down the BUtton of the Remote and after the first (#1) button the Buzzer is counted down, too. 
void btDownAction(){
  if(!btWholeIsPressed)
  {
    CountButtonDown();  
  }
  else if(btWholeIsPressed){
    CountBuzzerDown();
  }
  lcdPrintCurrentBuzzerAndButton(buzzerToProgramm, buttonToProgramm);
}

//Lets the Up and Down button Jump to the next corresponding Buzzer at Button zero.
void btWholeButtonAction(){
  //nothing needed here right now
}

//===================================================
// LCD Display
//===================================================

// void lcdDisplayUpdate(){
//   lcd.clear();

//   lcdPrintCurrentBuzzerAndButton(buzzerToProgramm, buttonToProgramm)
// }

//clears the first row with " ";
void lcdClearfirstRow(){
  lcd.setCursor(0,0);
  lcd.print("                ");
}

void lcdClearSecondRow(){
  lcd.setCursor(1,0);
  lcd.print("                ");
}

//is displaying at line 0 the SendingMessage (on which code is sender)
void lcdSendingMessage(String code){
  //lcdClearfirstRow();
  lcd.setCursor(0,0);
  lcd.print("Sending: " + code);
}

//is displaying at line 0 the RecievedMessage (on which code is sender)
void lcdRecievingMessage(String code)
{ 
  //lcdClearfirstRow();
  lcd.setCursor(0,0);
  lcd.print("Reciving: " + code);

}

//is displaying at line 1  the current Buztter and Button
void lcdPrintCurrentBuzzerAndButton(int _buzzer, int _button)
{
  lcdClearSecondRow();
  lcd.setCursor(0,1);
  lcd.print( "Buz: " + String (_buzzer) + " But:" + String(_button) +"       " );
}