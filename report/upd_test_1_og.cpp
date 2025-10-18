#define UPDATE_1_BENCH_OG()                                                                                            \
    BENCHMARK_ADVANCED("Update 1 openGauss")(Catch::Benchmark::Chronometer meter) {                                    \
        pqxx::connection conn{og_conn_str.data()};                                                                     \
        {                                                                                                              \
            pqxx::nontransaction ntx{conn};                                                                            \
            ntx.exec("SET SESSION search_path TO employees");                                                          \
        }                                                                                                              \
        std::random_device rd;                                                                                         \
        std::mt19937 gen{rd()};                                                                                        \
        std::uniform_int_distribution<std::int64_t> dis(10001, 499999);                                                \
        meter.measure([&] {                                                                                            \
            pqxx::work tx{conn};                                                                                       \
            tx.exec(std::format("UPDATE employee SET first_name = 'John' WHERE id = {}", dis(gen)));                   \
            tx.commit();                                                                                               \
            return 0;                                                                                                  \
        });                                                                                                            \
    }

TEST_CASE("update-test-1-opengauss", "[update-open][update-test-1][update-test]") {
    try {
        UPDATE_1_BENCH_OG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to openGauss server '{}'", e.what()));
    }
}