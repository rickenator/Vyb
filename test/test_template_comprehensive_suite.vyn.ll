; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [6 x i8] c"hello\00", align 1

define i64 @main() {
entry:
  %bool_test = alloca i1, align 1
  %string_test = alloca ptr, align 8
  %float_test = alloca double, align 8
  %int_test = alloca i64, align 8
  store i64 42, ptr %int_test, align 4
  store double 3.140000e+00, ptr %float_test, align 8
  store ptr @0, ptr %string_test, align 8
  store i1 true, ptr %bool_test, align 1
  %int_test1 = load i64, ptr %int_test, align 4
  ret i64 %int_test1
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
