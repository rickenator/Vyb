; ModuleID = 'VynModule'
source_filename = "VynModule"

@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str = private unnamed_addr constant [38 x i8] c"test/introspection/typename_basic.vyn\00", align 1
@0 = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"Int\00", align 1
@2 = private unnamed_addr constant [6 x i8] c"float\00", align 1
@3 = private unnamed_addr constant [7 x i8] c"string\00", align 1
@4 = private unnamed_addr constant [19 x i8] c"typename(x) works!\00", align 1

; Function Attrs: noinline
define i64 @main() #0 !dbg !4 {
entry:
  %x = alloca i64, align 8, !dbg !17
  %y = alloca double, align 8, !dbg !17
  %s = alloca { ptr, i64 }, align 8, !dbg !17
  %name_x = alloca { ptr, i64 }, align 8, !dbg !17
  %name_y = alloca { ptr, i64 }, align 8, !dbg !17
  %name_s = alloca { ptr, i64 }, align 8, !dbg !17
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str, i32 4, i32 1), !dbg !17
  store i64 42, ptr %x, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %x, metadata !9, metadata !DIExpression()), !dbg !18
  store double 3.140000e+00, ptr %y, align 8, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %y, metadata !10, metadata !DIExpression()), !dbg !19
  store { ptr, i64 } { ptr @0, i64 5 }, ptr %s, align 8, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %s, metadata !12, metadata !DIExpression()), !dbg !20
  store { ptr, i64 } { ptr @1, i64 3 }, ptr %name_x, align 8, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %name_x, metadata !14, metadata !DIExpression()), !dbg !21
  store { ptr, i64 } { ptr @2, i64 5 }, ptr %name_y, align 8, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %name_y, metadata !15, metadata !DIExpression()), !dbg !22
  store { ptr, i64 } { ptr @3, i64 6 }, ptr %name_s, align 8, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %name_s, metadata !16, metadata !DIExpression()), !dbg !23
  call void @__vyn_println(ptr @4), !dbg !17
  call void @__vyn_runtime_pop_call_frame(), !dbg !17
  ret i64 0, !dbg !17
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
!1 = !DIFile(filename: "typename_basic.vyn.ll", directory: "/home/rick/Projects/Vyn/test/introspection")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 4, type: !5, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10, !12, !14, !15, !16}
!9 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 5, type: !7)
!10 = !DILocalVariable(name: "y", scope: !4, file: !1, line: 6, type: !11)
!11 = !DIBasicType(name: "f64", size: 64, encoding: DW_ATE_float)
!12 = !DILocalVariable(name: "s", scope: !4, file: !1, line: 7, type: !13)
!13 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!14 = !DILocalVariable(name: "name_x", scope: !4, file: !1, line: 9, type: !13)
!15 = !DILocalVariable(name: "name_y", scope: !4, file: !1, line: 11, type: !13)
!16 = !DILocalVariable(name: "name_s", scope: !4, file: !1, line: 12, type: !13)
!17 = !DILocation(line: 4, column: 1, scope: !4)
!18 = !DILocation(line: 5, column: 1, scope: !4)
!19 = !DILocation(line: 6, column: 1, scope: !4)
!20 = !DILocation(line: 7, column: 1, scope: !4)
!21 = !DILocation(line: 9, column: 5, scope: !4)
!22 = !DILocation(line: 11, column: 1, scope: !4)
!23 = !DILocation(line: 12, column: 1, scope: !4)
