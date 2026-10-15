# Vehicle Rental Management System

A menu-driven vehicle rental system written in C, with a browser dashboard that updates live as the program runs.

The shop rents out cars, bikes and trucks, each charged differently:

| Type  | Charged per | Rules |
|-------|-------------|-------|
| Car   | day   | +10% for more than 5 seats, 10% off for 7+ days |
| Bike  | hour  | 2-hour minimum, +15% above 300 cc |
| Truck | km    | plus ₹200 per ton of load capacity |

Late returns are charged at 1.5× the base rate for the extra units, and 18% GST is added to every bill.

## Features

- Add, view, search (by ID or model name) and sort vehicles
- Rent and return vehicles, with bills that include late fees and GST
- Rental history and revenue report
- All data saved to text files and reloaded on the next run
- Demo data loader (menu option 12)
- Live web dashboard (menu option 11) that highlights vehicles as they are added, rented or returned

## Run it

Requires a C compiler such as GCC.

```bash
gcc vehicle_rental.c -o rental
./rental            # Windows: .\rental.exe
```

Choose **12** to load demo data, then **11** to open the dashboard. Keep `rental_dashboard.html` in the same folder as the program.

## How the dashboard connects

Every time the C program saves, it also writes `dashboard_data.js`, a copy of `fleet.txt` and `rentals.txt` as JavaScript. The dashboard reloads that file every 3 seconds. Browsers don't let a page read local `.txt` files directly, but they do let it load a local script, so this keeps the C program as the only place where data changes.

## C concepts used

| Module | Concepts | Where |
|--------|----------|-------|
| Basics | constants, operators, precedence, I/O, selection, loops | menu loop, validations, `truckRent()` |
| Arrays & functions | prototypes, call by value / reference, scope, `static`, recursion, string functions | `calcGST()`, `markRented()`, `searchById()`, `countAvailable()` |
| Pointers & memory | pointer arithmetic, `calloc`, `realloc`, `malloc`, `free`, function pointers, NULL and dangling pointers | `ensureCapacity()`, `pricing[]`, `bubbleSort()` |
| Structures, unions & files | `struct Vehicle`, `union VehicleDetails`, `fopen`, `fprintf`, `fscanf`, `fgets`, `fputs` | `saveFleet()`, `loadFleet()`, `writeReceipt()` |

A full concept-to-function map is in the comment block at the top of `vehicle_rental.c`.

## Files

| File | Purpose |
|------|---------|
| `vehicle_rental.c` | The complete program |
| `rental_dashboard.html` | Live dashboard (open in any browser) |
| `fleet.txt`, `rentals.txt`, `receipts.txt`, `dashboard_data.js` | Created when the program runs |