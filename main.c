#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <errno.h>
#include <string.h>

const int buttonWiPins[] = {2, 4, 5, 6};

const int pinToGpioOrangePi[] = {73, 227, 70, 75};

void unexportButtonPins() {
	printf("Unexport button pins...\n");

	FILE* fd;
  for (int i = 0; i < sizeof(buttonWiPins) / sizeof(buttonWiPins[0]); i++) {
    int wiPin = buttonWiPins[i];
    int gpioPin = pinToGpioOrangePi[i];

		printf("\tpin %d (gpio: %d)\n", wiPin, gpioPin); 
		if ((fd = fopen("/sys/class/gpio/unexport", "w")) == NULL) {
    	fprintf(stderr, "Unable to open GPIO export interface\n");
    	exit(1) ;
  	}

  	fprintf(fd, "%d\n", gpioPin);
  	fclose(fd);
	}
}

void configureButtonModes() {
  printf("Configure button modes...\n");

  FILE* fd;
  for (int i = 0; i < sizeof(buttonWiPins) / sizeof(buttonWiPins[0]); i++) {
    int wiPin = buttonWiPins[i];
    int gpioPin = pinToGpioOrangePi[i];

		printf("\tpin %d (gpio: %d)\n", wiPin, gpioPin);

    // Export pin
    if ((fd = fopen("/sys/class/gpio/export", "w")) == NULL) {
      fprintf(stderr, "Unable to open GPIO export interface\n");
      exit(1);
    }

    fprintf(fd, "%d\n", gpioPin);
    fclose(fd);

    // Set as input
    char fName[128];
    sprintf(fName, "/sys/class/gpio/gpio%d/direction", gpioPin);
    if ((fd = fopen (fName, "w")) == NULL) {
    	fprintf(stderr, "Unable to open GPIO direction interface for pin %d: %s\n", gpioPin, strerror(errno));
    	exit(1);
  	}
    fprintf(fd, "in\n");
		fclose(fd);
		
		// Enable pullup resistor
    if ((fd = fopen (fName, "w")) == NULL) {
      fprintf (stderr, "Unable to open GPIO direction interface for pin %d: %s\n", gpioPin, strerror(errno));
      exit(1);
    }
    fprintf(fd, "up\n");
    fclose(fd);
  }	
}

int main(void) {
	printf("OrangePi Zero 2 - Display daemon\n");

	// wiringPiSetup();
	
	unexportButtonPins();
  configureButtonModes();
}

