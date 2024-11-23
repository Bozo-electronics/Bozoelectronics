#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define BUTTON_UP_PIN 12
#define BUTTON_DOWN_PIN 13
#define PIEZO_PIN 25   // Pin for piezo buzzer (connected to GPIO 25)

// Initialize the OLED display object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pong game variables
int playerPaddleY = SCREEN_HEIGHT / 2 - 10;
int botPaddleY = SCREEN_HEIGHT / 2 - 10;
int paddleWidth = 6;
int paddleHeight = 20;
int ballX = SCREEN_WIDTH / 2;
int ballY = SCREEN_HEIGHT / 2;
float ballSpeedX = 2.0;  // Ball speed, float for more precision
float ballSpeedY = 2.0;
int playerScore = 0;
int botScore = 0;
float ballSpeedIncreaseFactor = 1.03; // Speed increase factor after each bounce

// Button states
bool buttonUpState = false;
bool buttonDownState = false;
bool buttonUpPrevState = false;
bool buttonDownPrevState = false;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;

// Reset and game states
bool gameOver = false;
bool ballResetting = false;

void setup() {
  Serial.begin(115200);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Halt if OLED initialization fails
  }

  display.clearDisplay();
  
  // Initialize button pins with pull-up resistors
  pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);

  // Initialize piezo buzzer pin
  pinMode(PIEZO_PIN, OUTPUT);
}

void loop() {
  // Read button states and debounce the buttons
  bool currentButtonUpState = digitalRead(BUTTON_UP_PIN) == LOW;
  bool currentButtonDownState = digitalRead(BUTTON_DOWN_PIN) == LOW;

  // Debounce handling
  if (millis() - lastDebounceTime > debounceDelay) {
    if (currentButtonUpState != buttonUpPrevState) {
      buttonUpState = currentButtonUpState;
      lastDebounceTime = millis();
    }

    if (currentButtonDownState != buttonDownPrevState) {
      buttonDownState = currentButtonDownState;
      lastDebounceTime = millis();
    }

    buttonUpPrevState = currentButtonUpState;
    buttonDownPrevState = currentButtonDownState;
  }

  // Run Pong game logic
  if (!gameOver) {
    runPongGame();
  } else {
    displayGameOver();
  }
}

void runPongGame() {
  // Clear the display and draw paddles and ball
  display.clearDisplay();

  // Draw player paddle with smooth rounded edges
  drawPaddle(10, playerPaddleY);

  // Draw bot paddle with smooth rounded edges
  drawPaddle(SCREEN_WIDTH - 10 - paddleWidth, botPaddleY);

  // Draw the ball (ball is now round using fillCircle)
  display.fillCircle(ballX, ballY, 3, SSD1306_WHITE);  // Ball radius is 3

  // Ball movement
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  // Ball collision with top and bottom walls
  if (ballY <= 0 || ballY >= SCREEN_HEIGHT - 5) {
    ballSpeedY = -ballSpeedY; // Reverse vertical direction
    tone(PIEZO_PIN, 1000, 50); // Play sound on ball bounce
  }

  // Ball goes out of bounds (left or right side) => goal
  if (ballX <= 0) { // AI scores
    botScore++;
    resetBall();
    tone(PIEZO_PIN, 1500, 100); // Play sound on goal
  }

  if (ballX >= SCREEN_WIDTH - 5) { // Player scores
    playerScore++;
    resetBall();
    tone(PIEZO_PIN, 1500, 100); // Play sound on goal
  }

  // Ball collision with paddles
  if (ballX <= 10 + paddleWidth && ballY >= playerPaddleY && ballY <= playerPaddleY + paddleHeight) {
    ballSpeedX = -ballSpeedX; // Reverse horizontal direction
    increaseBallSpeed(); // Increase ball speed after scoring
    tone(PIEZO_PIN, 800, 50); // Play sound on paddle hit
  }

  if (ballX >= SCREEN_WIDTH - 10 - paddleWidth && ballY >= botPaddleY && ballY <= botPaddleY + paddleHeight) {
    ballSpeedX = -ballSpeedX; // Reverse horizontal direction
    increaseBallSpeed(); // Increase ball speed after scoring
    tone(PIEZO_PIN, 800, 50); // Play sound on paddle hit
  }

  // Move player paddle with buttons
  if (buttonUpState && playerPaddleY > 0) {
    playerPaddleY -= 2; // Move up
  }

  if (buttonDownState && playerPaddleY < SCREEN_HEIGHT - paddleHeight) {
    playerPaddleY += 2; // Move down
  }

  // AI-controlled bot paddle movement with a 20% chance of missing
  if (random(0, 100) >= 20) { // 80% chance the bot will try to hit the ball
    if (ballY < botPaddleY + paddleHeight / 2) {
      botPaddleY -= 2; // AI moves at normal speed
    } else if (ballY > botPaddleY + paddleHeight / 2) {
      botPaddleY += 2; // AI moves at normal speed
    }
  }

  // Ensure bot paddle stays within the screen bounds
  if (botPaddleY < 0) {
    botPaddleY = 0;
  }
  if (botPaddleY > SCREEN_HEIGHT - paddleHeight) {
    botPaddleY = SCREEN_HEIGHT - paddleHeight;
  }

  // Display the score at the top center, fixing alignment
  display.setTextSize(1);      // Set text size
  display.setTextColor(SSD1306_WHITE); // Set text color to white

  // Centered score at the top
  int scoreXPos = (SCREEN_WIDTH - 24) / 2; // Centering score text

  display.setCursor(scoreXPos, 0); // Player score position
  display.print(playerScore);  // Print player score

  display.setCursor(scoreXPos + 12, 0); // Position for the colon separator
  display.print(":");

  display.setCursor(scoreXPos + 24, 0);  // Position for bot score
  display.print(botScore);  // Print bot score

  // Check for Game Over (if a player reaches 3 points)
  if (playerScore >= 3 || botScore >= 3) {
    gameOver = true; // Set the game over flag to true
  }

  // Update the display
  display.display();

  delay(20); // Slow down the game loop
}

