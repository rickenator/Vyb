; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [37 x i8] c"Empty Vec created and element pushed\00", align 1
@1 = private unnamed_addr constant [37 x i8] c"Preallocated Vec created with size 5\00", align 1
@2 = private unnamed_addr constant [37 x i8] c"First element verified (should be 0)\00", align 1
@3 = private unnamed_addr constant [29 x i8] c"Different sized Vecs created\00", align 1
@4 = private unnamed_addr constant [35 x i8] c"Element pushed to preallocated Vec\00", align 1

define i64 @main() {
entry:
  %large_vec = alloca { ptr, i64, i64 }, align 8
  %small_vec = alloca { ptr, i64, i64 }, align 8
  %first_element = alloca i64, align 8
  %preallocated_vec = alloca { ptr, i64, i64 }, align 8
  %empty_vec = alloca { ptr, i64, i64 }, align 8
  %vec.new = alloca { ptr, i64, i64 }, align 8
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0
  store ptr null, ptr %vec.ptr_field, align 8
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1
  store i64 0, ptr %vec.size_field, align 4
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2
  store i64 0, ptr %vec.cap_field, align 4
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8
  store { ptr, i64, i64 } %vec.new.value, ptr %empty_vec, align 8
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %empty_vec, i32 0, i32 1
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4
  %vec.new_size = add i64 %vec.current_size, 1
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4
  call void @__vyn_println(ptr @0)
  %vec.new1 = alloca { ptr, i64, i64 }, align 8
  %vec.data = call ptr @malloc(i64 40)
  %0 = call ptr @memset(ptr %vec.data, i32 0, i64 40)
  %vec.ptr_field2 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 0
  store ptr %vec.data, ptr %vec.ptr_field2, align 8
  %vec.size_field3 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 1
  store i64 5, ptr %vec.size_field3, align 4
  %vec.cap_field4 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 2
  store i64 5, ptr %vec.cap_field4, align 4
  %vec.new.value5 = load { ptr, i64, i64 }, ptr %vec.new1, align 8
  store { ptr, i64, i64 } %vec.new.value5, ptr %preallocated_vec, align 8
  call void @__vyn_println(ptr @1)
  store i64 42, ptr %first_element, align 4
  call void @__vyn_println(ptr @2)
  %vec.new6 = alloca { ptr, i64, i64 }, align 8
  %vec.data7 = call ptr @malloc.1(i64 16)
  %1 = call ptr @memset.2(ptr %vec.data7, i32 0, i64 16)
  %vec.ptr_field8 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new6, i32 0, i32 0
  store ptr %vec.data7, ptr %vec.ptr_field8, align 8
  %vec.size_field9 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new6, i32 0, i32 1
  store i64 2, ptr %vec.size_field9, align 4
  %vec.cap_field10 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new6, i32 0, i32 2
  store i64 2, ptr %vec.cap_field10, align 4
  %vec.new.value11 = load { ptr, i64, i64 }, ptr %vec.new6, align 8
  store { ptr, i64, i64 } %vec.new.value11, ptr %small_vec, align 8
  %vec.new12 = alloca { ptr, i64, i64 }, align 8
  %vec.data13 = call ptr @malloc.3(i64 80)
  %2 = call ptr @memset.4(ptr %vec.data13, i32 0, i64 80)
  %vec.ptr_field14 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new12, i32 0, i32 0
  store ptr %vec.data13, ptr %vec.ptr_field14, align 8
  %vec.size_field15 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new12, i32 0, i32 1
  store i64 10, ptr %vec.size_field15, align 4
  %vec.cap_field16 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new12, i32 0, i32 2
  store i64 10, ptr %vec.cap_field16, align 4
  %vec.new.value17 = load { ptr, i64, i64 }, ptr %vec.new12, align 8
  store { ptr, i64, i64 } %vec.new.value17, ptr %large_vec, align 8
  call void @__vyn_println(ptr @3)
  %vec.size_ptr18 = getelementptr inbounds { ptr, i64, i64 }, ptr %preallocated_vec, i32 0, i32 1
  %vec.current_size19 = load i64, ptr %vec.size_ptr18, align 4
  %vec.new_size20 = add i64 %vec.current_size19, 1
  store i64 %vec.new_size20, ptr %vec.size_ptr18, align 4
  call void @__vyn_println(ptr @4)
  ret i64 0
}

declare void @__vyn_println(ptr)

declare ptr @malloc(i64)

declare ptr @memset(ptr, i32, i64)

declare ptr @malloc.1(i64)

declare ptr @memset.2(ptr, i32, i64)

declare ptr @malloc.3(i64)

declare ptr @memset.4(ptr, i32, i64)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
