// include/data/VisualReportWindow.h
#pragma once

#include <QDialog>
#include <QTextEdit>
#include <string>

class VisualReportWindow : public QDialog {
    Q_OBJECT

public:
    explicit VisualReportWindow(const std::string& projectNum, QWidget* parent = nullptr);
    virtual ~VisualReportWindow() = default;

private slots:
    void handleExportToPdf();

private:
    std::string m_projectNum;
    QTextEdit* m_reportView;
};
