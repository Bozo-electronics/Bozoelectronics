#define MIC_PIN A0  // Analog pin for MAX4466
#define LED1 8      // LED pins
#define LED2 7
#define LED3 6
#define LED4 5
#define LED5 4
#define LED6 3

void setup() {
  pinMode(MIC_PIN, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(LED6, OUTPUT);
  Serial.begin(9600);  // For debugging
}

void loop() {
  int minValue = 1023;  // Initialize to max ADC value
  int maxValue = 0;     // Initialize to min ADC value

  // Sample the sound signal for 50ms
  unsigned long startTime = millis();
  while (millis() - startTime < 50) {
    int sample = analogRead(MIC_PIN);

    // Track the minimum and maximum values
    if (sample < minValue) minValue = sample;
    if (sample > maxValue) maxValue = sample;
  }

  // Calculate the amplitude of the sound signal
  int soundAmplitude = maxValue - minValue;
  Serial.println(soundAmplitude);  // Debugging

  // Map amplitude to LED thresholds
  digitalWrite(LED1, soundAmplitude > 50);   // Adjust thresholds as needed
  digitalWrite(LED2, soundAmplitude > 100);
  digitalWrite(LED3, soundAmplitude > 150);
  digitalWrite(LED4, soundAmplitude > 200);
  digitalWrite(LED5, soundAmplitude > 300);
  digitalWrite(LED6, soundAmplitude > 400);

  delay(50);  // Short delay
}
