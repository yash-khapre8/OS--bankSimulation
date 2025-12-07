#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// ------------------- MESSAGE QUEUE STRUCTURE ------------------
struct transaction {
    long type;              // Message type
    int amount;             // Withdrawal amount
};

// ------------------- SHARED RESOURCES -------------------------
int balance = 2000;         // Initial bank balance
sem_t mutex;                // Semaphore for critical section

int msgid;                  // Message queue ID

// ------------------- ATM PROCESS (Producer) -------------------
void* atm_request(void* arg) {
    int amount = *(int*)arg;
    struct transaction t;

    t.type = 1;             // All ATM messages type = 1
    t.amount = amount;

    printf("ATM sending withdrawal request of ₹%d\n", amount);

    // Send message to queue
    msgsnd(msgid, &t, sizeof(t.amount), 0);

    return NULL;
}

// ------------------- SERVER PROCESS (Consumer) -------------------
void* server_process() {
    struct transaction t;

    // Wait for 2 ATM transactions
    for (int i = 0; i < 2; i++) {

        // Receive a message from queue
        msgrcv(msgid, &t, sizeof(t.amount), 1, 0);

        printf("\nServer received withdrawal request of ₹%d\n", t.amount);

        sem_wait(&mutex);   // Enter critical section
        printf("Processing withdrawal...\n");
        sleep(1);           // Simulate processing delay

        if (balance >= t.amount) {
            balance -= t.amount;
            printf("Withdrawal successful! New balance = ₹%d\n", balance);
        } else {
            printf("Insufficient balance! Current balance = ₹%d\n", balance);
        }

        sem_post(&mutex);   // Exit critical section
    }

    return NULL;
}

// ------------------- MAIN PROGRAM ------------------------------
int main() {
    pthread_t atm1, atm2, server;

    // Create message queue
    key_t key = ftok("bankfile", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);

    // Initialize semaphore
    sem_init(&mutex, 0, 1);

    int amt1 = 1000;
    int amt2 = 500;

    // Create ATM threads (Producers)
    pthread_create(&atm1, NULL, atm_request, &amt1);
    pthread_create(&atm2, NULL, atm_request, &amt2);

    // Create Server thread (Consumer)
    pthread_create(&server, NULL, (void*)server_process, NULL);

    // Wait for all threads
    pthread_join(atm1, NULL);
    pthread_join(atm2, NULL);
    pthread_join(server, NULL);

    printf("\nFinal Bank Balance: ₹%d\n", balance);

    // Cleanup
    sem_destroy(&mutex);
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
