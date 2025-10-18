#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <format>
#include <glaze/beve.hpp>
#include <glaze/glaze.hpp>
#include <iostream>
#include <libpq-fe.h>
#include <pqxx/pqxx>
#include <print>
#include <random>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

constexpr std::string_view pg_conn_str =
        "dbname=employees port=5432 host='127.0.0.1' application_name='cs213-project1' user='postgres' "
        "connect_timeout=5 sslmode=allow password='CS213-Project1'";
constexpr std::string_view og_conn_str =
        "dbname=employees port=8888 host='127.0.0.1' application_name='cs213-project1' user='postgres' "
        "connect_timeout=5 sslmode=allow password='CS213-Project1'";

struct Department {
    std::string id;
    std::string dept_name;
};

struct Salary {
    std::int64_t employee_id;
    std::int64_t amount;
    std::string from_date;
    std::string to_date;
};

struct DepartmentManager {
    std::int64_t employee_id;
    std::string department_id;
    std::string from_date;
    std::string to_date;
};

struct Employee {
    std::int64_t id;
    std::string birth_date;
    std::string first_name;
    std::string last_name;
    std::string gender;
    std::string hire_date;
};

struct DepartmentEmployee {
    std::int64_t employee_id;
    std::string department_id;
    std::string from_date;
    std::string to_date;
};

struct Title {
    std::int64_t employee_id;
    std::string title;
    std::string from_date;
    std::string to_date;
};

static auto today_ymd() -> std::string {
    using namespace std::chrono;
    auto const now_days = floor<days>(system_clock::now());
    year_month_day ymd{now_days};
    return std::format("{:%F}", ymd);
}

TEST_CASE("connection-test-postgresql", "[conn-test]") {
    try {
        pqxx::connection conn{pg_conn_str.data()};
        pqxx::work tx{conn};

        tx.exec("SET search_path TO employees");
        for (auto const &[id, name]: tx.query<std::string, std::string>("SELECT * FROM department")) {
            std::println("Department ID: {}, name: {}", id, name);
        }

        tx.commit();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to PostgreSQL server '{}'", e.what()));
    }
}

TEST_CASE("connection-test-opengauss", "[conn-test]") {
    try {
        pqxx::connection conn{og_conn_str.data()};
        pqxx::work tx{conn};

        tx.exec("SET search_path TO employees");
        for (auto const &[id, name]: tx.query<std::string, std::string>("SELECT * FROM department")) {
            std::println("Department ID: {}, name: {}", id, name);
        }

        tx.commit();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to openGauss server '{}'", e.what()));
    }
}

TEST_CASE("connection-test-json", "[conn-test]") {
    std::vector<Department> departments;
    if (auto const error = glz::read_file_json(departments, "../employees/department.json", std::string{})) {
        FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));
    }
    for (auto const &[id, dept_name]: departments) {
        std::println("Department ID: {}, name: {}", id, dept_name);
    }
}

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

#define SALARY_BENCH_OG(LIM)                                                                                           \
    BENCHMARK_ADVANCED("openGauss salary ORDER BY employee_id LIMIT " #LIM)(Catch::Benchmark::Chronometer meter) {     \
        pqxx::connection conn{og_conn_str.data()};                                                                     \
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

TEST_CASE("retrieval-test-1-opengauss", "[retrieval-open][retrieval-test-1][retrieval-test]") {
    try {
        SALARY_BENCH_OG(100);
        SALARY_BENCH_OG(500);
        SALARY_BENCH_OG(1000);
        SALARY_BENCH_OG(5000);
        SALARY_BENCH_OG(10000);
        SALARY_BENCH_OG(50000);
        SALARY_BENCH_OG(100000);
        SALARY_BENCH_OG(500000);
        SALARY_BENCH_OG(1000000);
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to PostgreSQL server '{}'", e.what()));
    }
}

#define JSON_READ_BENCH(N)                                                                                             \
    BENCHMARK_ADVANCED("JSON Reading " #N)(Catch::Benchmark::Chronometer meter) {                                      \
        std::vector<Salary> salaries;                                                                                  \
        if (auto const error = glz::read_file_json(salaries, "../employees/salary.json", std::string{})) {             \
            FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                               \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &s: salaries | std::views::take(N))                                                        \
                sum += s.amount;                                                                                       \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-1-json", "[retrieval-json][retrieval-test-1][retrieval-test]") {
    BENCHMARK_ADVANCED("JSON File Loading")(Catch::Benchmark::Chronometer meter) {
        meter.measure([&] {
            std::vector<Salary> salaries;
            if (auto const error = glz::read_file_json(salaries, "../employees/salary.json", std::string{}))
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));
            return salaries.size();
        });
    };

    JSON_READ_BENCH(100);
    JSON_READ_BENCH(500);
    JSON_READ_BENCH(1000);
    JSON_READ_BENCH(5000);
    JSON_READ_BENCH(10000);
    JSON_READ_BENCH(50000);
    JSON_READ_BENCH(100000);
    JSON_READ_BENCH(500000);
    JSON_READ_BENCH(1000000);
}

