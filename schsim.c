// Date: 12/10/2021
// Class: CS 4541
// Assignment: Assignment 4 : CPU Scheduler Simulator
// Author(s): Jhye Tim Chi
// Email(s): jhyetim.chi@wmich.edu

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>

struct process {
    char processName;
    unsigned arrival;
    double processing;
    double originalProcessing;
    int ioTime[16];
    int ioBurstCycle[16];
    int ioCount;
    int ioCounter;
    unsigned start;
    unsigned totalWait;
    unsigned finish;
    unsigned turnaround;
    double normalizedTurnaround;
};

struct processNode {
	struct process *data;
	struct processNode *next;
};

struct queue {
	struct processNode *head;
	struct processNode *tail;
    int size;
};

struct queue arrivalQueue;
struct queue readyQueue;
struct queue waitingQueue;
struct queue finishQueue;
struct queue tempQueue;

// Creates a process node with pointer to data and next
struct processNode *createProcessNode (struct process *p) {
	struct processNode *node = (struct processNode*) malloc(sizeof(struct processNode));
	if (node == NULL){
		perror("Error: ");
	}
	node->data = p;
	node->next = NULL;

	return node;
}

// Initializes a process queue
void initializeProcessQueue(struct queue *q){
	q->head = NULL;
    q->tail = NULL;
	q->size = 0;
}

// Equeues a process
void enqueueProcess (struct queue *q, struct process *p){
	struct processNode *node = createProcessNode(p);
	if (q->head == NULL){
		assert(q->tail == NULL);
		q->head = q->tail = node;
	}
	else{
		assert(q->tail != NULL);
		q->tail->next = node;
		q->tail = node;
	}
	q->size++;
}

// Dequeues a process
void dequeueProcess(struct queue *q) {
    struct processNode *deleted = q->head;
    assert(q->size > 0);
    if (q->size == 1) {
        q->head = NULL;
        q->tail = NULL;
    } else {
        assert(q->head->next != NULL);
        q->head = q->head->next;
    }
    free(deleted);
    q->size--;  
}

// Function to parse the command line input and get s, q, input, and output
int cmdParser (int argc, char** argv, char* s, unsigned* q, char* inputFile, char* outputFile) {
    opterr = 0;
    int opt = 0;

    while((opt = getopt(argc, argv, "s:q:")) != -1){
        switch (opt) {
            case 's':
                strcpy(s, optarg);
                break;
            case 'q':
                *q = atoi(optarg);
                break;
        }
    }

    strcpy(inputFile, argv[optind++]);
    strcpy(outputFile, argv[optind]);

    return 0;
}

// FF: First-Come-First-Served function
int FF (struct process **p, unsigned numProcesses) {
    // Initialize queues
    initializeProcessQueue(&arrivalQueue);
	initializeProcessQueue(&readyQueue);
    initializeProcessQueue(&waitingQueue);
	initializeProcessQueue(&finishQueue);
    initializeProcessQueue(&tempQueue);

    // Initialize variables
    int clock = 0;
    char name[numProcesses];
    for (int x = 0; x < numProcesses; x++) {
        name[x] = 0;
    }
    int namePresent = 0;
    int nameCounter = 0;

    // Enqueue all processes to the arrival queue
    for (int k = 0; k < numProcesses; k++) {
        enqueueProcess(&arrivalQueue, p[k]);
    }

    // While there are still processes not yet finished
    while (finishQueue.size != numProcesses){
        // If the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue
        if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
            // Move the process to the ready queue
            enqueueProcess(&readyQueue, arrivalQueue.head->data);
            dequeueProcess(&arrivalQueue);
        }

        // If the waiting queue contains a process
        if (waitingQueue.size != 0) {
            // If the total wait time is larger than or equal to the burst cycle
            if (waitingQueue.head->data->totalWait >= 
                waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                // Decrement the ioCount
                waitingQueue.head->data->ioCount--;
                // Move the process to the ready queue
                enqueueProcess(&readyQueue, waitingQueue.head->data);
                dequeueProcess(&waitingQueue);
            } 
        }

        // If the ready queue contains a process
        if (readyQueue.size != 0) {
            // Initiate variables
            namePresent = 0;

            // Set the start value of the process only if the process has not been started
            // Loop trhough the name array to check if the process name exists
            for (int x = 0; x < numProcesses; x++) {
                // If the process name exists, set namePresent as 1
                if (name[x] == readyQueue.head->data->processName) {
                    namePresent = 1;
                }
            }
            // If process name is not present in the name array
            if (namePresent == 0) {
                // Update the process start number
                readyQueue.head->data->start = clock;
                // Add the process name into the name array
                name[nameCounter] =  readyQueue.head->data->processName;
                nameCounter++;
            }

            while (1) {
                // If the process is complete, leave the loop
                if (readyQueue.head->data->processing == 0) {
                    // Finish process
                    readyQueue.head->data->finish = clock;

                    // Once the process is completed, move the process from the ready queue to the finish queue
                    enqueueProcess(&finishQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);
                    
                    break;
                }

                // Decrement the processing time, increment the clock, and decrement the I/O wait time of the process in the wait queue
                readyQueue.head->data->processing--;
                clock++;
                if (waitingQueue.size != 0) {
                    // Initialize variables
                    int waitSize = waitingQueue.size;
                    int waitNum[waitSize];
                    int maxPoint = 0;
                    int tempMax = 0;

                    // Increment the total wait time for all processes in the wait queue
                    while (waitSize != 0) {
                        // Increment the total wait time
                        waitingQueue.head->data->totalWait++;

                        // Get the total weight for each process in the wait queue
                        waitNum[maxPoint] = waitingQueue.head->data->totalWait;
                        maxPoint++;

                        // Move the updated process to the back of the wait queue
                        enqueueProcess(&waitingQueue, waitingQueue.head->data);
                        dequeueProcess(&waitingQueue);

                        // Decrement the size of the wait queue
                        waitSize--;
                    }

                    waitSize = waitingQueue.size;
                    tempMax = waitNum[0];
                    // For loop to find the largest total weight in the wait queue
                    for (int k = 0; k < waitSize; k++) {
                        if (tempMax < waitNum[k]) {
                            tempMax = waitNum[k];
                            maxPoint = k;
                        }
                    }

                    // // Loop through all the processes in the wait queue
                    // while (waitSize != 0) {
                    //     // If the largest total weight does not match the total weight of the process
                    //     if (waitNum[maxPoint] != waitingQueue.head->data->totalWait) {
                    //         enqueueProcess(&waitingQueue, waitingQueue.head->data);
                    //         dequeueProcess(&waitingQueue);
                    //     }

                    //     // Decrement the size of the wait queue
                    //     waitSize--;
                    // }

                    waitSize = waitingQueue.size;
                    while (waitSize != 0) {
                        // If the total wait time is larger than or equal to the burst cycle
                        if (waitingQueue.head->data->totalWait >= 
                            waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                            // Decrement the ioCount
                            waitingQueue.head->data->ioCount--;
                            enqueueProcess(&readyQueue, waitingQueue.head->data);
                            dequeueProcess(&waitingQueue);
                        } 

                        // Decrement the size of the wait queue
                        waitSize--;
                    }
                }

                // If the I/O time matches how many ticks on the process
                if (readyQueue.head->data->ioTime[readyQueue.head->data->ioCounter - readyQueue.head->data->ioCount] == 
                    (readyQueue.head->data->originalProcessing - readyQueue.head->data->processing)) {
                    // Move the process from the ready queue to the waiting queue
                    enqueueProcess(&waitingQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);

                    break;
                } 
            }
        } 
        // Else increment the clock and decrement the I/O wait time of the process in the wait queue
        else {
            clock++;
            if (waitingQueue.size != 0) {
                // Initialize variables
                int waitSize = waitingQueue.size;
                int waitNum[waitSize];
                int maxPoint = 0;
                int tempMax = 0;

                // Increment the total wait time for all processes in the wait queue
                while (waitSize != 0) {
                    // Increment the total wait time
                    waitingQueue.head->data->totalWait++;

                    // Get the total weight for each process in the wait queue
                    waitNum[maxPoint] = waitingQueue.head->data->totalWait;
                    maxPoint++;

                    // Move the updated process to the back of the wait queue
                    enqueueProcess(&waitingQueue, waitingQueue.head->data);
                    dequeueProcess(&waitingQueue);

                    // Decrement the size of the wait queue
                    waitSize--;
                }

                waitSize = waitingQueue.size;
                // For loop to find the largest total weight in the wait queue
                for (int k = 0; k < waitSize; k++) {
                    if (tempMax < waitNum[k]) {
                        tempMax = waitNum[k];
                        maxPoint = k;
                    }
                }

                // // Loop through all the processes in the wait queue
                // while (waitSize != 0) {
                //     // If the largest total weight does not match the total weight of the process
                //     if (waitNum[maxPoint] != waitingQueue.head->data->totalWait) {
                //         enqueueProcess(&waitingQueue, waitingQueue.head->data);
                //         dequeueProcess(&waitingQueue);
                //     }

                //     // Decrement the size of the wait queue
                //     waitSize--;
                // }

                waitSize = waitingQueue.size;
                while (waitSize != 0) {
                    // If the total wait time is larger than or equal to the burst cycle
                    if (waitingQueue.head->data->totalWait >= 
                        waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                        // Decrement the ioCount
                        waitingQueue.head->data->ioCount--;
                        // Move the process to the ready queue
                        enqueueProcess(&readyQueue, waitingQueue.head->data);
                        dequeueProcess(&waitingQueue);
                    } 

                    // Decrement the size of the wait queue
                    waitSize--;
                }
            }
        }
	}

    // Dequeue all processes in the finish queue
    for (int k = 0; k < numProcesses; k++) {
        dequeueProcess(&finishQueue);
    }

    return 0;
}

