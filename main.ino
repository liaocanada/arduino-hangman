//Music notes used
#define C_NOTE 523
#define D_NOTE 587
#define Eb_NOTE 622
#define E_NOTE 659
#define F_NOTE 698

#include <LiquidCrystal.h>
#include <avr/pgmspace.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//Custom characters (First dimension - 8 "additions" or "pieces" of hangman)
//A 4x8 byte[][] - the [4] represents left column (up and down) and then second column (up and down)
//The [8] is for each row in a custom character
byte hangmanPieces[8][4][8] = {
{ //Hangman 1 (stand)
  { B00000, B00100, B00100, B00100, B00100, B00100, B00100, B00100}, 
  { B00100, B00100, B00100, B00100, B00100, B00100, B11111, B11111}, 
  { B0, B0, B0, B0, B0, B0, B0, B0}, 
  { B0, B0, B0, B0, B0, B0, B0, B0}
}, 

{ //Hangman 2 (head)
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B01110, B01110, B01110, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0}
}, 

{ //Hangman 3 (torso)
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B00100},
   { B00100, B00100, B00100, B00100, B0, B0, B0, B0}
}, 

{ //Hangman 4 (left arm)
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B11000, B0, B0, B0, B0, B0, B0, B0}
}, 
{ //Hangman 5 (right arm)
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B00011, B0, B0, B0, B0, B0, B0, B0}
}, 
{ //Hangman 6 (left leg)
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B01000, B01000, B01000, B0, B0}
}, 
{ //Hangman 7 (right leg)
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B00010, B00010, B00010, B0, B0}
}, 
{ //Hangman 8 (game over)
   { B00111, B0, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0},
   { B11100, B00100, B0, B0, B0, B0, B0, B0},
   { B0, B0, B0, B0, B0, B0, B0, B0}
}};

//Stores possible words used in the hangman game
const PROGMEM String randomWords[4][6] = {{
  "CHICKEN", "SEAGULL", "BUFFALO", "OCTOPUS", "FOX", "FLY" //First category: Animals
}, {
  "APPLE","ORANGE","COCONUT","PRUNE","PLUM","KIWI" //Second category: Fruit
}, {
  "CANADA","FRANCE","THAILAND","LUXEMBOURG","CHILE","LAOS" //Third category: Countries
}, {
  "ARDUINO","SWEATER","FAINT","JAYWALK","DIZZY","JAZZ" //Fourth category: Random
}};

//Pin numbers
const PROGMEM byte LEFT_BUTTON = 10; 
const PROGMEM byte SELECT_BUTTON = 9;
const PROGMEM byte RIGHT_BUTTON = 8;
const PROGMEM byte BUZZER = 13;

byte hangman[4][8]; //Stores how the hangman will look on the lcd
int mistakes = 0; //Counts number of mistakes
String wordToGuess = "RANDOM"; //Stores what word will be used for the game
char currentChar = 'A'; //Stores which letter the user is hovering over
boolean isGuessed[26]; //Boolean array stores which letter has been guessed and which hasn't

//Basic setup + getting user to choose a category and difficulty
void setup(){
  lcd.begin(16, 2);
  Serial.begin(9600);
  start(); //Get user input for word to guess
  lcd.clear();
  delay(1000); //Delay before starting game in case player is still holding button
}


void loop(){
  //If the player is pressing left and their current letter is 'B' or higher
  if (digitalRead(LEFT_BUTTON) == HIGH && currentChar > 65){ 
    currentChar--; //Go back a letter
    delay(200); //Delay in case they are still holding the button
  }
  
  //Pressing right, and their current letter is 'Y' or lower
  else if (digitalRead(RIGHT_BUTTON) == HIGH && currentChar < 90){
    currentChar++; //Go forward a letter
    delay(200); //Delay in case they are still holding the button
  }

  //Select button is pressed and the letter they're on hasn't been guessed yet
  else if (digitalRead(SELECT_BUTTON) == HIGH && !isGuessed[asciiToIndex(currentChar)]){ 
    
    if (wordToGuess.indexOf(currentChar) == -1){ //Letter isn't in word
      mistakes++;
      tone(BUZZER, 200, 500); //Indicates incorrect
    }
    else { //Letter is in word
      tone(BUZZER, 600, 500); //Indicates correct
    }
    isGuessed[asciiToIndex(currentChar)] = true; //Set corresponding boolean value for that letter to true

    if (hasWon()){ //Check for win
      //Show that they got all the letters before ending
      drawLetters();
      drawWord();
      delay(1000);
      gameOver(true); 
    }

    else if (mistakes == 7){ //Check for lose
      gameOver(false);
    }
  }
  //Repeatedly draw and update the 3 components of the game
  drawMan(); //Hangman
  drawLetters(); //Letters from A-Z
  drawWord(); //Word to guess
}