#define JOIN_BENCH_PG(DATE)                                                                                            \
    BENCHMARK_ADVANCED("Join Query PostgreSQL (>" DATE ")")(Catch::Benchmark::Chronometer meter) {                     \
        pqxx::connection conn{pg_conn_str.data()};                                                                     \
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

TEST_CASE("retrieval-test-2-postgresql", "[retrieval-post][retrieval-test-2][retrieval-test]") {
    try {
        JOIN_BENCH_PG("2000-01-01");
        JOIN_BENCH_PG("1999-01-01");
        JOIN_BENCH_PG("1998-01-01");
        JOIN_BENCH_PG("1997-01-01");
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to PostgreSQL server '{}'", e.what()));
    }
}

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

#define JOINT_JSON_READ_BENCH()                                                                                        \
    BENCHMARK_ADVANCED("JOINT JSON Reading")(Catch::Benchmark::Chronometer meter) {                                    \
        std::vector<Employee> employees;                                                                               \
        std::vector<Department> departments;                                                                           \
        std::vector<DepartmentEmployee> dept_emp;                                                                      \
        std::vector<Title> titles;                                                                                     \
        std::vector<Salary> salaries;                                                                                  \
        auto load_or_fail = [&](auto &out, std::string const &path) {                                                  \
            if (auto const error = glz::read_file_json(out, path, std::string{})) {                                    \
                FAIL(std::format("Failed to read JSON file {}: {}", path, glz::format_error(error)));                  \
            }                                                                                                          \
        };                                                                                                             \
        meter.measure([&] {                                                                                            \
            load_or_fail(employees, "../employees/employee.json");                                                     \
            load_or_fail(departments, "../employees/department.json");                                                 \
            load_or_fail(dept_emp, "../employees/department_employee.json");                                           \
            load_or_fail(titles, "../employees/title.json");                                                           \
            load_or_fail(salaries, "../employees/salary.json");                                                        \
        });                                                                                                            \
    }

#define JOIN_BENCH_JSON(DATE)                                                                                          \
    BENCHMARK_ADVANCED("Join Query JSON (>" DATE ")")(Catch::Benchmark::Chronometer meter) {                           \
        std::vector<Employee> employees;                                                                               \
        std::vector<Department> departments;                                                                           \
        std::vector<DepartmentEmployee> dept_emp;                                                                      \
        std::vector<Title> titles;                                                                                     \
        std::vector<Salary> salaries;                                                                                  \
        auto load_or_fail = [&](auto &out, std::string const &path) {                                                  \
            if (auto const error = glz::read_file_json(out, path, std::string{})) {                                    \
                FAIL(std::format("Failed to read JSON file {}: {}", path, glz::format_error(error)));                  \
            }                                                                                                          \
        };                                                                                                             \
        load_or_fail(employees, "../employees/employee.json");                                                         \
        load_or_fail(departments, "../employees/department.json");                                                     \
        load_or_fail(dept_emp, "../employees/department_employee.json");                                               \
        load_or_fail(titles, "../employees/title.json");                                                               \
        load_or_fail(salaries, "../employees/salary.json");                                                            \
                                                                                                                       \
        std::unordered_set<std::string> valid_depts;                                                                   \
        valid_depts.reserve(departments.size());                                                                       \
        for (auto const &d: departments)                                                                               \
            valid_depts.insert(d.id);                                                                                  \
                                                                                                                       \
        std::unordered_map<std::int64_t, std::int64_t> salary_sum_by_emp;                                              \
        salary_sum_by_emp.reserve(salaries.size() / 4 + 1);                                                            \
        for (auto const &s: salaries)                                                                                  \
            salary_sum_by_emp[s.employee_id] += s.amount;                                                              \
                                                                                                                       \
        std::unordered_map<std::int64_t, std::size_t> title_count_by_emp;                                              \
        title_count_by_emp.reserve(titles.size() / 2 + 1);                                                             \
        for (auto const &t: titles)                                                                                    \
            title_count_by_emp[t.employee_id]++;                                                                       \
                                                                                                                       \
        std::unordered_map<std::int64_t, std::size_t> dept_link_count_by_emp;                                          \
        dept_link_count_by_emp.reserve(dept_emp.size() / 2 + 1);                                                       \
        for (auto const &de: dept_emp) {                                                                               \
            if (valid_depts.find(de.department_id) != valid_depts.end()) {                                             \
                dept_link_count_by_emp[de.employee_id]++;                                                              \
            }                                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        meter.measure([&] {                                                                                            \
            std::int64_t total = 0;                                                                                    \
            for (auto const &e: employees) {                                                                           \
                if (e.hire_date > DATE) {                                                                              \
                    auto const titles_cnt = static_cast<std::int64_t>(title_count_by_emp[e.id]);                       \
                    auto const de_cnt = static_cast<std::int64_t>(dept_link_count_by_emp[e.id]);                       \
                    if (titles_cnt == 0 || de_cnt == 0)                                                                \
                        continue;                                                                                      \
                    auto const ssum = salary_sum_by_emp[e.id];                                                         \
                    total += ssum * (titles_cnt * de_cnt);                                                             \
                }                                                                                                      \
            }                                                                                                          \
            return total;                                                                                              \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-2-json", "[retrieval-json][retrieval-test-2][retrieval-test]") {
    JOINT_JSON_READ_BENCH();
    JOIN_BENCH_JSON("2000-01-01");
    JOIN_BENCH_JSON("1999-01-01");
    JOIN_BENCH_JSON("1998-01-01");
    JOIN_BENCH_JSON("1997-01-01");
}

#define AGGREGATE_BENCH_PG()                                                                                           \
    BENCHMARK_ADVANCED("Aggregate Query PostgreSQL")(Catch::Benchmark::Chronometer meter) {                            \
        pqxx::connection conn{pg_conn_str.data()};                                                                     \
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

TEST_CASE("retrieval-test-3-postgresql", "[retrieval-post][retrieval-test-3][retrieval-test]") {
    try {
        AGGREGATE_BENCH_PG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to PostgreSQL server '{}'", e.what()));
    }
}

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

#define AGGREGATE_JSON_READ_BENCH()                                                                                    \
    BENCHMARK_ADVANCED("JOINT JSON Reading")(Catch::Benchmark::Chronometer meter) {                                    \
        std::vector<Department> departments;                                                                           \
        std::vector<DepartmentEmployee> dept_emp;                                                                      \
        std::vector<Salary> salaries;                                                                                  \
        auto load_or_fail = [&](auto &out, std::string const &path) {                                                  \
            if (auto const error = glz::read_file_json(out, path, std::string{})) {                                    \
                FAIL(std::format("Failed to read JSON file {}: {}", path, glz::format_error(error)));                  \
            }                                                                                                          \
        };                                                                                                             \
        meter.measure([&] {                                                                                            \
            load_or_fail(departments, "../employees/department.json");                                                 \
            load_or_fail(dept_emp, "../employees/department_employee.json");                                           \
            load_or_fail(salaries, "../employees/salary.json");                                                        \
        });                                                                                                            \
    }

#define AGGREGATE_BENCH_JSON()                                                                                         \
    BENCHMARK_ADVANCED("Aggregate Query JSON")(Catch::Benchmark::Chronometer meter) {                                  \
        std::vector<Department> departments;                                                                           \
        std::vector<DepartmentEmployee> dept_emp;                                                                      \
        std::vector<Salary> salaries;                                                                                  \
        auto load_or_fail = [&](auto &out, std::string const &path) {                                                  \
            if (auto const error = glz::read_file_json(out, path, std::string{})) {                                    \
                FAIL(std::format("Failed to read JSON file {}: {}", path, glz::format_error(error)));                  \
            }                                                                                                          \
        };                                                                                                             \
        load_or_fail(departments, "../employees/department.json");                                                     \
        load_or_fail(dept_emp, "../employees/department_employee.json");                                               \
        load_or_fail(salaries, "../employees/salary.json");                                                            \
                                                                                                                       \
        std::unordered_map<std::string, std::string> dept_name_by_id;                                                  \
        dept_name_by_id.reserve(departments.size());                                                                   \
        for (auto const &d: departments)                                                                               \
            dept_name_by_id.emplace(d.id, d.dept_name);                                                                \
                                                                                                                       \
        std::unordered_map<std::int64_t, std::int64_t> curr_salary_by_emp;                                             \
        curr_salary_by_emp.reserve(salaries.size() / 4 + 1);                                                           \
        for (auto const &s: salaries) {                                                                                \
            if (s.to_date == "9999-01-01")                                                                             \
                curr_salary_by_emp[s.employee_id] = s.amount;                                                          \
        }                                                                                                              \
                                                                                                                       \
        meter.measure([&] {                                                                                            \
            struct Agg {                                                                                               \
                std::int64_t count{};                                                                                  \
                std::int64_t sum{};                                                                                    \
            };                                                                                                         \
            std::unordered_map<std::string, Agg> agg_by_dept;                                                          \
            agg_by_dept.reserve(dept_emp.size() / 2 + 1);                                                              \
                                                                                                                       \
            for (auto const &de: dept_emp) {                                                                           \
                auto it_name = dept_name_by_id.find(de.department_id);                                                 \
                if (it_name == dept_name_by_id.end())                                                                  \
                    continue;                                                                                          \
                auto it_sal = curr_salary_by_emp.find(de.employee_id);                                                 \
                if (it_sal == curr_salary_by_emp.end())                                                                \
                    continue;                                                                                          \
                auto &agg = agg_by_dept[it_name->second];                                                              \
                agg.count += 1;                                                                                        \
                agg.sum += it_sal->second;                                                                             \
            }                                                                                                          \
                                                                                                                       \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &kv: agg_by_dept) {                                                                        \
                auto const &a = kv.second;                                                                             \
                if (a.count > 0) {                                                                                     \
                    double avg = static_cast<double>(a.sum) / static_cast<double>(a.count);                            \
                    sum += avg;                                       \
                }                                                                                                      \
            }                                                                                                          \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-3-json", "[retrieval-json][retrieval-test-3][retrieval-test]") {
    AGGREGATE_JSON_READ_BENCH();
    AGGREGATE_BENCH_JSON();
}

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

#define TEXT_SEARCH_BENCH_OG()                                                                                         \
    BENCHMARK_ADVANCED("Text Search Query openGauss")(Catch::Benchmark::Chronometer meter) {                           \
        pqxx::connection conn{og_conn_str.data()};                                                                     \
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

TEST_CASE("retrieval-test-4-opengauss", "[retrieval-open][retrieval-test-4][retrieval-test]") {
    try {
        TEXT_SEARCH_BENCH_OG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to openGauss server '{}'", e.what()));
    }
}

#define TEXT_SEARCH_JSON_READ_BENCH()                                                                                  \
    BENCHMARK_ADVANCED("JSON Employee File Loading")(Catch::Benchmark::Chronometer meter) {                            \
        meter.measure([&] {                                                                                            \
            std::vector<Employee> employees;                                                                           \
            auto const error = glz::read_file_json(employees, "../employees/employee.json", std::string{});            \
            if (error)                                                                                                 \
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                           \
            return employees.size();                                                                                   \
        });                                                                                                            \
    }

#define TEXT_SEARCH_BENCH_JSON()                                                                                       \
    BENCHMARK_ADVANCED("Text Search Query JSON")(Catch::Benchmark::Chronometer meter) {                                \
        std::vector<Employee> employees;                                                                               \
        if (auto const error = glz::read_file_json(employees, "../employees/employee.json", std::string{})) {          \
            FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                               \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &e: employees) {                                                                           \
                if (e.first_name.starts_with("John") || e.last_name.ends_with("son"))                                  \
                    sum += e.id;                                                                                       \
            }                                                                                                          \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-4-json", "[retrieval-json][retrieval-test-4][retrieval-test]") {
    TEXT_SEARCH_JSON_READ_BENCH();
    TEXT_SEARCH_BENCH_JSON();
}


#define DATE_RANGE_BENCH_PG(START_DATE, END_DATE)                                                                      \
    BENCHMARK_ADVANCED("Date Range Query PostgreSQL " START_DATE " " END_DATE)(Catch::Benchmark::Chronometer meter) {  \
        pqxx::connection conn{pg_conn_str.data()};                                                                     \
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

TEST_CASE("retrieval-test-5-postgresql", "[retrieval-post][retrieval-test-5][retrieval-test]") {
    try {
        DATE_RANGE_BENCH_PG("1995-01-01", "1997-12-31");
        DATE_RANGE_BENCH_PG("1992-01-01", "2000-12-31");
        DATE_RANGE_BENCH_PG("1990-01-01", "2022-12-31");
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to PostgreSQL server '{}'", e.what()));
    }
}

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

#define DATE_RANGE_JSON_READ_BENCH()                                                                                   \
    BENCHMARK_ADVANCED("JSON Employee File Loading")(Catch::Benchmark::Chronometer meter) {                            \
        meter.measure([&] {                                                                                            \
            std::vector<Employee> employees;                                                                           \
            auto const error = glz::read_file_json(employees, "../employees/employee.json", std::string{});            \
            if (error)                                                                                                 \
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                           \
            return employees.size();                                                                                   \
        });                                                                                                            \
    }

#define DATE_RANGE_BENCH_JSON(START_DATE, END_DATE)                                                                    \
    BENCHMARK_ADVANCED("Date Range Query JSON " START_DATE " " END_DATE)(Catch::Benchmark::Chronometer meter) {        \
        std::vector<Employee> employees;                                                                               \
        if (auto const error = glz::read_file_json(employees, "../employees/employee.json", std::string{})) {          \
            FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                               \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &e: employees) {                                                                           \
                if (e.hire_date >= START_DATE && e.hire_date <= END_DATE)                                              \
                    sum += e.id;                                                                                       \
            }                                                                                                          \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-5-json", "[retrieval-json][retrieval-test-5][retrieval-test]") {
    DATE_RANGE_JSON_READ_BENCH();
    DATE_RANGE_BENCH_JSON("1995-01-01", "1997-12-31");
    DATE_RANGE_BENCH_JSON("1992-01-01", "2000-12-31");
    DATE_RANGE_BENCH_JSON("1990-01-01", "2022-12-31");
}

#define UPDATE_1_BENCH_PG()                                                                                            \
    BENCHMARK_ADVANCED("Update 1 PostgreSQL")(Catch::Benchmark::Chronometer meter) {                                   \
        pqxx::connection conn{pg_conn_str.data()};                                                                     \
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

TEST_CASE("update-test-1-postgresql", "[update-post][update-test-1][update-test]") {
    try {
        UPDATE_1_BENCH_PG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to PostgreSQL server '{}'", e.what()));
    }
}

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

// Update test 2:
// UPDATE salary SET amount = amount + 1 WHERE to_date = '9999-01-01'

#define UPDATE_2_BENCH_PG()                                                                                            \
    BENCHMARK_ADVANCED("Update 2 PostgreSQL")(Catch::Benchmark::Chronometer meter) {                                   \
        pqxx::connection conn{pg_conn_str.data()};                                                                     \
        {                                                                                                              \
            pqxx::nontransaction ntx{conn};                                                                            \
            ntx.exec("SET SESSION search_path TO employees");                                                          \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            pqxx::work tx{conn};                                                                                       \
            tx.exec("UPDATE salary SET amount = amount + 1 WHERE to_date = '9999-01-01'");                             \
            tx.commit();                                                                                               \
            return 0;                                                                                                  \
        });                                                                                                            \
    }

TEST_CASE("update-test-2-postgresql", "[update-post][update-test-2][update-test]") {
    try {
        UPDATE_2_BENCH_PG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to PostgreSQL server '{}'", e.what()));
    }
}

#define UPDATE_2_BENCH_OG()                                                                                            \
    BENCHMARK_ADVANCED("Update 2 openGauss")(Catch::Benchmark::Chronometer meter) {                                    \
        pqxx::connection conn{og_conn_str.data()};                                                                     \
        {                                                                                                              \
            pqxx::nontransaction ntx{conn};                                                                            \
            ntx.exec("SET SESSION search_path TO employees");                                                          \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            pqxx::work tx{conn};                                                                                       \
            tx.exec("UPDATE salary SET amount = amount + 1 WHERE to_date = '9999-01-01'");                             \
            tx.commit();                                                                                               \
            return 0;                                                                                                  \
        });                                                                                                            \
    }

TEST_CASE("update-test-2-opengauss", "[update-open][update-test-2][update-test]") {
    try {
        UPDATE_2_BENCH_OG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to openGauss server '{}'", e.what()));
    }
}

#define UPDATE_2_BENCH_JSON()                                                                                          \
    BENCHMARK_ADVANCED("Update 2 JSON")(Catch::Benchmark::Chronometer meter) {                                         \
        meter.measure([&] {                                                                                            \
            std::vector<Salary> salaries;                                                                              \
            if (auto const error = glz::read_file_json(salaries, "../employees/salary.json", std::string{})) {         \
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(error)));                           \
            }                                                                                                          \
            for (auto &s: salaries) {                                                                                  \
                if (s.to_date == "9999-01-01") {                                                                       \
                    ++s.amount;                                                                                        \
                }                                                                                                      \
            }                                                                                                          \
            if (auto const error = glz::write_file_json(salaries, "../employees/salary.json", std::string{})) {        \
                FAIL(std::format("Failed to write JSON file: {}", glz::format_error(error)));                          \
            }                                                                                                          \
            return 0;                                                                                                  \
        });                                                                                                            \
    }

TEST_CASE("update-test-2-json", "[update-json][update-test-2][update-test]") { UPDATE_2_BENCH_JSON(); }

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

#define UPDATE_3_BENCH_OG()                                                                                            \
    BENCHMARK_ADVANCED("Update 3 openGauss - Department Transfer")(Catch::Benchmark::Chronometer meter) {              \
        pqxx::connection conn{og_conn_str.data()};                                                                     \
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

TEST_CASE("update-test-3-opengauss", "[update-open][update-test-3][update-test]") {
    try {
        UPDATE_3_BENCH_OG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to openGauss server '{}'", e.what()));
    }
}

#define UPDATE_3_BENCH_JSON()                                                                                          \
    BENCHMARK_ADVANCED("Update 3 JSON - Department Transfer")(Catch::Benchmark::Chronometer meter) {                   \
        std::random_device rd;                                                                                         \
        std::mt19937 gen(rd());                                                                                        \
        meter.measure([&] {                                                                                            \
            std::vector<Department> departments;                                                                       \
            std::vector<DepartmentEmployee> dept_emp;                                                                  \
            if (auto const err1 = glz::read_file_json(departments, "../employees/department.json", std::string{})) {   \
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(err1)));                            \
            }                                                                                                          \
            if (auto const err2 =                                                                                      \
                        glz::read_file_json(dept_emp, "../employees/department_employee.json", std::string{})) {       \
                FAIL(std::format("Failed to read JSON file: {}", glz::format_error(err2)));                            \
            }                                                                                                          \
            std::vector<std::size_t> active_indices;                                                                   \
            active_indices.reserve(1024);                                                                              \
            for (std::size_t i = 0; i < dept_emp.size(); ++i) {                                                        \
                if (dept_emp[i].to_date == "9999-01-01")                                                               \
                    active_indices.push_back(i);                                                                       \
            }                                                                                                          \
            if (active_indices.empty() || departments.empty()) {                                                       \
                return 0;                                                                                              \
            }                                                                                                          \
            std::uniform_int_distribution<std::size_t> dis(0, active_indices.size() - 1);                              \
            auto const idx = active_indices[dis(gen)];                                                                 \
            auto const emp_id = dept_emp[idx].employee_id;                                                             \
            auto const curr_dept = dept_emp[idx].department_id;                                                        \
            auto const today = today_ymd();                                                                            \
            for (auto &de: dept_emp) {                                                                                 \
                if (de.employee_id == emp_id && de.to_date == "9999-01-01") {                                          \
                    de.to_date = today;                                                                                \
                }                                                                                                      \
            }                                                                                                          \
            std::vector<std::string> dept_ids;                                                                         \
            dept_ids.reserve(departments.size());                                                                      \
            for (auto const &d: departments)                                                                           \
                dept_ids.push_back(d.id);                                                                              \
            std::uniform_int_distribution<std::size_t> ddis(0, dept_ids.size() - 1);                                   \
            std::string new_dept = curr_dept;                                                                          \
            for (int attempt = 0; attempt < 5 and new_dept == curr_dept && dept_ids.size() > 1; ++attempt) {           \
                new_dept = dept_ids[ddis(gen)];                                                                        \
            }                                                                                                          \
            dept_emp.push_back(DepartmentEmployee{emp_id, new_dept, today, "9999-01-01"});                             \
            if (auto const err3 =                                                                                      \
                        glz::write_file_json(dept_emp, "../employees/department_employee.json", std::string{})) {      \
                FAIL(std::format("Failed to write JSON file: {}", glz::format_error(err3)));                           \
            }                                                                                                          \
            return 0;                                                                                                  \
        });                                                                                                            \
    }

TEST_CASE("update-test-3-json", "[update-json][update-test-3][update-test]") { UPDATE_3_BENCH_JSON(); }

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

#define UPDATE_4_BENCH_OG()                                                                                            \
    BENCHMARK_ADVANCED("Update 4 openGauss - Title Promotion")(Catch::Benchmark::Chronometer meter) {                  \
        pqxx::connection conn{og_conn_str.data()};                                                                     \
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

TEST_CASE("update-test-4-opengauss", "[update-open][update-test-4][update-test]") {
    try {
        UPDATE_4_BENCH_OG();
    } catch (pqxx::sql_error const &e) {
        FAIL(std::format("Query failed with: {}", e.what()));
    } catch (std::exception const &e) {
        FAIL(std::format("Failed to connect to openGauss server '{}'", e.what()));
    }
}

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

TEST_CASE("convert-json-to-beve", "") {
    std::vector<Department> departments;
    std::vector<DepartmentEmployee> departmentEmployees;
    std::vector<DepartmentManager> departmentManagers;
    std::vector<Employee> employees;
    std::vector<Salary> salaries;
    std::vector<Title> titles;

    auto const load_or_error = [&](auto &vec, std::string const &path) {
        if (auto const err = glz::read_file_json(vec, path, std::string{})) {
            FAIL(std::format("Failed to read JSON file '{}': {}", path, glz::format_error(err)));
        }
    };

    load_or_error(departments, "../employees/department.json");
    load_or_error(departmentEmployees, "../employees/department_employee.json");
    load_or_error(departmentManagers, "../employees/department_manager.json");
    load_or_error(employees, "../employees/employee.json");
    load_or_error(salaries, "../employees/salary.json");
    load_or_error(titles, "../employees/title.json");

    auto const save_or_error = [&](auto const &vec, std::string const &path) {
        if (auto const err = glz::write_file_beve(vec, path, std::string{})) {
            FAIL(std::format("Failed to write BEVE file '{}': {}", path, glz::format_error(err)));
        }
    };

    save_or_error(departments, "../employees/department.beve");
    save_or_error(departmentEmployees, "../employees/department_employee.beve");
    save_or_error(departmentManagers, "../employees/department_manager.beve");
    save_or_error(employees, "../employees/employee.beve");
    save_or_error(salaries, "../employees/salary.beve");
    save_or_error(titles, "../employees/title.beve");
}

#define BEVE_READ_BENCH(N)                                                                                             \
    BENCHMARK_ADVANCED("BEVE Reading " #N)(Catch::Benchmark::Chronometer meter) {                                      \
        std::vector<Salary> salaries;                                                                                  \
        if (auto const error = glz::read_file_beve(salaries, "../employees/salary.beve", std::string{})) {             \
            FAIL(std::format("Failed to read BEVE file: {}", glz::format_error(error)));                               \
        }                                                                                                              \
        meter.measure([&] {                                                                                            \
            std::int64_t sum = 0;                                                                                      \
            for (auto const &s: salaries | std::views::take(N))                                                        \
                sum += s.amount;                                                                                       \
            return sum;                                                                                                \
        });                                                                                                            \
    }

TEST_CASE("retrieval-test-1-beve", "[retrieval-beve][retrieval-test-1][retrieval-test]") {
    BENCHMARK_ADVANCED("BEBE File Loading")(Catch::Benchmark::Chronometer meter) {
        meter.measure([&] {
            std::vector<Salary> salaries;
            if (auto const error = glz::read_file_beve(salaries, "../employees/salary.beve", std::string{}))
                FAIL(std::format("Failed to read BEBE file: {}", glz::format_error(error)));
            return salaries.size();
        });
    };

    BEVE_READ_BENCH(100);
    BEVE_READ_BENCH(500);
    BEVE_READ_BENCH(1000);
    BEVE_READ_BENCH(5000);
    BEVE_READ_BENCH(10000);
    BEVE_READ_BENCH(50000);
    BEVE_READ_BENCH(100000);
    BEVE_READ_BENCH(500000);
    BEVE_READ_BENCH(1000000);
}