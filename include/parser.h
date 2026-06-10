/**
 * Mini SQL Compiler for Query Validation
 * =======================================
 * File: parser.h
 * Description: Syntax Analyzer Header
 * Team Member: Member 2
 *
 * The Syntax Analyzer (Parser) is the second phase of the compiler.
 * It takes the token stream from the Lexer and verifies that the tokens
 * follow the grammatical rules of SQL.
 *
 * THEORY:
 * -------
 * Syntax Analysis checks if the sequence of tokens conforms to the
 * Context-Free Grammar (CFG) of the language. We use a RECURSIVE DESCENT
 * PARSER which is a top-down parsing technique.
 *
 * Our SQL Grammar:
 * ----------------
 * <query>        ::= SELECT <column_list> FROM <table_name> [<where_clause>] ;
 * <column_list>  ::= * | <column_name> { , <column_name> }*
 * <column_name>  ::= IDENTIFIER
 * <table_name>   ::= IDENTIFIER
 * <where_clause> ::= WHERE <condition>
 * <condition>    ::= <column_name> <rel_op> <value>
 * <rel_op>       ::= = | < | >
 * <value>        ::= IDENTIFIER | NUMBER | STRING_LITERAL
 *
 * Responsibilities:
 * - Validate token sequence against SQL grammar
 * - Build Parse Tree (Abstract Syntax Tree)
 * - Report syntax errors with meaningful messages
 */

#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include <string>
#include <vector>

namespace MiniSQL {

class Parser {
private:
  std::vector<Token> tokens;         // Input token stream
  std::vector<CompilerError> errors; // Syntax errors
  size_t current;                    // Current token index

  // Token navigation
  Token peek() const;
  Token previous() const;
  Token advance();
  bool isAtEnd() const;
  bool check(TokenType type) const;
  bool match(TokenType type);

  // Error handling
  void error(const std::string &message);
  void synchronize();

  // Grammar rule methods (Recursive Descent)
  ParseTree parseQuery();
  ParseTree parseSelectClause();
  ParseTree parseColumnList();
  ParseTree parseFromClause();
  ParseTree parseWhereClause();
  ParseTree parseExpression();
  ParseTree parseLogicalOr();
  ParseTree parseLogicalAnd();
  ParseTree parsePrimary();
  ParseTree parseCondition();
  ParseTree parseInsert();
  ParseTree parseUpdate();
  ParseTree parseDelete();
  ParseTree parseValueList();

  // Utility
  Token consume(TokenType type, const std::string &message);

public:
  /**
   * Constructor
   * @param tokens Token stream from lexer
   */
  explicit Parser(const std::vector<Token> &tokens);

  /**
   * Parse the token stream and build parse tree
   * @return Parse tree (nullptr if parsing failed)
   */
  ParseTree parse();

  /**
   * Check if parsing had errors
   */
  bool hasErrors() const;

  /**
   * Get all syntax errors
   */
  std::vector<CompilerError> getErrors() const;

  /**
   * Print parse tree to console
   */
  static void printParseTree(const ParseTree &node, int indent = 0);
};

} // namespace MiniSQL

#endif // PARSER_H
