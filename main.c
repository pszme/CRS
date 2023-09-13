/* See LICENSE file for copyright and license details.
 * Program:
 *     Car Rental System (CRS)
 *
 * Authors:
 *     Furgeli Sherpa (furgelisherpa.github.io)
 *     Rohan Shilpakar (shilpakarrohan333@gmail.com)
 *     Rudra Pulami Magar (pulamirudra16@gmail.com)
 *
 * Description:
 *     The Car Rental System (CRS) is a software application that allows users to rent cars.
 *     This program manages user data, rental records, and provides a user-friendly interface for car rental services.
 *     Features:
 *     - User registration and data management
 *     - Car rental booking and tracking
 *     - Rental history and billing
 *
 * The default username for administrator
 * username: admin
 * password: admin
 *
 * (Check line 49 & 50 for changing username and password)
 *
 * Dependencies:
 *     It only requires gcc (compiler)
 *
 * Usage:
 *     To use this program, simply compile with any c compiler
 *     - gcc main.c -o car-rental-system
 *     - ./car-rental-system
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

/* Specially required for getch() (console input) */
#include <termios.h>
#include <unistd.h>

#define CLEAN_SCREEN() (printf("\033c")) /* Macro to clear the screen (for console-based UI). */

#define MAX_USERS 100 /* Maximum number of users that can be registered in the system. */
#define MAX_CAR_MODELS 100 /* Maximum number of car models that can be stored in the system. */

/* Admin User's default username and password */
const char admin_user[] = "admin";
const char admin_password[] = "admin";

/* Registered users data base file */
const char user_database[] = "data/registered_users.bin";
/* Number of recorded users within a user database */
const char current_num_of_user[] = "data/highest_recorded_number.txt";
/* Available cars data base file*/
const char car_database[] = "data/car_database.db";
/* Rental log file user for both admin and noral users*/
const char rental_records[] = "data/rental_records.bin";

struct CarModel {
  char model_name[50];
  char company[50];
  size_t year;
  double rental_rate;
  size_t passenger_capacity;
  double fuel_efficiency;
  char color[20];
  bool available_status;
};

struct Users {
  char fullname[20];
  char address[20];
  char number[11];
  char email[20];
  char username[20];
  char password[20];
};

struct Rental {
  struct CarModel selectedCar;
  struct Users rentingUser;
  char pickupDate[11];
  char returnDate[11];
  double totalCost;
  int selectedCarIndex;
  char rentalID[20];
  char time[20];
};

int checkIfFileIsEmpty(const char *filename);
void flushInputBuffer(void);
void getInput(char *str, size_t size);
char getch(void);
size_t loadHighestRecordedNumber(void);
void saveHighestRecordedNumber(size_t highestNumber);
void getPasswordInput(char *str, size_t size);
void showUserRentals(const char *username);
char *generateUniqueRentalID(const char *prefix);
void addCar(void);
void viewUsers(void);
void updateUser(char *usernameToFind);
void removeUserByUsername(const char *usernameToRemove);
void viewCars(void);
void updateCar(const char *modelToFind);
void removeCarModelByName(void);
void enterUserData(struct Users *user);
void registerNewUsers(void);
void adminDashboard(void);
int calculateRentalDays(const char *pickupDate, const char *returnDate);
void rentCar(struct Users *user);
void adminLogin(void);
void userDashboardMenu(struct Users *user);
void displayMainMenu(void);
void userLogin(struct Users *loggedInUser);

/* Main function */
int main(void)
{
  int choice;

  do {
    CLEAN_SCREEN();
    displayMainMenu();
    printf("\nEnter your choice: ");
    scanf("%d", &choice);
    flushInputBuffer();

    switch (choice) {
    case 1:
      registerNewUsers();
      break;
    case 2: {
      struct Users loggedInUser;
      userLogin(&loggedInUser);
      break;
    }
    case 3:
      adminLogin();
      break;
    case 4:
      printf("Exiting the Car Rental System. Goodbye!\n");
      exit(0);
    default:
      printf("Invalid choice. Please try again.\n");
    }
  } while (1);

  return 0;
}

int checkIfFileIsEmpty(const char *filename)
{
  FILE *file = fopen(filename, "rb");
  if (file == NULL) {
    fprintf(stderr, "Error, while opening a file %s", filename);
    return -1;
  }

  /* Check if the file is empty */
  fseek(file, 0, SEEK_END); /* Move the file pointer to the end of the file */
  long file_size = ftell(file); /* Get the current position, which is the file size */
  fseek(file, 0, SEEK_SET); /* Reset the file pointer to the beginning */

  fclose(file);
  return ((file_size == 0) ? 1 : 0);
}

/* Flushes the input buffer by reading characters until a newline or EOF is encountered. */
void flushInputBuffer(void)
{
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}

/* Get a string input and remove a '\n' taken by fgets  */
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

/**
 * Custom implementation of getch() for Unix-based systems.
 * Allows reading a single character from the terminal without buffering.
 * Returns the character read.
 *
 * Source: https://stackoverflow.com/questions/7469139/what-is-the-equivalent-to-getch-getche-in-linux
 */
char getch(void)
{
  char buf = 0;
  struct termios old = {0};
  fflush(stdout);
  if(tcgetattr(0, &old) < 0)
    perror("tcsetattr()");
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if(tcsetattr(0, TCSANOW, &old) < 0)
    perror("tcsetattr ICANON");
  if(read(0, &buf, 1) < 0)
    perror("read()");
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if(tcsetattr(0, TCSADRAIN, &old) < 0)
    perror("tcsetattr ~ICANON");
  return buf;
}

