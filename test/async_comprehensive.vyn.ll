; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [30 x i8] c"=== Async Operations Test ===\00", align 1
@1 = private unnamed_addr constant [39 x i8] c"Created future for compute_async_value\00", align 1
@2 = private unnamed_addr constant [34 x i8] c"Created future for process_values\00", align 1
@3 = private unnamed_addr constant [35 x i8] c"Created future for background_task\00", align 1
@4 = private unnamed_addr constant [31 x i8] c"All async operations initiated\00", align 1
@5 = private unnamed_addr constant [40 x i8] c"Starting async/await comprehensive test\00", align 1
@6 = private unnamed_addr constant [21 x i8] c"Async test completed\00", align 1

define void @test_async_operations() {
entry:
  call void @__vyn_println(ptr @0)
  call void @__vyn_println(ptr @1)
  call void @__vyn_println(ptr @2)
  call void @__vyn_println(ptr @3)
  call void @__vyn_println(ptr @4)
  ret void
}

declare void @__vyn_println(ptr)

define void @main() {
entry:
  call void @__vyn_println(ptr @5)
  call void @test_async_operations()
  call void @__vyn_println(ptr @6)
  ret void
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
