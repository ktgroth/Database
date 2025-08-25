
package sqlparser

import "unsafe"

//export SQLParser
func SQLParser(queryPtr unsafe.Pointer, length int) {
	queryBytes := (*[1 << 30]byte)(queryPtr)[:length:length]
	query := string(queryBytes)

	println("Parsed query:", query)
}

