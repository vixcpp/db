#  @file CMakeLists.txt
#  @author Gaspard Kirira
#
#  Copyright 2025, Gaspard Kirira.  All rights reserved.
#  https://github.com/vixcpp/vix
#  Use of this source code is governed by a MIT license
#  that can be found in the License file.
#
#  Vix.cpp | DB module (core anti-ORM)
cmake_minimum_required(VERSION 3.20)

project(vix_db VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

# ------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------
option(VIX_DB_BUILD_TESTS       "Build unit tests for Vix DB"                 OFF)
option(VIX_DB_BUILD_EXAMPLES    "Build examples for Vix DB"                  OFF)
option(VIX_DB_BUILD_TOOLS       "Build DB CLI tools (migrator)"              OFF)

# SQL engines (drivers)
option(VIX_DB_USE_MYSQL         "Enable MySQL Connector/C++ driver"          ON)
option(VIX_DB_REQUIRE_MYSQL     "Fail if MySQL requested but not found"      OFF)

option(VIX_DB_USE_SQLITE        "Enable SQLite3 driver"                      OFF)
option(VIX_DB_REQUIRE_SQLITE    "Fail if SQLite requested but not found"     OFF)

# Future (not enabled by default)
option(VIX_DB_USE_POSTGRES      "Enable PostgreSQL driver (future)"          OFF)
option(VIX_DB_REQUIRE_POSTGRES  "Fail if Postgres requested but not found"   OFF)

# Redis is not SQL, but useful for caching / sessions / queue (future)
option(VIX_DB_USE_REDIS         "Enable Redis client (future)"               OFF)
option(VIX_DB_REQUIRE_REDIS     "Fail if Redis requested but not found"      OFF)

# ------------------------------------------------------------------------------
# Language / PIC
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ------------------------------------------------------------------------------
# Warnings / Sanitizers
# ------------------------------------------------------------------------------
set(_WARNINGS_GNU
  -Wall -Wextra -Wshadow -Wconversion
  -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wpedantic
)
set(_WARNINGS_MSVC /W4 /permissive-)

# ------------------------------------------------------------------------------
# Optional deps
# ------------------------------------------------------------------------------
find_package(spdlog QUIET CONFIG)

# ------------------------------------------------------------------------------
# Feature flags (computed)
# ------------------------------------------------------------------------------
set(VIX_DB_HAS_MYSQL    OFF)
set(VIX_DB_HAS_SQLITE   OFF)
set(VIX_DB_HAS_POSTGRES OFF)
set(VIX_DB_HAS_REDIS    OFF)

# ------------------------------------------------------------------------------
# MySQL detection (via alias)
#  - Source of truth: cmake/MySQLCppConnAlias.cmake defines MySQLCppConn::MySQLCppConn
# ------------------------------------------------------------------------------
if (VIX_DB_USE_MYSQL)
  include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/MySQLCppConnAlias.cmake)

  if (TARGET MySQLCppConn::MySQLCppConn)
    set(VIX_DB_HAS_MYSQL ON)
    message(STATUS "[vix_db] mysql: enabled (alias)")
  else()
    if (VIX_DB_REQUIRE_MYSQL)
      message(FATAL_ERROR "[vix_db] mysql requested but Connector/C++ not found (VIX_DB_REQUIRE_MYSQL=ON).")
    endif()
    message(STATUS "[vix_db] mysql: disabled (connector not found)")
    set(VIX_DB_USE_MYSQL OFF)
  endif()
endif()

# ------------------------------------------------------------------------------
# SQLite detection (soft)
# ------------------------------------------------------------------------------
set(_VIX_SQLITE_TARGET "")

if (VIX_DB_USE_SQLITE)
  find_package(SQLite3 QUIET)

  if (TARGET SQLite::SQLite3)
    set(_VIX_SQLITE_TARGET SQLite::SQLite3)
  elseif (TARGET SQLite3::SQLite3)
    add_library(SQLite::SQLite3 ALIAS SQLite3::SQLite3)
    set(_VIX_SQLITE_TARGET SQLite::SQLite3)
  endif()

  if (SQLite3_FOUND OR _VIX_SQLITE_TARGET)
    set(VIX_DB_HAS_SQLITE ON)
    message(STATUS "[vix_db] sqlite: enabled")
  else()
    if (VIX_DB_REQUIRE_SQLITE)
      message(FATAL_ERROR "[vix_db] sqlite requested but not found (VIX_DB_REQUIRE_SQLITE=ON).")
    endif()
    message(STATUS "[vix_db] sqlite: disabled (not found)")
    set(VIX_DB_USE_SQLITE OFF)
  endif()
endif()

