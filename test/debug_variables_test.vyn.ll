; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [29 x i8] c"=== Debug Variables Test ===\00", align 1
@1 = private unnamed_addr constant [28 x i8] c"Sum calculated successfully\00", align 1

define i64 @add_numbers(i64 %a, i64 %b) !dbg !4 {
entry:
  %result = alloca i64, align 8
  %b2 = alloca i64, align 8
  %a1 = alloca i64, align 8
  store i64 %a, ptr %a1, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %a1, metadata !9, metadata !DIExpression()), !dbg !13
  store i64 %b, ptr %b2, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %b2, metadata !10, metadata !DIExpression()), !dbg !14
  %a3 = load i64, ptr %a1, align 4, !dbg !12
  %b4 = load i64, ptr %b2, align 4, !dbg !12
  %addtmp = add i64 %a3, %b4, !dbg !12
  store i64 %addtmp, ptr %result, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %result, metadata !11, metadata !DIExpression()), !dbg !15
  %result5 = load i64, ptr %result, align 4, !dbg !12
  ret i64 %result5, !dbg !12
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

define i64 @main() !dbg !16 {
entry:
  %sum = alloca i64, align 8, !dbg !23
  %y = alloca i64, align 8, !dbg !23
  %x = alloca i64, align 8, !dbg !23
  call void @__vyn_println(ptr @0), !dbg !23
  store i64 5, ptr %x, align 4, !dbg !23
  call void @llvm.dbg.declare(metadata ptr %x, metadata !20, metadata !DIExpression()), !dbg !24
  store i64 10, ptr %y, align 4, !dbg !23
  call void @llvm.dbg.declare(metadata ptr %y, metadata !21, metadata !DIExpression()), !dbg !25
  %x1 = load i64, ptr %x, align 4, !dbg !23
  %y2 = load i64, ptr %y, align 4, !dbg !23
  %calltmp = call i64 @add_numbers(i64 %x1, i64 %y2), !dbg !23
  store i64 %calltmp, ptr %sum, align 4, !dbg !23
  call void @llvm.dbg.declare(metadata ptr %sum, metadata !22, metadata !DIExpression()), !dbg !26
  call void @__vyn_println(ptr @1), !dbg !23
  ret i64 0, !dbg !23
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "debug_variables_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "add_numbers", linkageName: "add_numbers", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !7, !7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10, !11}
!9 = !DILocalVariable(name: "a", scope: !4, file: !1, line: 3, type: !7)
!10 = !DILocalVariable(name: "b", scope: !4, file: !1, line: 3, type: !7)
!11 = !DILocalVariable(name: "result", scope: !4, file: !1, line: 4, type: !7)
!12 = !DILocation(line: 3, column: 1, scope: !4)
!13 = !DILocation(line: 3, column: 14, scope: !4)
!14 = !DILocation(line: 3, column: 22, scope: !4)
!15 = !DILocation(line: 4, column: 1, scope: !4)
!16 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 8, type: !17, scopeLine: 8, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !19)
!17 = !DISubroutineType(types: !18)
!18 = !{!7}
!19 = !{!20, !21, !22}
!20 = !DILocalVariable(name: "x", scope: !16, file: !1, line: 10, type: !7)
!21 = !DILocalVariable(name: "y", scope: !16, file: !1, line: 11, type: !7)
!22 = !DILocalVariable(name: "sum", scope: !16, file: !1, line: 12, type: !7)
!23 = !DILocation(line: 8, column: 1, scope: !16)
!24 = !DILocation(line: 10, column: 1, scope: !16)
!25 = !DILocation(line: 11, column: 1, scope: !16)
!26 = !DILocation(line: 12, column: 1, scope: !16)
