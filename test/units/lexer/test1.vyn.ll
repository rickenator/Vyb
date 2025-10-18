; ModuleID = 'VynModule'
source_filename = "VynModule"

define void @example() !dbg !4 {
entry:
  %x = alloca i64, align 8
  store i64 10, ptr %x, align 4, !dbg !10
  call void @llvm.dbg.declare(metadata ptr %x, metadata !8, metadata !DIExpression()), !dbg !11
  ret void, !dbg !10
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
!1 = !DIFile(filename: "test1.vyn.ll", directory: "/home/rick/Projects/Vyn/test/units/lexer")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "example", linkageName: "example", scope: !1, file: !1, line: 10, type: !5, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
!5 = !DISubroutineType(types: !6)
!6 = !{null}
!7 = !{!8}
!8 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 11, type: !9)
!9 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!10 = !DILocation(line: 10, column: 1, scope: !4)
!11 = !DILocation(line: 11, column: 5, scope: !4)
