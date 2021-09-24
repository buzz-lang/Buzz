# Buzz Language Reference
The API documentation presents Buzz's syntax and available functions. It is structured as follows:

1. [Comments](#comments)
2. [File Inclusion](#inclusion)
3. [Variables](#variables)\
3.1 [Primitive Types](#primtypes)\
3.2 [Scope](#scope)
4. [Control Structures](#contrstruct)\
4.1 [Boolean Operators](#booleanoperators)\
4.2 [Comparison Operators](#comparisonoperators)\
4.3 [If Statements](#if)\
4.4 [While Loops](#while)\
4.5 [For Loops](#for)\
4.6 [Foreach Loops](#foreach)
5. [Tables](#tables)
6. [Functions](#function)\
6.1 [Function Definition](#fundef)\
6.2 [Function Pointers](#funpoint)\
6.3 [Namespaces](#namespaces)\
6.4 [Classes and Methods](#classes)
7. [Math](#math)\
7.1 [Basic Operators](#operators)\
7.2 [Math Library](#mathlib)\
7.3 [Random Library](#mathrnglib)\
7.4 [Vector2 Library](#mathvec2lib)\
7.5 [Matrix Library](#mathmatrixlib)
8. [Strings](#strings)
9. [File I/O](#files)
10. [Queues](#queues)
11. [Swarm Management](#swarm)
12. [Virtual Stigmergy](#vstig)
13. [Neighbor Management](#neighbors)
14. [User Data](#userdata)
<a name="comments"></a>

# Comments
Buzz code can be commented with `#`.
```ruby
# This is a comment
```

<a name="inclusion"></a>

# File inclusion
To allow cleaner code structure as well as library usage, .bzz files can be included with the following syntax:
```ruby
include "otherfile.bzz"
```
NOTE: A specific file can be included only once.
Any 'include' statement for a file is ignored after the first inclusion
occurred.
Relative paths are automatically transformed into absolute paths before
including a file.

<a name="variables"></a>

# Variables
Buzz is a dynamically typed language. This means that the type of a
variable can change during the execution of a script, and it
depends on the type of the value stored by a variable at any
moment.

Buzz does not include the concept of constant. Every variable is
always mutable.

A variable can be created in three ways, which will affect its [scope](#scope):
```ruby
# Declared, but not initialized (var keyword mandatory)
var x
# Declared and initialized (with var keyword)
var i = 5
# Declared and initialized (without var keyword)
n = 7
```

<a name="primtypes"></a>

## Primitive Types
Buzz supports the following primitive types:
- `nil` : encodes an unknown value for a variable, or a `false`
    [condition](#contrstruct)
- `integer` : a 32-bit signed integer (e.g., `90`)
- `float` : a 32-bit floating-point value (e.g., `1.0`, `0.25`,
    `.5`)
- `string` : a [strings](#strings) of characters (e.g., `"hello,
    world!"`)
- `table` : a structured type to encode various kinds of data
    structures (see [tables](#tables))
- `closure` : a [function pointer](#funpoint)
- `user data` : a pointer to a user-defined memory region managed
    externally to Buzz. See [User Data](#userdata) for more
    information.

The type of a variable can be checked with `type(my_variable)`.

<a name="scope"></a>

## Scope
Buzz variables can be *global* or *local*. To declare a variable as
*local* to a code block (e.g., a [functions](#function), you
need to prepend its first definition with the keyword `var`:
```ruby
# A local variable - by default its value is nil
var x
# Another local variable with initialization
var i = 5
# Assignment for a local variable
x = 7.8
```
To declare a *global* variable, you simply assign its value without
using the `var` keyword:
```ruby
# Local variables
function f() {
    var x = 42  # x is a local variable
    j = 5       # j is a global variable
}
```

<a name="contrstruct"></a>

# Control Structures
<a name="booleanoperators"></a>

## Boolean Operators
Buzz has three basic boolean operators to allow the combination of conditional statements. These operators are the standard `and`, `or` and `not` operators, which follow regular truth tables and operator precedence.

Because there is no explicit boolean type (unlike in Python for example, which has True and False as possible boolean values), we remind the reader that in Buzz, 0 and `nil` both evaluate as fasly values (represented by 0), while any other numeric value evaluates as truthy (represented by 1).

```ruby
log(not 1)    # 0
log(2 or 0)   # 1
log(1 or 0)   # 1
log(1 and 1)  # 1
```

<a name="comparisonoperators"></a>

## Comparison Operators
Along with boolean operators, Buzz also provides comparison operators: `<`, `<=`, `=>`, `>`, `==` and `!=`. Again, these follow standard operator precedence.

```ruby
log(1 < 2)   # 1
log(2 <= 2)  # 1
log(0 == 2)  # 0
log(0 or 0 == 0)  # 1
```

<a name="if"></a>

## `if` Statement
Buzz supports conditional code execution through `if`, `if/else` and `if/else if/else` statements. The parentheses around the condition are required. The brackets surrounding the block are not mandatory if said block is only one line long (much like in C++), but they are nevertheless recommended for clarity.

```ruby
# Simple if statement
if (x > 3) {
    log("x is too big")
}

# if / else if / else
if (x > 3) {
    log("x is too big")
} else if (x < 3) {
    log("x is too small")
} else {
    log("maybe I just don't like x")
}

# Checking for equality
if (x == 3) {
    log("x is equal to 3")
}
 
# Checking for inequality
if (x != 4) {
    log("x is different from 4")
}
 
# Combining conditions with OR
if (x < 3) or (x > 3) {
    log("x is not 3")
}
 
# Combining conditions with AND
if ((x > 3) and (y > 3)) {
    log("x and y are too big")
}
 
# Negating a condition
if (not (x < 3)) {
    log("x is <= 3")
}
```

<a name="while"></a>

## `while` Statement
```ruby
var i = 10
while (i < 20) {
    log(i)
    i = i + 1
}
```

<a name="for"></a>

## `for` Statement
Buzz supports standard for loops.
Note that statements defining the loop (like `i=0`) are separated by *commas*.

```ruby
for (i = 0, i < 10, i = i + 1) {
    log(i)
}
```

<a name="foreach"></a>

## `foreach` Statement
The foreach statement allows to iterate over tables, and as such is more extensively described in the [tables section](#tables).

```ruby
var my_table = { .x = 3, .y = 5 }
foreach(my_table, function(key, value) {
    log(key, " -> ", value)
})
```

<a name="tables"></a>

# Tables
Tables are the only structured type available in Buzz. Tables are
quite flexible: you can use them both as a hash map or as a
classical array. Tables are **always passed by reference**, whether it is through assignment or through parameters in a function call.

To create an empty table, use this syntax:
```ruby
t = {}
```

Internally, Buzz tables are implemented as hash maps, that is, a
collection of `(key, value)` pairs. While in principle you could
use any Buzz primitive type as `key`, the most common types are
integers, floats, and strings. In this case, you can use this
syntax to populate a table:
```ruby
t = { .x = 1, .2 = 5.6, .4.5 = "k" }
```
This syntax creates a table that contains three pairs: `("x", 1)`,
`(2, 5.6)`, and `(4.5, "k")`. The table is stored in variable `t`.

To add new values to a table, or to set new values for existing
keys (remember that in hash maps keys can't be duplicated!), use
this syntax:
```ruby
# If the key is an integer
t[6] = "six"
# If the key is a float
t[1.0] = "one point zero"
# If the key is a string, the following two lines are equivalent
t["hello"] = "this is a greeting"
t.hello = "this is a greeting"
```

To read the value of a table, the same syntax as above applies:
```ruby
# If the key is an integer
print(t[6])
# If the key is a float
print(t[1.0])
# If the key is a string, the following two lines are equivalent
print(t["hello"])
print(t.hello)
```

Finally, to erase an element from a table it is enough to set it to
`nil`:
```ruby
t[1.0] = nil
```

Table contents can be handled through a number of dedicated functions.

- `size(t)` : returns the current number of elements in table `t`:
  ```ruby
  t = { .x = 4 }
  print(size(t)) # prints 1
  ```
- `foreach(t, f)` : applies a function `f(key, value)` to each element of table `t`:
  ```ruby
  t = { .x = 4, .y = 5, .z = 6 }
  foreach(t, function(key,value) {
      print("(", key, ", ", value, ")")
    })

  # prints
  #   (x, 4)
  #   (y, 5)
  #   (z, 6)
  ```
  It is important to notice that `foreach(t, f)` is not meant to
  modify the values of the table. It is only meant to go through
  the elements of `t` and use its values in a read-only
  fashion. If you want to modify the elements of a table, use
  `map(t, f)`.
- `map(t, f)` : applies a function `f(key, value)` to each element
  of table `t` and returns a new table. For each element,
  function `f` must return a value, which is used to populate
  the new table. For instance:
  ```ruby
  t = { .x = 1, .y = 2 }
  u = map(t,
        function(key,value) {
          return value + 100
        })
  # now u contains:
  #   ("x", 101)
  #   ("y", 102)
  ```
- `reduce(t, f, a)` : applies a function `f(key, value, accumulator)`
  to each element of table `t`. Function `f` must accept three
  parameters: the current key, the corresponding value, and an
  accumulator. Function `f` must also return a value, that is
  passed as accumulator to the invocation of `f` on the next
  table element. Parameter `a` of `reduce(t, f, a)` is the initial
  value of the accumulator. For instance, if you want to
  calculate the average of the values in a table, write the
  following code:
  ```ruby
  t = { .1 = 1.0, .2 = 2.0, 3. = 3.0 }
  average = reduce(t, function(key, value, accumulator) {
      return value + accumulator
    }, 0.0) / size(t)
  # avg is now 2.0
  ```

Buzz also offers a library to handle more table operations. The library is
stored in `INSTALL_PREFIX/share/buzz/include/table.bzz`, so to use
it a script must first include it. The complete reference of these
functions is included in the file.

<a name="functions"></a>

# Functions

<a name="fundef"></a>

## Defining and Calling Functions
To define and call functions in Buzz, use this syntax:
```ruby
# Function definition
function my_add(x, y) {
  return x + y
}

# Function call: z = 1 + 2 = 3
z = myadd(1, 2)
```

Functions that do not return an explicit value implicitly return `nil`:
```ruby
function my_void_function(x, y) {
  log(x + y)
}

# Function call ignoring return value
# Prints 3
my_void_function(1, 2)

# Function call assigning return value
# Prints 3
# z is nil after the call
z = my_void_function(1, 2)
```

Function definitions can be nested:
```ruby
# Outer definition
function my_outer(x) {
  # Inner definition
  function my_inner(y) {
    return x + y
  }
  # Call the internally defined function
  return my_inner(2)
}

# Function call: z = 1 + 2 = 3
z = my_outer(1)
```

<a name="funpoint"></a>

## Function Pointers
Buzz supports function pointers. This means that you can define anonymous
functions and pass them as arguments or assign to variables. These are also referred to as lambdas. To define a function pointer, use this syntax:
```ruby
# Function definition
my_add = function(x, y) {
  return x + y
}

# Function call: z =  1 + 2 = 3
z = my_add(1, 2)
```

For all effects and purposes, this is identical to the definition we saw
above:
```ruby
function my_add(x, y) {
  return x + y
}
```

Using function pointers allows you pass functions as parameters to higher-level functions:
```ruby
# Some table...
t = { ... }

# Print all the table elements
foreach(t, function(k, v) { log("k=", k, "; v=", v) })
```

You can mix inner function definition with function pointers:
```ruby
# Function definition
function my_outer(x) {
  return function(y) {
    return x + y
  }
}

# Function call
f = my_outer(1)

# Using the returned function
# z = 1 + 2 = 3
z = f(2)
```
In the above example, the statement `my_outer(1)` creates a function in which
 parameter `x` of `my_outer(x)` is bound to 1. This means that the returned function
 sums 1 to the parameter given to it.

<a name="namespaces"></a>

## Namespaces
Using tables and function pointers, it is possible to define namespaces. This is used extensively in the core libraries, such as `string` and `math`. A namespace is nothing but a table:
```ruby
# Define the namespace
mynamespace = {}

# Define a constant
mynamespace.CONST = 3.14

# Define a function
mynamespace.myadd = function(x,y) {
  return x + y
}

# Use the namespace
log(mynamespace.CONST)
z = mynamespace.myadd(1, 2)
```

<a name="classes"></a>

## Classes and methods
By using tables and function pointers it is also possible to define classes and methods. In Buzz, the syntax to define namespaces and classes is, effectively, the same, and internally the virtual machine does not distinguish between these two scenarios.

However, when a function is meant to be interpreted as a class method, the
keyword `self` becomes important. The `self` keyword is interpreted upon
function call as shown in this example:
```ruby
# Function definition
function my_function() {
  log(self)
}

# Using the function standalone
# Prints nil
my_function()

# Using the function within a table
t = {}
t.f = my_function
# Prints [table with 1 element]
t.f()
```
In other words, the keyword `self` points to the context in which the function is called. When the function is called standalone, there is no context and `self` is `nil`. When a function is called from a table, the `self` keyword points to the table.

Using the `self` keyword, you can write methods that access class attributes:
```ruby
# Class definition
MyClass = {
  .my_attribute = 1,

  .my_method = function(x) {
    return x + self.my_attribute
  }
}

# Method call: z = 1 + 2 = 3
z = MyClass.my_method(2)
```

You can make full-fledged classes with constructors as follows:
```ruby
# Class definition
MyClass = {

  # Class constructor
  .new = function(x) {
    # Return a new table
    return {
      # Bind the attribute values
      .my_attribute = x,
      # Bind the methods
      .my_method = my_method
    }
  },

  # Method definition
  .my_method = function(x) {
    return self.my_attribute + x
  }
}

# Usage
# Create the object
my_object = MyClass.new(1)
# Call the method: z = 1 + 2 = 3
z = my_object.my_method(2)
```

Note that because classes are essentially tables, their elements (attributes and methods) must be separated by commas as in the previous example.

<a name="math"></a>

# Math
<a name="operators"></a>

## Basic Math Operations
Math works similarly to most programming languages you are used
to. The basic math operations, in decreasing order of precedence,
are:
1. Unary minus (e.g., `-5`)
2. Power (e.g., `3^5`)
3. Modulo (e.g., `10 % 4`)
4. Multiplication and division (e.g., `2 * 3 / 4`)
5. Addition and subtraction (e.g., `2 + 3 - 4`)
Analogously to other languages, parentheses are used to modify the
natural precedence of the operators:
```ruby
x = 5^(4 % (3 * (2 + 1)))
# x = 5^(4 % (3 * 3))
# x = 5^(4 % 9)
# x = 5^4
# x = 625.0
```
As the above example shows, the power operator transforms its
operands into float, even if the operands are both integers. The
other operators return an integer if both operands are integer,
and a float if either or both operands are float. A type error is
raised if the operands are not integers nor floats.

<a name="mathlib"></a>

## The `math` Library
A wider set of mathematical functions is available. These
functions are stored into the `math` table. The `math` table is
set up upon initialization of the Buzz VM, so no `include`
statement is necessary to use it.

The `math` functions work with both integer and float values. The
complete list of functions is as follows:
- `math.abs(x)` returns the absolute value of `x`. The type of the
  result is the same as the type of `x`.
- `math.log(x)` returns the natural logarithm of `x` as a float.
- `math.log2(x)` returns the base 2 logarithm of `x` as a float.
- `math.log10(x)` returns the base 10 logarithm of `x` as a float.
- `math.exp(x)` returns _e_ to the power of `x` as a float.
- `math.sqrt(x)` returns the square root of `x` as a float.
- `math.sin(x)` returns the sine of `x` as a float.
- `math.cos(x)` returns the cosine of `x` as a float.
- `math.tan(x)` returns the tangent of `x` as a float.
- `math.asin(x)` returns the arc sine of `x` as a float.
- `math.acos(x)` returns the arc cosine of `x` as a float.
- `math.atan2(y, x)` returns the arc tangent of `y, x` as a float.
- `math.min(x, y)` returns the minimum between `x` and `y`. The
  type of the return value corresponds to the type of the minimum
  value: `min(1.0, 2)` is `1.0`, and `min(1, 2.0)` is `1`.
- `math.max(x, y)` returns the maximum between `x` and `y`. The
  type of the return value corresponds to the type of the maximum
  value: `max(1.0, 2)` is `2`, and `max(1, 2.0)` is `2.0`.

In addition to these functions, the math table also includes the
constant `math.pi`.

As an example of how mathematical operators and the math library can be combined, take a look at an excerpt of the `math.vec2` library implementation:

```ruby
#
# Create a new namespace for vector2 functions
#
math.vec2 = {}

#
# Creates a new vector2.
# PARAM x: The x coordinate.
# PARAM y: The y coordinate.
# RETURN: A new vector2.
#
math.vec2.new = function(x, y) {
    return { .x = x, .y = y }
}

#
# Creates a new vector2 from polar coordinates.
# PARAM l: The length of the vector2.
# PARAM a: The angle of the vector2.
# RETURN: A new vector2.
#
math.vec2.newp = function(l, a) {
    return {
        .x = l * math.cos(a),
        .y = l * math.sin(a)
    }
}

#
# Rotates v by angle a (in radians)
# PARAM v: A vector2
# PARAM a: An angle (in radians)
# RETURN: the rotated vector
#
math.vec2.rotate = function(v, a) {
    return {
        .x = v.x * math.cos(a) - v.y * math.sin(a),
        .y = v.x * math.sin(a) + v.y * math.cos(a)
    }
}

```

<a name="mathrnglib"></a>

## The `math.rng` Library
The `math` library also includes a collection of functions for
random number generation. These functions are stored into the
`math.rng` table and, similarly to `math`, do not require an `include` statement to be used.

The random number generator is based on the well-known Mersenne
Twister algorithm.

### Setting the Seed
Upon initialization, the Buzz VM sets a random seed taken from the
current clock. If you wish to set the random seed explicitly to a
value `s`, use the function `math.rng.setseed(s)`. The value of
`s` must be an integer, or a type error is raised.

### Uniform Distribution
To draw numbers from a uniform distribution, use
`math.rng.uniform(...)`. The behavior of this function depends on
the number and type of parameters passed.
- `math.rng.uniform()` : returns an integer between $-2^{32}$ and 
  $+2^{31}-1$.
- `math.rng.uniform(x)` : returns a value between 0 and `x`.
  The type of the returned value matches the type of `x`.
- `math.rng.uniform(x, y)` : returns a value between `x` and `y`.
  If both `x` and `y` are integers, the returned value is an integer; if either or both are floats, the returned value is a float.

### Gaussian Distribution
To draw numbers from a Gaussian distribution, use
`math.rng.gaussian(...)`. The behavior of this function depends on
the number and type of parameters passed.
- `math.rng.gaussian()` : returns a float from a Gaussian with 0
  mean and standard deviation 1.
- `math.rng.gaussian(x)` : returns a float from a Gaussian with 0
  mean and standard deviation `x`.
- `math.rng.gaussian(x, y)` : returns a float from a Gaussian with
  mean `y` and standard deviation `x`.

### Exponential Distribution
To draw numbers from an exponential distribution, use
`math.rng.exponential(x)`, where `x` is the mean. The returned
value is a float.

### Usage Example

```ruby
# Sets the seed with current robot id
math.rng.setseed(id)

# Samples randomly from a uniform distribution
math.rng.uniform(-1.0, 1.0)

# Samples randomly from a exponential distribution
math.rng.exponential(-1.0)
```

<a name="mathvec2lib"></a>

## The `math.vec2` library
When dealing with the robots, it is often useful to manipulate
vectors. Buzz offers a library to handle 2D vectors. The library is
stored in `INSTALL_PREFIX/share/buzz/include/vec2.bzz`, so to use
it a script must first include it. The complete reference of these
functions is included in the file.

### Usage Example

```ruby
include "vec2.bzz"

# Vector creation
my_vec = math.vec2.new(1.0, 1.0)
my_other_vec = math.vec2.new(1.0, 1.0)

# Vector scaling
scaled_vec = math.vec2.scale(vec, 2.0)

# Vector addition
added = math.vec2.add(my_vec, my_other_vec)
```

<a name="mathmatrixlib"></a>

## The `math.matrix` Library
The `math` library also includes a collection of functions for
manipulating matrices. The library is stored
in `INSTALL_PREFIX/share/buzz/include/matrix.bzz`, so to use
it a script must first include it. The complete reference of these
functions is included in the file.

### Usage Example

```ruby
include "matrix.bzz"

# Empty (zeros) matrix creation
my_matrix = math.matrix.new(3, 3)

# Identity matrix creation
my_identity = math.matrix.identity(3)

# Matrix subtraction
math.matrix.sub(my_identity, my_matrix)

```

<a name="strings"></a>

# Strings
## Built-in String Operations
- `string.length(s)` returns the length of string `s`
- `string.sub(s, ...)` returns a substring of the given string. Two
  signatures are possible: `string.sub(s, n)` returns the substring
  starting at character `n` (`0` is the first character);
  `string.sub(s, n, m)` returns the substring starting at character
  `n` and ending at `m`.
- `string.concat(s1, s2, ...)` returns a new string that is the
  concatenation of the given strings.
- `string.tostring(o)` transforms object `o` into a new string.
- `string.toint(x)` converts a string into an integer. If the
  conversion fails, this function returns `nil`.
- `string.tofloat(x)` converts a string into a float. If the
  conversion fails, this function returns `nil`.

## Additional String Operations
A number of additional string operations is available as a library
that must be included. The library is stored in
`INSTALL_PREFIX/share/buzz/include/string.bzz`, so to use it a
script must first include it. The complete reference of these
functions is included in the file.

## String Implementation in Buzz
The Buzz VM maintains a data structure that stores every string
that was ever encountered during the execution of a script. Each
string is associated with a unique identifier, which is simply a
counter of strings created so far. Every time a string is used in a
script, only the identifier is used. The actual value of the string
is stored only once in the data structure. To make string lookup
operations fast, strings are stored in a binary tree ordered by
identifier.

String manipulation often creates large numbers of intermediate
strings, which are used only once over the lifetime of a script. To
save memory, the Buzz VM internally distinguishes between
*protected* and *non-protected* strings -- protected strings are
stored permanently, while non-protected strings are erased during
garbage collection if no script variable refers to them. Examples
of protected strings are function names, constant names, and other
strings that are produced during compilation. Strings that result
from manipulation with the string operations are typically
non-protected.

When strings are communicated between robots, they must be
serialized. It is not possible to force the string indentifiers to
be the same across different robots. This is because (in general)
different robots might execute different parts of a script, and
strings might be created in different order. Therefore, when two
robots exchange strings, the full value of the string must be
communicated, rather than their identifier.

## Usage Example

```ruby
include "string.bzz"

# String creation
my_string = "hello"

# String concatenation
my_other_string = string.concat(my_string, " world!")

# Conversion of int to string
my_number_string = string.tostring(3)
```

<a name="files"></a>

# Files
To handle files, Buzz offers a number of built-in functions
collected in the `io` table. All file management functions are static members of the `io` class,
and are therefore used with the syntax `io.a_function()` where `io` is the builtin class.
- `fopen(path, mode)` opens a file located at `path`. Both parameters are strings. Parameter `mode` encodes how the file is opened:
  - `"r"` opens the file read-only;
  - `"w"` opens the file write-only and truncates the file;
  - `"a"` opens the file write-only for appending data;
  - `"w+"` opens the file for both reading and writing, and truncates the file;
  - `"a+"` opens the file for both reading and writing, for appending.
  This function returns `nil` in case of error, and a table in case
  of success. The table contains the methods listed in the
  following.
- `fclose(f)` closes file `f`.
- `fsize(f)` returns the size of file `f`.
- `fforeach(f, func)` executes function `func` for each line of file `f`.
- `fwrite(f, s1, s2, ...)` writes the concatenation of strings `s1`, `s2`, ... into file `f`.

Additionally, two attributes are available on the `io` class and are accessed with the syntax `io.an_attribute`:
- `errno` returns an integer representing the last error id.
  If there is no error, returns 0.
- `error_message` returns a message describing the last error.
  If there is no error, returns "No error".

## Usage Example
This example appends a line into an existing csv file.

```ruby
# Open file in append mode
result_file = io.fopen("results.csv", "a")

# Write a line in the file
io.fwrite(result_file, x_key, ",", y_key, ",", radiation)

# Close the file
io.fclose(result_file)
```

<a name="queues"></a>

# Queues
Because it is often necessary to prioritize treating some data before other data, Buzz offers the possibility to store data in fixed-size queues.
The Buzz queue is essentially a fized-size circular buffer implemented using tables.
The library is stored in `INSTALL_PREFIX/share/buzz/include/queue.bzz`, so to use it a script must first include it. The complete reference of these
functions is included in the file.

## Usage Example
```ruby
include "queue.bzz"

# Queue creation, with max size of 3
my_queue = queue.new(3)

# Adding elements to queue
queue.push(my_queue, 0)
queue.push(my_queue, 1)
queue.push(my_queue, 2)

# Removing an element
queue.pop(my_queue)

# Printing the contents of the queue
queue.print(my_queue)

```

<a name="swarm"></a>

# Swarm Management
Because Buzz is a DSL specifically aimed at swarm management, it includes functions dedicated to this purpose.
It allows the creation of swarms as sets of robots to facilitate coordination, task allocation, etc.

There are two categories of functions exposed by the swarm API: 
those who can be likened to static functions in Python, and those which are instance functions.

## Static swarm functions
These functions are inspired from typical set operations.
They are used with the syntax `swarm.a_function()`, where `swarm` is a builtin class. For example:

```ruby
s = swarm.create(1)
```

- `create(i)` : Creates a swarm with identifier `i`.
- `intersection(i, a, b)` : Creates a new swarm with identifier `i` with the intersection
  of previously created swarms `a` and `b`. 
- `union(i, a, b)` : Creates a new swarm with identifier `i` with the union
  of previously created swarms `a` and `b`.
- `difference(i, a, b)` : Creates a new swarm with identifier `i` with the robots from swarm `a` who are not in swarm `b`. 

## Instance swarm functions
These functions are called on an instance of a swarm.
They are used with the syntax `s.a_function()` where `s` is an instance of `swarm`. For example:

```ruby
s = swarm.create(1)
s.join()
```

- `join()` : Join the swarm unconditionally.
- `select(conditonal_statement)` : Join the swarm only if `conditonal_statement` is true.
- `unselect(conditonal_statement)` : Leave the swarm only if `conditonal_statement` is true.
- `leave()` : Leave the swarm unconditionally.
- `in()` : Checks if the current robots belongs to the swarm.
- `exec(function() {...})` : Assigns a task to the swarm.
- `others(1)` : Creates a new swarm with identifier `i` wich is a negation of `s`.

## Instance swarm attributes
These are the attributes on each swarm instance.
They are used with the syntax `s.property` where `s` is an instance of `swarm`.

- `id` The id of the current swarm.

## Usage Example

```ruby
# Create a swarm and join it
my_swarm = swarm.create(1)
my_swarm.join()

# Assign a task to the swarm
my_swarm.exec(function() { log("I'm in a swarm!") })

# Leave the swarm
s.leave()
```

<a name="vstig"></a>

# Virtual Stigmergy
The virtual stigmergy is a conflict-free replicated data type (CRDT) and is implemented directly into Buzz as key-value distributed storage system.
It can be used to share information between the members of a swarm. The virtual stigmergy whitepaper can be found [here](https://doi.org/10.4108/eai.3-12-2015.2262503).

There are two categories of functions exposed by the virtual stigmergy API: 
those who can be likened to static functions in Python, and those which are instance functions.

## Static virtual stigmergy functions
This function is used with the syntax `stigmergy.a_function()`, where `stigmergy` is a builtin class.
For example:

```ruby
s = stigmergy.create(1)
```

- `create(i)` : Creates a virtual stigmergy with identifier `i`.

## Instance virtual stigmergy functions
- `get(key)` : Gets the element at position `key` in the virtual stigmergy.
  If there is no element at position `key`, returns `nil`.
- `put(key, value)` : Inserts element `value` at position `key` in the virtual stigmergy.
- `size()` : Gets the number of elements in the virtual stigmergy.
- `onconflict(i)` : Creates a virtual stigmergy with identifier `i`.
- `onconflictlost(i)` : Creates a virtual stigmergy with identifier `i`.
- `foreach(function(key, value, robot_id) {...})` : Iterates over each element contained in the stigmergy and applies a lambda function to it.

## Instance virtual stigmergy attributes
These are the attributes on each stigmergy instance.
They are used with the syntax `s.property` where `s` is an instance of `stigmergy`.

- `id` The id of the current stigmergy.

## Usage Example

```ruby
# Create a new virtual stigmergy
v = stigmergy.create(1)
 
# Write a (key, value) entry into the stigmergy
v.put("a", 6)
 
# Read a value from the stigmergy
x = v.get("a")
 
# Get the number of keys in the structure
log("The vstig has ", v.size(), " elements")
```


<a name="neighbors"></a>

# Neighbor Management
Buzz allows interactions with neighboring robots. This is especially useful for exchanging messages.
All neighbor management functions are static members of the `neighbors` class, and are therefore used with the syntax `neighbors.a_function()` where `neighbors` is the builtin class.

- `get(robot_id)` : Gets the data associated with the robot with `robot_id`.
  This data is a table with attributes representing the `elevation`, `distance` and `azimuth` of the given robot.
- `kin()` : Gets a table of the robots belonging to the same swarm as the current robot.
- `nonkin()` : Gets a table of the robots *not* belonging to the same swarm as the current robot.
- `map(function(robot_id, data) {...})` : Makes a new neighbor structure in which each element is transformed by the passed function.
- `reduce(function(robot_id, data, accumulator) {...}, init_value)` : Performs a left fold/accumulation/reduction operation on the neighbors' data.
  The `init_value` must have a format compatible with `data`'s format.
- `filter(function(robot_id, data) {...})` : Filters the neighbors according to a predicate ('boolean' function).
- `foreach(function(robot_id, data) {...})` : Calls a function for each neighbor.
- `count()` : Gets the number of neighbors.
- `broadcast(topic, value)` : Broadcasts a `value` on `topic` across the neighbors.
- `listen(topic, function(value_id, value, robot_id) {...})` : Installs a listener function for messages broadcast on `topic` by neighbors.
  When a message is received on `topic`, the listner function is called. The listner function must have parameters `value_id`, `value`, and `robot_id`.
- `ignore(topic)` : Removes the listener for a `topic` across the neighbors.

## Usage Example

```ruby
# Iteration (rid is the neighbor's id)
neighbors.foreach(function(rid, data) {
    log("robot ", rid, ": ",
        "distance  = ", data.distance, ", ",
        "azimuth   = ", data.azimuth, ", ",
        "elevation = ", data.elevation)
})
 
# Transformation
cartesian = neighbors.map(function(rid, data) {
    var c = {}
    c.x = data.distance * math.cos(data.elevation) * math.cos(data.azimuth)
    c.y = data.distance * math.cos(data.elevation) * math.sin(data.azimuth)
    c.z = data.distance * math.sin(data.elevation)

    return c
})
 
# Reduction (accum is a table) with values x, y, and z, initialized to 0
result = cartesian.reduce(function(rid, data, accum) {
    accum.x = accum.x + data.x
    accum.y = accum.y + data.y
    accum.z = accum.z + data.z

    return accum
}, { .x = 0, .y = 0, .z = 0 })
 
# Filtering
one_meter = neighbors.filter(function(rid, data) {
    # We assume the distance is expressed in centimeters
    return data.distance < 100
})
 
# Listening to a topic
neighbors.listen("key", function(vid, value, rid) {
    log("Got (", vid, ",", value, ") from robot #", rid)
})
 
# Stopping listening to a topic
neighbors.ignore("topic")
 
# Broadcasting a value on a topic
neighbors.broadcast("topic", value)
```


<a name="userdata"></a>

# User Data
