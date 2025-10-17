; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @add_shorthand(i64 %a, i64 %b) {
entry:
  %b2 = alloca i64, align 8
  %a1 = alloca i64, align 8
  store i64 %a, ptr %a1, align 4
  store i64 %b, ptr %b2, align 4
  %a3 = load i64, ptr %a1, align 4
  %b4 = load i64, ptr %b2, align 4
  %addtmp = add i64 %a3, %b4
  ret i64 %addtmp
}

define i64 @main() {
entry:
  %result = alloca i64, align 8
  %calltmp = call i64 @add_shorthand(i64 10, i64 20)
  store i64 %calltmp, ptr %result, align 4
  %result1 = load i64, ptr %result, align 4
  ret i64 %result1
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