/**
 * Load the highest recorded number from a file.
 * Returns the highest number found in the file, or 0 if the file doesn't exist or an error occurs.
 */
size_t loadHighestRecordedNumber() 
{
  size_t highestNumber = 0;
  FILE *file = fopen(current_num_of_user, "r");
  if (file != NULL) {
    if (fscanf(file, "%zu", &highestNumber) != 1) {
      /* Handle fscanf error if needed */
      highestNumber = 0; /* Set to 0 if fscanf fails */
    }
    fclose(file);
  }
  return highestNumber;
}

/**
 * Save the highest recorded number to a file.
 * @param highestNumber The highest number to be saved.
 */
void saveHighestRecordedNumber(size_t highestNumber) 
{
  FILE *file = fopen(current_num_of_user, "w");
  if (file != NULL) {
    fprintf(file, "%zu", highestNumber);
    fclose(file);
  }
}

void getPasswordInput(char *str, size_t size)
{
  char password[size];
  size_t i = 0;
  char ch;

  while (i < size - 1) {
    ch = getch();
     /* Handle Enter key to terminate input */
    if (ch == '\n' || ch == '\r') {
      break;
    }
     /* Handle backspace to erase characters */
    if (ch == 127 || ch == '\b') {
      if (i > 0) {
        putchar('\b');
        putchar(' ');
        putchar('\b');
        i--;
      }
    } else {
      password[i] = ch;
      putchar('*'); /*  Print asterisks for each character */
      i++;
    }
  }
  password[i] = '\0'; /* Null-terminate the password */
  strcpy(str, password);
}

void showUserRentals(const char *username)
{
  FILE *file = fopen(rental_records, "rb");
  if (file == NULL) {
    fprintf(stderr, "Error opening the file : %s\n", strerror(errno));
    return;
  }

  if (checkIfFileIsEmpty(rental_records) == -1 || checkIfFileIsEmpty(rental_records) == 1) {
    fprintf(stderr, "There is no renting transactions made yet\n");
  } else {
    struct Rental record;
    printf("%-25s%-15s%-15s%-15s%-12s%-10s%-15s%-15s%-10s\n",
           "Time", "Renta_ID", "Username", "Model Name", "Company", "Color",
           "Pickup Date", "Return Date", "Total Cost");

    while (fread(&record, sizeof(struct Rental), 1, file) == 1) {
      if (username == NULL ||
          strcmp(record.rentingUser.username, username) == 0) {
        printf("%-25s%-15s%-15s%-15s%-12s%-10s%-15s%-15s%-10.2lf\n",
               record.time, record.rentalID, record.rentingUser.username,
               record.selectedCar.model_name, record.selectedCar.company,
               record.selectedCar.color, record.pickupDate, record.returnDate,
               record.totalCost);
      }
    }
  }
  fclose(file);
}

char *generateUniqueRentalID(const char *prefix)
{
    /* Initialize random number generator */
    srand(time(NULL));
    /* Generate a random 5-digit ID */
    int uniqueID;
    do {
        uniqueID = rand() % 100000; /* Generates a random number between 0 and 99999 */
    } while (uniqueID < 10000); /* Ensure it's a 5-digit number */

    char *rentlID = (char *) malloc(10*sizeof(char));
    snprintf(rentlID, 10, "%s%05d", prefix, uniqueID);
    return rentlID;
}

/**
 * Add a new car to the car database.
 */
void addCar(void)
{
  struct CarModel car;

  flushInputBuffer();
  printf("Enter Car Model Name: ");
  scanf("%s", car.model_name);
  flushInputBuffer();
  printf("Enter Car Company: ");
  scanf("%s", car.company);
  flushInputBuffer();
  printf("Enter Year of Manufacture: ");
  scanf("%zu", &car.year);
  flushInputBuffer();
  printf("Enter Rental Rate per Day: ");
  scanf("%lf", &car.rental_rate);
  flushInputBuffer();
  printf("Enter Passenger Capacity: ");
  scanf("%zu", &car.passenger_capacity);
  flushInputBuffer();
  printf("Enter Fuel Efficiency (MPG): ");
  scanf("%lf", &car.fuel_efficiency);
  flushInputBuffer();
  printf("Enter Car Color: ");
  scanf("%s", car.color);

  /* Assuming the car is available initially */
  car.available_status = true; 

  /* Open a car database */
  FILE *file = fopen(car_database, "ab");
  if (file == NULL) {
    fprintf(stderr, "Error opening the file for writing: %s\n", strerror(errno));
    return;
  }
  /* Register and save a new car data into a car database */
  if (fwrite(&car, sizeof(struct CarModel), 1, file) != 1) {
    fprintf(stderr, "Error writing to file: %s\n", strerror(errno));
    fclose(file);
    return;
  }
  /* Close a database */
  fclose(file);
  printf("Car added successfully.\n");
}

/**
 * View user information stored in a binary file and display it in a tabular format.
 * @param filename - The name of the binary file containing user data.
 */
