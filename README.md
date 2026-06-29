
# SFMT fasm parser

- Parse tool for [Flat Assembler](https://flatassembler.net/)

## Build (CMake)

Uses some C++23 features:

- std::expected and std::unexpected for error handling without exceptions

- std::println for formatting and printing strings to stdout more easily than std::cout

If you need it to work with C++20:

- ignore everything in Diagnostic.h

- have SourceBytes::strip_bom() return void instead of Result, use standalone return to exit on its branches

- change SourceBytes::from_file(), have it return a SourceBytes instance, call strip_bom() unconditionally instead of checking for strip_bom()'s return value, don't check for !file after calling the std::ifstream constructor, and don't check for the buffer's size

Alternatively you could have anything that returns a Result<> just throw an exception

```sh
cmake -B <folder>

cmake --build <folder>

# Pass paths as args when you run it

./<folder>/sfmt <file1.s> <file2.s> <file3.s>
```

## Thoughts

- Primary use: assembler front-end

- DOD trial project (SoA, no multiple inheritance, no virtualization)
