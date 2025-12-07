#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int balance = 2000;     // Initial bank balance
sem_t mutex;            // Semaphore for mutual exclusion

void* withdraw(void* arg) {
    int amount = *(int*)arg;

    printf("ATM requesting withdrawal of ₹%d\n", amount);

    sem_wait(&mutex);   // Enter critical section

    printf("Processing withdrawal of ₹%d...\n", amount);
    sleep(1);           // Simulate delay

    if (balance >= amount) {
        balance -= amount;
        printf("Withdrawal successful! New balance: ₹%d\n", balance);
    } else {
        printf("Withdrawal failed! Insufficient balance. Current balance: ₹%d\n", balance);
    }

    sem_post(&mutex);   // Exit critical section
}

int main() {
    pthread_t atm1, atm2;

    int amt1 = 1000;
    int amt2 = 500;

    sem_init(&mutex, 0, 1);

    pthread_create(&atm1, NULL, withdraw, &amt1);
    pthread_create(&atm2, NULL, withdraw, &amt2);

    pthread_join(atm1, NULL);
    pthread_join(atm2, NULL);

    printf("\nFinal balance: ₹%d\n", balance);

    sem_destroy(&mutex);

    return 0;
}
