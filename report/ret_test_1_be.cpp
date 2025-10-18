#define BEVE_READ_BENCH(N)                                                                                             \
    BENCHMARK_ADVANCED("BEVE Reading " #N)(Catch::Benchmark::Chronometer meter) {                                      \
        std::vector<Salary> salaries;                                                                                  \
        if (auto const error = glz::read_file_beve(salaries, "../employees/salary.beve", std::string{})) {             \
            FAIL(std::format("Failed to read BEVE file: {}", glz::format_error(error)));                               \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &s: salaries | std::views::take(N))                                                        \
                sum += s.amount;                                                                                       \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-1-beve", "[retrieval-beve][retrieval-test-1][retrieval-test]") {
    BENCHMARK_ADVANCED("BEBE File Loading")(Catch::Benchmark::Chronometer meter) {
        meter.measure([&] {
            std::vector<Salary> salaries;
            if (auto const error = glz::read_file_beve(salaries, "../employees/salary.beve", std::string{}))
                FAIL(std::format("Failed to read BEBE file: {}", glz::format_error(error)));
            return salaries.size();
        });
    };

    BEVE_READ_BENCH(100);
    BEVE_READ_BENCH(500);
    BEVE_READ_BENCH(1000);
    BEVE_READ_BENCH(5000);
    BEVE_READ_BENCH(10000);
    BEVE_READ_BENCH(50000);
    BEVE_READ_BENCH(100000);
    BEVE_READ_BENCH(500000);
    BEVE_READ_BENCH(1000000);
}