# EnterpriseTimeMonitor

An Enterprise Clock-IN / Clock-OUT application for monitoring employee hours. Built using **C++17**, **Qt6 (Widgets, SQL)**, and **PostgreSQL**.

## 🛠️ Prerequisites (Fedora)
To install the dependencies required to build and run this application locally:
```bash
sudo dnf install -y gcc-c++ cmake qt6-qtbase-devel
```

## 🚀 How to Build and Run
1. Create a build directory:
   ```bash
   mkdir build && cd build
   ```
2. Configure the project:
   ```bash
   cmake ..
   ```
3. Compile and launch:
   ```bash
   cmake --build .
   ./EnterpriseTimeMonitor
   ```

## 📄 License
This project is licensed under the MIT License.


## Brief Description
The project follows a clean Model-View-Controller (MVC) / Data-Repository pattern. This means data storage, user interface (UI), and reporting engines are all decoupled into specialized files so that modifying one doesn't break the others.


## Code Description

### include/TimeTracker.h

Outlines the core Desktop UI window class. It lists all interactive screen components—such as project selection drops, manager control layouts, and tracking operational tables—and defines how UI button clicks connect to background operations.

### include/IRepository.h
An Abstract Interface that defines data access patterns. It creates a standardized "contract" (e.g., fetchAllEmployees(), clockIn()) so that the user interface can pull project data without needing to know whether it's coming from an active server database, a local file, or mock arrays.

### include/data/IReportEngine.h
A specialized interface blueprint that outlines data objects needed to satisfy corporate reporting mandates. It maps variables for progress weight ratios, financial values, and milestones.

### include/data/VisualReportWindow.h
The structural blueprint for the new popup modal window module. It declares the dialog interface frame and the user-triggered slots needed to generate official document PDF conversions.

