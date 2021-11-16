#include "../lib/doctest.h"
#include "../src/Parameters.h"

TEST_SUITE ("Parameters") {
    TEST_CASE ("parses 3 parameters") {
        std::string raw = "hello;123;foo";
        auto params = Parameters::parse(raw, 3);

        CHECK_EQ(params.size(), 3);
        CHECK_EQ(params.at(0), "hello");
        CHECK_EQ(params.at(1), "123");
        CHECK_EQ(params.at(2), "foo");
    }

    TEST_CASE("parses empty parameters") {
        auto params = Parameters::parse(";;", 3);
        CHECK_EQ(params.size(), 3);
        CHECK_EQ(params.at(0), "");
        CHECK_EQ(params.at(1), "");
        CHECK_EQ(params.at(2), "");
    }

}