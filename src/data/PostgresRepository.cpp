#include "data/PostgresRepository.h"
#include "TimeTracker.h"       // <-- ADD THIS LINE RIGHT HERE
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>

PostgresRepository::PostgresRepository() {}

bool PostgresRepository::initializeConnection() {
    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        db = QSqlDatabase::database("qt_sql_default_connection");
    } else {
        db = QSqlDatabase::addDatabase("QPSQL");
        db.setHostName("127.0.0.1");
        db.setDatabaseName("hr_database");
        db.setUserName("hr_admin");
        db.setPassword("12345678");
        db.setPort(5432);
    }
    
    bool opened = db.open();
    if (opened) {
        initializeDatabaseSchema();
    }
    return opened;
}

void PostgresRepository::initializeDatabaseSchema() {
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS employees ("
               "employee_id VARCHAR(50) PRIMARY KEY, "
               "employee_name VARCHAR(100) NOT NULL, "
               "is_manager BOOLEAN DEFAULT FALSE)");
               
    query.exec("CREATE TABLE IF NOT EXISTS projects ("
               "project_id SERIAL PRIMARY KEY, "
               "project_number VARCHAR(50) UNIQUE NOT NULL, "
               "project_name VARCHAR(100) NOT NULL)");

    query.exec("CREATE TABLE IF NOT EXISTS tasks ("
               "task_id SERIAL PRIMARY KEY, "
               "project_id INT REFERENCES projects(project_id), "
               "task_name VARCHAR(100) NOT NULL, "
               "is_active BOOLEAN DEFAULT TRUE)");

    query.exec("CREATE TABLE IF NOT EXISTS employee_time_logs ("
               "log_id SERIAL PRIMARY KEY, "
               "employee_id VARCHAR(50) REFERENCES employees(employee_id), "
               "task_id INT REFERENCES tasks(task_id), "
               "clock_in_time TIMESTAMP NOT NULL, "
               "clock_out_time TIMESTAMP)");

    query.prepare("SELECT COUNT(*) FROM employees WHERE is_manager = TRUE");
    if (query.exec() && query.next() && query.value(0).toInt() == 0) {
        query.exec("INSERT INTO employees (employee_id, employee_name, is_manager) "
                   "VALUES ('MGR01', 'Default Systems Manager Asset', TRUE)");
        query.exec("INSERT INTO employees (employee_id, employee_name, is_manager) "
                   "VALUES ('EMP01', 'Standard Corporate Staff Member', FALSE)");
    }
}

std::vector<Employee> PostgresRepository::fetchAllEmployees() {
    std::vector<Employee> list;
    QSqlQuery query("SELECT employee_id, employee_name, is_manager FROM employees ORDER BY employee_name ASC", db);
    while (query.next()) {
        Employee emp;
        emp.id = query.value("employee_id").toString().toStdString();
        emp.name = query.value("employee_name").toString().toStdString();
        emp.isManager = query.value("is_manager").toBool();
        list.push_back(emp);
    }
    return list;
}

std::vector<Project> PostgresRepository::fetchAllProjects() {
    std::vector<Project> list;
    QSqlQuery query("SELECT project_id, project_number, project_name FROM projects ORDER BY project_number ASC", db);
    while (query.next()) {
        Project proj;
        proj.id = query.value("project_id").toInt();
        proj.number = query.value("project_number").toString().toStdString();
        proj.name = query.value("project_name").toString().toStdString();
        list.push_back(proj);
    }
    return list;
}

std::vector<Task> PostgresRepository::fetchActiveTasksForProject(int projectId) {
    std::vector<Task> list;
    QSqlQuery query(db);
    query.prepare("SELECT task_id, task_name FROM tasks WHERE project_id = :projId AND is_active = TRUE ORDER BY task_name ASC");
    query.bindValue(":projId", projectId);
    
    if (query.exec()) {
        while (query.next()) {
            Task t;
            t.id = query.value("task_id").toInt();
            t.name = query.value("task_name").toString().toStdString();
            list.push_back(t);
        }
    }
    return list;
}

std::vector<std::string> PostgresRepository::fetchActiveTasksForProjectNumber(const std::string& projNum) {
    std::vector<std::string> list;
    QSqlQuery query(db);
    query.prepare("SELECT task_name FROM tasks WHERE project_id = (SELECT project_id FROM projects WHERE project_number = :pNum) AND is_active = TRUE ORDER BY task_name ASC");
    query.bindValue(":pNum", QString::fromStdString(projNum));
    
    if (query.exec()) {
        while (query.next()) {
            list.push_back(query.value("task_name").toString().toStdString());
        }
    }
    return list;
}

bool PostgresRepository::fetchEmployeeById(const std::string& empId, Employee& outEmp) {
    QSqlQuery query(db);
    query.prepare("SELECT employee_id, employee_name, is_manager FROM employees WHERE employee_id = :id");
    query.bindValue(":id", QString::fromStdString(empId));
    
    if (query.exec() && query.next()) {
        outEmp.id = query.value("employee_id").toString().toStdString();
        outEmp.name = query.value("employee_name").toString().toStdString();
        outEmp.isManager = query.value("is_manager").toBool();
        return true;
    }
    return false;
}

