; ModuleID = 'VynModule'
source_filename = "VynModule"

%Counter = type { i64 }

define i64 @main() !dbg !4 {
entry:
  %value = alloca i64, align 8, !dbg !13
  %result = alloca i64, align 8, !dbg !13
  %c = alloca %Counter, align 8, !dbg !13
  %Counter_obj = alloca %Counter, align 8, !dbg !13
  %count_ptr = getelementptr inbounds %Counter, ptr %Counter_obj, i32 0, i32 0, !dbg !13
  store i64 10, ptr %count_ptr, align 4, !dbg !13
  %Counter_val = load %Counter, ptr %Counter_obj, align 4, !dbg !13
  store %Counter %Counter_val, ptr %c, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %c, metadata !9, metadata !DIExpression()), !dbg !14
  %c.load = load %Counter, ptr %c, align 4, !dbg !13
  %trait.method.result = call i64 @add(%Counter %c.load, i64 5), !dbg !13
  store i64 %trait.method.result, ptr %result, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %result, metadata !11, metadata !DIExpression()), !dbg !15
  %c.load1 = load %Counter, ptr %c, align 4, !dbg !13
  %trait.method.result2 = call i64 @get_value(%Counter %c.load1), !dbg !13
  store i64 %trait.method.result2, ptr %value, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %value, metadata !12, metadata !DIExpression()), !dbg !16
  %result3 = load i64, ptr %result, align 4, !dbg !13
  %value4 = load i64, ptr %value, align 4, !dbg !13
  %addtmp = add i64 %result3, %value4, !dbg !13
  ret i64 %addtmp, !dbg !13
}

define i64 @add(%Counter %self, i64 %value) !dbg !17 {
entry:
  %value2 = alloca i64, align 8
  %self1 = alloca %Counter, align 8
  store %Counter %self, ptr %self1, align 4, !dbg !23
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !21, metadata !DIExpression()), !dbg !24
  store i64 %value, ptr %value2, align 4, !dbg !23
  call void @llvm.dbg.declare(metadata ptr %value2, metadata !22, metadata !DIExpression()), !dbg !25
  %self3 = load %Counter, ptr %self1, align 4, !dbg !23
  %temp_struct = alloca %Counter, align 8, !dbg !23
  store %Counter %self3, ptr %temp_struct, align 4, !dbg !23
  %count_ptr = getelementptr inbounds %Counter, ptr %temp_struct, i32 0, i32 0, !dbg !23
  %count_val = load i64, ptr %count_ptr, align 4, !dbg !23
  %value4 = load i64, ptr %value2, align 4, !dbg !23
  %addtmp = add i64 %count_val, %value4, !dbg !23
  ret i64 %addtmp, !dbg !23
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

define i64 @get_value(%Counter %self) !dbg !26 {
entry:
  %self1 = alloca %Counter, align 8
  store %Counter %self, ptr %self1, align 4, !dbg !31
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !30, metadata !DIExpression()), !dbg !32
  %self2 = load %Counter, ptr %self1, align 4, !dbg !31
  %temp_struct = alloca %Counter, align 8, !dbg !31
  store %Counter %self2, ptr %temp_struct, align 4, !dbg !31
  %count_ptr = getelementptr inbounds %Counter, ptr %temp_struct, i32 0, i32 0, !dbg !31
  %count_val = load i64, ptr %count_ptr, align 4, !dbg !31
  ret i64 %count_val, !dbg !31
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_trait_simple.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trait")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 23, type: !5, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12}
!9 = !DILocalVariable(name: "c", scope: !4, file: !1, line: 24, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "Counter", scope: !1, file: !1, size: 64, align: 8)
!11 = !DILocalVariable(name: "result", scope: !4, file: !1, line: 26, type: !7)
!12 = !DILocalVariable(name: "value", scope: !4, file: !1, line: 28, type: !7)
!13 = !DILocation(line: 23, column: 1, scope: !4)
!14 = !DILocation(line: 24, column: 1, scope: !4)
!15 = !DILocation(line: 26, column: 5, scope: !4)
!16 = !DILocation(line: 28, column: 1, scope: !4)
!17 = distinct !DISubprogram(name: "add", linkageName: "add", scope: !1, file: !1, line: 14, type: !18, scopeLine: 14, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !20)
!18 = !DISubroutineType(types: !19)
!19 = !{!7, !10, !7}
!20 = !{!21, !22}
!21 = !DILocalVariable(name: "self", scope: !17, file: !1, line: 14, type: !10)
!22 = !DILocalVariable(name: "value", scope: !17, file: !1, line: 14, type: !7)
!23 = !DILocation(line: 14, column: 1, scope: !17)
!24 = !DILocation(line: 14, column: 13, scope: !17)
!25 = !DILocation(line: 14, column: 29, scope: !17)
!26 = distinct !DISubprogram(name: "get_value", linkageName: "get_value", scope: !1, file: !1, line: 18, type: !27, scopeLine: 18, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !29)
!27 = !DISubroutineType(types: !28)
!28 = !{!7, !10}
!29 = !{!30}
!30 = !DILocalVariable(name: "self", scope: !26, file: !1, line: 18, type: !10)
!31 = !DILocation(line: 18, column: 1, scope: !26)
!32 = !DILocation(line: 18, column: 19, scope: !26)