void viewUsers(void)
{
  /* Open a user database file*/
  FILE *file = fopen(user_database, "rb");
  if (file == NULL) {
    fprintf(stderr, "Error opening the file for reading: %s\n", strerror(errno));
    return;
  }

  if (checkIfFileIsEmpty(user_database) == -1 || checkIfFileIsEmpty(user_database) == 1) {
    fprintf(stderr, "Users are not registered yet\n");
  }
  else {
    struct Users user;
    printf("╔══════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                               User information                                               ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ %-19s%-19s%-18s%-19s%-21s%-12s ║\n",
           "Full Name", "Address", "Phone Number", "Email", "Username", "Password");
    printf("╠══════════════════════════════════════════════════════════════════════════════════════════════════════════════╣\n");
    /* Loop through user records and display them */
    while (fread(&user, sizeof(struct Users), 1, file) == 1) {
      if (strlen(user.fullname) > 0 || strlen(user.address) > 0 || strlen(user.number) > 0 || strlen(user.email) > 0 || strlen(user.username) > 0 || strlen(user.password) > 0) {
        printf("║ %-19s%-19s%-18s%-19s%-21s%-12s ║\n",
               user.fullname, user.address, user.number, user.email, user.username, user.password);
      }
    }
    printf("╚══════════════════════════════════════════════════════════════════════════════════════════════════════════════╝\n");
    /* Close a database */
    if (fclose(file) != 0) {
      fprintf(stderr, "Error closing the file: %s\n", strerror(errno));
    }
  }
}

/* Update a user data if available over the database */
void updateUser(char *usernameToFind)
{
  FILE *file = fopen(user_database, "rb+");
  if (file == NULL) {
    printf("\nError opening the file for reading and writing.\n");
    return;
  }

  struct Users user;

  bool userFound = false;
  while (fread(&user, sizeof(struct Users), 1, file) == 1) {
    if (strcmp(user.username, usernameToFind) == 0) {
      userFound = true;
      break;
    }
  }

  if (!userFound) {
    printf("User '%s' not found in the file.\n", usernameToFind);
    fclose(file);
    return;
  }

  int choice;
  printf("Select the field to update:\n");
  printf("1. Full Name\n");
  printf("2. Address\n");
  printf("3. Phone Number\n");
  printf("4. Email\n");
  printf("5. Username\n");
  printf("6. Password\n");
  printf("\nEnter your choice: ");
  scanf("%d", &choice);

  switch (choice) {
    case 1:
      flushInputBuffer();
      printf("Enter New Full Name: ");
      getInput(user.fullname, sizeof(user.fullname));
      break;
    case 2:
      flushInputBuffer();
      printf("Enter New Address: ");
      scanf("%s", user.address);
      break;
    case 3:
      flushInputBuffer();
      printf("Enter New Number: ");
      scanf("%s", user.number);
      break;
    case 4:
      flushInputBuffer();
      printf("Enter New Email: ");
      scanf("%s", user.email);
      break;
    case 5:
      flushInputBuffer();
      printf("Enter New Username: ");
      scanf("%s", user.username);
      break;
    case 6:
      flushInputBuffer();
      printf("Enter New Password: ");
      scanf("%s", user.password);
      break;
    default:
      printf("\nInvalid choice!");
      break;
  }
  flushInputBuffer();

  printf("Full Name: %s\n", user.fullname);
  printf("Address: %s\n", user.address);
  printf("Contact Number: %s\n", user.number);
  printf("Email: %s\n", user.email);


   /* Move back to the start of the record and write the updated data */
  fseek(file, -sizeof(struct Users), SEEK_CUR);
  if(fwrite(&user, sizeof(struct Users), 1, file) != 1) {
    fprintf(stderr, "Error, while writing into a file : %s", strerror(errno));
  } else {
    printf("\nUser '%s' updated successfully.\n", usernameToFind);
  }
  fclose(file);
}

/**
 * Remove a user from the user database by username.
 */
void removeUserByUsername(const char *usernameToRemove)
{
  /* Open a user database */
  FILE *file = fopen(user_database, "rb");
  if (file == NULL) {
    fprintf(stderr, "Error opening the file for reading: %s\n", strerror(errno));
    return;
  }

  struct Users user;

  /* Open a newfile for writing a userdata temporaryly */
  FILE *tempFile = fopen("data/temp.dat", "wb");
  if (tempFile == NULL) {
    fprintf(stderr, "Error creating temporary file: %s\n", strerror(errno));
    fclose(file);
    return;
  }

  bool userFound = false;
  /* If user found change its status to true*/
  while (fread(&user, sizeof(struct Users), 1, file) == 1) {
    if (strcmp(user.username, usernameToRemove) == 0) {
      userFound = true;
      continue; /* Skip writing this user to the temporary file */
    }
    /* Write the structure to the temporary file for all other users */
    if (fwrite(&user, sizeof(struct Users), 1, tempFile) != 1) {
      fprintf(stderr, "Error writing data to the file: %s\n", strerror(errno));
      return;
    }
  }

  /* Close user databse and temporary file */
  fclose(file);
  fclose(tempFile);

  /* Show errors */
  if (!userFound) {
    fprintf(stderr, "User '%s' not found in the file.\n", usernameToRemove);
    remove("temp.dat"); /* Delete the temporary file */
    return;
  }
  if (remove(user_database) != 0) {
    fprintf(stderr, "Error deleting the original file: %s\n", strerror(errno));
    remove("temp.dat"); /* Delete the temporary file */
    return;
  }
  if (rename("temp.dat", user_database) != 0) {
    fprintf(stderr, "Error renaming the temporary file: %s\n", strerror(errno));
    return;
  }
  printf("User '%s' removed successfully.\n", usernameToRemove);
}

