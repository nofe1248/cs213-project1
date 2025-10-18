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