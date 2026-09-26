#ifndef TIMETRACKER_H
#define TIMETRACKER_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QMap>
#include <memory>
#include <string>
#include <vector>

// Forward declaration
class IRepository;

// Unified Structural Domains
struct Employee {
    std::string id;
    std::string name;
    bool isManager;
};

struct Project {
    int id;
    std::string number;
    std::string name;
};

struct Task {
    int id;
    std::string name;
};

struct TimeLog {
    std::string employeeId;
    std::string projectNumber;
    std::string taskName;
    std::string clockInTime;
};

struct ReportRecord {
    int logId;
    std::string employeeId;
    std::string projectNumber;
    std::string taskName;
    std::string clockInTime;
    std::string clockOutTime;
};

struct ActiveSession {
    int taskId;
    std::string projectNumber;
    std::string taskName;
    int logId;
};

class TimeTracker : public QWidget {
    Q_OBJECT

public:
    explicit TimeTracker(std::shared_ptr<IRepository> repo, QWidget *parent = nullptr);

private slots:
    void handleEmployeeSelectionChange(int index);
    void handleProjectSelectionChange(int index);
    void handleManagerProjectSelectionChange(int index);
    void processClockEvent();
    void createNewEmployee();
    void createNewProject();
    void createNewTask();
    void refreshManagerDashboard();
    void populateManagerProjectDropdown();
    void exitManagerView();
    void exportProjectReport();
    void exportEmployeeReport();
    void tryManagerAccess();          
    void handleIdChange(const QString &text); 

private:
    void populateProjectDropdown();
    void populateEmployeeDropdown();
    QWidget* createEmployeeView();
    QWidget* createManagerView();

    std::shared_ptr<IRepository> m_repo;
    QStackedWidget *stackedWidget;

    QMap<QString, int> projectMap;
    QMap<QString, int> taskMap;

    QComboBox *employeeDropdown;
    QComboBox *projectDropdown;
    QComboBox *taskDropdown;
    QLabel *statusLabel;
    QPushButton *actionButton;
    QTableWidget *publicActiveEmployeesTable;

    QLineEdit *newEmpIdInput;
    QLineEdit *newEmpNameInput;
    QComboBox *newEmpRoleDropdown;
    QPushButton *createEmpButton;
    
    QLineEdit *newProjNumInput;
    QLineEdit *newProjNameInput;
    QComboBox *managerProjectDropdown;
    QListWidget *managerTaskWidget; 
    QLineEdit *newTaskInput;
    QTableWidget *managerActiveEmployeesTable;

    QComboBox *reportProjectDropdown;
    QComboBox *reportEmployeeDropdown;
};

#endif // TIMETRACKER_H
