; ModuleID = 'VynModule'
source_filename = "VynModule"

@type_name = private unnamed_addr constant [4 x i8] c"Int\00", align 1
@type_name.1 = private unnamed_addr constant [4 x i8] c"Int\00", align 1
@0 = private unnamed_addr constant [14 x i8] c"Test starting\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [10 x i8] c"Test done\00", align 1
@type_name.3 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define void @test_blocks_with_pass() !dbg !4 {
entry:
  %result = alloca i64, align 8
  %msg = alloca i64, align 8
  %x = alloca i64, align 8
  %temp3 = alloca i64, align 8
  %temp = alloca i64, align 8
  %n = alloca i64, align 8
  store i64 3, ptr %n, align 4, !dbg !14
  call void @llvm.dbg.declare(metadata ptr %n, metadata !8, metadata !DIExpression()), !dbg !15
  %n1 = load i64, ptr %n, align 4, !dbg !16
  %select.result = alloca i64, align 8, !dbg !16
  %select.cmp = icmp eq i64 %n1, 1, !dbg !16
  br i1 %select.cmp, label %select.case, label %select.next, !dbg !16

select.case:                                      ; preds = %entry
  store i64 10, ptr %temp3, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %temp3, metadata !10, metadata !DIExpression()), !dbg !17
  %temp4 = load i64, ptr %temp3, align 4, !dbg !16
  store i64 %temp4, ptr %select.result, align 4, !dbg !16
  br label %select.end, !dbg !16

select.next:                                      ; preds = %entry
  %select.cmp6 = icmp eq i64 %n1, 2, !dbg !16
  br i1 %select.cmp6, label %select.case5, label %select.next8, !dbg !16

select.case5:                                     ; preds = %select.next
  store i64 20, ptr %x, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %x, metadata !11, metadata !DIExpression()), !dbg !18
  %x7 = load i64, ptr %x, align 4, !dbg !16
  store i64 %x7, ptr %select.result, align 4, !dbg !16
  br label %select.end, !dbg !16

select.next8:                                     ; preds = %select.next
  %select.cmp10 = icmp eq i64 %n1, 3, !dbg !16
  br i1 %select.cmp10, label %select.case9, label %select.next13, !dbg !16

select.case9:                                     ; preds = %select.next8
  store i64 300, ptr %msg, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !12, metadata !DIExpression()), !dbg !19
  %msg11 = load i64, ptr %msg, align 4, !dbg !16
  %serialize_temp = alloca i64, align 8, !dbg !16
  store i64 %msg11, ptr %serialize_temp, align 4, !dbg !16
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !16
  call void @__vyn_println(ptr %serialized_json), !dbg !16
  %msg12 = load i64, ptr %msg, align 4, !dbg !16
  store i64 %msg12, ptr %select.result, align 4, !dbg !16
  br label %select.end, !dbg !16

select.next13:                                    ; preds = %select.next8
  store i64 0, ptr %select.result, align 4, !dbg !16
  br label %select.end, !dbg !16

select.end:                                       ; preds = %select.next13, %select.case9, %select.case5, %select.case
  %select.result.load = load i64, ptr %select.result, align 4, !dbg !16
  store i64 %select.result.load, ptr %result, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %result, metadata !13, metadata !DIExpression()), !dbg !20
  %result14 = load i64, ptr %result, align 4, !dbg !16
  %serialize_temp15 = alloca i64, align 8, !dbg !16
  store i64 %result14, ptr %serialize_temp15, align 4, !dbg !16
  %serialized_json16 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp15, ptr @type_name.1), !dbg !16
  call void @__vyn_println(ptr %serialized_json16), !dbg !16
  ret void, !dbg !16
}

define void @main() !dbg !21 {
entry:
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !22
  store { ptr, i64 } { ptr @0, i64 13 }, ptr %serialize_temp, align 8, !dbg !22
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.2), !dbg !22
  call void @__vyn_println(ptr %serialized_json), !dbg !22
  call void @test_blocks_with_pass(), !dbg !22
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !22
  store { ptr, i64 } { ptr @1, i64 9 }, ptr %serialize_temp1, align 8, !dbg !22
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.3), !dbg !22
  call void @__vyn_println(ptr %serialized_json2), !dbg !22
  ret void, !dbg !22
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
!1 = !DIFile(filename: "test_select_pass.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_blocks_with_pass", linkageName: "test_blocks_with_pass", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
!5 = !DISubroutineType(types: !6)
!6 = !{null}
!7 = !{!8, !10, !10, !11, !12, !13}
!8 = !DILocalVariable(name: "n", scope: !4, file: !1, line: 4, type: !9)
!9 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!10 = !DILocalVariable(name: "temp", scope: !4, file: !1, line: 7, type: !9)
!11 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 11, type: !9)
!12 = !DILocalVariable(name: "msg", scope: !4, file: !1, line: 15, type: !9)
!13 = !DILocalVariable(name: "result", scope: !4, file: !1, line: 5, type: !9)
!14 = !DILocation(line: 3, column: 1, scope: !4)
!15 = !DILocation(line: 4, column: 1, scope: !4)
!16 = !DILocation(line: 5, column: 25, scope: !4)
!17 = !DILocation(line: 7, column: 1, scope: !4)
!18 = !DILocation(line: 11, column: 1, scope: !4)
!19 = !DILocation(line: 15, column: 1, scope: !4)
!20 = !DILocation(line: 5, column: 1, scope: !4)
!21 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 26, type: !5, scopeLine: 26, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!22 = !DILocation(line: 26, column: 1, scope: !21)
