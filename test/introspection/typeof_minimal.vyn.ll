; ModuleID = 'VynModule'
source_filename = "VynModule"

@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str = private unnamed_addr constant [38 x i8] c"test/introspection/typeof_minimal.vyn\00", align 1

; Function Attrs: noinline
define i64 @main() #0 !dbg !4 {
entry:
  %x = alloca i64, align 8, !dbg !13
  %y = alloca i64, align 8, !dbg !13
  %same = alloca i1, align 1, !dbg !13
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str, i32 2, i32 1), !dbg !13
  store i64 42, ptr %x, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %x, metadata !9, metadata !DIExpression()), !dbg !14
  store i64 100, ptr %y, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %y, metadata !10, metadata !DIExpression()), !dbg !15
  store i1 true, ptr %same, align 1, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %same, metadata !11, metadata !DIExpression()), !dbg !16
  call void @__vyn_runtime_pop_call_frame(), !dbg !13
  ret i64 0, !dbg !13
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @__vyn_runtime_pop_call_frame()

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noinline }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "typeof_minimal.vyn.ll", directory: "/home/runner/work/Vyn/Vyn/test/introspection")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10, !11}
!9 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 3, type: !7)
!10 = !DILocalVariable(name: "y", scope: !4, file: !1, line: 4, type: !7)
!11 = !DILocalVariable(name: "same", scope: !4, file: !1, line: 6, type: !12)
!12 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!13 = !DILocation(line: 2, column: 1, scope: !4)
!14 = !DILocation(line: 3, column: 1, scope: !4)
!15 = !DILocation(line: 4, column: 1, scope: !4)
!16 = !DILocation(line: 6, column: 5, scope: !4)
