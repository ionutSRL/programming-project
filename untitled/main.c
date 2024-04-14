#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#define MAX_ACCOUNTS 10000
#define MAX_NAME_LENGTH 500
#define MAX_SURNAME_LENGTH 500
#define MAX_TITLE_LENGTH 1000
#define MAX_AUTHOR_LENGTH 1000
#define FILENAME "accounts.txt"
#define FILENAME2 "books.txt"
#define NUMBOOKS_FILE "numberofbooks.txt"
char Name[MAX_NAME_LENGTH];
char Surname[MAX_SURNAME_LENGTH];


typedef struct {
    char name[MAX_NAME_LENGTH];
    char surname[MAX_SURNAME_LENGTH];
} Account;

typedef struct {
    int id;
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    int copiesAvailable;
} Book;
Book books[MAX_ACCOUNTS];
int numBooks = 0;

Account accounts[MAX_ACCOUNTS];
int numAccounts = 0;

void loadAccountsFromFile() {
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        printf("Failed to open file %s\n", FILENAME);
        return;
    }

    while (fscanf(file, "%49[^,],%49[^\n]\n", accounts[numAccounts].name, accounts[numAccounts].surname) == 2) {
        numAccounts++;
        if (numAccounts >= MAX_ACCOUNTS) {
            printf("Max account limit reached.\n");
            break;
        }
    }

    fclose(file);
}

void saveAccountsToFile() {
    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        printf("Error opening file %s\n", FILENAME);
    }
    for (int i = 0; i < numAccounts; i++) {
        fprintf(file, "%s,%s\n", accounts[i].name, accounts[i].surname);
    }
    fclose(file);
}

int login() {


    printf("Enter name: ");
    scanf("%s", Name);
    printf("Enter surname: ");
    scanf("%s", Surname);

    for (int i = 0; i < numAccounts; i++) {
        if (strcmp(accounts[i].name, Name) == 0 && strcmp(accounts[i].surname, Surname) == 0) {
            printf("Login successful!\n");
            return 1;
        }
    }

    printf("Incorrect name or surname. Please try again.\n");
    return 0;
}

void signup() {
    char name[MAX_NAME_LENGTH];
    char surname[MAX_SURNAME_LENGTH];

    printf("Enter name: ");
    scanf("%s", name);

    printf("Enter surname: ");
    scanf("%s", surname);

    int accountExists = 0;
    for (int i = 0; i < numAccounts; i++) {
        if (strcmp(accounts[i].name, name) == 0 && strcmp(accounts[i].surname, surname) == 0) {
            printf("Name and surname already exist. Please choose another one.\n");
            accountExists = 1;
            break;
        }
    }

    if (!accountExists && numAccounts < MAX_ACCOUNTS) {
        // Create folder with the user's name and surname
        char folderName[MAX_NAME_LENGTH + MAX_SURNAME_LENGTH + 2];
        snprintf(folderName, sizeof(folderName), "%s_%s", name, surname);
        if (mkdir(folderName) != 0) {
            printf("Error creating folder for user.\n");
            return;
        }

        // Create loans.txt file inside the user's folder with initial value 0
        char loansFileName[100];
        snprintf(loansFileName, sizeof(loansFileName), "%s/loans.txt", folderName);
        FILE *loansFile = fopen(loansFileName, "w");
        if (loansFile == NULL) {
            printf("Error creating loans file.\n");
            return;
        }
        fprintf(loansFile, "0\n"); // Initial number of loans is 0
        fclose(loansFile);

        // Update accounts data
        strcpy(accounts[numAccounts].name, name);
        strcpy(accounts[numAccounts].surname, surname);
        numAccounts++;
        printf("Account created successfully!\n");
        saveAccountsToFile();
    } else if (numAccounts >= MAX_ACCOUNTS) {
        printf("Max account limit reached. Cannot create new account.\n");
    }
}

