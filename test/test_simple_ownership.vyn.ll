; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [25 x i8] c"Vec created successfully\00", align 1
@1 = private unnamed_addr constant [28 x i8] c"Test completed successfully\00", align 1

define i64 @test_simple_vec() {
entry:
  %v1 = alloca { ptr, i64, i64 }, align 8
  %vec.new = alloca { ptr, i64, i64 }, align 8
  %vec.data = call ptr @malloc(i64 40)
  %0 = call ptr @memset(ptr %vec.data, i32 0, i64 40)
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0
  store ptr %vec.data, ptr %vec.ptr_field, align 8
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1
  store i64 5, ptr %vec.size_field, align 4
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2
  store i64 5, ptr %vec.cap_field, align 4
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8
  store { ptr, i64, i64 } %vec.new.value, ptr %v1, align 8
  call void @__vyn_println(ptr @0)
  ret i64 42
}

declare ptr @malloc(i64)

declare ptr @memset(ptr, i32, i64)

declare void @__vyn_println(ptr)

define i64 @main() {
entry:
  %result = alloca i64, align 8
  %calltmp = call i64 @test_simple_vec()
  store i64 %calltmp, ptr %result, align 4
  call void @__vyn_println(ptr @1)
  %result1 = load i64, ptr %result, align 4
  ret i64 %result1
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
