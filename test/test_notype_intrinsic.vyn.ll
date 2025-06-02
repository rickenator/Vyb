; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"world\00", align 1
@type_name = private unnamed_addr constant [7 x i8] c"string\00", align 1
@type_name.1 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1
@type_name.2 = private unnamed_addr constant [7 x i8] c"string\00", align 1

define i32 @main() {
entry:
  %array_ctx = call ptr @__vyn_begin_json_array()
  %obj_ctx = call ptr @__vyn_begin_json_object()
  call void @__vyn_add_json_field(ptr %obj_ctx, ptr @type_name, ptr @0, ptr @type_name)
  %elem_json = call ptr @__vyn_end_json_object(ptr %obj_ctx)
  call void @__vyn_add_json_array_element(ptr %array_ctx, ptr %elem_json)
  %ret_tmp = alloca i64, align 8
  store i64 42, ptr %ret_tmp, align 4
  %obj_ctx1 = call ptr @__vyn_begin_json_object()
  call void @__vyn_add_json_field(ptr %obj_ctx1, ptr @type_name.1, ptr %ret_tmp, ptr @type_name.1)
  %elem_json2 = call ptr @__vyn_end_json_object(ptr %obj_ctx1)
  call void @__vyn_add_json_array_element(ptr %array_ctx, ptr %elem_json2)
  %obj_ctx3 = call ptr @__vyn_begin_json_object()
  call void @__vyn_add_json_field(ptr %obj_ctx3, ptr @type_name.2, ptr @1, ptr @type_name.2)
  %elem_json4 = call ptr @__vyn_end_json_object(ptr %obj_ctx3)
  call void @__vyn_add_json_array_element(ptr %array_ctx, ptr %elem_json4)
  %json_array = call ptr @__vyn_end_json_array(ptr %array_ctx)
  call void @__vyn_println(ptr %json_array)
  ret i32 0
}

declare ptr @__vyn_begin_json_array()

declare ptr @__vyn_begin_json_object()

declare void @__vyn_add_json_field(ptr, ptr, ptr, ptr)

declare void @__vyn_add_json_field_notype(ptr, ptr, ptr)

declare ptr @__vyn_end_json_object(ptr)

declare void @__vyn_add_json_array_element(ptr, ptr)

declare ptr @__vyn_end_json_array(ptr)

declare void @__vyn_println(ptr)