// RR: Round Robin (with settable quantum) function
int RR (struct process **p, unsigned numProcesses, unsigned q) {
    // Initialize queues
    initializeProcessQueue(&arrivalQueue);
	initializeProcessQueue(&readyQueue);
    initializeProcessQueue(&waitingQueue);
	initializeProcessQueue(&finishQueue);
    initializeProcessQueue(&tempQueue);

    // Initialize variables
    int clock = 0;
    char name[numProcesses];
    for (int x = 0; x < numProcesses; x++) {
        name[x] = 0;
    }
    int namePresent = 0;
    int nameCounter = 0;
    int quanta = 0;

    // Enqueue all processes to the arrival queue
    for (int k = 0; k < numProcesses; k++) {
        enqueueProcess(&arrivalQueue, p[k]);
    }

    // While there are still processes not finished yet
    while (finishQueue.size != numProcesses){
        // If the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue, 
        if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
            // Move the process to the ready queue
            enqueueProcess(&readyQueue, arrivalQueue.head->data);
            dequeueProcess(&arrivalQueue);
        }

        // If the waiting queue contains a process
        if (waitingQueue.size != 0) {
            // If the total wait time is larger than or equal to the burst cycle
            if (waitingQueue.head->data->totalWait >= 
                waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                // Decrement the ioCount
                waitingQueue.head->data->ioCount--;
                // Move the process to the wait queue
                enqueueProcess(&readyQueue, waitingQueue.head->data);
                dequeueProcess(&waitingQueue);
            } 
        }

        // If the ready queue contains a process
        if (readyQueue.size != 0) {
            // Initiate variables
            namePresent = 0;
            quanta = 0;

            // Set the start value of the process only if the process has not been started
            // Loop trhough the name array to check if the process name exists
            for (int x = 0; x < numProcesses; x++) {
                // If the process name exists, set namePresent as 1
                if (name[x] == readyQueue.head->data->processName) {
                    namePresent = 1;
                }
            }
            // If process name is not present in the name array
            if (namePresent == 0) {
                // Update the process start number
                readyQueue.head->data->start = clock;
                // Add the process name into the name array
                name[nameCounter] =  readyQueue.head->data->processName;
                nameCounter++;
            }

            while (1) {
                // If the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue, 
                if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
                    // Move the process to the ready queue
                    enqueueProcess(&readyQueue, arrivalQueue.head->data);
                    dequeueProcess(&arrivalQueue);
                }

                // If the process is complete, leave the loop
                if (readyQueue.head->data->processing == 0) {
                    // Finish process
                    readyQueue.head->data->finish = clock;

                    // Once the process is completed, move the process from the ready queue to the finish queue
                    enqueueProcess(&finishQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);

                    // If the I/O time matches how many ticks on the waiting process
                    // then it was placed into the queue because of the quantum
                    if (waitingQueue.size != 0) {
                        if (waitingQueue.head->data->ioTime[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount] != 
                            (waitingQueue.head->data->originalProcessing - waitingQueue.head->data->processing)) {
                            // Move the process from the waiting queue to the ready queue
                            enqueueProcess(&readyQueue, waitingQueue.head->data);
                            dequeueProcess(&waitingQueue);
                        } 
                    }
                    
                    break;
                }

                // If the quanta is reached, move the process from the ready queue to the waiting queue
                if (quanta == q) {
                    enqueueProcess(&waitingQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);

                    break;
                }

                // Decrement the processing time, increment the clock, and decrement the I/O wait time of the processes in the wait queue                readyQueue.head->data->processing--;
                readyQueue.head->data->processing--;
                clock++;
                quanta++;

                // If the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue, it is moved to the Ready Queue
                if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
                    enqueueProcess(&readyQueue, arrivalQueue.head->data);
                    dequeueProcess(&arrivalQueue);
                }

                if (waitingQueue.size != 0) {
                    // If the I/O time matches how many ticks on the waiting process
                    // then it was placed into the queue because of the quantum
                    if (waitingQueue.head->data->ioTime[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount] != 
                        (waitingQueue.head->data->originalProcessing - waitingQueue.head->data->processing)) {
                        // Move the process from the waiting queue to the ready queue
                        enqueueProcess(&readyQueue, waitingQueue.head->data);
                        dequeueProcess(&waitingQueue);
                    } 

                    // Initialize variables
                    int waitSize = waitingQueue.size;
                    int waitNum[waitSize];
                    int maxPoint = 0;
                    int tempMax = 0;

                    // Increment the total wait time for all processes in the wait queue
                    while (waitSize != 0) {
                        // Increment the total wait time
                        waitingQueue.head->data->totalWait++;

                        // Get the total weight for each process in the wait queue
                        waitNum[maxPoint] = waitingQueue.head->data->totalWait;
                        maxPoint++;

                        // Move the updated process to the back of the wait queue
                        enqueueProcess(&waitingQueue, waitingQueue.head->data);
                        dequeueProcess(&waitingQueue);

                        // Decrement the size of the wait queue
                        waitSize--;
                    }

                    // Get the size of the waiting queue
                    waitSize = waitingQueue.size;
                    // For loop to find the largest total weight in the wait queue
                    for (int k = 0; k < waitSize; k++) {
                        if (tempMax < waitNum[k]) {
                            tempMax = waitNum[k];
                            maxPoint = k;
                        }
                    }

                    // Get the size of the waiting queue
                    waitSize = waitingQueue.size;
                    while (waitSize != 0) {
                        // If the total wait time is larger than or equal to the burst cycle
                        if (waitingQueue.head->data->totalWait >= 
                            waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                            // Decrement the ioCount
                            waitingQueue.head->data->ioCount--;
                            // Move the process to the ready queue
                            enqueueProcess(&readyQueue, waitingQueue.head->data);
                            dequeueProcess(&waitingQueue);
                        } 

                        // Decrement the size of the wait queue
                        waitSize--;
                    }
                }

                // If the I/O time matches how many ticks the process has gone through
                if (readyQueue.head->data->ioTime[readyQueue.head->data->ioCounter - readyQueue.head->data->ioCount] == 
                    (readyQueue.head->data->originalProcessing - readyQueue.head->data->processing)) {
                    // Move the process from the ready queue to the waiting queue
                    enqueueProcess(&waitingQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);

                    break;
                } 
            }
        } 
        // Else increment the clock and decrement the I/O wait time of the process in the wait queue
        else {
            clock++;
            if (waitingQueue.size != 0) {
                // If the I/O time matches how many ticks on the waiting process
                // then it was placed into the queue because of the quantum
                if (waitingQueue.head->data->ioTime[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount] != 
                    (waitingQueue.head->data->originalProcessing - waitingQueue.head->data->processing)) {
                    // Move the process from the waiting queue to the ready queue
                    enqueueProcess(&readyQueue, waitingQueue.head->data);
                    dequeueProcess(&waitingQueue);
                } 

                // Initialize variables
                int waitSize = waitingQueue.size;
                int waitNum[waitSize];
                int maxPoint = 0;
                int tempMax = 0;

                // Increment the total wait time for all processes in the wait queue
                while (waitSize != 0) {
                    // Increment the total wait time
                    waitingQueue.head->data->totalWait++;

                    // Get the total weight for each process in the wait queue
                    waitNum[maxPoint] = waitingQueue.head->data->totalWait;
                    maxPoint++;

                    // Move the updated process to the back of the wait queue
                    enqueueProcess(&waitingQueue, waitingQueue.head->data);
                    dequeueProcess(&waitingQueue);

                    // Decrement the size of the wait queue
                    waitSize--;
                }

                waitSize = waitingQueue.size;
                // For loop to find the largest total weight in the wait queue
                for (int k = 0; k < waitSize; k++) {
                    if (tempMax < waitNum[k]) {
                        tempMax = waitNum[k];
                        maxPoint = k;
                    }
                }

                waitSize = waitingQueue.size;
                while (waitSize != 0) {
                    // If the total wait time is larger than or equal to the burst cycle
                    if (waitingQueue.head->data->totalWait >= 
                        waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                        // Decrement the ioCount
                        waitingQueue.head->data->ioCount--;
                        // Move the process to the ready queue
                        enqueueProcess(&readyQueue, waitingQueue.head->data);
                        dequeueProcess(&waitingQueue);
                    } 

                    // Decrement the size of the wait queue
                    waitSize--;
                }
            }
        }
	}

    // Dequeue all processes in the finish queue
    for (int k = 0; k < numProcesses; k++) {
        dequeueProcess(&finishQueue);
    }

    return 0;
}

