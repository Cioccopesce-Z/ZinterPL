# ZinterPL

## Alwayse refer to this file for sintax no other sintax file are updated before this.

**ZinterPL** is a lightweight, interpreted programming language written entirely in C, designed to run efficiently on constrained hardware — including the **ESP32** microcontroller. The interpreter is self-contained, low on memory footprint, and built with performance in mind.

Source files use the `.Zim` extension. Library files use `.Zlib`.

---

## Features

- Statically typed: `int`, `char`, `float` — unified declaration syntax for all three
- Variables, 1D arrays, and 2D matrices for all types
- **Dynamic variables** (`$name`) — type is set at declaration but switches automatically at runtime on assignment
- Static type safety on regular (non-dynamic) variables — a mismatched-type assignment is a fatal error
- Functions with arguments and return values — **typeless**: a function can return any scalar value (`int`, `char`, `float`)
- Default values for scalar function arguments
- `if / oth if / oth` branching — any number of `oth if` chains, and `if` alone (with no `oth`) is valid
- `during` unified loop construct (count-style and condition-style) — both fully functional
- Extended `deven_` return expressions: arithmetic, power, square root, and nested function calls
- Function argument pass-by-reference: modifications made inside a called function are reflected back in the caller
- Renaming via `->`: permanently rename any data — functions, variables, arrays, and matrices
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

Every instruction ends with `:` — this is the statement terminator. **This includes comments.**

### Comments

Comments begin with `//` and must also end with `:`, like every other statement:

```
// this is a comment:
// another comment:
```

Omitting the `:` at the end of a comment is a syntax error.

---

### Program Entry Point

Every `.Zim` file must define `__start()`, which is the main entry point:

```
__start(){
    // your code here:
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
| `exec_:` | Start execution |

> **Note:** the `import_` directive is **obsolete** and should not be used in new code. Attach libraries via the command line: `./Zinterpreter -df file.Zim -libname.Zlib`.

---

### Variable Declarations

Declarations no longer use the `&type&name&` token form. Each scalar type has its own keyword, followed directly by the variable name:

```
int varname:
char varname:
float varname:
```

`float` variables are now declared with the exact same form as `int` and `char` — there is no separate `float_` syntax to learn.

#### Token format: `&TYPE&NAME&`

> **Note:** the `&type&name&` token syntax is **no longer used to declare** variables, arrays, or matrices — declarations use the plain form shown above (`int name:`, `char [5]name:`, etc.). The token syntax is still used for **function argument declarations** (see [Functions](#functions)) and for **literal values** — numbers, chars, strings — inside expressions, print statements, and function calls.

| Token | Meaning | Used for |
|-------|---------|----------|
| `&i&name&` | integer | function arguments |
| `&c&name&` | char | function arguments |
| `&l&name&` | float | function arguments |
| `&a&name&` | array | function arguments |
| `&m&name&` | matrix | function arguments |
| `&n&value&` | numeric literal | literals in expressions |
| `&k&C&` | char literal (e.g. `'C'`) | literals in expressions |
| `&s&text&` | string literal | literals in expressions / print |
| `&f&file&` | file reference | literals in expressions |

---

### Arrays

```
int  [5]arrname:
char [5]arrname:
float [5]arrname:
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
int   [3][3]matname:
char  [3][3]matname:
float [3][3]matname:
```

Access:
```
[0][0]matname = 42:
var = [1][2]matname:
[row][col]matname = var:
```

---

### Type Safety

Regular (non-dynamic) variables, array elements, and matrix elements are statically typed once declared. Assigning a value of a **different** type — `int` → `float`, `char` → `int`, and so on — is a **fatal error**: the interpreter reports it and execution stops immediately.

```
int a:
a = 5:       // OK:
a = 2.71:    // FATAL ERROR — a is int, 2.71 is a float; program stops:
```

This applies to array and matrix elements too, not just scalars. **Dynamic variables** are the one exception to this rule — see below.

---

### Dynamic Variables (`$`)

A **dynamic variable** is any variable, array, or matrix whose name contains a `$`. By convention the `$` goes immediately before the name. Declaration uses the exact same syntax as a regular variable, array, or matrix — the `$` is simply part of the name:

```
int $dvar:
char $charvar:
int  [5]$temp:
char [4]$vdarr:
char [4][4]$vdar:
```

Access and assignment also use the same syntax as regular variables/arrays/matrices — nothing new there:

```
$dvar = 5:
[4]$temp = 7:
[3][3]$vdar = 'k':
```

#### Behavior

The declared type of a dynamic variable is only its **starting** type. Unlike a regular variable, a dynamic variable — or a single element of a dynamic array/matrix — is allowed to change type at runtime instead of raising the fatal error described in [Type Safety](#type-safety):

- Assigning a value of a different type does **not** error.
- Instead, the variable (or the specific array/matrix element) is **reset** and its type switches to whatever type the new value requires.

This makes dynamic variables useful wherever a value's type can't be known ahead of time.

#### Example

```
//====== DYNAMIC VAR TEST ============:
println &s&"====== DYNAMIC VAR TEST ============"&:

int $dvar:
$dvar = 5:
print $dvar: println &s&" 5 int $dvar = 5"&: println:
$dvar = 'k':
print $dvar: println &s&" k (int to char) "&:

$dvar = 5:
$dvar = 2.71:
print $dvar: println &s&" 2.71 (int to float)"&:

println:
$dvar = 'j':
print $dvar: println &s&" j (float to char) "&:

$dvar = 2.71:
$dvar = 22:
print $dvar: println &s&" 22 (float to int)"&:

$dvar = 'j':

println:
$dvar = 6:
print $dvar: println &s&" 6 (char to int)"&:

$dvar = 'o':
$dvar = 4.6:
print $dvar: println &s&" 4.6 (char to float)"&:

println:
char $charvar:
$charvar = 'k':
print $charvar: println &s&" k $charvar = k "&:
$charvar = $dvar:
print $charvar: println &s&" 4.6 $charvar = $dvar(float) "&:
$charvar = 2 + 5:
print $charvar: println &s&" 7 $charvar = 2 + 5"&:

println:
int [5]$temp:
[4]$temp = 7:
print [4]$temp: println &s&" 7 [4]$temp = 7"&:
[4]$temp = 'u':
print [4]$temp: println &s&" u array(int to char)"&:
[3]$temp = 3.55:
print [3]$temp: println &s&" 3.55 array(char to float)"&:
print [4]$temp: println &s&" 0 array_float vuoto"&:
[1]$temp = $charvar:
print [1]$temp: println &s&" 7 array(float to int) -> $charvar"&:

println:
char [4]$vdarr:
[3]$vdarr = 'd':
print [3]$vdarr: println &s&" d char [3]$vdarr"&:
[3]$vdarr = [1]$temp:
print [3]$vdarr: println &s&" 7 array->array(char to int)"&:

