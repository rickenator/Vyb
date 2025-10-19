; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [40 x i8] c"Testing comparison patterns with score:\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@type_name.1 = private unnamed_addr constant [4 x i8] c"Int\00", align 1
@1 = private unnamed_addr constant [8 x i8] c"Result:\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@type_name.3 = private unnamed_addr constant [4 x i8] c"Int\00", align 1

define i64 @main() !dbg !4 {
entry:
  %result = alloca i64, align 8
  %score = alloca i64, align 8
  store i64 85, ptr %score, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %score, metadata !9, metadata !DIExpression()), !dbg !12
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !11
  store { ptr, i64 } { ptr @0, i64 39 }, ptr %serialize_temp, align 8, !dbg !11
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !11
  call void @__vyn_println(ptr %serialized_json), !dbg !11
  %score1 = load i64, ptr %score, align 4, !dbg !11
  %serialize_temp2 = alloca i64, align 8, !dbg !11
  store i64 %score1, ptr %serialize_temp2, align 4, !dbg !11
  %serialized_json3 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp2, ptr @type_name.1), !dbg !11
  call void @__vyn_println(ptr %serialized_json3), !dbg !11
  %score4 = load i64, ptr %score, align 4, !dbg !13
  %select.result = alloca i64, align 8, !dbg !13
  %select.cmp.ge = icmp sge i64 %score4, 90, !dbg !13
  br i1 %select.cmp.ge, label %select.case, label %select.next, !dbg !13

select.case:                                      ; preds = %entry
  store i64 1, ptr %select.result, align 4, !dbg !13
  br label %select.end, !dbg !13

select.next:                                      ; preds = %entry
  %select.cmp.ge6 = icmp sge i64 %score4, 80, !dbg !13
  br i1 %select.cmp.ge6, label %select.case5, label %select.next7, !dbg !13

select.case5:                                     ; preds = %select.next
  store i64 6, ptr %select.result, align 4, !dbg !13
  br label %select.end, !dbg !13

select.next7:                                     ; preds = %select.next
  %select.cmp.lt = icmp slt i64 %score4, 60, !dbg !13
  br i1 %select.cmp.lt, label %select.case8, label %select.next9, !dbg !13

select.case8:                                     ; preds = %select.next7
  store i64 10, ptr %select.result, align 4, !dbg !13
  br label %select.end, !dbg !13

select.next9:                                     ; preds = %select.next7
  store i64 99, ptr %select.result, align 4, !dbg !13
  br label %select.end, !dbg !13

select.end:                                       ; preds = %select.next9, %select.case8, %select.case5, %select.case
  %select.result.load = load i64, ptr %select.result, align 4, !dbg !13
  store i64 %select.result.load, ptr %result, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %result, metadata !10, metadata !DIExpression()), !dbg !14
  %serialize_temp10 = alloca { ptr, i64 }, align 8, !dbg !13
  store { ptr, i64 } { ptr @1, i64 7 }, ptr %serialize_temp10, align 8, !dbg !13
  %serialized_json11 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp10, ptr @type_name.2), !dbg !13
  call void @__vyn_println(ptr %serialized_json11), !dbg !13
  %result12 = load i64, ptr %result, align 4, !dbg !13
  %serialize_temp13 = alloca i64, align 8, !dbg !13
  store i64 %result12, ptr %serialize_temp13, align 4, !dbg !13
  %serialized_json14 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp13, ptr @type_name.3), !dbg !13
  call void @__vyn_println(ptr %serialized_json14), !dbg !13
  %result15 = load i64, ptr %result, align 4, !dbg !13
  ret i64 %result15, !dbg !13
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
!1 = !DIFile(filename: "test_comparison_simple.vyn.ll", directory: "/home/rick/Projects/Vyn/test/select_match")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10}
!9 = !DILocalVariable(name: "score", scope: !4, file: !1, line: 4, type: !7)
!10 = !DILocalVariable(name: "result", scope: !4, file: !1, line: 9, type: !7)
!11 = !DILocation(line: 3, column: 1, scope: !4)
!12 = !DILocation(line: 4, column: 1, scope: !4)
!13 = !DILocation(line: 9, column: 31, scope: !4)
!14 = !DILocation(line: 9, column: 1, scope: !4)
