#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

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

void button1(void) {
  printf("Button 1 pressed\n");
}

void button2(void) {
  printf("Button 2 pressed\n");
}

void button3(void) {
  printf("Button 3 pressed\n");
}

void button4(void) {
  printf("Button 4 pressed\n");
}

void configureInterrupts() {
  printf("Configure interrupts...\n");

  printf("\tpin %d\n", buttonWiPins[0]);
  wiringPiISR(buttonWiPins[0], INT_EDGE_FALLING, &button1);

  printf("\tpin %d\n", buttonWiPins[1]);
  wiringPiISR(buttonWiPins[1], INT_EDGE_FALLING, &button2);
  
  printf("\tpin %d\n", buttonWiPins[2]);
  wiringPiISR(buttonWiPins[2], INT_EDGE_FALLING, &button3);
  
  printf("\tpin %d\n", buttonWiPins[3]);
  wiringPiISR(buttonWiPins[3], INT_EDGE_FALLING, &button4);
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

