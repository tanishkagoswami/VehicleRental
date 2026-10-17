#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MODEL        40
#define MAX_NAME         40
#define FUEL_LEN         10
#define INITIAL_CAPACITY 2
#define MAX_UNITS        1000
#define MIN_BIKE_HOURS   2
#define FLEET_FILE       "fleet.txt"
#define RENTAL_LOG       "rentals.txt"
#define RECEIPT_FILE     "receipts.txt"
#define DASHBOARD_DATA   "dashboard_data.js"
#define DASHBOARD_PAGE   "rental_dashboard.html"
#define DEBUG            1      
const float GST_RATE        = 0.18f;
const float LATE_MULTIPLIER = 1.5f;


union VehicleDetails {
    struct {
        int  seats;
        char fuel[FUEL_LEN];
    } car;
    struct {
        int engineCC;
    } bike;
    struct {
        float loadTons;
    } truck;
};

struct Vehicle {
    int   id;
    char  model[MAX_MODEL];
    char  type;                    
    float rate;                    
    int   isAvailable;             
    int   bookedUnits;             
    char  rentedBy[MAX_NAME];      
    union VehicleDetails info;
};

struct Rental {
    int   vehicleId;
    char  customer[MAX_NAME];
    int   bookedUnits;
    int   actualUnits;
    float baseAmount;
    float lateFee;
    float gst;
    float total;
};


struct Vehicle *fleet = NULL;   
int   count        = 0;         
int   capacity     = 0;         
int   nextId       = 101;
float totalRevenue = 0.0f;


void  clearInput(void);
void  endOfInput(void);
int   readInt(const char *prompt);
float readFloat(const char *prompt);
char  readChar(const char *prompt);
void  readLine(const char *prompt, char *buffer, int size);
void  replaceSpaces(char *s);


int         typeIndex(char type);
const char *typeName(char type);
const char *unitName(char type);


float carRent(struct Vehicle *v, int days);
float bikeRent(struct Vehicle *v, int hours);
float truckRent(struct Vehicle *v, int km);
float calcGST(float amount);


int  ensureCapacity(void);
void freeFleet(void);


void printHeader(void);
void printVehicle(struct Vehicle v);
void displayAll(void);
void displayAvailable(void);
int  countAvailable(int index);

struct Vehicle *searchById(int low, int high, int id);
void searchByIdMenu(void);
void searchByModel(const char *key);
void searchByModelMenu(void);
int  compareByRate(struct Vehicle *a, struct Vehicle *b);
int  compareByModel(struct Vehicle *a, struct Vehicle *b);
void bubbleSort(struct Vehicle *arr, int n, int (*cmp)(struct Vehicle *, struct Vehicle *));
void sortAndShow(void);

void addVehicle(void);
void markRented(struct Vehicle *v, const char *customer, int units);
void markReturned(struct Vehicle *v);
void rentVehicle(void);
void returnVehicle(void);
void printBill(struct Rental *r, struct Vehicle *v);
void revenueReport(void);

void saveFleet(void);
void loadFleet(void);
void logEvent(const char *action, struct Vehicle *v, const char *customer, int units, float amount);
void writeReceipt(struct Rental *r, struct Vehicle *v);
void viewHistory(void);
void exportDashboard(void);
void writeJsLines(FILE *out, const char *key, const char *fileName);
void openDashboard(void);

void printMenu(void);

float (*pricing[3])(struct Vehicle *, int) = { carRent, bikeRent, truckRent };

