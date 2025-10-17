; ModuleID = 'VynModule'
source_filename = "VynModule"

@x = private global i64 42

define void @main() {
entry:
  ret void
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
