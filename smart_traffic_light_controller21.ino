#define EMERGENCY 6

int densityBtn[4] = {2, 3, 4, 5};

int red[] = {8, 11, A0, A3};
int yellow[] = {9, 12, A1, A4};
int green[] = {10, 13, A2, A5};

unsigned long greenTime = 3000;
unsigned long yellowTime = 1000;
unsigned long allRedTime = 500;
unsigned long redYellowTime = 1000;

int currentRoad = 0;
int nextRoad = 0;

unsigned long previousMillis = 0;

enum State { GREEN_STATE, YELLOW_STATE, ALL_RED_STATE, RED_YELLOW_STATE };
State currentState = GREEN_STATE;

enum Mode { CYCLIC, DENSITY, EMERGENCY_MODE };
Mode currentMode = CYCLIC;

bool lastEmergencyState = HIGH;
bool lastDensityState[4] = {HIGH, HIGH, HIGH, HIGH};

void setup() {
  Serial.begin(9600);

  Serial.println("SMART TRAFFIC LIGHT CONTROLLER");
  Serial.println("--------------------------------");

  pinMode(EMERGENCY, INPUT_PULLUP);

  for (int i = 0; i < 4; i++) {
    pinMode(densityBtn[i], INPUT_PULLUP);
    pinMode(red[i], OUTPUT);
    pinMode(yellow[i], OUTPUT);
    pinMode(green[i], OUTPUT);
  }

  activateGreen(currentRoad);
}

void loop() {

  unsigned long currentMillis = millis();

  // -------- EMERGENCY (Immediate Override) --------
  bool emergencyState = digitalRead(EMERGENCY);
  if (lastEmergencyState == HIGH && emergencyState == LOW) {

    currentMode = EMERGENCY_MODE;
    nextRoad = 0;

    // FORCE IMMEDIATE YELLOW
    activateYellow(currentRoad);
    currentState = YELLOW_STATE;
    previousMillis = millis();
  }
  lastEmergencyState = emergencyState;

  // -------- DENSITY (Immediate Override) --------
  for (int i = 0; i < 4; i++) {

    bool currentDensity = digitalRead(densityBtn[i]);

    if (lastDensityState[i] == HIGH && currentDensity == LOW) {

      currentMode = DENSITY;
      nextRoad = i;

      // FORCE IMMEDIATE YELLOW
      activateYellow(currentRoad);
      currentState = YELLOW_STATE;
      previousMillis = millis();
    }

    lastDensityState[i] = currentDensity;
  }

  // -------- STATE MACHINE --------

  switch (currentState) {

    case GREEN_STATE:
      if (currentMillis - previousMillis >= greenTime) {
        nextRoad = (currentRoad + 1) % 4;
        currentMode = CYCLIC;

        activateYellow(currentRoad);
        currentState = YELLOW_STATE;
        previousMillis = millis();
      }
      break;

    case YELLOW_STATE:
      if (currentMillis - previousMillis >= yellowTime) {

        digitalWrite(yellow[currentRoad], LOW);
        digitalWrite(red[currentRoad], HIGH);

        currentState = ALL_RED_STATE;
        previousMillis = currentMillis;
      }
      break;

    case ALL_RED_STATE:
      if (currentMillis - previousMillis >= allRedTime) {

        digitalWrite(red[nextRoad], HIGH);
        digitalWrite(yellow[nextRoad], HIGH);

        currentState = RED_YELLOW_STATE;
        previousMillis = currentMillis;
      }
      break;

    case RED_YELLOW_STATE:
      if (currentMillis - previousMillis >= redYellowTime) {

        digitalWrite(yellow[nextRoad], LOW);
        activateGreen(nextRoad);

        currentRoad = nextRoad;
        currentState = GREEN_STATE;
        previousMillis = currentMillis;
      }
      break;
  }
}

void activateGreen(int road) {

  allRed();

  digitalWrite(red[road], LOW);
  digitalWrite(green[road], HIGH);

  Serial.print(getModeName());
  Serial.print(" MODE : ROAD ");
  Serial.print(road + 1);
  Serial.println(" GREEN");

  previousMillis = millis();
}

void activateYellow(int road) {
  digitalWrite(green[road], LOW);
  digitalWrite(yellow[road], HIGH);
}

void allRed() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(red[i], HIGH);
    digitalWrite(yellow[i], LOW);
    digitalWrite(green[i], LOW);
  }
}

String getModeName() {
  if (currentMode == CYCLIC) return "CYCLIC";
  if (currentMode == DENSITY) return "DENSITY";
  if (currentMode == EMERGENCY_MODE) return "EMERGENCY";
  return "";
}
