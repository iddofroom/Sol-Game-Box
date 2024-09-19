#include <FastLED.h>

#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

#include "SoftwareSerial.h"
#define NUM_LEDS 16
#define BRIGHTNESS 64
#define COLOR_CHANGE_SPEED 10

#if (defined(ARDUINO_AVR_UNO) || defined(ESP8266))   // Using a soft serial port
#define FPSerial softSerial
#else
#define FPSerial Serial1
#endif

SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

    unsigned long lastInteraction = millis();

CRGB leds[NUM_LEDS];
const int ledPins[NUM_LEDS] = {22,24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50,52};
const int buttonPins[NUM_LEDS] = {23,25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51,53};
const int succsec[2] = {30,31};

bool buttonState[NUM_LEDS];
bool lastButtonState[NUM_LEDS];
uint8_t colorHue[NUM_LEDS];
bool sleep = false;
  bool gameon = false;
  bool MainMenu = false;
int MinSucces = 30;
int MaxSucces = 31;

//----------------- משחק צביעה
int currentColorIndex[NUM_LEDS];  // עוקב אחר הצבע הנוכחי עבור כל כפתור

  //---------------משתנים משחק זכרון
  bool isQuadMode = true; // מצב רבעיות - יהיה נכון בשני השלבים הראשונים
int quadColors[4];      // צבעים עבור ארבעת הקבוצות
  CRGB colors[8] = {CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow, CRGB::Purple, CRGB(125,255,0), CRGB(100,255,0), CRGB::White};
  int buttonColors[NUM_LEDS];    // שמירת צבע לכל כפתור
  bool matched[NUM_LEDS];        // האם הכפתור הזה כבר נמצא לו זוג
  int firstButton = -1;          // הכפתור הראשון שנלחץ בזוג
  int secondButton = -1;         // הכפתור השני שנלחץ בזוג
  int difficultyLevel = 1;       // דרגת הקושי הנוכחית
  unsigned long lastPressTime = 0;  // זמן הלחיצה האחרונה
  unsigned long delayTime[] = {3000, 2000, 1000};  // זמני כיבוי לאחר לחיצה לפי דרגת קושי
  
  //------------------------------------משתנים לסיימון אמר
  int sequence[100];         // שמירת רצף הצלילים/צבעים בכל סיבוב
int userSequence[100];     // שמירת הלחיצות של השחקן
int sequenceLength = 1;    // אורך הרצף הנוכחי
int currentStep = 0;       // השלב הנוכחי
int speed = 1000;          // מהירות הצגת הצלילים במילי-שניות (התחלה ב-1000ms)
int maxButtons = 4;        // מספר הכפתורים הפעילים (התחלה ב-4)
unsigned long lastActionTime = 0;  // זמן הפעולה האחרונה כדי להפעיל את BLABLA
bool gameOver = false;     // מצב המשחק אם השחקן נפסל
//-----------------------------------------------------משתנים למשחק מהירות
CRGB screenColor = CRGB::White; // צבע המסך הנוכחי (תחילה לבן)
CRGB targetColor = CRGB::Red;   // צבע הכפתור השונה (תחילה אדום)
int stage = 1;                  // שלב המשחק הנוכחי
int maxStages = 5;              // מספר שלבים לפני שנוסף כפתור נוסף
int differentButton = 0;        // הכפתור השונה בצבעו
int currentButtons = 1;         // מספר הכפתורים הנוכחיים הפעילים