/* Function to view a cars available in a database */
void viewCars(void)
{
  /* Open the car database */
  FILE *file = fopen(car_database, "rb");
  if (file == NULL) {
    fprintf(stderr, "Error opening the file for reading: %s\n", strerror(errno));
    return;
  }

  if (checkIfFileIsEmpty(car_database) == -1 || checkIfFileIsEmpty(car_database) == 1 ) {
    fprintf(stderr, "Cars are not available at the moment\nMight be went to garage or service center\nPlease visit later!\n");
  }
  else {
    struct CarModel car;
    printf("\n╔══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                     Available Car Models                                                     ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ %-15s%-15s%-12s%-19s%-20s%-12s%-17s%-14s ║\n",
           "Model Name", "Company", "Year", "Passenger Cap.", "Fuel Efficiency", "Color", "Rate (NPR)", "Status");
    printf("╠══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣\n");
    /* Read and display cars with a availability status */
    while (fread(&car, sizeof(struct CarModel), 1, file) == 1) {
      printf("║ %-15s%-15s%-12zu%-19zu%-20.2lf%-12s%-16.2lf %-15s║\n",
             car.model_name,
             car.company,
             car.year,
             car.passenger_capacity,
             car.fuel_efficiency,
             car.color,
             car.rental_rate,
             car.available_status ? "Available" : "Not Available");
    }
    printf("╚══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝");
    /* Close a database */
    if (fclose(file) != 0) {
      fprintf(stderr, "Error closing the file: %s\n", strerror(errno));
    }
  }
}

/**
 * Update car details in the car database by car model name.
 */
void updateCar(const char *modelToFind)
{
  int car_status;

  /* Open a car database */ 
  FILE *file = fopen(car_database, "rb+");
  if (file == NULL) {
    fprintf(stderr, "Error opening the file for reading and writing: %s\n", strerror(errno));
    return;
  }

  struct CarModel car;

  bool carFound = false;
  /* Change the carFound status to true if available */
  while (fread(&car, sizeof(struct CarModel), 1, file) == 1) {
    if (strcmp(car.model_name, modelToFind) == 0) {
      carFound = true;
      break;
    }
  }
  /* Show error if car not found */
  if (!carFound) {
    fprintf(stderr, "Car '%s' not found in the file.\n", modelToFind);
    fclose(file);
    return;
  }

  int choice;
  printf("Select the field to update:\n");
  printf("1. Model Name\n");
  printf("2. Company\n");
  printf("3. Year\n");
  printf("4. Rental Rate\n");
  printf("5. Passenger Capacity\n");
  printf("6. Fuel Efficiency\n");
  printf("7. Color\n");
  printf("8. Available Status\n");
  printf("\nEnter your choice: ");
  scanf("%d", &choice);
  flushInputBuffer();

  switch (choice) {
    case 1:
      printf("Enter Car Model Name: ");
      getInput(car.model_name, sizeof(car.model_name));
      flushInputBuffer();
      break;
    case 2:
      printf("Enter Car Company: ");
      getInput(car.company, sizeof(car.company));
      flushInputBuffer();
      break;
    case 3:
      printf("Enter Year of Manufacture: ");
      scanf("%zu", &car.year);
      flushInputBuffer();
      break;
    case 4:
      printf("Enter Rental Rate per Day: ");
      scanf("%lf", &car.rental_rate);
      flushInputBuffer();
      break;
    case 5:
      printf("Enter Passenger Capacity: ");
      scanf("%zu", &car.passenger_capacity);
      flushInputBuffer();
      break;
    case 6:
      printf("Enter Fuel Efficiency (MPG): ");
      scanf("%lf", &car.fuel_efficiency);
      flushInputBuffer();
      break;
    case 7:
      printf("Enter Car Color: ");
      getInput(car.color, sizeof(car.color));
      flushInputBuffer();
      break;
    case 8:
      printf("Enter Available Staus (1 for available / 0 for not available): ");
      scanf("%d", &car_status);
      flushInputBuffer();
      if (car_status)
        car.available_status = true;
      break;
    default:
      printf("\nInvalid choice. No fields updated.\n");
      break;
  }
  /* Move back to the start of the record and write the updated data */
  fseek(file, -sizeof(struct CarModel), SEEK_CUR);
  /* Show error if file didn't written successfully */
  if (fwrite(&car, sizeof(struct CarModel), 1, file) != 1) {
    fprintf(stderr, "Error writing data to the file: %s\n", strerror(errno));
  }
  printf("\nCar '%s' updated successfully.\n", modelToFind);
  /* Close a car database */
  fclose(file);
}

/**
 * Remove a car model by selecting its index.
 */
