; ModuleID = 'VynModule'
source_filename = "VynModule"

@type_name = private unnamed_addr constant [4 x i8] c"Int\00", align 1
@0 = private unnamed_addr constant [14 x i8] c"Starting test\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [5 x i8] c"Done\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define void @test_naked() !dbg !4 {
entry:
  %result = alloca i64, align 8
  %x = alloca i64, align 8
  store i64 42, ptr %x, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %x, metadata !8, metadata !DIExpression()), !dbg !12
  %x1 = load i64, ptr %x, align 4, !dbg !13
  %select.result = alloca i64, align 8, !dbg !13
  %select.cmp = icmp eq i64 %x1, 1, !dbg !13
  br i1 %select.cmp, label %select.case, label %select.next, !dbg !13

select.case:                                      ; preds = %entry
  store i64 10, ptr %select.result, align 4, !dbg !13
  br label %select.end, !dbg !13

select.next:                                      ; preds = %entry
  %select.cmp3 = icmp eq i64 %x1, 42, !dbg !13
  br i1 %select.cmp3, label %select.case2, label %select.next4, !dbg !13

select.case2:                                     ; preds = %select.next
  store i64 420, ptr %select.result, align 4, !dbg !13
  br label %select.end, !dbg !13

select.next4:                                     ; preds = %select.next
  store i64 0, ptr %select.result, align 4, !dbg !13
  br label %select.end, !dbg !13

select.end:                                       ; preds = %select.next4, %select.case2, %select.case
  %select.result.load = load i64, ptr %select.result, align 4, !dbg !13
  store i64 %select.result.load, ptr %result, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %result, metadata !10, metadata !DIExpression()), !dbg !14
  %result5 = load i64, ptr %result, align 4, !dbg !13
  %serialize_temp = alloca i64, align 8, !dbg !13
  store i64 %result5, ptr %serialize_temp, align 4, !dbg !13
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !13
  call void @__vyn_println(ptr %serialized_json), !dbg !13
  ret void, !dbg !13
}

define void @main() !dbg !15 {
entry:
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !16
  store { ptr, i64 } { ptr @0, i64 13 }, ptr %serialize_temp, align 8, !dbg !16
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name.1), !dbg !16
  call void @__vyn_println(ptr %serialized_json), !dbg !16
  call void @test_naked(), !dbg !16
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !16
  store { ptr, i64 } { ptr @1, i64 4 }, ptr %serialize_temp1, align 8, !dbg !16
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.2), !dbg !16
  call void @__vyn_println(ptr %serialized_json2), !dbg !16
  ret void, !dbg !16
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
!1 = !DIFile(filename: "test_select_simple.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_naked", linkageName: "test_naked", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
!5 = !DISubroutineType(types: !6)
!6 = !{null}
!7 = !{!8, !10}
!8 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 4, type: !9)
!9 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!10 = !DILocalVariable(name: "result", scope: !4, file: !1, line: 5, type: !9)
!11 = !DILocation(line: 3, column: 1, scope: !4)
!12 = !DILocation(line: 4, column: 1, scope: !4)
!13 = !DILocation(line: 5, column: 25, scope: !4)
!14 = !DILocation(line: 5, column: 1, scope: !4)
!15 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 13, type: !5, scopeLine: 13, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!16 = !DILocation(line: 13, column: 1, scope: !15)