# ------------------------------------------------------------------------------
# PostgreSQL detection (future placeholder)
# ------------------------------------------------------------------------------
set(_VIX_PQ_TARGET "")
if (VIX_DB_USE_POSTGRES)
  # Try CMake module first (may define PostgreSQL::PostgreSQL in some setups)
  find_package(PostgreSQL QUIET)

  # Generic fallback to libpq
  if (PostgreSQL_FOUND)
    # PostgreSQL CMake module typically exports variables, not a target. Create an alias target.
    if (NOT TARGET PostgreSQL::libpq)
      add_library(PostgreSQL::libpq INTERFACE IMPORTED)
      set_target_properties(PostgreSQL::libpq PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PostgreSQL_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES      "${PostgreSQL_LIBRARIES}"
      )
    endif()
    set(_VIX_PQ_TARGET PostgreSQL::libpq)
  else()
    # fallback manual find (libpq)
    find_library(_VIX_PQ_LIB NAMES pq libpq)
    find_path(_VIX_PQ_INC NAMES libpq-fe.h)

    if (_VIX_PQ_LIB AND _VIX_PQ_INC)
      add_library(PostgreSQL::libpq INTERFACE IMPORTED)
      set_target_properties(PostgreSQL::libpq PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${_VIX_PQ_INC}"
        INTERFACE_LINK_LIBRARIES      "${_VIX_PQ_LIB}"
      )
      set(_VIX_PQ_TARGET PostgreSQL::libpq)
    endif()
  endif()

  if (_VIX_PQ_TARGET)
    set(VIX_DB_HAS_POSTGRES ON)
    message(STATUS "[vix_db] postgres: enabled (placeholder target=${_VIX_PQ_TARGET})")
  else()
    if (VIX_DB_REQUIRE_POSTGRES)
      message(FATAL_ERROR "[vix_db] postgres requested but libpq not found (VIX_DB_REQUIRE_POSTGRES=ON).")
    endif()
    message(STATUS "[vix_db] postgres: disabled (not found)")
    set(VIX_DB_USE_POSTGRES OFF)
  endif()
endif()

# Redis detection (future placeholder)
set(_VIX_REDIS_TARGET "")
if (VIX_DB_USE_REDIS)
  # Try redis-plus-plus first (CMake package commonly provides redis++::redis++)
  find_package(redis++ QUIET CONFIG)
  if (TARGET redis++::redis++)
    set(_VIX_REDIS_TARGET redis++::redis++)
  else()
    # fallback hiredis (often exports hiredis::hiredis via config, but not always)
    find_package(hiredis QUIET CONFIG)
    if (TARGET hiredis::hiredis)
      set(_VIX_REDIS_TARGET hiredis::hiredis)
    else()
      find_library(_VIX_HIREDIS_LIB NAMES hiredis)
      find_path(_VIX_HIREDIS_INC NAMES hiredis/hiredis.h)
      if (_VIX_HIREDIS_LIB AND _VIX_HIREDIS_INC)
        add_library(hiredis::hiredis INTERFACE IMPORTED)
        set_target_properties(hiredis::hiredis PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES "${_VIX_HIREDIS_INC}"
          INTERFACE_LINK_LIBRARIES      "${_VIX_HIREDIS_LIB}"
        )
        set(_VIX_REDIS_TARGET hiredis::hiredis)
      endif()
    endif()
  endif()

  if (_VIX_REDIS_TARGET)
    set(VIX_DB_HAS_REDIS ON)
    message(STATUS "[vix_db] redis: enabled (placeholder target=${_VIX_REDIS_TARGET})")
  else()
    if (VIX_DB_REQUIRE_REDIS)
      message(FATAL_ERROR "[vix_db] redis requested but client not found (VIX_DB_REQUIRE_REDIS=ON).")
    endif()
    message(STATUS "[vix_db] redis: disabled (not found)")
    set(VIX_DB_USE_REDIS OFF)
  endif()
endif()

# ------------------------------------------------------------------------------
# Sources / Headers
# ------------------------------------------------------------------------------
set(VIX_DB_PUBLIC_HEADERS
  include/vix/db/db.hpp
  include/vix/db/Errors.hpp
  include/vix/db/Value.hpp
  include/vix/db/Drivers.hpp
  include/vix/db/ConnectionPool.hpp
  include/vix/db/Transaction.hpp
  include/vix/db/Database.hpp
  include/vix/db/Migration.hpp
  include/vix/db/MigrationsRunner.hpp
  include/vix/db/FileMigrationsRunner.hpp
  include/vix/db/Sha256.hpp
)

set(VIX_DB_SOURCES
  src/ConnectionPool.cpp
  src/Transaction.cpp
  src/Database.cpp
  src/MigrationsRunner.cpp
  src/FileMigrationsRunner.cpp
  src/Sha256.cpp
)

# MySQL driver sources
if (VIX_DB_HAS_MYSQL)
  list(APPEND VIX_DB_PUBLIC_HEADERS include/vix/db/mysql/MySQLDriver.hpp)
  list(APPEND VIX_DB_SOURCES        src/mysql/MySQLDriver.cpp)
endif()

# SQLite driver sources (future: create include/vix/db/sqlite/SQLiteDriver.hpp + src/sqlite/SQLiteDriver.cpp)
if (VIX_DB_HAS_SQLITE)
  list(APPEND VIX_DB_PUBLIC_HEADERS include/vix/db/sqlite/SQLiteDriver.hpp)
  list(APPEND VIX_DB_SOURCES        src/sqlite/SQLiteDriver.cpp)
endif()