void removeCarModelByName(void)
{
  /* Open a car database */
  FILE *file = fopen(car_database, "rb");
  if (file == NULL) {
    fprintf(stderr, "Error opening the file for reading: %s\n", strerror(errno));
    return;
  }

  int index = 0;
  printf("╔══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╗\n");
  printf("║                                        Available Car Models (Select a model to remove)                                               ║\n");
  printf("╠══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣\n");
  printf("║ %-8s%-15s%-15s%-12s%-19s%-20s%-12s%-17s%-14s ║\n",
         "Index", "Model Name", "Company", "Year", "Passenger Cap.", "Fuel Efficiency", "Color", "Rate (NPR)", "Status");
  printf("╠══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╣\n");

  struct CarModel car;
  while (fread(&car, sizeof(struct CarModel), 1, file) == 1) {
    printf("║ %-8d%-15s%-15s%-12zu%-19zu%-20.2lf%-12s%-16.2lf %-15s║\n",
           index,
           car.model_name,
           car.company,
           car.year,
           car.passenger_capacity,
           car.fuel_efficiency,
           car.color,
           car.rental_rate,
           car.available_status ? "Available" : "Not Available");
    index++;
  }
  printf("╚══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════╝\n");
  fclose(file);

  /* Ask the user for the index of the model to remove */
  printf("Enter the index of the model you want to remove : ");
  int selectedIndex;
  if (scanf("%d", &selectedIndex) != 1) {
    fprintf(stderr, "Invalid input.\n");
    return;
  }
  flushInputBuffer();

  /* Reopen the file in read mode and a temporary file for writing */
  file = fopen(car_database, "rb");
  if (file == NULL) {
    fprintf(stderr, "Error opening the file for reading: %s\n", strerror(errno));
    return;
  }

  FILE *tempFile = fopen("temp.dat", "wb");
  if (tempFile == NULL) {
    fprintf(stderr, "Error creating temporary file: %s\n", strerror(errno));
    fclose(file);
    return;
  }

  index = 0;
  while (fread(&car, sizeof(struct CarModel), 1, file) == 1) {
    if (index != selectedIndex) {
      /* Write the structure to the temporary file if it's not the selected one */
      fwrite(&car, sizeof(struct CarModel), 1, tempFile);
    }
    index++;
  }

  /* Close a open files */
  fclose(file);
  fclose(tempFile);

  remove(car_database);     /* Delete the original file */
  if (rename("data/temp.dat", car_database) != 0) {
    fprintf(stderr, "Error renaming the temporary file: %s\n", strerror(errno));
    return;
  }
  printf("Model data removed successfully.\n");
}

/**
 * Collects user information, validates it, and sets up a username and password.
 * @param user A pointer to the Users struct to store user information.
 */
void enterUserData(struct Users *user)
{
  char choice;

  printf("Please provide the following information:\n");
  printf("Full Name: ");
  getInput(user->fullname, sizeof(user->fullname));
  flushInputBuffer();
  printf("Address: ");
  getInput(user->address, sizeof(user->address));
  flushInputBuffer();
  printf("Contact Number: ");
  getInput(user->number, sizeof(user->number));
  flushInputBuffer();
  printf("Email Address: ");
  getInput(user->email, sizeof(user->email));
  flushInputBuffer();

  while (1) {
    printf("Review Your Information:\n");
    printf("Full Name: %s\n", user->fullname);
    printf("Address: %s\n", user->address);
    printf("Contact Number: %s\n", user->number);
    printf("Email : %s\n", user->email);

    printf("Choose a field to change\n");
    printf("(F)ull Name, (A)ddress, (N)umber, (E)mail, (O)kay : ");
    scanf("%c", &choice);
    flushInputBuffer();

    switch (choice) {
    case 'F': {
      printf("Full Name: ");
      getInput(user->fullname, sizeof(user->fullname));
      flushInputBuffer();
    } break;
    case 'A': {
      printf("Address: ");
      getInput(user->address, sizeof(user->address));
      flushInputBuffer();
    } break;
    case 'N': {
      printf("Contact Number: ");
      getInput(user->number, sizeof(user->number));
      flushInputBuffer();
    } break;
    case 'E': {
      printf("Email Address: ");
      getInput(user->email, sizeof(user->email));
      flushInputBuffer();
    } break;
    case 'O':
      goto create_username_and_password;
    default:
      printf("Invalid!");
      break;
    }
  }

create_username_and_password : {
  FILE *file = fopen(user_database, "rb");
  if (file == NULL) {
    fprintf(stderr, "Error while opening the user data file: %s\n",
            strerror(errno));
  }

  struct Users recordedUser;
  while (fread(&recordedUser, sizeof(struct Users), 1, file) == 1) {
    if (strcmp(user->number, recordedUser.number) == 0) {
      fprintf(
          stdout,
          "\n%s, It seems the contact number you have entered is already in use.\n \
Please use different contact number\n",
          user->fullname);

      printf("Re-enter Contact Number : ");
      getInput(user->number, sizeof(user->number));
      flushInputBuffer();
    } else {
      break;
    }
    fclose(file);
  }

  printf("\nThank you, %s, for providing your information.\n", user->fullname);
  printf("You can now set up your username and password for further access.\n");

  file = fopen(user_database, "rb");
  if (file == NULL) {
    fprintf(stderr, "Error while opening the user data file: %s\n",
            strerror(errno));
  }

  fseek(file, -sizeof(struct Users),
        SEEK_CUR); /* Move the file pointer back to update the record */

  printf("Enter New Username: ");
  scanf("%s", user->username);
  flushInputBuffer();

  while (fread(&recordedUser, sizeof(struct Users), 1, file) == 1) {
    if (strcmp(user->username, recordedUser.username) == 0) {
      fprintf(
          stdout,
          "\nThe user with this \"%s\" username, seems already registered!\n\
Please choose different user name\n",
          user->username);
      printf("Enter New Username: ");
      scanf("%s", user->username);
      flushInputBuffer();
    } else
      break;
  }
  fclose(file);

  char passwordVerification[20];
  do {
    printf("Enter New Password: ");
    getPasswordInput(user->password, sizeof(user->password));
    printf("\nRetype the password for verification: ");
    getPasswordInput(passwordVerification, sizeof(passwordVerification));

    /* Check if the entered passwords match */
    if (strcmp(user->password, passwordVerification) == 0) {
      break; /* Passwords match, exit the verification loop */
    } else {
      printf("\nPasswords do not match. Please try again.\n");
    }
  } while (1); /* Infinite loop until passwords match */
}
}

