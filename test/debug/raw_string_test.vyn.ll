; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [11 x i8] c"raw string\00", align 1

define ptr @main() {
entry:
  ret ptr @0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