int PostgresRepository::checkActiveSession(const std::string& empId, ActiveSession& outSession) {
    QSqlQuery query(db);
    query.prepare("SELECT l.log_id, l.task_id, t.task_name, p.project_number FROM employee_time_logs l "
                  "JOIN tasks t ON l.task_id = t.task_id "
                  "JOIN projects p ON t.project_id = p.project_id "
                  "WHERE l.employee_id = :empId AND l.clock_out_time IS NULL LIMIT 1");
    query.bindValue(":empId", QString::fromStdString(empId));

    if (query.exec() && query.next()) {
        outSession.taskId = query.value("task_id").toInt();
        outSession.projectNumber = query.value("project_number").toString().toStdString();
        outSession.taskName = query.value("task_name").toString().toStdString();
        outSession.logId = query.value("log_id").toInt();
        return outSession.logId;
    }
    return -1;
}

bool PostgresRepository::clockIn(const std::string& empId, int taskId) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO employee_time_logs (employee_id, task_id, clock_in_time) VALUES (:empId, :taskId, NOW())");
    query.bindValue(":empId", QString::fromStdString(empId));
    query.bindValue(":taskId", taskId);
    return query.exec();
}

bool PostgresRepository::clockOut(int logId) {
    QSqlQuery query(db);
    query.prepare("UPDATE employee_time_logs SET clock_out_time = NOW() WHERE log_id = :logId");
    query.bindValue(":logId", logId);
    return query.exec();
}

bool PostgresRepository::createEmployee(const std::string& id, const std::string& name, bool isManager) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO employees (employee_id, employee_name, is_manager) VALUES (:id, :name, :mgr)");
    query.bindValue(":id", QString::fromStdString(id));
    query.bindValue(":name", QString::fromStdString(name));
    query.bindValue(":mgr", isManager);
    return query.exec();
}

bool PostgresRepository::createProject(const std::string& number, const std::string& name) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO projects (project_number, project_name) VALUES (:num, :name)");
    query.bindValue(":num", QString::fromStdString(number));
    query.bindValue(":name", QString::fromStdString(name));
    return query.exec();
}

bool PostgresRepository::createTask(const std::string& projNum, const std::string& taskName) {
    QSqlQuery query(db);
    query.prepare("INSERT INTO tasks (project_id, task_name) VALUES ((SELECT project_id FROM projects WHERE project_number = :pNum), :tName)");
    query.bindValue(":pNum", QString::fromStdString(projNum));
    query.bindValue(":tName", QString::fromStdString(taskName));
    return query.exec();
}

std::vector<TimeLog> PostgresRepository::fetchActiveTimeLogs() {
    std::vector<TimeLog> list;
    QSqlQuery query("SELECT l.employee_id, p.project_number, t.task_name, l.clock_in_time FROM employee_time_logs l "
                    "JOIN tasks t ON l.task_id = t.task_id "
                    "JOIN projects p ON t.project_id = p.project_id "
                    "WHERE l.clock_out_time IS NULL ORDER BY l.clock_in_time ASC", db);
    while (query.next()) {
        TimeLog log;
        log.employeeId = query.value("employee_id").toString().toStdString();
        log.projectNumber = query.value("project_number").toString().toStdString();
        log.taskName = query.value("task_name").toString().toStdString();
        log.clockInTime = query.value("clock_in_time").toDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
        list.push_back(log);
    }
    return list;
}

std::vector<ReportRecord> PostgresRepository::fetchReportByProject(const std::string& projNum) {
    std::vector<ReportRecord> list;
    QSqlQuery query(db);
    query.prepare("SELECT l.log_id, l.employee_id, t.task_name, l.clock_in_time, l.clock_out_time FROM employee_time_logs l "
                  "JOIN tasks t ON l.task_id = t.task_id "
                  "JOIN projects p ON t.project_id = p.project_id "
                  "WHERE p.project_number = :pNum ORDER BY l.clock_in_time DESC");
    query.bindValue(":pNum", QString::fromStdString(projNum));

    if (query.exec()) {
        while (query.next()) {
            ReportRecord rec;
            rec.logId = query.value("log_id").toInt();
            rec.employeeId = query.value("employee_id").toString().toStdString();
            rec.taskName = query.value("task_name").toString().toStdString();
            rec.clockInTime = query.value("clock_in_time").toDateTime().toString(Qt::ISODate).toStdString();
            rec.clockOutTime = query.value("clock_out_time").isNull() ? "ACTIVE_UNTERMINATED" : query.value("clock_out_time").toDateTime().toString(Qt::ISODate).toStdString();
            list.push_back(rec);
        }
    }
    return list;
}

std::vector<ReportRecord> PostgresRepository::fetchReportByEmployee(const std::string& empId) {
    std::vector<ReportRecord> list;
    QSqlQuery query(db);
    query.prepare("SELECT l.log_id, p.project_number, t.task_name, l.clock_in_time, l.clock_out_time FROM employee_time_logs l "
                  "JOIN tasks t ON l.task_id = t.task_id "
                  "JOIN projects p ON t.project_id = p.project_id "
                  "WHERE l.employee_id = :empId ORDER BY l.clock_in_time DESC");
    query.bindValue(":empId", QString::fromStdString(empId));

    if (query.exec()) {
        while (query.next()) {
            ReportRecord rec;
            rec.logId = query.value("log_id").toInt();
            rec.projectNumber = query.value("project_number").toString().toStdString();
            rec.taskName = query.value("task_name").toString().toStdString();
            rec.clockInTime = query.value("clock_in_time").toDateTime().toString(Qt::ISODate).toStdString();
            rec.clockOutTime = query.value("clock_out_time").isNull() ? "ACTIVE_UNTERMINATED" : query.value("clock_out_time").toDateTime().toString(Qt::ISODate).toStdString();
            list.push_back(rec);
        }
    }
    return list;
}
