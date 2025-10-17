; ModuleID = 'VynModule'
source_filename = "VynModule"

@x = private global i64 42
@.str = private constant [6 x i8] c"hello\00"
@y = private global ptr @.str
@0 = private unnamed_addr constant [8 x i8] c"Hello, \00", align 1
@1 = private unnamed_addr constant [6 x i8] c"World\00", align 1

define i64 @add(i64 %a, i64 %b) {
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

define ptr @greet(ptr %name) {
entry:
  %name1 = alloca ptr, align 8
  store ptr %name, ptr %name1, align 8
  %name2 = load ptr, ptr %name1, align 8
  %strcattmp = call ptr @__vyn_string_concat(ptr @0, ptr %name2)
  ret ptr %strcattmp
}

declare ptr @__vyn_string_concat(ptr, ptr)

define void @main() {
entry:
  %message = alloca ptr, align 8
  %calltmp = call ptr @greet(ptr @1)
  store ptr %calltmp, ptr %message, align 8
  ret void
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
