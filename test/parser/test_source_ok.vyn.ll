; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %p = alloca ptr, align 8
  %addr = alloca i64, align 8
  store i64 0, ptr %addr, align 4
  store ptr null, ptr %p, align 8
  %addr1 = load i64, ptr %addr, align 4
  %fromint.ptr = inttoptr i64 %addr1 to ptr
  store ptr %fromint.ptr, ptr %p, align 8
  ret i64 0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
