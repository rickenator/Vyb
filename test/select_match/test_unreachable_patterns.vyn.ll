; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [38 x i8] c"Testing unreachable pattern detection\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@type_name.3 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@type_name.4 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@type_name.5 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define i64 @test_wildcard_position() !dbg !4 {
entry:
  %result = alloca i64, align 8
  %value = alloca i64, align 8
  store i64 42, ptr %value, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %value, metadata !9, metadata !DIExpression()), !dbg !12
  %value1 = load i64, ptr %value, align 4, !dbg !13
  %select.result = alloca i64, align 8, !dbg !13
  store i64 1, ptr %select.result, align 4, !dbg !13
  br label %select.end, !dbg !13

select.end:                                       ; preds = %entry
  %select.result.load = load i64, ptr %select.result, align 4, !dbg !13
  store i64 %select.result.load, ptr %result, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %result, metadata !10, metadata !DIExpression()), !dbg !14
  %result2 = load i64, ptr %result, align 4, !dbg !13
  ret i64 %result2, !dbg !13
}

define i64 @test_subsumption_gte() !dbg !15 {
entry:
  %result = alloca i64, align 8
  %value = alloca i64, align 8
  store i64 85, ptr %value, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %value, metadata !17, metadata !DIExpression()), !dbg !20
  %value1 = load i64, ptr %value, align 4, !dbg !21
  %select.result = alloca i64, align 8, !dbg !21
  %select.cmp.ge = icmp sge i64 %value1, 80, !dbg !21
  br i1 %select.cmp.ge, label %select.case, label %select.next, !dbg !21

select.case:                                      ; preds = %entry
  store i64 1, ptr %select.result, align 4, !dbg !21
  br label %select.end, !dbg !21

select.next:                                      ; preds = %entry
  %select.cmp.ge3 = icmp sge i64 %value1, 90, !dbg !21
  br i1 %select.cmp.ge3, label %select.case2, label %select.next4, !dbg !21

select.case2:                                     ; preds = %select.next
  store i64 2, ptr %select.result, align 4, !dbg !21
  br label %select.end, !dbg !21

select.next4:                                     ; preds = %select.next
  br label %select.end, !dbg !21

select.end:                                       ; preds = %select.next4, %select.case2, %select.case
  %select.result.load = load i64, ptr %select.result, align 4, !dbg !21
  store i64 %select.result.load, ptr %result, align 4, !dbg !21
  call void @llvm.dbg.declare(metadata ptr %result, metadata !18, metadata !DIExpression()), !dbg !22
  %result5 = load i64, ptr %result, align 4, !dbg !21
  ret i64 %result5, !dbg !21
}

define i64 @test_duplicate_patterns() !dbg !23 {
entry:
  %result = alloca i64, align 8
  %value = alloca i64, align 8
  store i64 75, ptr %value, align 4, !dbg !27
  call void @llvm.dbg.declare(metadata ptr %value, metadata !25, metadata !DIExpression()), !dbg !28
  %value1 = load i64, ptr %value, align 4, !dbg !29
  %select.result = alloca i64, align 8, !dbg !29
  %select.cmp.eq = icmp eq i64 %value1, 75, !dbg !29
  br i1 %select.cmp.eq, label %select.case, label %select.next, !dbg !29

select.case:                                      ; preds = %entry
  store i64 1, ptr %select.result, align 4, !dbg !29
  br label %select.end, !dbg !29

select.next:                                      ; preds = %entry
  %select.cmp.eq3 = icmp eq i64 %value1, 75, !dbg !29
  br i1 %select.cmp.eq3, label %select.case2, label %select.next4, !dbg !29

select.case2:                                     ; preds = %select.next
  store i64 2, ptr %select.result, align 4, !dbg !29
  br label %select.end, !dbg !29

select.next4:                                     ; preds = %select.next
  br label %select.end, !dbg !29

select.end:                                       ; preds = %select.next4, %select.case2, %select.case
  %select.result.load = load i64, ptr %select.result, align 4, !dbg !29
  store i64 %select.result.load, ptr %result, align 4, !dbg !29
  call void @llvm.dbg.declare(metadata ptr %result, metadata !26, metadata !DIExpression()), !dbg !30
  %result5 = load i64, ptr %result, align 4, !dbg !29
  ret i64 %result5, !dbg !29
}

