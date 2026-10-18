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
		// members
		const std::string _str;
		std::size_t _pos;
		std::size_t _line;
		// helpers
		Token tokenizeWord();
		void handleBackslash(std::string &text, std::size_t &quoted_newline_c);
		void checkClosingQuote(std::size_t line_temp);
};
