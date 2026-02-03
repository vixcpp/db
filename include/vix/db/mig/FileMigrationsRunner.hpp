/**
 *
 *  @file FileMigrationsRunner.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/vixcpp/vix
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 */
#ifndef VIX_DB_FILE_MIGRATIONS_RUNNER_HPP
#define VIX_DB_FILE_MIGRATIONS_RUNNER_HPP

#include <vix/db/core/Drivers.hpp>

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>

namespace vix::db
{
  /**
   * @brief Represents a matched migration pair on disk.
   *
   * A migration is identified by a base identifier and may have:
   * - an "up" SQL script (required)
   * - a "down" SQL script (optional, required for rollback)
   *
   * The checksum is computed from the contents of the "up" script
   * and stored to detect changes after application.
   */
  struct MigrationPair
  {
    /// Base migration identifier (without .up.sql / .down.sql)
    std::string id;

    /// Path to the up migration script
    std::filesystem::path up_path;

    /// Path to the down migration script (may be empty if missing)
    std::filesystem::path down_path;

    /// SHA-256 checksum of the up migration contents
    std::string up_checksum;
  };

  /**
   * @brief Execute file-based SQL migrations against a database.
   *
   * FileMigrationsRunner applies and rolls back SQL migrations stored
   * on disk. It tracks applied migrations in a dedicated database table
   * and ensures migrations are executed in a deterministic order.
   *
   * Migration files are expected to follow a naming convention such as:
   * - <id>.up.sql
   * - <id>.down.sql
   *
   * Where <id> typically starts with a sortable prefix (e.g. timestamp).
   */
  class FileMigrationsRunner
  {
    Connection &conn_;
    std::filesystem::path dir_;
    std::string table_ = "schema_migrations";

  public:
    /**
     * @brief Construct a migrations runner.
     *
     * @param c            Database connection to use.
     * @param migrationsDir Directory containing migration files.
     */
    FileMigrationsRunner(Connection &c, std::filesystem::path migrationsDir)
        : conn_(c), dir_(std::move(migrationsDir)) {}

    /**
     * @brief Override the migrations tracking table name.
     *
     * Defaults to "schema_migrations".
     *
     * @param t Table name.
     */
    void setTable(std::string t) { table_ = std::move(t); }

    /**
     * @brief Apply all pending up migrations.
     *
     * Migrations are applied in ascending order based on their identifier.
     * Already-applied migrations are skipped.
     */
    void applyAll();

    /**
     * @brief Roll back the last applied migrations.
     *
     * Requires corresponding down migration scripts to be present.
     *
     * @param steps Number of migrations to roll back.
     */
    void rollback(int steps);

  private:
    /**
     * @brief Ensure the migrations tracking table exists.
     */
    void ensureTable();

    /**
     * @brief Scan the migrations directory and build migration pairs.
     *
     * @return List of discovered migration pairs.
     */
    std::vector<MigrationPair> scanPairs() const;

    /**
     * @brief Check whether a migration has already been applied.
     *
     * @param id            Migration identifier.
     * @param checksum_out  Optional pointer to receive the stored checksum.
     * @return true if the migration is marked as applied.
     */
    bool isApplied(const std::string &id, std::string *checksum_out = nullptr);

    /**
     * @brief Mark a migration as applied.
     *
     * @param id       Migration identifier.
     * @param checksum Checksum of the applied up script.
     */
    void markApplied(const std::string &id, const std::string &checksum);

    /**
     * @brief Remove the applied mark for a migration.
     *
     * @param id Migration identifier.
     */
    void unmarkApplied(const std::string &id);

    /**
     * @brief Retrieve the identifier of the last applied migration.
     *
     * Typically ordered by identifier descending.
     *
     * @return Last applied migration identifier.
     */
    std::string lastAppliedId();

    /**
     * @brief Read an entire file into a string.
     *
     * @param p File path.
     * @return File contents.
     */
    static std::string readFileText(const std::filesystem::path &p);

    /**
     * @brief Trim leading and trailing whitespace.
     *
     * @param s Input string.
     * @return Trimmed string.
     */
    static std::string trim(std::string s);

    /**
     * @brief Split a SQL script into individual statements.
     *
     * @param sql SQL script.
     * @return List of SQL statements.
     */
    static std::vector<std::string> splitStatements(const std::string &sql);

    /**
     * @brief Execute a multi-statement SQL script.
     *
     * @param sql SQL script.
     */
    void execScript(const std::string &sql);
  };

} // namespace vix::db

#endif // VIX_DB_FILE_MIGRATIONS_RUNNER_HPP