println:
char [4][4]$vdar:
[3][3]$vdar = 'k':
print [3][3]$vdar: println &s&" k char [3][3]$vdar"&:
[3][3]$vdar = [3]$vdarr:
print [3][3]$vdar: println &s&" 7 matrix->array(char to int)"&:
```

This walks a single dynamic scalar (`$dvar`) through int → char → float → char → int → char → float, a dynamic char scalar (`$charvar`) receiving values from another dynamic variable and from an expression, a dynamic array (`$temp`), and cross-assignment between a dynamic array, a dynamic char array (`$vdarr`), and a dynamic char matrix (`$vdar`) — each read/write resolving to whatever type the source value actually is.

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

The standard comparison operators (`==`, `<`, `>`,'^=') work on scalar variables and on indexed array/matrix elements.

When `==` is applied to an **unindexed array or matrix name**, it compares their **declared size**, not the number of occupied slots:

```
int [5]arrA:
int [5]arrB:
int [3]arrC:

if(arrA == arrB){   // true  — both declared with size 5:
    println &s&"same size"&:
}
if(arrA == arrC){   // false — 5 != 3:
    println &s&"same size"&:
}
```

This also applies to matrices, where the declared total size (rows × cols) is compared.

---

### Increment / Decrement

```
varname++:        // +1:
varname--:        // -1:
varname++N:       // +N:
varname--N:       // -N:

[idx]arr++:
[idx]arr--N:
```

**`N` can be a literal, a variable, or a function call:**

```
int step:
step = 3:

plusplus++step:            // increment by the value of a variable:
plusplus++__retv(5):       // increment by the return value of a function:
```

**Special behavior on char arrays:** `++` and `++N` on a char array element do **not** perform numeric arithmetic. Instead:

- `[idx]arr++` — copies the character at `[idx]` into `[idx+1]`
- `[idx]arr++N` — copies the character at `[idx]` into `[idx+N]` only (not into every cell in between)

```
char [10]arr:
[2]arr = 'F':
[2]arr++:     // copies 'F' into [3] only:
[2]arr++3:    // copies 'F' into [5] only  ([2] + 3 = [5]):
```

On integer variables and arrays, `++` / `--` behave as standard numeric increment/decrement.

---

### Functions

Declare with `od_` (open door):

```
od_ funcname(){
    int res:
    res = 3:
    deven_ res:
}
```

- `deven_` is the return keyword (replaces `return`).
- `deven_:` with no argument returns void.

Functions are **typeless**: there is no return-type declaration. A function can return an `int`, a `char`, or a `float` — the interpreter handles the type at runtime. The only restriction is that **arrays and matrices cannot be returned** (see [Limitations](#limitations)).

#### Function with arguments

Function argument declarations keep the original `&type&name&` token form — this did **not** change:

```
od_ testargs(&i&a& ! &i&b&){
    int risultato:
    risultato = a + b:
    deven_ risultato:
}
```

#### Default argument values

A scalar argument can be given a **default value** by placing the value after the argument token, separated by a space. When the function is called without providing that argument, the default is used instead.

```
od_ defaultest(&i&var0& 5){
    print &s&" "&:
    print var0:
    deven_:
}

__start(){
    __defaultest():      // var0 uses default → prints 5:
    __defaultest(10):    // var0 = 10 → prints 10:
}
```

The default value can be a numeric literal or a variable name accessible in the caller's scope at call time.

**Rules for default arguments:**

- Default arguments must appear **at the end** of the argument list. Non-default arguments cannot follow a default argument.
- **Arrays and matrices cannot have default values** — only scalar types (`int`, `char`, `float`) can.
- There is a maximum of **16 arguments** per function (this limit can be raised by changing the constant in the interpreter source).
- When calling a function, if you omit one default argument you must omit **all remaining** default arguments after it — you cannot skip one and provide the next. Doing so is a parse error.

```
// ✓ correct — default is last:
od_ myfunc(&i&var& ! &a&arr& ! &i&varr& 5){}

// ✗ wrong — default is not at the end:
od_ myfunc(&i&var& 5 ! &a&arr& ! &i&varr&){}

// ✗ wrong — arrays cannot have defaults:
od_ myfunc(&i&var& ! &a&arr& 0 ! &i&varr&){}
```

Multiple trailing defaults are allowed:

```
od_ multi(&i&a& ! &i&b& 10 ! &i&c& 20){
    // b defaults to 10, c defaults to 20:
    deven_ a:
}

__start(){
    __multi(1):         // a=1, b=10, c=20:
    __multi(1 ! 2):     // a=1, b=2,  c=20:
    __multi(1 ! 2 ! 3): // a=1, b=2,  c=3:
}
```

---

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
print __testargs(10!20):
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
    deven_ inner++:       // returns inner + 1:
}

od_ retfunc(){
    deven_ __differenza():  // delegates return to another function:
}

od_ retretunc(&i&pass&){
    deven_ __valueplus(pass):  // chains two function calls in one return:
}
```

This makes it easy to build concise functional pipelines:

```
print __retretunc(5):    // → valueplus(5) → 6:
print __retfunc():       // → differenza() → 6:
```

---

### Pass-by-reference for function arguments

In ZinterPL, **all function arguments are passed by reference** — scalars, arrays, and matrices alike. This means a function never receives a copy of its argument: it receives a direct alias to the original. Any modification made inside the called function is immediately and permanently visible in the caller after the call returns.

This propagates across the full call chain. If function A passes a variable to function B, and B modifies it, the change is visible in A when B returns — and visible in whoever called A as well.

#### Scalar pass-by-reference

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
3. `__addtwo(value)` — `addtwo` receives the same alias and increments by 2 → `value = 8`
4. Back in `addone`, `value` is now `8` (the change from `addtwo` propagated back)
5. `deven_ value` returns `8`

```
print __addone(5):   // prints 8:
```

#### Array and matrix pass-by-reference

Arrays and matrices follow the exact same rule. When an array is passed to a function, the function works directly on the original array in the caller's scope — there is no copy. Writing to any element inside the function writes through to the original. The type tokens for arrays and matrices passed as arguments are `&a&` and `&m&` respectively; the interpreter resolves the actual element type automatically.

```
od_ fill(&a&arr&){
    [0]arr = 10:
    [1]arr = 20:
    [2]arr = 30:
    deven_:
}

__start(){
    int [5]data:
    __fill(data):
    println [0]data:    // prints 10:
    println [1]data:    // prints 20:
    println [2]data:    // prints 30:
}
```

The same applies to matrices:

```
od_ zero_matrix(&m&mat&){
    [0][0]mat = 0:
    [1][1]mat = 0:
    [2][2]mat = 0:
    deven_:
}
```

