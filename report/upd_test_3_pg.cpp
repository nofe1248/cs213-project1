#define UPDATE_3_BENCH_PG()                                                                                            \
    BENCHMARK_ADVANCED("Update 3 PostgreSQL - Department Transfer")(Catch::Benchmark::Chronometer meter) {             \
        pqxx::connection conn{pg_conn_str.data()};                                                                     \
        {                                                                                                              \
            pqxx::nontransaction ntx{conn};                                                                            \
            ntx.exec("SET SESSION search_path TO employees");                                                          \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            pqxx::work tx{conn};                                                                                       \
            pqxx::result r = tx.exec("SELECT employee_id, department_id FROM department_employee "                     \
                                     "WHERE to_date = '9999-01-01' ORDER BY random() LIMIT 1");                        \
            if (r.empty()) {                                                                                           \
                tx.commit();                                                                                           \
                return 0;                                                                                              \
            }                                                                                                          \
            auto const emp_id = r[0][0].as<std::int64_t>();                                                            \
            auto const curr_dept = r[0][1].as<std::string>();                                                          \
            auto const new_dept = tx.query_value<std::string>(std::format(                                             \
                    "SELECT id FROM department WHERE id <> {} ORDER BY random() LIMIT 1", tx.quote(curr_dept)));       \
            tx.exec(std::format("UPDATE department_employee SET to_date = CURRENT_DATE WHERE employee_id = {} AND "    \
                                "to_date = '9999-01-01'",                                                              \
                                emp_id));                                                                              \
            tx.exec(std::format("INSERT INTO department_employee (employee_id, department_id, from_date, to_date) "    \
                                "VALUES ({}, {}, CURRENT_DATE, '9999-01-01')",                                         \
                                emp_id, tx.quote(new_dept)));                                                          \
            tx.commit();                                                                                               \
            return 0;                                                                                                  \
        });                                                                                                            \
    }

TEST_CASE("update-test-3-postgresql", "[update-post][update-test-3][update-test]") {
    try {
        UPDATE_3_BENCH_PG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to PostgreSQL server '{}'", e.what()));
    }
}