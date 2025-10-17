; ModuleID = 'VynModule'
source_filename = "VynModule"

%Point = type { i64, i64 }

define i64 @main() {
entry:
  %sum = alloca i64, align 8
  %p2 = alloca %Point, align 8
  %p1 = alloca %Point, align 8
  %Point_obj = alloca %Point, align 8
  %x_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 0
  store i64 10, ptr %x_ptr, align 4
  %y_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 1
  store i64 20, ptr %y_ptr, align 4
  %Point_val = load %Point, ptr %Point_obj, align 4
  store %Point %Point_val, ptr %p1, align 4
  %Point_obj1 = alloca %Point, align 8
  %x_ptr2 = getelementptr inbounds %Point, ptr %Point_obj1, i32 0, i32 0
  store i64 5, ptr %x_ptr2, align 4
  %y_ptr3 = getelementptr inbounds %Point, ptr %Point_obj1, i32 0, i32 1
  store i64 15, ptr %y_ptr3, align 4
  %Point_val4 = load %Point, ptr %Point_obj1, align 4
  store %Point %Point_val4, ptr %p2, align 4
  %p15 = load %Point, ptr %p1, align 4
  %temp_struct = alloca %Point, align 8
  store %Point %p15, ptr %temp_struct, align 4
  %x_ptr6 = getelementptr inbounds %Point, ptr %temp_struct, i32 0, i32 0
  %x_val = load i64, ptr %x_ptr6, align 4
  %p17 = load %Point, ptr %p1, align 4
  %temp_struct8 = alloca %Point, align 8
  store %Point %p17, ptr %temp_struct8, align 4
  %y_ptr9 = getelementptr inbounds %Point, ptr %temp_struct8, i32 0, i32 1
  %y_val = load i64, ptr %y_ptr9, align 4
  %addtmp = add i64 %x_val, %y_val
  %p210 = load %Point, ptr %p2, align 4
  %temp_struct11 = alloca %Point, align 8
  store %Point %p210, ptr %temp_struct11, align 4
  %x_ptr12 = getelementptr inbounds %Point, ptr %temp_struct11, i32 0, i32 0
  %x_val13 = load i64, ptr %x_ptr12, align 4
  %addtmp14 = add i64 %addtmp, %x_val13
  %p215 = load %Point, ptr %p2, align 4
  %temp_struct16 = alloca %Point, align 8
  store %Point %p215, ptr %temp_struct16, align 4
  %y_ptr17 = getelementptr inbounds %Point, ptr %temp_struct16, i32 0, i32 1
  %y_val18 = load i64, ptr %y_ptr17, align 4
  %addtmp19 = add i64 %addtmp14, %y_val18
  store i64 %addtmp19, ptr %sum, align 4
  %sum20 = load i64, ptr %sum, align 4
  ret i64 %sum20
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
