; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [27 x i8] c"Future type test completed\00", align 1

define void @test_future_types() {
entry:
  call void @__vyn_println(ptr @0)
  ret void
}

declare void @__vyn_println(ptr)

define void @main() {
entry:
  call void @test_future_types()
  ret void
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
