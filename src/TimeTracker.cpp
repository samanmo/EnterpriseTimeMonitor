#include "TimeTracker.h"
#include "IRepository.h"       // <-- UPDATE THIS LINE EXACTLY TO THIS
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QDateTime>
#include <QHeaderView>
#include <QGroupBox>
#include <QFileDialog>
#include <QTextStream>

#include "data/VisualReportWindow.h"  // 🌟 Target the clean header map now
//#include "data/VisualReportWindow.cpp"  // <-- ADD THIS LINE HERE


TimeTracker::TimeTracker(std::shared_ptr<IRepository> repo, QWidget *parent) 
    : QWidget(parent), m_repo(repo) {
    
    stackedWidget = nullptr;
    employeeDropdown = nullptr;
    projectDropdown = nullptr;
    taskDropdown = nullptr;
    statusLabel = nullptr;
    actionButton = nullptr;
    publicActiveEmployeesTable = nullptr;
    
    newEmpIdInput = nullptr;
    newEmpNameInput = nullptr;
    newEmpRoleDropdown = nullptr;
    createEmpButton = nullptr;
    newProjNumInput = nullptr;
    newProjNameInput = nullptr;
    managerProjectDropdown = nullptr;
    managerTaskWidget = nullptr;
    newTaskInput = nullptr;
    managerActiveEmployeesTable = nullptr;
    reportProjectDropdown = nullptr;
    reportEmployeeDropdown = nullptr;

    stackedWidget = new QStackedWidget(this);

    QWidget *empView = createEmployeeView();
    QWidget *mgrView = createManagerView();

    stackedWidget->addWidget(empView);
    stackedWidget->addWidget(mgrView);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(stackedWidget);

    setWindowTitle("HR Roaming Enterprise Station (Refactored)");
    resize(800, 720);

    // Initial population using the abstraction contract layer
    projectDropdown->blockSignals(true);
    populateProjectDropdown();
    projectDropdown->blockSignals(false);

    populateEmployeeDropdown();
    populateManagerProjectDropdown();
    refreshManagerDashboard();

    if (projectDropdown->count() > 0) {
        projectDropdown->setCurrentIndex(0);
        handleProjectSelectionChange(0);
    }
    if (employeeDropdown->count() > 0) {
        employeeDropdown->setCurrentIndex(0);
        handleEmployeeSelectionChange(0);
    }
}

void TimeTracker::populateEmployeeDropdown() {
    if (!employeeDropdown || !reportEmployeeDropdown || !m_repo) return;
    
    employeeDropdown->blockSignals(true);
    reportEmployeeDropdown->blockSignals(true);
    
    employeeDropdown->clear();
    reportEmployeeDropdown->clear();

    auto employees = m_repo->fetchAllEmployees();
    for (const auto& emp : employees) {
        QString label = QString::fromStdString(emp.name) + " (" + 
                        QString::fromStdString(emp.id) + ")" + 
                        (emp.isManager ? " [MANAGER]" : "");
        
        employeeDropdown->addItem(label, QString::fromStdString(emp.id));
        reportEmployeeDropdown->addItem(label, QString::fromStdString(emp.id));
    }
    
    employeeDropdown->blockSignals(false);
    reportEmployeeDropdown->blockSignals(false);
}

void TimeTracker::populateProjectDropdown() {
    if (!projectDropdown || !reportProjectDropdown || !m_repo) return;
    
    projectDropdown->clear();
    reportProjectDropdown->clear();
    projectMap.clear();

    auto projects = m_repo->fetchAllProjects();
    for (const auto& proj : projects) {
        QString projNum = QString::fromStdString(proj.number);
        QString projName = QString::fromStdString(proj.name);
        projectMap.insert(projNum, proj.id);
        
        QString combined = QString("%1 - %2").arg(projNum).arg(projName);
        projectDropdown->addItem(combined, projNum);
        reportProjectDropdown->addItem(combined, projNum);
    }
}

