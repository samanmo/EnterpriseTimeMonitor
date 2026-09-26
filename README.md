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

