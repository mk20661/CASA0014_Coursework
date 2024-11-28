#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>

// OLED 设置
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// NeoPixel 设置
#define LED_PIN 6
#define NUM_PIXELS 12
Adafruit_NeoPixel strip(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// 旋转编码器引脚
const int A_pin = 2;
const int B_pin = 3;
const int Button_pin = 4;

// 菜单和颜色设置
int menuIndex = 0;                   // 当前菜单索引
int colorMenuIndex = 0;              // 当前颜色索引
const char* mainMenu[2] = {"Single Colour", "Random Colour"};
int mainMenuItems = 2;

const char* colorMenu[5] = {"Red", "Green", "Blue", "Yellow", "White"};
int colorMenuItems = 5;
bool isColorMenu = false;            // 是否在颜色菜单

int selectedColor[3] = {255, 0, 0};  // 当前选择的 RGB 颜色

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 10;
unsigned long buttonLastDebounceTime = 0;
const unsigned long buttonDebounceDelay = 50;
int lastButtonState = HIGH;
int lastA = LOW; 

void setup() {
  // 初始化串口
  Serial.begin(9600);

  // 初始化 OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED 初始化失败"));
    while (true);
  }
  display.clearDisplay();
  display.display();

  // 初始化 NeoPixel
  strip.begin();
  strip.show();

  // 初始化旋转编码器
  pinMode(A_pin, INPUT);
  pinMode(B_pin, INPUT);
  pinMode(Button_pin, INPUT_PULLUP);


  displayMenu();
}

void loop() {
  checkRotation();  // 检查旋转编码器
  checkButton();    // 检查按钮按下
}

// 检查旋转编码器
void checkRotation() {
  int currentA = digitalRead(A_pin);
  if (currentA != lastA && (millis() - lastDebounceTime) > debounceDelay) {
    lastDebounceTime = millis();
    int currentB = digitalRead(B_pin);
    if (!isColorMenu) {
      menuIndex += (currentA == HIGH) ? (currentB == LOW ? 1 : -1) : 0;
      menuIndex = (menuIndex + mainMenuItems) % mainMenuItems;
      displayMenu();
    } else {
      colorMenuIndex += (currentA == HIGH) ? (currentB == LOW ? 1 : -1) : 0;
      colorMenuIndex = (colorMenuIndex + colorMenuItems) % colorMenuItems;
      displayColorMenu();
    }
  }
  lastA = currentA;
}


void checkButton() {
  int buttonState = digitalRead(Button_pin);
  if (buttonState != lastButtonState && (millis() - buttonLastDebounceTime) > buttonDebounceDelay) {
    buttonLastDebounceTime = millis();
    if (buttonState == LOW) {
      if (!isColorMenu) {
        if (menuIndex == 0) {
          isColorMenu = true;
          displayColorMenu();
        } else if (menuIndex == 1) {
          Serial.println("Random Colour selected.");
          displaySelection("Random Colour");
        }
      } else {
        selectColor(colorMenuIndex);
        isColorMenu = false;
      }
    }
  }
  lastButtonState = buttonState;
}


void displayMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Select Colour Mode");
  for (int i = 0; i < mainMenuItems; i++) {
    display.print(i == menuIndex ? "> " : "  ");
    display.println(mainMenu[i]);
  }
  display.display();
}

// 显示颜色菜单
void displayColorMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Select a Colour");
  for (int i = 0; i < colorMenuItems; i++) {
    display.print(i == colorMenuIndex ? "> " : "  ");
    display.println(colorMenu[i]);
  }
  display.display();
}

// 选择颜色
void selectColor(int index) {
  switch (index) {
    case 0: selectedColor[0] = 255; selectedColor[1] = 0;   selectedColor[2] = 0; break; // Red
    case 1: selectedColor[0] = 0;   selectedColor[1] = 255; selectedColor[2] = 0; break; // Green
    case 2: selectedColor[0] = 0;   selectedColor[1] = 0;   selectedColor[2] = 255; break; // Blue
    case 3: selectedColor[0] = 255; selectedColor[1] = 255; selectedColor[2] = 0; break; // Yellow
    case 4: selectedColor[0] = 255; selectedColor[1] = 255; selectedColor[2] = 255; break; // White
  }
  displaySelection(colorMenu[index]);
}

// 显示选择结果
void displaySelection(const char* selection) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Selected:");
  display.println(selection);
  display.display();
}