void TimeTracker::handleProjectSelectionChange(int index) {
    Q_UNUSED(index);
    if (!projectDropdown || !taskDropdown || !m_repo) return;

    taskDropdown->clear();
    taskMap.clear();

    QString currentSelection = projectDropdown->currentText();
    QString currentProjNum = currentSelection.split(" - ").first();

    if (currentProjNum.isEmpty() || !projectMap.contains(currentProjNum)) return;
    int projectId = projectMap.value(currentProjNum);

    auto tasks = m_repo->fetchActiveTasksForProject(projectId);
    for (const auto& task : tasks) {
        QString name = QString::fromStdString(task.name);
        taskMap.insert(name, task.id);
        taskDropdown->addItem(name);
    }
}

void TimeTracker::handleEmployeeSelectionChange(int index) {
    if (index < 0 || !employeeDropdown || !actionButton || !statusLabel || !m_repo) return;
    
    QString empId = employeeDropdown->currentData().toString();
    if (empId.isEmpty()) return;

    Employee emp;
    if (!m_repo->fetchEmployeeById(empId.toStdString(), emp)) return;

    QString empName = QString::fromStdString(emp.name);

    if (emp.isManager) {
        statusLabel->setText(QString("Manager Identity Key Selected [%1]. System ready to bridge workspace.").arg(empName));
        actionButton->setText("Access Secure Manager Console");
        actionButton->setEnabled(true);
        projectDropdown->setEnabled(false);
        taskDropdown->setEnabled(false);
        return;
    }

    ActiveSession session;
    int logId = m_repo->checkActiveSession(empId.toStdString(), session);

    if (logId != -1) {
        statusLabel->setText(QString("Active Operations Logged: Clocked inside Project %1 (%2)")
            .arg(QString::fromStdString(session.projectNumber))
            .arg(QString::fromStdString(session.taskName)));
        actionButton->setText("Process Shift Clock-Out");
        actionButton->setEnabled(true);
        projectDropdown->setEnabled(false);
        taskDropdown->setEnabled(false);
    } else {
        statusLabel->setText("No active structural transaction detected. Select assignments below.");
        actionButton->setText("Process Shift Clock-In");
        projectDropdown->setEnabled(true);
        taskDropdown->setEnabled(true);
        actionButton->setEnabled(taskDropdown->count() > 0);
    }
}

void TimeTracker::processClockEvent() {
    if (!employeeDropdown || !actionButton || !m_repo) return;
    QString empId = employeeDropdown->currentData().toString();
    if (empId.isEmpty()) return;

    Employee emp;
    if (m_repo->fetchEmployeeById(empId.toStdString(), emp) && emp.isManager) {
        if (stackedWidget) {
            populateManagerProjectDropdown();
            refreshManagerDashboard();
            stackedWidget->setCurrentIndex(1);
        }
        return;
    }

    ActiveSession session;
    int logId = m_repo->checkActiveSession(empId.toStdString(), session);

    if (logId != -1) {
        if (m_repo->clockOut(logId)) {
            QMessageBox::information(this, "Success", "Shift clock-out tracking logged successfully.");
        } else {
            QMessageBox::critical(this, "Infrastructure Error", "Failed to compile clock-out transaction.");
        }
    } else {
        QString currentTask = taskDropdown->currentText();
        if (!taskMap.contains(currentTask)) return;
        int taskId = taskMap.value(currentTask);

        if (m_repo->clockIn(empId.toStdString(), taskId)) {
            QMessageBox::information(this, "Success", "Shift clock-in transaction recorded.");
        } else {
            QMessageBox::critical(this, "Infrastructure Error", "Failed to submit clock-in transaction.");
        }
    }

    refreshManagerDashboard();
    handleEmployeeSelectionChange(employeeDropdown->currentIndex());
}

