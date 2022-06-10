# Unit tests

This directory contains unit tests for Buzz.

The tests were created using the Google Test Framework.
A primer on the framework can be found [here](https://google.github.io/googletest/primer.html).

## Compiling and running the tests

Detailed compiling and test execution information can be found [here](https://google.github.io/googletest/quickstart-cmake.html).

To compile the tests, run these commands in the current directory:

```bash
cmake -S . -B build
cmake --build build
```

Then, to run the tests:

```bash
cd build && ctest
```
