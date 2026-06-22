# ZinterPL

**ZinterPL** is a lightweight, interpreted programming language written entirely in C, designed to run efficiently on constrained hardware — including the **ESP32** microcontroller. The interpreter is self-contained, low on memory footprint, and built with performance in mind.

Source files use the `.Zim` extension. Library files use `.Zlib`.

---

## Features

- Statically typed: `int`, `char`, `float`
- Variables, 1D arrays, and 2D matrices for all types
- Functions with arguments and return values — **typeless**: a function can return any scalar value (`int`, `char`, `float`)
- `if / oth if / oth` branching — any number of `oth if` chains
- `during` unified loop construct (count-style and condition-style) — both fully functional
- Extended `deven_` return expressions: arithmetic, power, square root, and nested function calls
- Function argument pass-by-reference: modifications made inside a called function are reflected back in the caller
- Function aliasing: rename any function with `newname --> oldname`
- `++N` / `--N` operations accept variables or function calls as `N`
- Input via `scan_`
- Built-in `status_` diagnostic command
- External library support via `.Zlib` files
- Built-in build test suite (runs automatically with no arguments)
- Debug mode togglable at runtime
- Designed to target ESP32 (minimal memory, no dynamic allocation beyond data structures)

---

## Usage

```bash
./Zinterpreter -df file.Zim
./Zinterpreter -dt file.Zim
./Zinterpreter -df file.Zim -libname.Zlib
```

| Flag | Meaning |
|------|---------|
| `-df` | Debug false (silent mode) |
| `-dt` | Debug true (verbose, prints internal state) |
| `-libname.Zlib` | Attach a `.Zlib` library (prefix with `-`) |

Running with no arguments executes the **built-in build test** and prints a pass/fail report.

```bash
./Zinterpreter
```

---

## Syntax Reference

Every instruction ends with `:` — this is the statement terminator.

### Program Entry Point

Every `.Zim` file must define `__start()`, which is the main entry point:

```
__start(){
    // your code here
}
```

> **Note:** the order of declarations in a `.Zim` file does not matter. The header block `#{...}`, `__start()`, and function definitions (`od_`) can appear in any order. The interpreter resolves all definitions before execution, so there is no requirement to declare functions before they are called, or to place the header at the top.

---

### Header Block `#{...}`

The optional header block at the top of the file configures the execution environment:

```
#{
    debug_ -df:
    exec_:
}
```

| Directive | Effect |
|-----------|--------|
| `debug_ -df:` | Disable debug output |
| `debug_ -dt:` | Enable debug output |
| `import_ &f&name.Zlib&:` | Import a library (legacy inline syntax) |
| `exec_:` | Start execution |

---

### Variable Declarations

```
int_  &i&varname&:
char_ &c&varname&:
```

#### Token format: `&TYPE&NAME&`

> **Note:** the `&type&name&` token syntax is **not required** in most contexts. You can reference variables, arrays and matrices directly by name in assignments, print statements, and expressions. The token syntax is mainly used when passing values as arguments or in ambiguous positions.

| Prefix | Type |
|--------|------|
| `&i&name&` | integer variable |
| `&c&name&` | char variable |
| `&l&name&` | float variable |
| `&n&value&` | numeric literal |
| `&k&C&` | char literal (e.g. `'C'`) |
| `&s&text&` | string literal |
| `&f&file&` | file reference |

---

### Arrays

```
int_  &i[5]&arrname&:
char_ &s[5]&arrname&:
```

Access:
```
[0]arrname = 42:
var = [0]arrname:
```

Index can be a literal or an integer variable:
```
[index]arrname = value:
```

---

### Matrices (2D arrays)

```
int_  &i[3][3]&matname&:
char_ &s[3][3]&matname&:
```

Access:
```
[0][0]matname = 42:
var = [1][2]matname:
[row][col]matname = var:
```

---

### Assignment

```
varname = value:
varname = other_var:
[idx]arrname = value:
[r][c]matname = var:
```

Math expressions are supported inline:
```
v2 = v0 + v1:
v2 = v0 - v1:
v2 = v0 * v1:
v2 = v0 / v1:
[2]arr = 5 * v1:
```

---

### Comparison operators

The standard comparison operators (`==`, `<`, `>`) work on scalar variables and on indexed array/matrix elements.

When `==` is applied to an **unindexed array or matrix name**, it compares their **declared size**, not the number of occupied slots:

