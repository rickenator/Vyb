; ModuleID = 'VynModule'
source_filename = "VynModule"

@array_string = private unnamed_addr constant [13 x i8] c"[10, 20, 30]\00", align 1
@array_string.1 = private unnamed_addr constant [5 x i8] c"[10]\00", align 1
@array_string.2 = private unnamed_addr constant [21 x i8] c"[10, 20, 30, 30, 40]\00", align 1

define i64 @main() {
entry:
  %arr3 = alloca [5 x i64], align 8
  %arr2 = alloca [1 x i64], align 8
  %arr1 = alloca [3 x i64], align 8
  store [3 x i64] [i64 10, i64 20, i64 30], ptr %arr1, align 4
  store [1 x i64] [i64 42], ptr %arr2, align 4
  store [5 x i64] [i64 1, i64 2, i64 3, i64 4, i64 5], ptr %arr3, align 4
  %element_ptr_0 = getelementptr [3 x i64], ptr %arr1, i32 0, i32 0
  %element_0 = load i64, ptr %element_ptr_0, align 4
  %element_ptr_1 = getelementptr [3 x i64], ptr %arr1, i32 0, i32 1
  %element_1 = load i64, ptr %element_ptr_1, align 4
  %element_ptr_2 = getelementptr [3 x i64], ptr %arr1, i32 0, i32 2
  %element_2 = load i64, ptr %element_ptr_2, align 4
  call void @__vyn_println(ptr @array_string)
  %element_ptr_01 = getelementptr [1 x i64], ptr %arr2, i32 0, i32 0
  %element_02 = load i64, ptr %element_ptr_01, align 4
  call void @__vyn_println(ptr @array_string.1)
  %element_ptr_03 = getelementptr [5 x i64], ptr %arr3, i32 0, i32 0
  %element_04 = load i64, ptr %element_ptr_03, align 4
  %element_ptr_15 = getelementptr [5 x i64], ptr %arr3, i32 0, i32 1
  %element_16 = load i64, ptr %element_ptr_15, align 4
  %element_ptr_27 = getelementptr [5 x i64], ptr %arr3, i32 0, i32 2
  %element_28 = load i64, ptr %element_ptr_27, align 4
  %element_ptr_3 = getelementptr [5 x i64], ptr %arr3, i32 0, i32 3
  %element_3 = load i64, ptr %element_ptr_3, align 4
  %element_ptr_4 = getelementptr [5 x i64], ptr %arr3, i32 0, i32 4
  %element_4 = load i64, ptr %element_ptr_4, align 4
  call void @__vyn_println(ptr @array_string.2)
  ret i64 0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
