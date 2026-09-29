-- Clean up existing logs to ensure a fresh test environment
TRUNCATE employee_time_logs, tasks, projects, employees CASCADE;

-- 1. Insert Employees with base configurations matching the document
INSERT INTO employees (employee_id, employee_name, is_manager) VALUES 
('MGR01', 'Default Systems Manager Asset', TRUE),
('ENG01', 'D A M S K Hemasiri', FALSE), -- Lead Engineer from the .doc form
('ENG02', 'Saman Perera', FALSE);

-- 2. Insert the EV Fast Charging Technology Project
INSERT INTO projects (project_id, project_number, project_name) VALUES 
(1, 'EV-FCT-2026', 'Development of EV Fast Charging Technology');

-- 3. Insert specific tasks matching the exact NERDC Form QSR/R&D/03 A Milestones
INSERT INTO tasks (task_id, project_id, task_name, is_active) VALUES 
(1, 1, 'Literature survey relevant to foreign EV charger modules kit', TRUE),
(2, 1, 'Import of EV charger modules kit', TRUE),
(3, 1, 'Module based system design including power converter and charging architecture', TRUE),
(4, 1, 'Hardware assembly and testing', TRUE),
(5, 1, 'Implementation of charging control and communication protocols', TRUE),
(6, 1, 'Efficiency, safety, and thermal testing', TRUE),
(7, 1, 'Documentation, publication', TRUE);

-- 4. Insert historical time logs to generate realistic cost evaluations
-- We model 8-hour working days to cleanly align with the 8,170 LKR/Day structural frame

-- Day 1: Literature Survey work by Hemasiri
INSERT INTO employee_time_logs (employee_id, task_id, clock_in_time, clock_out_time) VALUES 
('ENG01', 1, '2026-02-02 08:00:00', '2026-02-02 16:00:00');

-- Day 2: More Literature Survey work
INSERT INTO employee_time_logs (employee_id, task_id, clock_in_time, clock_out_time) VALUES 
('ENG01', 1, '2026-02-03 08:30:00', '2026-02-03 16:30:00');

-- Day 3: Module based system design logs
INSERT INTO employee_time_logs (employee_id, task_id, clock_in_time, clock_out_time) VALUES 
('ENG01', 3, '2026-02-04 08:00:00', '2026-02-04 16:00:00');

-- Day 4: Hardware Assembly & Testing logs
INSERT INTO employee_time_logs (employee_id, task_id, clock_in_time, clock_out_time) VALUES 
('ENG01', 4, '2026-02-05 09:00:00', '2026-02-05 17:00:00');

-- Day 5: Assisted task track by second engineer to aggregate metrics
INSERT INTO employee_time_logs (employee_id, task_id, clock_in_time, clock_out_time) VALUES 
('ENG02', 4, '2026-02-05 08:00:00', '2026-02-05 16:00:00');

-- Day 6: Ongoing active session (Clocked-In but not yet clocked-out)
INSERT INTO employee_time_logs (employee_id, task_id, clock_in_time, clock_out_time) VALUES 
('ENG01', 5, '2026-02-06 08:00:00', NULL);

