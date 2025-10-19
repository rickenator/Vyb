; ModuleID = 'VynModule'
source_filename = "VynModule"

%IntBox = type { i64 }

define i64 @main() !dbg !4 {
entry:
  %s = alloca i64, align 8, !dbg !12
  %box = alloca %IntBox, align 8, !dbg !12
  %IntBox_obj = alloca %IntBox, align 8, !dbg !12
  %value_ptr = getelementptr inbounds %IntBox, ptr %IntBox_obj, i32 0, i32 0, !dbg !12
  store i64 42, ptr %value_ptr, align 4, !dbg !12
  %IntBox_val = load %IntBox, ptr %IntBox_obj, align 4, !dbg !12
  store %IntBox %IntBox_val, ptr %box, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %box, metadata !9, metadata !DIExpression()), !dbg !13
  %box.load = load %IntBox, ptr %box, align 4, !dbg !12
  %trait.method.result = call i64 @size(%IntBox %box.load), !dbg !12
  store i64 %trait.method.result, ptr %s, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %s, metadata !11, metadata !DIExpression()), !dbg !14
  %s1 = load i64, ptr %s, align 4, !dbg !12
  ret i64 %s1, !dbg !12
}

define i64 @size(%IntBox %self) !dbg !15 {
entry:
  %self1 = alloca %IntBox, align 8
  store %IntBox %self, ptr %self1, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !19, metadata !DIExpression()), !dbg !21
  ret i64 1, !dbg !20
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

define i1 @is_empty(%IntBox %self) !dbg !22 {
entry:
  %self1 = alloca %IntBox, align 8
  store %IntBox %self, ptr %self1, align 4, !dbg !28
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !27, metadata !DIExpression()), !dbg !29
  ret i1 false, !dbg !28
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_trait_concrete.vyn.ll", directory: "/home/rick/Projects/Vyn/test/aspect")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 24, type: !5, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11}
!9 = !DILocalVariable(name: "box", scope: !4, file: !1, line: 25, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "IntBox", scope: !1, file: !1, size: 64, align: 8)
!11 = !DILocalVariable(name: "s", scope: !4, file: !1, line: 28, type: !7)
!12 = !DILocation(line: 24, column: 1, scope: !4)
!13 = !DILocation(line: 25, column: 1, scope: !4)
!14 = !DILocation(line: 28, column: 1, scope: !4)
!15 = distinct !DISubprogram(name: "size", linkageName: "size", scope: !1, file: !1, line: 15, type: !16, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!16 = !DISubroutineType(types: !17)
!17 = !{!7, !10}
!18 = !{!19}
!19 = !DILocalVariable(name: "self", scope: !15, file: !1, line: 15, type: !10)
!20 = !DILocation(line: 15, column: 1, scope: !15)
!21 = !DILocation(line: 15, column: 14, scope: !15)
!22 = distinct !DISubprogram(name: "is_empty", linkageName: "is_empty", scope: !1, file: !1, line: 19, type: !23, scopeLine: 19, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !26)
!23 = !DISubroutineType(types: !24)
!24 = !{!25, !10}
!25 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!26 = !{!27}
!27 = !DILocalVariable(name: "self", scope: !22, file: !1, line: 19, type: !10)
!28 = !DILocation(line: 19, column: 1, scope: !22)
!29 = !DILocation(line: 19, column: 18, scope: !22)
