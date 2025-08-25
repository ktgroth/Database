
package sqlparser

import (
	"fmt"
	"strings"
	"unicode"
)

type TokenType	string

const (
	SELECT	TokenType = "SELECT"
	FROM	TokenType = "FROM"
	WHERE	TokenType = "WHERE"

	COMMA	TokenType = ","
	STAR	TokenType = "*"
	EQ		TokenType = "="

	ID		TokenType = "ID"
	STRING	TokenType = "STRING"
	NUMBER	TokenType = "NUMBER"

	EOF		TokenType = "EOF"
)

type Token struct {
	Type	TokenType
	Value	string
}

type Lexer struct {
	input	[]rune
	pos		int
}

func NewLexer(input string) *Lexer {
	return &Lexer{
		input: []rune(strings.TrimSpace(input)),
	}
}

func (l *Lexer) NextToken() Token {
	for l.pos < len(l.input) {
		ch := l.input[l.pos]

		if unicode.IsSpace(ch) {
			l.pos++
			continue
		}

		switch ch {
		case ',':
			l.pos++
			return Token {
				Type: COMMA,
				Value: ",",
			}

		case '*':
			l.pos++
			return Token {
				Type: STAR,
				Value: "*",
			}

		case '=':
			l.pos++
			return Token {
				Type: EQ,
				Value: "=",
			}

		case '\'', '"':
			return l.readString(ch)
		}

		if unicode.IsLetter(ch) {
			return l.readId()
		}

		if unicode.IsDigit(ch) {
			return l.readNumber()
		}

		l.pos++
	}

	return Token {
		Type: EOF,
		Value: "EOF",
	}
}

func (l *Lexer) readId() Token {
	start := l.pos
	for l.pos < len(l.input) && (unicode.IsLetter(l.input[l.pos]) || unicode.IsDigit(l.input[l.pos])) {
		l.pos++
	}

	word := strings.ToUpper(string(l.input[start:l.pos]))
	switch word {
	case "SELECT":
		return Token {
			Type: SELECT,
			Value: word,
		}

	case "FROM":
		return Token {
			Type: FROM,
			Value: word,
		}

	case "WHERE":
		return Token {
			Type: WHERE,
			Value: word,
		}
	}

	return Token {
		Type: ID,
		Value: word,
	}
}

func (l *Lexer) readString(quote rune) Token {
	l.pos++
	start := l.pos
	for l.pos < len(l.input) && l.input[l.pos] != quote {
		l.pos++
	}

	str := string(l.input[start:l.pos])
	l.pos++
	return Token {
		Type: STRING,
		Value: str,
	}
}

func (l *Lexer) readNumber() Token {
	start := l.pos
	for l.pos < len(l.input) && unicode.IsDigit(l.input[l.pos]) {
		l.pos++
	}

	return Token {
		Type: NUMBER,
		Value: string(l.input[start:l.pos]),
	}
}

func LexAll(query string) {
	l := NewLexer(query)
	for tok := l.NextToken(); tok.Type != EOF; tok = l.NextToken() {
		fmt.Printf("%+v\n", tok)
	}
}