QWidget* TimeTracker::createEmployeeView() {
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    QFormLayout *formLayout = new QFormLayout();

    employeeDropdown = new QComboBox(widget);
    projectDropdown = new QComboBox(widget);
    taskDropdown = new QComboBox(widget);

    formLayout->addRow("Select Employee Profile:", employeeDropdown);
    formLayout->addRow("Project Allocation:", projectDropdown);
    formLayout->addRow("Assigned Task Vector:", taskDropdown);
    layout->addLayout(formLayout);

    statusLabel = new QLabel("Terminal Active.", widget);
    statusLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(statusLabel);

    actionButton = new QPushButton("Process Transaction", widget);
    actionButton->setStyleSheet("background-color: #003366; color: white; font-weight: bold; padding: 7px;");
    layout->addWidget(actionButton);

    QLabel *dashTitle = new QLabel("Live Operations Monitor: Currently Clocked-In Staff Elements", widget);
    dashTitle->setStyleSheet("font-weight: bold; margin-top: 15px; color: #333;");
    layout->addWidget(dashTitle);

    publicActiveEmployeesTable = new QTableWidget(0, 4, widget);
    publicActiveEmployeesTable->setHorizontalHeaderLabels({"Staff Key ID", "Project Node", "Task Vector", "Running Since"});
    publicActiveEmployeesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    publicActiveEmployeesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(publicActiveEmployeesTable);

    connect(employeeDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TimeTracker::handleEmployeeSelectionChange);
    connect(projectDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TimeTracker::handleProjectSelectionChange);
    connect(actionButton, &QPushButton::clicked, this, &TimeTracker::processClockEvent);

    return widget;
}

