// RUN: sqlgen %s | %FileCheck %s

include "Table.td"

// CHECK-LABEL: CREATE TABLE Customer (
// CHECK:         ID int,
// CHECK:         Name varchar(255),
// CHECK:         Affiliation varchar(255),
// CHECK:         PRIMARY KEY (ID)
// CHECK:       );
class Customer <string name, string affiliation = ""> : Table {
  int ID = PrimaryKey;
  string Name = name;
  string Affiliation = affiliation;
}

// CHECK-LABEL: CREATE TABLE Orders (
// CHECK:         SeqNumber int,
// CHECK:         ProductName varchar(255),
// CHECK:         Person int,
// CHECK:         Amount int,
// CHECK:         PRIMARY KEY (SeqNumber),
// CHECK:         FOREIGN KEY (Person) REFERENCES Customer(ID)
// CHECK:       );
class Orders <string product, Customer customer,
              int amount,
              int year, int month, int day> : Table {
  int SeqNumber = PrimaryKey;
  string ProductName = product;
  Customer Person = customer;
  int Amount = amount;
}

