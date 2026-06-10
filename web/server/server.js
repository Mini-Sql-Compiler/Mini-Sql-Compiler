/**
 * Mini SQL Compiler — Express API Server
 * Serves the web frontend and exposes compilation API endpoints.
 */

const express = require('express');
const cors = require('cors');
const helmet = require('helmet');
const rateLimit = require('express-rate-limit');
const path = require('path');
const { compileQuery, getSchema } = require('./compiler-bridge');

const app = express();
const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(helmet({
  contentSecurityPolicy: {
    directives: {
      defaultSrc: ["'self'"],
      styleSrc: ["'self'", "'unsafe-inline'", "https://fonts.googleapis.com"],
      fontSrc: ["'self'", "https://fonts.gstatic.com"],
      scriptSrc: ["'self'", "'unsafe-inline'"],
      imgSrc: ["'self'", "data:"],
    },
  },
}));
app.use(express.json({ limit: '10kb' }));

// Rate limiting: 60 requests per minute
const limiter = rateLimit({
  windowMs: 60 * 1000,
  max: 60,
  message: { error: 'Too many requests, please wait a moment.' },
});
app.use('/api/', limiter);

// Serve static frontend files
app.use(express.static(path.join(__dirname, '../public')));

// ============================================================================
// API ENDPOINTS
// ============================================================================

/**
 * POST /api/compile
 * Compile a SQL query and return structured results
 */
app.post('/api/compile', async (req, res) => {
  try {
    const { query } = req.body;

    if (!query || typeof query !== 'string') {
      return res.status(400).json({
        error: 'Missing or invalid "query" field in request body',
      });
    }

    const trimmed = query.trim();
    if (!trimmed) {
      return res.status(400).json({ error: 'Query cannot be empty' });
    }

    const result = await compileQuery(trimmed);
    res.json(result);
  } catch (err) {
    console.error('Compilation error:', err.message);
    res.status(500).json({ error: err.message });
  }
});

/**
 * GET /api/schema
 * Return the available database schema
 */
app.get('/api/schema', async (req, res) => {
  try {
    const schema = await getSchema();
    res.json(schema);
  } catch (err) {
    console.error('Schema error:', err.message);
    res.status(500).json({ error: err.message });
  }
});

/**
 * GET /api/examples
 * Return example queries for the UI
 */
app.get('/api/examples', (req, res) => {
  res.json({
    valid: [
      { name: 'Select All Employees', query: 'SELECT * FROM employees;' },
      { name: 'Filter by Salary', query: 'SELECT name, salary FROM employees WHERE salary > 70000;' },
      { name: 'Compound AND Filter', query: "SELECT * FROM employees WHERE age > 30 AND department = 'Engineering';" },
      { name: 'Compound OR & Parentheses', query: "SELECT name, salary FROM employees WHERE salary > 80000 OR (age < 30 AND department = 'HR');" },
      { name: 'User Emails', query: 'SELECT username, email FROM users WHERE age > 25;' },
      { name: 'Product Catalog', query: 'SELECT name, price, quantity FROM products;' },
      { name: 'Department Budgets', query: 'SELECT name, budget FROM departments;' },
      { name: 'Insert Employee', query: "INSERT INTO employees (id, name, age, salary, department) VALUES (9, 'Kavita Joshi', 31, 68000, 'Sales');" },
      { name: 'Update Salary', query: 'UPDATE employees SET salary = 100000 WHERE id = 3;' },
      { name: 'Delete Employee', query: 'DELETE FROM employees WHERE id = 9;' },
    ],
    invalid: [
      { name: 'Missing FROM (Syntax)', query: 'SELECT * employees;', errorType: 'Syntax' },
      { name: 'Bad Table (Semantic)', query: 'SELECT * FROM customers;', errorType: 'Semantic' },
      { name: 'Bad Column (Semantic)', query: 'SELECT invalid_col FROM employees;', errorType: 'Semantic' },
      { name: 'Invalid Char (Lexical)', query: 'SELECT @ FROM users;', errorType: 'Lexical' },
      { name: 'Missing Semicolon', query: 'SELECT * FROM users', errorType: 'Syntax' },
    ],
  });
});

/**
 * GET /api/health
 * Health check endpoint
 */
app.get('/api/health', (req, res) => {
  res.json({ status: 'ok', timestamp: new Date().toISOString() });
});

// Catch-all: serve index.html for SPA routing
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, '../public/index.html'));
});

// Start server
app.listen(PORT, () => {
  console.log(`\n🚀 Mini SQL Compiler Web IDE running at http://localhost:${PORT}`);
  console.log(`   API endpoints:`);
  console.log(`   POST /api/compile  — Compile a SQL query`);
  console.log(`   GET  /api/schema   — Get database schema`);
  console.log(`   GET  /api/examples — Get example queries\n`);
});
