# Zinter & Zinterpreter

> A hand-crafted scripting language and its interpreter — built from scratch in C.

---

## What is Zinter?

**Zinter** is a custom scripting language designed around a simple, unambiguous token syntax. Every data reference is wrapped in ampersand-delimited tokens (`&type&name&`), making the language trivially parseable without a traditional lexer — by design.

**Zinterpreter** is the runtime that brings Zinter to life. Written entirely in C, it reads, formats, and executes Zinter source files through a hand-written parser and a lightweight virtual machine with its own memory model, state stack, and execution pipeline.

This is not a toy calculator or an academic exercise. Zinter is a **fully typed, multi-scope scripting language** with variables, arrays, matrices, user-defined functions, control flow, I/O, and a system configuration block — all interpreted at runtime with no external dependencies.

---

## Why does it exist?

Most scripting languages are built on top of existing runtimes, compiler frameworks, or parsing libraries. Zinter and Zinterpreter were built to understand what happens *underneath* — how a language goes from raw text on disk to executing instructions, how a VM tracks state, how memory is laid out, and how a parser dispatches execution across a program's structure.

The result is a minimal but genuinely functional language implementation you can read, modify, and extend — with every design decision visible in the source.

---

## How it works

Execution goes through four stages:

1. **Read** — source code is read from a `.Zinter` file, stripping whitespace and normalizing the input into a flat character buffer.
2. **Format** — the buffer is split into `program_line` entries, one per instruction, delimited by `:`, `{`, and `}`.
3. **Build state** — a pre-pass walks all lines and builds a `state_stack`, mapping every block (`od_`, `if_`, `for_`, `while_`, `#{}`, `__start{}`) to its opening line, closing line, and name. No block-tracking is needed at runtime.
4. **Parse & execute** — `parse()` walks the program from the entry point, dispatching each instruction to the appropriate handler based on its prefix.

The VM struct (`VM`) holds all runtime state: variables, arrays, matrices, registers, the program buffer, the state stack, and execution pointers. A single global `vm` instance is used, with `#define` aliases for ergonomic access.

---

## Language overview

### Token syntax

Every data reference in Zinter follows a consistent pattern:

```
&type&name&
&type[index]&name&
&type[row][col]&name&
```

| Type prefix | Meaning                        |
|-------------|--------------------------------|
| `i`         | integer variable or array      |
| `l`         | float variable or array        |
| `c`         | char variable                  |
| `s`         | char array / string            |
| `n`         | immediate integer literal      |
| `k`         | immediate char literal         |
| `f`         | file (for import)              |

---

### Minimal syntax — writing without token wrappers

The explicit `&type&name&` form is always valid, but in most contexts it is entirely optional. Zinterpreter calls `is_what()` at runtime to look up a bare name across all declared variables, arrays, and matrices, and infers the correct type automatically.

This means the two styles are fully equivalent everywhere except declarations:

```
# explicit token syntax
print_ &i&myvar&:
[0]myarray = &i&myvar&:
println_ &s&"hello"&:

# minimal syntax — same result
print_ myvar:
[0]myarray = myvar:
println_ "hello":
```

The only place where the explicit form is **required** is in `int_` and `char_` declarations, because the type prefix carries the shape of the data being declared (scalar, array, or matrix) and Zinterpreter has no other way to know it before the name exists:

```
int_ &i&myvar&:          # required: declares a scalar int
int_ &i[10]&myarray&:    # required: declares an int array of size 10
int_ &i[4][4]&mymatrix&: # required: declares a 4×4 int matrix
```

Everywhere else — assignments, I/O, function arguments, indices — you can drop the wrappers and just use the name, with optional `[idx]` or `[row][col]` prefixes for arrays and matrices:

```
print_ myvar:
print_ [2]myarray:
print_ [1][3]mymatrix:
[0]myarray = myvar:
[0][1]mymatrix = [2]myarray:
```

---

### Space handling

Zinter has a deliberately simple rule: **spaces are meaningless outside of string literals.**

During the read phase, the entire source file is compacted into a continuous buffer — every space outside of `"..."` is dropped before a single instruction is parsed. This means indentation, alignment, and spacing are purely cosmetic and have zero effect on execution:

