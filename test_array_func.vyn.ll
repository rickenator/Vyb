; ModuleID = 'VynModule'
source_filename = "VynModule"

@type_name = private unnamed_addr constant [6 x i8] c"[Int]\00", align 1

define [3 x i64] @test_array() {
entry:
  %arr = alloca [3 x i64], align 8
  store [3 x i64] [i64 10, i64 20, i64 30], ptr %arr, align 4
  %arr1 = load [3 x i64], ptr %arr, align 4
  ret [3 x i64] %arr1
}

define i64 @main() {
entry:
  %result = alloca [3 x i64], align 8
  %calltmp = call [3 x i64] @test_array()
  store [3 x i64] %calltmp, ptr %result, align 4
  %result1 = load [3 x i64], ptr %result, align 4
  %serialize_temp = alloca [3 x i64], align 8
  store [3 x i64] %result1, ptr %serialize_temp, align 4
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name)
  call void @__vyn_println(ptr %serialized_json)
  ret i64 0
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_convert_lit_string(ptr)
