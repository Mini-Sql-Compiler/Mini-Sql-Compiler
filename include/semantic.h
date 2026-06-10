/**
 * Mini SQL Compiler for Query Validation
 * =======================================
 * File: semantic.h
 * Description: Semantic Analyzer Header
 * Team Member: Member 3
 *
 * The Semantic Analyzer is the third phase of compilation.
 * After syntax analysis confirms the query is grammatically correct,
 * semantic analysis checks for logical/meaningful correctness.
 *
 * THEORY:
 * -------
 * Semantic analysis performs checks that cannot be expressed in a
 * context-free grammar. These include:
 *
 * 1. Name Resolution: Do the referenced tables and columns exist?
 * 2. Type Checking: Are operations type-compatible?
 * 3. Scope Checking: Are identifiers used in valid contexts?
 *
 * For our mini SQL compiler, we validate:
 * - Table names exist in the schema
 * - Column names exist in the specified table
 * - WHERE clause columns are valid
 * - Basic type compatibility in conditions
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "common.h"
#include "symbol_table.h"
#include <string>
#include <vector>

namespace MiniSQL {

class SemanticAnalyzer {
private:
  SymbolTable symbolTable;
  std::vector<CompilerError> errors;

  std::string currentTable;                 // Table being queried
  std::vector<std::string> selectedColumns; // Columns in SELECT

  // Validation methods
  void validateQuery(const ParseTree &node);
  void validateSelectClause(const ParseTree &node);
  void validateFromClause(const ParseTree &node);
  void validateWhereClause(const ParseTree &node);
  void validateExpression(const ParseTree &node);
  void validateCondition(const ParseTree &node);
  void validateColumn(const std::string &columnName, int line, int col);
  void validateInsert(const ParseTree &node);
  void validateUpdate(const ParseTree &node);
  void validateDelete(const ParseTree &node);

  // Error reporting
  void reportError(const std::string &message, int line = 1, int col = 1);

public:
  /**
   * Constructor
   */
  SemanticAnalyzer();

  /**
   * Perform semantic analysis on the parse tree
   * @param tree Parse tree from syntax analysis
   * @return true if semantically valid, false otherwise
   */
  bool analyze(const ParseTree &tree);

  /**
   * Check if analysis had errors
   */
  bool hasErrors() const;

  /**
   * Get semantic errors
   */
  std::vector<CompilerError> getErrors() const;

  /**
   * Print symbol table for demonstration
   */
  void printSymbolTable() const;

  /**
   * Get the symbol table (for external access)
   */
  const SymbolTable &getSymbolTable() const;
};

} // namespace MiniSQL

#endif // SEMANTIC_H