int main(void)
{
    int choice;

    printf("=========================================\n");
    printf("     VEHICLE RENTAL MANAGEMENT SYSTEM\n");
    printf("=========================================\n");
    loadFleet();

    do {
        printMenu();
        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1:  addVehicle();        break;
            case 2:  displayAll();        break;
            case 3:  displayAvailable();  break;
            case 4:  searchByIdMenu();    break;
            case 5:  searchByModelMenu(); break;
            case 6:  sortAndShow();       break;
            case 7:  rentVehicle();       break;
            case 8:  returnVehicle();     break;
            case 9:  viewHistory();       break;
            case 10: revenueReport();     break;
            case 11: openDashboard();     break;
            case 0:
                saveFleet();
                printf("Data saved. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);

    freeFleet();
    return 0;
}

void printMenu(void)
{
    printf("\n----------------- MENU -----------------\n");
    printf(" 1. Add vehicle\n");
    printf(" 2. View all vehicles\n");
    printf(" 3. View available vehicles\n");
    printf(" 4. Search by ID\n");
    printf(" 5. Search by model name\n");
    printf(" 6. Sort vehicles\n");
    printf(" 7. Rent a vehicle\n");
    printf(" 8. Return a vehicle\n");
    printf(" 9. View rental history\n");
    printf("10. Revenue report\n");
    printf("11. Open dashboard\n");
    printf(" 0. Save and exit\n");
    printf("----------------------------------------\n");
}


void clearInput(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void endOfInput(void)
{
    printf("\nInput ended. Saving data and exiting.\n");
    saveFleet();
    freeFleet();
    exit(0);
}

int readInt(const char *prompt)
{
    int value, result;

    printf("%s", prompt);
    while ((result = scanf("%d", &value)) != 1) {
        if (result == EOF)
            endOfInput();
        clearInput();
        printf("Please enter a valid number: ");
    }
    clearInput();
    return value;
}

float readFloat(const char *prompt)
{
    float value;
    int result;

    printf("%s", prompt);
    while ((result = scanf("%f", &value)) != 1) {
        if (result == EOF)
            endOfInput();
        clearInput();
        printf("Please enter a valid number: ");
    }
    clearInput();
    return value;
}

char readChar(const char *prompt)
{
    char c;

    printf("%s", prompt);
    if (scanf(" %c", &c) != 1)
        endOfInput();
    clearInput();
    return c;
}

void readLine(const char *prompt, char *buffer, int size)
{
    int len;

    printf("%s", prompt);
    if (fgets(buffer, size, stdin) == NULL)
        endOfInput();

    len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
        buffer[len - 1] = '\0';
}

void replaceSpaces(char *s)
{
    while (*s != '\0') {
        if (*s == ' ')
            *s = '_';
        s++;                        
    }
}


int typeIndex(char type)
{
    switch (type) {
        case 'C': return 0;
        case 'B': return 1;
        case 'T': return 2;
        default:  return -1;
    }
}

const char *typeName(char type)
{
    switch (type) {
        case 'C': return "Car";
        case 'B': return "Bike";
        case 'T': return "Truck";
        default:  return "Unknown";
    }
}

const char *unitName(char type)
{
    switch (type) {
        case 'C': return "day";
        case 'B': return "hour";
        case 'T': return "km";
        default:  return "unit";
    }
}


float carRent(struct Vehicle *v, int days)
{
    float amount = v->rate * days;

    if (v->info.car.seats > 5)
        amount = amount + amount * 0.10f;      
    if (days >= 7)
        amount = amount - amount * 0.10f;      
    return amount;
}

float bikeRent(struct Vehicle *v, int hours)
{
    float amount;

    if (hours < MIN_BIKE_HOURS)
        hours = MIN_BIKE_HOURS;
    amount = v->rate * hours;
    if (v->info.bike.engineCC > 300)
        amount += amount * 0.15f;
    return amount;
}

float truckRent(struct Vehicle *v, int km)
{
    return v->rate * km + v->info.truck.loadTons * 200;
}


float calcGST(float amount)
{
    return amount * GST_RATE;
}


int ensureCapacity(void)
{
    struct Vehicle *temp;

    if (fleet == NULL) {
        fleet = (struct Vehicle *) calloc(INITIAL_CAPACITY, sizeof(struct Vehicle));
        if (fleet == NULL) {
            printf("Error: memory allocation failed.\n");
            return 0;
        }
        capacity = INITIAL_CAPACITY;
        if (DEBUG) printf("[DEBUG] calloc: space for %d vehicles\n", capacity);
    }
    else if (count == capacity) {
        temp = (struct Vehicle *) realloc(fleet, capacity * 2 * sizeof(struct Vehicle));
        if (temp == NULL) {
            printf("Error: could not expand memory.\n");
            return 0;
        }
        fleet = temp;
        capacity = capacity * 2;
        if (DEBUG) printf("[DEBUG] realloc: space for %d vehicles\n", capacity);
    }
    return 1;
}

void freeFleet(void)
{
    free(fleet);
    fleet = NULL;          
    count = 0;
    capacity = 0;
}


void printHeader(void)
{
    printf("\n%-5s %-18s %-6s %-14s %-10s %s\n",
           "ID", "Model", "Type", "Rate (Rs)", "Status", "Details");
    printf("--------------------------------------------------------------------------\n");
}

void printVehicle(struct Vehicle v)
{
    printf("%-5d %-18s %-6s %8.2f/%-5s %-10s ",
           v.id, v.model, typeName(v.type), v.rate, unitName(v.type),
           v.isAvailable ? "Available" : "Rented");

    switch (v.type) {
        case 'C': printf("%d seats, %s", v.info.car.seats, v.info.car.fuel); break;
        case 'B': printf("%d cc", v.info.bike.engineCC);                     break;
        case 'T': printf("%.1f tons", v.info.truck.loadTons);                break;
    }
    if (!v.isAvailable)
        printf(" [by %s]", v.rentedBy);
    printf("\n");
}

void displayAll(void)
{
    struct Vehicle *p;

    if (count == 0) {
        printf("No vehicles in the fleet yet.\n");
        return;
    }
    printHeader();
    for (p = fleet; p < fleet + count; p++)
        printVehicle(*p);
    printf("Total vehicles: %d\n", count);
}

int countAvailable(int index)
{
    if (index == count)
        return 0;                                  
    return fleet[index].isAvailable + countAvailable(index + 1);
}

void displayAvailable(void)
{
    int i, available = countAvailable(0);

    if (available == 0) {
        printf("No vehicles are available right now.\n");
        return;
    }
    printHeader();
    for (i = 0; i < count; i++) {
        if (!fleet[i].isAvailable)
            continue;                              
        printVehicle(fleet[i]);
    }
    printf("Available: %d of %d\n", available, count);
}

struct Vehicle *searchById(int low, int high, int id)
{
    int mid;

    if (low > high)
        return NULL;                              
    mid = (low + high) / 2;

    if (fleet[mid].id == id)
        return fleet + mid;
    else if ((*(fleet + mid)).id < id)
        return searchById(mid + 1, high, id);
    else
        return searchById(low, mid - 1, id);
}

void searchByIdMenu(void)
{
    int id = readInt("Enter vehicle ID: ");
    struct Vehicle *v = searchById(0, count - 1, id);

    if (v == NULL) {
        printf("No vehicle found with ID %d.\n", id);
        return;
    }
    printHeader();
    printVehicle(*v);
}

void searchByModel(const char *key)
{
    int i, found = 0;

    for (i = 0; i < count; i++) {
        if (strstr(fleet[i].model, key) != NULL) {
            if (found == 0)
                printHeader();
            printVehicle(fleet[i]);
            found++;
        }
    }
    if (found == 0)
        printf("No vehicle matches \"%s\".\n", key);
}

void searchByModelMenu(void)
{
    char key[MAX_MODEL];

    readLine("Enter model name (or part of it): ", key, MAX_MODEL);
    if (strlen(key) == 0) {
        printf("Search text cannot be empty.\n");
        return;
    }
    replaceSpaces(key);
    searchByModel(key);
}

int compareByRate(struct Vehicle *a, struct Vehicle *b)
{
    if (a->rate > b->rate) return 1;
    if (a->rate < b->rate) return -1;
    return 0;
}

int compareByModel(struct Vehicle *a, struct Vehicle *b)
{
    return strcmp(a->model, b->model);
}

void bubbleSort(struct Vehicle *arr, int n, int (*cmp)(struct Vehicle *, struct Vehicle *))
{
    int i, j;
    struct Vehicle temp;

    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (cmp(&arr[j], &arr[j + 1]) > 0) {
                temp = arr[j];             
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void sortAndShow(void)
{
    int choice, i;
    struct Vehicle *copy;
    int (*cmp)(struct Vehicle *, struct Vehicle *) = NULL;

    if (count == 0) {
        printf("No vehicles to sort.\n");
        return;
    }

    choice = readInt("Sort by: 1. Rate (low to high)  2. Model name : ");
    if (choice == 1)
        cmp = compareByRate;
    else if (choice == 2)
        cmp = compareByModel;
    else {
        printf("Invalid option.\n");
        return;
    }

    copy = (struct Vehicle *) malloc(count * sizeof(struct Vehicle));
    if (copy == NULL) {
        printf("Error: memory allocation failed.\n");
        return;
    }
    for (i = 0; i < count; i++)
        copy[i] = fleet[i];

    bubbleSort(copy, count, cmp);

    printHeader();
    for (i = 0; i < count; i++)
        printVehicle(copy[i]);

    free(copy);
    copy = NULL;        
}


void addVehicle(void)
{
    struct Vehicle v;
    char type;

    if (!ensureCapacity())
        return;

    type = readChar("Vehicle type (C = Car, B = Bike, T = Truck): ");
    if (type >= 'a' && type <= 'z')
        type = type - 32;                    
    if (typeIndex(type) == -1) {
        printf("Invalid vehicle type.\n");
        return;
    }
    v.type = type;

    do {
        readLine("Model name: ", v.model, MAX_MODEL);
    } while (strlen(v.model) == 0);
    replaceSpaces(v.model);

    do {
        printf("Base rate per %s ", unitName(type));
        v.rate = readFloat("(Rs): ");
    } while (v.rate <= 0);

    switch (type) {
        case 'C':
            do {
                v.info.car.seats = readInt("Number of seats (2-12): ");
            } while (v.info.car.seats < 2 || v.info.car.seats > 12);
            do {
                readLine("Fuel type (Petrol/Diesel/CNG/EV): ", v.info.car.fuel, FUEL_LEN);
            } while (strlen(v.info.car.fuel) == 0);
            replaceSpaces(v.info.car.fuel);
            break;
        case 'B':
            do {
                v.info.bike.engineCC = readInt("Engine capacity (cc): ");
            } while (v.info.bike.engineCC <= 0);
            break;
        case 'T':
            do {
                v.info.truck.loadTons = readFloat("Load capacity (tons): ");
            } while (v.info.truck.loadTons <= 0);
            break;
    }

    v.isAvailable = 1;
    v.bookedUnits = 0;
    strcpy(v.rentedBy, "-");
    v.id = nextId++;

    fleet[count] = v;                        
    count++;
    saveFleet();
    printf("%s added successfully with ID %d.\n", typeName(type), v.id);
}

void markRented(struct Vehicle *v, const char *customer, int units)
{
    v->isAvailable = 0;
    v->bookedUnits = units;
    strcpy(v->rentedBy, customer);
}

void markReturned(struct Vehicle *v)
{
    v->isAvailable = 1;
    v->bookedUnits = 0;
    strcpy(v->rentedBy, "-");
}

void rentVehicle(void)
{
    int id, units;
    float estimate;
    char customer[MAX_NAME];
    struct Vehicle *v;

    id = readInt("Enter vehicle ID to rent: ");
    v = searchById(0, count - 1, id);

    if (v == NULL) {
        printf("No vehicle found with ID %d.\n", id);
        return;
    }
    if (!v->isAvailable) {
        printf("Sorry, this vehicle is already rented by %s.\n", v->rentedBy);
        return;
    }

    do {
        readLine("Customer name: ", customer, MAX_NAME);
    } while (strlen(customer) == 0);
    replaceSpaces(customer);

    printf("Book for how many %ss? ", unitName(v->type));
    units = readInt("");
    if (units <= 0 || units > MAX_UNITS) {
        printf("Invalid value. Must be between 1 and %d.\n", MAX_UNITS);
        return;
    }

    markRented(v, customer, units);

    
    estimate = pricing[typeIndex(v->type)](v, units);
    printf("Vehicle %d rented to %s for %d %s(s).\n", v->id, customer, units, unitName(v->type));
    printf("Estimated charge: Rs %.2f + GST\n", estimate);

    logEvent("RENTED", v, customer, units, estimate);
    saveFleet();
}

void returnVehicle(void)
{
    int id, actual;
    struct Vehicle *v;
    struct Rental *r;

    id = readInt("Enter vehicle ID to return: ");
    v = searchById(0, count - 1, id);

    if (v == NULL) {
        printf("No vehicle found with ID %d.\n", id);
        return;
    }
    if (v->isAvailable) {
        printf("This vehicle is not currently rented.\n");
        return;
    }

    printf("Booked: %d %s(s). ", v->bookedUnits, unitName(v->type));
    printf("Actual %ss used: ", unitName(v->type));
    actual = readInt("");
    if (actual <= 0 || actual > MAX_UNITS) {
        printf("Invalid value.\n");
        return;
    }

    r = (struct Rental *) malloc(sizeof(struct Rental));
    if (r == NULL) {
        printf("Error: memory allocation failed.\n");
        return;
    }

    r->vehicleId   = v->id;
    strcpy(r->customer, v->rentedBy);
    r->bookedUnits = v->bookedUnits;
    r->actualUnits = actual;
    r->baseAmount  = pricing[typeIndex(v->type)](v, r->bookedUnits);

    if (actual > r->bookedUnits)
        r->lateFee = (actual - r->bookedUnits) * v->rate * LATE_MULTIPLIER;
    else
        r->lateFee = 0;

    r->gst   = calcGST(r->baseAmount + r->lateFee);
    r->total = r->baseAmount + r->lateFee + r->gst;

    printBill(r, v);
    writeReceipt(r, v);
    logEvent("RETURNED", v, r->customer, actual, r->total);

    totalRevenue += r->total;
    markReturned(v);
    saveFleet();

    free(r);
    r = NULL;              
}

void printBill(struct Rental *r, struct Vehicle *v)
{
    printf("\n=============== BILL ===============\n");
    printf("Vehicle   : %s (%s, ID %d)\n", v->model, typeName(v->type), v->id);
    printf("Customer  : %s\n", r->customer);
    printf("Booked    : %d %s(s)\n", r->bookedUnits, unitName(v->type));
    printf("Used      : %d %s(s)\n", r->actualUnits, unitName(v->type));
    printf("Base      : Rs %10.2f\n", r->baseAmount);
    printf("Late fee  : Rs %10.2f\n", r->lateFee);
    printf("GST (18%%) : Rs %10.2f\n", r->gst);
    printf("------------------------------------\n");
    printf("TOTAL     : Rs %10.2f\n", r->total);
    printf("====================================\n");
}

void revenueReport(void)
{
    int typeCount[3] = {0, 0, 0};
    int i, rented;

    for (i = 0; i < count; i++)
        typeCount[typeIndex(fleet[i].type)]++;

    rented = count - countAvailable(0);

    printf("\n========= REVENUE REPORT =========\n");
    printf("Cars        : %d\n", typeCount[0]);
    printf("Bikes       : %d\n", typeCount[1]);
    printf("Trucks      : %d\n", typeCount[2]);
    printf("Total fleet : %d\n", count);
    printf("Rented now  : %d\n", rented);
    printf("Available   : %d\n", count - rented);
    printf("Revenue     : Rs %.2f\n", totalRevenue);
    printf("==================================\n");
}


void saveFleet(void)
{
    FILE *fp;
    struct Vehicle *p;

    fp = fopen(FLEET_FILE, "w");
    if (fp == NULL) {
        printf("Error: could not save data.\n");
        return;
    }

    fprintf(fp, "%d %.2f %d\n", nextId, totalRevenue, count);
    for (p = fleet; p < fleet + count; p++) {
        fprintf(fp, "%d %s %c %.2f %d %d %s ",
                p->id, p->model, p->type, p->rate,
                p->isAvailable, p->bookedUnits, p->rentedBy);
        switch (p->type) {
            case 'C': fprintf(fp, "%d %s\n", p->info.car.seats, p->info.car.fuel); break;
            case 'B': fprintf(fp, "%d\n", p->info.bike.engineCC);                 break;
            case 'T': fprintf(fp, "%.2f\n", p->info.truck.loadTons);              break;
        }
    }
    fclose(fp);
    if (DEBUG) printf("[DEBUG] Saved %d vehicle(s) to %s\n", count, FLEET_FILE);

    exportDashboard();
}

void loadFleet(void)
{
    FILE *fp;
    struct Vehicle v;
    int n, i;

    fp = fopen(FLEET_FILE, "r");
    if (fp == NULL) {                 
        printf("No saved data found. Starting with an empty fleet.\n");
        return;
    }

    if (fscanf(fp, "%d %f %d", &nextId, &totalRevenue, &n) != 3) {
        printf("Saved file is empty or damaged. Starting fresh.\n");
        nextId = 101;
        totalRevenue = 0;
        fclose(fp);
        return;
    }

    for (i = 0; i < n; i++) {
        if (fscanf(fp, "%d %39s %c %f %d %d %39s",
                   &v.id, v.model, &v.type, &v.rate,
                   &v.isAvailable, &v.bookedUnits, v.rentedBy) != 7)
            break;

        int ok = 0;
        switch (v.type) {
            case 'C': ok = (fscanf(fp, "%d %9s", &v.info.car.seats, v.info.car.fuel) == 2); break;
            case 'B': ok = (fscanf(fp, "%d", &v.info.bike.engineCC) == 1);                  break;
            case 'T': ok = (fscanf(fp, "%f", &v.info.truck.loadTons) == 1);                 break;
        }
        if (!ok || !ensureCapacity())
            break;

        fleet[count] = v;
        count++;
    }
    fclose(fp);
    printf("Loaded %d vehicle(s) from %s.\n", count, FLEET_FILE);
}

void logEvent(const char *action, struct Vehicle *v, const char *customer, int units, float amount)
{
    FILE *fp = fopen(RENTAL_LOG, "a");

    if (fp == NULL) {
        printf("Warning: could not write to history file.\n");
        return;
    }
    fprintf(fp, "%-8s | ID %d | %-15s | %-10s | %d %s(s) | Rs %.2f\n",
            action, v->id, v->model, customer, units, unitName(v->type), amount);
    fclose(fp);
}

void writeReceipt(struct Rental *r, struct Vehicle *v)
{
    static int receiptNo = 1;      
    char line[100];
    FILE *fp = fopen(RECEIPT_FILE, "a");

    if (fp == NULL) {
        printf("Warning: could not write receipt.\n");
        return;
    }

    fputs("========================================\n", fp);
    fprintf(fp, "Receipt No : %d (this session)\n", receiptNo);

    strcpy(line, "Vehicle    : ");
    strcat(line, v->model);
    strcat(line, "\n");
    fputs(line, fp);

    strcpy(line, "Customer   : ");
    strcat(line, r->customer);
    strcat(line, "\n");
    fputs(line, fp);

    fprintf(fp, "Booked     : %d %s(s)\n", r->bookedUnits, unitName(v->type));
    fprintf(fp, "Used       : %d %s(s)\n", r->actualUnits, unitName(v->type));
    fprintf(fp, "Base       : Rs %.2f\n", r->baseAmount);
    fprintf(fp, "Late fee   : Rs %.2f\n", r->lateFee);
    fprintf(fp, "GST        : Rs %.2f\n", r->gst);
    fprintf(fp, "TOTAL      : Rs %.2f\n", r->total);
    fputs("========================================\n\n", fp);

    fclose(fp);
    printf("Receipt saved to %s.\n", RECEIPT_FILE);
    receiptNo++;
}


void viewHistory(void)
{
    char line[200];
    int lines = 0;
    FILE *fp = fopen(RENTAL_LOG, "r");

    if (fp == NULL) {
        printf("No rental history yet.\n");
        return;
    }
    printf("\n------------------------- RENTAL HISTORY -------------------------\n");
    while (fgets(line, 200, fp) != NULL) {
        printf("%s", line);
        lines++;
    }
    if (lines == 0)
        printf("History is empty.\n");
    fclose(fp);
}


void writeJsLines(FILE *out, const char *key, const char *fileName)
{
    char line[256];
    char *p;
    int len;
    FILE *in = fopen(fileName, "r");

    fprintf(out, "  %s: [\n", key);
    if (in != NULL) {
        while (fgets(line, 256, in) != NULL) {
            len = strlen(line);
            while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
                line[--len] = '\0';           
            if (len == 0)
                continue;                     

            fputs("    \"", out);
            for (p = line; *p != '\0'; p++) {
                if (*p == '"' || *p == '\\')
                    fputc('\\', out);
                fputc(*p, out);
            }
            fputs("\",\n", out);
        }
        fclose(in);
    }
    fputs("  ]", out);
}


void exportDashboard(void)
{
    FILE *out = fopen(DASHBOARD_DATA, "w");

    if (out == NULL) {
        printf("Warning: could not update the dashboard file.\n");
        return;
    }
    fputs("/* Written by vehicle_rental.c - do not edit by hand */\n", out);
    fputs("window.RENTAL_DATA = {\n", out);
    writeJsLines(out, "fleet", FLEET_FILE);
    fputs(",\n", out);
    writeJsLines(out, "rentals", RENTAL_LOG);
    fputs("\n};\n", out);
    fclose(out);
    if (DEBUG) printf("[DEBUG] Dashboard data updated in %s\n", DASHBOARD_DATA);
}


void openDashboard(void)
{
    int result;
    FILE *check = fopen(DASHBOARD_PAGE, "r");

    if (check == NULL) {
        printf("%s not found. Put it in the same folder as this program.\n", DASHBOARD_PAGE);
        return;
    }
    fclose(check);

    saveFleet();                   
#ifdef _WIN32
    result = system("start \"\" " DASHBOARD_PAGE);
#elif defined(__APPLE__)
    result = system("open " DASHBOARD_PAGE);
#else
    result = system("xdg-open " DASHBOARD_PAGE " >/dev/null 2>&1 &");
#endif

    if (result != 0)
        printf("Could not open the browser. Double-click %s instead.\n", DASHBOARD_PAGE);
    else
        printf("Dashboard opened. It refreshes every 3 seconds while this program runs.\n");
}

