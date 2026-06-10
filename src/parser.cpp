/**
 * Mini SQL Compiler for Query Validation
 * =======================================
 * File: parser.cpp
 * Description: Syntax Analyzer Implementation
 * Team Member: Member 2
 *
 * THEORY:
 * -------
 * This file implements a RECURSIVE DESCENT PARSER for SQL queries.
 *
 * Recursive Descent Parsing:
 * - A top-down parsing technique
 * - Each non-terminal in the grammar has a corresponding parsing function
 * - The parser starts from the start symbol and recursively matches rules
 * - It's called "predictive" because we can determine which rule to apply
 *   by looking at the current token (lookahead)
 *
 * Parse Tree Construction:
 * - As we match grammar rules, we build a tree structure
 * - Each node represents a grammar production
 * - Leaf nodes contain actual token values
 * - The tree captures the syntactic structure of the query
 *
 * Error Recovery:
 * - When a syntax error is detected, we report it and try to continue
 * - "Synchronization" helps the parser recover to a known state
 * - This allows reporting multiple errors in one pass
 */

#include "../include/parser.h"
#include <iostream>

namespace MiniSQL {

// Constructor
Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens), current(0) {}

// ============================================================================
// MAIN PARSING METHOD
// ============================================================================
ParseTree Parser::parse() {
  std::cout << "\n========================================\n";
  std::cout << "   PHASE 2: SYNTAX ANALYSIS\n";
  std::cout << "========================================\n";

  ParseTree tree = parseQuery();

  if (errors.empty() && tree != nullptr) {
    std::cout << "Syntax Analysis: SUCCESS\n";
    std::cout << "Parse Tree constructed successfully.\n";
  } else {
    std::cout << "Syntax Analysis: FAILED with " << errors.size()
              << " error(s)\n";
  }

  return tree;
}

// ============================================================================
// GRAMMAR RULE: <query> ::= SELECT <column_list> FROM <table_name>
// [<where_clause>] ;
// ============================================================================
ParseTree Parser::parseQuery() {
  // Dispatch based on first keyword
  if (check(TokenType::KEYWORD_INSERT)) {
    return parseInsert();
  } else if (check(TokenType::KEYWORD_UPDATE)) {
    return parseUpdate();
  } else if (check(TokenType::KEYWORD_DELETE)) {
    return parseDelete();
  }

  // Default: SELECT query
  auto queryNode = std::make_shared<ParseTreeNode>(NodeType::QUERY);

  // Parse SELECT clause
  auto selectClause = parseSelectClause();
  if (selectClause) {
    queryNode->addChild(selectClause);
  } else {
    return nullptr;
  }

  // Parse FROM clause
  auto fromClause = parseFromClause();
  if (fromClause) {
    queryNode->addChild(fromClause);
  } else {
    return nullptr;
  }

  // Parse optional WHERE clause
  if (check(TokenType::KEYWORD_WHERE)) {
    auto whereClause = parseWhereClause();
    if (whereClause) {
      queryNode->addChild(whereClause);
    } else {
      return nullptr;
    }
  }

  // Expect semicolon at the end
  consume(TokenType::OP_SEMICOLON, "Expected ';' at end of query");

  return queryNode;
}

// ============================================================================
// GRAMMAR RULE: SELECT <column_list>
// ============================================================================
ParseTree Parser::parseSelectClause() {
  // Expect SELECT keyword
  if (!match(TokenType::KEYWORD_SELECT)) {
    error("Expected 'SELECT' keyword at beginning of query");
    return nullptr;
  }

  auto selectNode =
      std::make_shared<ParseTreeNode>(NodeType::SELECT_CLAUSE, "SELECT");

  // Parse column list
  auto columnList = parseColumnList();
  if (columnList) {
    selectNode->addChild(columnList);
  } else {
    return nullptr;
  }

  return selectNode;
}

// ============================================================================
// GRAMMAR RULE: <column_list> ::= * | <column_name> { , <column_name> }*
// ============================================================================
ParseTree Parser::parseColumnList() {
  auto columnListNode = std::make_shared<ParseTreeNode>(NodeType::COLUMN_LIST);

  // Check for SELECT *
  if (match(TokenType::OP_STAR)) {
    auto starColumn = std::make_shared<ParseTreeNode>(NodeType::COLUMN, "*");
    columnListNode->addChild(starColumn);
    return columnListNode;
  }

  // Parse first column name
  if (!check(TokenType::IDENTIFIER)) {
    error("Expected column name or '*' after SELECT");
    return nullptr;
  }

  // Add first column
  Token colToken = advance();
  auto column =
      std::make_shared<ParseTreeNode>(NodeType::COLUMN, colToken.value);
  columnListNode->addChild(column);

  // Parse additional columns separated by commas
  while (match(TokenType::OP_COMMA)) {
    if (!check(TokenType::IDENTIFIER)) {
      error("Expected column name after ','");
      return nullptr;
    }
    Token nextCol = advance();
    auto nextColumn =
        std::make_shared<ParseTreeNode>(NodeType::COLUMN, nextCol.value);
    columnListNode->addChild(nextColumn);
  }

  return columnListNode;
}

