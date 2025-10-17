; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %result = alloca i64, align 8
  store i64 42, ptr %result, align 4
  %result1 = load i64, ptr %result, align 4
  ret i64 %result1
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
