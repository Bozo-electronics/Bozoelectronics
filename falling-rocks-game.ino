
,#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin definitions
const int buttonLeftPin = 12;
const int buttonRightPin = 13;
const int buzzerPin = 25;

// Display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Create an OLED display object with default I2C address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Player position (Simple Human)
int playerX = SCREEN_WIDTH / 2;
int playerY = SCREEN_HEIGHT - 20;
int playerWidth = 10;
int playerHeight = 20;

// Rock arrays
int rockX[10]; // Maximum number of rocks (maximum 10)
int rockY[10];
int rockRadius[10]; // Use radius for circular rocks
int rockSpeed = 2;
int rockCount = 1; // Initial number of rocks

// Game variables
bool gameOver = false;
bool gameStarted = false;
unsigned long previousMillis = 0;
const long interval = 5000; // Interval to increase difficulty (increase rock count every 5 seconds)

// Function to draw a simple mini human figure (stickman-like)
void drawPlayer() {
  // Head (small circle)
  display.fillCircle(playerX + playerWidth / 2, playerY, 5, WHITE);
  
  // Body (line)
  display.drawLine(playerX + playerWidth / 2, playerY + 5, playerX + playerWidth / 2, playerY + playerHeight - 10, WHITE);
  
  // Arms (lines) - Slightly angled arms
  display.drawLine(playerX - 3, playerY + 10, playerX + playerWidth + 3, playerY + 10, WHITE);  
  
  // Legs (lines) - Slightly angled downward legs to simulate standing
  display.drawLine(playerX + playerWidth / 2, playerY + playerHeight - 10, playerX - 5, playerY + playerHeight + 10, WHITE); // Left leg
  display.drawLine(playerX + playerWidth / 2, playerY + playerHeight - 10, playerX + playerWidth + 5, playerY + playerHeight + 10, WHITE); // Right leg
}

// Function to draw the rocks (rounded shapes)
void drawRocks() {
  for (int i = 0; i < rockCount; i++) {
    display.fillCircle(rockX[i], rockY[i], rockRadius[i], WHITE); // Draw a rock as a circle
  }
}

// Function to check if a new rock position collides with any existing rocks
bool checkCollision(int newRockX, int newRockY, int newRockRadius) {
  for (int i = 0; i < rockCount; i++) {
    // Check for collision with the player (simple human)
    if (newRockX - newRockRadius < playerX + playerWidth && newRockX + newRockRadius > playerX &&
        newRockY - newRockRadius < playerY + playerHeight && newRockY + newRockRadius > playerY) {
      return true; // Collision detected
    }
  }
  return false; // No collision
}

// Function to update the game
void updateGame() {
  // Move the player based on button input (slower movement)
  if (digitalRead(buttonLeftPin) == LOW && playerX > 0) {
    playerX -= 2; // Slower movement to the left
  }
  if (digitalRead(buttonRightPin) == LOW && playerX < SCREEN_WIDTH - playerWidth) {
    playerX += 2; // Slower movement to the right
  }

  // Update the positions of the rocks
  for (int i = 0; i < rockCount; i++) {
    rockY[i] += rockSpeed;
    if (rockY[i] > SCREEN_HEIGHT) {
      // Reset the rock position to the top with new random values
      bool positionIsValid = false;
      int newRockX, newRockY, newRockRadius;
      
      while (!positionIsValid) {
        newRockX = random(0, SCREEN_WIDTH - 10);
        newRockY = -random(20, 60); // Start above the screen
        newRockRadius = random(5, 15);  // Random rock radius
        
        positionIsValid = !checkCollision(newRockX, newRockY, newRockRadius);
      }
      
      // Assign the new valid position to the rock
      rockX[i] = newRockX;
      rockY[i] = newRockY;
      rockRadius[i] = newRockRadius;
    }
  }

  // Check for collisions with rocks
  for (int i = 0; i < rockCount; i++) {
    if (rockY[i] + rockRadius[i] >= playerY && rockY[i] <= playerY + playerHeight &&
        rockX[i] + rockRadius[i] >= playerX && rockX[i] <= playerX + playerWidth) {
      gameOver = true;
      tone(buzzerPin, 1000, 500); // Sound the buzzer
    }
  }

  // Draw everything
  display.clearDisplay();
  if (!gameOver) {
    drawRocks();
    drawPlayer();
  } else {
    display.setCursor(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2);
    display.setTextSize(2);
    display.print("GAME OVER");
    display.setCursor(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 + 20);
    display.setTextSize(1);
    display.print("Tap to Restart");
  }
  display.display();
}

// Show the Tap to Start screen
void showStartScreen() {
  display.clearDisplay();
  display.setCursor(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 - 10);
  display.setTextSize(2);
  display.print("TAP TO START");
  display.display();
}

// Show the Tap to Restart screen
void showRestartScreen() {
  display.clearDisplay();
  display.setCursor(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 - 10);
  display.setTextSize(2);
  display.print("GAME OVER");
  display.setCursor(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2 + 10);
  display.setTextSize(1);
  display.print("TAP TO RESTART");
  display.display();
}

// Setup function
void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize the display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  // Clear the display
  display.clearDisplay();

  // Initialize buttons and buzzer pin
  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
}

// Loop function
void loop() {
  if (!gameStarted) {
    showStartScreen();
    
    // Wait for button press to start the game
    if (digitalRead(buttonLeftPin) == LOW || digitalRead(buttonRightPin) == LOW) {
      gameStarted = true;
      gameOver = false;
      rockCount = 1; // Reset rock count to 1
      previousMillis = millis(); // Reset the timer for rock count increase
      playerX = SCREEN_WIDTH / 2; // Reset player position
      playerY = SCREEN_HEIGHT - 20;
      rockX[0] = random(0, SCREEN_WIDTH - 10);
      rockY[0] = -random(20, 60);  // Start above the screen
      rockRadius[0] = random(5, 15);  // Random rock radius
    }
  } else if (gameOver) {
    showRestartScreen();
    
    // Wait for button press to restart the game
    if (digitalRead(buttonLeftPin) == LOW || digitalRead(buttonRightPin) == LOW) {
      gameStarted = true;
      gameOver = false;
      rockCount = 1; // Reset rock count to 1
      previousMillis = millis(); // Reset the timer for rock count increase
      playerX = SCREEN_WIDTH / 2; // Reset player position
      playerY = SCREEN_HEIGHT - 20;
      rockX[0] = random(0, SCREEN_WIDTH - 10);
      rockY[0] = -random(20, 60);  // Start above the screen
      rockRadius[0] = random(5, 15);  // Random rock radius
    }
  } else {
    updateGame();
    
    // Check the time to increase difficulty (add more rocks)
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // Increase the number of rocks after every 5 seconds
      if (rockCount < 10) { // Limit the maximum number of rocks to 10
        rockCount++;
        // Initialize the new rock with random position and size
        rockX[rockCount - 1] = random(0, SCREEN_WIDTH - 10);
        rockY[rockCount - 1] = -random(20, 60);  // Start above the screen
        rockRadius[rockCount - 1] = random(5, 15);  // Random rock radius
      }
    }
  }

  delay(20);  // Slow down the loop for smoother gameplay
}

