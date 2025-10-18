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