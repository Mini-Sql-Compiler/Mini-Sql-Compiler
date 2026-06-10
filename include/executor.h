/**
 * Mini SQL Compiler for Query Validation & Execution
 * ====================================================
 * File: executor.h
 * Description: Query Execution Engine Header
 *
 * The Executor is the fourth phase of compilation.
 * After validation, it traverses the parse tree and executes
 * the query against the DataStore.
 */

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "common.h"
#include "data_store.h"
#include <string>
#include <vector>

namespace MiniSQL {

// Result of query execution
struct QueryResult {
  bool success;
  std::string message;
  std::vector<std::string> columnNames;
  std::vector<Row> rows;
  int affectedRows; // For INSERT/UPDATE/DELETE

  QueryResult() : success(false), affectedRows(0) {}
};

class Executor {
private:
  DataStore &dataStore;

  // Execute specific query types
  QueryResult executeSelect(const ParseTree &tree);
  QueryResult executeInsert(const ParseTree &tree);
  QueryResult executeUpdate(const ParseTree &tree);
  QueryResult executeDelete(const ParseTree &tree);

  // Evaluate condition recursively
  bool evaluateExpression(const ParseTree &expr, const Row &row) const;

  // Extract information from parse tree
  std::string extractTableName(const ParseTree &tree) const;
  std::vector<std::string> extractColumns(const ParseTree &tree) const;

  // Print results in tabular format
  void printResultTable(const QueryResult &result) const;

public:
  /**
   * Constructor
   * @param store Reference to the data store
   */
  explicit Executor(DataStore &store);

  /**
   * Execute a validated parse tree
   * @param tree Parse tree from the parser
   * @return QueryResult with execution outcome
   */
  QueryResult execute(const ParseTree &tree);

  /**
   * Print execution results
   */
  void printResults(const QueryResult &result) const;
};

} // namespace MiniSQL

#endif // EXECUTOR_H
