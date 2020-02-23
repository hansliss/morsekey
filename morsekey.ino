#define SPEAKER_PIN 3
#define KEY_PIN 2

#undef DEBUG

struct xtab_struct {
  char c;
  char *x;
} xtab[]=
  {
    {'"', ".-..-."},
    {'\'', ".----."},
    {'(', "-.--.-"},
    {')', "-.--.-"},
    {',', "--..--"},
    {'-', "-....-"},
    {'.', ".-.-.-"},
    {'/', "-..-."},
    {'&', ".-..."},
    {'=', "-...-"},
    {':', "---..."},
    {';', "-.-.-"},
    {'+', ".-.-."},
    {'_', "..--.-"},
    {'$', "...-..-"},
    {'@', ".--.-."},
    {'0', "-----"},
    {'1', ".----"},
    {'2', "..---"},
    {'3', "...--"},
    {'4', "....-"},
    {'5', "....."},
    {'6', "-...."},
    {'7', "--..."},
    {'8', "---.."},
    {'9', "----."},
    {':', "---..."},
    {'?', "..--.."},
    {'A', ".-"},
    {'B', "-..."},
    {'C', "-.-."},
    {'D', "-.."},
    {'E', "."},
    {'F', "..-."},
    {'G', "--."},
    {'H', "...."},
    {'I', ".."},
    {'J', ".---"},
    {'K', "-.-"},
    {'L', ".-.."},
    {'M', "--"},
    {'N', "-."},
    {'O', "---"},
    {'P', ".--."},
    {'Q', "--.-"},
    {'R', ".-."},
    {'S', "..."},
    {'T', "-"},
    {'U', "..-"},
    {'V', "...-"},
    {'W', ".--"},
    {'X', "-..-"},
    {'Y', "-.--"},
    {'Z', "--.."},
    {'Á', ".--.-"},
    {'Ä', ".-.-"},
    {'Å', ".--.-"},
    {'É', "..-.."},
    {'Ñ', "--.--"},
    {'Ö', "---."},
    {'Ü', "..--"},
    {'[', "-.-.-"},
    {'§', "...---..."}
  };

#define NCODES (sizeof(xtab)/sizeof(struct xtab_struct))

void setup() {
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(KEY_PIN, INPUT_PULLUP);
  Serial.begin(115200);
}

#define NSAVED 10
int history[NSAVED];
long sum=0;
int count=0;
int average=0;
static unsigned long currentChar=0;
static int currentLen=0;

boolean handleItem(int duration) {
  boolean isDot = false;
  float deviation = 0;
  if (average > 0 && count > 0) {
    deviation = ((float)abs(duration-average) / (float)average);
    if (deviation < 2) {
      if (deviation > 0.6) {
#ifdef DEBUG
        Serial.println("This item deviates from the average");
#endif        
        if (duration > average) {
          duration /= 3;
#ifdef DEBUG
          Serial.println("It's a long interval (dash/character break)");
#endif
        } else {
#ifdef DEBUG
          Serial.println("It's shorter than the average. Recalculating");
#endif
          sum = 0;
          // All the previous events were dashes, not dots, so we divide them by
          // three to get them to the base 1U length
          for (int i=0; i<count; i++) {
            history[i] /= 3;
            sum += history[i];
          }
          average = sum / count;
          // We have to assume the data resembles Morse code at least a bit,
          // which means at this point we haven't encountered any actual dots even though we
          // thought we did. Thus, we need to convert the existing, assumed dots to dashes.
          for (int i=0; i<currentLen; i++) {
            currentChar |= (1 << i);
          }
          isDot = true;
        }
      } else {
        // Note that the baseline for the average is 1U i.e. a dot. If we're not deviating
        // from the average, we're a dot.
        isDot = true;
      }
    }
  }
  if (deviation < 2) {
    if (count == NSAVED) {
      sum -= history[0];
      for (int i=0; i<count - 1; i++) {
        history[i] = history[i+1];
      }
      history[NSAVED-1] = duration;
    } else {
      history[count++] = duration;
    }
    sum += duration;
    average = sum / count;
#ifdef DEBUG
    Serial.print("Average is " + String(average) + ", History: ");
    for (int i=0; i<count; i++) {
      Serial.print(String(history[i]) + ", ");
    }
    Serial.println();
#endif    
  }
  return isDot;
}

void printChar(boolean inWord) {
  if (currentLen == 0) {
    return;
  }
  static String word = "";
  String s = "";
  for (int i=0; i<currentLen; i++) {
    s += (currentChar & (1 << i))?"-":".";
  }
  char c = '?';
  for (int i=0; i<NCODES; i++) {
    if (!strcmp(s.c_str(), xtab[i].x)) {
      c = xtab[i].c;
      break;
    }
  }
  Serial.print(s + " (" + c + ") ");
  word += c;
  if (!inWord) {
    Serial.println(":  " + word);
    word = "";
  }
}

void loop() {
  static int cstate=0;
  static long timer=0;
  long duration = millis() - timer;
  if (!digitalRead(KEY_PIN)) {
    if (!cstate) {
      if (duration > 20) {
        tone(SPEAKER_PIN, 440);
        timer=millis();
        cstate=1;
        //Serial.println("space for " + String(duration) + "ms");
        if (duration > 2000 || !handleItem(duration)) {
          printChar(true);
          currentLen = 0;
          currentChar = 0;
        }
      }
    }
  } else {
    if (cstate) {
      long duration = millis() - timer;
      if (duration > 20) {
        noTone(SPEAKER_PIN);
        //Serial.println("mark for " + String(duration) + "ms");
        timer=millis();
        cstate=0;
        boolean isDot = handleItem(duration);
        if (isDot) {
          currentChar &= ~(1L << currentLen);         
        } else {
          currentChar |= (1L << currentLen);
        }
        currentLen++;
      }
    } else if (duration > 2000 && currentLen > 0) {
      printChar(false);
      currentLen = 0;
      currentChar = 0;
      count = 0;
      sum = 0;
      average = 0;
    }
  }
}
