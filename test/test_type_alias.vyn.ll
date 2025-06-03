; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [14 x i8] c"Hello, world!\00", align 1
@field_name = private unnamed_addr constant [6 x i8] c"MyInt\00", align 1
@field_name.1 = private unnamed_addr constant [9 x i8] c"MyString\00", align 1
@typename = private unnamed_addr constant [13 x i8] c"{ i64, ptr }\00", align 1

define i32 @main() {
entry:
  %y = alloca ptr, align 8
  %x = alloca i64, align 8
  store i64 47, ptr %x, align 4
  store ptr @0, ptr %y, align 8
  %x_value = load i64, ptr %x, align 4
  %y_value = load ptr, ptr %y, align 8
  %multi_return_struct = alloca { i64, ptr }, align 8
  %field_ptr = getelementptr inbounds { i64, ptr }, ptr %multi_return_struct, i32 0, i32 0
  store i64 %x_value, ptr %field_ptr, align 4
  %field_ptr1 = getelementptr inbounds { i64, ptr }, ptr %multi_return_struct, i32 0, i32 1
  store ptr %y_value, ptr %field_ptr1, align 8
  %multi_return_value = load { i64, ptr }, ptr %multi_return_struct, align 8
  %struct_temp = alloca { i64, ptr }, align 8
  store { i64, ptr } %multi_return_value, ptr %struct_temp, align 8
  %field_names_array = alloca [2 x ptr], align 8
  %element_ptr = getelementptr [2 x ptr], ptr %field_names_array, i32 0, i32 0
  store ptr @field_name, ptr %element_ptr, align 8
  %element_ptr2 = getelementptr [2 x ptr], ptr %field_names_array, i32 0, i32 1
  store ptr @field_name.1, ptr %element_ptr2, align 8
  %field_names_ptr = getelementptr [2 x ptr], ptr %field_names_array, i32 0, i32 0
  %json_result = call ptr @__vyn_serialize_struct_with_names(ptr %struct_temp, ptr @typename, ptr %field_names_ptr, i32 2)
  call void @__vyn_println(ptr %json_result)
  ret i32 0
}

declare ptr @__vyn_serialize_struct_with_names(ptr, ptr, ptr, i32)

declare void @__vyn_println(ptr)