#### Planned: opt-in pass-by-copy (`cp`)

> ⚠️ **Planned — not yet implemented.** This is the next objective for the pass-by-reference system.

By default, every argument will keep working exactly as described above (passed by reference). A future version will let a per-argument override be declared by writing the keyword `cp` immediately before the argument's type token, **in the function declaration only** — never at the call site:

```
od_ ref_vs_copy(&i&rn& ! cp &i&cn& ! &c&rc& ! cp &c&cc& ! &a&rarr& ! cp &a&carr& ! &m&rmat& ! cp &m&cmat&){
    // rn, rc, rarr, rmat  -> passed by reference (default, unchanged):
    // cn, cc, carr, cmat  -> passed by copy (cp):
}
```

- `cp` will work on scalars, arrays, and matrices alike.
- `cp` only ever appears where the argument is **declared**. The caller doesn't write anything special — `__ref_vs_copy(a ! b ! c ! d ! e ! f ! g ! h)` looks the same regardless of which parameters are `cp` and which aren't.
- No `cp` → same as today: the parameter is an alias, modifications propagate back to the caller.
- `cp` → the parameter is an independent local copy: modifications inside the function stay local and are **not** reflected back in the caller.

#### Planned: `$` required for modifiable pass-through

Tied to the same objective: to pass a variable so it can be modified through the reference **including changing type**, the parameter name in the callee's declaration will need to contain `$`, the same marker used for [dynamic variables](#dynamic-variables-). Without `$` in the parameter name, a by-reference parameter can still be written to, but stays subject to the ordinary [static type-safety rule](#type-safety): writing a mismatched type through it is a fatal error, same as assigning it directly. Naming the parameter with `$` lifts that restriction, so the original variable can switch type through the call the same way a dynamic variable can on its own.

> Note: this interaction between `cp`/ref and `$` is still being worked out — treat the exact rule above as a working draft to confirm once implemented, not a locked spec.

#### ⚠️ Side-effect trap with `deven_` expressions

Because all arguments are aliases, any `deven_` expression that modifies the argument in place **also modifies the original variable in the caller**. This is the most common source of bugs with pass-by-reference.

`deven_ n**` does not compute a temporary square — it modifies `n` in place, and since `n` is an alias of whatever was passed, the caller's variable is overwritten too.

**Wrong — destroys the loop variable:**

```
od_ sq(&i&n&){
    deven_ n**:     // n** modifies n in place — the caller's i gets overwritten:
}

__start(){
    int i:
    i = 1:
    during(i < 9 ! i++){
        print __sq(i):    // after sq(3): i becomes 9 in __start, loop breaks:
    }
}
```

**Correct — use a local copy:**

```
od_ sq(&i&n&){
    int tmp:
    tmp = n:        // copy the value into a local variable:
    tmp**:          // square the local copy — n (and the caller's variable) untouched:
    deven_ tmp:
}
```

The rule is simple: if a `deven_` expression uses `**`, `~~`, `++`, or `--` on a parameter, always copy the parameter into a local variable first and operate on that.

##### Planned interaction with `cp`

Once `cp` (above) is implemented, it removes the need for the manual `tmp` workaround: declaring the parameter itself as `cp` makes it a genuine local copy, so a `deven_` expression using `**`, `~~`, `++`, or `--` on that parameter only touches the local copy — the caller's original variable stays untouched, with no extra `tmp` variable required.

```
od_ sq(cp &i&n&){
    deven_ n**:     // n is cp — this changes only the local copy, caller's variable is untouched:
}
```

By-reference parameters (no `cp`) keep today's behavior: `deven_` expressions still mutate them in place, since they remain aliases, so the manual `tmp` copy pattern is still needed for those.

---

### Renaming (`->`)

The `->` operator permanently renames any named entity in the VM: **functions, scalar variables, arrays, and matrices**. After the rename the entity is only accessible under the new name; the old name is no longer valid.

```
newname -> oldname:
```

#### Functions

```
newname -> originalfunc:           // function rename:
[i]newname  -> [i]originalfunc:    // array-indexed function form:
[r][c]newname -> [r][c]original:   // matrix-indexed function form:
```

#### Scalar variables

```
int score:
score = 42:
punti -> score:
println punti:    // prints 42:
println score:    // score no longer exists — returns 0:
```

#### Arrays

```
int [5]data:
[0]data = 99:
numeri -> data:
println [0]numeri:    // prints 99:
println [0]data:      // data no longer exists — returns 0:
```

#### Matrices

```
int [3][3]grid:
[0][0]grid = 7:
tavola -> grid:
println [0][0]tavola:    // prints 7:
println [0][0]grid:      // grid no longer exists — returns 0:
```

In all cases the rename is immediate and permanent for the remainder of the program's execution.

---

### Output

```
print value:        // print without newline:
println value:      // print with newline after the value:
println:            // print empty newline:
lnprintln value:     // print a newline, THEN the value, THEN another newline:
```

`lnprintln` is like `println`, but it also prints a leading newline before the value:

```
println value:      // → "value\n"
lnprintln value:     // → "\nvalue\n"
```

`value` can be a variable name, an `&s&string&` literal, an array element, a matrix element, or a direct function call:

```
print __testargs(10!20):    // prints 30:
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
    // ...:
}
oth if(condition){
    // ...:
}
oth if(condition){
    // any number of oth if chains are supported:
}
oth{
    // ...:
}
```

Conditions support `==`, `<`, `>`. String comparison (`s==`) is planned.

You can chain **any number of `oth if`** blocks before the final `oth`.

An `if` block does **not** require any `oth` or `oth if` after it — a plain `if` on its own is completely valid and will not crash the program.

**Standalone `oth` or `oth if` without a preceding `if`** are silently ignored — they do not crash or produce errors, they simply do not execute.

---

### Spaces and Whitespace

**Spaces are ignored everywhere** in ZinterPL source code, **except inside double quotes** (`"..."`). This means indentation and spacing are purely cosmetic and have no effect on parsing.

```
int x:          // canonical:
int    x:       // extra spaces are ignored:
int x   :       // trailing spaces before ':' are ignored too:
```

Inside a string literal, spaces are preserved:
```
println &s&"hello world"&:   // prints: hello world:
```

---

### Variable Scope

Variables declared inside a function are **local to that function** and are not accessible from outside.

Variables declared inside `__start()` are **global**: they are accessible from any function called during program execution.

```
__start(){
    int counter:    // global — visible to all functions called from here:
    counter = 10:
    __increment():
    println counter:    // prints 11:
}

od_ increment(){
    counter++:           // accesses the global 'counter' declared in __start:
    deven_:
}
```

Variables declared inside any other function (`od_`) are local to that function and are not accessible from outside. Attempting to read a variable that does not exist in the current scope returns a zero/null value rather than crashing.

