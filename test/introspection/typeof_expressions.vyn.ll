; ModuleID = 'VynModule'
source_filename = "VynModule"

@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str = private unnamed_addr constant [42 x i8] c"test/introspection/typeof_expressions.vyn\00", align 1
@0 = private unnamed_addr constant [29 x i8] c"typeof on expressions works!\00", align 1
@get_number.str = private unnamed_addr constant [11 x i8] c"get_number\00", align 1
@filepath.str.1 = private unnamed_addr constant [42 x i8] c"test/introspection/typeof_expressions.vyn\00", align 1

; Function Attrs: noinline
define i64 @main() #0 !dbg !4 {
entry:
  %x = alloca i64, align 8, !dbg !13
  %expr_match = alloca i1, align 1, !dbg !13
  %func_match = alloca i1, align 1, !dbg !13
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str, i32 3, i32 1), !dbg !13
  store i64 10, ptr %x, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %x, metadata !9, metadata !DIExpression()), !dbg !14
  store i1 true, ptr %expr_match, align 1, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %expr_match, metadata !10, metadata !DIExpression()), !dbg !15
  store i1 true, ptr %func_match, align 1, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %func_match, metadata !12, metadata !DIExpression()), !dbg !16
  call void @__vyn_println(ptr @0), !dbg !13
  call void @__vyn_runtime_pop_call_frame(), !dbg !13
  ret i64 0, !dbg !13
}

; Function Attrs: noinline
define i64 @get_number() #0 !dbg !17 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @get_number.str, ptr @filepath.str.1, i32 16, i32 1), !dbg !18
  call void @__vyn_runtime_pop_call_frame(), !dbg !18
  ret i64 42, !dbg !18
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @__vyn_println(ptr)

declare void @__vyn_runtime_pop_call_frame()

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noinline }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "typeof_expressions.vyn.ll", directory: "/home/rick/Projects/Vyn/test/introspection")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10, !12}
!9 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 4, type: !7)
!10 = !DILocalVariable(name: "expr_match", scope: !4, file: !1, line: 6, type: !11)
!11 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!12 = !DILocalVariable(name: "func_match", scope: !4, file: !1, line: 8, type: !11)
!13 = !DILocation(line: 3, column: 1, scope: !4)
!14 = !DILocation(line: 4, column: 5, scope: !4)
!15 = !DILocation(line: 6, column: 1, scope: !4)
!16 = !DILocation(line: 8, column: 5, scope: !4)
!17 = distinct !DISubprogram(name: "get_number", linkageName: "get_number", scope: !1, file: !1, line: 16, type: !5, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!18 = !DILocation(line: 16, column: 1, scope: !17)
