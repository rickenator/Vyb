; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [5 x i8] c"test\00", align 1

define ptr @main() !dbg !4 {
entry:
  %x = alloca i64, align 8
  store i64 42, ptr %x, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %x, metadata !9, metadata !DIExpression()), !dbg !12
  ret ptr @0, !dbg !11
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
!1 = !DIFile(filename: "debug_ownership.vyn.ll", directory: "/home/rick/Projects/Vyn/test/debug")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!8 = !{!9}
!9 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 1, type: !10)
!10 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!11 = !DILocation(line: 1, column: 1, scope: !4)
!12 = !DILocation(line: 1, column: 21, scope: !4)