```
od_ myfunc(){
    int local:
    local = 42:
    deven_ local:
}

__start(){
    local = 0:       // 'local' not declared here — silently fails:
    print local:    // prints 0:
}
```

---

### Variable Indices

Array and matrix elements can be accessed using either literal indices or integer variables as indices:

```
[0]arr = 42:           // literal index:
[idx]arr = 42:         // variable as index:

[0][0]mat = 99:        // literal row and col:
[row][col]mat = 99:    // variables as row and col:
[row][0]mat = 99:      // mixed:
```

---

### Inline C Code

> ⚠️ **Planned — not yet implemented.**

A future version will allow defining a C function directly inside a `.Zim` file using a `C{ }` block. The code inside will be compiled separately as a standalone C function.

```
C{
    // C code here:
    return 42;
}
```

To call it, use `__C(...)` passing variables, values, or arrays as arguments:

```
int result:
result = __C(myvar, myarr, 10):
```

The return value of `return` in the C block will be used as the result of `__C(...)` in the ZinterPL code.

---

### Loops

ZinterPL uses a single unified loop construct: **`during`**, replacing the earlier separate `for_` / `while_` keywords.

There are two forms.

#### Count-style

Repeats a fixed number of times. The argument can be a numeric literal, an integer variable, or an array element:

```
during(5){
    println &s&"ciao"&:
}

during(repetition){
    println &s&"hello"&:
}

during([4]tion){
    println &s&"hi"&:
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
    println &s&"bye"&:
}

during(repetition < 6 ! repetition++2){
    println &s&"tre"&:
}

during(i < 10 ! i++ ! j-- ! k++2){
    println i:
}
```

The condition is re-checked after the body and all steps have run; the loop exits as soon as it evaluates to false.

---

### Diagnostic

```
status_:
```

Prints the current state of all declared variables, arrays, and matrices. Useful for debugging.

#### Clearing state

Adding a clear keyword causes `status_` to **wipe the VM state** after printing. The accepted keywords are `cls`, `clear`, and `clr` — all three are equivalent. The optional flag `y` can appear anywhere in the statement, in any position relative to the clear keyword:

```
status_ cls:
status_ clear:
status_ clr:
status_ cls y:
status_ y cls:
status_ y clear:
```

> ⚠️ After a clear, all variables, arrays, and matrices that were declared before the `status_` call are **no longer accessible**. Their memory is wiped and any subsequent reads will return zero/null. Only declare new variables after the clear if you need them.

---

### Libraries

Libraries are `.Zlib` files containing additional function definitions. Pass them on the command line:

```bash
./Zinterpreter -df file.Zim -libstdfn.Zlib
```

The interpreter merges the library into the source before parsing.

> **Note:** the in-file `import_` directive is **obsolete** and should not be used in new code. Always attach libraries via the command line.

---

## Limitations

- **Arrays and matrices cannot be returned from functions** via `deven_`. Only scalar values (`int`, `char`, `float`) are returnable.
- **Arrays and matrices cannot have default argument values.** Only scalar parameters support defaults.
- **Maximum 16 arguments per function** (adjustable by changing the limit constant in the interpreter source).
- **Regular (non-dynamic) variables, array elements, and matrix elements cannot change type after declaration** — assigning a mismatched type is a fatal error. Use a dynamic variable (`$name`) if the type needs to vary at runtime.

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
    int a:
    int b:
    a = 3:
    b = 5:
    int res:
    res = a + b:
    deven_ res:
}

__start(){
    int result:
    result = __somma():
    println result:
}
```

---

## Build & Run

```bash
gcc Zinterpreter.c -o Zinterpreter -lm
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
| Dynamic variables (`$name`, runtime type switching) | Stable |
| Static type-checking on non-dynamic assignments (fatal error on mismatch) | Stable |
| Arrays (1D) | Stable |
| Matrices (2D) | Stable |
| Functions + return values | Stable |
| Function arguments (scalars) | Stable |
| Function arguments (arrays/matrices) | Stable |
| Default argument values (scalars only) | Stable |
| Pass-by-reference for function args (scalars) | Stable |
| Pass-by-reference for function args (arrays/matrices) | Stable |
| Opt-in pass-by-copy (`cp`) for function args | Planned — next objective |
| `$` in param name required for modifiable/type-switching pass-through | Planned |
| Renaming (`->`) — functions, variables, arrays, matrices | Stable |
| Inline C (`C{ }`) | Planned |
| Extended `deven_` expressions (`++`, `--`, `**`, `~~`, function) | Stable |
| `++N` / `--N` with variable or function as `N` | Stable |
| Arithmetic expressions | Stable |
| Conditionals (if/oth if/oth, `if` alone is valid) | Stable |
| String comparison (`s==`) | Planned |
| Increment/Decrement | Stable |
| Output (`print`, `println`, `lnprintln`) | Stable |
| Library support (.Zlib) | Stable |
| Build test suite | Stable |
| `scan_` (input) | Partial |
| `during` loops (condition-style) | Stable |
| `during` loops (count-style) | Stable |
| ESP32 port | Planned |

---

## Memory Footprint

Measured by running `status_` on the actual interpreter (compiled with gcc, Ubuntu, Debian, Arch):

| State | RAM used |
|-------|----------|
| VM at startup, no variables | ~284 KB |
| After declaring int/char/float variables | ~284 KB (no change) |
| After small arrays (10–50 elements) | ~285–292 KB |
| After large arrays (100–1000 elements) | ~292–300 KB |
| After large matrices (100×100) | grows ~64 KB per matrix |

The base VM footprint is **~284 KB**, determined by a set of fixed-size constants in the interpreter source (the program buffer, the state stack, the register file, and the maximum number of variable/array/matrix slots). These constants are **scalable**: they can be raised to support longer or more complex scripts — at the cost of a larger baseline footprint — or lowered to shrink it further for tighter targets like the ESP32.

> **Note:** the "maximum variables" constant is only an **error threshold**, not a pre-allocation count. Reaching it triggers an error — it does **not** mean that many # ZinterPL

## Alwayse refer to this file for sintax no other sintax file are updated before this.

**ZinterPL** is a lightweight, interpreted programming language written entirely in C, designed to run efficiently on constrained hardware — including the **ESP32** microcontroller. The interpreter is self-contained, low on memory footprint, and built with performance in mind.

Source files use the `.Zim` extension. Library files use `.Zlib`.

---

## Features

