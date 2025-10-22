; ModuleID = 'VynModule'
source_filename = "VynModule"

@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str = private unnamed_addr constant [36 x i8] c"test/introspection/test_println.vyn\00", align 1
@0 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@type_name = private unnamed_addr constant [7 x i8] c"string\00", align 1

; Function Attrs: noinline
define i64 @main() #0 !dbg !4 {
entry:
  %s = alloca { ptr, i64 }, align 8, !dbg !11
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str, i32 2, i32 1), !dbg !11
  store { ptr, i64 } { ptr @0, i64 11 }, ptr %s, align 8, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %s, metadata !9, metadata !DIExpression()), !dbg !12
  %s1 = load { ptr, i64 }, ptr %s, align 8, !dbg !11
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !11
  store { ptr, i64 } %s1, ptr %serialize_temp, align 8, !dbg !11
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !11
  call void @__vyn_println(ptr %serialized_json), !dbg !11
  call void @__vyn_runtime_pop_call_frame(), !dbg !11
  ret i64 0, !dbg !11
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
!1 = !DIFile(filename: "test_println.vyn.ll", directory: "/home/rick/Projects/Vyn/test/introspection")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DILocalVariable(name: "s", scope: !4, file: !1, line: 3, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!11 = !DILocation(line: 2, column: 1, scope: !4)
!12 = !DILocation(line: 3, column: 1, scope: !4)
