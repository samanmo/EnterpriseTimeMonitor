// include/data/IReportEngine.h
#pragma once
#include <string>
#include <vector>

struct ProgressMilestone {
    std::string description;
    double weightPercent;
    double progressThisMonth;
    double progressCumulative;
};

struct FinancialMetrics {
    double directCostThisMonth;
    double hrCostThisMonth;
    double totalCostThisMonth;
};

struct NERDCProgressReport {
    std::string projectName;
    std::string projectNumber;
    std::string engineerName;
    std::string allocationYear;
    std::vector<ProgressMilestone> milestones;
    FinancialMetrics financialLedger;
};

class IReportEngine {
public:
    virtual ~IReportEngine() = default;
    virtual NERDCProgressReport compileReport(const std::string& projectNum, int month, int year) = 0;
};
