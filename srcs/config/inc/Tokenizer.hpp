#pragma once

#include "Token.hpp"

#include <string>
#include <vector>

class Tokenizer
{
	public:
		Tokenizer(const std::string &str);
		std::vector<Token> tokenize();
		Token tokenize_word();
		void handle_backslash(std::string &text, std::size_t &quoted_newline_c);

	private:
		const std::string _str;
		std::size_t _pos;
		std::size_t _line;
};
