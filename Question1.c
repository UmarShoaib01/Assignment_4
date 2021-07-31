
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


void request_v1(char* command){
	char* CommandBackup = (char*) malloc(50);
	strcpy(CommandBackup, command); //Save copy of command for use in release if needed

	char *token = strtok(command, " "); //Seperate RQ from informational sections of command
	int cID = atoi(strtok(NULL, " ")); //Retrieve the ClientID
	int resourceVal; //Initialize variable to hold resource value being modified
	int valid = 0; //Verifies if request is able to be fulfilled (Does not include Safe State Check)

	if (cID >= numOfCustomers){
		printf("Error: Invalid Customer Number.\n");
		return;
	}

    int j = 0;
	token = strtok(NULL, " "); //Retrieve next resource value being requested
	while (token != NULL){
		resourceVal = atoi(token);
        //Allocate the resource amount to the thread
        allocated_resources[cID][j] = allocated_resources[cID][j] + resourceVal;
		available_resources[j] = available_resources[j] - resourceVal;
		required_resources[cID][j] = required_resources[cID][j] - resourceVal;
        //Process whether allocated_resources is possible
		if (required_resources[cID][j] < 0){ //Too many resources were requested, thread overcapacity. Will not be filled.
			valid = -1;
		}
		if (available_resources[j] < 0){ //Not enough resources are available. Will not be filled.
			valid = -2;
			
		}
		j++;
		token = strtok(NULL, " ");
	}

	switch (valid){
		case -1:
			printf("Error: Thread requested more resources than maximum allowed. Request denied\n");
			break;
		case -2:
			printf("Error: Thread requested more resources than currently available. Request denied\n");
			break;
	}

    if (valid != 0) {
		release_v2(CommandBackup); //Release resources and return to previous state
		return;
	}

	int safe = safetyAlgorithm(); //Checks if safe state is possible. 0 if able, -1 otherwise
	if (safe == -1){
		printf("Error: No safe state possible. Request denied.\n");
		release_v2(CommandBackup); //Release resources back to previous safe state
	}else{
		printf("Request valid and safe. Request Accepted. \n");
	}
}

void request_v2(char* command){
    char *token = strtok(command, " "); //Seperate RQ from informational sections of command
    int cID = atoi(strtok(NULL, " ")); //Retrieve the ClientID
    int resourceVal; //Initialize variable to hold resource value being modified
    int j = 0;
    token = strtok(NULL, " "); //Retrieve next resource value being requested
    while (token != NULL){
        resourceVal = atoi(token);
        //Allocate the resource amount to the thread
        allocated_resources[cID][j] = allocated_resources[cID][j] + resourceVal;
        available_resources[j] = available_resources[j] - resourceVal;
        required_resources[cID][j] = required_resources[cID][j] - resourceVal;
        j++;
        token = strtok(NULL, " ");
    }
}


void release_v1(char* command){
	char* CommandBackup = (char*) malloc(50);
	strcpy(CommandBackup, command); //Save copy of command for use in request if needed
	char *token = strtok(command, " "); //Remove unnecessary parts of the request
	int cID = atoi(strtok(NULL, " ")); //Retrieve the ClientID
	int j = 0;
	int error = 0;
	int resourceVal; //Initialize variable to hold resource value being modified

	if (cID >= numOfCustomers){
		printf("Error: Invalid Customer Number.\n");
		return;
	}

    token = strtok(NULL, " "); //Get resource value to modify by
	while (token != NULL){
		resourceVal = atoi(token);
		allocated_resources[cID][j] = allocated_resources[cID][j] - resourceVal; //Remove resources from amount allocated
		required_resources[cID][j] = required_resources[cID][j] + resourceVal; //Resources must be added back to the needed counter
        available_resources[j] = available_resources[j] + resourceVal; //Add the value back to globally available resources

		if (allocated_resources[cID][j] < 0){ //If request would attempt to release more resources than the Customer currently holds, deny it
			error = 1;
		}
		j++;
		token = strtok(NULL, " ");
	}
	if (error){
		printf("Error: More resource being released than Customer has allocated. Release denied.\n");
		request_v2(CommandBackup);
	}else{
		printf("Release request is valid. Release Accepted\n");
	}
}

void release_v2(char* command){
    char *token = strtok(command, " "); //Remove unnecessary parts of the request
    int cID = atoi(strtok(NULL, " ")); //Retrieve the ClientID
    int j = 0;
    int resourceVal; //Initialize variable to hold resource value being modified

    token = strtok(NULL, " "); //Get resource value to modify by
    while (token != NULL){
        resourceVal = atoi(token);
        allocated_resources[cID][j] = allocated_resources[cID][j] - resourceVal; //Remove resources from amount allocated
        required_resources[cID][j] = required_resources[cID][j] + resourceVal; //Resources must be added back to the needed counter
        available_resources[j] = available_resources[j] + resourceVal; //Add the value back to globally available resources
        j++;
        token = strtok(NULL, " ");
    }
    return;
}
