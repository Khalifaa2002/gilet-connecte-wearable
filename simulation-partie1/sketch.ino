const int PIN_STRETCH = 34;  // Potentiomètre : respiration/élongation simulée
const int PIN_TEMP    = 35;  // Potentiomètre : température simulée

const int BTN_FALL   = 18;   // Choc simulé
const int BTN_ARR    = 19;   // Épisode ECG anormal simulé
const int BTN_CANCEL = 23;   // Annulation par le sportif

const int LED_GREEN  = 26;  // Fonctionnement normal
const int LED_YELLOW = 25;  // Pré-alerte
const int LED_RED    = 33;  // Alerte confirmée
const int BUZZER     = 27;

const unsigned long FALL_CONFIRM_MS = 30000; // 30 s
const unsigned long ARR_CONFIRM_MS  = 6000;  // 3 fenêtres de 2 s
const unsigned long TEMP_CONFIRM_MS = 120000; // 2 min
const unsigned long SLOPE_WINDOW_MS = 30000;  // 30 s

bool lastFallButton = HIGH;
bool lastArrButton = HIGH;
bool lastCancelButton = HIGH;

bool fallPending = false;
bool arrPending = false;
bool fallConfirmed = false;
bool arrConfirmed = false;
bool tempConfirmed = false;
bool tempPreAlert = false;

unsigned long fallStart = 0;
unsigned long arrStart = 0;
unsigned long hotStart = 0;
unsigned long lastTempCheck = 0;
unsigned long lastPrint = 0;

float tempAtLastCheck = 30.0;

void resetAlerts() {
  fallPending = false;
  arrPending = false;
  fallConfirmed = false;
  arrConfirmed = false;
  tempConfirmed = false;
  tempPreAlert = false;
  hotStart = 0;

  Serial.println("Alertes annulees.");
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(BTN_FALL, INPUT_PULLUP);
  pinMode(BTN_ARR, INPUT_PULLUP);
  pinMode(BTN_CANCEL, INPUT_PULLUP);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lastTempCheck = millis();

  Serial.println("Simulation du gilet connecte demarree.");
  Serial.println("Bouton choc : pre-alerte chute.");
  Serial.println("Bouton ECG : simulation d'une anomalie.");
  Serial.println("Bouton annulation : annule les alertes.");
}

void loop() {
  unsigned long now = millis();

  // Les potentiomètres remplacent ici les capteurs analogiques.
  int stretchRaw = analogRead(PIN_STRETCH);
  int tempRaw = analogRead(PIN_TEMP);

  float stretchPercent = (stretchRaw / 4095.0) * 100.0;
  float tempC = 30.0 + (tempRaw / 4095.0) * 12.0;

  bool fallButton = digitalRead(BTN_FALL);
  bool arrButton = digitalRead(BTN_ARR);
  bool cancelButton = digitalRead(BTN_CANCEL);

  // Un appui simule un choc détecté par l'IMU.
  if (lastFallButton == HIGH && fallButton == LOW) {
    fallPending = true;
    fallConfirmed = false;
    fallStart = now;
    Serial.println("Choc detecte : pre-alerte locale.");
  }

  // Un appui simule une fenêtre ECG anormale.
  if (lastArrButton == HIGH && arrButton == LOW) {
    arrPending = true;
    arrConfirmed = false;
    arrStart = now;
    Serial.println("ECG anormal : confirmation en cours.");
  }

  // Un appui annule les alertes.
  if (lastCancelButton == HIGH && cancelButton == LOW) {
    resetAlerts();
  }

  lastFallButton = fallButton;
  lastArrButton = arrButton;
  lastCancelButton = cancelButton;

  // Alerte chute après 30 s sans annulation.
  if (fallPending && now - fallStart >= FALL_CONFIRM_MS) {
    fallConfirmed = true;
    Serial.println("ALERTE CHUTE confirmee : envoi BLE/SOS simule.");
    fallPending = false;
  }

  // Confirmation ECG après trois fenêtres de 2 s.
  if (arrPending && now - arrStart >= ARR_CONFIRM_MS) {
    arrConfirmed = true;
    Serial.println("ALERTE ECG confirmee : envoi BLE simule.");
    arrPending = false;
  }

  // Pré-alerte thermique si la température augmente de plus de
  // 0,1 °C/min sur la fenêtre observée de 30 s.
  if (now - lastTempCheck >= SLOPE_WINDOW_MS) {
    float minutes = (now - lastTempCheck) / 60000.0;
    float slope = (tempC - tempAtLastCheck) / minutes;

    tempPreAlert = (slope > 0.1);

    if (tempPreAlert) {
      Serial.println("Pre-alerte thermique : hausse rapide estimee.");
    }

    tempAtLastCheck = tempC;
    lastTempCheck = now;
  }

  // Alerte thermique si Tc simulée atteint 39 °C pendant 2 min.
  if (tempC >= 39.0) {
    if (hotStart == 0) hotStart = now;

    if (now - hotStart >= TEMP_CONFIRM_MS && !tempConfirmed) {
      tempConfirmed = true;
      Serial.println("ALERTE THERMIQUE confirmee : BLE/SOS simule.");
    }
  } else {
    // Sous 39 C avant confirmation : remise a zero du delai continu.
    hotStart = 0;
    // tempConfirmed reste memorisee jusqu'a resetAlerts() / Annuler.
  }

  bool confirmed = fallConfirmed || arrConfirmed || tempConfirmed;
  bool preAlert = fallPending || arrPending || tempPreAlert;

  digitalWrite(LED_GREEN, (!confirmed && !preAlert) ? HIGH : LOW);
  digitalWrite(LED_YELLOW, preAlert ? HIGH : LOW);
  digitalWrite(LED_RED, confirmed ? HIGH : LOW);

  if (confirmed) {
    tone(BUZZER, 2200);
  } else {
    noTone(BUZZER);
  }

  // Affichage des mesures simulées dans le moniteur série.
  if (now - lastPrint >= 1000) {
    lastPrint = now;
    Serial.print("Elongation simulee: ");
    Serial.print(stretchPercent, 1);
    Serial.print("% | Temperature simulee: ");
    Serial.print(tempC, 1);
    Serial.println(" C");
  }

  delay(20);
}