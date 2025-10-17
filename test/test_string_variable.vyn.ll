; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1

define i64 @main() {
entry:
  %greeting = alloca ptr, align 8
  store ptr @0, ptr %greeting, align 8
  %greeting1 = load ptr, ptr %greeting, align 8
  call void @__vyn_println(ptr %greeting1)
  ret i64 0
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
