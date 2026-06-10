/**
 * Mini SQL Compiler for Query Validation & Execution
 * ====================================================
 * File: data_store.h
 * Description: In-Memory Data Storage with CSV Persistence
 *
 * Provides actual data storage for the SQL engine.
 * Tables are stored as vectors of row maps, with CSV file I/O.
 */

#ifndef DATA_STORE_H
#define DATA_STORE_H

#include "symbol_table.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace MiniSQL {

// A single row: column_name -> value (all stored as strings)
using Row = std::unordered_map<std::string, std::string>;

// A table's data: schema info + rows
struct TableData {
  TableInfo schema;
  std::vector<Row> rows;

  TableData() = default;
  TableData(const TableInfo &info) : schema(info) {}
};

class DataStore {
private:
  std::unordered_map<std::string, TableData> tables;
  std::string dataDir; // Directory for CSV persistence

public:
  /**
   * Constructor - Initialize with schema from SymbolTable and sample data
   */
  DataStore(const SymbolTable &symbolTable);

  /**
   * Insert a row into a table
   * @return true on success
   */
  bool insertRow(const std::string &tableName,
                 const std::vector<std::string> &columns,
                 const std::vector<std::string> &values);

  /**
   * Get all rows from a table
   */
  std::vector<Row> getRows(const std::string &tableName) const;

  /**
   * Get rows matching a predicate
   */
  std::vector<Row> getFilteredRows(const std::string &tableName,
                                   std::function<bool(const Row&)> predicate) const;

  /**
   * Update rows matching a predicate
   * @return number of rows updated
   */
  int updateRows(const std::string &tableName, const std::string &setColumn,
                 const std::string &setValue, std::function<bool(const Row&)> predicate);

  /**
   * Delete rows matching a predicate
   * @return number of rows deleted
   */
  int deleteRows(const std::string &tableName, std::function<bool(const Row&)> predicate);

  /**
   * Delete all rows from a table
   * @return number of rows deleted
   */
  int deleteAllRows(const std::string &tableName);

  /**
   * Get row count for a table
   */
  int getRowCount(const std::string &tableName) const;

  /**
   * Get column names for a table (ordered)
   */
  std::vector<std::string> getColumnNames(const std::string &tableName) const;

  /**
   * Load data from CSV files in the data directory
   */
  void loadFromFiles(const std::string &dir);

  /**
   * Save data to CSV files in the data directory
   */
  void saveToFiles(const std::string &dir) const;

  /**
   * Check if a table exists
   */
  bool tableExists(const std::string &tableName) const;

  /**
   * Compare two values based on operator
   */
  bool compareValues(const std::string &left, const std::string &op,
                     const std::string &right) const;

private:
  /**
   * Load sample data into tables
   */
  void loadSampleData();
};

} // namespace MiniSQL

#endif // DATA_STORE_H