- Statically typed: `int`, `char`, `float` — unified declaration syntax for all three
- Variables, 1D arrays, and 2D matrices for all types
- **Dynamic variables** (`$name`) — type is set at declaration but switches automatically at runtime on assignment
- Static type safety on regular (non-dynamic) variables — a mismatched-type assignment is a fatal error
- Functions with arguments and return values — **typeless**: a function can return any scalar value (`int`, `char`, `float`)
- Default values for scalar function arguments
- `if / oth if / oth` branching — any number of `oth if` chains, and `if` alone (with no `oth`) is valid
- `during` unified loop construct (count-style and condition-style) — both fully functional
- Extended `deven_` return expressions: arithmetic, power, square root, and nested function calls
- Function argument pass-by-reference by default, with an opt-in pass-by-copy (`cp`) override declared per-argument
- Renaming via `->`: permanently rename any data — functions, variables, arrays, and matrices
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

Every instruction ends with `:` — this is the statement terminator. **This includes comments.**

### Comments

Comments begin with `//` and must also end with `:`, like every other statement:

```
// this is a comment:
// another comment:
```

Omitting the `:` at the end of a comment is a syntax error.

---

### Program Entry Point

Every `.Zim` file must define `__start()`, which is the main entry point:

```
__start(){
    // your code here:
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
| `exec_:` | Start execution |

> **Note:** the `import_` directive is **obsolete** and should not be used in new code. Attach libraries via the command line: `./Zinterpreter -df file.Zim -libname.Zlib`.

---

### Variable Declarations

Declarations no longer use the `&type&name&` token form. Each scalar type has its own keyword, followed directly by the variable name:

```
int varname:
char varname:
float varname:
```

`float` variables are now declared with the exact same form as `int` and `char` — there is no separate `float_` syntax to learn.

#### Token format: `&TYPE&NAME&`

> **Note:** the `&type&name&` token syntax is **no longer used to declare** variables, arrays, or matrices — declarations use the plain form shown above (`int name:`, `char [5]name:`, etc.). The token syntax is still used for **function argument declarations** (see [Functions](#functions)) and for **literal values** — numbers, chars, strings — inside expressions, print statements, and function calls.

| Token | Meaning | Used for |
|-------|---------|----------|
| `&i&name&` | integer | function arguments |
| `&c&name&` | char | function arguments |
| `&l&name&` | float | function arguments |
| `&a&name&` | array | function arguments |
| `&m&name&` | matrix | function arguments |
| `&n&value&` | numeric literal | literals in expressions |
| `&k&C&` | char literal (e.g. `'C'`) | literals in expressions |
| `&s&text&` | string literal | literals in expressions / print |
| `&f&file&` | file reference | literals in expressions |

---

### Arrays

```
int  [5]arrname:
char [5]arrname:
float [5]arrname:
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
int   [3][3]matname:
char  [3][3]matname:
float [3][3]matname:
```

Access:
```
[0][0]matname = 42:
var = [1][2]matname:
[row][col]matname = var:
```

---

### Type Safety

Regular (non-dynamic) variables, array elements, and matrix elements are statically typed once declared. Assigning a value of a **different** type — `int` → `float`, `char` → `int`, and so on — is a **fatal error**: the interpreter reports it and execution stops immediately.

```
int a:
a = 5:       // OK:
a = 2.71:    // FATAL ERROR — a is int, 2.71 is a float; program stops:
```

This applies to array and matrix elements too, not just scalars. **Dynamic variables** are the one exception to this rule — see below.

---

### Dynamic Variables (`$`)

A **dynamic variable** is any variable, array, or matrix whose name contains a `$`. By convention the `$` goes immediately before the name. Declaration uses the exact same syntax as a regular variable, array, or matrix — the `$` is simply part of the name:

```
int $dvar:
char $charvar:
int  [5]$temp:
char [4]$vdarr:
char [4][4]$vdar:
```

Access and assignment also use the same syntax as regular variables/arrays/matrices — nothing new there:

```
$dvar = 5:
[4]$temp = 7:
[3][3]$vdar = 'k':
```

#### Behavior

The declared type of a dynamic variable is only its **starting** type. Unlike a regular variable, a dynamic variable — or a single element of a dynamic array/matrix — is allowed to change type at runtime instead of raising the fatal error described in [Type Safety](#type-safety):

- Assigning a value of a different type does **not** error.
- Instead, the variable (or the specific array/matrix element) is **reset** and its type switches to whatever type the new value requires.

This makes dynamic variables useful wherever a value's type can't be known ahead of time.

#### Example

```
//====== DYNAMIC VAR TEST ============:
println &s&"====== DYNAMIC VAR TEST ============"&:

int $dvar:
$dvar = 5:
print $dvar: println &s&" 5 int $dvar = 5"&: println:
$dvar = 'k':
print $dvar: println &s&" k (int to char) "&:

$dvar = 5:
$dvar = 2.71:
print $dvar: println &s&" 2.71 (int to float)"&:

println:
$dvar = 'j':
print $dvar: println &s&" j (float to char) "&:

$dvar = 2.71:
$dvar = 22:
print $dvar: println &s&" 22 (float to int)"&:

$dvar = 'j':

println:
$dvar = 6:
print $dvar: println &s&" 6 (char to int)"&:

$dvar = 'o':
$dvar = 4.6:
print $dvar: println &s&" 4.6 (char to float)"&:

println:
char $charvar:
$charvar = 'k':
print $charvar: println &s&" k $charvar = k "&:
$charvar = $dvar:
print $charvar: println &s&" 4.6 $charvar = $dvar(float) "&:
$charvar = 2 + 5:
print $charvar: println &s&" 7 $charvar = 2 + 5"&:

println:
int [5]$temp:
[4]$temp = 7:
print [4]$temp: println &s&" 7 [4]$temp = 7"&:
[4]$temp = 'u':
print [4]$temp: println &s&" u array(int to char)"&:
[3]$temp = 3.55:
print [3]$temp: println &s&" 3.55 array(char to float)"&:
print [4]$temp: println &s&" 0 array_float vuoto"&:
[1]$temp = $charvar:
print [1]$temp: println &s&" 7 array(float to int) -> $charvar"&:

println:
char [4]$vdarr:
[3]$vdarr = 'd':
print [3]$vdarr: println &s&" d char [3]$vdarr"&:
[3]$vdarr = [1]$temp:
print [3]$vdarr: println &s&" 7 array->array(char to int)"&:

println:
char [4][4]$vdar:
[3][3]$vdar = 'k':
print [3][3]$vdar: println &s&" k char [3][3]$vdar"&:
[3][3]$vdar = [3]$vdarr:
print [3][3]$vdar: println &s&" 7 matrix->array(char to int)"&:
```

This walks a single dynamic scalar (`$dvar`) through int → char → float → char → int → char → float, a dynamic char scalar (`$charvar`) receiving values from another dynamic variable and from an expression, a dynamic array (`$temp`), and cross-assignment between a dynamic array, a dynamic char array (`$vdarr`), and a dynamic char matrix (`$vdar`) — each read/write resolving to whatever type the source value actually is.

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

The standard comparison operators (`==`, `<`, `>`,'^=') work on scalar variables and on indexed array/matrix elements.

When `==` is applied to an **unindexed array or matrix name**, it compares their **declared size**, not the number of occupied slots:

```
int [5]arrA:
int [5]arrB:
int [3]arrC:

