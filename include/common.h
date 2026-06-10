/**
 * Mini SQL Compiler for Query Validation
 * =======================================
 * File: common.h
 * Description: Common type definitions and shared structures
 *
 * This header contains token types, error types, and common structures
 * used across all compiler phases.
 */

#ifndef COMMON_H
#define COMMON_H

#include <memory>
#include <string>
#include <vector>

namespace MiniSQL {

// ============================================================================
// TOKEN TYPES - Used by Lexical Analyzer (Member 1)
// ============================================================================
enum class TokenType {
  // Keywords
  KEYWORD_SELECT,
  KEYWORD_FROM,
  KEYWORD_WHERE,
  KEYWORD_AND,
  KEYWORD_OR,
  KEYWORD_INSERT,
  KEYWORD_INTO,
  KEYWORD_VALUES,
  KEYWORD_UPDATE,
  KEYWORD_SET,
  KEYWORD_DELETE,
  KEYWORD_CREATE,
  KEYWORD_TABLE,

  // Identifiers and Literals
  IDENTIFIER,     // Table names, column names
  NUMBER,         // Numeric constants (e.g., 25, 100.5)
  STRING_LITERAL, // String constants (e.g., 'John')

  // Operators
  OP_EQUALS,         // =
  OP_NOT_EQUALS,     // !=
  OP_LESS_THAN,      // <
  OP_LESS_EQUALS,    // <=
  OP_GREATER_THAN,   // >
  OP_GREATER_EQUALS, // >=
  OP_COMMA,          // ,
  OP_STAR,           // *
  OP_SEMICOLON,      // ;
  OP_LPAREN,         // (
  OP_RPAREN,         // )

  // Special
  END_OF_INPUT, // End of query
  UNKNOWN       // Unknown/Invalid token
};

// Convert TokenType to string for display
inline std::string tokenTypeToString(TokenType type) {
  switch (type) {
  case TokenType::KEYWORD_SELECT:
    return "KEYWORD_SELECT";
  case TokenType::KEYWORD_FROM:
    return "KEYWORD_FROM";
  case TokenType::KEYWORD_WHERE:
    return "KEYWORD_WHERE";
  case TokenType::KEYWORD_AND:
    return "KEYWORD_AND";
  case TokenType::KEYWORD_OR:
    return "KEYWORD_OR";
  case TokenType::KEYWORD_INSERT:
    return "KEYWORD_INSERT";
  case TokenType::KEYWORD_INTO:
    return "KEYWORD_INTO";
  case TokenType::KEYWORD_VALUES:
    return "KEYWORD_VALUES";
  case TokenType::KEYWORD_UPDATE:
    return "KEYWORD_UPDATE";
  case TokenType::KEYWORD_SET:
    return "KEYWORD_SET";
  case TokenType::KEYWORD_DELETE:
    return "KEYWORD_DELETE";
  case TokenType::KEYWORD_CREATE:
    return "KEYWORD_CREATE";
  case TokenType::KEYWORD_TABLE:
    return "KEYWORD_TABLE";
  case TokenType::IDENTIFIER:
    return "IDENTIFIER";
  case TokenType::NUMBER:
    return "NUMBER";
  case TokenType::STRING_LITERAL:
    return "STRING_LITERAL";
  case TokenType::OP_EQUALS:
    return "OP_EQUALS";
  case TokenType::OP_NOT_EQUALS:
    return "OP_NOT_EQUALS";
  case TokenType::OP_LESS_THAN:
    return "OP_LESS_THAN";
  case TokenType::OP_LESS_EQUALS:
    return "OP_LESS_EQUALS";
  case TokenType::OP_GREATER_THAN:
    return "OP_GREATER_THAN";
  case TokenType::OP_GREATER_EQUALS:
    return "OP_GREATER_EQUALS";
  case TokenType::OP_COMMA:
    return "OP_COMMA";
  case TokenType::OP_STAR:
    return "OP_STAR";
  case TokenType::OP_SEMICOLON:
    return "OP_SEMICOLON";
  case TokenType::OP_LPAREN:
    return "OP_LPAREN";
  case TokenType::OP_RPAREN:
    return "OP_RPAREN";
  case TokenType::END_OF_INPUT:
    return "END_OF_INPUT";
  case TokenType::UNKNOWN:
    return "UNKNOWN";
  default:
    return "UNDEFINED";
  }
}

// ============================================================================
// TOKEN STRUCTURE - Represents a single token
// ============================================================================
struct Token {
  TokenType type;
  std::string value; // The actual text of the token
  int line;          // Line number (1-indexed)
  int column;        // Column number (1-indexed)

