; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %arr = alloca [3 x i64], align 8
  store [3 x i64] [i64 1, i64 2, i64 3], ptr %arr, align 4
  ret i64 0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
