#pragma once
#include <string>
#include <vector>

// Pull architecture structures forward
struct Employee;
struct Project;
struct Task;
struct TimeLog;
struct ReportRecord;
struct ActiveSession;

class IRepository {
public:
    virtual ~IRepository() = default;
    
    virtual std::vector<Employee> fetchAllEmployees() = 0;
    virtual std::vector<Project> fetchAllProjects() = 0;
    virtual std::vector<Task> fetchActiveTasksForProject(int projectId) = 0;
    virtual std::vector<std::string> fetchActiveTasksForProjectNumber(const std::string& projNum) = 0;
    virtual bool fetchEmployeeById(const std::string& empId, Employee& outEmp) = 0;
    
    virtual int checkActiveSession(const std::string& empId, ActiveSession& outSession) = 0;
    virtual bool clockIn(const std::string& empId, int taskId) = 0;
    virtual bool clockOut(int logId) = 0;
    
    virtual bool createEmployee(const std::string& id, const std::string& name, bool isManager) = 0;
    virtual bool createProject(const std::string& number, const std::string& name) = 0;
    virtual bool createTask(const std::string& projNum, const std::string& taskName) = 0;
    
    virtual std::vector<TimeLog> fetchActiveTimeLogs() = 0;
    virtual std::vector<ReportRecord> fetchReportByProject(const std::string& projNum) = 0;
    virtual std::vector<ReportRecord> fetchReportByEmployee(const std::string& empId) = 0;
};