QWidget* TimeTracker::createManagerView() {
    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);

    QGroupBox *empGroup = new QGroupBox("Corporate Workforce Enlistment Panel", widget);
    QFormLayout *empForm = new QFormLayout(empGroup);
    newEmpIdInput = new QLineEdit(widget);
    newEmpNameInput = new QLineEdit(widget);
    newEmpRoleDropdown = new QComboBox(widget);
    newEmpRoleDropdown->addItem("Standard Employee Asset", false);
    newEmpRoleDropdown->addItem("Privileged Executive Manager", true);
    
    createEmpButton = new QPushButton("Authorize Account Configuration Structure", widget);
    createEmpButton->setStyleSheet("background-color: #003366; color: white; font-weight: bold;");
    empForm->addRow("Assign Structural Key ID:", newEmpIdInput);
    empForm->addRow("Legal Identity Full Name:", newEmpNameInput);
    empForm->addRow("Functional System Privilege Role:", newEmpRoleDropdown);
    empForm->addRow(createEmpButton);
    layout->addWidget(empGroup);

    QHBoxLayout *projectHorizontalLayout = new QHBoxLayout();
    
    QGroupBox *projGroup = new QGroupBox("Project Architecture Setup", widget);
    QFormLayout *projForm = new QFormLayout(projGroup);
    newProjNumInput = new QLineEdit(widget);
    newProjNameInput = new QLineEdit(widget);
    QPushButton *createProjButton = new QPushButton("Publish Project Definition Module", widget);
    createProjButton->setStyleSheet("background-color: #28a745; color: white; font-weight: bold;");
    projForm->addRow("Unique Project Identifier Code:", newProjNumInput);
    projForm->addRow("Descriptive Assignment Title:", newProjNameInput);
    projForm->addRow(createProjButton);
    projectHorizontalLayout->addWidget(projGroup);

    QGroupBox *taskGroup = new QGroupBox("Linked Vector Assignment Space", widget);
    QVBoxLayout *taskLayout = new QVBoxLayout(taskGroup);
    managerProjectDropdown = new QComboBox(widget);
    managerTaskWidget = new QListWidget(widget);
    managerTaskWidget->setMaximumHeight(100);
    newTaskInput = new QLineEdit(widget);
    QPushButton *createTaskButton = new QPushButton("Append Task Target Strategy Vector", widget);
    createTaskButton->setStyleSheet("background-color: #28a745; color: white; font-weight: bold;");
    
    QFormLayout *taskForm = new QFormLayout();
    taskForm->addRow("Target Architecture Profile:", managerProjectDropdown);
    taskLayout->addLayout(taskForm);
    taskLayout->addWidget(new QLabel("Currently Registered Associated Tasks:", widget));
    taskLayout->addWidget(managerTaskWidget);
    
    QFormLayout *taskInputForm = new QFormLayout();
    taskInputForm->addRow("New Strategy Identifier String:", newTaskInput);
    taskLayout->addLayout(taskInputForm);
    taskLayout->addWidget(createTaskButton);
    projectHorizontalLayout->addWidget(taskGroup);
    
    layout->addLayout(projectHorizontalLayout);

    QGroupBox *reportingGroup = new QGroupBox("System Reporting Data Aggregation Matrix", widget);
    QFormLayout *reportingForm = new QFormLayout(reportingGroup);
    reportProjectDropdown = new QComboBox(widget);
    reportEmployeeDropdown = new QComboBox(widget);
    
    QHBoxLayout *exportButtonsLayout = new QHBoxLayout();
    QPushButton *exportProjButton = new QPushButton("Generate Selected Project History File", widget);
    QPushButton *exportEmpButton = new QPushButton("Generate Selected Employee Ledger File", widget);
    exportProjButton->setStyleSheet("font-weight: bold; padding: 4px;");
    exportEmpButton->setStyleSheet("font-weight: bold; padding: 4px;");
    
    exportButtonsLayout->addWidget(exportProjButton);
    exportButtonsLayout->addWidget(exportEmpButton);
    
    reportingForm->addRow("Filter Target Project Record Vector:", reportProjectDropdown);
    reportingForm->addRow("Filter Target Staff Ledger Vector:", reportEmployeeDropdown);
    reportingForm->addRow(exportButtonsLayout);
    layout->addWidget(reportingGroup);

    QLabel *managerTableLabel = new QLabel("Executive Monitor Overview: Active Operating Workspace Elements", widget);
    managerTableLabel->setStyleSheet("font-weight: bold; color: #111;");
    layout->addWidget(managerTableLabel);
    
    managerActiveEmployeesTable = new QTableWidget(0, 4, widget);
    managerActiveEmployeesTable->setHorizontalHeaderLabels({"Staff Key ID", "Target Allocation Node", "Assigned Strategy", "Session Duration Base"});
    managerActiveEmployeesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    layout->addWidget(managerActiveEmployeesTable);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *refreshButton = new QPushButton("Re-Query System Cluster Logs", widget);
    QPushButton *exitButton = new QPushButton("Secure Control Lock Out", widget);
    exitButton->setStyleSheet("background-color: #6c757d; color: white;");

    btnLayout->addWidget(refreshButton);
    btnLayout->addWidget(exitButton);
    layout->addLayout(btnLayout);

    connect(createEmpButton, &QPushButton::clicked, this, &TimeTracker::createNewEmployee);
    connect(createProjButton, &QPushButton::clicked, this, &TimeTracker::createNewProject);
    connect(createTaskButton, &QPushButton::clicked, this, &TimeTracker::createNewTask);
    connect(managerProjectDropdown, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TimeTracker::handleManagerProjectSelectionChange);
    connect(exportProjButton, &QPushButton::clicked, this, &TimeTracker::exportProjectReport);
    connect(exportEmpButton, &QPushButton::clicked, this, &TimeTracker::exportEmployeeReport);
    connect(refreshButton, &QPushButton::clicked, this, &TimeTracker::refreshManagerDashboard);
    connect(exitButton, &QPushButton::clicked, this, &TimeTracker::exitManagerView);

    return widget;
}

void TimeTracker::createNewEmployee() {
    if (!newEmpIdInput || !newEmpNameInput || !newEmpRoleDropdown || !m_repo) return;
    std::string empId = newEmpIdInput->text().trimmed().toStdString();
    std::string empName = newEmpNameInput->text().trimmed().toStdString();
    bool isManager = newEmpRoleDropdown->currentData().toBool();

    if (empId.empty() || empName.empty()) {
        QMessageBox::warning(this, "Input Violation", "All configuration parameters must be supplied to populate accounts.");
        return;
    }

    if (m_repo->createEmployee(empId, empName, isManager)) {
        newEmpIdInput->clear();
        newEmpNameInput->clear();
        populateEmployeeDropdown();
        QMessageBox::information(this, "Success", "Workforce configuration asset mapped to registry table space.");
    } else {
        QMessageBox::critical(this, "Database Fault", "Failed to write employee account specification mapping.");
    }
}

