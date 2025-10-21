; ModuleID = 'VynModule'
source_filename = "VynModule"

@processValue.str = private unnamed_addr constant [13 x i8] c"processValue\00", align 1
@filepath.str = private unnamed_addr constant [31 x i8] c"test/stack_trace_with_data.vyn\00", align 1
@0 = private unnamed_addr constant [17 x i8] c"Processing value\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [28 x i8] c"Negative values not allowed\00", align 1
@2 = private unnamed_addr constant [28 x i8] c"Value exceeds maximum (100)\00", align 1
@validateAndProcess.str = private unnamed_addr constant [19 x i8] c"validateAndProcess\00", align 1
@filepath.str.1 = private unnamed_addr constant [31 x i8] c"test/stack_trace_with_data.vyn\00", align 1
@3 = private unnamed_addr constant [17 x i8] c"Validating input\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@handleRequest.str = private unnamed_addr constant [14 x i8] c"handleRequest\00", align 1
@filepath.str.3 = private unnamed_addr constant [31 x i8] c"test/stack_trace_with_data.vyn\00", align 1
@4 = private unnamed_addr constant [17 x i8] c"Handling request\00", align 1
@type_name.4 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str.5 = private unnamed_addr constant [31 x i8] c"test/stack_trace_with_data.vyn\00", align 1
@5 = private unnamed_addr constant [45 x i8] c"=== Stack Trace Test: Error With Context ===\00", align 1
@type_name.6 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@6 = private unnamed_addr constant [29 x i8] c"Processing valid value (50):\00", align 1
@type_name.7 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@7 = private unnamed_addr constant [32 x i8] c"Processing invalid value (150):\00", align 1
@type_name.8 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

; Function Attrs: noinline
define { i64, ptr } @processValue(i64 %value) #0 !dbg !4 {
entry:
  %value1 = alloca i64, align 8, !dbg !11
  call void @__vyn_runtime_push_call_frame(ptr @processValue.str, ptr @filepath.str, i32 4, i32 1), !dbg !11
  store i64 %value, ptr %value1, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %value1, metadata !10, metadata !DIExpression()), !dbg !12
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !11
  store { ptr, i64 } { ptr @0, i64 16 }, ptr %serialize_temp, align 8, !dbg !11
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !11
  call void @__vyn_println(ptr %serialized_json), !dbg !11
  %value2 = load i64, ptr %value1, align 4, !dbg !11
  %icmpslttmp = icmp slt i64 %value2, 0, !dbg !11
  br i1 %icmpslttmp, label %then, label %ifcont, !dbg !11

then:                                             ; preds = %entry
  %error.heap = call ptr @malloc(i64 24), !dbg !11
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !11
  store { ptr, i64 } { ptr @1, i64 27 }, ptr %error.data.ptr, align 8, !dbg !11
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.heap, 1, !dbg !11
  ret { i64, ptr } %error.ptr, !dbg !11

ifcont:                                           ; preds = %entry
  %value3 = load i64, ptr %value1, align 4, !dbg !11
  %icmpsgttmp = icmp sgt i64 %value3, 100, !dbg !11
  br i1 %icmpsgttmp, label %then4, label %ifcont8, !dbg !11

then4:                                            ; preds = %ifcont
  %error.heap5 = call ptr @malloc(i64 24), !dbg !11
  %error.data.ptr6 = getelementptr i8, ptr %error.heap5, i64 8, !dbg !11
  store { ptr, i64 } { ptr @2, i64 27 }, ptr %error.data.ptr6, align 8, !dbg !11
  %error.ptr7 = insertvalue { i64, ptr } undef, ptr %error.heap5, 1, !dbg !11
  ret { i64, ptr } %error.ptr7, !dbg !11

ifcont8:                                          ; preds = %ifcont
  %value9 = load i64, ptr %value1, align 4, !dbg !11
  %multmp = mul i64 %value9, 2, !dbg !11
  %result.value = insertvalue { i64, ptr } undef, i64 %multmp, 0, !dbg !11
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !11
  call void @__vyn_runtime_pop_call_frame(), !dbg !11
  ret { i64, ptr } %result.error, !dbg !11
}

