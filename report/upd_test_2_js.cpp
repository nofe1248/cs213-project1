#define UPDATE_2_BENCH_JSON()                                                                                          \
    BENCHMARK_ADVANCED("Update 2 JSON")(Catch::Benchmark::Chronometer meter) {                                         \
        meter.measure([&] {                                                                                            \
            std::vector<Salary> salaries;                                                                              \
            if (auto const error = glz::read_file_json(salaries, "../employees/salary.json", std::string{})) {         \
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                           \
            }                                                                                                          \
            for (auto &s: salaries) {                                                                                  \
                if (s.to_date == "9999-01-01") {                                                                       \
                    ++s.amount;                                                                                        \
                }                                                                                                      \
            }                                                                                                          \
            if (auto const error = glz::write_file_json(salaries, "../employees/salary.json", std::string{})) {        \
                FAIL(std::format("Failed to write JSON file: {}", glz::format_error(error)));                          \
            }                                                                                                          \
            return 0;                                                                                                  \
        });                                                                                                            \
    }

TEST_CASE("update-test-2-json", "[update-json][update-test-2][update-test]") { UPDATE_2_BENCH_JSON(); }