define i64 @test_range_overlap() !dbg !31 {
entry:
  %result = alloca i64, align 8
  %value = alloca i64, align 8
  store i64 75, ptr %value, align 4, !dbg !35
  call void @llvm.dbg.declare(metadata ptr %value, metadata !33, metadata !DIExpression()), !dbg !36
  %value1 = load i64, ptr %value, align 4, !dbg !37
  %select.result = alloca i64, align 8, !dbg !37
  %select.cmp.ge = icmp sge i64 %value1, 70, !dbg !37
  br i1 %select.cmp.ge, label %select.case, label %select.next, !dbg !37

select.case:                                      ; preds = %entry
  store i64 1, ptr %select.result, align 4, !dbg !37
  br label %select.end, !dbg !37

select.next:                                      ; preds = %entry
  %select.cmp.eq = icmp eq i64 %value1, 75, !dbg !37
  br i1 %select.cmp.eq, label %select.case2, label %select.next3, !dbg !37

select.case2:                                     ; preds = %select.next
  store i64 2, ptr %select.result, align 4, !dbg !37
  br label %select.end, !dbg !37

select.next3:                                     ; preds = %select.next
  br label %select.end, !dbg !37

select.end:                                       ; preds = %select.next3, %select.case2, %select.case
  %select.result.load = load i64, ptr %select.result, align 4, !dbg !37
  store i64 %select.result.load, ptr %result, align 4, !dbg !37
  call void @llvm.dbg.declare(metadata ptr %result, metadata !34, metadata !DIExpression()), !dbg !38
  %result4 = load i64, ptr %result, align 4, !dbg !37
  ret i64 %result4, !dbg !37
}

define i64 @test_correct_ordering() !dbg !39 {
entry:
  %result = alloca i64, align 8
  %value = alloca i64, align 8
  store i64 85, ptr %value, align 4, !dbg !43
  call void @llvm.dbg.declare(metadata ptr %value, metadata !41, metadata !DIExpression()), !dbg !44
  %value1 = load i64, ptr %value, align 4, !dbg !45
  %select.result = alloca i64, align 8, !dbg !45
  %select.cmp.ge = icmp sge i64 %value1, 90, !dbg !45
  br i1 %select.cmp.ge, label %select.case, label %select.next, !dbg !45

select.case:                                      ; preds = %entry
  store i64 1, ptr %select.result, align 4, !dbg !45
  br label %select.end, !dbg !45

select.next:                                      ; preds = %entry
  %select.cmp.ge3 = icmp sge i64 %value1, 80, !dbg !45
  br i1 %select.cmp.ge3, label %select.case2, label %select.next4, !dbg !45

select.case2:                                     ; preds = %select.next
  store i64 2, ptr %select.result, align 4, !dbg !45
  br label %select.end, !dbg !45

select.next4:                                     ; preds = %select.next
  %select.cmp.ge6 = icmp sge i64 %value1, 70, !dbg !45
  br i1 %select.cmp.ge6, label %select.case5, label %select.next7, !dbg !45

select.case5:                                     ; preds = %select.next4
  store i64 3, ptr %select.result, align 4, !dbg !45
  br label %select.end, !dbg !45

select.next7:                                     ; preds = %select.next4
  store i64 4, ptr %select.result, align 4, !dbg !45
  br label %select.end, !dbg !45

select.end:                                       ; preds = %select.next7, %select.case5, %select.case2, %select.case
  %select.result.load = load i64, ptr %select.result, align 4, !dbg !45
  store i64 %select.result.load, ptr %result, align 4, !dbg !45
  call void @llvm.dbg.declare(metadata ptr %result, metadata !42, metadata !DIExpression()), !dbg !46
  %result8 = load i64, ptr %result, align 4, !dbg !45
  ret i64 %result8, !dbg !45
}

