
package sqlparser

import "fmt"

type Parser struct {
	lexer	*Lexer
	cur		Token
}

func NewParser(input string) *Parser {
	l := NewLexer(input)
	return &Parser {
		lexer: l,
		cur: l.NextToken(),
	}
}

func (p *Parser) eat(tt TokenType) {
	if p.cur.Type != tt {
		panic(fmt.Sprintf("Expected %s, got %s", tt, p.cur.Type))
	}

	p.cur = p.lexer.NextToken()
}

func (p *Parser) ParseSelect() *SelectStmt {
	stmt := &SelectStmt {}
	p.eat(SELECT)

	for {
		if p.cur.Type == STAR {
			stmt.Columns = append(stmt.Columns, "*")
			p.eat(STAR)
			break
		} else if p.cur.Type == ID {
			stmt.Columns = append(stmt.Columns, p.cur.Value)
			p.eat(ID)
		}

		if p.cur.Type == COMMA {
			p.eat(COMMA)
			continue
		}

		break
	}

	p.eat(FROM)
	stmt.Table = p.cur.Value
	p.eat(ID)

	if (p.cur.Type == WHERE) {
		p.eat(WHERE)
		left := p.cur.Value
		p.eat(p.cur.Type)
		op := p.cur.Value
		p.eat(EQ)
		right := p.cur.Value
		p.eat(p.cur.Type)
		stmt.Where = &Condition {
			Left: left,
			Op: op,
			Right: right,
		}
	}

	return stmt
}

