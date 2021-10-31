# Buzz Assembly Language Specification

## Assembly Commands

The Buzz Virtual Machine (BVM) is a [stack machine](https://en.wikipedia.org/wiki/Stack_machine) that works with a custom assembly language. The instruction set is composed of 46 elements. Each command can be used either by writing Buzz Assembly code directly (in a `.basm` file); most of them, have a corresponding C-function that can be used when integrating Buzz with other software.

The table below reports both the plain assembly command and the corresponding C function, along with a brief description of the effect of a command. In the description, `stack(N)` stands for the `N`-th element on the stack. The stack-top element is `stack(1)`; the element beneath it is `stack(2)`, and so on. If the stack contains `K` elements, the bottom element is `stack(K)`.

| Command       | C function                | Description |
| ------------- | ------------------------- | ----------- |
| `nop`         |                           | No operation |
| `done`        | `buzzvm_done(VM)`         | Ends the execution |
| `pushnil`     | `buzzvm_pushnil(VM)`      | Pushes `nil` on the current stack |
| `dup`         | `buzzvm_dup(VM)`          | Pushes `stack(1)` on the current stack |
| `pop`         | `buzzvm_pop(VM)`          | Removes `stack(1)` from the current stack |
| `ret0`        | `buzzvm_ret0(VM)`         | Returns from a closure call without returning a value to the caller |
| `ret1`        | `buzzvm_ret0(VM)`         | Returns from a closure call and returns the current `stack(1)` to the caller |
| `add`         | `buzzvm_add(VM)`          | Pushes the result of `stack(1) + stack(2)`, pops operands |
| `sub`         | `buzzvm_sub(VM)`          | Pushes the result of `stack(1) - stack(2)`, pops operands |
| `mul`         | `buzzvm_mul(VM)`          | Pushes the result of `stack(1) * stack(2)`, pops operands |
| `div`         | `buzzvm_div(VM)`          | Pushes the result of `stack(1) / stack(2)`, pops operands |
| `mod`         | `buzzvm_mod(VM)`          | Pushes the result of `stack(1) % stack(2)`, pops operands |
| `pow`         | `buzzvm_pow(VM)`          | Pushes the result of `stack(1) ^ stack(2)`, pops operands |
| `unm`         | `buzzvm_unm(VM)`          | Pushes `-stack(1)`, pops operand |
| `and`         | `buzzvm_and(VM)`          | Pushes the result of logical `stack(1) AND stack(2)`, pops operands |
| `or`          | `buzzvm_or(VM)`           | Pushes the result of logical `stack(1) OR stack(2)`, pops operands |
| `not`         | `buzzvm_not(VM)`          | Pushes the result of logical `NOT stack(1)`, pops operand |
| `eq`          | `buzzvm_eq(VM)`           | Pushes the result of `stack(1) == stack(2)`, pops operands |
| `neq`         | `buzzvm_neq(VM)`          | Pushes the result of `stack(1) != stack(2)`, pops operands |
| `gt`          | `buzzvm_gt(VM)`           | Pushes the result of `stack(1) > stack(2)`, pops operands |
| `gte`         | `buzzvm_gte(VM)`          | Pushes the result of `stack(1) >= stack(2)`, pops operands |
| `lt`          | `buzzvm_lt(VM)`           | Pushes the result of `stack(1) < stack(2)`, pops operands |
| `lte`         | `buzzvm_lte(VM)`          | Pushes the result of `stack(1) <= stack(2)`, pops operands |
| `gload`       | `buzzvm_gload(VM)`        | Pushes the global variable corresponding to string at `stack(1)`, pops operand |
| `gstore`      | `buzzvm_gstore(VM)`       | Stores `stack(1)` into global variable whose name is at `stack(2)`, pop operands |
| `pusht`       | `buzzvm_pusht(VM)`        | Pushes an empty table on the current stack |
| `tput`        | `buzzvm_tput(VM)`         | Performs `t[k] = v`; `v` is `stack(1)`, `k` is `stack(2)`), `t` is `stack(3)`; pops operands |
| `tget`        | `buzzvm_tget(VM)`         | Pushes `t[k]` on the current stack; `k` is `stack(1)`), `t` is `stack(2)`; pops operands     |
| `callc`       | `buzzvm_callc(VM)`        | Calls the closure at `stack(1)` as a normal closure |
| `calls`       | `buzzvm_callc(VM)`        | Calls the closure at `stack(1)` as a swarm closure |
| `pushf CONST` | `buzzvm_pushf(VM, CONST)` | Pushes a floating-point constant on the current stack |
| `pushi CONST` | `buzzvm_pushi(VM, CONST)` | Pushes a 32-bit signed integer constant on the current stack |
| `pushs SID`   | `buzzvm_pushs(VM, SID)`   | Pushes a string (identified by the string id `SID`) on the current stack |
| `pushcn ADDR` | `buzzvm_pushcn(VM, ADDR)` | Pushes the native closure at address `ADDR` on the current stack |
| `pushcc CID`  | `buzzvm_pushcc(VM, CID)`  | Pushes the C closure identified by the id `CID` on the current stack |
| `pushl ADDR`  | `buzzvm_pushl(VM, ADDR)`  | Pushes the lambda at address `ADDR` on the current stack |
| `lload IDX`   | `buzzvm_lload(VM, IDX)`   | Pushes the local variable at index `IDX` on the current stack; index count starts at `1`. |
| `lstore IDX`  | `buzzvm_lstore(VM, IDX)`  | Stores `stack(1)` into the local variable at index `IDX`, pops operand; index count starts at `1` |
| `jump POS`    |                             | Sets the program counter to `POS` |
| `jumpz POS`   |                             | If `stack(1) == 0`, sets the program counter to `POS`; pops operand |
| `jumpnz POS`  |                             | If `stack(1) != 0`, sets the program counter to `POS`; pops operand |

## Debugging Information

To make human-readable error reporting possible, assembly code can be annotated with extra information. Debugging annotations are added to each assembly code line. To mark the beginning of the information, the character `|` is used; after this character, the line number, column number, and file name are reported, separated by commas. No spaces are allowed before or after the commas. Line and column counts start from 1. For example:

```
@__label_0
	pushs 0	|7,18,/Users/myuser/test.bzz
	pushi 2	|7,20,/Users/myuser/test.bzz
	gstore	|7,21,/Users/myuser/test.bzz
```

Debugging information is not mandatory for each line. In fact, the preamble of any Buzz assembly file (the part in which strings are recorded and global symbols are registered) has no associated debugging information.

### Tool support

* Debugging information is automatically generated by [bzzparse](../toolset.md#bzzparse) upon compiling a Buzz script.
* [bzzasm](../toolset.md#bzzasm) takes each assembly line and uses the assembly command to produce bytecode, and the associated debugging information to produce a debugging information file.
* [bzzdeasm](../toolset.md#bzzdeasm) performs the opposite process: it takes as input a bytecode file and a debugging information file, and produces an annotated assembly code file.
