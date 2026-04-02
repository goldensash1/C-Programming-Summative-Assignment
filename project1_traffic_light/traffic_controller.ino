/*
 * Adaptive Traffic Signal Manager
 * Controls dual junctions with vehicle sensing
 * Uses millis() for non-blocking execution
 *
 * NOTE: Enum and structs must be declared before usage
 */

// ════════════════════════════════════════════════════════
// PIN CONFIGURATION (LED pins for RGB traffic heads)
// ════════════════════════════════════════════════════════
#define RED1_PIN    2
#define YELLOW1_PIN 3
#define GREEN1_PIN  4
#define BUTTON1_PIN 8

#define RED2_PIN    5
#define YELLOW2_PIN 6
#define GREEN2_PIN  7
#define BUTTON2_PIN 9

// ════════════════════════════════════════════════════════
// TYPES
// ════════════════════════════════════════════════════════
enum SignalPhase {
  PHASE_RED,
  PHASE_YELLOW,
  PHASE_GREEN
};

struct Junction {
  int junctionId;
  int redLED;
  int yellowLED;
  int greenLED;
  int sensorBtn;

  SignalPhase activePhase;

  unsigned long phaseStart;
  unsigned long greenTime;
  unsigned long yellowTime;
  unsigned long redTime;

  int carsCurrentCycle;
  int carsTotal;

  bool emergencyMode;
  bool manualControl;
};

struct SystemMetrics {
  int totalCars;
  unsigned long bootTime;
  int manualSwitches;
  int emergencyTriggers;
};

// ════════════════════════════════════════════════════════
// GLOBALS
// ════════════════════════════════════════════════════════
Junction* junctionA = nullptr;
Junction* junctionB = nullptr;
SystemMetrics* metrics = nullptr;

unsigned long lastUpdatePrint = 0;
bool systemRunning = true;

// ════════════════════════════════════════════════════════
// FUNCTION DECLARATIONS
// ════════════════════════════════════════════════════════
void initPins(Junction* j);
void changePhase(Junction* j, SignalPhase next);
void processJunction(Junction* j);
void detectVehicle(Junction* j);
void serialHandler();

void showStatus();
void showFullReport();
void showMainMenu();
void showManualHelp(int id);
void displayPhase(SignalPhase p);

void triggerEmergency();
void rebootSystem();

// ════════════════════════════════════════════════════════
void setup() {
  Serial.begin(9600);
  Serial.println("Booting Traffic Signal Manager...");

  junctionA = new Junction;
  junctionB = new Junction;
  metrics   = new SystemMetrics;

  // Junction A
  junctionA->junctionId = 1;
  junctionA->redLED     = RED1_PIN;
  junctionA->yellowLED  = YELLOW1_PIN;
  junctionA->greenLED   = GREEN1_PIN;
  junctionA->sensorBtn  = BUTTON1_PIN;

  junctionA->activePhase = PHASE_GREEN;
  junctionA->phaseStart  = millis();

  junctionA->greenTime  = 10000;
  junctionA->yellowTime = 2000;
  junctionA->redTime    = 10000;

  junctionA->carsCurrentCycle = 0;
  junctionA->carsTotal        = 0;
  junctionA->emergencyMode    = false;
  junctionA->manualControl    = false;

  // Junction B
  junctionB->junctionId = 2;
  junctionB->redLED     = RED2_PIN;
  junctionB->yellowLED  = YELLOW2_PIN;
  junctionB->greenLED   = GREEN2_PIN;
  junctionB->sensorBtn  = BUTTON2_PIN;

  junctionB->activePhase = PHASE_RED;
  junctionB->phaseStart  = millis();

  junctionB->greenTime  = 10000;
  junctionB->yellowTime = 2000;
  junctionB->redTime    = 10000;

  junctionB->carsCurrentCycle = 0;
  junctionB->carsTotal        = 0;
  junctionB->emergencyMode    = false;
  junctionB->manualControl    = false;

  // Metrics
  metrics->totalCars        = 0;
  metrics->bootTime         = millis();
  metrics->manualSwitches   = 0;
  metrics->emergencyTriggers = 0;

  initPins(junctionA);
  initPins(junctionB);

  changePhase(junctionA, PHASE_GREEN);
  changePhase(junctionB, PHASE_RED);

  Serial.println("System initialized successfully.");
  showMainMenu();
}

