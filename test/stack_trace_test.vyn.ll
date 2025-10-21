; ModuleID = 'VynModule'
source_filename = "VynModule"

@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str = private unnamed_addr constant [26 x i8] c"test/stack_trace_test.vyn\00", align 1
@0 = private unnamed_addr constant [26 x i8] c"Starting stack trace test\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [23 x i8] c"Error in main function\00", align 1

define { i64, ptr } @main() !dbg !4 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str, i32 5, i32 1), !dbg !8
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !8
  store { ptr, i64 } { ptr @0, i64 25 }, ptr %serialize_temp, align 8, !dbg !8
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !8
  call void @__vyn_println(ptr %serialized_json), !dbg !8
  %error.heap = call ptr @malloc(i64 24), !dbg !8
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !8
  store { ptr, i64 } { ptr @1, i64 22 }, ptr %error.data.ptr, align 8, !dbg !8
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.heap, 1, !dbg !8
  ret { i64, ptr } %error.ptr, !dbg !8
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @malloc(i64)

declare ptr @__vyn_convert_lit_string(ptr)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "stack_trace_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 5, type: !5, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DILocation(line: 5, column: 1, scope: !4)