// ============================================================================
// GRAMMAR RULE: FROM <table_name>
// ============================================================================
ParseTree Parser::parseFromClause() {
  if (!match(TokenType::KEYWORD_FROM)) {
    error("Expected 'FROM' keyword");
    return nullptr;
  }

  auto fromNode =
      std::make_shared<ParseTreeNode>(NodeType::FROM_CLAUSE, "FROM");

  if (!check(TokenType::IDENTIFIER)) {
    error("Expected table name after 'FROM'");
    return nullptr;
  }

  Token tableToken = advance();
  auto tableName =
      std::make_shared<ParseTreeNode>(NodeType::TABLE_NAME, tableToken.value);
  fromNode->addChild(tableName);

  return fromNode;
}

// ============================================================================
// GRAMMAR RULE: WHERE <condition>
// ============================================================================
ParseTree Parser::parseWhereClause() {
  if (!match(TokenType::KEYWORD_WHERE)) {
    return nullptr;
  }

  auto whereNode =
      std::make_shared<ParseTreeNode>(NodeType::WHERE_CLAUSE, "WHERE");

  auto expr = parseExpression();
  if (expr) {
    whereNode->addChild(expr);
  } else {
    return nullptr;
  }

  return whereNode;
}

// ============================================================================
// GRAMMAR RULE: <expression> ::= <logical_or>
// ============================================================================
ParseTree Parser::parseExpression() {
  return parseLogicalOr();
}

// ============================================================================
// GRAMMAR RULE: <logical_or> ::= <logical_and> { OR <logical_and> }*
// ============================================================================
ParseTree Parser::parseLogicalOr() {
  auto expr = parseLogicalAnd();
  if (!expr) return nullptr;

  while (match(TokenType::KEYWORD_OR)) {
    auto orNode = std::make_shared<ParseTreeNode>(NodeType::OR_EXPR, "OR");
    auto right = parseLogicalAnd();
    if (!right) return nullptr;
    orNode->addChild(expr);
    orNode->addChild(right);
    expr = orNode;
  }

  return expr;
}

// ============================================================================
// GRAMMAR RULE: <logical_and> ::= <primary> { AND <primary> }*
// ============================================================================
ParseTree Parser::parseLogicalAnd() {
  auto expr = parsePrimary();
  if (!expr) return nullptr;

  while (match(TokenType::KEYWORD_AND)) {
    auto andNode = std::make_shared<ParseTreeNode>(NodeType::AND_EXPR, "AND");
    auto right = parsePrimary();
    if (!right) return nullptr;
    andNode->addChild(expr);
    andNode->addChild(right);
    expr = andNode;
  }

  return expr;
}

// ============================================================================
// GRAMMAR RULE: <primary> ::= <condition> | ( <expression> )
// ============================================================================
ParseTree Parser::parsePrimary() {
  if (match(TokenType::OP_LPAREN)) {
    auto expr = parseExpression();
    if (!expr) return nullptr;
    consume(TokenType::OP_RPAREN, "Expected ')' after expression");
    return expr;
  }
  return parseCondition();
}

// ============================================================================
// GRAMMAR RULE: <condition> ::= <column_name> <rel_op> <value>
// ============================================================================
ParseTree Parser::parseCondition() {
  auto conditionNode = std::make_shared<ParseTreeNode>(NodeType::CONDITION);

  // Left operand: column name
  if (!check(TokenType::IDENTIFIER)) {
    error("Expected column name in WHERE condition");
    return nullptr;
  }
  Token colToken = advance();
  auto column =
      std::make_shared<ParseTreeNode>(NodeType::COLUMN, colToken.value);
  conditionNode->addChild(column);

  // Relational operator
  if (!check(TokenType::OP_EQUALS) && !check(TokenType::OP_NOT_EQUALS) &&
      !check(TokenType::OP_LESS_THAN) && !check(TokenType::OP_LESS_EQUALS) &&
      !check(TokenType::OP_GREATER_THAN) &&
      !check(TokenType::OP_GREATER_EQUALS)) {
    error("Expected relational operator (=, !=, <, <=, >, >=) in condition");
    return nullptr;
  }
  Token opToken = advance();
  auto opNode =
      std::make_shared<ParseTreeNode>(NodeType::OPERATOR, opToken.value);
  conditionNode->addChild(opNode);

  // Right operand: value (identifier, number, or string)
  if (!check(TokenType::IDENTIFIER) && !check(TokenType::NUMBER) &&
      !check(TokenType::STRING_LITERAL)) {
    error("Expected value (identifier, number, or string) in condition");
    return nullptr;
  }
  Token valueToken = advance();
  auto valueNode =
      std::make_shared<ParseTreeNode>(NodeType::VALUE, valueToken.value);
  conditionNode->addChild(valueNode);

  return conditionNode;
}

