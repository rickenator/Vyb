; ModuleID = 'VynModule'
source_filename = "VynModule"

@global_x = private global i64 100
@0 = private unnamed_addr constant [36 x i8] c"Created Vec<Int32> and Vec<Float32>\00", align 1
@1 = private unnamed_addr constant [20 x i8] c"Basic types working\00", align 1
@2 = private unnamed_addr constant [30 x i8] c"Testing all working features:\00", align 1
@3 = private unnamed_addr constant [30 x i8] c"Vec operations test completed\00", align 1
@4 = private unnamed_addr constant [27 x i8] c"Basic types test completed\00", align 1
@5 = private unnamed_addr constant [33 x i8] c"All tests completed successfully\00", align 1

define i64 @test_vec_operations() {
entry:
  %v2 = alloca { ptr, i64, i64 }, align 8
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
  %vec.new1 = alloca { ptr, i64, i64 }, align 8
  %vec.data2 = call ptr @malloc.1(i64 24)
  %1 = call ptr @memset.2(ptr %vec.data2, i32 0, i64 24)
  %vec.ptr_field3 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 0
  store ptr %vec.data2, ptr %vec.ptr_field3, align 8
  %vec.size_field4 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 1
  store i64 3, ptr %vec.size_field4, align 4
  %vec.cap_field5 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 2
  store i64 3, ptr %vec.cap_field5, align 4
  %vec.new.value6 = load { ptr, i64, i64 }, ptr %vec.new1, align 8
  store { ptr, i64, i64 } %vec.new.value6, ptr %v2, align 8
  call void @__vyn_println(ptr @0)
  %v2_cleanup_load = load { ptr, i64, i64 }, ptr %v2, align 8
  %v2_data_ptr = extractvalue { ptr, i64, i64 } %v2_cleanup_load, 0
  %v2_null_check = icmp ne ptr %v2_data_ptr, null
  br i1 %v2_null_check, label %v2_free_block, label %v2_continue

v2_free_block:                                    ; preds = %entry
  call void @free(ptr %v2_data_ptr)
  br label %v2_continue

v2_continue:                                      ; preds = %v2_free_block, %entry
  %v1_cleanup_load = load { ptr, i64, i64 }, ptr %v1, align 8
  %v1_data_ptr = extractvalue { ptr, i64, i64 } %v1_cleanup_load, 0
  %v1_null_check = icmp ne ptr %v1_data_ptr, null
  br i1 %v1_null_check, label %v1_free_block, label %v1_continue

v1_free_block:                                    ; preds = %v2_continue
  call void @free(ptr %v1_data_ptr)
  br label %v1_continue

v1_continue:                                      ; preds = %v1_free_block, %v2_continue
  ret i64 42
}

declare ptr @malloc(i64)

declare ptr @memset(ptr, i32, i64)

declare ptr @malloc.1(i64)

declare ptr @memset.2(ptr, i32, i64)

declare void @__vyn_println(ptr)

declare void @free(ptr)

define void @test_basic_types() {
entry:
  %i32 = alloca i32, align 4
  store i32 42, ptr %i32, align 4
  call void @__vyn_println(ptr @1)
  ret void
}

define i64 @main() {
entry:
  %vec_result = alloca i64, align 8
  call void @__vyn_println(ptr @2)
  %calltmp = call i64 @test_vec_operations()
  store i64 %calltmp, ptr %vec_result, align 4
  call void @__vyn_println(ptr @3)
  call void @test_basic_types()
  call void @__vyn_println(ptr @4)
  call void @__vyn_println(ptr @5)
  %vec_result1 = load i64, ptr %vec_result, align 4
  ret i64 %vec_result1
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
