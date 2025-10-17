; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [14 x i8] c"Hello, World!\00", align 1
@typename = private unnamed_addr constant [13 x i8] c"{ i64, ptr }\00", align 1

define { i64, ptr } @get_values() {
entry:
  ret { i64, ptr } { i64 42, ptr @0 }
}

define i32 @main() {
entry:
  %calltmp = call { i64, ptr } @get_values()
  %ret_temp = alloca { i64, ptr }, align 8
  store { i64, ptr } %calltmp, ptr %ret_temp, align 8
  %json_result = call ptr @__vyn_serialize_to_json(ptr %ret_temp, ptr @typename)
  call void @__vyn_println(ptr %json_result)
  ret i32 0
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)
