; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [5 x i8] c"zero\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"one\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [10 x i8] c"forty-two\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@3 = private unnamed_addr constant [6 x i8] c"other\00", align 1
@type_name.3 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@4 = private unnamed_addr constant [5 x i8] c"init\00", align 1
@type_name.4 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@5 = private unnamed_addr constant [8 x i8] c"running\00", align 1
@type_name.5 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@6 = private unnamed_addr constant [5 x i8] c"done\00", align 1
@type_name.6 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@7 = private unnamed_addr constant [15 x i8] c"unknown status\00", align 1
@type_name.7 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@8 = private unnamed_addr constant [25 x i8] c"Testing match statement:\00", align 1
@type_name.8 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define void @test_int_match() !dbg !4 {
entry:
  %x = alloca i64, align 8
  store i64 42, ptr %x, align 4, !dbg !10
  call void @llvm.dbg.declare(metadata ptr %x, metadata !8, metadata !DIExpression()), !dbg !11
  %x1 = load i64, ptr %x, align 4, !dbg !10
  br label %match.case.0, !dbg !10

match.case.0:                                     ; preds = %entry
  %match.icmp = icmp eq i64 %x1, 0, !dbg !10
  br i1 %match.icmp, label %match.case.body.0, label %match.case.1, !dbg !10

match.case.body.0:                                ; preds = %match.case.0
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @0, i64 4 }, ptr %serialize_temp, align 8, !dbg !10
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !10
  call void @__vyn_println(ptr %serialized_json), !dbg !10
  br label %match.end, !dbg !10

match.case.1:                                     ; preds = %match.case.0
  %match.icmp2 = icmp eq i64 %x1, 1, !dbg !10
  br i1 %match.icmp2, label %match.case.body.1, label %match.case.2, !dbg !10

match.case.body.1:                                ; preds = %match.case.1
  %serialize_temp3 = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @1, i64 3 }, ptr %serialize_temp3, align 8, !dbg !10
  %serialized_json4 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp3, ptr @type_name.1), !dbg !10
  call void @__vyn_println(ptr %serialized_json4), !dbg !10
  br label %match.end, !dbg !10

match.case.2:                                     ; preds = %match.case.1
  %match.icmp5 = icmp eq i64 %x1, 42, !dbg !10
  br i1 %match.icmp5, label %match.case.body.2, label %match.case.3, !dbg !10

match.case.body.2:                                ; preds = %match.case.2
  %serialize_temp6 = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @2, i64 9 }, ptr %serialize_temp6, align 8, !dbg !10
  %serialized_json7 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp6, ptr @type_name.2), !dbg !10
  call void @__vyn_println(ptr %serialized_json7), !dbg !10
  br label %match.end, !dbg !10

match.case.3:                                     ; preds = %match.case.2
  br label %match.case.body.3, !dbg !10

match.case.body.3:                                ; preds = %match.case.3
  %serialize_temp8 = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @3, i64 5 }, ptr %serialize_temp8, align 8, !dbg !10
  %serialized_json9 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp8, ptr @type_name.3), !dbg !10
  call void @__vyn_println(ptr %serialized_json9), !dbg !10
  br label %match.end, !dbg !10

match.default:                                    ; No predecessors!
  br label %match.end, !dbg !10

match.end:                                        ; preds = %match.default, %match.case.body.3, %match.case.body.2, %match.case.body.1, %match.case.body.0
  ret void, !dbg !10
}

define void @test_enum_like() !dbg !12 {
entry:
  %status = alloca i64, align 8
  store i64 1, ptr %status, align 4, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %status, metadata !14, metadata !DIExpression()), !dbg !16
  %status1 = load i64, ptr %status, align 4, !dbg !15
  br label %match.case.0, !dbg !15

match.case.0:                                     ; preds = %entry
  %match.icmp = icmp eq i64 %status1, 0, !dbg !15
  br i1 %match.icmp, label %match.case.body.0, label %match.case.1, !dbg !15

