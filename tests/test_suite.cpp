#include <iostream>
#include "test/TestSuite.hpp"

int main() {
    smartcampus::test::TestSuite suite;
    int result = suite.runAll(true);
    if (result != 0) {
        return result;
    }

    // Also verify non-verbose execution
    int resultQuiet = smartcampus::test::TestSuite::run(false);
    return resultQuiet;
}