void seeLoans() {
    char folderName[MAX_NAME_LENGTH + MAX_SURNAME_LENGTH + 2];
    snprintf(folderName, sizeof(folderName), "%s_%s", Name, Surname);

    // Open the user's loan folder
    DIR *dir = opendir(folderName);
    if (dir == NULL) {
        printf("Error opening loan directory for user.\n");
        return;
    }

    // Read all loan files in the directory
    struct dirent *ent;
    int loanCount = 0;
    char loanFileName[256][256];
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "loan_", 5) == 0) {
            snprintf(loanFileName[loanCount], sizeof(loanFileName[loanCount]), "%s/%s", folderName, ent->d_name);
            loanCount++;
        }
    }
    closedir(dir);

    if (loanCount == 0) {
        printf("No loans found for this user.\n");
        return;
    }

    // Display the loans available for the user
    printf("Loans for %s %s:\n", Name, Surname);
    printf("%-5s%-40s%-25s%-25s%-20s\n", "ID", "File Name", "Title", "Author", "Copies Borrowed");
    printf("--------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < loanCount; i++) {
        FILE *loanFile = fopen(loanFileName[i], "r");
        if (loanFile == NULL) {
            printf("Error opening loan file: %s\n", strerror(errno));
            continue;
        }

        char line[100];
        int bookId, copiesBorrowed;
        char title[MAX_TITLE_LENGTH], author[MAX_AUTHOR_LENGTH];
        while (fgets(line, sizeof(line), loanFile) != NULL) {
            if (sscanf(line, "Book ID: %d", &bookId) == 1) {
                fgets(line, sizeof(line), loanFile); // Read title line
                sscanf(line, "Title: %[^\n]", title);
                fgets(line, sizeof(line), loanFile); // Read author line
                sscanf(line, "Author: %[^\n]", author);
                fgets(line, sizeof(line), loanFile); // Read copies borrowed line
                sscanf(line, "Copies Borrowed: %d", &copiesBorrowed);
                break;
            }
        }
        fclose(loanFile);

        // Print loan details with a space between ".txt" and "Author"
        char fileName[256];
        char *pos = strstr(loanFileName[i], ".txt");
        strncpy(fileName, loanFileName[i], pos - loanFileName[i] + 4);
        fileName[pos - loanFileName[i] + 4] = '\0'; // Add null terminator
        printf("%-5d%-40s%-30s%-25s%-20d\n", i + 1, fileName, title, author, copiesBorrowed);
    }
}




void saveBooksToFile() {
    FILE *file = fopen("books.txt", "w");
    if (file == NULL) {
        printf("Error opening file for writing.\n");
        return;
    }

    for (int i = 0; i < numBooks; i++) {
        fprintf(file, "%d;%s;%s;%d\n", books[i].id, books[i].title, books[i].author, books[i].copiesAvailable);
    }

    fclose(file);
}

void UpdateBooksFile(const char *title, int returnedCopies) {
    for (int i = 0; i < numBooks; ++i) {
        if (strcmp(books[i].title, title) == 0) {
            books[i].copiesAvailable += returnedCopies;
            break;
        }
    }
    saveBooksToFile(); // Assuming you have a function to save books to file
}


