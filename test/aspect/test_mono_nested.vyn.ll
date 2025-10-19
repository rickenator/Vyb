; ModuleID = 'VynModule'
source_filename = "VynModule"

%Box_Int = type { i64 }

define i64 @main() !dbg !4 {
entry:
  %b = alloca %Box_Int, align 8, !dbg !11
  %Box_Int_obj = alloca %Box_Int, align 8, !dbg !11
  %value_ptr = getelementptr inbounds %Box_Int, ptr %Box_Int_obj, i32 0, i32 0, !dbg !11
  store i64 42, ptr %value_ptr, align 4, !dbg !11
  %Box_Int_val = load %Box_Int, ptr %Box_Int_obj, align 4, !dbg !11
  store %Box_Int %Box_Int_val, ptr %b, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %b, metadata !9, metadata !DIExpression()), !dbg !12
  ret i64 0, !dbg !11
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
!1 = !DIFile(filename: "test_mono_nested.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trait")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 13, type: !5, scopeLine: 13, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DILocalVariable(name: "b", scope: !4, file: !1, line: 15, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "Box_Int", scope: !1, file: !1, size: 64, align: 8)
!11 = !DILocation(line: 13, column: 1, scope: !4)
!12 = !DILocation(line: 15, column: 1, scope: !4)
