; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1

define i64 @main() {
entry:
  call void @__vyn_println(ptr @0)
  ret i64 0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
