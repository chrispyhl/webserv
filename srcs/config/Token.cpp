#include "Token.hpp"
#include <stdexcept>

Token::Token(TokenType type, const std::string &text, std::size_t line)
	: _type(type), _text(text), _line(line)
{
}

TokenType Token::tokenType() const
{
	return (_type);
}

std::string Token::tokenTypeString() const
{
	switch (_type)
	{
		case (TokenType::WORD):
			return ("WORD");
		case (TokenType::LBRACE):
			return ("LBRACE");
		case (TokenType::RBRACE):
			return ("RBRACE");
		case (TokenType::SEMICOLON):
			return ("SEMICOLON");
		case (TokenType::END):
			return ("END");
	}
	throw std::logic_error("ERROR/DEBUG: tokenTypeString() revealed _type that somehow isn't in TokenType definition. Someone did some static_casting.");
}

const std::string &Token::text() const
{
	return (_text);
}

std::size_t Token::line() const
{
	return (_line);
}