```
int_  &i[5]&arrA&:
int_  &i[5]&arrB&:
int_  &i[3]&arrC&:

if(arrA == arrB){   // true  — both declared with size 5
    println_ &s&"same size"&:
}
if(arrA == arrC){   // false — 5 != 3
    println_ &s&"same size"&:
}
```

This also applies to matrices, where the declared total size (rows × cols) is compared.

---

### Increment / Decrement

```
varname++:        // +1
varname--:        // -1
varname++N:       // +N
varname--N:       // -N

[idx]arr++:
[idx]arr--N:
```

**`N` can be a literal, a variable, or a function call:**

```
int_ &i&step&:
step = 3:

plusplus++step:            // increment by the value of a variable
plusplus++__retv(5):       // increment by the return value of a function
```

**Special behavior on char arrays:** `++` and `++N` on a char array element do **not** perform numeric arithmetic. Instead:

- `[idx]arr++` — copies the character at `[idx]` into `[idx+1]`
- `[idx]arr++N` — copies the character at `[idx]` into `[idx+N]` only (not into every cell in between)

```
char_ &s[10]&arr&:
[2]arr = 'F':
[2]arr++:     // copies 'F' into [3] only
[2]arr++3:    // copies 'F' into [5] only  ([2] + 3 = [5])
```

On integer variables and arrays, `++` / `--` behave as standard numeric increment/decrement.

---

### Functions

Declare with `od_` (open door):

```
od_ funcname(){
    int_ &i&res&:
    res = 3:
    deven_ res:
}
```

- `deven_` is the return keyword (replaces `return`).
- `deven_:` with no argument returns void.

