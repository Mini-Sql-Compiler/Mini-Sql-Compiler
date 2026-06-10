/**
 * Mini SQL Compiler for Query Validation & Execution
 * ====================================================
 * File: data_store.cpp
 * Description: In-Memory Data Storage Implementation
 *
 * Stores table data as vectors of row maps. Supports CRUD operations
 * and CSV file persistence. Pre-loads sample data for demonstration.
 */

#include "../include/data_store.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

namespace MiniSQL {

// Constructor
DataStore::DataStore(const SymbolTable &symbolTable) : dataDir("data") {
  // Initialize table structures from schema
  auto tableNames = symbolTable.getTableNames();
  for (const auto &name : tableNames) {
    const TableInfo *info = symbolTable.getTable(name);
    if (info) {
      tables[name] = TableData(*info);
    }
  }

  // Load sample data
  loadSampleData();
}

// ============================================================================
// SAMPLE DATA - Pre-loaded for demonstration
// ============================================================================
void DataStore::loadSampleData() {
  // Employees table
  if (tables.find("employees") != tables.end()) {
    std::vector<std::vector<std::string>> empData = {
        {"1", "Rahul Sharma", "28", "55000", "Engineering"},
        {"2", "Priya Patel", "32", "72000", "Marketing"},
        {"3", "Amit Kumar", "45", "95000", "Engineering"},
        {"4", "Sneha Gupta", "26", "48000", "HR"},
        {"5", "Vikram Singh", "38", "82000", "Sales"},
        {"6", "Anjali Verma", "29", "61000", "Engineering"},
        {"7", "Rajesh Nair", "41", "88000", "Marketing"},
        {"8", "Deepa Iyer", "35", "75000", "HR"}};
    std::vector<std::string> empCols = {"id", "name", "age", "salary",
                                        "department"};
    for (const auto &row : empData) {
      insertRow("employees", empCols, row);
    }
  }

  // Departments table
  if (tables.find("departments") != tables.end()) {
    std::vector<std::vector<std::string>> deptData = {
        {"1", "Engineering", "500000"},
        {"2", "Marketing", "300000"},
        {"3", "HR", "200000"},
        {"4", "Sales", "350000"}};
    std::vector<std::string> deptCols = {"id", "name", "budget"};
    for (const auto &row : deptData) {
      insertRow("departments", deptCols, row);
    }
  }

  // Users table
  if (tables.find("users") != tables.end()) {
    std::vector<std::vector<std::string>> userData = {
        {"1", "rahul_dev", "rahul@example.com", "28", "active"},
        {"2", "priya_m", "priya@example.com", "32", "active"},
        {"3", "amit_k", "amit@example.com", "45", "inactive"},
        {"4", "sneha_g", "sneha@example.com", "26", "active"},
        {"5", "vikram_s", "vikram@example.com", "38", "active"}};
    std::vector<std::string> userCols = {"id", "username", "email", "age",
                                         "status"};
    for (const auto &row : userData) {
      insertRow("users", userCols, row);
    }
  }

  // Products table
  if (tables.find("products") != tables.end()) {
    std::vector<std::vector<std::string>> prodData = {
        {"1", "Laptop", "75000.50", "25"},
        {"2", "Mouse", "500.00", "150"},
        {"3", "Keyboard", "1500.00", "80"},
        {"4", "Monitor", "22000.00", "40"},
        {"5", "Headphones", "3500.00", "100"}};
    std::vector<std::string> prodCols = {"id", "name", "price", "quantity"};
    for (const auto &row : prodData) {
      insertRow("products", prodCols, row);
    }
  }
}

// ============================================================================
// INSERT ROW
// ============================================================================
bool DataStore::insertRow(const std::string &tableName,
                          const std::vector<std::string> &columns,
                          const std::vector<std::string> &values) {
  auto it = tables.find(tableName);
  if (it == tables.end())
    return false;

  if (columns.size() != values.size())
    return false;

  Row row;
  for (size_t i = 0; i < columns.size(); i++) {
    row[columns[i]] = values[i];
  }

  it->second.rows.push_back(row);
  return true;
}

// ============================================================================
// GET ALL ROWS
// ============================================================================
std::vector<Row> DataStore::getRows(const std::string &tableName) const {
  auto it = tables.find(tableName);
  if (it == tables.end())
    return {};
  return it->second.rows;
}

// ============================================================================
// GET FILTERED ROWS (WHERE clause)
// ============================================================================
std::vector<Row> DataStore::getFilteredRows(const std::string &tableName,
                                            std::function<bool(const Row&)> predicate) const {
  auto it = tables.find(tableName);
  if (it == tables.end())
    return {};

  std::vector<Row> result;
  for (const auto &row : it->second.rows) {
    if (predicate(row)) {
      result.push_back(row);
    }
  }
  return result;
}

// ============================================================================
// UPDATE ROWS
// ============================================================================
int DataStore::updateRows(const std::string &tableName,
                          const std::string &setColumn,
                          const std::string &setValue,
                          std::function<bool(const Row&)> predicate) {
  auto it = tables.find(tableName);
  if (it == tables.end())
    return 0;

  int count = 0;
  for (auto &row : it->second.rows) {
    if (predicate(row)) {
      row[setColumn] = setValue;
      count++;
    }
  }
  return count;
}

// ============================================================================
// DELETE ROWS
// ============================================================================
int DataStore::deleteRows(const std::string &tableName,
                          std::function<bool(const Row&)> predicate) {
  auto it = tables.find(tableName);
  if (it == tables.end())
    return 0;

  int originalSize = static_cast<int>(it->second.rows.size());

  it->second.rows.erase(
      std::remove_if(it->second.rows.begin(), it->second.rows.end(),
                     [&](const Row &row) {
                       return predicate(row);
                     }),
      it->second.rows.end());

  return originalSize - static_cast<int>(it->second.rows.size());
}

int DataStore::deleteAllRows(const std::string &tableName) {
  auto it = tables.find(tableName);
  if (it == tables.end())
    return 0;

  int count = static_cast<int>(it->second.rows.size());
  it->second.rows.clear();
  return count;
}

// ============================================================================
// UTILITY METHODS
// ============================================================================
int DataStore::getRowCount(const std::string &tableName) const {
  auto it = tables.find(tableName);
  if (it == tables.end())
    return 0;
  return static_cast<int>(it->second.rows.size());
}

std::vector<std::string>
DataStore::getColumnNames(const std::string &tableName) const {
  auto it = tables.find(tableName);
  if (it == tables.end())
    return {};

  std::vector<std::string> names;
  for (const auto &col : it->second.schema.columns) {
    names.push_back(col.name);
  }
  return names;
}

bool DataStore::tableExists(const std::string &tableName) const {
  return tables.find(tableName) != tables.end();
}

// ============================================================================
// VALUE COMPARISON
// ============================================================================
bool DataStore::compareValues(const std::string &left, const std::string &op,
                              const std::string &right) const {
  // Try numeric comparison first
  try {
    double lVal = std::stod(left);
    double rVal = std::stod(right);

    if (op == "=")
      return lVal == rVal;
    if (op == "!=")
      return lVal != rVal;
    if (op == "<")
      return lVal < rVal;
    if (op == "<=")
      return lVal <= rVal;
    if (op == ">")
      return lVal > rVal;
    if (op == ">=")
      return lVal >= rVal;
  } catch (...) {
    // Fall through to string comparison
  }

  // String comparison
  if (op == "=")
    return left == right;
  if (op == "!=")
    return left != right;
  if (op == "<")
    return left < right;
  if (op == "<=")
    return left <= right;
  if (op == ">")
    return left > right;
  if (op == ">=")
    return left >= right;

  return false;
}

// ============================================================================
// CSV FILE I/O
// ============================================================================
void DataStore::loadFromFiles(const std::string &dir) {
  dataDir = dir;
  for (auto &pair : tables) {
    std::string filePath = dir + "/" + pair.first + ".csv";
    std::ifstream file(filePath);

    if (!file.is_open())
      continue;

    // Clear existing data
    pair.second.rows.clear();

    // Read header line
    std::string headerLine;
    if (!std::getline(file, headerLine))
      continue;

    std::vector<std::string> headers;
    std::istringstream headerStream(headerLine);
    std::string col;
    while (std::getline(headerStream, col, ',')) {
      // Trim whitespace
      size_t start = col.find_first_not_of(" \t");
      size_t end = col.find_last_not_of(" \t");
      if (start != std::string::npos) {
        headers.push_back(col.substr(start, end - start + 1));
      }
    }

    // Read data rows
    std::string rowLine;
    while (std::getline(file, rowLine)) {
      if (rowLine.empty())
        continue;

      std::vector<std::string> values;
      std::istringstream rowStream(rowLine);
      std::string val;
      while (std::getline(rowStream, val, ',')) {
        size_t start = val.find_first_not_of(" \t");
        size_t end = val.find_last_not_of(" \t");
        if (start != std::string::npos) {
          values.push_back(val.substr(start, end - start + 1));
        } else {
          values.push_back("");
        }
      }

      if (values.size() == headers.size()) {
        insertRow(pair.first, headers, values);
      }
    }

    file.close();
    std::cout << "Loaded " << pair.second.rows.size() << " rows from "
              << filePath << "\n";
  }
}

void DataStore::saveToFiles(const std::string &dir) const {
  for (const auto &pair : tables) {
    std::string filePath = dir + "/" + pair.first + ".csv";
    std::ofstream file(filePath);

    if (!file.is_open()) {
      std::cerr << "Warning: Could not save to " << filePath << "\n";
      continue;
    }

    // Write header
    const auto &cols = pair.second.schema.columns;
    for (size_t i = 0; i < cols.size(); i++) {
      if (i > 0)
        file << ",";
      file << cols[i].name;
    }
    file << "\n";

    // Write rows
    for (const auto &row : pair.second.rows) {
      for (size_t i = 0; i < cols.size(); i++) {
        if (i > 0)
          file << ",";
        auto it = row.find(cols[i].name);
        if (it != row.end()) {
          file << it->second;
        }
      }
      file << "\n";
    }

    file.close();
    std::cout << "Saved " << pair.second.rows.size() << " rows to " << filePath
              << "\n";
  }
}

} // namespace MiniSQL
