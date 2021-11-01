#include "philosopher.h"

void *philosopher(void *philosopherNumber) {
	while (1) {
		think(philosopherNumber);
		pickUp(philosopherNumber);
		eat(philosopherNumber);
		putDown(philosopherNumber);
	}
}

void think(int philosopherNumber) {
	int sleepTime = rand() % 3 + 1;
	printf("Philosopher %d will think for %d seconds\n", philosopherNumber, sleepTime);
	sleep(sleepTime);
}

void pickUp(int philosopherNumber) {
    /*Your code here*/
}

void eat(int philosopherNumber) {
	int eatTime = rand() % 3 + 1;
	printf("Philosopher %d will eat for %d seconds\n", philosopherNumber, eatTime);
	sleep(eatTime);
}

void putDown(int philosopherNumber) {
    /*Your code here*/
}