//---------------------------------------------------------------------------- SETUP
void setup() {
   mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms

  //----Set volume----
  myDFPlayer.volume(30);  //Set volume value (0~30).
  
  //----Mp3 play----
 // myDFPlayer.pause();  //pause the mp3
 // myDFPlayer.playMp3Folder(1); //play specific mp3 in SD:/MP3/0001.mp3; File Name(0~65535)
  
  // הגדרת פינים ללדים
  FastLED.addLeds<WS2812B,22,GRB>(leds,  0, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,24,GRB>(leds,  1, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,26,GRB>(leds,  2, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,28,GRB>(leds,  3, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,30,GRB>(leds,  4, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,32,GRB>(leds,  5, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,34,GRB>(leds,  6, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,36,GRB>(leds,  7, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,38,GRB>(leds,  8, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,40,GRB>(leds,  9, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,42,GRB>(leds,  10, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,44,GRB>(leds,  11, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,46,GRB>(leds,  12, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,48,GRB>(leds,  13, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,50,GRB>(leds,  14, 1).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<WS2812B,52,GRB>(leds,  15, 1).setCorrection(TypicalLEDStrip);
  for (int i = 0; i < NUM_LEDS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    buttonState[i] = false;
    lastButtonState[i] = false;
    colorHue[i] = 0;
  }

  // אנימציה קצרה בכל הלדים
  myDFPlayer.playMp3Folder(1);
  
  
  displayFinalButtons();
}


void loop() {
  
}

//--------------------------------------------------------------------------------------Mane Menu - 0

void displayFinalButtons() {
  MainMenu = true;
  
  while (true && !gameon) {
      if (sleep)
      {
        CheckIfAweak();
      }
      else
      {
  leds[0] = CRGB::Green;    // כפתור 0 - אדום
  leds[4] = CRGB::Red;  // כפתור 4 - ירוק
  leds[8] = CRGB::Yellow; // כפתור 8 - צהוב
  leds[12] = CRGB::Blue;  // כפתור 12 - כחול
  FastLED.show();
        if (digitalRead(buttonPins[0]) == LOW) {
      Serial.println(" red press");
      gameon = true;
      MainMenu=false;
      myDFPlayer.playMp3Folder(2);
      playColoringGame(); // מעבר למשחק צביעה
    } else if (digitalRead(buttonPins[4]) == LOW) {
      Serial.println(" green press");
      gameon = true;
      MainMenu=false;
      myDFPlayer.playMp3Folder(3);
      playMemoryGame(); // מעבר למשחק זיכרון
    } else if (digitalRead(buttonPins[8]) == LOW) {
      Serial.println(" yellow press");
      gameon = true;
      MainMenu=false;
      playSpeedGame(); // מעבר למשחק מהירות
    } else if (digitalRead(buttonPins[12]) == LOW) {
      Serial.println(" blue press");
      gameon = true;
      MainMenu=false;
      playMusicGame(); // מעבר למשחק השלמת נגינה
    } 
    if (gameon){ResetTime();}
   
    CheakTime();
      }       
  }
}
//--------------------------------------------------------------------------------------Color Game - 1

void startFirstGame() {
  for (int i = 0; i < NUM_LEDS; i++) {
    currentColorIndex[i] = -1;  // כל כפתור מתחיל בצבע שחור (שווה ל- -1)
    leds[i] = CRGB::Black;      // כיבוי כל הלדים
  }
  FastLED.show();
}

void playFirstGame() {
  startFirstGame();  // אתחול המשחק

  while (true) {
    // בדיקת לחיצות כפתורים
    for (int i = 0; i < NUM_LEDS; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {  // אם הכפתור נלחץ
        delay(200);  // מניעת קריאה כפולה עקב לחיצה מהירה

        // עדכון הצבע הנוכחי
        currentColorIndex[i] = (currentColorIndex[i] + 1) % 9;  // מעבר לצבע הבא או לשחור (9 אפשרויות כולל שחור)

        if (currentColorIndex[i] == 8) {
          leds[i] = CRGB::Black;  // צבע שחור לאחר הלחיצה ה-8
        } else {
          leds[i] = colors[currentColorIndex[i]];  // מעבר לצבע הבא
        }

        FastLED.show();
      }
    }
  }
}



void playColoringGame()
{  
  sleep = false;
  unsigned long lastInteraction2 = millis();
  Serial.print("playColoringGame ");
  shortAnimation(15);
  allblack();
  Serial.print("allblack ");
  while(gameon)
  { 
    for (int i = 0; i < NUM_LEDS; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {  // אם הכפתור נלחץ
       myDFPlayer.playMp3Folder(10 + i);
        ResetTime();
        delay(350);  // מניעת קריאה כפולה עקב לחיצה מהירה
        // עדכון הצבע הנוכחי
        currentColorIndex[i] = (currentColorIndex[i] + 1) % 9;  // מעבר לצבע הבא או לשחור (9 אפשרויות כולל שחור)
        if (currentColorIndex[i] == 8) {
          leds[i] = CRGB::Black;  // צבע שחור לאחר הלחיצה ה-8
        } else {
          leds[i] = colors[currentColorIndex[i]];  // מעבר לצבע הבא
        }
        FastLED.show();
      }
  }
  CheakTime();
  }
}
//------------------------------------------------------------Memory Game - 2-------------------------------------------------------

void playMemoryGame() {
  
  shortAnimation(15);
  allblack();
  startMemoryGame();
  
  while (true) {
// בדיקת לחיצות כפתורים
    for (int i = 0; i < NUM_LEDS; i++) {
      if (digitalRead(buttonPins[i]) == LOW && !matched[i]) {
        if (firstButton == -1) {
          firstButton = i;  // הכפתור הראשון שנלחץ
          leds[i] = colors[buttonColors[i]];  // הדלקת הלד בצבע שנבחר
          FastLED.show();
        } else if (secondButton == -1 && i != firstButton) {
          secondButton = i;  // הכפתור השני שנלחץ
          leds[i] = colors[buttonColors[i]];  // הדלקת הלד בצבע שנבחר
          FastLED.show();

          // בדיקת התאמה בין הכפתורים
          if (buttonColors[firstButton] == buttonColors[secondButton]) {
            // זוג מתאים - הבהוב 4 פעמים והשארת הכפתורים דלוקים
            for (int j = 0; j < 4; j++) {
              leds[firstButton] = leds[secondButton] = CRGB::Black;
              FastLED.show();
              delay(125);
              leds[firstButton] = leds[secondButton] = colors[buttonColors[firstButton]];
              FastLED.show();
              delay(125);
            }
            matched[firstButton] = true;
            matched[secondButton] = true;
            myDFPlayer.playMp3Folder(getRandomNumber(MinSucces,MaxSucces));//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
          } else {
            // זוג לא מתאים - הכיבוי תלוי ברמת הקושי
            delay(delayTime[difficultyLevel - 1]);
            leds[firstButton] = leds[secondButton] = CRGB::Black;
            FastLED.show();
          }

          // איפוס לחיצות
          firstButton = -1;
          secondButton = -1;

          // עדכון זמן לחיצה אחרונה
          lastPressTime = millis();

          // בדיקת אם השחקן מצא את כל הצבעים
          if (checkIfGameComplete()) {
            delay(1000);  // המתנה קטנה לפני עליית דרגה
            if (difficultyLevel < 3) {
              difficultyLevel++;
              resetLevel();  // מעבר לשלב הבא
            } else {
              resetLevel();  // בדרגה 3 ניתן לשחק אינסוף
            }
          }
        }
      }
    }
    // אם עברו 15 שניות בלי לחיצה, סיום המשחק עם אנימציה
    if (millis() - lastPressTime > 15000) {
      endGameAnimation();
        asm volatile ("  jmp 0");  // איפוס המערכת
      break;
    }
  }
}
void resetLevel() {
  // איפוס לחיצות
  firstButton = -1;
  secondButton = -1;

  // איפוס צבעים ומצבים
  for (int i = 0; i < NUM_LEDS; i++) {
    matched[i] = false;  // איפוס המצב של הכפתורים שנמצאו
    leds[i] = CRGB::Black;  // כיבוי כל הלדים
  }
  FastLED.show();

  // אתחול המשחק מחדש עבור השלב הבא
  startMemoryGame();
}

//----------------------------------------------startMemoryGame
void startMemoryGame() {
  randomSeed(analogRead(0));  // התחלה רנדומלית

  if (difficultyLevel <= 2) {
    // שני השלבים הראשונים עם 4 רבעיות
    isQuadMode = true;
    int colorIndexes[NUM_LEDS];

    // בחירת 4 צבעים רנדומליים עבור הרבעיות
    for (int i = 0; i < 4; i++) {
      quadColors[i] = i;  // שמירת צבע לכל רבעיה
    }

    // כל צבע חוזר 4 פעמים במערך
    for (int i = 0; i < NUM_LEDS; i++) {
      colorIndexes[i] = i / 4;
    }

    // ערבוב צבעי הרבעיות
    for (int i = 0; i < NUM_LEDS; i++) {
      int swapWith = random(NUM_LEDS);
      int temp = colorIndexes[i];
      colorIndexes[i] = colorIndexes[swapWith];
      colorIndexes[swapWith] = temp;
    }

    // שמירת הצבעים לכפתורים
    for (int i = 0; i < NUM_LEDS; i++) {
      buttonColors[i] = colorIndexes[i];
      matched[i] = false;  // איפוס הזוגות שנמצאו
    }

  } else {
    // שלבים רגילים עם 8 זוגות
    isQuadMode = false;
    int colorIndexes[NUM_LEDS];

    // מילוי המערך בצבעים רנדומליים עבור 8 זוגות
    for (int i = 0; i < NUM_LEDS / 2; i++) {
      colorIndexes[i * 2] = i;
      colorIndexes[i * 2 + 1] = i;
    }

    // ערבוב הצבעים
    for (int i = 0; i < NUM_LEDS; i++) {
      int swapWith = random(NUM_LEDS);
      int temp = colorIndexes[i];
      colorIndexes[i] = colorIndexes[swapWith];
      colorIndexes[swapWith] = temp;
    }

    // שמירת הצבעים לכפתורים
    for (int i = 0; i < NUM_LEDS; i++) {
      buttonColors[i] = colorIndexes[i];
      matched[i] = false;  // איפוס הזוגות שנמצאו
    }
  }

  firstButton = -1;
  secondButton = -1;
  lastPressTime = millis();  // אתחול זמן הלחיצה האחרונה
}


bool checkIfGameComplete() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (!matched[i]) {
      return false;
    }
  }
  return true;
}
void endGameAnimation() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  if (difficultyLevel == 1) {
    // אנימציה לשלב ראשון
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = CRGB::Red;
      }
      FastLED.show();
      delay(500);
      for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = CRGB::Black;
      }
      FastLED.show();
      delay(500);
    }
  } else if (difficultyLevel == 2) {
    // אנימציה לשלב שני
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = CRGB::Green;
      }
      FastLED.show();
      delay(500);
      for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = CRGB::Black;
      }
      FastLED.show();
      delay(500);
    }
  } else {
    // אנימציה לשלב שלישי
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = CRGB::Blue;
      }
      FastLED.show();
      delay(500);
      for (int j = 0; j < NUM_LEDS; j++) {
        leds[j] = CRGB::Black;
      }
      FastLED.show();
      delay(500);
    }
  }
}
//--------------------------------------------------------------------------playMusicGame
void playMusicGame() {
  allblack();
  startMusicGame();  // אתחול המשחק

  while (!gameOver) {
    
      Serial.println("Game on");
    checkUserInput();  // בדיקת לחיצות שחקן
  
    // בדיקת זמן מאז הפעולה האחרונה
    if (millis() - lastActionTime > 15000) {
    asm volatile ("  jmp 0");  // איפוס המער
      break;
    }
  }
}
void gameOverSequence() {
  // אנימציית סיום משחק
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < maxButtons; j++) {
      leds[j] = CRGB::Red;
    }
    FastLED.show();
    delay(500);
    for (int j = 0; j < maxButtons; j++) {
      leds[j] = CRGB::Black;
    }
    FastLED.show();
    delay(500);
  }
  
  delay(2000);  // המתנה של 2 שניות לפני אתחול המשחק מחדש
  startMusicGame();  // אתחול המשחק מהתחלה
}

void nextStage() {
  sequenceLength++;  // הוספת צליל נוסף לרצף
  
  // הורדת המהירות בכל שלב עד שלב 10, ואז מהירות קבועה של 500ms
  if (sequenceLength <= 10) {
    speed -= 50;  // הורדת מהירות ב-50ms
  } else {
    speed = 500;  // מהירות קבועה מ-500ms ואילך
    if (maxButtons < NUM_LEDS) {
      maxButtons++;  // הוספת כפתורים לאחר שלב 10
    }
  }
  
  playSequence();  // שחק את הרצף החדש
}

void checkUserInput() {
  for (int i = 0; i < maxButtons; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {  // בדיקת לחיצה על כפתור
      
      Serial.println("Button on");
      leds[i] = colors[i];  // הדלקת הלד
     // myDFPlayer.playMp3Folder(i + 1); // ניגון הצליל בהתאם לכפתור
      FastLED.show();
      delay(500);
      leds[i] = CRGB::Black;  // כיבוי הלד
      FastLED.show();
      
      // השחקן לחץ על כפתור - בדיקת התאמה לרצף
      if (i == sequence[currentStep]) {
        currentStep++;
        lastActionTime = millis();  // עדכון זמן הפעולה האחרונה
        
        // אם השחקן השלים את הרצף
        if (currentStep == sequenceLength) {
          delay(1000);  // המתנה קטנה לפני העלאת שלב
          nextStage();  // מעבר לשלב הבא
        }
      } else {
        gameOver = true;  // השחקן נכשל
        gameOverSequence();  // הצגת אנימציית סוף משחק
        return;
      }
    }
  }
}

  void playSequence() {
  for (int i = 0; i < sequenceLength; i++) {
    int button = sequence[i];
    
    leds[button] = colors[button];  // הדלקת הלד לפי הצבע שנבחר
   // myDFPlayer.playMp3Folder(button + 1); // ניגון צליל בהתאם
    FastLED.show();
    delay(speed);  // המתנה לפי המהירות הנוכחית
    
    leds[button] = CRGB::Black;  // כיבוי הלד
    FastLED.show();
    delay(250);  // הפסקה בין הצלילים
  }
  
  currentStep = 0;  // אתחול שלב לחיצות השחקן
  lastActionTime = millis();  // עדכון זמן הפעולה האחרונה
}

 void startMusicGame() {
  randomSeed(analogRead(0));  // התחלה רנדומלית
  sequenceLength = 1;         // התחל עם צליל אחד ברצף
  speed = 1000;               // מהירות ראשונית
  maxButtons = 4;             // התחל עם 4 כפתורים
  gameOver = false;
  lastActionTime = millis();
  
  // אתחול הרצף הרנדומלי
  for (int i = 0; i < 100; i++) {
    sequence[i] = random(maxButtons);  // יצירת רצף רנדומלי
  }
  
  playSequence();  // שחק את הרצף הנוכחי
}

//-------------------------------------playSpeedGame
void playSpeedGame() {
  lastActionTime = 0;
  gameOver = false;
      Serial.println("playSpeedGame");
  startColorDifferenceGame();  // אתחול המשחק

  while (!gameOver) {
      Serial.println("!");
    checkUserInputColorGame();  // בדיקת לחיצות של השחקן

    // בדיקת אם עברו 15 שניות ללא לחיצה
    if (millis() - lastActionTime > 15000) {
      resetGame();  // איפוס המשחק
      break;
    }
  }
}

  
void startColorDifferenceGame() {
  allblack();
  screenColor = CRGB::White;  // אתחול המסך ללבן
  targetColor = CRGB::Green;    // אתחול הצבע השונה לאדום
  stage = 1;                  // התחלת המשחק בשלב הראשון
  currentButtons = 16;         // כפתור אחד בשלב הראשון
  differentButton = random(currentButtons);  // בחירת הכפתור השונה
  lastActionTime = millis();  // אתחול זמן הפעולה האחרונה
  gameOver = false;

  displayColors();  // הצגת הצבעים
}
void displayColors() {
  
  for (int i = 0; i < 16; i++) {
  FastLED.show();
     leds[i] = screenColor;
  }
  
  for (int i = 0; i < currentButtons; i++) {
    if (i == differentButton) {
      leds[i] = targetColor;  // הכפתור השונה יהיה בצבע האדום
    }
  }
  FastLED.show();
}
void checkUserInputColorGame() {
  for (int i = 0; i < currentButtons; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {  // בדיקת לחיצה על כפתור
      lastActionTime = millis();
      if (i == differentButton) {
        
       myDFPlayer.playMp3Folder(10 + i);
        advanceToNextStage();  // מעבר לשלב הבא אם הכפתור הנכון נלחץ
      } else {
       // gameOver = true;  // אם נלחץ כפתור לא נכון
       // resetGame();  // אתחול המשחק
      }
    }
  }
}
void advanceToNextStage() {
  /////////////////////////////////////////////////////////////////////////////////////////////////////////
  stage++;
  lastActionTime = millis();  // עדכון זמן הפעולה האחרונה

  // כל חמישה שלבים נוסף כפתור, והצבע הלבן יתאפס
  if (stage % 5 == 0) {
    screenColor = CRGB::White; // אתחול צבע המסך ללבן
  } else {
    // התקרבות בין הצבעים (הקטנת ההבדל בין המסך לאדום)
    screenColor.r += (targetColor.r - screenColor.r) / 5 ;
    screenColor.g += (targetColor.g - screenColor.g) / 5+ 10;
    screenColor.b += (targetColor.b - screenColor.b) / 5+ 10;
  }

  differentButton = random(currentButtons);  // בחירת הכפתור השונה מחדש
  displayColors();  // הצגת הצבעים החדשים
}
void resetGame() {
    asm volatile ("  jmp 0");  // איפוס המערכת
}


//---------------------------------------------------------- Genral Functions --------------------------------------------

int getRandomNumber(int minVal, int maxVal) {
  return random(minVal, maxVal + 1);  // החזרת מספר רנדומלי בטווח [minVal, maxVal]
}

void allblack()
{

  for (int i = 0; i < NUM_LEDS; i++) {
  leds[i] = CRGB(0,0,0);
  FastLED.show();
}
}
void shortAnimation(int timer) {
  for (int j = 0; j < 255; j++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CHSV(j, 255, BRIGHTNESS);
    }
    FastLED.show();
    delay(timer);
  }
  // כיבוי כל הלדים לאחר האנימציה
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  ResetTime();
}


void CheakTime(){
  if (millis() - lastInteraction > 10000) {  // לשנות ל30 שניות
    if (MainMenu)
    {
      sleep = true;
    }
    else
    {
    asm volatile ("  jmp 0");  // איפוס המערכת
    }
  }
  else {
    //  Serial.println(millis() - lastInteraction);
    
    }
}

void CheckIfAweak()
{
  allblack();
  FastLED.show();
  for (int i=0;i<16;i++)
  {
      if (digitalRead(buttonPins[i]) == LOW)
      {
         ResetTime();
         sleep = false;
       }
  }
}

void ResetTime()
{
        lastInteraction = millis(); // עדכון זמן אחרון של לחיצה
}
