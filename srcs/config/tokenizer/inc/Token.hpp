#pragma once

#include <cstddef>
#include <string>

enum class TokenType
{
	WORD,
	LBRACE,
	RBRACE,
	SEMICOLON,
	END
};
// END gets _text "end of file" for comprehensive error message output

class Token
{
	public:
		Token(TokenType type, const std::string &text, std::size_t line);
		TokenType tokenType() const;
		std::string tokenTypeString() const;
		const std::string &text() const;
		std::size_t line() const;

	private:
		TokenType _type;
		std::string _text;
		std::size_t _line;
};
