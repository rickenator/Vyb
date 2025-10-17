; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [30 x i8] c"Vec populated with 3 elements\00", align 1
@1 = private unnamed_addr constant [17 x i8] c"Copied elements:\00", align 1
@type_name = private unnamed_addr constant [4 x i8] c"Int\00", align 1

define i64 @main() {
entry:
  %copied = alloca i64, align 8
  %buffer = alloca [3 x i64], align 8
  %vec = alloca { ptr, i64, i64 }, align 8
  %vec.new = alloca { ptr, i64, i64 }, align 8
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0
  store ptr null, ptr %vec.ptr_field, align 8
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1
  store i64 0, ptr %vec.size_field, align 4
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2
  store i64 0, ptr %vec.cap_field, align 4
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8
  store { ptr, i64, i64 } %vec.new.value, ptr %vec, align 8
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %vec, i32 0, i32 1
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4
  %vec.new_size = add i64 %vec.current_size, 1
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4
  %vec.size_ptr1 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec, i32 0, i32 1
  %vec.current_size2 = load i64, ptr %vec.size_ptr1, align 4
  %vec.new_size3 = add i64 %vec.current_size2, 1
  store i64 %vec.new_size3, ptr %vec.size_ptr1, align 4
  %vec.size_ptr4 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec, i32 0, i32 1
  %vec.current_size5 = load i64, ptr %vec.size_ptr4, align 4
  %vec.new_size6 = add i64 %vec.current_size5, 1
  store i64 %vec.new_size6, ptr %vec.size_ptr4, align 4
  call void @__vyn_println(ptr @0)
  store [3 x i64] zeroinitializer, ptr %buffer, align 4
  %buffer7 = load [3 x i64], ptr %buffer, align 4
  %vec.size_ptr8 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec, i32 0, i32 1
  %vec.size = load i64, ptr %vec.size_ptr8, align 4
  store i64 %vec.size, ptr %copied, align 4
  call void @__vyn_println(ptr @1)
  %copied9 = load i64, ptr %copied, align 4
  %serialize_temp = alloca i64, align 8
  store i64 %copied9, ptr %serialize_temp, align 4
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name)
  call void @__vyn_println(ptr %serialized_json)
  ret i64 0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
