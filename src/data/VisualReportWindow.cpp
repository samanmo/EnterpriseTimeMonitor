// src/data/VisualReportWindow.cpp
#include "data/VisualReportWindow.h"
#include "data/IReportEngine.h"
#include "PostgresReportEngine.cpp" // Assuming your processing engine remains here

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextDocument>
#include <QPdfWriter>
#include <QString>

VisualReportWindow::VisualReportWindow(const std::string& projectNum, QWidget* parent) 
    : QDialog(parent), m_projectNum(projectNum) {
    
    setWindowTitle("NERDC Progress Dashboard Frame");
    resize(750, 680);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_reportView = new QTextEdit(this);
    m_reportView->setReadOnly(true);
    m_reportView->setStyleSheet("background-color: #fafafa; font-family: Arial; font-size: 11px;");

    // Pull database metrics payload
    PostgresReportEngine engine;
    NERDCProgressReport data = engine.compileReport(m_projectNum, 2, 2026); 

    // Generate Layout Framework
    QString html = "<html><body style='font-family: Arial, sans-serif; color: #333;'>";
    html += "<div style='text-align: center; border-bottom: 2px solid #003366; padding-bottom: 6px;'>";
    html += "  <b style='font-size: 13px;'>NATIONAL ENGINEERING RESEARCH AND DEVELOPMENT CENTRE OF SRI LANKA</b><br/>";
    html += "  <span style='font-size: 10px;'>Monthly Progress Report Form for Research Projects (Form No: QSR/R&D/03 A)</span>";
    html += "</div>";

    html += "<table width='100%' style='margin-top: 10px; font-size: 11px;'>";
    html += QString("<tr><td><b>1. Project Name:</b> %1</td><td><b>2. Code No:</b> %2</td></tr>").arg(QString::fromStdString(data.projectName)).arg(QString::fromStdString(data.projectNumber));
    html += QString("<tr><td><b>11. Project Lead:</b> %1</td><td><b>Allocation Year:</b> %2</td></tr>").arg(QString::fromStdString(data.engineerName)).arg(QString::fromStdString(data.allocationYear));
    html += "</table>";

    html += "<p style='font-size: 11px; font-weight: bold; margin-top: 12px;'>8. Physical Progress Tracking Model:</p>";
    html += "<table width='100%' border='1' cellspacing='0' cellpadding='4' style='border-collapse: collapse; font-size: 10px;'>";
    html += "  <tr style='background-color: #f2f2f2; font-weight: bold;'><th>Milestone Description</th><th>Weight</th><th>This Month</th><th>Cumulative</th></tr>";
    
    double totalWeight = 0, totalThisMonth = 0, totalCum = 0;
    for (const auto& ms : data.milestones) {
        html += QString("<tr><td>%1</td><td>%2%</td><td>%3%</td><td>%4%</td></tr>")
                .arg(QString::fromStdString(ms.description)).arg(ms.weightPercent).arg(ms.progressThisMonth).arg(ms.progressCumulative);
        totalWeight += ms.weightPercent;
        totalThisMonth += ms.progressThisMonth;
        totalCum += ms.progressCumulative;
    }
    html += QString("<tr style='font-weight: bold; background-color: #eaeaea;'><td>Total Documented Progress</td><td>%1%</td><td>%2%</td><td>%3%</td></tr>")
            .arg(totalWeight).arg(totalThisMonth).arg(totalCum);
    html += "</table>";

    html += "<p style='font-size: 11px; font-weight: bold; margin-top: 12px;'>11. Financial Cost Analysis Matrix (LKR):</p>";
    html += "<table width='100%' border='1' cellspacing='0' cellpadding='4' style='border-collapse: collapse; font-size: 10px;'>";
    html += "  <tr style='background-color: #f2f2f2; font-weight: bold;'><th>Cost Breakdown Frame</th><th>Value Ledger (LKR)</th></tr>";
    html += QString("<tr><td>Direct Operational Expenses Base</td><td>%1 LKR</td></tr>").arg(QString::number(data.financialLedger.directCostThisMonth, 'f', 2));
    html += QString("<tr><td>Human Resource Allocation Cost</td><td>%1 LKR</td></tr>").arg(QString::number(data.financialLedger.hrCostThisMonth, 'f', 2));
    html += QString("<tr style='font-weight: bold; background-color: #f9f9f9;'><td>Total Structural Cost Summary</td><td>%1 LKR</td></tr>").arg(QString::number(data.financialLedger.totalCostThisMonth, 'f', 2));
    html += "</table>";

    html += "</body></html>";

    m_reportView->setHtml(html);
    mainLayout->addWidget(m_reportView);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* pdfBtn = new QPushButton("Export Official PDF Document", this);
    QPushButton* closeBtn = new QPushButton("Dismiss View", this);
    
    pdfBtn->setStyleSheet("background-color: #28a745; color: white; font-weight: bold; padding: 5px;");
    closeBtn->setStyleSheet("background-color: #6c757d; color: white; padding: 5px;");

    buttonLayout->addWidget(pdfBtn);
    buttonLayout->addWidget(closeBtn);
    mainLayout->addLayout(buttonLayout);

    connect(pdfBtn, &QPushButton::clicked, this, &VisualReportWindow::handleExportToPdf);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void VisualReportWindow::handleExportToPdf() {
    QString defaultName = QString("NERDC_Report_%1.pdf").arg(QString::fromStdString(m_projectNum));
    QString fileName = QFileDialog::getSaveFileName(this, "Save Official PDF Progress Document", defaultName, "PDF Documents (*.pdf)");
    
    if (fileName.isEmpty()) return;

    QPdfWriter pdfWriter(fileName);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    QTextDocument* doc = m_reportView->document();
    doc->print(&pdfWriter);

    QMessageBox::information(this, "Export Status", "Official PDF report vector built successfully.");
}