if(arrA == arrB){   // true  — both declared with size 5:
    println &s&"same size"&:
}
if(arrA == arrC){   // false — 5 != 3:
    println &s&"same size"&:
}
```

This also applies to matrices, where the declared total size (rows × cols) is compared.

---

### Increment / Decrement

```
varname++:        // +1:
varname--:        // -1:
varname++N:       // +N:
varname--N:       // -N:

[idx]arr++:
[idx]arr--N:
```

**`N` can be a literal, a variable, or a function call:**

```
int step:
step = 3:

plusplus++step:            // increment by the value of a variable:
plusplus++__retv(5):       // increment by the return value of a function:
```

**Special behavior on char arrays:** `++` and `++N` on a char array element do **not** perform numeric arithmetic. Instead:

- `[idx]arr++` — copies the character at `[idx]` into `[idx+1]`
- `[idx]arr++N` — copies the character at `[idx]` into `[idx+N]` only (not into every cell in between)

```
char [10]arr:
[2]arr = 'F':
[2]arr++:     // copies 'F' into [3] only:
[2]arr++3:    // copies 'F' into [5] only  ([2] + 3 = [5]):
```

On integer variables and arrays, `++` / `--` behave as standard numeric increment/decrement.

---

### Functions

Declare with `od_` (open door):

```
od_ funcname(){
    int res:
    res = 3:
    deven_ res:
}
```

- `deven_` is the return keyword (replaces `return`).
- `deven_:` with no argument returns void.

Functions are **typeless**: there is no return-type declaration. A function can return an `int`, a `char`, or a `float` — the interpreter handles the type at runtime. The only restriction is that **arrays and matrices cannot be returned** (see [Limitations](#limitations)).

#### Function with arguments

Function argument declarations keep the original `&type&name&` token form — this did **not** change:

```
od_ testargs(&i&a& ! &i&b&){
    int risultato:
    risultato = a + b:
    deven_ risultato:
}
```

#### Default argument values

A scalar argument can be given a **default value** by placing the value after the argument token, separated by a space. When the function is called without providing that argument, the default is used instead.

```
od_ defaultest(&i&var0& 5){
    print &s&" "&:
    print var0:
    deven_:
}

__start(){
    __defaultest():      // var0 uses default → prints 5:
    __defaultest(10):    // var0 = 10 → prints 10:
}
```

The default value can be a numeric literal or a variable name accessible in the caller's scope at call time.

**Rules for default arguments:**

- Default arguments must appear **at the end** of the argument list. Non-default arguments cannot follow a default argument.
- **Arrays and matrices cannot have default values** — only scalar types (`int`, `char`, `float`) can.
- There is a maximum of **16 arguments** per function (this limit can be raised by changing the constant in the interpreter source).
- When calling a function, if you omit one default argument you must omit **all remaining** default arguments after it — you cannot skip one and provide the next. Doing so is a parse error.

```
// ✓ correct — default is last:
od_ myfunc(&i&var& ! &a&arr& ! &i&varr& 5){}

// ✗ wrong — default is not at the end:
od_ myfunc(&i&var& 5 ! &a&arr& ! &i&varr&){}

// ✗ wrong — arrays cannot have defaults:
od_ myfunc(&i&var& ! &a&arr& 0 ! &i&varr&){}
```

Multiple trailing defaults are allowed:

```
od_ multi(&i&a& ! &i&b& 10 ! &i&c& 20){
    // b defaults to 10, c defaults to 20:
    deven_ a:
}

__start(){
    __multi(1):         // a=1, b=10, c=20:
    __multi(1 ! 2):     // a=1, b=2,  c=20:
    __multi(1 ! 2 ! 3): // a=1, b=2,  c=3:
}
```

---

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
print __testargs(10!20):
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
    deven_ inner++:       // returns inner + 1:
}

od_ retfunc(){
    deven_ __differenza():  // delegates return to another function:
}

od_ retretunc(&i&pass&){
    deven_ __valueplus(pass):  // chains two function calls in one return:
}
```

This makes it easy to build concise functional pipelines:

```
print __retretunc(5):    // → valueplus(5) → 6:
print __retfunc():       // → differenza() → 6:
```

---

### Pass-by-reference for function arguments

In ZinterPL, function arguments are passed **by reference by default** — scalars, arrays, and matrices alike. This means a function does not receive a copy of its argument: it receives a direct alias to the original. Any modification made inside the called function is immediately and permanently visible in the caller after the call returns.

This propagates across the full call chain. If function A passes a variable to function B, and B modifies it, the change is visible in A when B returns — and visible in whoever called A as well.

