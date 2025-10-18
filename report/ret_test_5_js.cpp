#define DATE_RANGE_JSON_READ_BENCH()                                                                                   \
    BENCHMARK_ADVANCED("JSON Employee File Loading")(Catch::Benchmark::Chronometer meter) {                            \
        meter.measure([&] {                                                                                            \
            std::vector<Employee> employees;                                                                           \
            auto const error = glz::read_file_json(employees, "../employees/employee.json", std::string{});            \
            if (error)                                                                                                 \
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                           \
            return employees.size();                                                                                   \
        });                                                                                                            \
    }

#define DATE_RANGE_BENCH_JSON(START_DATE, END_DATE)                                                                    \
    BENCHMARK_ADVANCED("Date Range Query JSON " START_DATE " " END_DATE)(Catch::Benchmark::Chronometer meter) {        \
        std::vector<Employee> employees;                                                                               \
        if (auto const error = glz::read_file_json(employees, "../employees/employee.json", std::string{})) {          \
            FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                               \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &e: employees) {                                                                           \
                if (e.hire_date >= START_DATE && e.hire_date <= END_DATE)                                              \
                    sum += e.id;                                                                                       \
            }                                                                                                          \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-5-json", "[retrieval-json][retrieval-test-5][retrieval-test]") {
    DATE_RANGE_JSON_READ_BENCH();
    DATE_RANGE_BENCH_JSON("1995-01-01", "1997-12-31");
    DATE_RANGE_BENCH_JSON("1992-01-01", "2000-12-31");
    DATE_RANGE_BENCH_JSON("1990-01-01", "2022-12-31");
}