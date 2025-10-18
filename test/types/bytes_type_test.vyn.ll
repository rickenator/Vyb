; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @test_bytes_concept() !dbg !4 {
entry:
  ret i64 42, !dbg !8
}

define i64 @main() !dbg !9 {
entry:
  %result = alloca i64, align 8, !dbg !12
  %calltmp = call i64 @test_bytes_concept(), !dbg !12
  store i64 %calltmp, ptr %result, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %result, metadata !11, metadata !DIExpression()), !dbg !13
  %result1 = load i64, ptr %result, align 4, !dbg !12
  ret i64 %result1, !dbg !12
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
!1 = !DIFile(filename: "bytes_type_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/types")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_bytes_concept", linkageName: "test_bytes_concept", scope: !1, file: !1, line: 6, type: !5, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !DILocation(line: 6, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 16, type: !5, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !10)
!10 = !{!11}
!11 = !DILocalVariable(name: "result", scope: !9, file: !1, line: 17, type: !7)
!12 = !DILocation(line: 16, column: 1, scope: !9)
!13 = !DILocation(line: 17, column: 1, scope: !9)
