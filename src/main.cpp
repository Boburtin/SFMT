
#include <print>

#include "SourceBytes.h"

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::println(stderr, "usage: SFMT <file path>");
        return 2;
    }
    ++argv;
    for (auto i {argc}; i-- > 1;) { std::println("{}", argv[argc - i - 1]); }
    return 0;
}