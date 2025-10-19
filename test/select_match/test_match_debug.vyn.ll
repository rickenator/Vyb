; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [10 x i8] c"matched 1\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [10 x i8] c"matched 2\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [10 x i8] c"matched 3\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@3 = private unnamed_addr constant [8 x i8] c"Test 1:\00", align 1
@type_name.3 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@4 = private unnamed_addr constant [8 x i8] c"Test 2:\00", align 1
@type_name.4 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@5 = private unnamed_addr constant [8 x i8] c"Test 3:\00", align 1
@type_name.5 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define void @test_one() !dbg !4 {
entry:
  %x = alloca i64, align 8
  store i64 1, ptr %x, align 4, !dbg !10
  call void @llvm.dbg.declare(metadata ptr %x, metadata !8, metadata !DIExpression()), !dbg !11
  %x1 = load i64, ptr %x, align 4, !dbg !10
  br label %match.case.0, !dbg !10

match.case.0:                                     ; preds = %entry
  %match.icmp = icmp eq i64 %x1, 1, !dbg !10
  br i1 %match.icmp, label %match.case.body.0, label %match.default, !dbg !10

match.case.body.0:                                ; preds = %match.case.0
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @0, i64 9 }, ptr %serialize_temp, align 8, !dbg !10
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !10
  call void @__vyn_println(ptr %serialized_json), !dbg !10
  br label %match.end, !dbg !10

match.default:                                    ; preds = %match.case.0
  br label %match.end, !dbg !10

match.end:                                        ; preds = %match.default, %match.case.body.0
  ret void, !dbg !10
}

define void @test_two() !dbg !12 {
entry:
  %x = alloca i64, align 8
  store i64 2, ptr %x, align 4, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %x, metadata !14, metadata !DIExpression()), !dbg !16
  %x1 = load i64, ptr %x, align 4, !dbg !15
  br label %match.case.0, !dbg !15

match.case.0:                                     ; preds = %entry
  %match.icmp = icmp eq i64 %x1, 2, !dbg !15
  br i1 %match.icmp, label %match.case.body.0, label %match.default, !dbg !15

match.case.body.0:                                ; preds = %match.case.0
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !15
  store { ptr, i64 } { ptr @1, i64 9 }, ptr %serialize_temp, align 8, !dbg !15
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.1), !dbg !15
  call void @__vyn_println(ptr %serialized_json), !dbg !15
  br label %match.end, !dbg !15

match.default:                                    ; preds = %match.case.0
  br label %match.end, !dbg !15

match.end:                                        ; preds = %match.default, %match.case.body.0
  ret void, !dbg !15
}

define void @test_three() !dbg !17 {
entry:
  %x = alloca i64, align 8
  store i64 3, ptr %x, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %x, metadata !19, metadata !DIExpression()), !dbg !21
  %x1 = load i64, ptr %x, align 4, !dbg !20
  br label %match.case.0, !dbg !20

match.case.0:                                     ; preds = %entry
  %match.icmp = icmp eq i64 %x1, 3, !dbg !20
  br i1 %match.icmp, label %match.case.body.0, label %match.default, !dbg !20

match.case.body.0:                                ; preds = %match.case.0
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !20
  store { ptr, i64 } { ptr @2, i64 9 }, ptr %serialize_temp, align 8, !dbg !20
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.2), !dbg !20
  call void @__vyn_println(ptr %serialized_json), !dbg !20
  br label %match.end, !dbg !20

match.default:                                    ; preds = %match.case.0
  br label %match.end, !dbg !20

match.end:                                        ; preds = %match.default, %match.case.body.0
  ret void, !dbg !20
}

define void @main() !dbg !22 {
entry:
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !23
  store { ptr, i64 } { ptr @3, i64 7 }, ptr %serialize_temp, align 8, !dbg !23
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.3), !dbg !23
  call void @__vyn_println(ptr %serialized_json), !dbg !23
  call void @test_one(), !dbg !23
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !23
  store { ptr, i64 } { ptr @4, i64 7 }, ptr %serialize_temp1, align 8, !dbg !23
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.4), !dbg !23
  call void @__vyn_println(ptr %serialized_json2), !dbg !23
  call void @test_two(), !dbg !23
  %serialize_temp3 = alloca { ptr, i64 }, align 8, !dbg !23
  store { ptr, i64 } { ptr @5, i64 7 }, ptr %serialize_temp3, align 8, !dbg !23
  %serialized_json4 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp3, ptr @type_name.5), !dbg !23
  call void @__vyn_println(ptr %serialized_json4), !dbg !23
  call void @test_three(), !dbg !23
  ret void, !dbg !23
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
!1 = !DIFile(filename: "test_match_debug.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_one", linkageName: "test_one", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
!5 = !DISubroutineType(types: !6)
!6 = !{null}
!7 = !{!8}
!8 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 4, type: !9)
!9 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!10 = !DILocation(line: 3, column: 1, scope: !4)
!11 = !DILocation(line: 4, column: 1, scope: !4)
!12 = distinct !DISubprogram(name: "test_two", linkageName: "test_two", scope: !1, file: !1, line: 10, type: !5, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!13 = !{!14}
!14 = !DILocalVariable(name: "x", scope: !12, file: !1, line: 11, type: !9)
!15 = !DILocation(line: 10, column: 1, scope: !12)
!16 = !DILocation(line: 11, column: 1, scope: !12)
!17 = distinct !DISubprogram(name: "test_three", linkageName: "test_three", scope: !1, file: !1, line: 17, type: !5, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!18 = !{!19}
!19 = !DILocalVariable(name: "x", scope: !17, file: !1, line: 18, type: !9)
!20 = !DILocation(line: 17, column: 1, scope: !17)
!21 = !DILocation(line: 18, column: 1, scope: !17)
!22 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 24, type: !5, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!23 = !DILocation(line: 24, column: 1, scope: !22)
