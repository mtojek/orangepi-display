#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <linux/tiocl.h>
#include <fcntl.h>

void toggleBacklight(void) {
  printf("Backlight: toggle\n");

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

  // Setup blanking
  int fdTTY = open("/dev/tty1", O_WRONLY + O_NOCTTY);
  char ioctlarg = state ? TIOCL_UNBLANKSCREEN : TIOCL_BLANKSCREEN;
  if (ioctl(fdTTY, TIOCLINUX, &ioctlarg))
    fprintf(stderr, "Unable to set screen blanking: %s\n", strerror(errno));
  close(fdTTY);

  // Write new state
  if ((fd = fopen("/sys/class/pwm/pwmchip0/pwm2/enable", "w")) == NULL) {
  	fprintf(stderr, "Unable to write to pwm2/enable\n");
  	return;
 	}
	fprintf(fd, "%d\n", state);
  fclose(fd);
}

void brightnessIncrease(void) {
  printf("Brightness: increase\n");

	FILE* fd;

  // Read current state
	if ((fd = fopen("/sys/class/pwm/pwmchip0/pwm2/duty_cycle", "r")) == NULL) {
    fprintf(stderr, "Unable to read pwm2/duty_cycle\n");
    return;
  }

  int state = 0;
  if (fscanf(fd, "%d", &state) == EOF) {
    fclose(fd);

    fprintf(stderr, "Unable to parse pwm2/duty_cycle\n");
    return;
  }

  fclose(fd);
  
  // Check if limit is reached
	if (state == 0) {
		printf("Min brightness level\n");
		return;
	}

  // If not, increase brightness level
	state -= 20000;
  if (state < 0) {
    state = 0;
  }
  printf("\tduty_cycle: %d\n", state);

	if ((fd = fopen("/sys/class/pwm/pwmchip0/pwm2/duty_cycle", "w")) == NULL) {
    fprintf(stderr, "Unable to write to pwm2/enable\n");
    return;
  }
  fprintf(fd, "%d\n", state);
	fclose(fd);
}

void brightnessDecrease(void) {
  printf("Brightness: decrease\n");

  FILE* fd;

  // Read current state
  if ((fd = fopen("/sys/class/pwm/pwmchip0/pwm2/duty_cycle", "r")) == NULL) {
    fprintf(stderr, "Unable to read pwm2/duty_cycle\n");
    return;
  }

  int state = 0;
  if (fscanf(fd, "%d", &state) == EOF) {
    fclose(fd);

    fprintf(stderr, "Unable to parse pwm2/duty_cycle\n");
    return;
  }

  fclose(fd);

  // Check if limit is reached
  if (state == 100000) {
    printf("Max brightness level\n");
    return;
  }

  // If not, increase brightness level
  state += 20000;
  if (state > 100000) {
    state = 100000;
  }
  printf("\tduty_cycle: %d\n", state);

  if ((fd = fopen("/sys/class/pwm/pwmchip0/pwm2/duty_cycle", "w")) == NULL) {
    fprintf(stderr, "Unable to write to pwm2/enable\n");
    return;
  }
  fprintf(fd, "%d\n", state);
  fclose(fd);
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

  toggleBacklight();
}

void button2(void) {
  if (debounceFunction(BUTTON2_ID)) {
    return;
  }

  brightnessIncrease();
}

void button3(void) {
  if (debounceFunction(BUTTON3_ID)) {
    return;
  }

  brightnessDecrease();
}

void button4(void) {
  if (debounceFunction(BUTTON4_ID)) {
    return;
  }

  printf("Not implemented yet\n");
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
  for (;;) {
    fflush(stdout);
    fflush(stderr);
    sleep(1);
  }
  return 0;
}

