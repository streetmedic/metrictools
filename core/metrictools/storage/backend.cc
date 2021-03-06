/**
 * This file is part of the "FnordMetric" project
 *   Copyright (c) 2014 Paul Asmuth, Google Inc.
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <metrictools/storage/backend.h>

#if ENABLE_MYSQL
#include <metrictools/storage/mysql/mysqlbackend.h>
#endif

#if ENABLE_SQLITE
#include <metrictools/storage/sqlite/sqlite_backend.h>
#endif

namespace fnordmetric {

ReturnCode Backend::openBackend(
    const URI& backend_uri,
    std::unique_ptr<Backend>* backend) {
  auto backend_type = backend_uri.scheme();

  if (backend_type == "sqlite") {
#if ENABLE_SQLITE
    return sqlite_backend::SQLiteBackend::connect(backend_uri, backend);
#else
    return ReturnCode::error("ERUNTIME", "compiled without SQLite support");
#endif
  }

  if (backend_type == "mysql") {
#if ENABLE_MYSQL
    return mysql_backend::MySQLBackend::connect(backend_uri, backend);
#else
    return ReturnCode::error("ERUNTIME", "compiled without MySQL support");
#endif
  }

  return ReturnCode::errorf("ERUNTIME", "invalid backend type: $0", backend_type);
}

} // namespace fnordmetric

