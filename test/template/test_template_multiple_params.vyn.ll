; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %result = alloca i64, align 8
  store i64 100, ptr %result, align 4, !dbg !10
  call void @llvm.dbg.declare(metadata ptr %result, metadata !9, metadata !DIExpression()), !dbg !11
  %result1 = load i64, ptr %result, align 4, !dbg !10
  ret i64 %result1, !dbg !10
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
!1 = !DIFile(filename: "test_template_multiple_params.vyn.ll", directory: "/home/rick/Projects/Vyn/test/template")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 17, type: !5, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DILocalVariable(name: "result", scope: !4, file: !1, line: 18, type: !7)
!10 = !DILocation(line: 17, column: 1, scope: !4)
!11 = !DILocation(line: 18, column: 5, scope: !4)
