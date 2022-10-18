#include <LedKeypad.h>
#include <EEPROM.h>
#include <arduino-timer.h>

auto timer = timer_create_default();

char current_brightness = 0;
char max_brightness = 7;
char min_brightness = 0;
int memory_addr = 0;

// An hour in miliseconds.
unsigned long interval = 60UL*60UL*1000UL;

Timer<>::Task startHoursCounter() {
  return timer.every(interval, incrementHours);
}

Timer<>::Task resetHoursCounter() {
  timer.cancel();
  return startHoursCounter();
}

int hoursToDays(int hours) {
  return (int) floor(hours/24);
}

void writeIntIntoEEPROM(int address, int number) { 
  EEPROM.update(address, number >> 8);
  EEPROM.update(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address) {
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

int readDaysFromMemory() {
  int hours = readIntFromEEPROM(memory_addr);
  return hoursToDays(hours);
}

int readHoursFromMemory() {
  return readIntFromEEPROM(memory_addr);  
}

void writeHoursToMemory(int hours) {
  writeIntIntoEEPROM(memory_addr, hours);
}

void resetDaysInMemory() {
  writeIntIntoEEPROM(memory_addr, 0);
}

void displayNumber(int days) {
  unsigned int maxDigitsNumber = 3;
  unsigned int counter = maxDigitsNumber + 1;

  // Zero all digits first.
  for (int i = 0; i < maxDigitsNumber; i++) {
    ledkeypad.display(i, 0);
  }

  // Set digits.
  do {
    counter--;

    int digit = days % 10;

    ledkeypad.display(counter, digit);
   
    days /= 10;
  } while (days > 0 && counter > 0);
}

void reset() {
  resetHoursCounter();
  resetDaysInMemory();
  displayNumber(0);
}

bool incrementHours(void *) {
  int hours = readHoursFromMemory();
  hours++;
  writeHoursToMemory(hours);

  int days = hoursToDays(hours);
  displayNumber(days);
  
  return true;
}

void setup() {
  ledkeypad.begin();
  ledkeypad.setBrightness(current_brightness);
  ledkeypad.display(readDaysFromMemory());

  pinMode(LED_BUILTIN, OUTPUT);

  startHoursCounter();
}

void loop() {
  unsigned char keyValue = 0;
  keyValue = ledkeypad.getKey();
  switch (keyValue) {
    case KEY_UP:
      if (current_brightness <= max_brightness) {
        current_brightness++;
        ledkeypad.setBrightness(current_brightness);
      }
      break;

    case KEY_DOWN:
      if (current_brightness > min_brightness) {
        current_brightness--;
        ledkeypad.setBrightness(current_brightness);
      }
      break;  

    case KEY_LEFT:
      {
        int hours = readHoursFromMemory();
        hours = hours > 24 ? hours-24 : 0;
        writeHoursToMemory(hours);
        displayNumber(hoursToDays(hours));
      }
      break;

    case KEY_RIGHT:
      {
        int hours = readHoursFromMemory();
        hours += 24;
        writeHoursToMemory(hours);
        displayNumber(hoursToDays(hours));
      }
      break;

    case KEY_SELECT:
      reset();
      break;

    default:
      break;
 }

 timer.tick();
}
