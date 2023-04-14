#include<stdio.h>
#include <stdlib.h>
#include<wiringPi.h>

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

  	fprintf(fd, "%d\n", gpioPin) ;
  	fclose(fd) ;
	}
}

int main(void) {
	printf("OrangePi Zero 2 - Display daemon\n");

	// wiringPiSetup();
	
	unexportButtonPins();
}

