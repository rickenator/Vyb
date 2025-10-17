; ModuleID = 'VynModule'
source_filename = "VynModule"

%Point = type { i64, i64 }

define %Point @main() {
entry:
  %p = alloca %Point, align 8
  %Point_obj = alloca %Point, align 8
  %x_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 0
  store i64 10, ptr %x_ptr, align 4
  %y_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 1
  store i64 20, ptr %y_ptr, align 4
  %Point_val = load %Point, ptr %Point_obj, align 4
  store %Point %Point_val, ptr %p, align 4
  %p1 = load %Point, ptr %p, align 4
  ret %Point %p1
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