//Gets user input for category and difficulty and sets 
void start(){
  boolean valid = false; //Stores whether or not a word has been chosen
  boolean inCategorySelect = true; //Stores whether player is selecting categories or difficulties
  int counter = 0; //Keeps track of which position the user is at
  int category = -1; //Stores selected category
  int difficulty = -1; //Stores selected difficulty
  
  printStart(inCategorySelect, counter); //Print to lcd

  while (!valid){ //Stay in this loop until a word has been chosen
    
    //Read input
    if (digitalRead(LEFT_BUTTON) == HIGH){ //Left is pressed
      counter--;
      printStart(inCategorySelect, counter); //Print to lcd
    }

    else if (digitalRead(RIGHT_BUTTON) == HIGH) { //Right is pressed
      counter++;
      printStart(inCategorySelect, counter); //Print to lcd
    }
    else if (digitalRead(SELECT_BUTTON) == HIGH){ //Select is pressed
      if (inCategorySelect){ //Selecting category
        inCategorySelect = false;
        
        //Counter may be negative, so I add a large number before the mod 4 to avoid a negative output (-4 % 3 = -1)
        //4 and 40000000 because there are 4 possible categories
        category = (counter + 4000000) % 4; 
        printStart(inCategorySelect, counter); //Print to lcd
        delay(1000); //Delay in case player is still holding button
      }
      else { //Selecting difficulty
        valid = true;
        //See lines 170 and 171, there are 3 possible difficulties
        difficulty = (counter + 3000000) % 3; 
        
        //If difficulty was 0, randomDifficulty could be 0 or 1, if difficulty was 1, randomDifficulty could be 2 or 3, etc.
        int randomDifficulty = ((difficulty + 1) * random(1, 3)) - 1; 
        
        //Set random word based on user input
        wordToGuess = randomWords[category][randomDifficulty];
        Serial.println(wordToGuess);
      }
    } //End if select is pressed
  } //End while loop
} //End method

/* Prints to the lcd for the start() method
 * param inCategorySelect: true if selecting category, false if selecting difficulty
 * param counter: counter for position of cursor
 */
void printStart(boolean inCategorySelect, int counter){
    lcd.clear();

    if (inCategorySelect){ //Selecting category
      lcd.setCursor(0, 0);
      lcd.print(F("Category:"));
      lcd.setCursor(0, 1);
      switch ((counter + 4000000) % 4){ //See lines 170 and 171 for explanation
        case 0:
        lcd.print(F("1. Animals"));
        break;
        case 1:
        lcd.print(F("2. Fruits"));
        break;
        case 2:
        lcd.print(F("3. Countries"));
        break;
        case 3:
        lcd.print(F("4. Random"));
        break;
      }
    }
    else { //Selecting difficulty
      lcd.setCursor(0, 0);
      lcd.print(F("Difficulty:"));
      lcd.setCursor(0, 1);
      switch ((counter + 3000000) % 3){ //See lines 170 and 171 for explanation
        case 0:
        lcd.print(F("     Easy"));
        break;
        case 1:
        lcd.print(F("    Medium"));
        break;
        case 2:
        lcd.print(F("     Hard"));
        break;
      }
      
    }
    delay(200); //Slow down reaction to left/right button
  }


