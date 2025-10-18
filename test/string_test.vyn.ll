; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [25 x i8] c"Starting String tests...\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@2 = private unnamed_addr constant [26 x i8] c"Created string from bytes\00", align 1
@3 = private unnamed_addr constant [16 x i8] c"String length: \00", align 1
@4 = private unnamed_addr constant [23 x i8] c"String tests completed\00", align 1

define i64 @main() !dbg !4 {
entry:
  %len = alloca i64, align 8, !dbg !12
  %msg = alloca { ptr, i64 }, align 8, !dbg !12
  call void @__vyn_println(ptr @0), !dbg !12
  store { ptr, i64 } { ptr @1, i64 5 }, ptr %msg, align 8, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !9, metadata !DIExpression()), !dbg !13
  call void @__vyn_println(ptr @2), !dbg !12
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !12
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !12
  store i64 %str.len, ptr %len, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %len, metadata !11, metadata !DIExpression()), !dbg !14
  %len1 = load i64, ptr %len, align 4, !dbg !12
  %tostring = call ptr @__vyn_toString_int(i64 %len1), !dbg !12
  %strcattmp = call ptr @__vyn_string_concat(ptr @3, ptr %tostring), !dbg !12
  call void @__vyn_println(ptr %strcattmp), !dbg !12
  call void @__vyn_println(ptr @4), !dbg !12
  ret i64 0, !dbg !12
}

declare void @__vyn_println(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "string_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11}
!9 = !DILocalVariable(name: "msg", scope: !4, file: !1, line: 6, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!11 = !DILocalVariable(name: "len", scope: !4, file: !1, line: 10, type: !7)
!12 = !DILocation(line: 3, column: 1, scope: !4)
!13 = !DILocation(line: 6, column: 5, scope: !4)
!14 = !DILocation(line: 10, column: 5, scope: !4)
