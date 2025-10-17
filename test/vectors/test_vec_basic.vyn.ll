; ModuleID = 'VynModule'
source_filename = "VynModule"

@type_name = private unnamed_addr constant [9 x i8] c"Vec<Int>\00", align 1

define i64 @main() {
entry:
  %numbers = alloca { ptr, i64, i64 }, align 8
  %vec.new = alloca { ptr, i64, i64 }, align 8
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0
  store ptr null, ptr %vec.ptr_field, align 8
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1
  store i64 0, ptr %vec.size_field, align 4
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2
  store i64 0, ptr %vec.cap_field, align 4
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8
  store { ptr, i64, i64 } %vec.new.value, ptr %numbers, align 8
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4
  %vec.new_size = add i64 %vec.current_size, 1
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4
  %vec.size_ptr1 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1
  %vec.current_size2 = load i64, ptr %vec.size_ptr1, align 4
  %vec.new_size3 = add i64 %vec.current_size2, 1
  store i64 %vec.new_size3, ptr %vec.size_ptr1, align 4
  %vec.size_ptr4 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1
  %vec.current_size5 = load i64, ptr %vec.size_ptr4, align 4
  %vec.new_size6 = add i64 %vec.current_size5, 1
  store i64 %vec.new_size6, ptr %vec.size_ptr4, align 4
  %numbers7 = load { ptr, i64, i64 }, ptr %numbers, align 8
  %serialize_temp = alloca { ptr, i64, i64 }, align 8
  store { ptr, i64, i64 } %numbers7, ptr %serialize_temp, align 8
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name)
  call void @__vyn_println(ptr %serialized_json)
  ret i64 0
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_convert_lit_string(ptr)
