; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %x = alloca i64, align 8, !dbg !12
  %v = alloca { ptr, i64, i64 }, align 8, !dbg !12
  %vec.new = alloca { ptr, i64, i64 }, align 8, !dbg !12
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0, !dbg !12
  store ptr null, ptr %vec.ptr_field, align 8, !dbg !12
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1, !dbg !12
  store i64 0, ptr %vec.size_field, align 4, !dbg !12
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2, !dbg !12
  store i64 0, ptr %vec.cap_field, align 4, !dbg !12
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8, !dbg !12
  store { ptr, i64, i64 } %vec.new.value, ptr %v, align 8, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %v, metadata !9, metadata !DIExpression()), !dbg !13
  %vec.data_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 0, !dbg !12
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !12
  %vec.cap_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 2, !dbg !12
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4, !dbg !12
  %vec.current_cap = load i64, ptr %vec.cap_ptr, align 4, !dbg !12
  %vec.data = load ptr, ptr %vec.data_ptr, align 8, !dbg !12
  %vec.needs_alloc = icmp eq i64 %vec.current_cap, 0, !dbg !12
  %vec.needs_grow = icmp eq i64 %vec.current_size, %vec.current_cap, !dbg !12
  %vec.needs_realloc = or i1 %vec.needs_alloc, %vec.needs_grow, !dbg !12
  br i1 %vec.needs_realloc, label %vec.alloc, label %vec.merge, !dbg !12

vec.alloc:                                        ; preds = %entry
  %0 = mul i64 %vec.current_cap, 2, !dbg !12
  %vec.new_cap = select i1 %vec.needs_alloc, i64 4, i64 %0, !dbg !12
  %vec.alloc_size = mul i64 %vec.new_cap, 8, !dbg !12
  %vec.new_data = call ptr @malloc(i64 %vec.alloc_size), !dbg !12
  %vec.has_data = icmp ne i64 %vec.current_size, 0, !dbg !12
  br i1 %vec.has_data, label %vec.copy, label %vec.no_copy, !dbg !12

vec.copy:                                         ; preds = %vec.alloc
  %vec.copy_size = mul i64 %vec.current_size, 8, !dbg !12
  %1 = call ptr @memcpy(ptr %vec.new_data, ptr %vec.data, i64 %vec.copy_size), !dbg !12
  br label %vec.no_copy, !dbg !12

vec.no_copy:                                      ; preds = %vec.copy, %vec.alloc
  store ptr %vec.new_data, ptr %vec.data_ptr, align 8, !dbg !12
  store i64 %vec.new_cap, ptr %vec.cap_ptr, align 4, !dbg !12
  br label %vec.merge, !dbg !12

vec.merge:                                        ; preds = %vec.no_copy, %entry
  %vec.final_data = load ptr, ptr %vec.data_ptr, align 8, !dbg !12
  %vec.reloaded_size = load i64, ptr %vec.size_ptr, align 4, !dbg !12
  %vec.offset = mul i64 %vec.reloaded_size, 8, !dbg !12
  %vec.element_ptr = getelementptr i8, ptr %vec.final_data, i64 %vec.offset, !dbg !12
  store i64 5, ptr %vec.element_ptr, align 4, !dbg !12
  %vec.new_size = add i64 %vec.reloaded_size, 1, !dbg !12
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4, !dbg !12
  %vec.size_ptr1 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !12
  %vec.len = load i64, ptr %vec.size_ptr1, align 4, !dbg !12
  store i64 %vec.len, ptr %x, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %x, metadata !11, metadata !DIExpression()), !dbg !14
  %x2 = load i64, ptr %x, align 4, !dbg !12
  %v_cleanup_load = load { ptr, i64, i64 }, ptr %v, align 8, !dbg !12
  %v_data_ptr = extractvalue { ptr, i64, i64 } %v_cleanup_load, 0, !dbg !12
  %v_null_check = icmp ne ptr %v_data_ptr, null, !dbg !12
  br i1 %v_null_check, label %v_free_block, label %v_continue, !dbg !12

v_free_block:                                     ; preds = %vec.merge
  call void @free(ptr %v_data_ptr), !dbg !12
  br label %v_continue, !dbg !12

v_continue:                                       ; preds = %v_free_block, %vec.merge
  ret i64 %x2, !dbg !12
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

declare void @free(ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_vec_no_iter.vyn.ll", directory: "/home/rick/Projects/Vyn/test/vec_for")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11}
!9 = !DILocalVariable(name: "v", scope: !4, file: !1, line: 2, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64, i64 }", scope: !1, file: !1, size: 192, align: 8)
!11 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 4, type: !7)
!12 = !DILocation(line: 1, column: 1, scope: !4)
!13 = !DILocation(line: 2, column: 1, scope: !4)
!14 = !DILocation(line: 4, column: 1, scope: !4)