// ════════════════════════════════════════════════════════
void loop() {
  if (systemRunning) {
    processJunction(junctionA);
    processJunction(junctionB);

    detectVehicle(junctionA);
    detectVehicle(junctionB);

    if (millis() - lastUpdatePrint >= 2000) {
      showStatus();
      lastUpdatePrint = millis();
    }
  }

  serialHandler();
}

// ════════════════════════════════════════════════════════
void initPins(Junction* j) {
  pinMode(j->redLED, OUTPUT);
  pinMode(j->yellowLED, OUTPUT);
  pinMode(j->greenLED, OUTPUT);
  pinMode(j->sensorBtn, INPUT_PULLUP);
}

// ════════════════════════════════════════════════════════
void changePhase(Junction* j, SignalPhase next) {
  digitalWrite(j->redLED, LOW);
  digitalWrite(j->yellowLED, LOW);
  digitalWrite(j->greenLED, LOW);

  // RED: only red on, GREEN: only green on
  if (next == PHASE_RED) {
    digitalWrite(j->redLED, HIGH);
  }
  else if (next == PHASE_GREEN) {
    digitalWrite(j->greenLED, HIGH);
  }
  // YELLOW: mix red + green on RGB LED
  else if (next == PHASE_YELLOW) {
    digitalWrite(j->redLED, HIGH);
    digitalWrite(j->greenLED, HIGH);
  }

  j->activePhase = next;
  j->phaseStart  = millis();
}

// ════════════════════════════════════════════════════════
void processJunction(Junction* j) {
  if (j->manualControl) return;

  unsigned long duration = millis() - j->phaseStart;

  if (j->activePhase == PHASE_GREEN && duration >= j->greenTime) {
    changePhase(j, PHASE_YELLOW);
  }
  else if (j->activePhase == PHASE_YELLOW && duration >= j->yellowTime) {
    changePhase(j, PHASE_RED);
  }
  else if (j->activePhase == PHASE_RED && duration >= j->redTime) {
    j->carsCurrentCycle = 0;
    changePhase(j, PHASE_GREEN);
  }
}

// ════════════════════════════════════════════════════════
void detectVehicle(Junction* j) {
  if (digitalRead(j->sensorBtn) == LOW) {

    if (j->activePhase == PHASE_GREEN) {
      j->carsCurrentCycle++;
      j->carsTotal++;
      metrics->totalCars++;

      Serial.print("Car sensed @ Junction ");
      Serial.print(j->junctionId);
      Serial.print(" | Cycle Count: ");
      Serial.print(j->carsCurrentCycle);
      Serial.print(" | Lifetime: ");
      Serial.println(j->carsTotal);

      // adaptive timing
      if (j->carsCurrentCycle > 8) {
        j->greenTime = 15000;
      } else if (j->carsCurrentCycle > 3) {
        j->greenTime = 12000;
      } else {
        j->greenTime = 10000;
      }

      delay(50);
    }
  }
}