match.case.body.0:                                ; preds = %match.case.0
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !15
  store { ptr, i64 } { ptr @4, i64 4 }, ptr %serialize_temp, align 8, !dbg !15
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.4), !dbg !15
  call void @__vyn_println(ptr %serialized_json), !dbg !15
  br label %match.end, !dbg !15

match.case.1:                                     ; preds = %match.case.0
  %match.icmp2 = icmp eq i64 %status1, 1, !dbg !15
  br i1 %match.icmp2, label %match.case.body.1, label %match.case.2, !dbg !15

match.case.body.1:                                ; preds = %match.case.1
  %serialize_temp3 = alloca { ptr, i64 }, align 8, !dbg !15
  store { ptr, i64 } { ptr @5, i64 7 }, ptr %serialize_temp3, align 8, !dbg !15
  %serialized_json4 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp3, ptr @type_name.5), !dbg !15
  call void @__vyn_println(ptr %serialized_json4), !dbg !15
  br label %match.end, !dbg !15

match.case.2:                                     ; preds = %match.case.1
  %match.icmp5 = icmp eq i64 %status1, 2, !dbg !15
  br i1 %match.icmp5, label %match.case.body.2, label %match.case.3, !dbg !15

match.case.body.2:                                ; preds = %match.case.2
  %serialize_temp6 = alloca { ptr, i64 }, align 8, !dbg !15
  store { ptr, i64 } { ptr @6, i64 4 }, ptr %serialize_temp6, align 8, !dbg !15
  %serialized_json7 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp6, ptr @type_name.6), !dbg !15
  call void @__vyn_println(ptr %serialized_json7), !dbg !15
  br label %match.end, !dbg !15

match.case.3:                                     ; preds = %match.case.2
  br label %match.case.body.3, !dbg !15

match.case.body.3:                                ; preds = %match.case.3
  %serialize_temp8 = alloca { ptr, i64 }, align 8, !dbg !15
  store { ptr, i64 } { ptr @7, i64 14 }, ptr %serialize_temp8, align 8, !dbg !15
  %serialized_json9 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp8, ptr @type_name.7), !dbg !15
  call void @__vyn_println(ptr %serialized_json9), !dbg !15
  br label %match.end, !dbg !15

match.default:                                    ; No predecessors!
  br label %match.end, !dbg !15

match.end:                                        ; preds = %match.default, %match.case.body.3, %match.case.body.2, %match.case.body.1, %match.case.body.0
  ret void, !dbg !15
}

define void @main() !dbg !17 {
entry:
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !18
  store { ptr, i64 } { ptr @8, i64 24 }, ptr %serialize_temp, align 8, !dbg !18
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.8), !dbg !18
  call void @__vyn_println(ptr %serialized_json), !dbg !18
  call void @test_int_match(), !dbg !18
  call void @test_enum_like(), !dbg !18
  ret void, !dbg !18
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
!1 = !DIFile(filename: "test_match_basic.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_int_match", linkageName: "test_int_match", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
!5 = !DISubroutineType(types: !6)
!6 = !{null}
!7 = !{!8}
!8 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 4, type: !9)
!9 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!10 = !DILocation(line: 3, column: 1, scope: !4)
!11 = !DILocation(line: 4, column: 1, scope: !4)
!12 = distinct !DISubprogram(name: "test_enum_like", linkageName: "test_enum_like", scope: !1, file: !1, line: 14, type: !5, scopeLine: 14, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!13 = !{!14}
!14 = !DILocalVariable(name: "status", scope: !12, file: !1, line: 15, type: !9)
!15 = !DILocation(line: 14, column: 1, scope: !12)
!16 = !DILocation(line: 15, column: 1, scope: !12)
!17 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 25, type: !5, scopeLine: 25, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!18 = !DILocation(line: 25, column: 1, scope: !17)
