/**
 * Mini SQL Compiler for Query Validation & Execution
 * ====================================================
 * File: executor.cpp
 * Description: Query Execution Engine Implementation
 *
 * Traverses validated parse trees and executes queries against
 * the DataStore. Supports SELECT, INSERT, UPDATE, DELETE.
 */

#include "../include/executor.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

namespace MiniSQL {

// Constructor
Executor::Executor(DataStore &store) : dataStore(store) {}

// ============================================================================
// MAIN EXECUTION METHOD
// ============================================================================
QueryResult Executor::execute(const ParseTree &tree) {
  std::cout << "\n========================================\n";
  std::cout << "   PHASE 4: QUERY EXECUTION\n";
  std::cout << "========================================\n";

  if (!tree) {
    QueryResult result;
    result.message = "No parse tree to execute";
    return result;
  }

  QueryResult result;

  switch (tree->type) {
  case NodeType::QUERY:
    result = executeSelect(tree);
    break;
  case NodeType::INSERT_QUERY:
    result = executeInsert(tree);
    break;
  case NodeType::UPDATE_QUERY:
    result = executeUpdate(tree);
    break;
  case NodeType::DELETE_QUERY:
    result = executeDelete(tree);
    break;
  default:
    result.message = "Unknown query type";
    break;
  }

  return result;
}

// ============================================================================
// SELECT EXECUTION
// ============================================================================
QueryResult Executor::executeSelect(const ParseTree &tree) {
  QueryResult result;

  std::string tableName;
  std::vector<std::string> selectedCols;
  bool selectAll = false;

  // WHERE clause info
  ParseTree whereExpr = nullptr;
  bool hasWhere = false;

  // Extract information from parse tree
  for (const auto &child : tree->children) {
    if (child->type == NodeType::FROM_CLAUSE) {
      for (const auto &fc : child->children) {
        if (fc->type == NodeType::TABLE_NAME) {
          tableName = fc->value;
          // Convert to lowercase
          std::transform(tableName.begin(), tableName.end(), tableName.begin(),
                         ::tolower);
        }
      }
    } else if (child->type == NodeType::SELECT_CLAUSE) {
      for (const auto &sc : child->children) {
        if (sc->type == NodeType::COLUMN_LIST) {
          for (const auto &col : sc->children) {
            if (col->type == NodeType::COLUMN) {
              if (col->value == "*") {
                selectAll = true;
              } else {
                selectedCols.push_back(col->value);
              }
            }
          }
        }
      }
    } else if (child->type == NodeType::WHERE_CLAUSE) {
      hasWhere = true;
      if (!child->children.empty()) {
        whereExpr = child->children[0];
      }
    }
  }

  // Get the right columns
  if (selectAll) {
    selectedCols = dataStore.getColumnNames(tableName);
  }

  // Fetch rows
  std::vector<Row> rows;
  if (hasWhere && whereExpr) {
    rows = dataStore.getFilteredRows(tableName, [&](const Row &row) {
      return evaluateExpression(whereExpr, row);
    });
  } else {
    rows = dataStore.getRows(tableName);
  }

  result.success = true;
  result.columnNames = selectedCols;
  result.rows = rows;
  result.message = "Query executed successfully. " +
                   std::to_string(rows.size()) + " row(s) returned.";

  std::cout << "Execution: SUCCESS\n";
  std::cout << result.message << "\n";

  return result;
}

// ============================================================================
// INSERT EXECUTION
// ============================================================================
QueryResult Executor::executeInsert(const ParseTree &tree) {
  QueryResult result;

  std::string tableName;
  std::vector<std::string> columns;
  std::vector<std::string> values;

  for (const auto &child : tree->children) {
    if (child->type == NodeType::TABLE_NAME) {
      tableName = child->value;
      std::transform(tableName.begin(), tableName.end(), tableName.begin(),
                     ::tolower);
    } else if (child->type == NodeType::COLUMN_LIST) {
      for (const auto &col : child->children) {
        if (col->type == NodeType::COLUMN) {
          columns.push_back(col->value);
        }
      }
    } else if (child->type == NodeType::VALUE_LIST) {
      for (const auto &val : child->children) {
        if (val->type == NodeType::VALUE) {
          values.push_back(val->value);
        }
      }
    }
  }

  bool ok = dataStore.insertRow(tableName, columns, values);
  result.success = ok;
  result.affectedRows = ok ? 1 : 0;
  result.message =
      ok ? "1 row inserted successfully."
         : "INSERT failed: column/value count mismatch or table not found.";

  std::cout << "Execution: " << (ok ? "SUCCESS" : "FAILED") << "\n";
  std::cout << result.message << "\n";

  return result;
}

// ============================================================================
// UPDATE EXECUTION
// ============================================================================
QueryResult Executor::executeUpdate(const ParseTree &tree) {
  QueryResult result;

  std::string tableName;
  std::string setCol, setVal;
  ParseTree whereExpr = nullptr;
  bool hasWhere = false;

  for (const auto &child : tree->children) {
    if (child->type == NodeType::TABLE_NAME) {
      tableName = child->value;
      std::transform(tableName.begin(), tableName.end(), tableName.begin(),
                     ::tolower);
    } else if (child->type == NodeType::SET_CLAUSE) {
      for (const auto &assign : child->children) {
        if (assign->type == NodeType::ASSIGNMENT) {
          for (const auto &ac : assign->children) {
            if (ac->type == NodeType::COLUMN)
              setCol = ac->value;
            else if (ac->type == NodeType::VALUE)
              setVal = ac->value;
          }
        }
      }
    } else if (child->type == NodeType::WHERE_CLAUSE) {
      hasWhere = true;
      if (!child->children.empty()) {
        whereExpr = child->children[0];
      }
    }
  }

  if (!hasWhere || !whereExpr) {
    result.message = "UPDATE without WHERE is not supported for safety.";
    result.success = false;
    return result;
  }

  int count = dataStore.updateRows(tableName, setCol, setVal, [&](const Row &row) {
    return evaluateExpression(whereExpr, row);
  });
  result.success = true;
  result.affectedRows = count;
  result.message = std::to_string(count) + " row(s) updated successfully.";

  std::cout << "Execution: SUCCESS\n";
  std::cout << result.message << "\n";

  return result;
}

// ============================================================================
// DELETE EXECUTION
// ============================================================================
QueryResult Executor::executeDelete(const ParseTree &tree) {
  QueryResult result;

  std::string tableName;
  ParseTree whereExpr = nullptr;
  bool hasWhere = false;

  for (const auto &child : tree->children) {
    if (child->type == NodeType::FROM_CLAUSE) {
      for (const auto &fc : child->children) {
        if (fc->type == NodeType::TABLE_NAME) {
          tableName = fc->value;
          std::transform(tableName.begin(), tableName.end(), tableName.begin(),
                         ::tolower);
        }
      }
    } else if (child->type == NodeType::WHERE_CLAUSE) {
      hasWhere = true;
      if (!child->children.empty()) {
        whereExpr = child->children[0];
      }
    }
  }

  int count;
  if (hasWhere && whereExpr) {
    count = dataStore.deleteRows(tableName, [&](const Row &row) {
      return evaluateExpression(whereExpr, row);
    });
  } else {
    count = dataStore.deleteAllRows(tableName);
  }

  result.success = true;
  result.affectedRows = count;
  result.message = std::to_string(count) + " row(s) deleted successfully.";

  std::cout << "Execution: SUCCESS\n";
  std::cout << result.message << "\n";

  return result;
}

// ============================================================================
// RESULT PRINTING
// ============================================================================
void Executor::printResults(const QueryResult &result) const {
  if (!result.success) {
    std::cout << "\nExecution Error: " << result.message << "\n";
    return;
  }

  // For non-SELECT queries (INSERT/UPDATE/DELETE)
  if (result.columnNames.empty()) {
    std::cout << "\n" << result.message << "\n";
    if (result.affectedRows > 0) {
      std::cout << "Affected rows: " << result.affectedRows << "\n";
    }
    return;
  }

  // For SELECT queries - print result table
  printResultTable(result);
}

void Executor::printResultTable(const QueryResult &result) const {
  if (result.columnNames.empty())
    return;

  // Calculate column widths
  std::vector<size_t> widths;
  for (const auto &col : result.columnNames) {
    widths.push_back(col.length());
  }

  // Check row data widths
  for (const auto &row : result.rows) {
    for (size_t i = 0; i < result.columnNames.size(); i++) {
      auto it = row.find(result.columnNames[i]);
      if (it != row.end()) {
        widths[i] = std::max(widths[i], it->second.length());
      }
    }
  }

  // Minimum column width
  for (auto &w : widths) {
    w = std::max(w, (size_t)4);
    w += 2; // padding
  }

  // Print separator
  auto printSep = [&]() {
    std::cout << "+";
    for (size_t w : widths) {
      for (size_t i = 0; i < w + 2; i++)
        std::cout << "-";
      std::cout << "+";
    }
    std::cout << "\n";
  };

  std::cout << "\n--- Query Results ---\n";
  printSep();

  // Print header
  std::cout << "|";
  for (size_t i = 0; i < result.columnNames.size(); i++) {
    std::cout << " " << std::left << std::setw(widths[i] + 1)
              << result.columnNames[i] << "|";
  }
  std::cout << "\n";
  printSep();

  // Print rows
  if (result.rows.empty()) {
    std::cout << "| (no rows returned)";
    size_t totalWidth = 0;
    for (size_t w : widths)
      totalWidth += w + 3;
    for (size_t i = 19; i < totalWidth - 1; i++)
      std::cout << " ";
    std::cout << "|\n";
    printSep();
  } else {
    for (const auto &row : result.rows) {
      std::cout << "|";
      for (size_t i = 0; i < result.columnNames.size(); i++) {
        auto it = row.find(result.columnNames[i]);
        std::string val = (it != row.end()) ? it->second : "NULL";
        std::cout << " " << std::left << std::setw(widths[i] + 1) << val << "|";
      }
      std::cout << "\n";
    }
    printSep();
  }

  std::cout << result.rows.size() << " row(s) in set\n";
}

bool Executor::evaluateExpression(const ParseTree &expr, const Row &row) const {
  if (!expr) return false;

  if (expr->type == NodeType::AND_EXPR) {
    if (expr->children.size() < 2) return false;
    return evaluateExpression(expr->children[0], row) && evaluateExpression(expr->children[1], row);
  }
  if (expr->type == NodeType::OR_EXPR) {
    if (expr->children.size() < 2) return false;
    return evaluateExpression(expr->children[0], row) || evaluateExpression(expr->children[1], row);
  }
  if (expr->type == NodeType::CONDITION) {
    std::string columnName;
    std::string op;
    std::string value;

    for (const auto &child : expr->children) {
      if (child->type == NodeType::COLUMN) {
        columnName = child->value;
      } else if (child->type == NodeType::OPERATOR) {
        op = child->value;
      } else if (child->type == NodeType::VALUE) {
        value = child->value;
      }
    }

    if (columnName.empty()) return false;

    // Case-insensitive lookup of column name in row
    std::string lowerCol = columnName;
    std::transform(lowerCol.begin(), lowerCol.end(), lowerCol.begin(), ::tolower);

    auto it = row.end();
    for (auto rowCol = row.begin(); rowCol != row.end(); ++rowCol) {
      std::string rowColLower = rowCol->first;
      std::transform(rowColLower.begin(), rowColLower.end(), rowColLower.begin(), ::tolower);
      if (rowColLower == lowerCol) {
        it = rowCol;
        break;
      }
    }

    if (it != row.end()) {
      return dataStore.compareValues(it->second, op, value);
    }
    return false;
  }
  return false;
}

} // namespace MiniSQL