define i64 @main() !dbg !47 {
entry:
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !48
  store { ptr, i64 } { ptr @0, i64 37 }, ptr %serialize_temp, align 8, !dbg !48
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !48
  call void @__vyn_println(ptr %serialized_json), !dbg !48
  %calltmp = call i64 @test_wildcard_position(), !dbg !48
  %serialize_temp1 = alloca i64, align 8, !dbg !48
  store i64 %calltmp, ptr %serialize_temp1, align 4, !dbg !48
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.1), !dbg !48
  call void @__vyn_println(ptr %serialized_json2), !dbg !48
  %calltmp3 = call i64 @test_subsumption_gte(), !dbg !48
  %serialize_temp4 = alloca i64, align 8, !dbg !48
  store i64 %calltmp3, ptr %serialize_temp4, align 4, !dbg !48
  %serialized_json5 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp4, ptr @type_name.2), !dbg !48
  call void @__vyn_println(ptr %serialized_json5), !dbg !48
  %calltmp6 = call i64 @test_duplicate_patterns(), !dbg !48
  %serialize_temp7 = alloca i64, align 8, !dbg !48
  store i64 %calltmp6, ptr %serialize_temp7, align 4, !dbg !48
  %serialized_json8 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp7, ptr @type_name.3), !dbg !48
  call void @__vyn_println(ptr %serialized_json8), !dbg !48
  %calltmp9 = call i64 @test_range_overlap(), !dbg !48
  %serialize_temp10 = alloca i64, align 8, !dbg !48
  store i64 %calltmp9, ptr %serialize_temp10, align 4, !dbg !48
  %serialized_json11 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp10, ptr @type_name.4), !dbg !48
  call void @__vyn_println(ptr %serialized_json11), !dbg !48
  %calltmp12 = call i64 @test_correct_ordering(), !dbg !48
  %serialize_temp13 = alloca i64, align 8, !dbg !48
  store i64 %calltmp12, ptr %serialize_temp13, align 4, !dbg !48
  %serialized_json14 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp13, ptr @type_name.5), !dbg !48
  call void @__vyn_println(ptr %serialized_json14), !dbg !48
  ret i64 0, !dbg !48
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_unreachable_patterns.vyn.ll", directory: "/home/rick/Projects/Vyn/test/select_match")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_wildcard_position", linkageName: "test_wildcard_position", scope: !1, file: !1, line: 5, type: !5, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10}
!9 = !DILocalVariable(name: "value", scope: !4, file: !1, line: 6, type: !7)
!10 = !DILocalVariable(name: "result", scope: !4, file: !1, line: 7, type: !7)
!11 = !DILocation(line: 5, column: 1, scope: !4)
!12 = !DILocation(line: 6, column: 1, scope: !4)
!13 = !DILocation(line: 7, column: 31, scope: !4)
!14 = !DILocation(line: 7, column: 1, scope: !4)
!15 = distinct !DISubprogram(name: "test_subsumption_gte", linkageName: "test_subsumption_gte", scope: !1, file: !1, line: 15, type: !5, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!16 = !{!17, !18}
!17 = !DILocalVariable(name: "value", scope: !15, file: !1, line: 17, type: !7)
!18 = !DILocalVariable(name: "result", scope: !15, file: !1, line: 18, type: !7)
!19 = !DILocation(line: 15, column: 1, scope: !15)
!20 = !DILocation(line: 17, column: 1, scope: !15)
!21 = !DILocation(line: 18, column: 31, scope: !15)
!22 = !DILocation(line: 18, column: 1, scope: !15)
!23 = distinct !DISubprogram(name: "test_duplicate_patterns", linkageName: "test_duplicate_patterns", scope: !1, file: !1, line: 25, type: !5, scopeLine: 25, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !24)
!24 = !{!25, !26}
!25 = !DILocalVariable(name: "value", scope: !23, file: !1, line: 27, type: !7)
!26 = !DILocalVariable(name: "result", scope: !23, file: !1, line: 28, type: !7)
!27 = !DILocation(line: 25, column: 1, scope: !23)
!28 = !DILocation(line: 27, column: 1, scope: !23)
!29 = !DILocation(line: 28, column: 31, scope: !23)
!30 = !DILocation(line: 28, column: 1, scope: !23)
!31 = distinct !DISubprogram(name: "test_range_overlap", linkageName: "test_range_overlap", scope: !1, file: !1, line: 35, type: !5, scopeLine: 35, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !32)
!32 = !{!33, !34}
!33 = !DILocalVariable(name: "value", scope: !31, file: !1, line: 37, type: !7)
!34 = !DILocalVariable(name: "result", scope: !31, file: !1, line: 38, type: !7)
!35 = !DILocation(line: 35, column: 1, scope: !31)
!36 = !DILocation(line: 37, column: 1, scope: !31)
!37 = !DILocation(line: 38, column: 31, scope: !31)
!38 = !DILocation(line: 38, column: 1, scope: !31)
!39 = distinct !DISubprogram(name: "test_correct_ordering", linkageName: "test_correct_ordering", scope: !1, file: !1, line: 45, type: !5, scopeLine: 45, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !40)
!40 = !{!41, !42}
!41 = !DILocalVariable(name: "value", scope: !39, file: !1, line: 47, type: !7)
!42 = !DILocalVariable(name: "result", scope: !39, file: !1, line: 48, type: !7)
!43 = !DILocation(line: 45, column: 1, scope: !39)
!44 = !DILocation(line: 47, column: 1, scope: !39)
!45 = !DILocation(line: 48, column: 31, scope: !39)
!46 = !DILocation(line: 48, column: 1, scope: !39)
!47 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 57, type: !5, scopeLine: 57, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!48 = !DILocation(line: 57, column: 1, scope: !47)
