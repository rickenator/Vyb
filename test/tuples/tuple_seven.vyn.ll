; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [2 x i8] c"a\00", align 1
@1 = private unnamed_addr constant [2 x i8] c"b\00", align 1

define { i64, { ptr, i64 }, i1, i64, { ptr, i64 }, i1, i64 } @main() !dbg !4 {
entry:
  ret { i64, { ptr, i64 }, i1, i64, { ptr, i64 }, i1, i64 } { i64 1, { ptr, i64 } { ptr @0, i64 1 }, i1 true, i64 2, { ptr, i64 } { ptr @1, i64 1 }, i1 false, i64 3 }, !dbg !8
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "tuple_seven.vyn.ll", directory: "/home/rick/Projects/Vyn/test/tuples")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 448, align: 8)
!8 = !DILocation(line: 2, column: 1, scope: !4)