# Postgres driver sources (future placeholders)
if (VIX_DB_HAS_POSTGRES)
  list(APPEND VIX_DB_PUBLIC_HEADERS include/vix/db/postgres/PostgresDriver.hpp)
  list(APPEND VIX_DB_SOURCES        src/postgres/PostgresDriver.cpp)
endif()

# Redis client sources (future placeholders)
if (VIX_DB_HAS_REDIS)
  list(APPEND VIX_DB_PUBLIC_HEADERS include/vix/db/redis/RedisClient.hpp)
  list(APPEND VIX_DB_SOURCES        src/redis/RedisClient.cpp)
endif()

# ------------------------------------------------------------------------------
# If building DB standalone (not from umbrella), bring core as subdir
# ------------------------------------------------------------------------------
if (NOT TARGET vix::core)
  add_subdirectory(../core core_build)
endif()

# ------------------------------------------------------------------------------
# Library target
# ------------------------------------------------------------------------------
add_library(vix_db STATIC ${VIX_DB_SOURCES} ${VIX_DB_PUBLIC_HEADERS})
add_library(vix::db ALIAS vix_db)

# Keep feature macros consistent everywhere (build + consumers)
target_compile_definitions(vix_db PUBLIC
  VIX_DB_HAS_MYSQL=$<BOOL:${VIX_DB_HAS_MYSQL}>
  VIX_DB_HAS_SQLITE=$<BOOL:${VIX_DB_HAS_SQLITE}>
  VIX_DB_HAS_POSTGRES=$<BOOL:${VIX_DB_HAS_POSTGRES}>
  VIX_DB_HAS_REDIS=$<BOOL:${VIX_DB_HAS_REDIS}>
)

# export name => installed imported target is vix::db
set_target_properties(vix_db PROPERTIES EXPORT_NAME db)

target_include_directories(vix_db PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# DB depends on Vix Core (Config.hpp, util, etc.)
target_link_libraries(vix_db PUBLIC vix::core)

# Inherit umbrella sanitizers if present
if (VIX_ENABLE_SANITIZERS AND TARGET vix_sanitizers)
  target_link_libraries(vix_db PUBLIC vix_sanitizers)
endif()

if (MSVC)
  target_compile_options(vix_db PRIVATE ${_WARNINGS_MSVC})
else()
  target_compile_options(vix_db PRIVATE ${_WARNINGS_GNU})
endif()

# deps / defines
if (spdlog_FOUND)
  if (TARGET spdlog::spdlog_header_only)
    target_link_libraries(vix_db PUBLIC spdlog::spdlog_header_only)
  elseif (TARGET spdlog::spdlog)
    target_link_libraries(vix_db PUBLIC spdlog::spdlog)
  endif()
  target_compile_definitions(vix_db PUBLIC VIX_DB_HAS_SPDLOG=1)
else()
  target_compile_definitions(vix_db PUBLIC VIX_DB_NO_LOGGER=1)
endif()

# ------------------------------------------------------------------------------
# Link drivers (single source of truth)
# ------------------------------------------------------------------------------
if (VIX_DB_HAS_MYSQL)
  target_link_libraries(vix_db PUBLIC MySQLCppConn::MySQLCppConn)
endif()

if (VIX_DB_HAS_SQLITE)
  if (_VIX_SQLITE_TARGET)
    target_link_libraries(vix_db PUBLIC ${_VIX_SQLITE_TARGET})
  else()
    message(WARNING "[vix_db] sqlite enabled but no target found; linkage skipped.")
  endif()
endif()

if (VIX_DB_HAS_POSTGRES)
  if (_VIX_PQ_TARGET)
    target_link_libraries(vix_db PUBLIC ${_VIX_PQ_TARGET})
  else()
    message(WARNING "[vix_db] postgres enabled but no libpq target found; linkage skipped.")
  endif()
endif()

if (VIX_DB_HAS_REDIS)
  if (_VIX_REDIS_TARGET)
    target_link_libraries(vix_db PUBLIC ${_VIX_REDIS_TARGET})
  else()
    message(WARNING "[vix_db] redis enabled but no client target found; linkage skipped.")
  endif()
endif()

# ------------------------------------------------------------------------------
# Install / export via umbrella export-set "VixTargets"
# ------------------------------------------------------------------------------
if (DEFINED VIX_UMBRELLA_BUILD)
  install(TARGETS vix_db
    EXPORT  VixTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
else()
  install(TARGETS vix_db
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
endif()

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

message(STATUS "------------------------------------------------------")
message(STATUS "[vix_db] configured (${PROJECT_VERSION})")
message(STATUS "[vix_db] mysql:    requested=${VIX_DB_USE_MYSQL} available=${VIX_DB_HAS_MYSQL}")
message(STATUS "[vix_db] sqlite:   requested=${VIX_DB_USE_SQLITE} available=${VIX_DB_HAS_SQLITE}")
message(STATUS "[vix_db] postgres: requested=${VIX_DB_USE_POSTGRES} available=${VIX_DB_HAS_POSTGRES}")
message(STATUS "[vix_db] redis:    requested=${VIX_DB_USE_REDIS} available=${VIX_DB_HAS_REDIS}")
message(STATUS "------------------------------------------------------")
