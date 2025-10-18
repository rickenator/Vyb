; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [25 x i8] c"Starting String tests...\00", align 1
@1 = private unnamed_addr constant [23 x i8] c"String tests completed\00", align 1

define i64 @main() !dbg !4 {
entry:
  %bytes = alloca [5 x i64], align 8, !dbg !11
  call void @__vyn_println(ptr @0), !dbg !11
  store [5 x i64] [i64 72, i64 101, i64 108, i64 108, i64 111], ptr %bytes, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %bytes, metadata !9, metadata !DIExpression()), !dbg !12
  call void @__vyn_println(ptr @1), !dbg !11
  ret i64 0, !dbg !11
}

declare void @__vyn_println(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

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
!8 = !{!9}
!9 = !DILocalVariable(name: "bytes", scope: !4, file: !1, line: 6, type: !10)
!10 = !DIBasicType(tag: DW_TAG_unspecified_type, name: "[5 x i64]")
!11 = !DILocation(line: 3, column: 1, scope: !4)
!12 = !DILocation(line: 6, column: 5, scope: !4)
