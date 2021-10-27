def all;
def fields;
def none;

def eq;
def ne;
def gt;
def ge;
def and;
def or;

class Query <string table, dag query_fields = (all), dag condition = (none)> {
  string TableName = table;
  dag Fields = query_fields;
  dag WhereClause = condition;
  list<string> OrderedBy = [];
}

