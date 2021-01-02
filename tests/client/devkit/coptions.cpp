/**
 * Copyright 2018-2020 Pejman Ghorbanzade. All rights reserved.
 */

#include "catch2/catch.hpp"
#include "tmpfile.hpp"
#include "weasel/devkit/coptions.hpp"

using namespace weasel;

TEST_CASE("configure")
{
    weasel::COptions opts;
    std::unordered_map<std::string, std::string> input;

    SECTION("empty")
    {
        REQUIRE_NOTHROW(opts.parse(input));
        CHECK(opts.parse(input) == false);
        REQUIRE(opts.parse_error.empty() == false);
        CHECK_THAT(opts.parse_error, Catch::Contains("team"));
    }
    SECTION("api-key")
    {
        input.emplace("api-key", "some-api-key");
        REQUIRE_NOTHROW(opts.parse(input));
        CHECK(opts.parse(input) == false);
        REQUIRE(opts.parse_error.empty() == false);
        CHECK_THAT(opts.parse_error, Catch::Contains("team"));
        CHECK(opts.api_key == "some-api-key");
    }
    SECTION("single-call")
    {
        input = {
            { "team", "some-team" },
            { "suite", "some-suite" },
            { "version", "some-version" },
            { "api-key", "some-api-key" },
            { "api-url", "some-api-url" },
            { "handshake", "false" }
        };
        REQUIRE_NOTHROW(opts.parse(input));
        CHECK(opts.parse(input) == true);
        REQUIRE(opts.parse_error.empty() == true);
        CHECK(opts.team == "some-team");
        CHECK(opts.suite == "some-suite");
        CHECK(opts.revision == "some-version");
        CHECK(opts.api_key == "some-api-key");
        CHECK(opts.api_url == "some-api-url");
        CHECK(opts.handshake == false);
    }
    SECTION("multiple-calls")
    {
        input.emplace("team", "some-team");
        REQUIRE_NOTHROW(opts.parse(input));
        CHECK(opts.parse(input) == false);
        REQUIRE(opts.parse_error.empty() == false);
        CHECK_THAT(opts.parse_error, Catch::Contains("suite"));
        CHECK(opts.team == "some-team");
        input.emplace("suite", "some-suite");
        input.emplace("version", "some-version");
        CHECK(opts.parse(input) == false);
        REQUIRE(opts.parse_error.empty() == false);
        CHECK_THAT(opts.parse_error, Catch::Contains("api-key"));
        CHECK(opts.team == "some-team");
        CHECK(opts.suite == "some-suite");
        CHECK(opts.revision == "some-version");
        input.emplace("handshake", "false");
        CHECK(opts.parse(input) == true);
        CHECK(opts.handshake == false);
        CHECK(opts.parse_error.empty() == true);
    }
    SECTION("long-api-url")
    {
        input.emplace("api-url", "https://api.example.com/@/some-team/some-suite/some-version");
        input.emplace("api-key", "some-api-key");
        input.emplace("handshake", "false");
        CHECK(opts.parse(input) == true);
        CHECK(opts.parse_error.empty() == true);
        CHECK(opts.parse_error == "");
        CHECK(opts.team == "some-team");
        CHECK(opts.suite == "some-suite");
        CHECK(opts.revision == "some-version");
    }
    SECTION("missing-api-key")
    {
        input.emplace("api-url", "https://api.example.com/@/some-team/some-suite/some-version");
        CHECK(opts.parse(input) == false);
        CHECK(opts.parse_error.empty() == false);
        CHECK_THAT(opts.parse_error, Catch::Contains("api-key"));
        CHECK(opts.team == "some-team");
        CHECK(opts.suite == "some-suite");
        CHECK(opts.revision == "some-version");
    }
    SECTION("override-defaults")
    {
        input.emplace("api-url", "https://api.example.com/@/some-team/some-suite/some-version");
        input.emplace("handshake", "false");
        CHECK(opts.parse(input) == true);
        CHECK(opts.parse_error.empty() == true);
        CHECK(opts.case_declaration == weasel::ConcurrencyMode::AllThreads);
        CHECK(opts.post_max_cases == 10);
        CHECK(opts.post_max_retries == 2);
        input.emplace("testcase-declaration-mode", "per-thread");
        input.emplace("post-testcases", "3");
        input.emplace("post-maxretries", "20");
        CHECK(opts.parse(input) == true);
        CHECK(opts.case_declaration == weasel::ConcurrencyMode::PerThread);
        CHECK(opts.post_max_cases == 3);
        CHECK(opts.post_max_retries == 20);
    }
}

TEST_CASE("configure-by-file")
{
    weasel::COptions opts;
    TmpFile file;

    SECTION("missing-file")
    {
        CHECK_THROWS_AS(opts.parse_file(file.path), std::invalid_argument);
        CHECK_THROWS_WITH(opts.parse_file(file.path), "configuration file is missing");
    }

    SECTION("invalid-file")
    {
        file.write("");
        CHECK_THROWS_AS(opts.parse_file(file.path), std::invalid_argument);
        CHECK_THROWS_WITH(opts.parse_file(file.path), "configuration file is not valid");
    }

    SECTION("valid-file")
    {
        file.write(R"({"weasel":{"team":"myteam","suite":"mysuite","version":"myversion"}})");
        CHECK_NOTHROW(opts.parse_file(file.path));
        CHECK(opts.team == "myteam");
        CHECK(opts.suite == "mysuite");
        CHECK(opts.revision == "myversion");
    }

    SECTION("valid-file-verbose")
    {
        file.write(R"({"weasel":{"team":"myteam","suite":"mysuite","version":"myversion"}})");
        CHECK_NOTHROW(opts.parse_file(file.path));
    }
}