// SP: Shortest Process Next function
int SP (struct process **p, unsigned numProcesses) {
    // Initialize queues
    initializeProcessQueue(&arrivalQueue);
	initializeProcessQueue(&readyQueue);
    initializeProcessQueue(&waitingQueue);
	initializeProcessQueue(&finishQueue);
    initializeProcessQueue(&tempQueue);

    // Initialize variables
    int clock = 0;
    char name[numProcesses];
    for (int x = 0; x < numProcesses; x++) {
        name[x] = 0;
    }
    int namePresent = 0;
    int nameCounter = 0;

    // Enqueue all processes to the arrival queue
    for (int k = 0; k < numProcesses; k++) {
        enqueueProcess(&arrivalQueue, p[k]);
    }

    // While there are still processes not finished
    while (finishQueue.size != numProcesses){
        // If the CPU cycle clock is larger than or eequal to the arrival time for a process in the Arrival Queue, 
        if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
            // Move the process to the ready queue
            enqueueProcess(&readyQueue, arrivalQueue.head->data);
            dequeueProcess(&arrivalQueue);
        }

         // If the waiting queue contains a process
        if (waitingQueue.size != 0) {
            // If the total wait time is larger than or equal to the burst cycle,
            if (waitingQueue.head->data->totalWait >= 
                waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                // Decrement the ioCount
                waitingQueue.head->data->ioCount--;
                // Move the process to the ready queue
                enqueueProcess(&readyQueue, waitingQueue.head->data);
                dequeueProcess(&waitingQueue);
            } 
        }

        // If the ready queue has more than 1 process, move the process with the shortest process to the front
        if (readyQueue.size > 1) {
            // If the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue,
            if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
                // Move the process to the ready queue
                enqueueProcess(&readyQueue, arrivalQueue.head->data);
                dequeueProcess(&arrivalQueue);
            }
            
            // Initialize variables
            int readySize = readyQueue.size;
            int readyNum[readySize];
            int minPoint = 0;
            int tempMin = 0;

            // Loop through all the ready processes to populate the readyNum array with all the process' time
            while (readySize != 0) {
                // Get the total weight for each process in the ready queue
                readyNum[minPoint] = readyQueue.head->data->originalProcessing;

                minPoint++;

                // Move the updated process to the back of the ready queue
                enqueueProcess(&readyQueue, readyQueue.head->data);
                dequeueProcess(&readyQueue);

                // Decrement the size of the ready queue
                readySize--;
            }

            // Get the size of the ready queue and initialsize variables
            readySize = readyQueue.size;
            tempMin = readyNum[0];

            // Loop through all the ready processes to find the shortest process
            for (int k = 0; k < readySize; k++) {
                if (tempMin > readyNum[k]) {
                    tempMin = readyNum[k];
                    minPoint = k;
                }
            }

            // Loop through all the processes in the ready queue
            while (readySize != 0) {
                // If the shortest process matches the process time, leave it at the front of the queue
                if (readyNum[minPoint] != readyQueue.head->data->originalProcessing) {
                    enqueueProcess(&readyQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);
                }
                
                // Decrement the size of the ready queue
                readySize--;
            }
        }

        // If the ready queue contains a process
        if (readyQueue.size != 0) {
            // Initiate variables
            namePresent = 0;

            // Set the start value of the process only if the process has not been started
            // Loop trhough the name array to check if the process name exists
            for (int x = 0; x < numProcesses; x++) {
                // If the process name exists, set namePresent as 1
                if (name[x] == readyQueue.head->data->processName) {
                    namePresent = 1;
                }
            }
            // If process name is not present in the name array
            if (namePresent == 0) {
                // Update the process start number
                readyQueue.head->data->start = clock;
                // Add the process name into the name array
                name[nameCounter] =  readyQueue.head->data->processName;
                nameCounter++;
            }

            while (1) {
                // If the process is complete, leave the loop
                if (readyQueue.head->data->processing == 0) {
                    // Finish process
                    readyQueue.head->data->finish = clock;

                    // Once the process is completed, move the process from the ready queue to the finish queue
                    enqueueProcess(&finishQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);
                    
                    break;
                }

                // Decrement the processing time, increment the clock, and decrement the I/O wait time of the processes in the wait queue
                readyQueue.head->data->processing--;
                clock++;
                if (waitingQueue.size != 0) {
                    // Initialize variables
                    int waitSize = waitingQueue.size;
                    int waitNum[waitSize];
                    int maxPoint = 0;
                    int tempMax = 0;

                    // Increment the total wait time for all processes in the wait queue
                    while (waitSize != 0) {
                        // Increment the total wait time
                        waitingQueue.head->data->totalWait++;

                        // Get the total weight for each process in the wait queue
                        waitNum[maxPoint] = waitingQueue.head->data->totalWait;
                        maxPoint++;

                        // Move the updated process to the back of the wait queue
                        enqueueProcess(&waitingQueue, waitingQueue.head->data);
                        dequeueProcess(&waitingQueue);

                        // Decrement the size of the wait queue
                        waitSize--;
                    }

                    waitSize = waitingQueue.size;
                    // For loop to find the largest total weight in the wait queue
                    for (int k = 0; k < waitSize; k++) {
                        if (tempMax < waitNum[k]) {
                            tempMax = waitNum[k];
                            maxPoint = k;
                        }
                    }

                    waitSize = waitingQueue.size;
                    while (waitSize != 0) {
                        // If the total wait time is larger than or equal to the burst cycle
                        if (waitingQueue.head->data->totalWait >= 
                            waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                            // Decrement the ioCount
                            waitingQueue.head->data->ioCount--;
                            // Move the process to the ready queue
                            enqueueProcess(&readyQueue, waitingQueue.head->data);
                            dequeueProcess(&waitingQueue);
                        } 

                        // Decrement the size of the wait queue
                        waitSize--;
                    }
                }

                // If the I/O time matches how many ticks the process has gone through
                if (readyQueue.head->data->ioTime[readyQueue.head->data->ioCounter - readyQueue.head->data->ioCount] == 
                    (readyQueue.head->data->originalProcessing - readyQueue.head->data->processing)) {
                    // Move the process from the ready queue to the waiting queue
                    enqueueProcess(&waitingQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);

                    break;
                } 
            }
        } 
        // Else increment the clock and decrement the I/O wait time of the process in the wait queue
        else {
            clock++;
            if (waitingQueue.size != 0) {
                // Initialize variables
                int waitSize = waitingQueue.size;
                int waitNum[waitSize];
                int maxPoint = 0;
                int tempMax = 0;

                // Increment the total wait time for all processes in the wait queue
                while (waitSize != 0) {
                    // Increment the total wait time
                    waitingQueue.head->data->totalWait++;

                    // Get the total weight time for each process in the wait queue
                    waitNum[maxPoint] = waitingQueue.head->data->totalWait;
                    maxPoint++;

                    // Move the updated process to the back of the wait queue
                    enqueueProcess(&waitingQueue, waitingQueue.head->data);
                    dequeueProcess(&waitingQueue);

                    // Decrement the size of the wait queue
                    waitSize--;
                }

                waitSize = waitingQueue.size;
                // For loop to find the largest total weight in the wait queue
                for (int k = 0; k < waitSize; k++) {
                    if (tempMax < waitNum[k]) {
                        tempMax = waitNum[k];
                        maxPoint = k;
                    }
                }

                waitSize = waitingQueue.size;
                while (waitSize != 0) {
                    // If the total wait time is larger than or equal to the burst cycle, move the process to the ready queue
                    if (waitingQueue.head->data->totalWait >= 
                        waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                        // Decrement the ioCount
                        waitingQueue.head->data->ioCount--;
                        enqueueProcess(&readyQueue, waitingQueue.head->data);
                        dequeueProcess(&waitingQueue);
                    } 

                    // Decrement the size of the wait queue
                    waitSize--;
                }
            }
        }
	}

    // Dequeue all processes in the finish queue
    for (int k = 0; k < numProcesses; k++) {
        dequeueProcess(&finishQueue);
    }

    return 0;
}

