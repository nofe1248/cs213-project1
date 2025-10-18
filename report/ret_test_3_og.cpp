#define AGGREGATE_BENCH_OG()                                                                                           \
    BENCHMARK_ADVANCED("Aggregate Query openGauss")(Catch::Benchmark::Chronometer meter) {                             \
        pqxx::connection conn{og_conn_str.data()};                                                                     \
        pqxx::read_transaction tx{conn};                                                                               \
        tx.exec("SET search_path TO employees");                                                                       \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &[name, count, avg]:                                                                       \
                 tx.query<std::string, std::int64_t, double>("SELECT d.dept_name, COUNT(e.id), AVG(s.amount) "         \
                                                             "FROM department d "                                      \
                                                             "JOIN department_employee de ON d.id = de.department_id " \
                                                             "JOIN employee e ON de.employee_id = e.id "               \
                                                             "JOIN salary s ON e.id = s.employee_id "                  \
                                                             "WHERE s.to_date = '9999-01-01' "                         \
                                                             "GROUP BY d.dept_name")) {                                \
                sum += avg;                                                                                            \
            }                                                                                                          \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-3-opengauss", "[retrieval-open][retrieval-test-3][retrieval-test]") {
    try {
        AGGREGATE_BENCH_OG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to openGauss server '{}'", e.what()));
    }
}