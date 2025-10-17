; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [30 x i8] c"Vec populated with 5 elements\00", align 1
@1 = private unnamed_addr constant [47 x i8] c"Testing get_array with different buffer sizes:\00", align 1
@2 = private unnamed_addr constant [35 x i8] c"Performance test - reusing buffer:\00", align 1
@3 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@4 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@5 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@6 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@7 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define i64 @main() {
entry:
  %string_buffer = alloca [5 x ptr], align 8
  %string_vec = alloca { ptr, i64, i64 }, align 8
  %i = alloca i64, align 8
  %reusable_buffer = alloca [10 x i64], align 8
  %large_buffer = alloca [8 x i64], align 8
  %exact_buffer = alloca [5 x i64], align 8
  %small_buffer = alloca [3 x i64], align 8
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
  call void @__vyn_println(ptr @0)
  store [3 x i64] zeroinitializer, ptr %small_buffer, align 4
  store [5 x i64] zeroinitializer, ptr %exact_buffer, align 4
  store [8 x i64] zeroinitializer, ptr %large_buffer, align 4
  call void @__vyn_println(ptr @1)
  call void @__vyn_println(ptr @2)
  store [10 x i64] zeroinitializer, ptr %reusable_buffer, align 4
  store i64 0, ptr %i, align 4
  br label %loop.header

loop.header:                                      ; preds = %loop.body, %entry
  %i1 = load i64, ptr %i, align 4
  %icmpslttmp = icmp slt i64 %i1, 3
  br i1 %icmpslttmp, label %loop.body, label %loop.exit

loop.body:                                        ; preds = %loop.header
  %i2 = load i64, ptr %i, align 4
  %addtmp = add i64 60, %i2
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %vec, i32 0, i32 1
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4
  %vec.new_size = add i64 %vec.current_size, 1
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4
  %i3 = load i64, ptr %i, align 4
  %addtmp4 = add i64 %i3, 1
  store i64 %addtmp4, ptr %i, align 4
  br label %loop.header

loop.exit:                                        ; preds = %loop.header
  %vec.new5 = alloca { ptr, i64, i64 }, align 8
  %vec.ptr_field6 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new5, i32 0, i32 0
  store ptr null, ptr %vec.ptr_field6, align 8
  %vec.size_field7 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new5, i32 0, i32 1
  store i64 0, ptr %vec.size_field7, align 4
  %vec.cap_field8 = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new5, i32 0, i32 2
  store i64 0, ptr %vec.cap_field8, align 4
  %vec.new.value9 = load { ptr, i64, i64 }, ptr %vec.new5, align 8
  store { ptr, i64, i64 } %vec.new.value9, ptr %string_vec, align 8
  store [5 x ptr] [ptr @3, ptr @4, ptr @5, ptr @6, ptr @7], ptr %string_buffer, align 8
  ret i64 0
}

declare void @__vyn_println(ptr)

declare void @println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