A parameter can opt out of this and be passed by copy instead using the `cp` keyword — see [Opt-in pass-by-copy](#opt-in-pass-by-copy-cp) below.

#### Scalar pass-by-reference

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
3. `__addtwo(value)` — `addtwo` receives the same alias and increments by 2 → `value = 8`
4. Back in `addone`, `value` is now `8` (the change from `addtwo` propagated back)
5. `deven_ value` returns `8`

```
print __addone(5):   // prints 8:
```

#### Array and matrix pass-by-reference

Arrays and matrices follow the exact same rule. When an array is passed to a function, the function works directly on the original array in the caller's scope — there is no copy. Writing to any element inside the function writes through to the original. The type tokens for arrays and matrices passed as arguments are `&a&` and `&m&` respectively; the interpreter resolves the actual element type automatically.

```
od_ fill(&a&arr&){
    [0]arr = 10:
    [1]arr = 20:
    [2]arr = 30:
    deven_:
}

__start(){
    int [5]data:
    __fill(data):
    println [0]data:    // prints 10:
    println [1]data:    // prints 20:
    println [2]data:    // prints 30:
}
```

The same applies to matrices:

```
od_ zero_matrix(&m&mat&){
    [0][0]mat = 0:
    [1][1]mat = 0:
    [2][2]mat = 0:
    deven_:
}
```

#### Opt-in pass-by-copy (`cp`)

By default, every argument is passed by reference, as described above. A per-argument override lets a parameter be passed by copy instead: write the keyword `cp` immediately before the argument's type token, **in the function declaration only**:

```
od_ ref_vs_copy(&i&rn& ! cp &i&cn& ! &c&rc& ! cp &c&cc& ! &a&rarr& ! cp &a&carr& ! &m&rmat& ! cp &m&cmat&){
    // rn, rc, rarr, rmat  -> passed by reference (default):
    // cn, cc, carr, cmat  -> passed by copy (cp):
}
```

- `cp` works on scalars, arrays, and matrices alike.
- `cp` is only ever written in the **declaration**. It has no syntax at the call site: the call always looks the same regardless of which parameters are `cp` —
  ```
  __ref_vs_copy(a ! b ! c ! d ! e ! f ! g ! h):
  ```
  Writing `cp` in the call itself (e.g. `__ref_vs_copy(cp a ! ...)`) is invalid.
- No `cp` → the parameter is an alias: modifications inside the function propagate back to the caller.
- `cp` → the parameter is an independent local copy: modifications inside the function stay local and are **not** reflected back in the caller.

#### `$` required for modifiable/type-switching pass-through

To pass a variable so it can be modified through the reference **including changing type**, the parameter name in the callee's declaration needs to contain `$`, the same marker used for [dynamic variables](#dynamic-variables-). Without `$` in the parameter name, a by-reference parameter can still be written to, but stays subject to the ordinary [static type-safety rule](#type-safety): writing a mismatched type through it is a fatal error, same as assigning it directly. Naming the parameter with `$` lifts that restriction, so the original variable can switch type through the call the same way a dynamic variable can on its own.

---

### ⚠️ Errors and pitfalls: `deven_` expressions and aliasing

#### The trap

Because by-reference arguments are aliases, any `deven_` expression that modifies the argument in place **also modifies the original variable in the caller**. This is the most common source of bugs with pass-by-reference.

`deven_ n**` does not compute a temporary square — it modifies `n` in place, and since `n` is an alias of whatever was passed, the caller's variable is overwritten too.

**Wrong — destroys the loop variable:**

```
od_ sq(&i&n&){
    deven_ n**:     // n** modifies n in place — the caller's i gets overwritten:
}

__start(){
    int i:
    i = 1:
    during(i < 9 ! i++){
        print __sq(i):    // after sq(3): i becomes 9 in __start, loop breaks:
    }
}
```

**Correct — use a local copy:**

```
od_ sq(&i&n&){
    int tmp:
    tmp = n:        // copy the value into a local variable:
    tmp**:          // square the local copy — n (and the caller's variable) untouched:
    deven_ tmp:
}
```

The rule: if a `deven_` expression uses `**`, `~~`, `++`, or `--` on a by-reference parameter, always copy the parameter into a local variable first and operate on that.

#### The fix: `cp`

Declaring the parameter itself as `cp` removes the need for the manual `tmp` workaround above: `cp` makes the parameter a genuine local copy, so a `deven_` expression using `**`, `~~`, `++`, or `--` on that parameter only touches the local copy — the caller's original variable stays untouched, with no extra `tmp` variable required.

```
od_ sq(cp &i&n&){
    deven_ n**:     // n is cp — this changes only the local copy, caller's variable is untouched:
}
```

By-reference parameters (no `cp`) keep the aliasing behavior described above: `deven_` expressions still mutate them in place, so the manual `tmp` copy pattern is still needed for those.

---

### Renaming (`->`)

The `->` operator permanently renames any named entity in the VM: **functions, scalar variables, arrays, and matrices**. After the rename the entity is only accessible under the new name; the old name is no longer valid.

```
newname -> oldname:
```

#### Functions

```
newname -> originalfunc:           // function rename:
[i]newname  -> [i]originalfunc:    // array-indexed function form:
[r][c]newname -> [r][c]original:   // matrix-indexed function form:
```

#### Scalar variables

```
int score:
score = 42:
punti -> score:
println punti:    // prints 42:
println score:    // score no longer exists — returns 0:
```

#### Arrays

```
int [5]data:
[0]data = 99:
numeri -> data:
println [0]numeri:    // prints 99:
println [0]data:      // data no longer exists — returns 0:
```

#### Matrices

```
int [3][3]grid:
[0][0]grid = 7:
tavola -> grid:
println [0][0]tavola:    // prints 7:
println [0][0]grid:      // grid no longer exists — returns 0:
```

In all cases the rename is immediate and permanent for the remainder of the program's execution.

---

### Output

```
print value:        // print without newline:
println value:      // print with newline after the value:
println:            // print empty newline:
lnprintln value:     // print a newline, THEN the value, THEN another newline:
```

`lnprintln` is like `println`, but it also prints a leading newline before the value:

```
println value:      // → "value\n"
lnprintln value:     // → "\nvalue\n"
```

`value` can be a variable name, an `&s&string&` literal, an array element, a matrix element, or a direct function call:

```
print __testargs(10!20):    // prints 30:
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
    // ...:
}
oth if(condition){
    // ...:
}
oth if(condition){
    // any number of oth if chains are supported:
}
oth{
    // ...:
}
```

Conditions support `==`, `<`, `>`. String comparison (`s==`) is planned.

You can chain **any number of `oth if`** blocks before the final `oth`.

An `if` block does **not** require any `oth` or `oth if` after it — a plain `if` on its own is completely valid and will not crash the program.

**Standalone `oth` or `oth if` without a preceding `if`** are silently ignored — they do not crash or produce errors, they simply do not execute.

---

### Spaces and Whitespace

**Spaces are ignored everywhere** in ZinterPL source code, **except inside double quotes** (`"..."`). This means indentation and spacing are purely cosmetic and have no effect on parsing.

```
int x:          // canonical:
int    x:       // extra spaces are ignored:
int x   :       // trailing spaces before ':' are ignored too:
```

Inside a string literal, spaces are preserved:
```
println &s&"hello world"&:   // prints: hello world:
```

---

### Variable Scope

Variables declared inside a function are **local to that function** and are not accessible from outside.

Variables declared inside `__start()` are **global**: they are accessible from any function called during program execution.

```
__start(){
    int counter:    // global — visible to all functions called from here:
    counter = 10:
    __increment():
    println counter:    // prints 11:
}

od_ increment(){
    counter++:           // accesses the global 'counter' declared in __start:
    deven_:
}
```

Variables declared inside any other function (`od_`) are local to that function and are not accessible from outside. Attempting to read a variable that does not exist in the current scope returns a zero/null value rather than crashing.

```
od_ myfunc(){
    int local:
    local = 42:
    deven_ local:
}

__start(){
    local = 0:       // 'local' not declared here — silently fails:
    print local:    // prints 0:
}
```

---

### Variable Indices

Array and matrix elements can be accessed using either literal indices or integer variables as indices:

```
[0]arr = 42:           // literal index:
[idx]arr = 42:         // variable as index:

[0][0]mat = 99:        // literal row and col:
[row][col]mat = 99:    // variables as row and col:
[row][0]mat = 99:      // mixed:
```

---

### Inline C Code

> ⚠️ **Planned — not yet implemented.**

A future version will allow defining a C function directly inside a `.Zim` file using a `C{ }` block. The code inside will be compiled separately as a standalone C function.

```
C{
    // C code here:
    return 42;
}
```

To call it, use `__C(...)` passing variables, values, or arrays as arguments:

```
int result:
result = __C(myvar, myarr, 10):
```

The return value of `return` in the C block will be used as the result of `__C(...)` in the ZinterPL code.

---

### Loops

ZinterPL uses a single unified loop construct: **`during`**, replacing the earlier separate `for_` / `while_` keywords.

There are two forms.

#### Count-style

Repeats a fixed number of times. The argument can be a numeric literal, an integer variable, or an array element:

```
during(5){
    println &s&"ciao"&:
}

during(repetition){
    println &s&"hello"&:
}

during([4]tion){
    println &s&"hi"&:
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
    println &s&"bye"&:
}

during(repetition < 6 ! repetition++2){
    println &s&"tre"&:
}

during(i < 10 ! i++ ! j-- ! k++2){
    println i:
}
```

The condition is re-checked after the body and all steps have run; the loop exits as soon as it evaluates to false.

---

### Diagnostic

```
status_:
```

Prints the current state of all declared variables, arrays, and matrices. Useful for debugging.

#### Clearing state

Adding a clear keyword causes `status_` to **wipe the VM state** after printing. The accepted keywords are `cls`, `clear`, and `clr` — all three are equivalent. The optional flag `y` can appear anywhere in the statement, in any position relative to the clear keyword:

```
status_ cls:
status_ clear:
status_ clr:
status_ cls y:
status_ y cls:
status_ y clear:
```

> ⚠️ After a clear, all variables, arrays, and matrices that were declared before the `status_` call are **no longer accessible**. Their memory is wiped and any subsequent reads will return zero/null. Only declare new variables after the clear if you need them.

---

### Libraries

Libraries are `.Zlib` files containing additional function definitions. Pass them on the command line:

```bash
./Zinterpreter -df file.Zim -libstdfn.Zlib
```

The interpreter merges the library into the source before parsing.

> **Note:** the in-file `import_` directive is **obsolete** and should not be used in new code. Always attach libraries via the command line.

---

## Limitations

- **Arrays and matrices cannot be returned from functions** via `deven_`. Only scalar values (`int`, `char`, `float`) are returnable.
- **Arrays and matrices cannot have default argument values.** Only scalar parameters support defaults.
- **Maximum 16 arguments per function** (adjustable by changing the limit constant in the interpreter source).
- **Regular (non-dynamic) variables, array elements, and matrix elements cannot change type after declaration** — assigning a mismatched type is a fatal error. Use a dynamic variable (`$name`) if the type needs to vary at runtime.
- **`cp` has no call-site syntax.** It is only ever written in a function's `od_` declaration; a call always looks the same regardless of which parameters are `cp`.

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
    int a:
    int b:
    a = 3:
    b = 5:
    int res:
    res = a + b:
    deven_ res:
}