void TimeTracker::createNewProject() {
    if (!newProjNumInput || !newProjNameInput || !m_repo) return;
    std::string pNum = newProjNumInput->text().trimmed().toStdString();
    std::string pName = newProjNameInput->text().trimmed().toStdString();
    if (pNum.empty() || pName.empty()) return;

    if (m_repo->createProject(pNum, pName)) {
        newProjNumInput->clear();
        newProjNameInput->clear();
        
        projectDropdown->blockSignals(true);
        populateProjectDropdown();
        populateManagerProjectDropdown();
        projectDropdown->blockSignals(false);
        
        if (projectDropdown->count() > 0) {
            handleProjectSelectionChange(projectDropdown->currentIndex());
        }
        
        QMessageBox::information(this, "Success", "Project cluster parameters established.");
    } else {
        QMessageBox::critical(this, "Database Fault", "Failed to register corporate project structure template.");
    }
}

void TimeTracker::handleManagerProjectSelectionChange(int index) {
    Q_UNUSED(index);
    if (!managerProjectDropdown || !managerTaskWidget || !m_repo) return;
    
    managerTaskWidget->clear();
    
    QString currentSelection = managerProjectDropdown->currentText();
    QString pNum = currentSelection.split(" - ").first();
    if (pNum.isEmpty()) return;

    auto tasks = m_repo->fetchActiveTasksForProjectNumber(pNum.toStdString());
    for (const auto& taskName : tasks) {
        managerTaskWidget->addItem(QString::fromStdString(taskName));
    }
}

void TimeTracker::createNewTask() {
    if (!newTaskInput || !managerProjectDropdown || !m_repo) return;
    std::string tName = newTaskInput->text().trimmed().toStdString();
    QString currentSelection = managerProjectDropdown->currentText();
    QString pNum = currentSelection.split(" - ").first();

    if (tName.empty() || pNum.isEmpty()) return;

    if (m_repo->createTask(pNum.toStdString(), tName)) {
        newTaskInput->clear();
        handleManagerProjectSelectionChange(managerProjectDropdown->currentIndex());
        
        projectDropdown->blockSignals(true);
        handleProjectSelectionChange(projectDropdown->currentIndex());
        projectDropdown->blockSignals(false);
        
        QMessageBox::information(this, "Success", "Sub-task strategy route successfully updated.");
    } else {
        QMessageBox::critical(this, "Database Fault", "Failed to bind sub-task node onto project architecture target.");
    }
}

void TimeTracker::refreshManagerDashboard() {
    if (!managerActiveEmployeesTable || !publicActiveEmployeesTable || !m_repo) return;
    
    managerActiveEmployeesTable->setRowCount(0);
    publicActiveEmployeesTable->setRowCount(0);

    auto logs = m_repo->fetchActiveTimeLogs();
    int row = 0;
    for (const auto& log : logs) {
        QString empId = QString::fromStdString(log.employeeId);
        QString projNum = QString::fromStdString(log.projectNumber);
        QString taskName = QString::fromStdString(log.taskName);
        QString clockIn = QString::fromStdString(log.clockInTime);

        managerActiveEmployeesTable->insertRow(row);
        managerActiveEmployeesTable->setItem(row, 0, new QTableWidgetItem(empId));
        managerActiveEmployeesTable->setItem(row, 1, new QTableWidgetItem(projNum));
        managerActiveEmployeesTable->setItem(row, 2, new QTableWidgetItem(taskName));
        managerActiveEmployeesTable->setItem(row, 3, new QTableWidgetItem(clockIn));

        publicActiveEmployeesTable->insertRow(row);
        publicActiveEmployeesTable->setItem(row, 0, new QTableWidgetItem(empId));
        publicActiveEmployeesTable->setItem(row, 1, new QTableWidgetItem(projNum));
        publicActiveEmployeesTable->setItem(row, 2, new QTableWidgetItem(taskName));
        publicActiveEmployeesTable->setItem(row, 3, new QTableWidgetItem(clockIn));
        row++;
    }
}

