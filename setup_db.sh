#!/bin/bash

# Exit immediately if any command fails
set -e

echo "========================================================="
echo " Starting Automated Enterprise Database Configuration   "
echo "========================================================="

# 1. Install PostgreSQL server and client libraries if missing
echo "[1/6] Validating PostgreSQL System Package Installations..."
if ! command -v psql &> /dev/null; then
    echo "PostgreSQL not found. Installing via dnf..."
    sudo dnf install -y postgresql-server postgresql-contrib
    
    echo "Initializing PostgreSQL database cluster store..."
    sudo postgresql-setup --initdb
else
    echo "PostgreSQL packages are already present on the system."
fi

# 2. Boot up the service and set it to start automatically on system startup
echo "[2/6] Restructuring System Daemon Automation Status..."
sudo systemctl enable postgresql
sudo systemctl start postgresql

# 3. Securely provision the database user role with the required password
echo "[3/6] Generating Secure User Role Configuration (hr_admin)..."
sudo -u postgres psql -c "DO \$\$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_catalog.pg_user WHERE usename = 'hr_admin') THEN
        CREATE ROLE hr_admin WITH LOGIN PASSWORD '12345678' SUPERUSER;
    ELSE
        ALTER USER hr_admin WITH PASSWORD '12345678';
    END IF;
END \$\$;"

# 4. Safely initialize the environment log database instance
echo "[4/6] Instantiating Corporate Storage Cluster Workspace (hr_database)..."
sudo -u postgres psql -c "SELECT 1 FROM pg_database WHERE datname = 'hr_database';" | grep -q 1 || \
sudo -u postgres psql -c "CREATE DATABASE hr_database OWNER hr_admin;"

# 5. Populate structural target system tables using schema blueprints
echo "[5/6] Deploying Relational System Tables from schema.sql blueprint..."
if [ -f "schema.sql" ]; then
    # Set environment password safely so psql executes non-interactively
    export PGPASSWORD='12345678'
    psql -h 127.0.0.1 -U hr_admin -d hr_database -f schema.sql
    unset PGPASSWORD
    echo "Schema mapping schema.sql executed successfully!"
else
    echo "ERROR: schema.sql file not located in current working folder context path."
    exit 1
fi

echo "[6/6] Verifying Target Connection Matrix..."
export PGPASSWORD='12345678'
psql -h 127.0.0.1 -U hr_admin -d hr_database -c "\dt"
unset PGPASSWORD

echo "========================================================="
echo " SUCCESS: Enterprise DB Engine Live on Port 5432!       "
echo "========================================================="
