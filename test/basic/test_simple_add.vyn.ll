; ModuleID = 'VynModule'
source_filename = "VynModule"

define i32 @main() !dbg !4 {
entry:
  %result = alloca i32, align 4
  %b = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 10, ptr %a, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %a, metadata !9, metadata !DIExpression()), !dbg !13
  store i32 20, ptr %b, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %b, metadata !10, metadata !DIExpression()), !dbg !14
  %a1 = load i32, ptr %a, align 4, !dbg !12
  %b2 = load i32, ptr %b, align 4, !dbg !12
  %addtmp = add i32 %a1, %b2, !dbg !12
  store i32 %addtmp, ptr %result, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %result, metadata !11, metadata !DIExpression()), !dbg !15
  %result3 = load i32, ptr %result, align 4, !dbg !12
  ret i32 %result3, !dbg !12
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
!1 = !DIFile(filename: "test_simple_add.vyn.ll", directory: "/home/rick/Projects/Vyn/test/basic")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!8 = !{!9, !10, !11}
!9 = !DILocalVariable(name: "a", scope: !4, file: !1, line: 2, type: !7)
!10 = !DILocalVariable(name: "b", scope: !4, file: !1, line: 3, type: !7)
!11 = !DILocalVariable(name: "result", scope: !4, file: !1, line: 4, type: !7)
!12 = !DILocation(line: 1, column: 1, scope: !4)
!13 = !DILocation(line: 2, column: 1, scope: !4)
!14 = !DILocation(line: 3, column: 1, scope: !4)
!15 = !DILocation(line: 4, column: 1, scope: !4)
