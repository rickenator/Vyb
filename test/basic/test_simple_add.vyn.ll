; ModuleID = 'VynModule'
source_filename = "VynModule"

define i32 @main() {
entry:
  %result = alloca i32, align 4
  %b = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 10, ptr %a, align 4
  store i32 20, ptr %b, align 4
  %a1 = load i32, ptr %a, align 4
  %b2 = load i32, ptr %b, align 4
  %addtmp = add i32 %a1, %b2
  store i32 %addtmp, ptr %result, align 4
  %result3 = load i32, ptr %result, align 4
  ret i32 %result3
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
