//how many ms between flashes of an individual LED
int refRate = 28;
//how long in ms to hold each one or two digit state before scrolling
int characterHold = 750;
//how many milliseconds between checks for new data
int dataRefresh = 5000;
//How long of a phrase can we display?
int maxPhraseLength = 32;
//an array of the pins driving the active LED segments
int activePins[16];
int pinsCount = 0;
//are we monitoring a value or displaying a message?
bool monitoring = false;
//have we received the first valid message from CoreTemp?
bool coreTempLoaded = false;

void setup() {
  // put your setup code here, to run once:
   pinMode(2, OUTPUT);
   //tens digit
   pinMode(4, OUTPUT);
   pinMode(5, OUTPUT);
   pinMode(6, OUTPUT);
   pinMode(7, OUTPUT);
   pinMode(8, OUTPUT);
   pinMode(9, OUTPUT);
   pinMode(10, OUTPUT);
   //ones digit
   pinMode(25, OUTPUT);
   pinMode(26, OUTPUT);
   pinMode(27, OUTPUT);
   pinMode(28, OUTPUT);
   pinMode(29, OUTPUT);
   pinMode(30, OUTPUT);
   pinMode(31, OUTPUT);
   //voltage +
   pinMode(32, OUTPUT);
   digitalWrite(32, HIGH);

   digitalWrite(2, HIGH);

   //setup connection to CoreTemp
   Serial.setTimeout(50);
   Serial.begin(9600);
   //display power-on message
   parseMessage("SERENDIPITY", 11);
   //parseMessage("SEND NUDES", 10);
   monitoring = true;
   
}

void loop() {
  
  if(monitoring == true) {
    displayTemp();
  }
  if(monitoring == false){
    parseMessage("SERENDIPITY", 11);
  }
}
  //the segments are designated as such:
  //--FAAAB--
  //--F---B--
  //--GGGGG--
  //--E---C--
  //--EDDDC--
  //
  //digit 0 is the ones place (right-most digit)
  
int getPinFromSegment(char seg, int digit) {

  if(digit == 0) {  
    switch(seg) {
      case 'A': return 25;
      case 'B': return 26;
      case 'C': return 27;
      case 'D': return 28;
      case 'E': return 29;
      case 'F': return 30;
      case 'G': return 31;
    }
  }
  if(digit == 1) {
    switch(seg) {
      case 'A': return 4;
      case 'B': return 5;
      case 'C': return 6;
      case 'D': return 7;
      case 'E': return 8;
      case 'F': return 9;
      case 'G': return 10;
    }
  }
  //if none of the cases match, then return the pin for the left-most '1' LED
  return 2;
}
//Given a character you wish to display, return an array of segments that need ot be illuminated 
//to display it
int segmentsFromChar(char ledChar, char **segmentsToLight) {
  switch(ledChar) {
    //Digits
    case '1': *segmentsToLight = "BC";
            return 2; 
    case '2': *segmentsToLight = "ABGED";
            return 5;
    case '3': *segmentsToLight = "ABGCD";
            return 5;
    case '4': *segmentsToLight = "FGBC";
            return 4;
    case '5': *segmentsToLight = "AFGCD";
            return 5;
    case '6': *segmentsToLight = "AFGCDE";
            return 6;
    case '7': *segmentsToLight = "ABC";
            return 3;
    case '8': *segmentsToLight = "ABCDEFG";
            return 7;
    case '9': *segmentsToLight = "ABGFC";
            return 5;
    case '0': *segmentsToLight = "ABCDEF";
            return 6;
    //Letters
    case 'A': *segmentsToLight = "ABCEFG";
            return 6;
    case 'B': *segmentsToLight = "FGCDE";
            return 5;
    case 'C': *segmentsToLight = "AFED";
            return 4;
    case 'D': *segmentsToLight = "BGCDE";
            return 5;
    case 'E': *segmentsToLight = "ADEFG";
            return 5;
    case 'F': *segmentsToLight = "AEFG";
            return 4;
    case 'G': *segmentsToLight = "ABCDFG";
            return 6;
    case 'H': *segmentsToLight = "CEFG";
            return 4;
    case 'I': *segmentsToLight = "BC";
            return 2;
    case 'J': *segmentsToLight = "BCDE";
            return 4;
    case 'K': *segmentsToLight = "AFGECA";
            return 6;
    case 'L': *segmentsToLight = "EFD";
            return 3;
    case 'M': *segmentsToLight = "EGC";
            return 3;
    case 'N': *segmentsToLight = "EGC";
            return 3;
    case 'O': *segmentsToLight = "EGCD";
            return 4;
    case 'P': *segmentsToLight = "AFBGE";
            return 5;
    case 'Q': *segmentsToLight = "AFBGED";
            return 6;
    case 'R': *segmentsToLight = "EG";
            return 2;
    case 'S': *segmentsToLight = "AFGCD";
            return 5;
    case 'T': *segmentsToLight = "AFE";
            return 3;
    case 'U': *segmentsToLight = "FEDCB";
            return 5;
    case 'V': *segmentsToLight = "EDC";
            return 3;
    case 'W': *segmentsToLight = "EDC";
            return 3;
    case 'X': *segmentsToLight = "BCEFG";
            return 5;
    case 'Y': *segmentsToLight = "FBGCD";
            return 5;
    case 'Z': *segmentsToLight = "ABGED";
            return 5;
  }
  //If it's not a valid character, leave segmentsToLight alone and return 0 length.
  return 0;
}
int displayChars(char digit1, char digit0) {
  char *segmentsToLight0 = "";
  char *segmentsToLight1 = "";
  
  int numOfSegs0 = segmentsFromChar(digit0, &segmentsToLight0);
  int numOfSegs1 = segmentsFromChar(digit1, &segmentsToLight1);
  for(int i = 0; i < numOfSegs0; i++){
    activePins[i] = getPinFromSegment(segmentsToLight0[i],0);
  }
  for(int i = numOfSegs0; i < numOfSegs0+numOfSegs1; i++){
    activePins[i] = getPinFromSegment(segmentsToLight1[i-numOfSegs0],1);
  }
  //return the number of active segments for the whole display
  return numOfSegs0 + numOfSegs1;
}