  Token(TokenType t, const std::string &v, int l, int c)
      : type(t), value(v), line(l), column(c) {}

  // Display token information
  std::string toString() const {
    return "<" + tokenTypeToString(type) + ", \"" + value +
           "\", Line:" + std::to_string(line) +
           ", Col:" + std::to_string(column) + ">";
  }
};

// ============================================================================
// ERROR TYPES - Used by Error Handler (Member 4)
// ============================================================================
enum class ErrorType {
  LEXICAL_ERROR, // Invalid characters, malformed tokens
  SYNTAX_ERROR,  // Grammar violations
  SEMANTIC_ERROR // Logical errors (undefined columns, etc.)
};

inline std::string errorTypeToString(ErrorType type) {
  switch (type) {
  case ErrorType::LEXICAL_ERROR:
    return "Lexical Error";
  case ErrorType::SYNTAX_ERROR:
    return "Syntax Error";
  case ErrorType::SEMANTIC_ERROR:
    return "Semantic Error";
  default:
    return "Unknown Error";
  }
}

// ============================================================================
// ERROR STRUCTURE - Represents a compilation error
// ============================================================================
struct CompilerError {
  ErrorType type;
  std::string message;
  int line;
  int column;

  CompilerError(ErrorType t, const std::string &msg, int l, int c)
      : type(t), message(msg), line(l), column(c) {}

  std::string toString() const {
    return errorTypeToString(type) + " at Line " + std::to_string(line) +
           ", Column " + std::to_string(column) + ": " + message;
  }
};

// ============================================================================
// PARSE TREE NODE - Used by Parser (Member 2)
// ============================================================================
enum class NodeType {
  QUERY,
  SELECT_CLAUSE,
  COLUMN_LIST,
  COLUMN,
  FROM_CLAUSE,
  TABLE_NAME,
  WHERE_CLAUSE,
  CONDITION,
  OPERATOR,
  VALUE,
  INSERT_QUERY,
  UPDATE_QUERY,
  DELETE_QUERY,
  VALUE_LIST,
  ASSIGNMENT,
  SET_CLAUSE,
  AND_EXPR,
  OR_EXPR
};

inline std::string nodeTypeToString(NodeType type) {
  switch (type) {
  case NodeType::QUERY:
    return "QUERY";
  case NodeType::SELECT_CLAUSE:
    return "SELECT_CLAUSE";
  case NodeType::COLUMN_LIST:
    return "COLUMN_LIST";
  case NodeType::COLUMN:
    return "COLUMN";
  case NodeType::FROM_CLAUSE:
    return "FROM_CLAUSE";
  case NodeType::TABLE_NAME:
    return "TABLE_NAME";
  case NodeType::WHERE_CLAUSE:
    return "WHERE_CLAUSE";
  case NodeType::CONDITION:
    return "CONDITION";
  case NodeType::OPERATOR:
    return "OPERATOR";
  case NodeType::VALUE:
    return "VALUE";
  case NodeType::INSERT_QUERY:
    return "INSERT_QUERY";
  case NodeType::UPDATE_QUERY:
    return "UPDATE_QUERY";
  case NodeType::DELETE_QUERY:
    return "DELETE_QUERY";
  case NodeType::VALUE_LIST:
    return "VALUE_LIST";
  case NodeType::ASSIGNMENT:
    return "ASSIGNMENT";
  case NodeType::SET_CLAUSE:
    return "SET_CLAUSE";
  case NodeType::AND_EXPR:
    return "AND_EXPR";
  case NodeType::OR_EXPR:
    return "OR_EXPR";
  default:
    return "UNKNOWN_NODE";
  }
}

struct ParseTreeNode {
  NodeType type;
  std::string value;
  std::vector<std::shared_ptr<ParseTreeNode>> children;

  ParseTreeNode(NodeType t, const std::string &v = "") : type(t), value(v) {}

  void addChild(std::shared_ptr<ParseTreeNode> child) {
    children.push_back(child);
  }
};

using ParseTree = std::shared_ptr<ParseTreeNode>;

} // namespace MiniSQL

#endif // COMMON_H