// SR: Shortest Remaining Time function
int SR (struct process **p, unsigned numProcesses) {
    // Initialize queues
    initializeProcessQueue(&arrivalQueue);
	initializeProcessQueue(&readyQueue);
    initializeProcessQueue(&waitingQueue);
	initializeProcessQueue(&finishQueue);
    initializeProcessQueue(&tempQueue);

    // Initialize variables
    int clock = 0;
    char name[numProcesses];
    for (int x = 0; x < numProcesses; x++) {
        name[x] = 0;
    }
    int namePresent = 0;
    int nameCounter = 0;

    // Enqueue all processes to the arrival queue
    for (int k = 0; k < numProcesses; k++) {
        enqueueProcess(&arrivalQueue, p[k]);
    }

    // While there are still processes not finished
    while (finishQueue.size != numProcesses){
        // If the CPU cycle clock is larger than or eequal to the arrival time for a process in the Arrival Queue, 
        if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
            // Move the process to the ready queue
            enqueueProcess(&readyQueue, arrivalQueue.head->data);
            dequeueProcess(&arrivalQueue);
        }

         // If the waiting queue contains a process
        if (waitingQueue.size != 0) {
            // If the total wait time is larger than or equal to the burst cycle,
            if (waitingQueue.head->data->totalWait >= 
                waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                // Decrement the ioCount
                waitingQueue.head->data->ioCount--;
                // Move the process to the ready queue
                enqueueProcess(&readyQueue, waitingQueue.head->data);
                dequeueProcess(&waitingQueue);
            } 
        }

        // If the ready queue has more than 1 process, move the process with the shortest process to the front
        if (readyQueue.size > 1) {
            // If the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue,
            if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
                // Move the process to the ready queue
                enqueueProcess(&readyQueue, arrivalQueue.head->data);
                dequeueProcess(&arrivalQueue);
            }
            
            // Initialize variables
            int readySize = readyQueue.size;
            int readyNum[readySize];
            int minPoint = 0;
            int tempMin = 0;

            // Loop through all the ready processes to populate the readyNum array with all the process' time
            while (readySize != 0) {
                // Get the remaining process time for each process in the ready queue
                readyNum[minPoint] = readyQueue.head->data->processing;

                minPoint++;

                // Move the updated process to the back of the ready queue
                enqueueProcess(&readyQueue, readyQueue.head->data);
                dequeueProcess(&readyQueue);

                // Decrement the size of the ready queue
                readySize--;
            }

            // Get the size of the ready queue and initialsize variables
            readySize = readyQueue.size;
            tempMin = readyNum[0];

            // Loop through all the ready processes to find the shortest remaining process
            for (int k = 0; k < readySize; k++) {
                if (tempMin > readyNum[k]) {
                    tempMin = readyNum[k];
                    minPoint = k;
                }
            }
            
            // Loop through all the processes in the ready queue
            while (readySize != 0) {
                // If the shortest remaining process matches the process time, leave it at the front of the queue
                if (readyNum[minPoint] != readyQueue.head->data->originalProcessing) {
                    enqueueProcess(&readyQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);
                }
                
                // Decrement the size of the ready queue
                readySize--;
            }
        }

        // If the ready queue contains a process
        if (readyQueue.size != 0) {
            // Initiate variables
            namePresent = 0;

            // Set the start value of the process only if the process has not been started
            // Loop trhough the name array to check if the process name exists
            for (int x = 0; x < numProcesses; x++) {
                // If the process name exists, set namePresent as 1
                if (name[x] == readyQueue.head->data->processName) {
                    namePresent = 1;
                }
            }
            // If process name is not present in the name array
            if (namePresent == 0) {
                // Update the process start number
                readyQueue.head->data->start = clock;
                // Add the process name into the name array
                name[nameCounter] =  readyQueue.head->data->processName;
                nameCounter++;
            }

            while (1) {
                // If the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue,
                if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
                    // Move the process to the ready queue
                    enqueueProcess(&readyQueue, arrivalQueue.head->data);
                    dequeueProcess(&arrivalQueue);

                    break;
                }

                // If the process is complete, leave the loop
                if (readyQueue.head->data->processing == 0) {
                    // Finish process
                    readyQueue.head->data->finish = clock;

                    // Once the process is completed, move the process from the ready queue to the finish queue
                    enqueueProcess(&finishQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);
                    
                    break;
                }

                // Decrement the processing time, increment the clock, and decrement the I/O wait time of the processes in the wait queue
                readyQueue.head->data->processing--;
                clock++;
                if (waitingQueue.size != 0) {
                    // Initialize variables
                    int waitSize = waitingQueue.size;
                    int waitNum[waitSize];
                    int maxPoint = 0;
                    int tempMax = 0;

                    // Increment the total wait time for all processes in the wait queue
                    while (waitSize != 0) {
                        // Increment the total wait time
                        waitingQueue.head->data->totalWait++;

                        // Get the total weight for each process in the wait queue
                        waitNum[maxPoint] = waitingQueue.head->data->totalWait;
                        maxPoint++;

                        // Move the updated process to the back of the wait queue
                        enqueueProcess(&waitingQueue, waitingQueue.head->data);
                        dequeueProcess(&waitingQueue);

                        // Decrement the size of the wait queue
                        waitSize--;
                    }

                    waitSize = waitingQueue.size;
                    // For loop to find the largest total weight in the wait queue
                    for (int k = 0; k < waitSize; k++) {
                        if (tempMax < waitNum[k]) {
                            tempMax = waitNum[k];
                            maxPoint = k;
                        }
                    }

                    waitSize = waitingQueue.size;
                    while (waitSize != 0) {
                        // If the total wait time is larger than or equal to the burst cycle
                        if (waitingQueue.head->data->totalWait >= 
                            waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                            // Decrement the ioCount
                            waitingQueue.head->data->ioCount--;
                            // Move the process to the ready queue
                            enqueueProcess(&readyQueue, waitingQueue.head->data);
                            dequeueProcess(&waitingQueue);
                        } 

                        // Decrement the size of the wait queue
                        waitSize--;
                    }
                }

                // If the I/O time matches how many ticks the process has gone through
                if (readyQueue.head->data->ioTime[readyQueue.head->data->ioCounter - readyQueue.head->data->ioCount] == 
                    (readyQueue.head->data->originalProcessing - readyQueue.head->data->processing)) {
                    // Move the process from the ready queue to the waiting queue
                    enqueueProcess(&waitingQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);

                    break;
                } 
            }
        } 
        // Else increment the clock and decrement the I/O wait time of the process in the wait queue
        else {
            clock++;
            if (waitingQueue.size != 0) {
                // Initialize variables
                int waitSize = waitingQueue.size;
                int waitNum[waitSize];
                int maxPoint = 0;
                int tempMax = 0;

                // Increment the total wait time for all processes in the wait queue
                while (waitSize != 0) {
                    // Increment the total wait time
                    waitingQueue.head->data->totalWait++;

                    // Get the total weight time for each process in the wait queue
                    waitNum[maxPoint] = waitingQueue.head->data->totalWait;
                    maxPoint++;

                    // Move the updated process to the back of the wait queue
                    enqueueProcess(&waitingQueue, waitingQueue.head->data);
                    dequeueProcess(&waitingQueue);

                    // Decrement the size of the wait queue
                    waitSize--;
                }

                waitSize = waitingQueue.size;
                // For loop to find the largest total weight in the wait queue
                for (int k = 0; k < waitSize; k++) {
                    if (tempMax < waitNum[k]) {
                        tempMax = waitNum[k];
                        maxPoint = k;
                    }
                }

                waitSize = waitingQueue.size;
                while (waitSize != 0) {
                    // If the total wait time is larger than or equal to the burst cycle, move the process to the ready queue
                    if (waitingQueue.head->data->totalWait >= 
                        waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                        // Decrement the ioCount
                        waitingQueue.head->data->ioCount--;
                        enqueueProcess(&readyQueue, waitingQueue.head->data);
                        dequeueProcess(&waitingQueue);
                    } 

                    // Decrement the size of the wait queue
                    waitSize--;
                }
            }
        }
	}

    // Dequeue all processes in the finish queue
    for (int k = 0; k < numProcesses; k++) {
        dequeueProcess(&finishQueue);
    }

    return 0;
}

