; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @some_test_function() !dbg !4 {
entry:
  ret i64 100, !dbg !8
}

define i64 @another_function() !dbg !9 {
entry:
  ret i64 200, !dbg !10
}

define i64 @main() !dbg !11 {
entry:
  %calltmp = call i64 @some_test_function(), !dbg !12
  %calltmp1 = call i64 @another_function(), !dbg !12
  %addtmp = add i64 %calltmp, %calltmp1, !dbg !12
  ret i64 %addtmp, !dbg !12
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_snake_case.vyn.ll", directory: "/home/rick/Projects/Vyn/test/basic")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "some_test_function", linkageName: "some_test_function", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !DILocation(line: 3, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "another_function", linkageName: "another_function", scope: !1, file: !1, line: 7, type: !5, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!10 = !DILocation(line: 7, column: 1, scope: !9)
!11 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 11, type: !5, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!12 = !DILocation(line: 11, column: 1, scope: !11)
