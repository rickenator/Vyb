; ModuleID = 'VynModule'
source_filename = "VynModule"

@level5.str = private unnamed_addr constant [7 x i8] c"level5\00", align 1
@filepath.str = private unnamed_addr constant [26 x i8] c"test/stack_trace_deep.vyn\00", align 1
@0 = private unnamed_addr constant [37 x i8] c"Level 5: Deepest call - failing here\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [17 x i8] c"Error at depth 5\00", align 1
@level4.str = private unnamed_addr constant [7 x i8] c"level4\00", align 1
@filepath.str.1 = private unnamed_addr constant [26 x i8] c"test/stack_trace_deep.vyn\00", align 1
@2 = private unnamed_addr constant [25 x i8] c"Level 4: Calling level 5\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@level3.str = private unnamed_addr constant [7 x i8] c"level3\00", align 1
@filepath.str.3 = private unnamed_addr constant [26 x i8] c"test/stack_trace_deep.vyn\00", align 1
@3 = private unnamed_addr constant [25 x i8] c"Level 3: Calling level 4\00", align 1
@type_name.4 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@level2.str = private unnamed_addr constant [7 x i8] c"level2\00", align 1
@filepath.str.5 = private unnamed_addr constant [26 x i8] c"test/stack_trace_deep.vyn\00", align 1
@4 = private unnamed_addr constant [25 x i8] c"Level 2: Calling level 3\00", align 1
@type_name.6 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@level1.str = private unnamed_addr constant [7 x i8] c"level1\00", align 1
@filepath.str.7 = private unnamed_addr constant [26 x i8] c"test/stack_trace_deep.vyn\00", align 1
@5 = private unnamed_addr constant [25 x i8] c"Level 1: Calling level 2\00", align 1
@type_name.8 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str.9 = private unnamed_addr constant [26 x i8] c"test/stack_trace_deep.vyn\00", align 1
@6 = private unnamed_addr constant [53 x i8] c"=== Stack Trace Test: Deep Call Stack (5 levels) ===\00", align 1
@type_name.10 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

; Function Attrs: noinline
define { i64, ptr } @level5() #0 !dbg !4 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @level5.str, ptr @filepath.str, i32 4, i32 1), !dbg !8
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !8
  store { ptr, i64 } { ptr @0, i64 36 }, ptr %serialize_temp, align 8, !dbg !8
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !8
  call void @__vyn_println(ptr %serialized_json), !dbg !8
  %error.heap = call ptr @malloc(i64 24), !dbg !8
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !8
  store { ptr, i64 } { ptr @1, i64 16 }, ptr %error.data.ptr, align 8, !dbg !8
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.heap, 1, !dbg !8
  ret { i64, ptr } %error.ptr, !dbg !8
}

; Function Attrs: noinline
define { i64, ptr } @level4() #0 !dbg !9 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @level4.str, ptr @filepath.str.1, i32 10, i32 1), !dbg !10
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @2, i64 24 }, ptr %serialize_temp, align 8, !dbg !10
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.2), !dbg !10
  call void @__vyn_println(ptr %serialized_json), !dbg !10
  %calltmp = call { i64, ptr } @level5(), !dbg !10
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !10
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !10
  %has.error = icmp ne ptr %call.error, null, !dbg !10
  br i1 %has.error, label %call.error1, label %call.success, !dbg !10

call.error1:                                      ; preds = %entry
  %prop.error = insertvalue { i64, ptr } undef, ptr %call.error, 1, !dbg !10
  ret { i64, ptr } %prop.error, !dbg !10

call.success:                                     ; preds = %entry
  %result.value = insertvalue { i64, ptr } undef, i64 %call.value, 0, !dbg !10
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !10
  call void @__vyn_runtime_pop_call_frame(), !dbg !10
  ret { i64, ptr } %result.error, !dbg !10
}

; Function Attrs: noinline
define { i64, ptr } @level3() #0 !dbg !11 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @level3.str, ptr @filepath.str.3, i32 15, i32 1), !dbg !12
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !12
  store { ptr, i64 } { ptr @3, i64 24 }, ptr %serialize_temp, align 8, !dbg !12
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.4), !dbg !12
  call void @__vyn_println(ptr %serialized_json), !dbg !12
  %calltmp = call { i64, ptr } @level4(), !dbg !12
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !12
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !12
  %has.error = icmp ne ptr %call.error, null, !dbg !12
  br i1 %has.error, label %call.error1, label %call.success, !dbg !12

call.error1:                                      ; preds = %entry
  %prop.error = insertvalue { i64, ptr } undef, ptr %call.error, 1, !dbg !12
  ret { i64, ptr } %prop.error, !dbg !12

