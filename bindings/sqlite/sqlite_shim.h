#pragma once
/* Curated SQLite3 FFI surface - SDK Phase 3 (#174). Small, declarable subset
   of sqlite3.h (avoids bindgen-ing the full ~100KB header). The opaque
   `sqlite3*` handle is modeled as `void*` (ABI-identical; keeps the Vyb FFI
   types trivial). */
typedef void (*sqlite3_callback)(void*, int, char**, char**);
int sqlite3_open(const char* filename, void** ppDb);
int sqlite3_close(void* db);
int sqlite3_exec(void* db, const char* sql, sqlite3_callback cb, void* arg, char** errmsg);
const char* sqlite3_errmsg(void* db);
/* Prepared-statement reads (added for the all-Vyb `vyb-traffic` rewrite):
   prepare -> step to SQLITE_ROW (100) / SQLITE_DONE (101) -> read columns by
   index -> finalize. Text columns come back as NUL-terminated C strings; the
   wrapper measures them with strlen and copies them into Vyb Strings. */
int sqlite3_prepare_v2(void* db, const char* sql, int nByte, void** ppStmt, const char** pzTail);
int sqlite3_step(void* stmt);
int sqlite3_finalize(void* stmt);
int sqlite3_column_count(void* stmt);
const char* sqlite3_column_text(void* stmt, int iCol);
const char* sqlite3_column_name(void* stmt, int iCol);
int sqlite3_changes(void* db);
size_t strlen(const char* s);
