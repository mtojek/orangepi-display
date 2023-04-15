#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>

#define DEBOUNCE_TIME 150

int debounceTimes[] = {0, 0, 0, 0};

bool debounceFunction(int buttonIndex) {
 if (millis() < debounceTimes[buttonIndex] + DEBOUNCE_TIME) {
    return true;
  }
  debounceTimes[buttonIndex] = millis();
  return false;
}

#define BUTTON1_ID 0
#define BUTTON2_ID 1
#define BUTTON3_ID 2
#define BUTTON4_ID 3

void button1(void) {
  if (debounceFunction(BUTTON1_ID)) {
		return;
  }

  printf("Button 1 pressed\n");
}

void button2(void) {
  if (debounceFunction(BUTTON2_ID)) {
    return;
  }

  printf("Button 2 pressed\n");
}

void button3(void) {
  if (debounceFunction(BUTTON3_ID)) {
    return;
  }

  printf("Button 3 pressed\n");
}

void button4(void) {
  if (debounceFunction(BUTTON4_ID)) {
    return;
  }

  printf("Button 4 pressed\n");
}

void (*buttonFunctions[])(void) = { &button1, &button2, &button3, &button4 };

const int buttonWiPins[] = {2, 5, 4, 6};
const int wiPinsToGpio[] = {73, 70, 227, 75};

void unexportButtonPins() {
	printf("Unexport button pins...\n");

	FILE* fd;
  for (int i = 0; i < sizeof(buttonWiPins) / sizeof(buttonWiPins[0]); i++) {
    int wiPin = buttonWiPins[i];
    int gpioPin = wiPinsToGpio[i];

		printf("\tpin %d (gpio: %d)\n", wiPin, gpioPin); 
		if ((fd = fopen("/sys/class/gpio/unexport", "w")) == NULL) {
    	fprintf(stderr, "Unable to open GPIO export interface\n");
    	exit(1) ;
  	}

  	fprintf(fd, "%d\n", gpioPin);
  	fclose(fd);
	}
}

void configureIntterupt(int buttonIndex) {
  int wiPin = buttonWiPins[buttonIndex];
  printf("\tpin %d\n", wiPin);

	wiringPiISR(wiPin, INT_EDGE_FALLING, buttonFunctions[buttonIndex]);
}

void configureInterrupts() {
  printf("Configure interrupts...\n");

  for (int i = 0; i < sizeof(buttonWiPins) / sizeof(buttonWiPins[0]); i++) {
    configureIntterupt(i);
  }
}

int main() {
	printf("OrangePi Zero 2 - Display daemon\n");

	if (wiringPiSetup() < 0) {
    fprintf(stderr, "Unable to initialise wiringPi: %s\n", strerror(errno));
    exit(1);
  }

  unexportButtonPins();
  configureInterrupts();

  printf("Wait infinitely...\n");
  for (;;) { sleep(UINT_MAX); }
  return 0;
}

