/*
  Project 1: Smart Traffic Light Controller
  - Two intersections
  - Non-blocking control with millis()
  - Vehicle detection via buttons
  - Dynamic memory + structures + pointers
  - Serial menu for monitoring and manual override
*/

struct Intersection {
  const char* name;
  int redPin;
  int yellowPin;
  int greenPin;
  int sensorPin;

  unsigned long greenDurationMs;
  unsigned long yellowDurationMs;
  unsigned long redDurationMs;

  unsigned long lastStateChangeMs;
  unsigned long vehicleCount;
  bool lastSensorState;
  int currentState; // 0 = RED, 1 = YELLOW, 2 = GREEN
  bool manualMode;
};

struct TrafficSystem {
  Intersection* intersections;
  int count;
  bool autoMode;
  unsigned long logIntervalMs;
  unsigned long lastLogMs;
};

TrafficSystem* g_system = NULL;

const int STATE_RED = 0;
const int STATE_YELLOW = 1;
const int STATE_GREEN = 2;

String stateToText(int state) {
  if (state == STATE_RED) return "RED";
  if (state == STATE_YELLOW) return "YELLOW";
  if (state == STATE_GREEN) return "GREEN";
  return "INVALID";
}

void setLights(Intersection* itx, int state) {
  if (itx == NULL) return;

  if (state < STATE_RED || state > STATE_GREEN) {
    Serial.print("[ERROR] Invalid state for ");
    Serial.println(itx->name);
    return;
  }

  itx->currentState = state;
  digitalWrite(itx->redPin, state == STATE_RED ? HIGH : LOW);
  digitalWrite(itx->yellowPin, state == STATE_YELLOW ? HIGH : LOW);
  digitalWrite(itx->greenPin, state == STATE_GREEN ? HIGH : LOW);
  itx->lastStateChangeMs = millis();
}

void configureIntersection(Intersection* itx, const char* name,
                           int redPin, int yellowPin, int greenPin, int sensorPin) {
  itx->name = name;
  itx->redPin = redPin;
  itx->yellowPin = yellowPin;
  itx->greenPin = greenPin;
  itx->sensorPin = sensorPin;

  itx->greenDurationMs = 4000;
  itx->yellowDurationMs = 1500;
  itx->redDurationMs = 4000;

  itx->lastStateChangeMs = 0;
  itx->vehicleCount = 0;
  itx->lastSensorState = false;
  itx->manualMode = false;

  pinMode(itx->redPin, OUTPUT);
  pinMode(itx->yellowPin, OUTPUT);
  pinMode(itx->greenPin, OUTPUT);
  pinMode(itx->sensorPin, INPUT_PULLUP);

  setLights(itx, STATE_RED);
}

void printHelp() {
  Serial.println("\n=== Smart Traffic Controller Menu ===");
  Serial.println("help                -> show commands");
  Serial.println("status              -> print current state");
  Serial.println("auto                -> enable automatic mode");
  Serial.println("manual A RED        -> set intersection A state");
  Serial.println("manual A YELLOW");
  Serial.println("manual A GREEN");
  Serial.println("manual B RED/YELLOW/GREEN");
  Serial.println("reset               -> reset vehicle counters");
  Serial.println("=====================================\n");
}

Intersection* getIntersectionByName(char nameChar) {
  if (g_system == NULL || g_system->intersections == NULL) return NULL;
  for (int i = 0; i < g_system->count; i++) {
    if (g_system->intersections[i].name[0] == nameChar) {
      return &g_system->intersections[i];
    }
  }
  return NULL;
}

int parseState(const String& text) {
  if (text == "RED") return STATE_RED;
  if (text == "YELLOW") return STATE_YELLOW;
  if (text == "GREEN") return STATE_GREEN;
  return -1;
}

void printStatus() {
  if (g_system == NULL) return;

  Serial.println("\n--- Traffic Status ---");
  Serial.print("Mode: ");
  Serial.println(g_system->autoMode ? "AUTO" : "MANUAL");

  for (int i = 0; i < g_system->count; i++) {
    Intersection* itx = &g_system->intersections[i];
    Serial.print("Intersection ");
    Serial.print(itx->name);
    Serial.print(" | State: ");
    Serial.print(stateToText(itx->currentState));
    Serial.print(" | Vehicles: ");
    Serial.print(itx->vehicleCount);
    Serial.print(" | Green(ms): ");
    Serial.println(itx->greenDurationMs);
  }
}

