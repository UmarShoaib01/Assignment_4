
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

int main(int argc, char **argv){

    c = (Customer*) malloc(sizeof(Customer)*numOfCustomers); 
    char *command = (char*) malloc(50);
    if (argc < 2){
        //Invalid number 
        printf("No maximum resource amounts were given. (Invalid)\n");
        return -1;
    }

    numOfResources = argc-1;
    cmdArgs = argv;
    //Call readFile function to read sample file with resources
    readFile("sample4_in.txt");

    //Print out base information, #clients, and available resources at the time 
    printf("Number of Customers: %d\n", numOfCustomers);
    printf("Currently Available Resources: ");
    for (int i = 0; i < numOfResources; i++){
        printf("%d ", available_resources[i]);
    }
    printf("\n");
    printf("Maximum Resources from file: ");
    printf("\n");
    //Print max resources
    int row = 4;
    int column = 3;
    for(int r=0;r<=row;r++){
        for(int c = 0;c<=column;c++){
            printf("%d ", max_resources[r][c]);
        }
        printf("\n");
            
    }
     //Exit while loop
    int whileLoop = 0; 
    while (whileLoop == 0){
        //Ask and get user input
        printf("Enter Request: ");
        fgets(command, 25, stdin);
        //Check input, gather all relevant information 
        whileLoop = check_input(command);
    }

    return 0;
}