call.success:                                     ; preds = %entry
  %result.value = insertvalue { i64, ptr } undef, i64 %call.value, 0, !dbg !12
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !12
  call void @__vyn_runtime_pop_call_frame(), !dbg !12
  ret { i64, ptr } %result.error, !dbg !12
}

; Function Attrs: noinline
define { i64, ptr } @level2() #0 !dbg !13 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @level2.str, ptr @filepath.str.5, i32 20, i32 1), !dbg !14
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !14
  store { ptr, i64 } { ptr @4, i64 24 }, ptr %serialize_temp, align 8, !dbg !14
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.6), !dbg !14
  call void @__vyn_println(ptr %serialized_json), !dbg !14
  %calltmp = call { i64, ptr } @level3(), !dbg !14
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !14
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !14
  %has.error = icmp ne ptr %call.error, null, !dbg !14
  br i1 %has.error, label %call.error1, label %call.success, !dbg !14

call.error1:                                      ; preds = %entry
  %prop.error = insertvalue { i64, ptr } undef, ptr %call.error, 1, !dbg !14
  ret { i64, ptr } %prop.error, !dbg !14

call.success:                                     ; preds = %entry
  %result.value = insertvalue { i64, ptr } undef, i64 %call.value, 0, !dbg !14
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !14
  call void @__vyn_runtime_pop_call_frame(), !dbg !14
  ret { i64, ptr } %result.error, !dbg !14
}

; Function Attrs: noinline
define { i64, ptr } @level1() #0 !dbg !15 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @level1.str, ptr @filepath.str.7, i32 25, i32 1), !dbg !16
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !16
  store { ptr, i64 } { ptr @5, i64 24 }, ptr %serialize_temp, align 8, !dbg !16
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.8), !dbg !16
  call void @__vyn_println(ptr %serialized_json), !dbg !16
  %calltmp = call { i64, ptr } @level2(), !dbg !16
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !16
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !16
  %has.error = icmp ne ptr %call.error, null, !dbg !16
  br i1 %has.error, label %call.error1, label %call.success, !dbg !16

call.error1:                                      ; preds = %entry
  %prop.error = insertvalue { i64, ptr } undef, ptr %call.error, 1, !dbg !16
  ret { i64, ptr } %prop.error, !dbg !16

call.success:                                     ; preds = %entry
  %result.value = insertvalue { i64, ptr } undef, i64 %call.value, 0, !dbg !16
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !16
  call void @__vyn_runtime_pop_call_frame(), !dbg !16
  ret { i64, ptr } %result.error, !dbg !16
}

; Function Attrs: noinline
define i64 @main() #0 !dbg !17 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str.9, i32 30, i32 1), !dbg !21
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !21
  store { ptr, i64 } { ptr @6, i64 52 }, ptr %serialize_temp, align 8, !dbg !21
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.10), !dbg !21
  call void @__vyn_println(ptr %serialized_json), !dbg !21
  %calltmp = call { i64, ptr } @level1(), !dbg !21
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !21
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !21
  %has.error = icmp ne ptr %call.error, null, !dbg !21
  br i1 %has.error, label %call.error1, label %call.success, !dbg !21

call.error1:                                      ; preds = %entry
  call void @__vyn_runtime_untrapped_error(ptr %call.error), !dbg !21
  unreachable, !dbg !21

call.success:                                     ; preds = %entry
  call void @__vyn_runtime_pop_call_frame(), !dbg !21
  ret i64 %call.value, !dbg !21
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @malloc(i64)

declare void @__vyn_runtime_pop_call_frame()

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #1

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noinline }
attributes #1 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "stack_trace_deep.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "level5", linkageName: "level5", scope: !1, file: !1, line: 4, type: !5, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DILocation(line: 4, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "level4", linkageName: "level4", scope: !1, file: !1, line: 10, type: !5, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!10 = !DILocation(line: 10, column: 1, scope: !9)
!11 = distinct !DISubprogram(name: "level3", linkageName: "level3", scope: !1, file: !1, line: 15, type: !5, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!12 = !DILocation(line: 15, column: 1, scope: !11)
!13 = distinct !DISubprogram(name: "level2", linkageName: "level2", scope: !1, file: !1, line: 20, type: !5, scopeLine: 20, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!14 = !DILocation(line: 20, column: 1, scope: !13)
!15 = distinct !DISubprogram(name: "level1", linkageName: "level1", scope: !1, file: !1, line: 25, type: !5, scopeLine: 25, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!16 = !DILocation(line: 25, column: 1, scope: !15)
!17 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 30, type: !18, scopeLine: 30, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!18 = !DISubroutineType(types: !19)
!19 = !{!20}
!20 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!21 = !DILocation(line: 30, column: 1, scope: !17)