void returnBooks() {
    char folderName[MAX_NAME_LENGTH + MAX_SURNAME_LENGTH + 2];
    snprintf(folderName, sizeof(folderName), "%s_%s", Name, Surname);

    // Open the user's loan folder
    DIR *dir = opendir(folderName);
    if (dir == NULL) {
        printf("Error opening loan directory for user.\n");
        return;
    }

    // Read all loan files in the directory
    struct dirent *ent;
    int loanCount = 0;
    char loanFileName[256][256];
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "loan_", 5) == 0) {
            snprintf(loanFileName[loanCount], sizeof(loanFileName[loanCount]), "%s/%s", folderName, ent->d_name);
            loanCount++;
        }
    }
    closedir(dir);

    if (loanCount == 0) {
        printf("No loans found for this user.\n");
        return;
    }

    // Display the loans available for the user
    printf("Loans for %s %s:\n", Name, Surname);
    for (int i = 0; i < loanCount; i++) {
        printf("%d. %s\n", i + 1, loanFileName[i]);
    }

    // Ask the user which loan to return
    int choice;
    do {
        printf("Enter the loan number you want to return (1-%d): ", loanCount);
        scanf("%d", &choice);
    } while (choice < 1 || choice > loanCount);

    // Get the index of the chosen loan file
    int chosenLoanIndex = choice - 1;

    // Open the chosen loan file
    FILE *loanFile = fopen(loanFileName[chosenLoanIndex], "r");
    if (loanFile == NULL) {
        printf("Error opening loan file: %s\n", strerror(errno));
        return;
    }

    // Read from the loan file to determine which book(s) were borrowed
    char line[100];
    int totalReturnedCopies = 0;
    while (fgets(line, sizeof(line), loanFile) != NULL) {
        int bookId, borrowedCopies;
        char title[MAX_TITLE_LENGTH], author[MAX_AUTHOR_LENGTH];

        if (sscanf(line, "Book ID: %d", &bookId) == 1) {
            fgets(line, sizeof(line), loanFile); // Read title line
            sscanf(line, "Title: %[^\n]", title);
            fgets(line, sizeof(line), loanFile); // Read author line
            sscanf(line, "Author: %[^\n]", author);
            fgets(line, sizeof(line), loanFile); // Read copies borrowed line
            sscanf(line, "Copies Borrowed: %d", &borrowedCopies);

            // Update the books file with the returned copies
            UpdateBooksFile(title, borrowedCopies);

            totalReturnedCopies += borrowedCopies;
        }
    }

    fclose(loanFile);

    // Remove the chosen loan file since books have been returned
    if (remove(loanFileName[chosenLoanIndex]) != 0) {
        printf("Error removing loan file: %s\n", strerror(errno));
        return;
    }

    // Rename the subsequent loan files
    for (int i = chosenLoanIndex + 1; i < loanCount; i++) {
        char newLoanFileName[256];
        snprintf(newLoanFileName, sizeof(newLoanFileName), "%s/loan_%d.txt", folderName, i);
        if (rename(loanFileName[i], newLoanFileName) != 0) {
            printf("Error renaming loan file: %s\n", strerror(errno));
            return;
        }
    }

    // Update loans.txt to reflect the decreased number of loans
    char loansFileName[256];
    snprintf(loansFileName, sizeof(loansFileName), "%s/loans.txt", folderName);
    FILE *loansFile = fopen(loansFileName, "r+");
    if (loansFile == NULL) {
        printf("Error opening loans file: %s\n", strerror(errno));
        return;
    }

    int currentLoans;
    fscanf(loansFile, "%d", &currentLoans);
    fseek(loansFile, 0, SEEK_SET);
    fprintf(loansFile, "%d", currentLoans - 1);
    fclose(loansFile);

    printf("Successfully returned %d book(s) from loan %d.\n", totalReturnedCopies, choice);
}

static int loanNumber = 0; // Global variable to store the loan number

void loadLoanNumber() {
    FILE *file = fopen("loan_number.txt", "r");
    if (file != NULL) {
        fscanf(file, "%d", &loanNumber);
        fclose(file);
    }
}

void saveLoanNumber() {
    FILE *file = fopen("loan_number.txt", "w");
    if (file != NULL) {
        fprintf(file, "%d", loanNumber);
        fclose(file);
    }
}

