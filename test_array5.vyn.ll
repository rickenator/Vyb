; ModuleID = 'VynModule'
source_filename = "VynModule"

@array_string = private unnamed_addr constant [13 x i8] c"[10, 20, 30]\00", align 1

define i64 @main() {
entry:
  %arr = alloca [3 x i64], align 8
  store [3 x i64] [i64 10, i64 20, i64 30], ptr %arr, align 4
  %element_ptr_0 = getelementptr [3 x i64], ptr %arr, i32 0, i32 0
  %element_0 = load i64, ptr %element_ptr_0, align 4
  %element_ptr_1 = getelementptr [3 x i64], ptr %arr, i32 0, i32 1
  %element_1 = load i64, ptr %element_ptr_1, align 4
  %element_ptr_2 = getelementptr [3 x i64], ptr %arr, i32 0, i32 2
  %element_2 = load i64, ptr %element_ptr_2, align 4
  call void @__vyn_println(ptr @array_string)
  ret i64 0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
