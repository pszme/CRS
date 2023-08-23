/* 
Program: 
    Car Rental System (CRS)
Authors:
    Furgeli Sherpa (furgelisherpa.github.io)
    Rohan Shilpakar 
    Rudra Pulami Magar
License:
    GPLv3
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 100
#define CLEAN_SCREEN() printf("\033c")

size_t recorded_number_of_users = 0; // Initialize to 0

struct Users 
{
  char fullname[20];
  char address[20];
  char email[20];
  char username[20];
  char password[20];
  char number[11];
};

void getInput(char *str, size_t size) 
{
  if (fgets(str, size, stdin) != NULL) {
    char *newline = strchr(str, '\n');
    if (newline != NULL) {
      *newline = '\0';
    } else {
      if (size > 0) {
        str[size - 1] = '\0';
      }
    }
  }
}

void flushInputBuffer() 
{
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {
    // Read and discard characters until a newline or EOF is encountered
  }
}

void saveHighestRecordedNumber(size_t highestNumber) 
{
  FILE *file = fopen("data/highest_recorded_number.txt", "w");
  if (file != NULL) {
    fprintf(file, "%zu", highestNumber);
    fclose(file);
  }
}

size_t loadHighestRecordedNumber() 
{
  size_t highestNumber = 0;
  FILE *file = fopen("data/highest_recorded_number.txt", "r");
  if (file != NULL) {
    fscanf(file, "%zu", &highestNumber);
    fclose(file);
  }
  return highestNumber;
}

void enterUserData(struct Users* user) 
{
  printf("Enter Full Name: ");
  getInput(user->fullname, sizeof(user->fullname));
  flushInputBuffer();
  printf("Enter Address: ");
  getInput(user->address, sizeof(user->address));
  flushInputBuffer();
  printf("Enter Contact: ");
  getInput(user->number, sizeof(user->number));
  flushInputBuffer();
  printf("Enter Email: ");
  getInput(user->email, sizeof(user->email));
  flushInputBuffer();
  printf("Thank you for providing your information, %s\n", user->fullname);
  printf("Now you can set your Username and Password for further process\n");
  printf("Enter New Username: ");
  getInput(user->username, sizeof(user->username));
  flushInputBuffer();

  char passwordVerification[20];
  do {
    printf("Enter New Password: ");
    getInput(user->password, sizeof(user->password));
    printf("Retype the password for verification: ");
    getInput(passwordVerification, sizeof(passwordVerification));

    if (strcmp(user->password, passwordVerification) == 0) {
      break; // Passwords match, exit the verification loop
    } else {
      printf("Passwords do not match. Please try again.\n");
    }
  } while (1); // Infinite loop until passwords match

  CLEAN_SCREEN();
  printf("Review User Data:\n");
  printf("Full Name: %s\n", user->fullname);
  printf("Address: %s\n", user->address);
  printf("Contact: %s\n", user->number);
  printf("Email: %s\n", user->email);
}

void registerNewUsers(void) 
{
  size_t highestRecordedNumber = loadHighestRecordedNumber(); // Load the highest recorded number

  FILE *registerFile = fopen("data/registered_users.bin", "ab+");
  if (registerFile == NULL) {
    fprintf(stderr, "Error while opening file: %s", strerror(errno));
    exit(1);
  }

  char choice;
  struct Users newUser[MAX_USERS];
  size_t num_Users = highestRecordedNumber; // Start from the highest recorded number

  do {
    CLEAN_SCREEN();
    enterUserData(&newUser[num_Users]);

    printf("\nIs the data correct? (y/n): ");
    scanf(" %c", &choice);
    flushInputBuffer();

    if (choice == 'n' || choice == 'N') {
      do {
        CLEAN_SCREEN();
        enterUserData(&newUser[num_Users]);

        printf("\nIs the re-entered data correct? (y/n): ");
        scanf(" %c", &choice);
        flushInputBuffer();
      } while (choice != 'y' && choice != 'Y'); // Repeat until data is correct
    }

    if (fwrite(&newUser[num_Users], sizeof(struct Users), 1, registerFile) == 1)
      fprintf(stdout, "\nUser data has been registered successfully");
    else
      fprintf(stderr, "\nError! while writing user data into a file : %s", strerror(errno));

    printf("\nDo you want to register another account? (y/n): ");
    scanf(" %c", &choice);
    flushInputBuffer();

    num_Users++; // Increment the number of registered users

  } while ((choice == 'y' || choice == 'Y') && num_Users < MAX_USERS);

  // Save the new highest recorded number
  saveHighestRecordedNumber(num_Users);

  fclose(registerFile);
  printf("Total users registered: %zu\n", num_Users);
}

int main(void)
{
  registerNewUsers();
  return 0;
}