void borrowBooks() {
    int bookId, copiesToBorrow;

    printf("Enter the ID of the book you want to borrow: ");
    scanf("%d", &bookId);

    printf("Enter the number of copies you want to borrow: ");
    scanf("%d", &copiesToBorrow);

    // Find the book by ID
    int bookIndex = -1;
    for (int i = 0; i < numBooks; i++) {
        if (books[i].id == bookId) {
            bookIndex = i;
            break;
        }
    }

    if (bookIndex == -1) {
        printf("Book with ID %d not found.\n", bookId);
        return;
    }

    // Check if there are enough copies available
    if (copiesToBorrow > books[bookIndex].copiesAvailable) {
        printf("Sorry, there are only %d copies available for \"%s\".\n", books[bookIndex].copiesAvailable, books[bookIndex].title);
        return;
    }

    // Decrement the available copies
    books[bookIndex].copiesAvailable -= copiesToBorrow;
    saveBooksToFile(); // Update book.txt

    // Construct folder name
    char folderName[MAX_NAME_LENGTH + MAX_SURNAME_LENGTH + 2];
    snprintf(folderName, sizeof(folderName), "%s_%s", Name, Surname);

    // Increment the number of loans for the current user
    char loansFileName[100];
    snprintf(loansFileName, sizeof(loansFileName), "%s/loans.txt", folderName);
    FILE *loansFile = fopen(loansFileName, "r+");
    if (loansFile == NULL) {
        printf("Error opening loans file.\n");
        return;
    }
    int numLoans;
    fscanf(loansFile, "%d", &numLoans);
    numLoans++; // Increment the number of loans
    rewind(loansFile); // Move file position indicator to the beginning
    fprintf(loansFile, "%d", numLoans); // Write the updated number of loans
    fclose(loansFile);

    // Construct loan file name
    char loanFileName[50];
    snprintf(loanFileName, sizeof(loanFileName), "%s/loan_%d.txt", folderName, numLoans);

    // Create loan file
    FILE *loanFile = fopen(loanFileName, "w");
    if (loanFile == NULL) {
        printf("Error creating loan file.\n");
        return;
    }

    // Write loan details to the loan file
    fprintf(loanFile, "Book ID: %d\n", bookId);
    fprintf(loanFile, "Title: %s\n", books[bookIndex].title);
    fprintf(loanFile, "Author: %s\n", books[bookIndex].author);
    fprintf(loanFile, "Copies Borrowed: %d\n", copiesToBorrow);
    fclose(loanFile);

    printf("You have successfully borrowed %d copies of \"%s\".\n", copiesToBorrow, books[bookIndex].title);
}



void seeBooks() {
    printf("List of Books:\n");
    printf("%-5s%-30s%-20s%-10s\n", "ID", "Title", "Author", "Copies");
    printf("--------------------------------------------------------\n");

    FILE *file = fopen(FILENAME2, "r");
    if (file == NULL) {
        printf("Error opening file %s.\n", FILENAME2);
        return;
    }

    int id, copies;
    char title[MAX_TITLE_LENGTH], author[MAX_AUTHOR_LENGTH];
    while (fscanf(file, "%d;%[^;];%[^;];%d\n", &id, title, author, &copies) == 4) {
        printf("%-5d%-30s%-20s%-10d\n", id, title, author, copies);
    }

    fclose(file);
}

void loadBooksFromFile() {
    FILE *file = fopen(FILENAME2, "r");
    if (file == NULL) {
        printf("Failed to open file %s\n", FILENAME2);
        return;
    }

    while (fscanf(file, "%d;%999[^;];%999[^;];%d\n", &books[numBooks].id, books[numBooks].title, books[numBooks].author, &books[numBooks].copiesAvailable) == 4) {
        numBooks++;
        if (numBooks >= MAX_ACCOUNTS) {
            printf("Max books limit reached.\n");
            break;
        }
    }

    fclose(file);
}


