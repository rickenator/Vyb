; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [12 x i8] c"Vec created\00", align 1
@1 = private unnamed_addr constant [17 x i8] c"After scope exit\00", align 1

define i64 @main() {
entry:
  %v = alloca { ptr, i64, i64 }, align 8
  %vec.new = alloca { ptr, i64, i64 }, align 8
  %vec.data = call ptr @malloc(i64 8)
  %0 = call ptr @memset(ptr %vec.data, i32 0, i64 8)
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0
  store ptr %vec.data, ptr %vec.ptr_field, align 8
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1
  store i64 1, ptr %vec.size_field, align 4
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2
  store i64 1, ptr %vec.cap_field, align 4
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8
  store { ptr, i64, i64 } %vec.new.value, ptr %v, align 8
  call void @__vyn_println(ptr @0)
  %v_cleanup_load = load { ptr, i64, i64 }, ptr %v, align 8
  %v_data_ptr = extractvalue { ptr, i64, i64 } %v_cleanup_load, 0
  %v_null_check = icmp ne ptr %v_data_ptr, null
  br i1 %v_null_check, label %v_free_block, label %v_continue

v_free_block:                                     ; preds = %entry
  call void @free(ptr %v_data_ptr)
  br label %v_continue

v_continue:                                       ; preds = %v_free_block, %entry
  call void @__vyn_println(ptr @1)
  ret i64 0
}

declare ptr @malloc(i64)

declare ptr @memset(ptr, i32, i64)

declare void @__vyn_println(ptr)

declare void @free(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
