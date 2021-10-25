// RUN: sqlgen %s | %FileCheck %s

include "CreateTables.td"

def john : Customer<"John Smith">;
def mary : Customer<"Mary Blackburn">;
def bob  : Customer<"Bob Glass">;

// Customer rows will be created on-demand.

// CHECK: INSERT INTO Customer (ID, Name, Affiliation)
// CHECK: VALUES (0, "John Smith", "");
//
// CHECK: INSERT INTO Orders (SeqNumber, ProductName, Person, Amount)
// CHECK: VALUES (0, "water bottle", 0, 100);
def : Orders<"water bottle", john, 100,
             2021, 10, 21>;

// CHECK: INSERT INTO Customer (ID, Name, Affiliation)
// CHECK: VALUES (1, "Mary Blackburn", "");
//
// CHECK: INSERT INTO Orders (SeqNumber, ProductName, Person, Amount)
// CHECK: VALUES (1, "F150", 1, 2);
//
// CHECK: INSERT INTO Orders (SeqNumber, ProductName, Person, Amount)
// CHECK: VALUES (2, "Tide Pods", 1, 60);
def : Orders<"F150", mary, 2,
             2021, 1, 4>;
def : Orders<"Tide Pods", mary, 60,
             2021, 2, 1>;

// CHECK: INSERT INTO Customer (ID, Name, Affiliation)
// CHECK: VALUES (2, "Bob Glass", "");
//
// CHECK: INSERT INTO Orders (SeqNumber, ProductName, Person, Amount)
// CHECK: VALUES (3, "RTX 3090", 2, 1);
def : Orders<"RTX 3090", bob, 1,
             2020, 5, 6>;
