
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

int check_input(char* input){
	//Option for requestion resources
    if (memcmp("RQ", input, 2) == 0){ 
        request_v1(input);
    }
	//Option for releasing resources
    else if (memcmp("RL", input, 2) == 0){ 
        release_v1(input);
    }
	//Check status of resources
    else if (memcmp("Status", input, 1) == 0){ 
        status();
    }
	//Run threads
    else if (memcmp("Run", input, 3) == 0){ 
		Run(&c);
        return 1;
    }
    else{
        printf("Invalid Command was entered, Please enter an appropriate command...\n");
    }
    fflush(stdin); 
    return 0;
}

int readFile(char* fileName){ 

	numOfCustomers = 0;
	//Create file handle and fp for each character
	FILE *file_handle = fopen(fileName, "r");
	char fp;

	//Check if file handle exists, if not return error
	if(!file_handle){
		printf("File does not exist...\n");
		return -1;
	}

	//Count number of clients based on each whitespace at the end of each line
	fp = fgetc(file_handle);
	while (fp != EOF){
        if (fp == '\n'){
            numOfCustomers = numOfCustomers + 1; 
        }
		//Next Character
        fp = fgetc(file_handle); 
    }
	numOfCustomers = numOfCustomers + 1;

	//Set up informational matrices for use with Safety Algorithm
	available_resources = (int *)malloc(numOfResources * sizeof(int*)); //Shows resources available
	max_resources = (int **)malloc(numOfCustomers * sizeof(int *)); //Shows max required_resources resources 
    allocated_resources = (int **)malloc(numOfCustomers * sizeof(int *)); //Shows how many resources are currently allocated
    required_resources = (int **)malloc(numOfCustomers * sizeof(int *)); //Shows the remaining resources needed by each function

    //Create second dimension
	for (int i=0; i < numOfCustomers; i++){
         max_resources[i] = (int *)malloc(numOfResources * sizeof(int));
         allocated_resources[i] = (int *)malloc(numOfResources * sizeof(int));
         required_resources[i] = (int *)malloc(numOfResources * sizeof(int));
	}

    //Set up known array values:
	//Move given resource amounts into proper variable
	for (int i=1; i <= (numOfResources); i++){
		available_resources[i-1] = atoi(*(cmdArgs+i));
	}
    //allocated_resources array will begin at 0
	for (int i = 0; i < numOfCustomers; i++){
		for (int j = 0; j < numOfResources; j++){
			allocated_resources[i][j] = 0;
		}
	}

	fseek(file_handle, 0, SEEK_SET);

	char *temp;
    int i = 0, j = 0;
	fp = fgetc(file_handle);
    
	while (fp != EOF){
		if (fp != ',' && fp != '\n'){
			temp = &fp;

            //Max and need will begin at the same number (allocated_resources is at 0 across all Clients)
			max_resources[i][j] = atoi(temp);
			required_resources[i][j] = atoi(temp);
			j++;

			if (j == numOfResources){
				j = 0;
				i++;
			}
		}
		fp = fgetc(file_handle);
	}
	
	return 0;
}
