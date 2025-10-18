; ModuleID = 'VynModule'
source_filename = "VynModule"

%TestStruct = type { i64 }
%TestStruct.0 = type { i64 }

@0 = private unnamed_addr constant [28 x i8] c"Testing direct method call:\00", align 1

define void @test_method_call(%TestStruct %obj) !dbg !4 {
entry:
  %obj1 = alloca %TestStruct, align 8
  store %TestStruct %obj, ptr %obj1, align 4, !dbg !10
  call void @llvm.dbg.declare(metadata ptr %obj1, metadata !9, metadata !DIExpression()), !dbg !11
  call void @__vyn_println(ptr @0), !dbg !10
  ret void, !dbg !10
}

define i64 @main() !dbg !12 {
entry:
  %TestStruct.0_obj = alloca %TestStruct.0, align 8, !dbg !16
  ret i64 0, !dbg !16
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
!1 = !DIFile(filename: "test_method_call2.vyn.ll", directory: "/home/rick/Projects/Vyn")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_method_call", linkageName: "test_method_call", scope: !1, file: !1, line: 7, type: !5, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{null, !7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "TestStruct", scope: !1, file: !1, size: 64, align: 8)
!8 = !{!9}
!9 = !DILocalVariable(name: "obj", scope: !4, file: !1, line: 7, type: !7)
!10 = !DILocation(line: 7, column: 1, scope: !4)
!11 = !DILocation(line: 7, column: 21, scope: !4)
!12 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 13, type: !13, scopeLine: 13, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!13 = !DISubroutineType(types: !14)
!14 = !{!15}
!15 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!16 = !DILocation(line: 13, column: 1, scope: !12)