// ============================================================================
// TOKEN NAVIGATION HELPERS
// ============================================================================

Token Parser::peek() const { return tokens[current]; }

Token Parser::previous() const { return tokens[current - 1]; }

Token Parser::advance() {
  if (!isAtEnd())
    current++;
  return previous();
}

bool Parser::isAtEnd() const { return peek().type == TokenType::END_OF_INPUT; }

bool Parser::check(TokenType type) const {
  if (isAtEnd())
    return false;
  return peek().type == type;
}

bool Parser::match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }
  return false;
}

Token Parser::consume(TokenType type, const std::string &message) {
  if (check(type))
    return advance();
  error(message);
  return Token(TokenType::UNKNOWN, "", peek().line, peek().column);
}

// ============================================================================
// ERROR HANDLING
// ============================================================================

void Parser::error(const std::string &message) {
  Token token = peek();
  errors.push_back(CompilerError(ErrorType::SYNTAX_ERROR,
                                 message + " (found '" + token.value + "')",
                                 token.line, token.column));
}

void Parser::synchronize() {
  advance();

  while (!isAtEnd()) {
    if (previous().type == TokenType::OP_SEMICOLON)
      return;

    switch (peek().type) {
    case TokenType::KEYWORD_SELECT:
      return;
    default:
      break;
    }
    advance();
  }
}

// ============================================================================
// GRAMMAR RULE: INSERT INTO <table> (<columns>) VALUES (<values>);
// ============================================================================
ParseTree Parser::parseInsert() {
  auto insertNode = std::make_shared<ParseTreeNode>(NodeType::INSERT_QUERY);

  // Consume INSERT
  if (!match(TokenType::KEYWORD_INSERT)) {
    error("Expected 'INSERT' keyword");
    return nullptr;
  }

  // Consume INTO
  if (!match(TokenType::KEYWORD_INTO)) {
    error("Expected 'INTO' after 'INSERT'");
    return nullptr;
  }

  // Table name
  if (!check(TokenType::IDENTIFIER)) {
    error("Expected table name after 'INSERT INTO'");
    return nullptr;
  }
  Token tableToken = advance();
  auto tableName =
      std::make_shared<ParseTreeNode>(NodeType::TABLE_NAME, tableToken.value);
  insertNode->addChild(tableName);

  // Column list in parentheses
  consume(TokenType::OP_LPAREN, "Expected '(' after table name");

  auto columnList = std::make_shared<ParseTreeNode>(NodeType::COLUMN_LIST);
  if (!check(TokenType::IDENTIFIER)) {
    error("Expected column name in column list");
    return nullptr;
  }

  Token colToken = advance();
  columnList->addChild(
      std::make_shared<ParseTreeNode>(NodeType::COLUMN, colToken.value));

  while (match(TokenType::OP_COMMA)) {
    if (!check(TokenType::IDENTIFIER)) {
      error("Expected column name after ','");
      return nullptr;
    }
    Token nextCol = advance();
    columnList->addChild(
        std::make_shared<ParseTreeNode>(NodeType::COLUMN, nextCol.value));
  }

  consume(TokenType::OP_RPAREN, "Expected ')' after column list");
  insertNode->addChild(columnList);

  // VALUES keyword
  if (!match(TokenType::KEYWORD_VALUES)) {
    error("Expected 'VALUES' keyword");
    return nullptr;
  }

  // Value list in parentheses
  consume(TokenType::OP_LPAREN, "Expected '(' after VALUES");
  auto valueList = parseValueList();
  if (valueList) {
    insertNode->addChild(valueList);
  } else {
    return nullptr;
  }
  consume(TokenType::OP_RPAREN, "Expected ')' after value list");

  // Semicolon
  consume(TokenType::OP_SEMICOLON, "Expected ';' at end of INSERT statement");

  return insertNode;
}

