; ModuleID = 'VynModule'
source_filename = "VynModule"

%Point = type { i64, i64 }

define i64 @main() {
entry:
  %p = alloca %Point, align 8
  %Point_obj = alloca %Point, align 8
  %x_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 0
  store i64 10, ptr %x_ptr, align 4
  %y_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 1
  store i64 20, ptr %y_ptr, align 4
  %Point_val = load %Point, ptr %Point_obj, align 4
  store %Point %Point_val, ptr %p, align 4
  %x_ptr1 = getelementptr inbounds %Point, ptr %p, i32 0, i32 0
  store i64 30, ptr %x_ptr1, align 4
  %y_ptr2 = getelementptr inbounds %Point, ptr %p, i32 0, i32 1
  store i64 40, ptr %y_ptr2, align 4
  %p3 = load %Point, ptr %p, align 4
  %temp_struct = alloca %Point, align 8
  store %Point %p3, ptr %temp_struct, align 4
  %x_ptr4 = getelementptr inbounds %Point, ptr %temp_struct, i32 0, i32 0
  %x_val = load i64, ptr %x_ptr4, align 4
  %p5 = load %Point, ptr %p, align 4
  %temp_struct6 = alloca %Point, align 8
  store %Point %p5, ptr %temp_struct6, align 4
  %y_ptr7 = getelementptr inbounds %Point, ptr %temp_struct6, i32 0, i32 1
  %y_val = load i64, ptr %y_ptr7, align 4
  %addtmp = add i64 %x_val, %y_val
  ret i64 %addtmp
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
