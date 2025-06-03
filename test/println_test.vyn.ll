; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [18 x i8] c"Hello, Vyn World!\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [13 x i8] c"The sum is: \00", align 1

define i32 @main() {
entry:
  call void @__vyn_println(ptr @0)
  %serialize_temp = alloca i64, align 8
  store i64 3, ptr %serialize_temp, align 4
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name)
  call void @__vyn_println(ptr %serialized_json)
  call void @__vyn_println(ptr add (ptr @1, i64 12))
  ret i32 0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)
