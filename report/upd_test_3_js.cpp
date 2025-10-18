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