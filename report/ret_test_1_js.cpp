#define JSON_READ_BENCH(N)                                                                                             \
    BENCHMARK_ADVANCED("JSON Reading " #N)(Catch::Benchmark::Chronometer meter) {                                      \
        std::vector<Salary> salaries;                                                                                  \
        if (auto const error = glz::read_file_json(salaries, "../employees/salary.json", std::string{})) {             \
            FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                               \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &s: salaries | std::views::take(N))                                                        \
                sum += s.amount;                                                                                       \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-1-json", "[retrieval-json][retrieval-test-1][retrieval-test]") {
    BENCHMARK_ADVANCED("JSON File Loading")(Catch::Benchmark::Chronometer meter) {
        meter.measure([&] {
            std::vector<Salary> salaries;
            if (auto const error = glz::read_file_json(salaries, "../employees/salary.json", std::string{}))
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));
            return salaries.size();
        });
    };

    JSON_READ_BENCH(100);
    JSON_READ_BENCH(500);
    JSON_READ_BENCH(1000);
    JSON_READ_BENCH(5000);
    JSON_READ_BENCH(10000);
    JSON_READ_BENCH(50000);
    JSON_READ_BENCH(100000);
    JSON_READ_BENCH(500000);
    JSON_READ_BENCH(1000000);
}