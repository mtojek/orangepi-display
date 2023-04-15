#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>

void toggleBacklight(void) {
	FILE* fd;

  // bl_power: /sys/class/backlight/fb_ili9341/bl_power
  // Read current state
  if ((fd = fopen("/sys/class/pwm/pwmchip0/pwm2/enable", "r")) == NULL) {
    fprintf(stderr, "Unable to read pwm2/enable\n");
    return;
  }

	int state = fgetc(fd);
  fclose(fd);

  // Update state
  state = (state - 0x30) ? 0 : 1;

  // Write new state
  if ((fd = fopen("/sys/class/pwm/pwmchip0/pwm2/enable", "w")) == NULL) {
  	fprintf(stderr, "Unable to write to pwm2/enable\n");
  	return;
 	}
	fprintf(fd, "%d\n", state);
  fclose(fd);
}

void backlightStepUp(void) {
}

void backlightStepDown(void) {
}

#define DEBOUNCE_TIME 500

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
  toggleBacklight();
}

void button2(void) {
  if (debounceFunction(BUTTON2_ID)) {
    return;
  }

  printf("Button 2 pressed\n");
  backlightStepUp();
}

void button3(void) {
  if (debounceFunction(BUTTON3_ID)) {
    return;
  }

  printf("Button 3 pressed\n");
  backlightStepDown();
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

void unexportPWM(void) {
  // Check if pwm2 exists
  DIR* pwm2 = opendir("/sys/class/pwm/pwmchip0/pwm2");
  if (pwm2) {
		closedir(pwm2);
  } else if ((errno > 0) && (errno != ENOENT)) {
		fprintf(stderr, "Unable to open pwmchip0/pwm2\n");
    exit(1) ;
  }

  // if exists, then unexport it
  FILE* fd;
	if ((fd = fopen("/sys/class/pwm/pwmchip0/unexport", "w")) == NULL) {
    fprintf(stderr, "Unable to write to pwmchip0/unexport\n");
    return;
  }
  fprintf(fd, "%d\n", 2);
  fclose(fd);
}

void configureBacklight(void) {
	FILE* fd;

  // Export PWM2
  if ((fd = fopen("/sys/class/pwm/pwmchip0/export", "w")) == NULL) {
    fprintf(stderr, "Unable to write to pwmchip0/export\n");
    return;
  }
  fprintf(fd, "%d\n", 2);
  fclose(fd);

  // Set period
	if ((fd = fopen("/sys/class/pwm/pwmchip0/pwm2/period", "w")) == NULL) {
    fprintf(stderr, "Unable to write to pwm2/period\n");
    return;
  }
  fprintf(fd, "%d\n", 100000);
  fclose(fd);
}

void unexportButtonPins(void) {
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

void configureInterrupts(void) {
  printf("Configure interrupts...\n");

  for (int i = 0; i < sizeof(buttonWiPins) / sizeof(buttonWiPins[0]); i++) {
    configureIntterupt(i);
  }
}

int main(void) {
	printf("OrangePi Zero 2 - Display daemon\n");

	if (wiringPiSetup() < 0) {
    fprintf(stderr, "Unable to initialise wiringPi: %s\n", strerror(errno));
    exit(1);
  }

  unexportPWM();
  unexportButtonPins();
  configureBacklight();
  configureInterrupts();

  printf("Wait infinitely...\n");
  for (;;) { sleep(UINT_MAX); }
  return 0;
}

