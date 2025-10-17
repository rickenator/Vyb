; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %p = alloca ptr, align 8
  %a = alloca i64, align 8
  store i64 0, ptr %a, align 4
  %a1 = load i64, ptr %a, align 4
  %fromint.ptr = inttoptr i64 %a1 to ptr
  store ptr %fromint.ptr, ptr %p, align 8
  ret i64 0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
