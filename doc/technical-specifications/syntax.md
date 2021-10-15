# Buzz Syntax BNF Specification

For an example-driven explanation of the Buzz syntax, see the [Buzz API](../api.org). In what follows, we report a formal [Backus-Naur Form grammar](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_Form) specification.

## Tokens

An **identifier** in Buzz is defined by this regular expression:

```
[[:alpha:]_][[:alnum:]_]*
```

The recognized tokens are:

| Token                                 | Id              |
| ------------------------------------- | --------------- |
| *identifier*                          | `TOKID`         |
| *numeric constant*                    | `TOKCONST`      |
| `'_string_'`, `"_string_"`            | `TOKSTRING`     |
| `var`                                 | `TOKVAR`        |
| `nil`                                 | `TOKNIL`        |
| `if`                                  | `TOKIF`         |
| `else`                                | `TOKELSE`       |
| `function`                            | `TOKFUN`        |
| `return`                              | `TOKRETURN`     |
| `for`                                 | `TOKFOR`        |
| `while`                               | `TOKWHILE`      |
| `and`, `or`                           | `TOKLANDOR`     |
| `not`                                 | `TOKLNOT`       |
| `+`, `-`                              | `TOKADDSUB`     |
| `*`, `/`                              | `TOKMULDIV`     |
| `%`                                   | `TOKMOD`        |
| `^`                                   | `TOKPOW`        |
| `<<`, `>>`                            | `TOKLRSHIFT`    |
| `&`, `\|`                              | `TOKBANDOR`     |
| `!`                                   | `TOKBNOT`       |
| `{`                                   | `TOKBLOCKOPEN`  |
| `}`                                   | `TOKBLOCKCLOSE` |
| `(`                                   | `TOKPAROPEN`    |
| `)`                                   | `TOKPARCLOSE`   |
| `[`                                   | `TOKIDXOPEN`    |
| `]`                                   | `TOKIDXCLOSE`   |
| `;`,`\n`                              | `TOKSTATEND`    |
| `,`                                   | `TOKLISTSEP`    |
| `=`                                   | `TOKASSIGN`     |
| `.`                                   | `TOKDOT`        |
| `<`, `<=`, `>`, `>=`, `==`, `!=`      | `TOKCMP`        |

## Grammar
```
  script             ::= statlist
```

```
  statlist           ::= stat | statlist stat
  stat               ::= <nil> | vardef | fundef | if | loop | command
```

```
  block              ::= TOKBLOCKOPEN statlist TOKBLOCKCLOSE
```

```
  vardef             ::= TOKVAR TOKID | TOKVAR TOKID assignment
  fundef             ::= TOKFUN TOKID TOKPAROPEN idlist TOKPARCLOSE block
  if                 ::= TOKIF TOKPAROPEN condition TOKPARCLOSE block endif
  endif              ::= <nil> |  TOKELSE block
  loop               ::= forloop | whileloop
  forloop            ::= TOKFOR TOKPAROPEN idref TOKASSIGN expression TOKLISTSEP condition TOKLISTSEP idref TOKASSIGN expression TOKPARCLOSE block
  whileloop          ::= TOKWHILE TOKPAROPEN condition TOKPARCLOSE block
```

```
  conditionlist      ::= condition | conditionlist TOKLISTSEP condition
  condition          ::= comparison | condition TOKLANDOR comparison | TOKLNOT condition
  comparison         ::= expression | expression TOKCMP expression
```

```
  expression         ::= product | expression TOKADDSUB product
  product            ::= modulo | product TOKMULDIV modulo
  modulo             ::= power | modulo TOKMOD power
  power              ::= bitshift powerrest
  powerrest          ::= <nil> | TOKPOW power
  bitshift           ::= bitwiseandor | operand TOKLRSHIFT bitwiseandor
  bitwiseandor       ::= bitwisenot | bitwiseandor TOKBANDOR bitwisenot
  bitwisenot         ::= operand | TOKBNOT bitwisenot
  operand            ::= TOKNIL | TOKCONST | TOKSTRING | TOKPAROPEN condition TOKPARCLOSE | TOKADDSUB power | idref | lambda | tabledef
```

```
  command            ::= idref | idref assignment | TOKRETURN expression
  assignment         ::= TOKASSIGN expression
```

```
  tabledef           ::= TOKBLOCKOPEN TOKBLOCKCLOSE | TOKBLOCKOPEN tablefielddeflist TOKBLOCKCLOSE
  tablefielddef      ::= TOKDOT TOKID assignment | TOKDOT TOKCONST assignment
  tablefielddeflist  ::= tablefielddef | tablefielddeflist TOKLISTSEP tablefielddef
```

```
  idlist             ::= <nil> | TOKID | idlist TOKLISTSEP TOKID
  idreflist          ::= idref | idreflist TOKLISTSEP idref
  idref              ::= TOKID | idref TOKDOT TOKID | idref TOKIDXOPEN expression TOKIDXCLOSE | idref TOKPAROPEN conditionlist TOKPARCLOSE
```

```
  lambda             ::= TOKFUN TOKPAROPEN idlist TOKPARCLOSE block
```
