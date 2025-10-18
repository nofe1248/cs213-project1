#define DATE_RANGE_BENCE_OG(START_DATE, END_DATE)                                                                      \
    BENCHMARK_ADVANCED("Date Range Query openGauss " START_DATE " " END_DATE)(Catch::Benchmark::Chronometer meter) {   \
        pqxx::connection conn{og_conn_str.data()};                                                                     \
        pqxx::read_transaction tx{conn};                                                                               \
        tx.exec("SET search_path TO employees");                                                                       \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &[id, hire_date]:                                                                          \
                 tx.query<std::int64_t, std::string>("SELECT id, hire_date FROM employee "                             \
                                                     "WHERE hire_date BETWEEN '" START_DATE "' AND '" END_DATE "' "    \
                                                     "ORDER BY hire_date DESC")) {                                     \
                sum += id;                                                                                             \
            }                                                                                                          \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-5-opengauss", "[retrieval-open][retrieval-test-5][retrieval-test]") {
    try {
        DATE_RANGE_BENCE_OG("1995-01-01", "1997-12-31");
        DATE_RANGE_BENCE_OG("1992-01-01", "2000-12-31");
        DATE_RANGE_BENCE_OG("1990-01-01", "2022-12-31");
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to openGauss server '{}'", e.what()));
    }
}