#pragma once

#include <vector>

#include "Token.hpp"
#include "Directive.hpp"

class Parser
{
	public:
		Parser(const std::vector<Token> &tokens);
		std::vector<Directive> parse();

	private:
		// helper
		const Token &peek() const;
		const Token &advance();
		const Token &expect(TokenType type);
		Directive parseDirective();
		// member
		const std::vector<Token> _tokens;
		std::size_t _idx;
};
