; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [24 x i8] c"Created two Vec objects\00", align 1
@1 = private unnamed_addr constant [29 x i8] c"Testing multiple Vec objects\00", align 1
@2 = private unnamed_addr constant [28 x i8] c"Multiple Vec test completed\00", align 1

define i64 @test_multiple_vecs() {
entry:
  %v2 = alloca { ptr, i64, i64 }, align 8
  %v1 = alloca { ptr, i64, i64 }, align 8
  %vec.new = alloca { ptr, i64, i64 }, align 8
  %vec.data = call ptr @malloc(i64 24)
  %0 = call ptr @memset(ptr %vec.data, i32 0, i64 24)
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0
  store ptr %vec.data, ptr %vec.ptr_field, align 8
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1
  store i64 3, ptr %vec.size_field, align 4
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2
  store i64 3, ptr %vec.cap_field, align 4
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8
  store { ptr, i64, i64 } %vec.new.value, ptr %v1, align 8
  %vec.new1 = alloca { ptr, i64, i64 }, align 8
  %vec.data2 = call ptr @malloc.1(i64 16)
  %1 = call ptr @memset.2(ptr %vec.data2, i32 0, i64 16)
  %vec.ptr_field3 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 0
  store ptr %vec.data2, ptr %vec.ptr_field3, align 8
  %vec.size_field4 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 1
  store i64 2, ptr %vec.size_field4, align 4
  %vec.cap_field5 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 2
  store i64 2, ptr %vec.cap_field5, align 4
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
  ret i64 99
}

declare ptr @malloc(i64)

declare ptr @memset(ptr, i32, i64)

declare ptr @malloc.1(i64)

declare ptr @memset.2(ptr, i32, i64)

declare void @__vyn_println(ptr)

declare void @free(ptr)

define i64 @main() {
entry:
  %result = alloca i64, align 8
  call void @__vyn_println(ptr @1)
  %calltmp = call i64 @test_multiple_vecs()
  store i64 %calltmp, ptr %result, align 4
  call void @__vyn_println(ptr @2)
  %result1 = load i64, ptr %result, align 4
  ret i64 %result1
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
