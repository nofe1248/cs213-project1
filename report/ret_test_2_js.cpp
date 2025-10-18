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