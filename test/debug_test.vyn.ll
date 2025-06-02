; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [27 x i8] c"DEBUG: Inside get_values()\00", align 1
@1 = private unnamed_addr constant [14 x i8] c"Hello, World!\00", align 1
@2 = private unnamed_addr constant [53 x i8] c"DEBUG: Inside main, returning single Int value first\00", align 1

define { i64, ptr } @get_values() {
entry:
  call void @__vyn_println(ptr @0)
  %return_struct = alloca { i64, ptr }, align 8
  %field_ptr = getelementptr inbounds { i64, ptr }, ptr %return_struct, i32 0, i32 0
  store i64 42, ptr %field_ptr, align 4
  %field_ptr1 = getelementptr inbounds { i64, ptr }, ptr %return_struct, i32 0, i32 1
  store ptr @1, ptr %field_ptr1, align 8
  %return_value = load { i64, ptr }, ptr %return_struct, align 8
  ret { i64, ptr } %return_value
}

declare void @__vyn_println(ptr)

define i64 @main() {
entry:
  call void @__vyn_println(ptr @2)
  ret i64 100
}
