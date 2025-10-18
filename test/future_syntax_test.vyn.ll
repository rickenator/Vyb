; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [42 x i8] c"Future type syntax parsing test completed\00", align 1

define void @main() {
entry:
  call void @__vyn_println(ptr @0)
  ret void
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
