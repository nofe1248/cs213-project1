#define JOIN_BENCH_OG(DATE)                                                                                            \
    BENCHMARK_ADVANCED("Join Query openGauss (>" DATE ")")(Catch::Benchmark::Chronometer meter) {                      \
        pqxx::connection conn{og_conn_str.data()};                                                                     \
        pqxx::read_transaction tx{conn};                                                                               \
        tx.exec("SET search_path TO employees");                                                                       \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &[fn, ln, dn, title, amount]:                                                              \
                 tx.query<std::string, std::string, std::string, std::string, std::int64_t>(                           \
                         "SELECT e.first_name, e.last_name, d.dept_name, t.title, s.amount "                           \
                         "FROM employee e "                                                                            \
                         "JOIN department_employee de ON e.id = de.employee_id "                                       \
                         "JOIN department d ON de.department_id = d.id "                                               \
                         "JOIN title t ON e.id = t.employee_id "                                                       \
                         "JOIN salary s ON e.id = s.employee_id "                                                      \
                         "WHERE e.hire_date > '" DATE "' "                                                             \
                         "ORDER BY e.id")) {                                                                           \
                sum += amount;                                                                                         \
            }                                                                                                          \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-2-opengauss", "[retrieval-open][retrieval-test-2][retrieval-test]") {
    try {
        JOIN_BENCH_OG("2000-01-01");
        JOIN_BENCH_OG("1999-01-01");
        JOIN_BENCH_OG("1998-01-01");
        JOIN_BENCH_OG("1997-01-01");
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to openGauss server '{}'", e.what()));
    }
}