void illuminatePins(){
  //We want the overall number of writes to a pin per second to remain constant, regardless of
  //the number of pins being written to.
  int pinRefresh = refRate / pinsCount;
  for(int i = 0; i < pinsCount; i++)
  {
    //We shouldn't ever have out of bounds pins, but check just in case...
    if((activePins[i] >= 4 && activePins[i] <= 10) || (activePins[i] >= 25 && activePins[i] <= 31) ) {
      digitalWrite(activePins[i], LOW);
      delay(pinRefresh);
      digitalWrite(activePins[i], HIGH);
    }
  }
}

void parseMessage(char message[], int size) {
  //at least two characters/digits in the message
  //display an empty space in digit 1 to start scrolling effect
  pinsCount = displayChars(' ', message[0]);
  for(int j = 0; j < characterHold / refRate; j++) {
    illuminatePins();
  }
  //clearDisplay();

  for(int i = 0; i < size - 1; i++) {
    pinsCount = displayChars(message[i], message[i+1]);
    for(int j = 0; j < characterHold / refRate; j++) {
      illuminatePins();
    }
    clearDisplay();    
  }
  //Showing one letter at a time
 /*  for(int i = 0; i < size - 1; i++) {
    pinsCount = displayChars(' ', message[i]);
    for(int j = 0; j < characterHold / refRate; j++) {
      illuminatePins();
    }
    pinsCount = displayChars(message[i], ' ');
    for(int j = 0; j < characterHold / refRate; j++) {
      illuminatePins();
    }
    clearDisplay();    
  }*/
  pinsCount = displayChars(message[size-1], ' ');
  for(int j = 0; j < characterHold / refRate; j++) {
      illuminatePins();
  }
  //clearDisplay();

  
}
//illuminatePins should take care of this by itself, but it may be a useful function to have
void clearDisplay() {
  for(int i = 4; i <= 10; i++) {
    digitalWrite(i, HIGH);
  }
  for(int i = 25; i <= 31; i++) {
    digitalWrite(i, HIGH);
  }
}

void displayTemp() {
  String currentTemp = Serial.readString();
  if(coreTempLoaded == false) parseMessage("SERENDIPITY",11);
  if(currentTemp == "" && coreTempLoaded == true){
    for(int i = 0; i < dataRefresh / refRate; i++) {
      illuminatePins();
    }
  }
  //Serial.println(Serial.readString());
  //delay(2000);
  if(isDigit(currentTemp.charAt(1)) && isDigit(currentTemp.charAt(0))){
    coreTempLoaded = true;
    pinsCount = displayChars(currentTemp.charAt(0),currentTemp.charAt(1));
    for(int i = 0; i < dataRefresh / refRate; i++) {
      illuminatePins();
    }
  }
}