; Function Attrs: noinline
define { i64, ptr } @validateAndProcess(i64 %input) #0 !dbg !13 {
entry:
  %input1 = alloca i64, align 8, !dbg !16
  call void @__vyn_runtime_push_call_frame(ptr @validateAndProcess.str, ptr @filepath.str.1, i32 15, i32 1), !dbg !16
  store i64 %input, ptr %input1, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %input1, metadata !15, metadata !DIExpression()), !dbg !17
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !16
  store { ptr, i64 } { ptr @3, i64 16 }, ptr %serialize_temp, align 8, !dbg !16
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.2), !dbg !16
  call void @__vyn_println(ptr %serialized_json), !dbg !16
  %input2 = load i64, ptr %input1, align 4, !dbg !16
  %calltmp = call { i64, ptr } @processValue(i64 %input2), !dbg !16
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !16
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !16
  %has.error = icmp ne ptr %call.error, null, !dbg !16
  br i1 %has.error, label %call.error3, label %call.success, !dbg !16

call.error3:                                      ; preds = %entry
  %prop.error = insertvalue { i64, ptr } undef, ptr %call.error, 1, !dbg !16
  ret { i64, ptr } %prop.error, !dbg !16

call.success:                                     ; preds = %entry
  %result.value = insertvalue { i64, ptr } undef, i64 %call.value, 0, !dbg !16
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !16
  call void @__vyn_runtime_pop_call_frame(), !dbg !16
  ret { i64, ptr } %result.error, !dbg !16
}

; Function Attrs: noinline
define { i64, ptr } @handleRequest(i64 %requestId) #0 !dbg !18 {
entry:
  %requestId1 = alloca i64, align 8, !dbg !21
  call void @__vyn_runtime_push_call_frame(ptr @handleRequest.str, ptr @filepath.str.3, i32 20, i32 1), !dbg !21
  store i64 %requestId, ptr %requestId1, align 4, !dbg !21
  call void @llvm.dbg.declare(metadata ptr %requestId1, metadata !20, metadata !DIExpression()), !dbg !22
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !21
  store { ptr, i64 } { ptr @4, i64 16 }, ptr %serialize_temp, align 8, !dbg !21
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.4), !dbg !21
  call void @__vyn_println(ptr %serialized_json), !dbg !21
  %requestId2 = load i64, ptr %requestId1, align 4, !dbg !21
  %calltmp = call { i64, ptr } @validateAndProcess(i64 %requestId2), !dbg !21
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !21
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !21
  %has.error = icmp ne ptr %call.error, null, !dbg !21
  br i1 %has.error, label %call.error3, label %call.success, !dbg !21

call.error3:                                      ; preds = %entry
  %prop.error = insertvalue { i64, ptr } undef, ptr %call.error, 1, !dbg !21
  ret { i64, ptr } %prop.error, !dbg !21

call.success:                                     ; preds = %entry
  %result.value = insertvalue { i64, ptr } undef, i64 %call.value, 0, !dbg !21
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !21
  call void @__vyn_runtime_pop_call_frame(), !dbg !21
  ret { i64, ptr } %result.error, !dbg !21
}

; Function Attrs: noinline
define i64 @main() #0 !dbg !23 {
entry:
  %valid = alloca i64, align 8, !dbg !29
  %invalid = alloca i64, align 8, !dbg !29
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str.5, i32 25, i32 1), !dbg !29
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !29
  store { ptr, i64 } { ptr @5, i64 44 }, ptr %serialize_temp, align 8, !dbg !29
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.6), !dbg !29
  call void @__vyn_println(ptr %serialized_json), !dbg !29
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !29
  store { ptr, i64 } { ptr @6, i64 28 }, ptr %serialize_temp1, align 8, !dbg !29
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.7), !dbg !29
  call void @__vyn_println(ptr %serialized_json2), !dbg !29
  %calltmp = call { i64, ptr } @handleRequest(i64 50), !dbg !29
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !29
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !29
  %has.error = icmp ne ptr %call.error, null, !dbg !29
  br i1 %has.error, label %call.error3, label %call.success, !dbg !29

