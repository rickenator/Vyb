; ModuleID = 'VynModule'
source_filename = "VynModule"

%SimplePoint = type { i64, i64 }

define %SimplePoint @main() !dbg !4 {
entry:
  %SimplePoint_obj = alloca %SimplePoint, align 8, !dbg !8
  %x_ptr = getelementptr inbounds %SimplePoint, ptr %SimplePoint_obj, i32 0, i32 0, !dbg !8
  store i64 10, ptr %x_ptr, align 4, !dbg !8
  %y_ptr = getelementptr inbounds %SimplePoint, ptr %SimplePoint_obj, i32 0, i32 1, !dbg !8
  store i64 20, ptr %y_ptr, align 4, !dbg !8
  %SimplePoint_val = load %SimplePoint, ptr %SimplePoint_obj, align 4, !dbg !8
  ret %SimplePoint %SimplePoint_val, !dbg !8
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_simple_angle.vyn.ll", directory: "/home/rick/Projects/Vyn/test/basic")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 7, type: !5, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "SimplePoint", scope: !1, file: !1, size: 128, align: 8)
!8 = !DILocation(line: 7, column: 1, scope: !4)
