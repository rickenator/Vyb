; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [6 x i8] c"Alice\00", align 1
@1 = private unnamed_addr constant [10 x i8] c"User ID: \00", align 1
@2 = private unnamed_addr constant [12 x i8] c"User Name: \00", align 1
@3 = private unnamed_addr constant [8 x i8] c"Score: \00", align 1
@typename = private unnamed_addr constant [17 x i8] c"{ i64, ptr, i8 }\00", align 1

define i64 @getUserId() {
entry:
  ret i64 42
}

define ptr @getUserName() {
entry:
  ret ptr @0
}

define i8 @getScore() {
entry:
  ret i8 100
}

define i32 @main() {
entry:
  %score = alloca i8, align 1
  %name = alloca ptr, align 8
  %id = alloca i64, align 8
  %calltmp = call i64 @getUserId()
  store i64 %calltmp, ptr %id, align 4
  %calltmp1 = call ptr @getUserName()
  store ptr %calltmp1, ptr %name, align 8
  %calltmp2 = call i8 @getScore()
  store i8 %calltmp2, ptr %score, align 1
  %id3 = load i64, ptr %id, align 4
  %tostring = call ptr @__vyn_toString_int(i64 %id3)
  %strcattmp = call ptr @__vyn_string_concat(ptr @1, ptr %tostring)
  call void @__vyn_println(ptr %strcattmp)
  %name4 = load ptr, ptr %name, align 8
  %strcattmp5 = call ptr @__vyn_string_concat(ptr @2, ptr %name4)
  call void @__vyn_println(ptr %strcattmp5)
  %score6 = load i8, ptr %score, align 1
  %tostring7 = call ptr @__vyn_toString_int8(i8 %score6)
  %strcattmp8 = call ptr @__vyn_string_concat(ptr @3, ptr %tostring7)
  call void @__vyn_println(ptr %strcattmp8)
  %id9 = load i64, ptr %id, align 4
  %name10 = load ptr, ptr %name, align 8
  %score11 = load i8, ptr %score, align 1
  %tuple_insert = insertvalue { i64, ptr, i8 } undef, i64 %id9, 0
  %tuple_insert12 = insertvalue { i64, ptr, i8 } %tuple_insert, ptr %name10, 1
  %tuple_insert13 = insertvalue { i64, ptr, i8 } %tuple_insert12, i8 %score11, 2
  %ret_temp = alloca { i64, ptr, i8 }, align 8
  store { i64, ptr, i8 } %tuple_insert13, ptr %ret_temp, align 8
  %json_result = call ptr @__vyn_serialize_to_json(ptr %ret_temp, ptr @typename)
  call void @__vyn_println(ptr %json_result)
  ret i32 0
}

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_toString_int8(i8)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
