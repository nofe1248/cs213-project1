#define UPDATE_4_BENCH_PG()                                                                                            \
    BENCHMARK_ADVANCED("Update 4 PostgreSQL - Title Promotion")(Catch::Benchmark::Chronometer meter) {                 \
        pqxx::connection conn{pg_conn_str.data()};                                                                     \
        {                                                                                                              \
            pqxx::nontransaction ntx{conn};                                                                            \
            ntx.exec("SET SESSION search_path TO employees");                                                          \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            pqxx::work tx{conn};                                                                                       \
            tx.exec("UPDATE title SET title = 'Senior Engineer' WHERE title = 'Engineer' AND from_date < "             \
                    "'2010-01-01'");                                                                                   \
            tx.commit();                                                                                               \
            return 0;                                                                                                  \
        });                                                                                                            \
    }

TEST_CASE("update-test-4-postgresql", "[update-post][update-test-4][update-test]") {
    try {
        UPDATE_4_BENCH_PG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to PostgreSQL server '{}'", e.what()));
    }
}