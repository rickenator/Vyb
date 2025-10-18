; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [36 x i8] c"Minimal test completed successfully\00", align 1

define i64 @main() !dbg !4 {
entry:
  %v1 = alloca { ptr, i64, i64 }, align 8
  %i32 = alloca i32, align 4
  store i32 42, ptr %i32, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %i32, metadata !9, metadata !DIExpression()), !dbg !14
  %vec.new = alloca { ptr, i64, i64 }, align 8, !dbg !13
  %vec.data = call ptr @malloc(i64 24), !dbg !13
  %0 = call ptr @memset(ptr %vec.data, i32 0, i64 24), !dbg !13
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0, !dbg !13
  store ptr %vec.data, ptr %vec.ptr_field, align 8, !dbg !13
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1, !dbg !13
  store i64 3, ptr %vec.size_field, align 4, !dbg !13
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2, !dbg !13
  store i64 3, ptr %vec.cap_field, align 4, !dbg !13
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8, !dbg !13
  store { ptr, i64, i64 } %vec.new.value, ptr %v1, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %v1, metadata !11, metadata !DIExpression()), !dbg !15
  call void @__vyn_println(ptr @0), !dbg !13
  %v1_cleanup_load = load { ptr, i64, i64 }, ptr %v1, align 8, !dbg !13
  %v1_data_ptr = extractvalue { ptr, i64, i64 } %v1_cleanup_load, 0, !dbg !13
  %v1_null_check = icmp ne ptr %v1_data_ptr, null, !dbg !13
  br i1 %v1_null_check, label %v1_free_block, label %v1_continue, !dbg !13

v1_free_block:                                    ; preds = %entry
  call void @free(ptr %v1_data_ptr), !dbg !13
  br label %v1_continue, !dbg !13

v1_continue:                                      ; preds = %v1_free_block, %entry
  ret i64 42, !dbg !13
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare ptr @memset(ptr, i32, i64)

declare void @__vyn_println(ptr)

declare void @free(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_minimal_working.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11}
!9 = !DILocalVariable(name: "i32", scope: !4, file: !1, line: 3, type: !10)
!10 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "v1", scope: !4, file: !1, line: 6, type: !12)
!12 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64, i64 }", scope: !1, file: !1, size: 192, align: 8)
!13 = !DILocation(line: 2, column: 1, scope: !4)
!14 = !DILocation(line: 3, column: 5, scope: !4)
!15 = !DILocation(line: 6, column: 5, scope: !4)
