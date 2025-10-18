; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %p = alloca ptr, align 8
  %x = alloca i64, align 8
  store i64 77, ptr %x, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %x, metadata !9, metadata !DIExpression()), !dbg !13
  store ptr null, ptr %p, align 8, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %p, metadata !10, metadata !DIExpression()), !dbg !14
  %x1 = load i64, ptr %x, align 4, !dbg !12
  %loc_alloca = alloca i64, align 8, !dbg !12
  store i64 %x1, ptr %loc_alloca, align 4, !dbg !12
  store ptr %loc_alloca, ptr %p, align 8, !dbg !12
  %ptr_load = load ptr, ptr %p, align 8, !dbg !12
  %ptr.is_not_null = icmp ne ptr %ptr_load, null, !dbg !12
  br i1 %ptr.is_not_null, label %ptr.not_null, label %ptr.null, !dbg !12

ptr.not_null:                                     ; preds = %entry
  br label %ptr.merge, !dbg !12

ptr.null:                                         ; preds = %entry
  unreachable, !dbg !12

ptr.merge:                                        ; preds = %ptr.not_null
  store i64 99, ptr %ptr_load, align 4, !dbg !12
  %x2 = load i64, ptr %x, align 4, !dbg !12
  ret i64 %x2, !dbg !12
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
!1 = !DIFile(filename: "test_basic_pointer_operations_test_1.vyn.ll", directory: "/home/rick/Projects/Vyn/test/units/extracted")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 11, type: !5, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10}
!9 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 12, type: !7)
!10 = !DILocalVariable(name: "p", scope: !4, file: !1, line: 13, type: !11)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!12 = !DILocation(line: 11, column: 1, scope: !4)
!13 = !DILocation(line: 12, column: 1, scope: !4)
!14 = !DILocation(line: 13, column: 1, scope: !4)
