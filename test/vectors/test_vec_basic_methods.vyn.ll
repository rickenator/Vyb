; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [28 x i8] c"Vec created with 3 elements\00", align 1
@1 = private unnamed_addr constant [31 x i8] c"Buffer contents: 100, 200, 300\00", align 1
@2 = private unnamed_addr constant [41 x i8] c"Large buffer first element should be 100\00", align 1

define i64 @main() {
entry:
  %empty_vec = alloca { ptr, i64, i64 }, align 8
  %copied2 = alloca i64, align 8
  %big_buffer = alloca [5 x i64], align 8
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
  store [5 x i64] [i64 1, i64 2, i64 3, i64 4, i64 5], ptr %big_buffer, align 4
  %big_buffer9 = load [5 x i64], ptr %big_buffer, align 4
  %vec.size_ptr10 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec, i32 0, i32 1
  %vec.size11 = load i64, ptr %vec.size_ptr10, align 4
  store i64 %vec.size11, ptr %copied2, align 4
  call void @__vyn_println(ptr @2)
  %vec.new12 = alloca { ptr, i64, i64 }, align 8
  %vec.ptr_field13 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new12, i32 0, i32 0
  store ptr null, ptr %vec.ptr_field13, align 8
  %vec.size_field14 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new12, i32 0, i32 1
  store i64 0, ptr %vec.size_field14, align 4
  %vec.cap_field15 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new12, i32 0, i32 2
  store i64 0, ptr %vec.cap_field15, align 4
  %vec.new.value16 = load { ptr, i64, i64 }, ptr %vec.new12, align 8
  store { ptr, i64, i64 } %vec.new.value16, ptr %empty_vec, align 8
  ret i64 0
}

declare void @__vyn_println(ptr)

declare void @println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