// ════════════════════════════════════════════════════════
void serialHandler() {
  if (!Serial.available()) return;

  char cmd = Serial.read();
  while (Serial.available()) Serial.read();

  switch (cmd) {

    case '1':
      junctionA->manualControl = !junctionA->manualControl;
      metrics->manualSwitches++;
      Serial.println(junctionA->manualControl ? "Junction 1 manual ON" : "Junction 1 manual OFF");
      if (junctionA->manualControl) showManualHelp(1);
      break;

    case '2':
      junctionB->manualControl = !junctionB->manualControl;
      metrics->manualSwitches++;
      Serial.println(junctionB->manualControl ? "Junction 2 manual ON" : "Junction 2 manual OFF");
      if (junctionB->manualControl) showManualHelp(2);
      break;

    case 'R': case 'r':
      if (junctionA->manualControl) { changePhase(junctionA, PHASE_RED); Serial.println("J1 -> RED"); }
      if (junctionB->manualControl) { changePhase(junctionB, PHASE_RED); Serial.println("J2 -> RED"); }
      break;

    case 'G': case 'g':
      if (junctionA->manualControl) { changePhase(junctionA, PHASE_GREEN); Serial.println("J1 -> GREEN"); }
      if (junctionB->manualControl) { changePhase(junctionB, PHASE_GREEN); Serial.println("J2 -> GREEN"); }
      break;

    case 'Y': case 'y':
      if (junctionA->manualControl) { changePhase(junctionA, PHASE_YELLOW); Serial.println("J1 -> YELLOW"); }
      if (junctionB->manualControl) { changePhase(junctionB, PHASE_YELLOW); Serial.println("J2 -> YELLOW"); }
      break;

    case 'E': case 'e':
      triggerEmergency();
      break;

    case 'S': case 's':
      rebootSystem();
      break;

    case 'M': case 'm':
      showMainMenu();
      break;

    case 'V': case 'v':
      showFullReport();
      break;

    default:
      Serial.println("Unknown input. Press M for help.");
  }
}

// ════════════════════════════════════════════════════════
void showMainMenu() {
  Serial.println("\n=== CONTROL PANEL ===");
  Serial.println("1 -> Manual Mode J1");
  Serial.println("2 -> Manual Mode J2");
  Serial.println("R/G/Y -> Change Light");
  Serial.println("E -> Emergency Stop");
  Serial.println("S -> Restart System");
  Serial.println("V -> Statistics");
  Serial.println("M -> Menu");
}

// ════════════════════════════════════════════════════════
void showManualHelp(int id) {
  Serial.print("Manual Control Enabled for Junction ");
  Serial.println(id);
  Serial.println("Use: R / G / Y");
}

// ════════════════════════════════════════════════════════
void showStatus() {
  Serial.println("\n--- LIVE STATUS ---");

  Serial.print("J1: ");
  displayPhase(junctionA->activePhase);
  Serial.print(" | Cars: ");
  Serial.print(junctionA->carsCurrentCycle);

  Serial.print("   J2: ");
  displayPhase(junctionB->activePhase);
  Serial.print(" | Cars: ");
  Serial.print(junctionB->carsCurrentCycle);

  Serial.print("   Total: ");
  Serial.println(metrics->totalCars);
}

// ════════════════════════════════════════════════════════
void showFullReport() {
  Serial.println("\n--- SYSTEM REPORT ---");

  unsigned long up = (millis() - metrics->bootTime) / 1000;

  Serial.print("Uptime: ");
  Serial.print(up / 60);
  Serial.print("m ");
  Serial.print(up % 60);
  Serial.println("s");

  Serial.print("Total Cars: ");
  Serial.println(metrics->totalCars);

  Serial.print("Manual Toggles: ");
  Serial.println(metrics->manualSwitches);

  Serial.print("Emergency Events: ");
  Serial.println(metrics->emergencyTriggers);
}

// ════════════════════════════════════════════════════════
void displayPhase(SignalPhase p) {
  if (p == PHASE_RED) Serial.print("RED");
  else if (p == PHASE_YELLOW) Serial.print("YEL");
  else Serial.print("GRN");
}

// ════════════════════════════════════════════════════════
void triggerEmergency() {
  Serial.println("\n!!! EMERGENCY MODE !!!");

  metrics->emergencyTriggers++;

  changePhase(junctionA, PHASE_RED);
  changePhase(junctionB, PHASE_RED);

  systemRunning = false;

  Serial.println("System halted. Reset with S.");
}

// ════════════════════════════════════════════════════════
void rebootSystem() {
  Serial.println("\nReinitializing system...");

  systemRunning = true;

  junctionA->carsCurrentCycle = 0;
  junctionA->manualControl = false;

  junctionB->carsCurrentCycle = 0;
  junctionB->manualControl = false;

  metrics->totalCars = 0;
  metrics->bootTime  = millis();

  changePhase(junctionA, PHASE_GREEN);
  changePhase(junctionB, PHASE_RED);

  Serial.println("System back online.");
  showMainMenu();
}