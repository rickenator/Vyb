; ModuleID = 'VynModule'
source_filename = "VynModule"

@type_name = private unnamed_addr constant [4 x i8] c"Int\00", align 1
@type_name.1 = private unnamed_addr constant [4 x i8] c"Int\00", align 1
@type_name.2 = private unnamed_addr constant [4 x i8] c"Int\00", align 1

define i64 @main() {
entry:
  %popped = alloca i64, align 8
  %first = alloca i64, align 8
  %length = alloca i64, align 8
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
  %vec.size_ptr7 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1
  %vec.len = load i64, ptr %vec.size_ptr7, align 4
  store i64 %vec.len, ptr %length, align 4
  %length8 = load i64, ptr %length, align 4
  %serialize_temp = alloca i64, align 8
  store i64 %length8, ptr %serialize_temp, align 4
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name)
  call void @__vyn_println(ptr %serialized_json)
  store i64 42, ptr %first, align 4
  %first9 = load i64, ptr %first, align 4
  %serialize_temp10 = alloca i64, align 8
  store i64 %first9, ptr %serialize_temp10, align 4
  %serialized_json11 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp10, ptr @type_name.1)
  call void @__vyn_println(ptr %serialized_json11)
  %vec.size_ptr12 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1
  %vec.current_size13 = load i64, ptr %vec.size_ptr12, align 4
  %vec.is_empty = icmp eq i64 %vec.current_size13, 0
  %vec.new_size14 = sub i64 %vec.current_size13, 1
  %vec.safe_new_size = select i1 %vec.is_empty, i64 0, i64 %vec.new_size14
  store i64 %vec.safe_new_size, ptr %vec.size_ptr12, align 4
  store i64 0, ptr %popped, align 4
  %popped15 = load i64, ptr %popped, align 4
  %serialize_temp16 = alloca i64, align 8
  store i64 %popped15, ptr %serialize_temp16, align 4
  %serialized_json17 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp16, ptr @type_name.2)
  call void @__vyn_println(ptr %serialized_json17)
  ret i64 0
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_convert_lit_string(ptr)
