#pragma once

#include <string>
#include <cstddef>

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
		// TODO: make 1000% sure that orthodox canonical form is not requried
		// only a parameterized constructor since an incomplete token object is useless
		Token(TokenType type, const std::string &text, std::size_t line);
		TokenType tokenType() const;
		// this helper constructs the std::string version of the TokenType and returns it
		// afterwards, the local version gets destroyed --> can't return by reference!
		std::string tokenTypeString() const;
		const std::string &text() const;
		std::size_t line() const;

	private:
		TokenType _type;
		std::string _text;
		std::size_t _line;
};