void handleSerialCommand(String cmdLine) {
  cmdLine.trim();
  cmdLine.toUpperCase();

  if (cmdLine.length() == 0) return;

  if (cmdLine == "HELP") {
    printHelp();
    return;
  }

  if (cmdLine == "STATUS") {
    printStatus();
    return;
  }

  if (cmdLine == "AUTO") {
    g_system->autoMode = true;
    for (int i = 0; i < g_system->count; i++) {
      g_system->intersections[i].manualMode = false;
    }
    Serial.println("[OK] Automatic mode enabled");
    return;
  }

  if (cmdLine == "RESET") {
    for (int i = 0; i < g_system->count; i++) {
      g_system->intersections[i].vehicleCount = 0;
    }
    Serial.println("[OK] Vehicle counters reset");
    return;
  }

  if (cmdLine.startsWith("MANUAL ")) {
    int firstSpace = cmdLine.indexOf(' ');
    int secondSpace = cmdLine.indexOf(' ', firstSpace + 1);
    if (secondSpace < 0) {
      Serial.println("[ERROR] Use format: MANUAL A RED");
      return;
    }

    String namePart = cmdLine.substring(firstSpace + 1, secondSpace);
    String statePart = cmdLine.substring(secondSpace + 1);

    if (namePart.length() != 1) {
      Serial.println("[ERROR] Intersection must be A or B");
      return;
    }

    Intersection* itx = getIntersectionByName(namePart[0]);
    if (itx == NULL) {
      Serial.println("[ERROR] Unknown intersection");
      return;
    }

    int state = parseState(statePart);
    if (state < 0) {
      Serial.println("[ERROR] State must be RED, YELLOW, or GREEN");
      return;
    }

    g_system->autoMode = false;
    itx->manualMode = true;
    setLights(itx, state);
    Serial.print("[OK] ");
    Serial.print(itx->name);
    Serial.print(" set to ");
    Serial.println(stateToText(state));
    return;
  }

  Serial.println("[ERROR] Unknown command. Type HELP");
}

void updateVehicleDetection(Intersection* itx) {
  bool pressed = (digitalRead(itx->sensorPin) == LOW);
  if (pressed && !itx->lastSensorState) {
    itx->vehicleCount++;
  }
  itx->lastSensorState = pressed;
}

void adjustTimings(Intersection* itx) {
  // Increase green duration if many vehicles detected recently.
  if (itx->vehicleCount > 10) {
    itx->greenDurationMs = 7000;
  } else if (itx->vehicleCount > 5) {
    itx->greenDurationMs = 5500;
  } else {
    itx->greenDurationMs = 4000;
  }
  itx->redDurationMs = itx->greenDurationMs;
}

void updateIntersectionAuto(Intersection* itx) {
  if (itx->manualMode) return;

  adjustTimings(itx);

  unsigned long now = millis();
  unsigned long elapsed = now - itx->lastStateChangeMs;

  if (itx->currentState == STATE_GREEN && elapsed >= itx->greenDurationMs) {
    setLights(itx, STATE_YELLOW);
  } else if (itx->currentState == STATE_YELLOW && elapsed >= itx->yellowDurationMs) {
    setLights(itx, STATE_RED);
  } else if (itx->currentState == STATE_RED && elapsed >= itx->redDurationMs) {
    setLights(itx, STATE_GREEN);
  }
}

void setupSystem() {
  g_system = (TrafficSystem*)malloc(sizeof(TrafficSystem));
  if (g_system == NULL) {
    Serial.println("[FATAL] Failed to allocate traffic system");
    while (true) {}
  }

  g_system->count = 2;
  g_system->intersections = (Intersection*)malloc(sizeof(Intersection) * g_system->count);
  if (g_system->intersections == NULL) {
    Serial.println("[FATAL] Failed to allocate intersections");
    while (true) {}
  }

  g_system->autoMode = true;
  g_system->logIntervalMs = 2000;
  g_system->lastLogMs = 0;

  configureIntersection(&g_system->intersections[0], "A", 2, 3, 4, 8);
  configureIntersection(&g_system->intersections[1], "B", 5, 6, 7, 9);

  setLights(&g_system->intersections[0], STATE_GREEN);
  setLights(&g_system->intersections[1], STATE_RED);
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {}

  setupSystem();
  printHelp();
}

void loop() {
  if (g_system == NULL) return;

  for (int i = 0; i < g_system->count; i++) {
    updateVehicleDetection(&g_system->intersections[i]);
  }

  if (g_system->autoMode) {
    updateIntersectionAuto(&g_system->intersections[0]);

    // Keep B opposite to A for safety in this simple model.
    if (g_system->intersections[0].currentState == STATE_GREEN) {
      if (g_system->intersections[1].currentState != STATE_RED) {
        setLights(&g_system->intersections[1], STATE_RED);
      }
    } else if (g_system->intersections[0].currentState == STATE_RED) {
      if (g_system->intersections[1].currentState != STATE_GREEN) {
        setLights(&g_system->intersections[1], STATE_GREEN);
      }
    } else {
      if (g_system->intersections[1].currentState != STATE_RED) {
        setLights(&g_system->intersections[1], STATE_RED);
      }
    }
  }

  unsigned long now = millis();
  if (now - g_system->lastLogMs >= g_system->logIntervalMs) {
    g_system->lastLogMs = now;
    printStatus();
  }

  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    handleSerialCommand(cmd);
  }
}
