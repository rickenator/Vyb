; ModuleID = 'VynModule'
source_filename = "VynModule"

@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str = private unnamed_addr constant [38 x i8] c"test/introspection/typename_print.vyn\00", align 1
@0 = private unnamed_addr constant [4 x i8] c"Int\00", align 1
@type_name = private unnamed_addr constant [7 x i8] c"string\00", align 1

; Function Attrs: noinline
define i64 @main() #0 !dbg !4 {
entry:
  %x = alloca i64, align 8, !dbg !12
  %name = alloca { ptr, i64 }, align 8, !dbg !12
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str, i32 2, i32 1), !dbg !12
  store i64 42, ptr %x, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %x, metadata !9, metadata !DIExpression()), !dbg !13
  store { ptr, i64 } { ptr @0, i64 3 }, ptr %name, align 8, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %name, metadata !10, metadata !DIExpression()), !dbg !14
  %name1 = load { ptr, i64 }, ptr %name, align 8, !dbg !12
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !12
  store { ptr, i64 } %name1, ptr %serialize_temp, align 8, !dbg !12
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !12
  call void @__vyn_println(ptr %serialized_json), !dbg !12
  call void @__vyn_runtime_pop_call_frame(), !dbg !12
  ret i64 0, !dbg !12
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare void @__vyn_runtime_pop_call_frame()

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noinline }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "typename_print.vyn.ll", directory: "/home/rick/Projects/Vyn/test/introspection")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10}
!9 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 3, type: !7)
!10 = !DILocalVariable(name: "name", scope: !4, file: !1, line: 4, type: !11)
!11 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!12 = !DILocation(line: 2, column: 1, scope: !4)
!13 = !DILocation(line: 3, column: 1, scope: !4)
!14 = !DILocation(line: 4, column: 1, scope: !4)
