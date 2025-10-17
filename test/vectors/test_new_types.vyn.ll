; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %int32_first = alloca i32, align 4
  %float32_vec = alloca { ptr, i64, i64 }, align 8
  %int32_vec = alloca { ptr, i64, i64 }, align 8
  %large_float = alloca double, align 8
  %large_int = alloca i64, align 8
  %small_int = alloca i32, align 4
  store i32 42, ptr %small_int, align 4
  store i64 9223372036854775807, ptr %large_int, align 4
  store double 0x400921FB54442D18, ptr %large_float, align 8
  %vec.new = alloca { ptr, i64, i64 }, align 8
  %vec.data = call ptr @malloc(i64 24)
  %0 = call ptr @memset(ptr %vec.data, i32 0, i64 24)
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0
  store ptr %vec.data, ptr %vec.ptr_field, align 8
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1
  store i64 3, ptr %vec.size_field, align 4
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2
  store i64 3, ptr %vec.cap_field, align 4
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8
  store { ptr, i64, i64 } %vec.new.value, ptr %int32_vec, align 8
  %vec.new1 = alloca { ptr, i64, i64 }, align 8
  %vec.data2 = call ptr @malloc.1(i64 16)
  %1 = call ptr @memset.2(ptr %vec.data2, i32 0, i64 16)
  %vec.ptr_field3 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 0
  store ptr %vec.data2, ptr %vec.ptr_field3, align 8
  %vec.size_field4 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 1
  store i64 2, ptr %vec.size_field4, align 4
  %vec.cap_field5 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new1, i32 0, i32 2
  store i64 2, ptr %vec.cap_field5, align 4
  %vec.new.value6 = load { ptr, i64, i64 }, ptr %vec.new1, align 8
  store { ptr, i64, i64 } %vec.new.value6, ptr %float32_vec, align 8
  store i32 42, ptr %int32_first, align 4
  ret i64 0
}

declare void @println(ptr)

declare ptr @malloc(i64)

declare ptr @memset(ptr, i32, i64)

declare ptr @malloc.1(i64)

declare ptr @memset.2(ptr, i32, i64)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