void TimeTracker::exportProjectReport() {

    (new VisualReportWindow(reportProjectDropdown->currentData().toString().toStdString(), this))->exec();

    if (!reportProjectDropdown || !m_repo) return;
    QString pNum = reportProjectDropdown->currentData().toString();
    if (pNum.isEmpty()) {
        QMessageBox::warning(this, "Reporting Failure", "Select a valid structural target project registry mapping to compile records.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save Compiled Project Report Log", QString("Report_Project_%1.csv").arg(pNum), "Spreadsheet CSV Target (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);
    out << "Transaction Log ID,Corporate Employee Key ID,Operational Strategy Component,Shift Initialization Vector,Shift Termination Vector\n";

    auto records = m_repo->fetchReportByProject(pNum.toStdString());
    for (const auto& rec : records) {
        out << rec.logId << ","
            << QString::fromStdString(rec.employeeId) << ","
            << QString::fromStdString(rec.taskName) << ","
            << QString::fromStdString(rec.clockInTime) << ","
            << QString::fromStdString(rec.clockOutTime) << "\n";
    }
    QMessageBox::information(this, "Export Executed", "Target log history generated flawlessly.");
}

void TimeTracker::exportEmployeeReport() {
    if (!reportEmployeeDropdown || !m_repo) return;
    QString empId = reportEmployeeDropdown->currentData().toString();
    if (empId.isEmpty()) {
        QMessageBox::warning(this, "Reporting Failure", "Select a registered employee resource identifier to trace historic activity charts.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save Compiled Staff Ledger Log", QString("Report_Employee_%1.csv").arg(empId), "Spreadsheet CSV Target (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);
    out << "Transaction Log ID,Project Architecture Code,Operational Strategy Component,Shift Initialization Vector,Shift Termination Vector\n";

    auto records = m_repo->fetchReportByEmployee(empId.toStdString());
    for (const auto& rec : records) {
        out << rec.logId << ","
            << QString::fromStdString(rec.projectNumber) << ","
            << QString::fromStdString(rec.taskName) << ","
            << QString::fromStdString(rec.clockInTime) << ","
            << QString::fromStdString(rec.clockOutTime) << "\n";
    }
    QMessageBox::information(this, "Export Executed", "Target staff shift history ledger generated flawlessly.");
}

void TimeTracker::populateManagerProjectDropdown() {
    if (!managerProjectDropdown || !m_repo) return;
    
    managerProjectDropdown->blockSignals(true);
    managerProjectDropdown->clear();

    auto projects = m_repo->fetchAllProjects();
    for (const auto& proj : projects) {
        QString projNum = QString::fromStdString(proj.number);
        QString projName = QString::fromStdString(proj.name);
        managerProjectDropdown->addItem(QString("%1 - %2").arg(projNum).arg(projName));
    }
    
    managerProjectDropdown->blockSignals(false);
    if (managerProjectDropdown->count() > 0) {
        handleManagerProjectSelectionChange(0);
    }
}

void TimeTracker::exitManagerView() {
    if (stackedWidget) {
        if (employeeDropdown) {
            employeeDropdown->blockSignals(true);
            populateEmployeeDropdown();
            employeeDropdown->blockSignals(false);
            if (employeeDropdown->count() > 0) {
                employeeDropdown->setCurrentIndex(0);
                handleEmployeeSelectionChange(0);
            }
        }
        stackedWidget->setCurrentIndex(0);
    }
}

void TimeTracker::tryManagerAccess() {}
void TimeTracker::handleIdChange(const QString &text) { Q_UNUSED(text); }
