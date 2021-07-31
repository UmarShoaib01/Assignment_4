
//Imports
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//single Customer Struct
typedef struct customer //Represents a single Customer
{
    int customerNum; 
    int customerOrder; 
    pthread_t customerThread;
    int customerReturn; 
} Customer;

//H "file" for functions
int readFile (char* fileName);
void request_v1(char* command);
void request_v2(char* command);
void release_v1(char* command);
void release_v2(char* command);
void status();
void Run(Customer** c);
int safetyAlgorithm();
void* runCustomer(void* t);
int check_input(char* input);

//Number of clients
int numOfCustomers; 
//Number of resources
int numOfResources; 
//To allow access to command line arguments globally
char **cmdArgs = NULL; 
//Available resources
int *available_resources = NULL, 
//Allocated Resources
**allocated_resources = NULL,
//Max Resources
**max_resources = NULL, 
//Required Resources
**required_resources = NULL;
//Resources
int *res = NULL, 
//SequenceTemp
*sequenceTemp = NULL, 
//Remaining
*amount_remaining = NULL, 
*fin = NULL; 
Customer* c = NULL; 
pthread_mutex_t mutex; 
