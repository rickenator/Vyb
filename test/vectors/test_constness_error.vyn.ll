; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %const_vec = alloca { ptr, i64, i64 }, align 8
  %mutable_vec = alloca { ptr, i64, i64 }, align 8
  %vec.new = alloca { ptr, i64, i64 }, align 8
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0
  store ptr null, ptr %vec.ptr_field, align 8
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1
  store i64 0, ptr %vec.size_field, align 4
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2
  store i64 0, ptr %vec.cap_field, align 4
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8
  store { ptr, i64, i64 } %vec.new.value, ptr %mutable_vec, align 8
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %mutable_vec, i32 0, i32 1
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4
  %vec.new_size = add i64 %vec.current_size, 1
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4
  %vec.new1 = alloca { ptr, i64, i64 }, align 8
  %vec.ptr_field2 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 0
  store ptr null, ptr %vec.ptr_field2, align 8
  %vec.size_field3 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 1
  store i64 0, ptr %vec.size_field3, align 4
  %vec.cap_field4 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 2
  store i64 0, ptr %vec.cap_field4, align 4
  %vec.new.value5 = load { ptr, i64, i64 }, ptr %vec.new1, align 8
  store { ptr, i64, i64 } %vec.new.value5, ptr %const_vec, align 8
  %vec.size_ptr6 = getelementptr inbounds { ptr, i64, i64 }, ptr %const_vec, i32 0, i32 1
  %vec.current_size7 = load i64, ptr %vec.size_ptr6, align 4
  %vec.new_size8 = add i64 %vec.current_size7, 1
  store i64 %vec.new_size8, ptr %vec.size_ptr6, align 4
  ret i64 0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
