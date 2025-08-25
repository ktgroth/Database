
package sqlparser

type SelectStmt struct {
	Columns	[]string
	Table	string
	Where	*Condition
}

type Condition struct {
	Left	string
	Op		string
	Right	string
}

