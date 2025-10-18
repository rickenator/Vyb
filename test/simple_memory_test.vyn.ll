; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %v = alloca i64, align 8
  %b = alloca i64, align 8
  %p = alloca ptr, align 8
  %x = alloca i64, align 8
  store i64 42, ptr %x, align 4, !dbg !14
  call void @llvm.dbg.declare(metadata ptr %x, metadata !9, metadata !DIExpression()), !dbg !15
  %x1 = load i64, ptr %x, align 4, !dbg !14
  %loc_alloca = alloca i64, align 8, !dbg !14
  store i64 %x1, ptr %loc_alloca, align 4, !dbg !14
  store ptr %loc_alloca, ptr %p, align 8, !dbg !14
  call void @llvm.dbg.declare(metadata ptr %p, metadata !10, metadata !DIExpression()), !dbg !16
  %ptr_load = load ptr, ptr %p, align 8, !dbg !14
  %ptr.is_not_null = icmp ne ptr %ptr_load, null, !dbg !14
  br i1 %ptr.is_not_null, label %ptr.not_null, label %ptr.null, !dbg !14

ptr.not_null:                                     ; preds = %entry
  br label %ptr.merge, !dbg !14

ptr.null:                                         ; preds = %entry
  unreachable, !dbg !14

ptr.merge:                                        ; preds = %ptr.not_null
  store i64 100, ptr %ptr_load, align 4, !dbg !14
  %x2 = load i64, ptr %x, align 4, !dbg !14
  %borrow.tmp = alloca i64, align 8, !dbg !14
  store i64 %x2, ptr %borrow.tmp, align 4, !dbg !14
  %ptrtoint_cast = ptrtoint ptr %borrow.tmp to i64, !dbg !14
  store i64 %ptrtoint_cast, ptr %b, align 4, !dbg !14
  call void @llvm.dbg.declare(metadata ptr %b, metadata !12, metadata !DIExpression()), !dbg !17
  %x3 = load i64, ptr %x, align 4, !dbg !14
  %view.tmp = alloca i64, align 8, !dbg !14
  store i64 %x3, ptr %view.tmp, align 4, !dbg !14
  %ptrtoint_cast4 = ptrtoint ptr %view.tmp to i64, !dbg !14
  store i64 %ptrtoint_cast4, ptr %v, align 4, !dbg !14
  call void @llvm.dbg.declare(metadata ptr %v, metadata !13, metadata !DIExpression()), !dbg !18
  %x5 = load i64, ptr %x, align 4, !dbg !14
  ret i64 %x5, !dbg !14
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
!1 = !DIFile(filename: "simple_memory_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10, !12, !13}
!9 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 3, type: !7)
!10 = !DILocalVariable(name: "p", scope: !4, file: !1, line: 6, type: !11)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!12 = !DILocalVariable(name: "b", scope: !4, file: !1, line: 7, type: !7)
!13 = !DILocalVariable(name: "v", scope: !4, file: !1, line: 9, type: !7)
!14 = !DILocation(line: 2, column: 1, scope: !4)
!15 = !DILocation(line: 3, column: 1, scope: !4)
!16 = !DILocation(line: 6, column: 1, scope: !4)
!17 = !DILocation(line: 7, column: 23, scope: !4)
!18 = !DILocation(line: 9, column: 1, scope: !4)
