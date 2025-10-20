; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @low_level_operation() !dbg !4 {
entry:
  ret i64 0, !dbg !8
}

define i64 @high_level_operation() !dbg !9 {
entry:
  %calltmp = call i64 @low_level_operation(), !dbg !10
  ret i64 0, !dbg !10
}

define i64 @main() !dbg !11 {
entry:
  %calltmp = call i64 @high_level_operation(), !dbg !12
  ret i64 0, !dbg !12
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "09_rethrow_transform.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "low_level_operation", linkageName: "low_level_operation", scope: !1, file: !1, line: 4, type: !5, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !DILocation(line: 4, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "high_level_operation", linkageName: "high_level_operation", scope: !1, file: !1, line: 9, type: !5, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!10 = !DILocation(line: 9, column: 1, scope: !9)
!11 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 23, type: !5, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!12 = !DILocation(line: 23, column: 1, scope: !11)
