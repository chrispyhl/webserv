#include "Parser.hpp"
#include "ConfigError.hpp"

Parser::Parser(const std::vector<Token> &tokens) : _tokens(tokens), _idx(0)
{
}

std::vector<Directive> Parser::parse()
{
	std::vector<Directive> directives;
	while (peek().tokenType() != TokenType::END)
		directives.push_back(parseDirective());
	return (directives);
}

const Token &Parser::peek() const
{
	return (_tokens[_idx]);
}

const Token &Parser::advance()
{
	if (peek().tokenType() != TokenType::END)
	{
		std::size_t idx_temp = _idx;
		_idx++;
		return (_tokens[idx_temp]);
	}
	return (_tokens[_idx]);
}

const Token &Parser::expect(TokenType type)
{
	if (type != peek().tokenType())
		throw ConfigError("unexpected \"" + peek().text() + "\"", peek().line());
	return (advance());
}

Directive Parser::parseDirective()
{
	Directive directive { };

	Token token = expect(TokenType::WORD);
	directive.name = token.text();
	directive.line = token.line();

	while (peek().tokenType() == TokenType::WORD)
		directive.args.push_back(advance().text());

	// simple directive (no children)
	if (peek().tokenType() == TokenType::SEMICOLON)
		advance();
	// complex directive (with children)
	else if (peek().tokenType() == TokenType::LBRACE)
	{
		// consume {
		advance();
		directive.children.emplace();
		while (peek().tokenType() != TokenType::RBRACE)
		{
			if (peek().tokenType() == TokenType::END)
				throw ConfigError("unexpected end of file, expecting \"}\"", peek().line());
			directive.children.value().push_back(parseDirective());
		}
		// consume }
		advance();
	}
	else
	{
		// special treatment for unexpected END to match nginx's message
		if (peek().tokenType() == TokenType::END)
			throw ConfigError("unexpected end of file, expecting \";\" or \"}\"", peek().line());
		else
			throw ConfigError("unexpected \"" + peek().text() + "\"", peek().line());
	}
	return (directive);
}
