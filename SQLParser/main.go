
package main

/*
#include <stdlib.h>

typedef struct {
	char *left;
	char *op;
	char *right;
} Condition;

typedef struct {
	char **columns;
	int ncolumns;
	char *table;
	Condition *where;
} SelectStmt;
*/
import "C"
import "fmt"
import "unsafe"
import "SQLParser/sqlparser"


func cString(s string) *C.char {
	return C.CString(s)
}

//export SQLParser
func SQLParser(query *C.char) *C.SelectStmt {
	stmt := sqlparser.NewParser(C.GoString(query)).ParseSelect()
	fmt.Printf("Columns: %+v\nTable: %+v\nWhere%+v\n",
			stmt.Columns, stmt.Table, stmt.Where)

	cstmt := (*C.SelectStmt)(C.malloc(C.size_t(unsafe.Sizeof(C.SelectStmt {}))))

	cstmt.table = cString(stmt.Table)

	n := len(stmt.Columns)
	cstmt.ncolumns = C.int(n)
	cstmt.columns = (**C.char)(C.malloc(C.size_t(n) * C.size_t(unsafe.Sizeof(uintptr(0)))))
	colPtr := (*[1 << 30]*C.char)(unsafe.Pointer(cstmt.columns))
	for i, col := range stmt.Columns {
		colPtr[i] = cString(col)
	}

	if stmt.Where != nil {
		cstmt.where = (*C.Condition)(C.malloc(C.size_t(unsafe.Sizeof(C.Condition {}))))
		cstmt.where.left = cString(stmt.Where.Left)
		cstmt.where.op = cString(stmt.Where.Op)
		cstmt.where.right = cString(stmt.Where.Right)
	} else {
		cstmt.where = nil;
	}

	return cstmt
}

func main() {}

