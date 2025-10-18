#define UPDATE_4_BENCH_JSON()                                                                                          \
    BENCHMARK_ADVANCED("Update 4 JSON - Title Promotion")(Catch::Benchmark::Chronometer meter) {                       \
        meter.measure([&] {                                                                                            \
            std::vector<Title> titles;                                                                                 \
            if (auto const err = glz::read_file_json(titles, "../employees/title.json", std::string{})) {              \
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(err)));                             \
            }                                                                                                          \
            for (auto &t: titles) {                                                                                    \
                if (t.title == "Engineer" && t.from_date < std::string("2010-01-01")) {                                \
                    t.title = "Senior Engineer";                                                                       \
                }                                                                                                      \
            }                                                                                                          \
            if (auto const err2 = glz::write_file_json(titles, "../employees/title.json", std::string{})) {            \
                FAIL(std::format("Failed to write JSON file: {}", glz::format_error(err2)));                           \
            }                                                                                                          \
            return 0;                                                                                                  \
        });                                                                                                            \
    }

TEST_CASE("update-test-4-json", "[update-json][update-test-4][update-test]") { UPDATE_4_BENCH_JSON(); }