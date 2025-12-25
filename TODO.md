# KrisLang

KrisLang is a custom programming language written in C.
This project aims to evolve from a simple interpreter into a real, structured programming language with modern features.

This document describes the roadmap and architecture plan for KrisLang, organized from easiest to hardest, and categorized by importance.

---

## Project Goals

- Build a real programming language, not a toy
- Clean lexer, parser, AST, and runtime separation
- Clear language semantics
- Good developer experience
- Extensible architecture for future features

---

## Roadmap Overview

The roadmap is divided into four categories:

1. CORE (mandatory foundation)
2. IMPORTANT (language usability)
3. ADVANCED (powerful language features)
4. FUTURE (long-term goals)

Each section is ordered from easier to harder.

---

## CORE FEATURES (Must Have)

These features are required for KrisLang to be considered a real language.

---

### 1. Tokenization (Lexer)

Description:
The lexer converts source code into tokens. This must be stable and complete.

Tasks:
- Add all missing keyword tokens (IF, ELSE, WHILE, FOR, CLASS, FUNCTION, RETURN )
- Emit tokens for operators and delimiters
- Emit whitespace and comments consistently
- Track line and column for each token
- Ensure keywords are not parsed as identifiers

Difficulty: Easy

---

### 2. Parser Structure

Description:
The parser builds the AST using a recursive descent approach.

Tasks:
- Create a parseStatement dispatcher
- Implement parseBlock for brace-based blocks
- Implement parseIf, parseWhile, parseFor
- Implement operator precedence
- Skip whitespace and comments consistently
- Add basic error recovery (do not exit on first error)

Difficulty: Easy to Medium

---

### 3. AST Architecture

Description:
The AST represents the program structure in memory.

Tasks:
- Define all AST node types in one header
- Add nodes for IF, ELSE IF, WHILE, FOR
- Add nodes for FUNCTION and RETURN
- Add nodes for CLASS and PACKAGE
- Use embedded ASTBlock structures
- Always initialize blocks

Difficulty: Easy to Medium

---

### 4. Runtime Interpreter

Description:
The runtime executes the AST.

Tasks:
- Fix string memory ownership (heap only)
- Implement boolean coercion rules
- Implement numeric coercion (int and float)
- Implement comparison operators
- Implement block execution
- Implement variable lookup and assignment

Difficulty: Medium

---

## IMPORTANT FEATURES (Language Usability)

These features make the language practical to use.

---

### 5. Else If Chains

Description:
Support multiple conditional branches without nesting.

Example:
IF (x == 1) { ... }
ELSE IF (x == 2) { ... }
ELSE { ... }

Tasks:
- Add ELSE IF token
- Add AST structure for else-if blocks
- Execute conditions in order
- Execute ELSE only if no condition matches

Difficulty: Medium

---

### 6. While Loop

Description:
Execute a block while a condition is true.

Example:
WHILE (i < 10) { ... }

Tasks:
- Add WHILE token
- Add AST_WHILE node
- Parse condition and body
- Execute loop in runtime

Difficulty: Medium

---

### 7. For Loop

Description:
Classic counted loop.

Example:
FOR (INT i = 0; i < 10; i = i + 1) { ... }

Tasks:
- Add FOR token
- Parse initializer, condition, increment
- Add AST_FOR node
- Execute loop correctly

Difficulty: Medium to Hard

---

### 8. Functions with Return

Description:
User-defined functions with parameters and return values.

Example:
FUNCTION add(a, b) { return a + b; }

Tasks:
- Add FUNCTION and RETURN tokens
- Add AST_FUNCTION_DEF and AST_RETURN nodes
- Implement call stack
- Implement local scopes
- Implement return propagation

Difficulty: Medium to Hard

---

## ADVANCED FEATURES (Powerful Language Features)

These features make KrisLang expressive and modern.

---

### 9. Namespaces and Packages

Description:
Organize code into modules like Java packages or PHP namespaces.

Example:
package utils.math;

Tasks:
- Add PACKAGE and IMPORT tokens
- Add AST_PACKAGE and AST_IMPORT nodes
- Implement module loader
- Resolve symbols using namespace paths
- Load files from directory structure

Difficulty: Hard

---

### 10. Classes and Objects

Description:
Object-oriented programming support.

Example:
class Person { STRING name; FUNCTION greet() { ... } }

Tasks:
- Add CLASS token
- Add AST_CLASS node
- Add AST nodes for properties and methods
- Implement object instances
- Implement this keyword
- Implement method dispatch

Difficulty: Hard

---

### 11. Lexical Scoping

Description:
Each block introduces a new variable scope.

Tasks:
- Implement symbol table stack
- Push scope on block entry
- Pop scope on block exit
- Resolve variables from inner to outer scopes

Difficulty: Hard

---

## TOOLING AND DEVELOPER EXPERIENCE

These features improve usability and professionalism.

---

### 12. Error Messages

Description:
Errors should be helpful and precise.

Tasks:
- Show line and column
- Print source line
- Print caret under error
- Add hints for common mistakes
- Support multiple errors per run

Difficulty: Medium to Hard

---

### 13. Standard Library

Description:
Built-in functions for common tasks.

Tasks:
- string.length
- string.split
- math.random
- math.floor
- file.read
- file.write

Difficulty: Medium

---

### 14. CLI Tooling

Description:
Command-line interface for KrisLang.

Tasks:
- kris run file.kris
- kris repl
- kris fmt
- kris lint (optional)

Difficulty: Medium to Hard

---

## FUTURE FEATURES (Long-Term Goals)

These features are optional and advanced.

---

### 15. Bytecode Virtual Machine

Description:
Compile AST to bytecode for performance.

Tasks:
- [ ] Design instruction set
- [ ] Implement stack-based VM
- [ ] Implement garbage collector
- [ ] Add optimizations

Difficulty: Very Hard

---

### 16. Package Manager

Description:
Install and manage external libraries.

Tasks:
- kris.json file
- Dependency resolver
- Package downloader
- Local cache

Difficulty: Very Hard

---

## Final Notes

This roadmap is designed to be incremental.
Each section builds on the previous one.
Focus on correctness and clarity before adding complexity.

KrisLang is intended to be a serious language project.