// HR: Highest Response Ratio Next function
int HR (struct process **p, unsigned numProcesses) {
    // Initialize queues
    initializeProcessQueue(&arrivalQueue);
	initializeProcessQueue(&readyQueue);
    initializeProcessQueue(&waitingQueue);
	initializeProcessQueue(&finishQueue);
    initializeProcessQueue(&tempQueue);

    // Initialize variables
    int clock = 0;
    char name[numProcesses];
    for (int x = 0; x < numProcesses; x++) {
        name[x] = 0;
    }
    int namePresent = 0;
    int nameCounter = 0;

    // Enqueue all processes to the arrival queue
    for (int k = 0; k < numProcesses; k++) {
        enqueueProcess(&arrivalQueue, p[k]);
    }

    // While there are still processes not finished
    while (finishQueue.size != numProcesses){
        // If the CPU cycle clock is larger than or eequal to the arrival time for a process in the Arrival Queue, 
        if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
            // Move the process to the ready queue
            enqueueProcess(&readyQueue, arrivalQueue.head->data);
            dequeueProcess(&arrivalQueue);
        }

         // If the waiting queue contains a process
        if (waitingQueue.size != 0) {
            // If the total wait time is larger than or equal to the burst cycle,
            if (waitingQueue.head->data->totalWait >= 
                waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                // Decrement the ioCount
                waitingQueue.head->data->ioCount--;
                // Move the process to the ready queue
                enqueueProcess(&readyQueue, waitingQueue.head->data);
                dequeueProcess(&waitingQueue);
            } 
        }

        // If the ready queue contains a process
        if (readyQueue.size != 0) {
            // Initiate variables
            namePresent = 0;

            // Set the start value of the process only if the process has not been started
            // Loop trhough the name array to check if the process name exists
            for (int x = 0; x < numProcesses; x++) {
                // If the process name exists, set namePresent as 1
                if (name[x] == readyQueue.head->data->processName) {
                    namePresent = 1;
                }
            }
            // If process name is not present in the name array
            if (namePresent == 0) {
                // Update the process start number
                readyQueue.head->data->start = clock;
                // Add the process name into the name array
                name[nameCounter] =  readyQueue.head->data->processName;
                nameCounter++;
            }

            while (1) {
                // If the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue, 
                if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
                    // Move the process to the ready queue
                    enqueueProcess(&readyQueue, arrivalQueue.head->data);
                    dequeueProcess(&arrivalQueue);
                }

                // If the process is complete, leave the loop
                if (readyQueue.head->data->processing == 0) {
                    // Finish process
                    readyQueue.head->data->finish = clock;

                    // Once the process is completed, move the process from the ready queue to the finish queue
                    enqueueProcess(&finishQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);

                    // If the ready queue has more than 1 process, move the process with the Highest Response Ratio to the front
                    if (readyQueue.size > 1) {
                        // If the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue,
                        if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
                            // Move the process to the ready queue
                            enqueueProcess(&readyQueue, arrivalQueue.head->data);
                            dequeueProcess(&arrivalQueue);
                        }
                        
                        // Initialize variables
                        int readySize = readyQueue.size;
                        double hrr[readySize];
                        int maxPoint = 0;
                        double tempMax = 0;

                        // Loop through all the ready processes to populate the readyNum array with all the process' hrr
                        while (readySize != 0) {
                            // Calculate the hrr for each process in the ready queue
                            hrr[maxPoint] = ((clock - readyQueue.head->data->arrival) + 
                                            readyQueue.head->data->processing)/
                                            readyQueue.head->data->originalProcessing;

                            maxPoint++;

                            // Move the updated process to the back of the ready queue
                            enqueueProcess(&readyQueue, readyQueue.head->data);
                            dequeueProcess(&readyQueue);

                            // Decrement the size of the ready queue
                            readySize--;
                        }

                        // Get the size of the ready queue and initialsize variables
                        readySize = readyQueue.size;
                        tempMax = hrr[0];
                        maxPoint = 0;

                        // Loop through all the ready processes to find the shortest process
                        for (int k = 0; k < readySize; k++) {
                            if (tempMax < hrr[k]) {
                                tempMax = hrr[k];
                                maxPoint = k;
                            }
                        }

                        // Loop through all the processes in the ready queue
                        while (readySize != 0) {
                            // If the hrr matches the process' rr, leave it at the front of the queue
                            if (hrr[maxPoint] != ((clock - readyQueue.head->data->arrival) + 
                                            readyQueue.head->data->processing)/
                                            readyQueue.head->data->originalProcessing) {
                                enqueueProcess(&readyQueue, readyQueue.head->data);
                                dequeueProcess(&readyQueue);
                            }
                            
                            // Decrement the size of the ready queue
                            readySize--;
                        }
                    }
                    
                    break;
                }

                // Decrement the processing time, increment the clock, and decrement the I/O wait time of the processes in the wait queue
                readyQueue.head->data->processing--;
                clock++;

                // If the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue, 
                if (arrivalQueue.size != 0 && clock >= arrivalQueue.head->data->arrival) {
                    // Move the process to the ready queue
                    enqueueProcess(&readyQueue, arrivalQueue.head->data);
                    dequeueProcess(&arrivalQueue);
                }
                
                if (waitingQueue.size != 0) {
                    // Initialize variables
                    int waitSize = waitingQueue.size;
                    int waitNum[waitSize];
                    int maxPoint = 0;
                    int tempMax = 0;

                    // Increment the total wait time for all processes in the wait queue
                    while (waitSize != 0) {
                        // Increment the total wait time
                        waitingQueue.head->data->totalWait++;

                        // Get the total weight for each process in the wait queue
                        waitNum[maxPoint] = waitingQueue.head->data->totalWait;
                        maxPoint++;

                        // Move the updated process to the back of the wait queue
                        enqueueProcess(&waitingQueue, waitingQueue.head->data);
                        dequeueProcess(&waitingQueue);

                        // Decrement the size of the wait queue
                        waitSize--;
                    }

                    waitSize = waitingQueue.size;
                    // For loop to find the largest total weight in the wait queue
                    for (int k = 0; k < waitSize; k++) {
                        if (tempMax < waitNum[k]) {
                            tempMax = waitNum[k];
                            maxPoint = k;
                        }
                    }

                    waitSize = waitingQueue.size;
                    while (waitSize != 0) {
                        // If the total wait time is larger than or equal to the burst cycle
                        if (waitingQueue.head->data->totalWait >= 
                            waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                            // Decrement the ioCount
                            waitingQueue.head->data->ioCount--;
                            // Move the process to the ready queue
                            enqueueProcess(&readyQueue, waitingQueue.head->data);
                            dequeueProcess(&waitingQueue);
                        } 

                        // Decrement the size of the wait queue
                        waitSize--;
                    }
                }

                // If the I/O time matches how many ticks the process has gone through
                if (readyQueue.head->data->ioTime[readyQueue.head->data->ioCounter - readyQueue.head->data->ioCount] == 
                    (readyQueue.head->data->originalProcessing - readyQueue.head->data->processing)) {
                    // Move the process from the ready queue to the waiting queue
                    enqueueProcess(&waitingQueue, readyQueue.head->data);
                    dequeueProcess(&readyQueue);

                    break;
                } 
            }
        } 
        // Else increment the clock and decrement the I/O wait time of the process in the wait queue
        else {
            clock++;
            if (waitingQueue.size != 0) {
                // Initialize variables
                int waitSize = waitingQueue.size;
                int waitNum[waitSize];
                int maxPoint = 0;
                int tempMax = 0;

                // Increment the total wait time for all processes in the wait queue
                while (waitSize != 0) {
                    // Increment the total wait time
                    waitingQueue.head->data->totalWait++;

                    // Get the total weight time for each process in the wait queue
                    waitNum[maxPoint] = waitingQueue.head->data->totalWait;
                    maxPoint++;

                    // Move the updated process to the back of the wait queue
                    enqueueProcess(&waitingQueue, waitingQueue.head->data);
                    dequeueProcess(&waitingQueue);

                    // Decrement the size of the wait queue
                    waitSize--;
                }

                waitSize = waitingQueue.size;
                // For loop to find the largest total weight in the wait queue
                for (int k = 0; k < waitSize; k++) {
                    if (tempMax < waitNum[k]) {
                        tempMax = waitNum[k];
                        maxPoint = k;
                    }
                }

                waitSize = waitingQueue.size;
                while (waitSize != 0) {
                    // If the total wait time is larger than or equal to the burst cycle, move the process to the ready queue
                    if (waitingQueue.head->data->totalWait >= 
                        waitingQueue.head->data->ioBurstCycle[waitingQueue.head->data->ioCounter - waitingQueue.head->data->ioCount]) {
                        // Decrement the ioCount
                        waitingQueue.head->data->ioCount--;
                        enqueueProcess(&readyQueue, waitingQueue.head->data);
                        dequeueProcess(&waitingQueue);
                    } 

                    // Decrement the size of the wait queue
                    waitSize--;
                }
            }
        }
	}

    // Dequeue all processes in the finish queue
    for (int k = 0; k < numProcesses; k++) {
        dequeueProcess(&finishQueue);
    }

    return 0;
}

