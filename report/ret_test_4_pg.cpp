#define TEXT_SEARCH_BENCH_PG()                                                                                         \
    BENCHMARK_ADVANCED("Text Search Query PostgreSQL")(Catch::Benchmark::Chronometer meter) {                          \
        pqxx::connection conn{pg_conn_str.data()};                                                                     \
        pqxx::read_transaction tx{conn};                                                                               \
        tx.exec("SET search_path TO employees");                                                                       \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &[id, first_name, last_name]: tx.query<std::int64_t, std::string, std::string>(            \
                         "SELECT id, first_name, last_name FROM employee WHERE first_name LIKE 'John%' OR last_name "  \
                         "LIKE '%son'")) {                                                                             \
                sum += id;                                                                                             \
            }                                                                                                          \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-4-postgresql", "[retrieval-post][retrieval-test-4][retrieval-test]") {
    try {
        TEXT_SEARCH_BENCH_PG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to PostgreSQL server '{}'", e.what()));
    }
}