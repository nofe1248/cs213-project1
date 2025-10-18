#define TEXT_SEARCH_JSON_READ_BENCH()                                                                                  \
    BENCHMARK_ADVANCED("JSON Employee File Loading")(Catch::Benchmark::Chronometer meter) {                            \
        meter.measure([&] {                                                                                            \
            std::vector<Employee> employees;                                                                           \
            auto const error = glz::read_file_json(employees, "../employees/employee.json", std::string{});            \
            if (error)                                                                                                 \
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                           \
            return employees.size();                                                                                   \
        });                                                                                                            \
    }

#define TEXT_SEARCH_BENCH_JSON()                                                                                       \
    BENCHMARK_ADVANCED("Text Search Query JSON")(Catch::Benchmark::Chronometer meter) {                                \
        std::vector<Employee> employees;                                                                               \
        if (auto const error = glz::read_file_json(employees, "../employees/employee.json", std::string{})) {          \
            FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                               \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &e: employees) {                                                                           \
                if (e.first_name.starts_with("John") || e.last_name.ends_with("son"))                                  \
                    sum += e.id;                                                                                       \
            }                                                                                                          \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-4-json", "[retrieval-json][retrieval-test-4][retrieval-test]") {
    TEXT_SEARCH_JSON_READ_BENCH();
    TEXT_SEARCH_BENCH_JSON();
}