//Draws hangman on LCD
void drawMan(){
  
  //Add a piece of hangman to byte hangman[4][8]
  for (int i = 0; i < 4; i++){
    for (int j = 0; j < 8; j++){
      hangman[i][j] |= hangmanPieces[mistakes][i][j]; //Use bitwise AND to add a piece depending on how many mistakes were made so far
    }
    lcd.createChar(i, hangman[i]); //Create 4 custom characters, one for each bit of the hangman
  }

  //Draw custom characters
  lcd.setCursor(0, 0);
  lcd.write(byte(0));
  lcd.setCursor(0, 1);
  lcd.write(1);
  lcd.setCursor(1, 0);
  lcd.write(2);
  lcd.setCursor(1, 1);
  lcd.write(3);
}

//Draws letters on LCD (A to Z)
void drawLetters(){

  //Cursor for current character
  byte rightArrow[8] = {
    0b00000,
    0b00100,
    0b00110,
    0b11111,
    0b11111,
    0b00110,
    0b00100,
    0b00000
  };
  byte leftArrow[8] = {
    0b00000,
    0b00100,
    0b01100,
    0b11111,
    0b11111,
    0b01100,
    0b00100,
    0b00000
  };
  lcd.createChar(6, rightArrow);
  lcd.createChar(7, leftArrow);

  //Draw 9 letters on LCD
  lcd.setCursor(3, 1);
  lcd.print(getLetter(currentChar - 4));
  lcd.print(getLetter(currentChar - 3));
  lcd.print(getLetter(currentChar - 2));  
  lcd.print(getLetter(currentChar - 1));    
  lcd.write(6); //Right arrow
  lcd.print(getLetter(currentChar));
  lcd.write(7); //Left arrow
  lcd.print(getLetter(currentChar + 1));
  lcd.print(getLetter(currentChar + 2));
  lcd.print(getLetter(currentChar + 3));
  lcd.print(getLetter(currentChar + 4));
  
}

//Returns the appropriate letter to print for the drawLetters() method
char getLetter(int ascii){
  
  //Valid letter
  if (ascii >= 65 && ascii <= 90){
      if (isGuessed[asciiToIndex(ascii)]){ //Already guessed
        return 254; //Returns blank TODO replace with another
      }
      else { //Not already guessed
        return char(ascii);
      }
  }
  else //Invalid letter (before A, or after Z)
    return 255; //Returns block
  
}

//Prints the word to the LCD
void drawWord(){
    //Set cursor to center the word (8 minus half the word length)
    lcd.setCursor(8 - wordToGuess.length()/2, 0); 

    for (int i = 0; i < wordToGuess.length(); i++){ //For each letter in word
      if (isGuessed[asciiToIndex(wordToGuess.charAt(i))]) //If guessed, print the letter
        lcd.print(wordToGuess.charAt(i));
      
      else  //Print underscore if not guessed
        lcd.print(char(95));
    }
}

//Converts from ascii to a system of 0 to 25 for the boolean[26]
char asciiToIndex(int ascii){
  return ascii-65;
}

//Checks if user has won
boolean hasWon(){
  for (int i = 0; i < wordToGuess.length(); i++){
    if (!isGuessed[asciiToIndex(wordToGuess.charAt(i))]){
      return false;
    }
  }
  return true;
}

/* Prints to lcd for either win or lose conditions
 * param win: true if won, false if lost
 */
void gameOver(boolean win){

  //Play a melody
  if (win){
    tone(BUZZER, C_NOTE, 500);
    delay(600);
    tone(BUZZER, C_NOTE, 250);
    delay(350);
    tone(BUZZER, F_NOTE, 1500);
  }
  else {
    tone(BUZZER, F_NOTE, 500);
    delay(600);
    tone(BUZZER, E_NOTE, 500);
    delay(600);
    tone(BUZZER, Eb_NOTE, 500);
    delay(600);
    tone(BUZZER, D_NOTE, 2000);
    delay(600);
  }

  //Flash and print win/lose on LCD 5 times
  for (int i = 0; i < 5; i++){
    lcd.clear();
    delay(300);
    
    if (win) {
      drawWord();
      lcd.setCursor(4, 1);
      lcd.print(F("YOU WIN! :)"));
    }
    else {
      drawMan(); 
      lcd.setCursor(6, 0);
      lcd.print(F("GAME")); 
      lcd.setCursor(6, 1);
      lcd.print(F("OVER"));
    }

    delay(300);
  }

  //Exit program
  delay(7000); 
  exit(0);
}