void resetBall() {
  // Reset ball to the middle
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;

  // Ball stays in the middle for 0.5 seconds before moving
  delay(500); // Wait 0.5 seconds before starting

  // Randomize ball's direction
  ballSpeedX = random(0, 2) == 0 ? 2.0 : -2.0; // Randomize initial X direction
  ballSpeedY = random(0, 2) == 0 ? 2.0 : -2.0; // Randomize initial Y direction
}

void increaseBallSpeed() {
  // Increase ball speed slightly after each bounce (by 1.03x)
  ballSpeedX *= ballSpeedIncreaseFactor;
  ballSpeedY *= ballSpeedIncreaseFactor;
}

void displayGameOver() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  if (playerScore >= 3) {
    display.setCursor((SCREEN_WIDTH - 80) / 2, SCREEN_HEIGHT / 2 - 10); // Centered text
    display.print("YOU WIN!");
    display.display(); // Immediately display "YOU WIN!" text

    playWinningMelody();  // Play the winning melody right after showing the text
  } else {
    display.setCursor((SCREEN_WIDTH - 100) / 2, SCREEN_HEIGHT / 2 - 10); // Centered text
    display.print("GAME OVER");
    tone(PIEZO_PIN, 500, 500);  // Play lose sound
    display.display();
  }

  delay(2000); // Show the message for 2 seconds

  // Wait for button press to restart the game
  while (digitalRead(BUTTON_UP_PIN) == HIGH && digitalRead(BUTTON_DOWN_PIN) == HIGH) {
    // Wait for button press
  }

  // Reset game variables
  resetGame();
}

void resetGame() {
  playerScore = 0;
  botScore = 0;
  ballSpeedX = 2.0;
  ballSpeedY = 2.0;
  ballResetting = false;
  gameOver = false;
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;
  resetBall();
}

void drawPaddle(int x, int y) {
  display.fillRoundRect(x, y, paddleWidth, paddleHeight, 4, SSD1306_WHITE);  // Rounded paddle with radius of 4
}

// Play a popular winning melody
void playWinningMelody() {
  tone(PIEZO_PIN, 1046, 300); // C6
  delay(300);
  tone(PIEZO_PIN, 1318, 300); // D6
  delay(300);
  tone(PIEZO_PIN, 1568, 300); // E6
  delay(300);
  tone(PIEZO_PIN, 1746, 300); // F6
  delay(300);
  tone(PIEZO_PIN, 2093, 500); // C7 (higher pitch)
  delay(500);
}