// FB: Feedback (with settable quantum) function
int FB (struct process **p, unsigned numProcesses, unsigned q) {
    return 0;
}

// Function to compare finishTimes of all the processes
int compare (const void *a, const void *b) {
    const struct process **left  = (const struct process **)a;
    const struct process **right = (const struct process **)b;

    return (*left)->finish - (*right)->finish;
}

int main (int argc, char** argv) {
    // Initialise variables
    char s[20] = "";
    unsigned q = 0;
    char inputFile[20] = "";
    char outputFile[20] = "";
    errno = 0;
    FILE *input;
    FILE *output;
    char *line = malloc(64);
    char *token;
    unsigned numProcesses = 0;
    int i = 0;
    int j = 0;
    int notFirstLine = 0;
    double meanTurnaround = 0;
    double meanNormalizedTurnaround = 0;

    // Call function cmdParser to get s, q, input, and output
    cmdParser(argc, argv, s, &q, inputFile, outputFile);

    // Open the input file for reading
    input = fopen(inputFile, "r");
    // If no such file exists
    if (errno == ENOENT) {
		fprintf(stderr, "./schsim: cannot open(\"%s\"): No such file or directory\n", inputFile);
		exit(1);
	}

    // Loop through the input file to get the number of processes
    while (fgets(line, 64, input)) {
        char *original = line;
        if (original[0] == '\"') {
            numProcesses++;
        }
    }
    // Rewind the input file
    rewind(input);

    // Allocate memory for the processes
    struct process **process = malloc(numProcesses * sizeof(struct process *));
    // Allocate memory for each process
    for (int k = 0; k < numProcesses; k++) {
        process[k] = malloc(sizeof(struct process));
    }
    // Initialise processes
    for (int k = 0; k < numProcesses; k++) {
        process[k]->arrival = 0;
        process[k]->processing = 0;
        process[k]->originalProcessing = 0;
        for (int l = 0; l < 16; l++) {
            process[k]->ioTime[l] = 0;
            process[k]->ioBurstCycle[l] = 0;
        }
        process[k]->ioCount = 0;
        process[k]->ioCounter = 0;
        process[k]->start = 0;
        process[k]->totalWait = 0;
        process[k]->finish = 0;
        process[k]->turnaround = 0;
        process[k]->normalizedTurnaround = 0;
    }
    
    // Loop through the input file to get the data
    while (fgets(line, 64, input)) {
        char *original = line;
        
        // If the line is empty, read the next line
        if (strcmp(original, "\n") == 0) {
            continue;
        }
        // If the line is a comment, read the next line
        if (original[0] == '#') {
            continue;
        }

        // If the line is a continuation of the previous line
        if (original[0] == ',') {
            // Split the string into tokens using a delimiter
            token = strtok(original, ",");

            // Loop through the tokens and assign them to their respective variables
            while (token != NULL) {
                if (j == 3) {
                    process[i]->ioTime[process[i]->ioCount] = atoi(token);
                } else if (j == 4) {
                    process[i]->ioBurstCycle[process[i]->ioCount] = atoi(token);
                }
                
                token = strtok(NULL, ",");
                j++;
            }

            j = 3;
            process[i]->ioCount++;
            process[i]->ioCounter++;
            continue;
        }

        // If it's not the first line, increment i
        if (notFirstLine == 1) {
            i++; 
        }
        j = 0;

        // Split the string into tokens using a delimiter
        token = strtok(original, ",");

        // Loop through the tokens and assign them to their respective variables
        while (token != NULL) {
            if (j == 0) {
                // If the token contains double quotes, remove them
                if (token[0] == '"') {
                    token++;
                    token[strlen(token) - 1] = '\0';
                }
                process[i]->processName = *token;
            } else if (j == 1) {
                process[i]->arrival = atoi(token);
            } else if (j == 2) {
                process[i]->processing = atoi(token);
                process[i]->originalProcessing = atoi(token);
            }
            
            token = strtok(NULL, ",");
            j++;
        }

        notFirstLine = 1;
    }

    // If-else block to select which function to call according to the s value entered
    if (strcmp(s, "FF") == 0) {
        FF(process, numProcesses);
    } else if (strcmp(s, "RR") == 0) {
        RR(process, numProcesses, q);
    } else if (strcmp(s, "SP") == 0) {
        SP(process, numProcesses);
    } else if (strcmp(s, "SR") == 0) {
        SR(process, numProcesses);
    } else if (strcmp(s, "HR") == 0) {
        HR(process, numProcesses);
    } else if (strcmp(s, "FB") == 0) {
        FB(process, numProcesses, q);
    } 

    // Open the output file for writing
    output = fopen(outputFile, "w");
    // If the file cannot be opened
    if (errno == ENOENT) {
        fprintf(stderr, "./schsim: cannot open(\"%s\"): Error opening the file\n", outputFile);
		exit(1);
    }

    // Call qsort to sort the processes according to the finish time
    qsort(process, numProcesses, sizeof(struct process *), compare);

    // write to the output file
    fprintf(output, "\"name\", \"arrival time\", \"service time\", \"start time\", \"total wait time\", "
    "\"finish time\", \"turnaround time\", \"normalized turnaround\"\n");
    for (int k = 0; k < numProcesses; k++) {
        // Calculate the total wait time
        process[k]->totalWait = process[k]->start - process[k]->arrival + 
        (process[k]->finish - process[k]->start - process[k]->originalProcessing);

        // Calculate the turnaround and normalizedTurnaround times
        process[k]->turnaround = process[k]->finish - process[k]->arrival;
        process[k]->normalizedTurnaround = process[k]->turnaround/process[k]->originalProcessing;
        
        // Calculate the meanTurnaround and meanNormalizedTurnaround times
        meanTurnaround = meanTurnaround + process[k]->turnaround;
        meanNormalizedTurnaround = meanNormalizedTurnaround + process[k]->normalizedTurnaround;

        fprintf(output, "\"%c\", %d, %.0f, %d, %d, %d, %d, %.2f\n", process[k]->processName, process[k]->arrival, 
        process[k]->originalProcessing, process[k]->start, process[k]->totalWait, process[k]->finish, 
        process[k]->turnaround, process[k]->normalizedTurnaround);
    }

    // Continue calculation of the meanTurnaround and meanNormalizedTurnaround times
    meanTurnaround = meanTurnaround/numProcesses;
    meanNormalizedTurnaround = meanNormalizedTurnaround/numProcesses;

    // write to the output file
    fprintf(output, "%.2f, %.2f\n", meanTurnaround, meanNormalizedTurnaround);

    // Free allocated memory
    for (int k = 0; k < numProcesses; k++) {
        free(process[k]);
    }
    free(process);
    free(line);

    // Close files
    fclose(output);
    fclose(input);

    return 0;
}
