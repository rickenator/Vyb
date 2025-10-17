; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  ret i64 0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