/**
 * Registers new users by collecting their information and saving it to the user
 * database file.
 */
void registerNewUsers(void)
{
  char choice[4];
  struct Users newUser[MAX_USERS];
  size_t numUsers =
      loadHighestRecordedNumber(); /* Start from the highest recorded number */

  /* Open the user database file for reading and appending */
  FILE *userDatabaseFile = fopen(user_database, "ab+");
  if (userDatabaseFile == NULL) {
    fprintf(stderr, "Error while opening the user data file: %s\n",
            strerror(errno));
    exit(1); /* Exit the program on error */
  }

  CLEAN_SCREEN();
  enterUserData(&newUser[numUsers]);

  /* Write user data to the user database file */
  if (fwrite(&newUser[numUsers], sizeof(struct Users), 1, userDatabaseFile) ==
      1) {
    printf("\nUser data has been registered successfully.\n");
    fclose(userDatabaseFile);
  } else {
    fprintf(stderr, "\nError while writing user data into a file: %s\n",
            strerror(errno));
    fclose(userDatabaseFile);
  }

  printf("\nPress any key to return to the menu!\n");
  getch();

  numUsers++; /* Increment the number of registered users */

  /* Save the new highest recorded number */
  saveHighestRecordedNumber(numUsers);
}

/**
 * Displays the admin dashboard with various options to manage cars, users, and
 * view rental logs.
 */
void adminDashboard(void)
{
  int choice;
  char car_model[20];
  char username[20];

  do {
    printf("\nAdmin Dashboard");
    printf("\n1. View Cars");
    printf("\n2. Manage Cars");
    printf("\n3. View Users");
    printf("\n4. Manage Users");
    printf("\n5. Rental Log");
    printf("\n6. Exit");

    printf("\nChoose the option : ");
    scanf("%d", &choice);
    flushInputBuffer();

    switch (choice) {
    case 1:
      viewCars();
      break;
    case 2: {
      do {
        viewCars();
        printf("\n1. Update Cars");
        printf("\n2. Remove Cars");
        printf("\n3. Add Cars");
        printf("\n4. Return to main menu");
        printf("\nChoose the option : ");
        scanf("%d", &choice);
        flushInputBuffer();
        switch (choice) {
        case 1:
          printf("\nEnter the Model name : ");
          scanf("%s", car_model);
          flushInputBuffer();
          updateCar(car_model);
          break;
        case 2:
          removeCarModelByName();
          break;
        case 3:
          addCar();
          break;
        case 4:
          break;
        default:
          printf("\nInvalid choice!");
          break;
        }
      } while (choice != 4);
    } break;
    case 3:
      viewUsers();
      break;
    case 4: {
      do {
        viewUsers();
        printf("\n1. Update Users");
        printf("\n2. Remove Users");
        printf("\n3. Add Users");
        printf("\n4. Return to main menu");
        printf("\nChoose the option : ");
        scanf("%d", &choice);
        flushInputBuffer();
        switch (choice) {
        case 1:
          printf("\nEnter the username to update : ");
          scanf("%s", username);
          flushInputBuffer();
          updateUser(username);
          break;
        case 2:
          printf("\nEnter the username to remove : ");
          scanf("%s", username);
          flushInputBuffer();
          removeUserByUsername(username);
          break;
        case 3:
          registerNewUsers();
          break;
        case 4:
          break;
        default:
          printf("\nInvalid choice!");
          break;
        }
      } while (choice != 4);
    } break;
    case 5: {
      char user_log_menu_choice[4];
      char log_username[20];

      printf("Do you want to see a specific users log ? (yes/no) : ");
      scanf("%3s", user_log_menu_choice);

      if (strcmp(user_log_menu_choice, "yes") == 0 ||
          strcmp(user_log_menu_choice, "Yes") == 0 ||
          strcmp(user_log_menu_choice, "YES") == 0) {
        printf("Enter the specific user's username : ");
        scanf("%s", log_username);
        showUserRentals(log_username);
      } else
        showUserRentals(NULL);
    } break;
    case 6:
      break;
    default:
      printf("\nInvalid choice. Please try again.");
    }
  } while (choice != 6);
}

