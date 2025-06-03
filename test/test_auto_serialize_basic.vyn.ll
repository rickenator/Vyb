; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@typename = private unnamed_addr constant [17 x i8] c"{ ptr, i64, i1 }\00", align 1

define i32 @main() {
entry:
  %ret_temp = alloca { ptr, i64, i1 }, align 8
  store { ptr, i64, i1 } { ptr @0, i64 42, i1 true }, ptr %ret_temp, align 8
  %json_result = call ptr @__vyn_serialize_to_json(ptr %ret_temp, ptr @typename)
  call void @__vyn_println(ptr %json_result)
  ret i32 0
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)
