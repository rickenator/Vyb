; ModuleID = 'VynModule'
source_filename = "VynModule"

%Box_Int = type { i64 }

define %Box_Int @make_int_box(i64 %val) !dbg !4 {
entry:
  %val1 = alloca i64, align 8
  store i64 %val, ptr %val1, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %val1, metadata !10, metadata !DIExpression()), !dbg !12
  ret %Box_Int undef, !dbg !11
}

define i64 @main() !dbg !13 {
entry:
  %b = alloca %Box_Int, align 8, !dbg !18
  %calltmp = call %Box_Int @make_int_box(i64 42), !dbg !18
  store %Box_Int %calltmp, ptr %b, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %b, metadata !17, metadata !DIExpression()), !dbg !19
  ret i64 0, !dbg !18
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
!1 = !DIFile(filename: "test_monomorph_simple.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trait")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "make_int_box", linkageName: "make_int_box", scope: !1, file: !1, line: 9, type: !5, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !9)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "Box_Int", scope: !1, file: !1, size: 64, align: 8)
!8 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!9 = !{!10}
!10 = !DILocalVariable(name: "val", scope: !4, file: !1, line: 9, type: !8)
!11 = !DILocation(line: 9, column: 1, scope: !4)
!12 = !DILocation(line: 9, column: 17, scope: !4)
!13 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 14, type: !14, scopeLine: 14, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!14 = !DISubroutineType(types: !15)
!15 = !{!8}
!16 = !{!17}
!17 = !DILocalVariable(name: "b", scope: !13, file: !1, line: 16, type: !7)
!18 = !DILocation(line: 14, column: 1, scope: !13)
!19 = !DILocation(line: 16, column: 1, scope: !13)
