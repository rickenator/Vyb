; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [14 x i8] c"Hello, World!\00", align 1

define { i64, ptr } @get_values() !dbg !4 {
entry:
  ret { i64, ptr } { i64 42, ptr @0 }, !dbg !8
}

define { i64, ptr } @main() !dbg !9 {
entry:
  %calltmp = call { i64, ptr } @get_values(), !dbg !10
  ret { i64, ptr } %calltmp, !dbg !10
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "direct_return.vyn.ll", directory: "/home/rick/Projects/Vyn/test/basic")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "get_values", linkageName: "get_values", scope: !1, file: !1, line: 8, type: !5, scopeLine: 8, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DILocation(line: 8, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 12, type: !5, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!10 = !DILocation(line: 12, column: 1, scope: !9)