```
# these three lines are identical to Zinterpreter
int_ &i&myvar&:
int_   &i&  myvar  &:
i n t _ & i & m y v a r & :
```

Inside a string literal delimited by double quotes, spaces are preserved exactly as written:

```
println_ &s&"hello world"&:   # prints: hello world
println_ &s&"helloworld"&:    # prints: helloworld
```

The practical benefit is that source files can be written in any style — compact, heavily indented, spread across lines — and the interpreter treats them identically. There is no significant whitespace, no indentation requirement, and no line-length concern.

---

### Declarations

```
int_ &i&myvar&:
int_ &i[10]&myarray&:
int_ &i[4][4]&mymatrix&:

char_ &c&letter&:
char_ &s[32]&mystring&:
```

---

### Assignment

```
myvar = othervar:
[2]myarray = [0]otherarray:
[1][3]mymatrix = 99:
```

---

### I/O

```
print_ &i&myvar&:
print_ [2]myarray:
println_ &s&"Hello, world"&:
lnprint_ myvar:
scan_ &i&myvar&:
```

---

### Functions

User-defined functions are declared with `od_` and called with `__`:

```
od_ add(a,b){
    return_ a + b:
}

result = __add(var0, var1):
```

Functions support multiple return values via `--` piping, and `return NULL` is valid for void-style calls.

---

### Control flow

```
if_(var1 == var2){
    ...
}
else_{
    ...
}

for_(i != 10, + &i&i& &n&1&){
    ...
}

while_(var1 != var2){
    ...
}
```

Conditions support `==`, `!=`, `<`, `>` and can be chained with `&&` and `||`.

---

### System block

The `#{}` block runs before `__start` and handles interpreter-level configuration:

```
#{
    debug_ -df:
    import_ &f&mylib.Zlib&:
    exec_:
}
```

`exec_:` transfers control to `__start{}`, the program's entry point.

---

## Project structure

```
Zinterpreter.c       — full interpreter source (single-file)
docs/                — language reference and syntax specification
*.Zinter             — example programs
```

---

## Building and running

```bash
gcc Zinterpreter.c -o Zinterpreter
./Zinterpreter -dt myprogram.Zinter
```

| Flag | Effect              |
|------|---------------------|
| `-dt` | enable debug output |
| `-df` | disable debug output |

Running with no arguments executes the built-in test suite, which validates variables, arrays, matrices, and `get_index` resolution.

---

## What it currently supports

- Integer, float, and char variables
- Integer, float, and char arrays (with bounds checking)
- Integer, float, and char matrices (2D)
- User-defined functions with arguments and return values
- Recursive-capable state stack
- Assignment between any combination of variables, array cells, and matrix cells
- Type checking at assignment time
- Immediate literals (numbers and chars) in assignments and I/O
- Variable-indexed array and matrix access
- Print / scan / println / lnprint / lnprintln
- System configuration block (`#{}`)
- Comment lines (`//`)
- Built-in test suite (`run_test()`)

## What is not yet implemented

- `if` / `else` / `for` / `while` runtime execution (state tracking is built; dispatch hooks are in place)
- `scan_` input handling
- Library imports (`.Zlib`)
- Float declarations via the `float_` keyword (currently handled through `set_to_variable` with type `l`)

---

## Design notes

**No external dependencies.** The entire interpreter is a single `.c` file. No lex, no yacc, no LLVM, no runtime library beyond the C standard library.

**Predictable parsing.** Because every token is delimited by `&`, the parser never needs to backtrack. `sscanf` patterns are sufficient to extract all relevant data from any instruction.

**Typed from the ground up.** Every variable, array cell, and matrix cell carries an explicit type at declaration time. Assignments are type-checked before execution, with compatible-type promotions (e.g. `n` → `i`, `k` → `c`) handled explicitly.

**Separation of structure and execution.** `build_state()` runs as a pre-pass and populates the full state stack before any instruction executes. The parser never needs to scan forward to find a closing brace or a function definition — it already knows where everything is.

---

## Status

Active development. Core infrastructure is stable. Control flow and scan are the next milestone.

---

*Zinter and Zinterpreter are original work. The language design, token syntax, VM architecture, and interpreter are written from scratch.*
