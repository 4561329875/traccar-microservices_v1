#pragma once
#include <pqxx/pqxx>
#include <memory>

class DatabaseConnection {
public:
    DatabaseConnection();
    std::shared_ptr<pqxx::connection> getConnection();

private:
    std::shared_ptr<pqxx::connection> connection;
};
