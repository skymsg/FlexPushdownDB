#
# Path variables for SQLite3
#

set(EP_BASE sqlite_ep)
set(EP_PREFIX ${DEPS_PREFIX}/${EP_BASE})
set(EP_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${EP_PREFIX})
set(EP_SOURCE_DIR ${EP_BASE_DIR}/src/${EP_BASE})
set(EP_BUILD_DIR ${EP_BASE_DIR}/src/${EP_BASE}-build)
set(EP_INSTALL_DIR ${EP_BASE_DIR}/install)
set(EP_BIN_DIR ${EP_INSTALL_DIR}/bin)
set(EP_INCLUDE_DIR ${EP_INSTALL_DIR}/include)
set(EP_LIB_DIR ${EP_INSTALL_DIR}/lib)

set(SQLITE_STATIC_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}sqlite3${CMAKE_STATIC_LIBRARY_SUFFIX})
set(SQLITE_STATIC_LIB ${EP_LIB_DIR}/${SQLITE_STATIC_LIB_NAME})

set(SQLITE_SHARED_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}sqlite3${CMAKE_SHARED_LIBRARY_SUFFIX})
set(SQLITE_SHARED_LIB ${EP_LIB_DIR}/${SQLITE_SHARED_LIB_NAME})

set(SQLITE_CSV_STATIC_LIB_NAME ${CMAKE_STATIC_LIBRARY_PREFIX}csv${CMAKE_STATIC_LIBRARY_SUFFIX})
set(SQLITE_CSV_STATIC_LIB ${EP_LIB_DIR}/${SQLITE_CSV_STATIC_LIB_NAME})

set(SQLITE_CSV_SHARED_LIB_NAME ${CMAKE_SHARED_LIBRARY_PREFIX}csv${CMAKE_SHARED_LIBRARY_SUFFIX})
set(SQLITE_CSV_SHARED_LIB ${EP_LIB_DIR}/${SQLITE_CSV_SHARED_LIB_NAME})