void donateBook() {
    Book newBook;
    FILE *numBooksFile = fopen(NUMBOOKS_FILE, "r+");
    if (numBooksFile == NULL) {
        printf("Error opening numberofbooks.txt file.\n");
        return;
    }

    // Read current number of books
    int numBooks;
    fscanf(numBooksFile, "%d", &numBooks);
    fseek(numBooksFile, 0, SEEK_SET); // Move file pointer to the beginning

    // Ask for book details
    printf("Enter title of the book: ");
    scanf(" %[^\n]", newBook.title); // Read until newline character
    while (getchar() != '\n'); // Clear input buffer

    printf("Enter author of the book: ");
    scanf(" %[^\n]", newBook.author); // Read until newline character
    while (getchar() != '\n'); // Clear input buffer

    printf("Enter number of copies available (should be >= 1): ");
    scanf("%d", &newBook.copiesAvailable);
    while (getchar() != '\n'); // Clear input buffer

    // Get the next ID from the numberofbooks.txt file
    int newId = numBooks + 1;

    // Open books.txt to append the new book
    FILE *file = fopen(FILENAME2, "a");
    if (file == NULL) {
        printf("Error opening file %s for appending.\n", FILENAME2);
        fclose(numBooksFile);
        return;
    }

    // Write the new book to books.txt
    fprintf(file, "%d;%s;%s;%d\n", newId, newBook.title, newBook.author, newBook.copiesAvailable);
    fclose(file);

    // Update the number of books in the numberofbooks.txt file
    fprintf(numBooksFile, "%d", newId);
    fclose(numBooksFile);

    printf("Book \"%s\" donated successfully with ID %d and %d copies available.\n",
           newBook.title, newId, newBook.copiesAvailable);
}



void searchBook() {
    int choice;
    char keyword[MAX_TITLE_LENGTH];
    printf("Search by:\n");
    printf("1. Author\n");
    printf("2. Book Name\n");
    printf("3. Both\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    printf("Enter the search keyword: ");
    getchar(); // Clear input buffer
    fgets(keyword, sizeof(keyword), stdin);
    // Remove newline character
    if ((strlen(keyword) > 0) && (keyword[strlen(keyword) - 1] == '\n')) {
        keyword[strlen(keyword) - 1] = '\0';
    }

    int found = 0;
    printf("Results:\n");
    printf("%-5s%-25s%-25s%-7s\n", "ID", "Author", "Book Name", "Copies");
    printf("--------------------------------------------------\n");
    for (int i = 0; i < numBooks; i++) {
        if ((choice == 1 || choice == 3) && strstr(books[i].author, keyword) != NULL) {
            printf("%-5d%-25s%-25s%-7d\n", books[i].id, books[i].author, books[i].title, books[i].copiesAvailable);
            found = 1;
        } else if ((choice == 2 || choice == 3) && strstr(books[i].title, keyword) != NULL) {
            printf("%-5d%-25s%-25s%-7d\n", books[i].id, books[i].author, books[i].title, books[i].copiesAvailable);
            found = 1;
        }
    }
    if (!found) {
        printf("No books found.\n");
    }
}

int main() {
    loadAccountsFromFile();
    loadBooksFromFile();

    int choice;
    do {
        printf("\n=========================================\n");
        printf("         Welcome to the Library          \n");
        printf("           Management System!            \n");
        printf("=========================================\n");
        printf("1. Login\n");
        printf("2. Signup\n");
        printf("3. Exit\n");
        printf("-----------------------------------------\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                if(login())
                {
                    int subChoice;
                    do {
                        printf("\n=========================================\n");
                        printf("                  Menu                   \n");
                        printf("=========================================\n");
                        printf("1. Log out\n");
                        printf("2. View loans\n");
                        printf("3. Borrow books\n");
                        printf("4. Return borrowed books\n");
                        printf("5. Donate a book\n");
                        printf("6. Search a book\n");
                        printf("7. See books\n");
                        printf("-----------------------------------------\n");
                        printf("Enter your choice: ");
                        scanf("%d", &subChoice);

                        switch (subChoice) {
                            case 1:
                                printf("\nLogging out...\n");
                                break;
                            case 2:
                                seeLoans();
                                break;
                            case 3:
                                borrowBooks();
                                break;
                            case 4:
                                returnBooks();
                                break;
                            case 5:
                                donateBook();
                                break;
                            case 6:
                                searchBook();
                                break;
                            case 7:
                                seeBooks();
                                break;
                            default:
                                printf("\nInvalid choice. Please try again.\n");
                        }
                    } while (subChoice != 1);
                }
                break;
            case 2:
                signup();
                break;
            case 3:
                printf("\nExiting...\n");
                break;
            default:
                printf("\nInvalid choice. Please try again.\n");
        }
    } while (choice != 3);

    return 0;
}