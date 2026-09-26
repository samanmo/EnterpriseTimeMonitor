-- PostgreSQL Relational Schema Setup Script
-- Database Context: hr_database

-- Drop tables in reverse dependency order if resetting
DROP TABLE IF EXISTS employee_time_logs CASCADE;
DROP TABLE IF EXISTS tasks CASCADE;
DROP TABLE IF EXISTS projects CASCADE;
DROP TABLE IF EXISTS employees CASCADE;

-- 1. Corporate Workforce Master Registry Table
CREATE TABLE employees (
    employee_id VARCHAR(50) PRIMARY KEY,
    employee_name VARCHAR(100) NOT NULL,
    is_manager BOOLEAN DEFAULT FALSE
);

-- 2. Projects Master Cluster Table
CREATE TABLE projects (
    project_id SERIAL PRIMARY KEY,
    project_number VARCHAR(50) UNIQUE NOT NULL,
    project_name VARCHAR(100) NOT NULL
);

-- 3. Tasks / Strategy Vector Allocation Space
CREATE TABLE tasks (
    task_id SERIAL PRIMARY KEY,
    project_id INT REFERENCES projects(project_id) ON DELETE CASCADE,
    task_name VARCHAR(100) NOT NULL,
    is_active BOOLEAN DEFAULT TRUE
);

-- 4. Employee Shift Transaction Logs Matrix
CREATE TABLE employee_time_logs (
    log_id SERIAL PRIMARY KEY,
    employee_id VARCHAR(50) REFERENCES employees(employee_id) ON DELETE CASCADE,
    task_id INT REFERENCES tasks(task_id) ON DELETE CASCADE,
    clock_in_time TIMESTAMP NOT NULL,
    clock_out_time TIMESTAMP DEFAULT NULL
);

-- Index optimizations for lightning-fast polling logs
CREATE INDEX idx_active_sessions ON employee_time_logs(employee_id) WHERE clock_out_time IS NULL;
CREATE INDEX idx_tasks_project ON tasks(project_id);

-- Populate Core Architecture Baseline Seeds (Matching Code Fallbacks)
INSERT INTO employees (employee_id, employee_name, is_manager) VALUES 
('MGR01', 'Default Systems Manager Asset', TRUE),
('EMP01', 'Standard Corporate Staff Member', FALSE);

INSERT INTO projects (project_number, project_name) VALUES
('PRJ_001', 'Enterprise Cloud Sync Architecture'),
('PRJ_002', 'Next-Gen Mobile UI/UX Overhaul'),
('PRJ_003', 'Database Optimization & Hardening Cluster');

INSERT INTO tasks (project_id, task_name) VALUES
(1, 'API Endpoint Load Testing'),
(1, 'Kubernetes Core Config Optimization'),
(2, 'Figma Asset Variable Matrix Extraction'),
(2, 'SwiftUI Core State Management Layout'),
(3, 'PostgreSQL Vacuum Routine Parameter Tuning');

