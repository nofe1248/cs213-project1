#define SALARY_BENCH_PG(LIM)                                                                                           \
    BENCHMARK_ADVANCED("PostgreSQL salary ORDER BY employee_id LIMIT " #LIM)(Catch::Benchmark::Chronometer meter) {    \
        pqxx::connection conn{pg_conn_str.data()};                                                                     \
        pqxx::read_transaction tx{conn};                                                                               \
        tx.exec("SET search_path TO employees");                                                                       \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto [amount]:                                                                                        \
                 tx.query<std::int64_t>("SELECT amount FROM salary ORDER BY employee_id LIMIT " #LIM)) {               \
                sum += amount;                                                                                         \
            }                                                                                                          \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-1-postgresql", "[retrieval-post][retrieval-test-1][retrieval-test]") {
    try {
        SALARY_BENCH_PG(100);
        SALARY_BENCH_PG(500);
        SALARY_BENCH_PG(1000);
        SALARY_BENCH_PG(5000);
        SALARY_BENCH_PG(10000);
        SALARY_BENCH_PG(50000);
        SALARY_BENCH_PG(100000);
        SALARY_BENCH_PG(500000);
        SALARY_BENCH_PG(1000000);
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to PostgreSQL server '{}'", e.what()));
    }
}