Functions are **typeless**: there is no return-type declaration. A function can return an `int`, a `char`, or a `float` — the interpreter handles the type at runtime. The only restriction is that **arrays and matrices cannot be returned** (see [Limitations](#limitations)).

#### Function with arguments

Arguments separated by `!`:

```
od_ testargs(&i&a&!&i&b&){
    int_&i&risultato&:
    risultato = a + b:
    deven_ risultato:
}
```

#### Calling functions

Void call:
```
__funcname():
```

Call with return value stored in a variable:
```
var = __funcname():
```

Call with arguments (separated by `!`):
```
print_ __testargs(10!20):
```

Store return value in array or matrix:
```
[0]arr = __funcname():
[1][2]mat = __funcname():
```

---

### Extended `deven_` expressions

`deven_` can return more than just a plain value. The following inline operations are supported directly on the return expression:

| Syntax | Effect |
|--------|--------|
| `deven_ val++:` | returns `val + 1` |
| `deven_ val--:` | returns `val - 1` |
| `deven_ val**:` | returns `val²` (squared) |
| `deven_ val~~:` | returns `√val` (square root) |
| `deven_ __func():` | returns the result of calling `func` |
| `deven_ __func(arg):` | returns the result of calling `func` with an argument |

Examples:

```
od_ valueplus(&i&inner&){
    deven_ inner++:       // returns inner + 1
}

od_ retfunc(){
    deven_ __differenza():  // delegates return to another function
}

od_ retretunc(&i&pass&){
    deven_ __valueplus(pass):  // chains two function calls in one return
}
```

This makes it easy to build concise functional pipelines:

```
print_ __retretunc(5):    // → valueplus(5) → 6
print_ __retfunc():       // → differenza() → 6
```

---

### Pass-by-reference for function arguments

When a variable is passed to a function as an argument, it is passed **by reference**: if the called function modifies it, the change is visible in the caller after the call returns.

This also propagates across call chains. A variable passed into function A, then forwarded to function B, will reflect changes made in B when A returns.

**Example from `Zcomple.Zim`:**

```
od_ addone(&i&value&){
    value++:
    __addtwo(value):
    deven_ value:
}

od_ addtwo(&i&value&){
    value++2:
    deven_:
}
```

Calling `__addone(5)`:

1. `addone` receives `value = 5`
2. `value++` → `value = 6`
3. `__addtwo(value)` is called — `addtwo` receives the same `value` and increments it by 2 → `value = 8`
4. Back in `addone`, `value` is now `8` (the change from `addtwo` propagated back)
5. `deven_ value` returns `8`

```
print_ __addone(5):   // prints 8
```

> ⚠️ **Limitation:** arrays and matrices cannot currently be passed as function arguments. This is the next planned feature.

---

### Function aliasing

A function can be given a new name using the `-->` operator. The new name becomes an alias for the original function:

```
newname --> oldname:
```

This works for plain function names as well as for array-indexed and matrix-indexed forms, depending on the type of entity being aliased:

```
myalias   --> originalfunc:     // function alias
[i]alias  --> [i]originalfunc:  // array-form alias
[r][c]alias --> [r][c]original: // matrix-form alias
```

> **Coming soon:** aliasing for plain scalar variables.

---

### Output

```
print_ value:           // print without newline
println_ value:         // print with newline
println_:               // print empty newline
```

`value` can be a variable name, an `&s&string&` literal, an array element, a matrix element, or a direct function call:

```
print_ __testargs(10!20):    // prints 30
```

---

### Input

```
scan_ &s&varname&:
```

---

### Conditionals

```
if(condition){
    // ...
}
oth if(condition){
    // ...
}
oth if(condition){
    // any number of oth if chains are supported
}
oth{
    // ...
}
```

Conditions support `==`, `<`, `>`, `s==` (string comparison).

You can chain **any number of `oth if`** blocks before the final `oth`.

**Standalone `oth` or `oth if` without a preceding `if`** are silently ignored — they do not crash or produce errors, they simply do not execute.

---

### Spaces and Whitespace

**Spaces are ignored everywhere** in ZinterPL source code, **except inside double quotes** (`"..."`). This means indentation and spacing are purely cosmetic and have no effect on parsing.

```
int_ &i&x&:        // same as
int_&i&x&:         // same as
int_   &i& x &:   // all equivalent
```

Inside a string literal, spaces are preserved:
```
println_ &s&"hello world"&:   // prints: hello world
```

---

### Variable Scope

Variables declared inside a function are **local to that function** and are not accessible from outside. Attempting to read a variable that does not exist in the current scope returns a zero/null value rather than crashing.

```
od_ myfunc(){
    int_ &i&local&:
    local = 42:
    deven_ local:
}

__start(){
    local = 0:       // 'local' not declared here — silently fails
    print_ local:    // prints 0
}
```

---

### Variable Indices

Array and matrix elements can be accessed using either literal indices or integer variables as indices:

```
[0]arr = 42:           // literal index
[idx]arr = 42:         // variable as index

[0][0]mat = 99:        // literal row and col
[row][col]mat = 99:    // variables as row and col
[row][0]mat = 99:      // mixed
```

---

### Inline C Code

You can define a C function directly inside a `.Zim` file using a `C{ }` block. The code inside is **compiled separately** as a standalone C function.

```
C{
    // C code here
    return 42;
}
```

To call it, use `__C(...)` passing variables, values, or arrays as arguments:

```
int_ &i&result&:
result = __C(myvar, myarr, 10):
```

The return value of `return` in the C block is used as the result of `__C(...)` in the ZinterPL code.

---

### Loops

ZinterPL uses a single unified loop construct: **`during`**, replacing the earlier separate `for_` / `while_` keywords.

There are two forms.

#### Count-style

Repeats a fixed number of times. The argument can be a numeric literal, an integer variable, or an array element:

```
during(5){
    println_ &s&"ciao"&:
}

during(repetition){
    println_ &s&"hello"&:
}

during([4]tion){
    println_ &s&"hi"&:
}
```

The loop runs exactly N times, where N is the value of the argument at the moment the loop starts. The argument is evaluated once at entry.

#### Condition-style

Behaves like a `while` loop. The first element is the condition; after the condition, any number of additional statements can be added, each separated by `!`. These extra statements are executed as steps at the end of every iteration:

```
during(condition ! step1):
during(condition ! step1 ! step2):
during(condition ! step1 ! step2 ! step3):
```

The condition uses the same syntax as `if` (`<`, `>`, `==`, etc.). Steps are typically increments or decrements but can be any valid ZinterPL statement.

```
during(repetition < 7 ! repetition++){
    println_ &s&"bye"&:
}

during(repetition < 6 ! repetition++2){
    println_ &s&"tre"&:
}

during(i < 10 ! i++ ! j-- ! k++2){
    println_ i:
}
```

The condition is re-checked after the body and all steps have run; the loop exits as soon as it evaluates to false.

---

### Diagnostic

```
status_:
```

Prints the current state of all declared variables, arrays, and matrices. Useful for debugging.

```
status_ cls y:
```

Clears internal state after printing (useful for resetting the VM snapshot mid-program).

---

### Libraries

Libraries are `.Zlib` files containing additional function definitions. Pass them on the command line:

```bash
./Zinterpreter -df file.Zim -libstdfn.Zlib
```

The interpreter merges the library into the source before parsing.

In-file import (legacy, may print a warning in some builds):
```
#import_&f&stdfn.Zlib&:
```

---

## Limitations

- **Arrays and matrices cannot be passed as function arguments.** Workaround: use global-style naming conventions or pass individual elements. Proper array-passing support is the next planned feature.
- **Arrays and matrices cannot be returned from functions** via `deven_`. Only scalar values (`int`, `char`, `float`) are returnable.
- **Function aliasing for scalar variables** is not yet supported. Coming soon.

---

## Minimal Example

```
#{
    debug_ -df:
    exec_:
}

__start(){
    status_:
}
```

---

## Complete Example

```
od_ somma(){
    int_ &i&a&:
    int_ &i&b&:
    a = 3:
    b = 5:
    int_ &i&res&:
    res = a + b:
    deven_ res:
}

__start(){
    int_ &i&result&:
    result = __somma():
    println_ result:
}
```

---

## Build & Run

```bash
gcc Zinterpreter.c -o Zinterpreter
./Zinterpreter -df myprogram.Zim
```

Run the built-in test suite:
```bash
./Zinterpreter
```

Output ends with:
```
build OK
```
or a count of failed tests if something is broken.

---

## Repository Structure

```
ZinterPL/
├── Zinterpreter.c       # The interpreter — single source file
├── code_test/           # Test programs (.Zim files)
└── stable_build/        # Archived stable interpreter builds
```

### `code_test/`

Contains `.Zim` test programs that cover all language features. These tests are version-agnostic: they are written against the ZinterPL language specification and are valid for every interpreter version, past and future.

### `stable_build/`

Contains archived builds of older interpreter versions. All newer versions of ZinterPL are **fully backwards compatible** — any `.Zim` file that ran correctly on an older build will run identically on a newer one.

---

## Project Status

| Feature | Status |
|---------|--------|
| Variables (int, char, float) | Stable |
| Arrays (1D) | Stable |
| Matrices (2D) | Stable |
| Functions + return values | Stable |
| Function arguments (scalars) | Stable |
| Function arguments (arrays/matrices) | Planned |
| Pass-by-reference for function args | Stable |
| Function aliasing (`-->`) | Stable |
| Variable aliasing (`-->`) | Planned |
| Extended `deven_` expressions (`++`, `--`, `**`, `~~`, function) | Stable |
| `++N` / `--N` with variable or function as `N` | Stable |
| Arithmetic expressions | Stable |
| Conditionals (if/oth if/oth) | Stable |
| Increment/Decrement | Stable |
| Library support (.Zlib) | Stable |
| Build test suite | Stable |
| `scan_` (input) | Partial |
| `during` loops (condition-style) | Stable |
| `during` loops (count-style) | Stable |
| ESP32 port | Planned |

---

## Memory Footprint

Measured by running `status_` on the actual interpreter (compiled with gcc, Ubuntu):

| State | RAM used |
|-------|----------|
| VM at startup, no variables | ~284 KB |
| After declaring int/char variables | ~284 KB (no change — fixed-size slots) |
| After small arrays (10–50 elements) | ~285–292 KB |
| After large arrays (100–1000 elements) | ~292–300 KB |
| After large matrices (100×100) | grows ~64 KB per matrix |

The base VM footprint is **~284 KB**, which covers the full fixed structure: variable slots, array slots, matrix slots, the program buffer, the state stack, and the register file. Memory only grows beyond that when large arrays or matrices are dynamically allocated.

This makes ZinterPL suitable for constrained environments like the **ESP32**, where the entire VM lives inside a single `struct VM` and can be statically allocated.

---

- **Minimal memory usage** — fixed-size data structures, no heap sprawl; suitable for microcontrollers
- **Self-contained** — single C source file, no external dependencies
- **ESP32 target** — the entire VM is a single `struct VM`, easy to port to embedded environments
- **Transparent execution** — debug mode prints every step of parsing and execution

---

*ZinterPL — a custom scripting language built in C, designed to go small.*