call.error3:                                      ; preds = %entry
  call void @__vyn_runtime_untrapped_error(ptr %call.error), !dbg !29
  unreachable, !dbg !29

call.success:                                     ; preds = %entry
  store i64 %call.value, ptr %valid, align 4, !dbg !29
  call void @llvm.dbg.declare(metadata ptr %valid, metadata !27, metadata !DIExpression()), !dbg !30
  %serialize_temp4 = alloca { ptr, i64 }, align 8, !dbg !29
  store { ptr, i64 } { ptr @7, i64 31 }, ptr %serialize_temp4, align 8, !dbg !29
  %serialized_json5 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp4, ptr @type_name.8), !dbg !29
  call void @__vyn_println(ptr %serialized_json5), !dbg !29
  %calltmp6 = call { i64, ptr } @handleRequest(i64 150), !dbg !29
  %call.value7 = extractvalue { i64, ptr } %calltmp6, 0, !dbg !29
  %call.error8 = extractvalue { i64, ptr } %calltmp6, 1, !dbg !29
  %has.error9 = icmp ne ptr %call.error8, null, !dbg !29
  br i1 %has.error9, label %call.error10, label %call.success11, !dbg !29

call.error10:                                     ; preds = %call.success
  call void @__vyn_runtime_untrapped_error(ptr %call.error8), !dbg !29
  unreachable, !dbg !29

call.success11:                                   ; preds = %call.success
  store i64 %call.value7, ptr %invalid, align 4, !dbg !29
  call void @llvm.dbg.declare(metadata ptr %invalid, metadata !28, metadata !DIExpression()), !dbg !31
  call void @__vyn_runtime_pop_call_frame(), !dbg !29
  ret i64 0, !dbg !29
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @malloc(i64)

declare void @__vyn_runtime_pop_call_frame()

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #2

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noinline }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "stack_trace_with_data.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "processValue", linkageName: "processValue", scope: !1, file: !1, line: 4, type: !5, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !9)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!9 = !{!10}
!10 = !DILocalVariable(name: "value", scope: !4, file: !1, line: 4, type: !8)
!11 = !DILocation(line: 4, column: 1, scope: !4)
!12 = !DILocation(line: 4, column: 19, scope: !4)
!13 = distinct !DISubprogram(name: "validateAndProcess", linkageName: "validateAndProcess", scope: !1, file: !1, line: 15, type: !5, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!14 = !{!15}
!15 = !DILocalVariable(name: "input", scope: !13, file: !1, line: 15, type: !8)
!16 = !DILocation(line: 15, column: 1, scope: !13)
!17 = !DILocation(line: 15, column: 25, scope: !13)
!18 = distinct !DISubprogram(name: "handleRequest", linkageName: "handleRequest", scope: !1, file: !1, line: 20, type: !5, scopeLine: 20, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !19)
!19 = !{!20}
!20 = !DILocalVariable(name: "requestId", scope: !18, file: !1, line: 20, type: !8)
!21 = !DILocation(line: 20, column: 1, scope: !18)
!22 = !DILocation(line: 20, column: 24, scope: !18)
!23 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 25, type: !24, scopeLine: 25, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !26)
!24 = !DISubroutineType(types: !25)
!25 = !{!8}
!26 = !{!27, !28}
!27 = !DILocalVariable(name: "valid", scope: !23, file: !1, line: 28, type: !8)
!28 = !DILocalVariable(name: "invalid", scope: !23, file: !1, line: 31, type: !8)
!29 = !DILocation(line: 25, column: 1, scope: !23)
!30 = !DILocation(line: 28, column: 1, scope: !23)
!31 = !DILocation(line: 31, column: 1, scope: !23)
