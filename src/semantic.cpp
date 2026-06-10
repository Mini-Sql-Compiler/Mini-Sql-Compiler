/**
 * Mini SQL Compiler for Query Validation
 * =======================================
 * File: semantic.cpp
 * Description: Semantic Analyzer Implementation
 * Team Member: Member 3
 *
 * THEORY:
 * -------
 * Semantic Analysis is the phase where the compiler validates the
 * MEANING of the program, not just its structure.
 *
 * Key Concepts:
 *
 * 1. SYMBOL TABLE LOOKUP:
 *    - When we encounter a table name, we check if it exists in our schema
 *    - When we encounter a column name, we verify it belongs to the table
 *
 * 2. ATTRIBUTE GRAMMAR:
 *    - We traverse the parse tree and compute/validate attributes
 *    - Inherited attributes: Table context flows down to column validation
 *    - Synthesized attributes: Validation results flow up
 *
 * 3. TYPE CHECKING:
 *    - Ensure compatible types in comparisons
 *    - Verify operator applicability
 *
 * PROCESS:
 * 1. First validate FROM clause to establish table context
 * 2. Then validate SELECT columns against that table
 * 3. Finally validate WHERE conditions
 */

#include "../include/semantic.h"
#include <algorithm>
#include <iostream>

namespace MiniSQL {

SemanticAnalyzer::SemanticAnalyzer() : currentTable("") {}

// ============================================================================
// MAIN ANALYSIS METHOD
// ============================================================================
bool SemanticAnalyzer::analyze(const ParseTree &tree) {
  std::cout << "\n========================================\n";
  std::cout << "   PHASE 3: SEMANTIC ANALYSIS\n";
  std::cout << "========================================\n";

  errors.clear();
  currentTable = "";
  selectedColumns.clear();

  if (!tree) {
    reportError("No parse tree to analyze");
    return false;
  }

  // Dispatch based on query type
  switch (tree->type) {
  case NodeType::QUERY:
    validateQuery(tree);
    break;
  case NodeType::INSERT_QUERY:
    validateInsert(tree);
    break;
  case NodeType::UPDATE_QUERY:
    validateUpdate(tree);
    break;
  case NodeType::DELETE_QUERY:
    validateDelete(tree);
    break;
  default:
    reportError("Unknown query type for semantic analysis");
    break;
  }

  if (errors.empty()) {
    std::cout << "Semantic Analysis: SUCCESS\n";
    std::cout << "All identifiers resolved correctly.\n";
    return true;
  } else {
    std::cout << "Semantic Analysis: FAILED with " << errors.size()
              << " error(s)\n";
    return false;
  }
}

// ============================================================================
// QUERY VALIDATION
// ============================================================================
void SemanticAnalyzer::validateQuery(const ParseTree &node) {
  if (!node || node->type != NodeType::QUERY)
    return;

  // IMPORTANT: We must validate FROM first to establish table context
  // Then validate SELECT and WHERE against that context

  ParseTree fromClause = nullptr;
  ParseTree selectClause = nullptr;
  ParseTree whereClause = nullptr;

  // Find clauses
  for (const auto &child : node->children) {
    switch (child->type) {
    case NodeType::SELECT_CLAUSE:
      selectClause = child;
      break;
    case NodeType::FROM_CLAUSE:
      fromClause = child;
      break;
    case NodeType::WHERE_CLAUSE:
      whereClause = child;
      break;
    default:
      break;
    }
  }

  // Validate in correct order: FROM -> SELECT -> WHERE
  if (fromClause) {
    validateFromClause(fromClause);
  }

  if (selectClause && !currentTable.empty()) {
    validateSelectClause(selectClause);
  }

  if (whereClause && !currentTable.empty()) {
    validateWhereClause(whereClause);
  }
}

// ============================================================================
// FROM CLAUSE VALIDATION - Validate table exists
// ============================================================================
void SemanticAnalyzer::validateFromClause(const ParseTree &node) {
  for (const auto &child : node->children) {
    if (child->type == NodeType::TABLE_NAME) {
      std::string tableName = child->value;

      // Convert to lowercase for comparison
      std::string lowerTable = tableName;
      std::transform(lowerTable.begin(), lowerTable.end(), lowerTable.begin(),
                     ::tolower);

      if (!symbolTable.tableExists(lowerTable)) {
        std::string msg = "Table '" + tableName + "' does not exist. ";
        msg += "Available tables: ";
        auto tables = symbolTable.getTableNames();
        for (size_t i = 0; i < tables.size(); i++) {
          if (i > 0)
            msg += ", ";
          msg += tables[i];
        }
        reportError(msg);
      } else {
        currentTable = lowerTable;
        std::cout << "Table '" << tableName << "' validated.\n";
      }
    }
  }
}

// ============================================================================
// SELECT CLAUSE VALIDATION - Validate column names
// ============================================================================
void SemanticAnalyzer::validateSelectClause(const ParseTree &node) {
  for (const auto &child : node->children) {
    if (child->type == NodeType::COLUMN_LIST) {
      for (const auto &column : child->children) {
        if (column->type == NodeType::COLUMN) {
          std::string colName = column->value;

          // * is always valid
          if (colName == "*") {
            std::cout << "SELECT * - All columns selected.\n";
            continue;
          }

          validateColumn(colName, 1, 1);
          selectedColumns.push_back(colName);
        }
      }
    }
  }
}

// ============================================================================
// WHERE CLAUSE VALIDATION
// ============================================================================
void SemanticAnalyzer::validateWhereClause(const ParseTree &node) {
  for (const auto &child : node->children) {
    validateExpression(child);
  }
}

// ============================================================================
// EXPRESSION VALIDATION - Recursively traverse AND/OR expression trees
// ============================================================================
void SemanticAnalyzer::validateExpression(const ParseTree &node) {
  if (!node) return;

  if (node->type == NodeType::AND_EXPR || node->type == NodeType::OR_EXPR) {
    for (const auto &child : node->children) {
      validateExpression(child);
    }
  } else if (node->type == NodeType::CONDITION) {
    validateCondition(node);
  }
}

// ============================================================================
// CONDITION VALIDATION - Validate column and type compatibility
// ============================================================================
void SemanticAnalyzer::validateCondition(const ParseTree &node) {
  std::string columnName;
  std::string operatorStr;
  std::string value;

  for (const auto &child : node->children) {
    switch (child->type) {
    case NodeType::COLUMN:
      columnName = child->value;
      validateColumn(columnName, 1, 1);
      break;
    case NodeType::OPERATOR:
      operatorStr = child->value;
      break;
    case NodeType::VALUE:
      value = child->value;
      break;
    default:
      break;
    }
  }

  // Basic type compatibility check
  if (!columnName.empty() && !value.empty()) {
    const TableInfo *table = symbolTable.getTable(currentTable);
    if (table) {
      for (const auto &col : table->columns) {
        if (col.name == columnName) {
          // Check type compatibility
          bool isNumericColumn =
              (col.dataType == "INT" || col.dataType == "FLOAT");
          bool isNumericValue =
              !value.empty() && (std::isdigit(value[0]) ||
                                 (value[0] == '-' && value.length() > 1));

          if (isNumericColumn && !isNumericValue) {
            std::cout << "Warning: Comparing numeric column '" << columnName
                      << "' with non-numeric value.\n";
          }
          break;
        }
      }
    }
  }

  std::cout << "Condition validated: " << columnName << " " << operatorStr
            << " " << value << "\n";
}

// ============================================================================
// COLUMN VALIDATION HELPER
// ============================================================================
void SemanticAnalyzer::validateColumn(const std::string &columnName, int line,
                                      int col) {
  if (currentTable.empty()) {
    reportError("No table context for column validation", line, col);
    return;
  }

  // Convert to lowercase for comparison
  std::string lowerCol = columnName;
  std::transform(lowerCol.begin(), lowerCol.end(), lowerCol.begin(), ::tolower);

  if (!symbolTable.columnExists(currentTable, lowerCol)) {
    std::string msg = "Column '" + columnName + "' does not exist in table '" +
                      currentTable + "'. Available columns: ";

    const TableInfo *table = symbolTable.getTable(currentTable);
    if (table) {
      for (size_t i = 0; i < table->columns.size(); i++) {
        if (i > 0)
          msg += ", ";
        msg += table->columns[i].name;
      }
    }
    reportError(msg, line, col);
  } else {
    std::cout << "Column '" << columnName << "' validated in table '"
              << currentTable << "'.\n";
  }
}

// ============================================================================
// INSERT VALIDATION
// ============================================================================
void SemanticAnalyzer::validateInsert(const ParseTree &node) {
  std::string tableName;
  std::vector<std::string> columns;
  int valueCount = 0;

  for (const auto &child : node->children) {
    if (child->type == NodeType::TABLE_NAME) {
      tableName = child->value;
      std::string lowerTable = tableName;
      std::transform(lowerTable.begin(), lowerTable.end(), lowerTable.begin(),
                     ::tolower);

      if (!symbolTable.tableExists(lowerTable)) {
        std::string msg = "Table '" + tableName + "' does not exist.";
        reportError(msg);
        return;
      }
      currentTable = lowerTable;
      std::cout << "Table '" << tableName << "' validated for INSERT.\n";
    } else if (child->type == NodeType::COLUMN_LIST) {
      for (const auto &col : child->children) {
        if (col->type == NodeType::COLUMN) {
          columns.push_back(col->value);
          validateColumn(col->value, 1, 1);
        }
      }
    } else if (child->type == NodeType::VALUE_LIST) {
      for (const auto &val : child->children) {
        if (val->type == NodeType::VALUE) {
          valueCount++;
        }
      }
    }
  }

  // Check column count matches value count
  if (!columns.empty() && valueCount > 0 &&
      columns.size() != static_cast<size_t>(valueCount)) {
    reportError("Column count (" + std::to_string(columns.size()) +
                ") does not match value count (" + std::to_string(valueCount) +
                ")");
  }
}

// ============================================================================
// UPDATE VALIDATION
// ============================================================================
void SemanticAnalyzer::validateUpdate(const ParseTree &node) {
  for (const auto &child : node->children) {
    if (child->type == NodeType::TABLE_NAME) {
      std::string tableName = child->value;
      std::string lowerTable = tableName;
      std::transform(lowerTable.begin(), lowerTable.end(), lowerTable.begin(),
                     ::tolower);

      if (!symbolTable.tableExists(lowerTable)) {
        std::string msg = "Table '" + tableName + "' does not exist.";
        reportError(msg);
        return;
      }
      currentTable = lowerTable;
      std::cout << "Table '" << tableName << "' validated for UPDATE.\n";
    } else if (child->type == NodeType::SET_CLAUSE) {
      for (const auto &assign : child->children) {
        if (assign->type == NodeType::ASSIGNMENT) {
          for (const auto &ac : assign->children) {
            if (ac->type == NodeType::COLUMN) {
              validateColumn(ac->value, 1, 1);
            }
          }
        }
      }
    } else if (child->type == NodeType::WHERE_CLAUSE) {
      validateWhereClause(child);
    }
  }
}

// ============================================================================
// DELETE VALIDATION
// ============================================================================
void SemanticAnalyzer::validateDelete(const ParseTree &node) {
  for (const auto &child : node->children) {
    if (child->type == NodeType::FROM_CLAUSE) {
      validateFromClause(child);
    } else if (child->type == NodeType::WHERE_CLAUSE) {
      if (!currentTable.empty()) {
        validateWhereClause(child);
      }
    }
  }
}

// ============================================================================
// ERROR HANDLING
// ============================================================================
void SemanticAnalyzer::reportError(const std::string &message, int line,
                                   int col) {
  errors.push_back(
      CompilerError(ErrorType::SEMANTIC_ERROR, message, line, col));
}

bool SemanticAnalyzer::hasErrors() const { return !errors.empty(); }

std::vector<CompilerError> SemanticAnalyzer::getErrors() const {
  return errors;
}

void SemanticAnalyzer::printSymbolTable() const { symbolTable.print(); }

const SymbolTable &SemanticAnalyzer::getSymbolTable() const {
  return symbolTable;
}

} // namespace MiniSQL
