// RUN: sqlgen %s | %FileCheck %s

include "Query.td"

// CHECK: SELECT * FROM Customer;
def : Query<"Customer">;

// CHECK: SELECT Person, Amount FROM Orders;
def : Query<"Orders", (fields "Person", "Amount")>;

// CHECK:      SELECT Affiliation FROM Customer
// CHECK-NEXT: WHERE Name = "Mary Blackburn";
def : Query<"Customer", (fields "Affiliation"),
            (eq "Name", "Mary Blackburn":$str)>;

// CHECK:      SELECT ProductName FROM Orders
// CHECK-NEXT: WHERE Amount > 8;
def : Query<"Orders", (fields "ProductName"),
            (gt "Amount", 8)>;

// CHECK:      SELECT ProductName, Person FROM Orders
// CHECK-NEXT: WHERE Amount > 8 AND Person <> 1
// CHECK-NEXT: ORDER BY ProductName;
def : Query<"Orders", (fields "ProductName":$name, "Person"),
            (and (gt "Amount", 8), (ne "Person", 1))> {
  let OrderedBy = ["$name"];
}

