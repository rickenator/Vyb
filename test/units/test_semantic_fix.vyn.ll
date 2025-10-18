; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %p = alloca ptr, align 8
  %x = alloca double, align 8
  store double 0.000000e+00, ptr %x, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %x, metadata !9, metadata !DIExpression()), !dbg !14
  %x1 = load double, ptr %x, align 8, !dbg !13
  %loc_alloca = alloca double, align 8, !dbg !13
  store double %x1, ptr %loc_alloca, align 8, !dbg !13
  store ptr %loc_alloca, ptr %p, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %p, metadata !11, metadata !DIExpression()), !dbg !15
  ret i64 0, !dbg !13
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
!1 = !DIFile(filename: "test_semantic_fix.vyn.ll", directory: "/home/rick/Projects/Vyn/test/units")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11}
!9 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 2, type: !10)
!10 = !DIBasicType(name: "f64", size: 64, encoding: DW_ATE_float)
!11 = !DILocalVariable(name: "p", scope: !4, file: !1, line: 4, type: !12)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!13 = !DILocation(line: 1, column: 1, scope: !4)
!14 = !DILocation(line: 2, column: 1, scope: !4)
!15 = !DILocation(line: 4, column: 1, scope: !4)
