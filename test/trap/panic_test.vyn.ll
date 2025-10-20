; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [25 x i8] c"Critical system failure!\00", align 1

define i64 @critical_failure(i1 %condition) !dbg !4 {
entry:
  %condition1 = alloca i1, align 1
  store i1 %condition, ptr %condition1, align 1, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %condition1, metadata !10, metadata !DIExpression()), !dbg !12
  %condition2 = load i1, ptr %condition1, align 1, !dbg !11
  br i1 %condition2, label %then, label %ifcont, !dbg !11

then:                                             ; preds = %entry
  br label %ifcont, !dbg !11

ifcont:                                           ; preds = %then, %entry
  ret i64 0, !dbg !11
}

define i64 @main() !dbg !13 {
entry:
  %result = alloca i64, align 8, !dbg !18
  %calltmp = call i64 @critical_failure(i1 true), !dbg !18
  store i64 %calltmp, ptr %result, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %result, metadata !17, metadata !DIExpression()), !dbg !19
  %result1 = load i64, ptr %result, align 4, !dbg !18
  ret i64 %result1, !dbg !18
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "panic_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "critical_failure", linkageName: "critical_failure", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !9)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!9 = !{!10}
!10 = !DILocalVariable(name: "condition", scope: !4, file: !1, line: 3, type: !8)
!11 = !DILocation(line: 3, column: 1, scope: !4)
!12 = !DILocation(line: 3, column: 27, scope: !4)
!13 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 10, type: !14, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!14 = !DISubroutineType(types: !15)
!15 = !{!7}
!16 = !{!17}
!17 = !DILocalVariable(name: "result", scope: !13, file: !1, line: 11, type: !7)
!18 = !DILocation(line: 10, column: 1, scope: !13)
!19 = !DILocation(line: 11, column: 1, scope: !13)