int calculateRentalDays(const char *pickupDate, const char *returnDate)
{
  /* Ensure that the date strings are in the correct format (YYYY-MM-DD) */
  if (strlen(pickupDate) != 10 || strlen(returnDate) != 10 ||
      pickupDate[4] != '-' || returnDate[4] != '-' || pickupDate[7] != '-' ||
      returnDate[7] != '-') {
    printf("Invalid date format. Please use YYYY-MM-DD format.\n");
    return -1; /* Error: Invalid date format */
  }

  /* Extract year, month, and day components from the date strings */
  int pickupYear, pickupMonth, pickupDay;
  int returnYear, returnMonth, returnDay;
  if (sscanf(pickupDate, "%d-%d-%d", &pickupYear, &pickupMonth, &pickupDay) !=
          3 ||
      sscanf(returnDate, "%d-%d-%d", &returnYear, &returnMonth, &returnDay) !=
          3) {
    printf("Invalid date format. Please use YYYY-MM-DD format.\n");
    return -1; /* Error: Invalid date components */
  }

  /* Calculate the difference in days between the pickup and return dates */
  struct tm pickupTm = {0};
  struct tm returnTm = {0};
  pickupTm.tm_year = pickupYear - 1900;
  pickupTm.tm_mon = pickupMonth - 1;
  pickupTm.tm_mday = pickupDay;

  returnTm.tm_year = returnYear - 1900;
  returnTm.tm_mon = returnMonth - 1;
  returnTm.tm_mday = returnDay;

  time_t pickupTime = mktime(&pickupTm);
  time_t returnTime = mktime(&returnTm);

  if (pickupTime == -1 || returnTime == -1) {
    printf("Error in date conversion.\n");
    return -1; /* Error: Date conversion failed */
  }

  double seconds = difftime(returnTime, pickupTime);
  if (seconds < 0) {
    printf("Return date cannot be earlier than pickup date.\n");
    return -1; /* Error: Invalid date range */
  }

  int days = (int)(seconds / 86400); /* 86400 seconds in a day */
  return days;
}

void rentCar(struct Users *user)
{
  char choice[4];
  int numAvailableCars = 0;
  bool rentalCompleted = false;

  /* Open a Car database */
  FILE *file = fopen(car_database, "rb+");
  if (file == NULL) {
    fprintf(stderr, "Error opening file %s: %s\n", car_database,
            strerror(errno));
    return;
  }

  struct Rental rental;
  struct CarModel availableCarModels[MAX_CAR_MODELS];

  /* Read each and every cars details and increement the numAvailableCars if
   * available status is true */
  while (fread(&availableCarModels[numAvailableCars], sizeof(struct CarModel),
               1, file)) {
    if (availableCarModels[numAvailableCars].available_status)
      numAvailableCars++;
  }
  /* Close a database */
  fclose(file);

  printf("=== Rent a Car ===\n");
  /* Shows errors if numAvailableCars is ZERO */
  if (numAvailableCars == 0) {
    printf("Sorry, there are no cars available for rent at the moment.\n");
    return;
  } else {
    printf("Available Car Models:\n");
    printf("%-5s %-15s %-12s %-10s %-11s\n", "Index", "Model Name", "Company",
           "Color", "Rate (NPR)");

    for (int i = 0; i < numAvailableCars; i++) {
      printf("%-5d %-15s %-12s %-10s %-11.2lf\n", i + 1,
             availableCarModels[i].model_name, availableCarModels[i].company,
             availableCarModels[i].color, availableCarModels[i].rental_rate);
    }

    printf("\nEnter the index of the car you want to rent (0 to cancel): ");
    scanf("%d", &rental.selectedCarIndex);
    flushInputBuffer();

    if (rental.selectedCarIndex == 0) {
      printf("Rental canceled. Returning to the User Dashboard...\n");
      return;
    } else if (rental.selectedCarIndex < 1 ||
               rental.selectedCarIndex > numAvailableCars) {
      printf("Invalid car index. Please try again.\n");
      return;
    }
  }

  /* Valid car selection */
  rental.selectedCar = availableCarModels[rental.selectedCarIndex - 1];

  /* Gather rental dates */
  printf("Enter Pickup Date (YYYY-MM-DD): ");
  scanf("%s", rental.pickupDate);
  flushInputBuffer();
  printf("Enter Return Date (YYYY-MM-DD): ");
  scanf("%s", rental.returnDate);

  /* Calculate total cost (simple example: rate * number of days) */
  /* You can implement more complex pricing logic as needed. */
  rental.totalCost = rental.selectedCar.rental_rate *
                     calculateRentalDays(rental.pickupDate, rental.returnDate);

  /* Generate a unique rental-ID for each transaction */
  char *uniqueID = generateUniqueRentalID("R");
  strncpy(rental.rentalID, uniqueID, sizeof(rental.rentalID));
  free(uniqueID);

  /* Display rental summary */
  printf("\nRental Summary:\n");
  printf("Rental ID: %s\n", rental.rentalID);
  printf("Model: %s\n", rental.selectedCar.model_name);
  printf("Color: %s\n", rental.selectedCar.color);
  printf("Company: %s\n", rental.selectedCar.company);
  printf("Rate (NPR): %.2lf per day\n", rental.selectedCar.rental_rate);
  printf("Pickup Date: %s\n", rental.pickupDate);
  printf("Return Date: %s\n", rental.returnDate);
  printf("Total Cost: NRS %0.2lf\n", rental.totalCost);

  /* Confirm the rental */
  printf("Confirm rental? (yes/no): ");
  scanf("%3s", choice);
  flushInputBuffer();

  if (strcmp(choice, "yes") == 0 || strcmp(choice, "Yes") == 0 ||
      strcmp(choice, "YES") == 0) {
    rentalCompleted = true;
    printf("\nRental completed. Enjoy your ride!\n");
  }
  /* Change the availability status to false for the rented car model */
  availableCarModels[rental.selectedCarIndex - 1].available_status = false;

  /* Opening file to update the selected Car availability status */
  file = fopen(car_database, "rb+");
  if (file == NULL) {
    fprintf(stderr, "Error opening file %s: %s\n", car_database,
            strerror(errno));
  }

  struct CarModel car;
  while (fread(&car, sizeof(struct CarModel), 1, file) == 1) {
    if (strcmp(car.model_name, rental.selectedCar.model_name) == 0) {
      car.available_status = false;
      fseek(file, -sizeof(struct CarModel),
            SEEK_CUR); /* Move the file pointer back to update the record */
      fwrite(&car, sizeof(struct CarModel), 1,
             file); /* Write the updated record back to the file */
      break;        /* No need to continue searching after the update */
    }
  }
  fclose(file);

  /* Generate a current date and time and assign it with rental.time */
  time_t current_time = time(NULL);
  char *timestamp = ctime(&current_time);
  if (timestamp != NULL)
    timestamp[strlen(timestamp) - 1] = '\0';
  strncpy(rental.time, timestamp, sizeof(rental.time));

  file = fopen(rental_records, "ab+");
  if (file == NULL) {
    fprintf(stderr, "Error while opening file %s", rental_records);
  }

  strncpy(rental.rentingUser.username, user->username,
          sizeof(rental.rentingUser.username));

  if (fwrite(&rental, sizeof(struct Rental), 1, file) != 1) {
    fprintf(stderr, "Error writing to file: %s\n", strerror(errno));
    fclose(file);
  } else {
    fclose(file);
  }
  while (!rentalCompleted)
    ;
}

