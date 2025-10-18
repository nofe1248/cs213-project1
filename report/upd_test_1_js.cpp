#define UPDATE_1_BENCH_JSON()                                                                                          \
    BENCHMARK_ADVANCED("Update 1 JSON")(Catch::Benchmark::Chronometer meter) {                                         \
        std::random_device rd;                                                                                         \
        std::mt19937 gen(rd());                                                                                        \
        std::uniform_int_distribution<std::int64_t> dis(10001, 499999);                                                \
        auto const rand_id = dis(gen);                                                                                 \
        meter.measure([&] {                                                                                            \
            std::vector<Employee> employees;                                                                           \
            if (auto const error = glz::read_file_json(employees, "../employees/employee.json", std::string{})) {      \
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                           \
            }                                                                                                          \
            for (auto &e: employees) {                                                                                 \
                if (e.id == rand_id) {                                                                                 \
                    e.first_name = "John";                                                                             \
                    break;                                                                                             \
                }                                                                                                      \
            }                                                                                                          \
            if (auto const error = glz::write_file_json(employees, "../employees/employee.json", std::string{})) {     \
                FAIL(std::format("Failed to write JSON file: {}", glz::format_error(error)));                          \
            }                                                                                                          \
            return 0;                                                                                                  \
        });                                                                                                            \
    }

TEST_CASE("update-test-1-json", "[update-json][update-test-1][update-test]") { UPDATE_1_BENCH_JSON(); }