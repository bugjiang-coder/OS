#include "philosopher.h"

void *philosopher(void *philosopherNumber)
{
	while (1)
	{
		think(philosopherNumber);
		pickUp(philosopherNumber);
		eat(philosopherNumber);
		putDown(philosopherNumber);
	}
}

void think(int philosopherNumber)
{
	int sleepTime = rand() % 3 + 1;
	printf("Philosopher %d will think for %d seconds\n", philosopherNumber, sleepTime);
	sleep(sleepTime);
}

void pickUp(int philosopherNumber)
{
	/*Your code here*/
	// 限制拿筷子的顺序
	pthread_mutex_lock(&chopsticks[(philosopherNumber + (philosopherNumber % 2)) % NUMBER_OF_PHILOSOPHERS]);
	pthread_mutex_lock(&chopsticks[(philosopherNumber + ((philosopherNumber + 1) % 2)) % NUMBER_OF_PHILOSOPHERS]);
	printf("+++Philosopher %d pick up two chopsticks\n", philosopherNumber);
}

void eat(int philosopherNumber)
{
	int eatTime = rand() % 3 + 1;
	printf("Philosopher %d will eat for %d seconds\n", philosopherNumber, eatTime);
	sleep(eatTime);
}

void putDown(int philosopherNumber)
{
	/*Your code here*/
	pthread_mutex_unlock(&chopsticks[(philosopherNumber + ((philosopherNumber + 1) % 2)) % NUMBER_OF_PHILOSOPHERS]);
	pthread_mutex_unlock(&chopsticks[(philosopherNumber + (philosopherNumber % 2)) % NUMBER_OF_PHILOSOPHERS]);
	printf("---Philosopher %d put down two chopsticks\n", philosopherNumber);
}