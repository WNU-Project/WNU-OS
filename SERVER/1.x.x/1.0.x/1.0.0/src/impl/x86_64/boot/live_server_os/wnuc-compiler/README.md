# WNUC - WNU Compiler

## Overview
WNUC (WNU Compiler) is a custom programming language compiler designed specifically for WNU OS Server. It compiles WNU-LANG source code into optimized x86_64 assembly code that runs natively on WNU OS.

## WNU-LANG Language Specification

### Syntax
WNU-LANG is a simple, server-oriented programming language with the following features:

#### 1. Output Statements
```
print "Hello World"
print variable_name
```

#### 2. Variable Assignment
```
set variable_name = value
set port = 8080
set name = "WNU-Server"
```

#### 3. Conditional Statements
```
if condition then
    // statements
end
```

#### 4. Loop Statements
```
loop count times
    // statements  
end
```

#### 5. Comments
```
// This is a line comment
```

### Data Types
- **Strings**: Text enclosed in double quotes `"Hello"`
- **Numbers**: Integer values `42`, `8080`
- **Identifiers**: Variable names `server_port`, `max_connections`

### Keywords
- `print` - Output statement
- `set` - Variable assignment
- `if` - Conditional statement
- `then` - Conditional body start
- `end` - Block terminator
- `loop` - Loop statement
- `times` - Loop count specifier

## Compiler Architecture

### 1. Lexical Analysis (Tokenization)
- Breaks source code into tokens
- Identifies keywords, identifiers, strings, numbers
- Handles whitespace and comments

### 2. Syntax Analysis (Parsing)
- Uses recursive descent parser
- Builds Abstract Syntax Tree (AST)
- Enforces WNU-LANG grammar rules

### 3. Semantic Analysis
- Type checking
- Variable scope resolution
- Dead code elimination
- Optimization passes

### 4. Code Generation
- Emits x86_64 assembly code
- Generates system calls for WNU OS
- Optimizes register usage

## Usage

### From WNU OS Shell
```
root@live:~# wnuc
```

This will:
1. Display compiler information
2. Compile the default sample program
3. Generate `hello_world.asm` output file
4. Show compilation status

### Sample Program
See `sample.wnulang` for a complete example demonstrating all language features.

### Generated Assembly
The compiler generates optimized assembly code with:
- Proper system call interfaces
- Memory management
- String handling routines
- Control flow structures

## Implementation Details

### Token Types
- `TOKEN_KEYWORD` - Language keywords
- `TOKEN_IDENTIFIER` - Variable names
- `TOKEN_STRING` - String literals  
- `TOKEN_NUMBER` - Numeric literals
- `TOKEN_OPERATOR` - Assignment and comparison operators

### AST Nodes
- `NODE_PRINT` - Print statements
- `NODE_ASSIGN` - Variable assignments
- `NODE_IF` - Conditional statements
- `NODE_LOOP` - Loop statements
- `NODE_BLOCK` - Statement blocks

### Memory Layout
- **Token Buffer**: 4KB for tokenization
- **Output Buffer**: 8KB for generated assembly
- **Symbol Table**: Variable and function definitions
- **AST Storage**: Parse tree representation

## Technical Features

### Optimization Passes
1. **Constant Folding** - Evaluate compile-time constants
2. **Dead Code Elimination** - Remove unreachable code
3. **Register Allocation** - Efficient register usage
4. **String Interning** - Deduplicate string literals

### Error Handling
- Comprehensive error messages
- Line number reporting
- Syntax error recovery
- Semantic error detection

### Target Platform
- **Architecture**: x86_64
- **Operating System**: WNU OS Server
- **Calling Convention**: System V ABI
- **Output Format**: NASM-compatible assembly

## Future Enhancements

### Planned Features
- Function definitions and calls
- Array and structure support
- File I/O operations
- Network programming primitives
- Inline assembly support
- Debugging information generation

### Advanced Optimizations
- Loop unrolling
- Instruction scheduling
- Profile-guided optimization
- Link-time optimization

## Examples

### Hello World
```
print "Hello, WNU OS!"
```

### Server Configuration
```
set port = 8080
set max_clients = 100

if port > 8000 then
    print "Using secure port"
end

print "Server configured successfully"
```

### Processing Loop
```
set count = 10

loop count times
    print "Processing request..."
end

print "All requests processed"
```

## Compiler Invocation
The WNUC compiler is integrated into the WNU OS Server Live system and can be invoked using the `wnuc` command from the WNU-SH shell. It demonstrates modern compiler construction techniques adapted for a custom operating system environment.