// ============================================================================
// GRAMMAR RULE: UPDATE <table> SET <col> = <val> WHERE <condition>;
// ============================================================================
ParseTree Parser::parseUpdate() {
  auto updateNode = std::make_shared<ParseTreeNode>(NodeType::UPDATE_QUERY);

  // Consume UPDATE
  if (!match(TokenType::KEYWORD_UPDATE)) {
    error("Expected 'UPDATE' keyword");
    return nullptr;
  }

  // Table name
  if (!check(TokenType::IDENTIFIER)) {
    error("Expected table name after 'UPDATE'");
    return nullptr;
  }
  Token tableToken = advance();
  auto tableName =
      std::make_shared<ParseTreeNode>(NodeType::TABLE_NAME, tableToken.value);
  updateNode->addChild(tableName);

  // SET keyword
  if (!match(TokenType::KEYWORD_SET)) {
    error("Expected 'SET' keyword after table name");
    return nullptr;
  }

  // SET clause: col = value
  auto setClause = std::make_shared<ParseTreeNode>(NodeType::SET_CLAUSE, "SET");
  auto assignment = std::make_shared<ParseTreeNode>(NodeType::ASSIGNMENT);

  if (!check(TokenType::IDENTIFIER)) {
    error("Expected column name in SET clause");
    return nullptr;
  }
  Token setCol = advance();
  assignment->addChild(
      std::make_shared<ParseTreeNode>(NodeType::COLUMN, setCol.value));

  consume(TokenType::OP_EQUALS, "Expected '=' in SET clause");

  if (!check(TokenType::IDENTIFIER) && !check(TokenType::NUMBER) &&
      !check(TokenType::STRING_LITERAL)) {
    error("Expected value in SET clause");
    return nullptr;
  }
  Token setVal = advance();
  assignment->addChild(
      std::make_shared<ParseTreeNode>(NodeType::VALUE, setVal.value));

  setClause->addChild(assignment);
  updateNode->addChild(setClause);

  // Optional WHERE clause
  if (check(TokenType::KEYWORD_WHERE)) {
    auto whereClause = parseWhereClause();
    if (whereClause) {
      updateNode->addChild(whereClause);
    } else {
      return nullptr;
    }
  }

  // Semicolon
  consume(TokenType::OP_SEMICOLON, "Expected ';' at end of UPDATE statement");

  return updateNode;
}

// ============================================================================
// GRAMMAR RULE: DELETE FROM <table> [WHERE <condition>];
// ============================================================================
ParseTree Parser::parseDelete() {
  auto deleteNode = std::make_shared<ParseTreeNode>(NodeType::DELETE_QUERY);

  // Consume DELETE
  if (!match(TokenType::KEYWORD_DELETE)) {
    error("Expected 'DELETE' keyword");
    return nullptr;
  }

  // FROM clause
  auto fromClause = parseFromClause();
  if (fromClause) {
    deleteNode->addChild(fromClause);
  } else {
    return nullptr;
  }

  // Optional WHERE clause
  if (check(TokenType::KEYWORD_WHERE)) {
    auto whereClause = parseWhereClause();
    if (whereClause) {
      deleteNode->addChild(whereClause);
    } else {
      return nullptr;
    }
  }

  // Semicolon
  consume(TokenType::OP_SEMICOLON, "Expected ';' at end of DELETE statement");

  return deleteNode;
}

// ============================================================================
// VALUE LIST PARSING - For INSERT VALUES clause
// ============================================================================
ParseTree Parser::parseValueList() {
  auto valueListNode = std::make_shared<ParseTreeNode>(NodeType::VALUE_LIST);

  // First value
  if (!check(TokenType::IDENTIFIER) && !check(TokenType::NUMBER) &&
      !check(TokenType::STRING_LITERAL)) {
    error("Expected value in VALUES list");
    return nullptr;
  }
  Token val = advance();
  valueListNode->addChild(
      std::make_shared<ParseTreeNode>(NodeType::VALUE, val.value));

  // Additional values
  while (match(TokenType::OP_COMMA)) {
    if (!check(TokenType::IDENTIFIER) && !check(TokenType::NUMBER) &&
        !check(TokenType::STRING_LITERAL)) {
      error("Expected value after ',' in VALUES list");
      return nullptr;
    }
    Token nextVal = advance();
    valueListNode->addChild(
        std::make_shared<ParseTreeNode>(NodeType::VALUE, nextVal.value));
  }

  return valueListNode;
}

bool Parser::hasErrors() const { return !errors.empty(); }

std::vector<CompilerError> Parser::getErrors() const { return errors; }

// ============================================================================
// PARSE TREE VISUALIZATION
// ============================================================================
void Parser::printParseTree(const ParseTree &node, int indent) {
  if (!node)
    return;

  // Print indentation
  for (int i = 0; i < indent; i++) {
    std::cout << "  ";
  }

  // Print node
  std::cout << "|-- " << nodeTypeToString(node->type);
  if (!node->value.empty()) {
    std::cout << ": \"" << node->value << "\"";
  }
  std::cout << "\n";

  // Recursively print children
  for (const auto &child : node->children) {
    printParseTree(child, indent + 1);
  }
}

} // namespace MiniSQL