void adminLogin(void)
{
  char loginInput[20];
  char passwordInput[20];

  CLEAN_SCREEN();

  /* Prompt for admin username */
  printf("Enter Admin Username : ");
  getInput(loginInput, sizeof(loginInput));
  flushInputBuffer();

  /* Prompt for admin password securely */
  printf("Enter Admin Password : ");
  getPasswordInput(passwordInput, sizeof(passwordInput));

  /* Check if the entered credentials match the admin credentials */
  if (strcmp(loginInput, admin_user) == 0 &&
      strcmp(passwordInput, admin_password) == 0) {
    CLEAN_SCREEN();
    printf("\nSuccessfully logged in!\nYou are in the administration "
           "dashboard!\n");
    adminDashboard(); /* Redirect to the admin dashboard upon successful login
                       */
  } else {
    printf("\nLogin Failed!\n"); /* Display a failure message for incorrect
                                    credentials */
  }
}

void userDashboardMenu(struct Users *user)
{
  int choice;
  printf("\nWelcome to the User Dashboard, %s!\n", user->fullname);
  do {
    /* Display the user dashboard */
    printf("\n1. View Available Car Models\n");
    printf("2. Rent a Car\n");
    printf("3. View Rental History\n");
    printf("4. Account Settings\n"); /* Allow users to change their password,
                                        contact info, etc. */
    printf("5. Logout\n");

    /* Prompt for user input */
    printf("\nEnter your choice: ");
    scanf("%d", &choice);
    flushInputBuffer();

    switch (choice) {
    case 1:
      viewCars();
      break;
    case 2:
      rentCar(user);
      break;
    case 3:
      showUserRentals(user->username);
      break;
    case 4:
      updateUser(user->username);
      break;
    case 5:
      printf("Logging out from User Dashboard.\n");
      return;
    default:
      printf("Invalid choice. Please try again.\n");
    }
  } while (1); /* Infinite loop for the user menu */
}

void displayMainMenu(void)
{
  printf("=== Car Rental System ===\n");
  printf("1. User Registration\n");
  printf("2. User Login\n");
  printf("3. Admin Login\n");
  printf("4. Exit\n");
}

void userLogin(struct Users *loggedInUser)
{
  char loginInput[20];
  char passwordInput[20];
  char choice[4];

  CLEAN_SCREEN();
  printf("=== User Login ===\n");
  printf("Please enter your Username, Contact Number, or Email: ");
  getInput(loginInput, sizeof(loginInput));
  printf("Please enter your Password: ");
  getPasswordInput(passwordInput, sizeof(passwordInput));

  FILE *file = fopen("data/registered_users.bin", "rb");
  if (file == NULL) {
    printf("Error while opening user data file.\n");
    return; /* Login failed */
  }

  struct Users user;
  int found = 0;

  while (fread(&user, sizeof(struct Users), 1, file)) {
    if ((strcmp(loginInput, user.username) == 0 ||
         strcmp(loginInput, user.number) == 0 ||
         strcmp(loginInput, user.email) == 0) &&
        strcmp(passwordInput, user.password) == 0) {
      found = 1;
      *loggedInUser = user; /* Copy user data to the loggedInUser pointer */
      break;
    }
  }

  fclose(file);

  if (found) {
    /* User successfully logged in, call the user dashboard */
    printf("\nLogin successful.");
    userDashboardMenu(loggedInUser);
  } else {
    printf("\nLogin failed. Please check your credentials.\n");
    printf("If you have forgotten your password, please consult your "
           "administrator for assistance.\n");
    printf("\nDo you want to login again ? (yes/no) : ");
    scanf("%3s", choice);
    flushInputBuffer();

    if (strcmp(choice, "yes") == 0 || strcmp(choice, "Yes") == 0 ||
        strcmp(choice, "YES") == 0)
      userLogin(loggedInUser);
  }
}

