; ModuleID = 'VynModule'
source_filename = "VynModule"

@failer.str = private unnamed_addr constant [7 x i8] c"failer\00", align 1
@filepath.str = private unnamed_addr constant [21 x i8] c"test/simple_fail.vyn\00", align 1
@0 = private unnamed_addr constant [14 x i8] c"About to fail\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [11 x i8] c"Test error\00", align 1
@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str.1 = private unnamed_addr constant [21 x i8] c"test/simple_fail.vyn\00", align 1
@2 = private unnamed_addr constant [15 x i8] c"Calling failer\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

; Function Attrs: noinline
define { i64, ptr } @failer() #0 !dbg !4 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @failer.str, ptr @filepath.str, i32 3, i32 1), !dbg !8
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !8
  store { ptr, i64 } { ptr @0, i64 13 }, ptr %serialize_temp, align 8, !dbg !8
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !8
  call void @__vyn_println(ptr %serialized_json), !dbg !8
  %error.heap = call ptr @malloc(i64 24), !dbg !8
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !8
  store { ptr, i64 } { ptr @1, i64 10 }, ptr %error.data.ptr, align 8, !dbg !8
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.heap, 1, !dbg !8
  ret { i64, ptr } %error.ptr, !dbg !8
}

; Function Attrs: noinline
define i64 @main() #0 !dbg !9 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str.1, i32 9, i32 1), !dbg !13
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !13
  store { ptr, i64 } { ptr @2, i64 14 }, ptr %serialize_temp, align 8, !dbg !13
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.2), !dbg !13
  call void @__vyn_println(ptr %serialized_json), !dbg !13
  %calltmp = call { i64, ptr } @failer(), !dbg !13
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !13
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !13
  %has.error = icmp ne ptr %call.error, null, !dbg !13
  br i1 %has.error, label %call.error1, label %call.success, !dbg !13

call.error1:                                      ; preds = %entry
  call void @__vyn_runtime_untrapped_error(ptr %call.error), !dbg !13
  unreachable, !dbg !13

call.success:                                     ; preds = %entry
  call void @__vyn_runtime_pop_call_frame(), !dbg !13
  ret i64 %call.value, !dbg !13
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @malloc(i64)

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #1

declare void @__vyn_runtime_pop_call_frame()

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noinline }
attributes #1 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "simple_fail.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "failer", linkageName: "failer", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DILocation(line: 3, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 9, type: !10, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!10 = !DISubroutineType(types: !11)
!11 = !{!12}
!12 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!13 = !DILocation(line: 9, column: 1, scope: !9)
