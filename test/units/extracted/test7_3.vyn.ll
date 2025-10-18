; ModuleID = 'VynModule'
source_filename = "VynModule"

declare void @bar()

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test7_3.vyn.ll", directory: "/home/rick/Projects/Vyn/test/units/extracted")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
