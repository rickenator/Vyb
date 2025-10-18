; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [9 x i8] c"John Doe\00", align 1

define { ptr, i64, double, i1 } @getEmployeeData() !dbg !4 {
entry:
  ret { ptr, i64, double, i1 } { ptr @0, i64 1001, double 7.500000e+04, i1 true }, !dbg !8
}

define { ptr, i64, double, i1 } @main() !dbg !9 {
entry:
  %calltmp = call { ptr, i64, double, i1 } @getEmployeeData(), !dbg !10
  ret { ptr, i64, double, i1 } %calltmp, !dbg !10
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "simple_employee_example.vyn.ll", directory: "/home/rick/Projects/Vyn/test/basic")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "getEmployeeData", linkageName: "getEmployeeData", scope: !1, file: !1, line: 7, type: !5, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 256, align: 8)
!8 = !DILocation(line: 7, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 11, type: !5, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!10 = !DILocation(line: 11, column: 1, scope: !9)
