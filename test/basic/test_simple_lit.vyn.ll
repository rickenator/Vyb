; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [3 x i8] c"42\00", align 1

define ptr @main() {
entry:
  %lit_result = call ptr @__vyn_convert_lit_string(ptr @0)
  ret ptr %lit_result
}

declare ptr @__vyn_convert_lit_string(ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)
