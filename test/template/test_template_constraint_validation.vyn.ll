; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %test = alloca i64, align 8
  store i64 42, ptr %test, align 4
  %test1 = load i64, ptr %test, align 4
  ret i64 %test1
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
