#pragma once
#include "IRepository.h"
#include <QSqlDatabase>
#include <vector>
#include <string>

class PostgresRepository : public IRepository {
public:
    PostgresRepository();
    ~PostgresRepository() override = default;

    bool initializeConnection();

    std::vector<Employee> fetchAllEmployees() override;
    std::vector<Project> fetchAllProjects() override;
    std::vector<Task> fetchActiveTasksForProject(int projectId) override;
    std::vector<std::string> fetchActiveTasksForProjectNumber(const std::string& projNum) override;
    bool fetchEmployeeById(const std::string& empId, Employee& outEmp) override;
    
    int checkActiveSession(const std::string& empId, ActiveSession& outSession) override;
    bool clockIn(const std::string& empId, int taskId) override;
    bool clockOut(int logId) override;
    
    bool createEmployee(const std::string& id, const std::string& name, bool isManager) override;
    bool createProject(const std::string& number, const std::string& name) override;
    bool createTask(const std::string& projNum, const std::string& taskName) override;
    
    std::vector<TimeLog> fetchActiveTimeLogs() override;
    std::vector<ReportRecord> fetchReportByProject(const std::string& projNum) override;
    std::vector<ReportRecord> fetchReportByEmployee(const std::string& empId) override;

private:
    void initializeDatabaseSchema();
    QSqlDatabase db;
};
