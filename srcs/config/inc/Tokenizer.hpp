#pragma once

#include "Token.hpp"

#include <string>
#include <vector>

class Tokenizer
{
	public:
		Tokenizer(const std::string &str);
		std::vector<Token> tokenize();

	private:
		// helpers
		Token tokenizeWord();
		std::string unquotedWord();
		std::string quotedWord();
		void handleBackslash(std::string &text);
		void checkClosingQuote();
		void advance();
		// members
		const std::string _str;
		std::size_t _pos;
		std::size_t _line;
};
