build:
	gcc -o orangepi-display -lwiringPi main.c

	sudo chown root:root orangepi-display
	sudo chmod u+s orangepi-display