__start(){
    int result:
    result = __somma():
    println result:
}
```

---

## Build & Run

```bash
gcc Zinterpreter.c -o Zinterpreter -lm
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
| Dynamic variables (`$name`, runtime type switching) | Stable |
| Static type-checking on non-dynamic assignments (fatal error on mismatch) | Stable |
| Arrays (1D) | Stable |
| Matrices (2D) | Stable |
| Functions + return values | Stable |
| Function arguments (scalars) | Stable |
| Function arguments (arrays/matrices) | Stable |
| Default argument values (scalars only) | Stable |
| Pass-by-reference for function args (scalars) | Stable |
| Pass-by-reference for function args (arrays/matrices) | Stable |
| Opt-in pass-by-copy (`cp`) for function args | Stable — declaration only, no call-site syntax |
| `$` in param name required for modifiable/type-switching pass-through | Stable |
| Renaming (`->`) — functions, variables, arrays, matrices | Stable |
| Inline C (`C{ }`) | Planned |
| Extended `deven_` expressions (`++`, `--`, `**`, `~~`, function) | Stable |
| `++N` / `--N` with variable or function as `N` | Stable |
| Arithmetic expressions | Stable |
| Conditionals (if/oth if/oth, `if` alone is valid) | Stable |
| String comparison (`s==`) | Planned |
| Increment/Decrement | Stable |
| Output (`print`, `println`, `lnprintln`) | Stable |
| Library support (.Zlib) | Stable |
| Build test suite | Stable |
| `scan_` (input) | Partial |
| `during` loops (condition-style) | Stable |
| `during` loops (count-style) | Stable |
| ESP32 port | Planned |

---

## Memory Footprint

Measured by running `status_` on the actual interpreter (compiled with gcc, Ubuntu, Debian, Arch):

| State | RAM used |
|-------|----------|
| VM at startup, no variables | ~284 KB |
| After declaring int/char/float variables | ~284 KB (no change) |
| After small arrays (10–50 elements) | ~285–292 KB |
| After large arrays (100–1000 elements) | ~292–300 KB |
| After large matrices (100×100) | grows ~64 KB per matrix |

The base VM footprint is **~284 KB**, determined by a set of fixed-size constants in the interpreter source (the program buffer, the state stack, the register file, and the maximum number of variable/array/matrix slots). These constants are **scalable**: they can be raised to support longer or more complex scripts — at the cost of a larger baseline footprint — or lowered to shrink it further for tighter targets like the ESP32.

> **Note:** the "maximum variables" constant is only an **error threshold**, not a pre-allocation count. Reaching it triggers an error — it does **not** mean that many variables are pre-declared or reserved up front. Declaring fewer variables than the maximum does not cost extra memory.

This makes ZinterPL suitable for constrained environments like the **ESP32**, where the entire VM lives inside a single `struct VM` and can be statically allocated.

---

- **Minimal memory usage** — fixed-size data structures, no heap sprawl; suitable for microcontrollers
- **Self-contained** — single C source file, no external dependencies
- **ESP32 target** — the entire VM is a single `struct VM`, easy to port to embedded environments
- **Transparent execution** — debug mode prints every step of parsing and execution

---

*ZinterPL — a custom scripting language built in C, designed to go small.*
 are pre-declared or reserved up front. Declaring fewer variables than the maximum does not cost extra memory.

This makes ZinterPL suitable for constrained environments like the **ESP32**, where the entire VM lives inside a single `struct VM` and can be statically allocated.

---

- **Minimal memory usage** — fixed-size data structures, no heap sprawl; suitable for microcontrollers
- **Self-contained** — single C source file, no external dependencies
- **ESP32 target** — the entire VM is a single `struct VM`, easy to port to embedded environments
- **Transparent execution** — debug mode prints every step of parsing and execution

---

*ZinterPL — a custom scripting language built in C, designed to go small.*
