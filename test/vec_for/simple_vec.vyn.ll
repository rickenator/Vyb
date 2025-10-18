; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %num = alloca i64, align 8, !dbg !16
  %__idx_num = alloca i64, align 8, !dbg !16
  %__len_num = alloca i64, align 8, !dbg !16
  %__vec_tmp_num = alloca { ptr, i64, i64 }, align 8, !dbg !16
  %sum = alloca i64, align 8, !dbg !16
  %numbers = alloca { ptr, i64, i64 }, align 8, !dbg !16
  %vec.new = alloca { ptr, i64, i64 }, align 8, !dbg !16
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0, !dbg !16
  store ptr null, ptr %vec.ptr_field, align 8, !dbg !16
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1, !dbg !16
  store i64 0, ptr %vec.size_field, align 4, !dbg !16
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2, !dbg !16
  store i64 0, ptr %vec.cap_field, align 4, !dbg !16
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8, !dbg !16
  store { ptr, i64, i64 } %vec.new.value, ptr %numbers, align 8, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %numbers, metadata !9, metadata !DIExpression()), !dbg !17
  %vec.data_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !16
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !16
  %vec.cap_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !16
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4, !dbg !16
  %vec.current_cap = load i64, ptr %vec.cap_ptr, align 4, !dbg !16
  %vec.data = load ptr, ptr %vec.data_ptr, align 8, !dbg !16
  %vec.needs_alloc = icmp eq i64 %vec.current_cap, 0, !dbg !16
  %vec.needs_grow = icmp eq i64 %vec.current_size, %vec.current_cap, !dbg !16
  %vec.needs_realloc = or i1 %vec.needs_alloc, %vec.needs_grow, !dbg !16
  br i1 %vec.needs_realloc, label %vec.alloc, label %vec.merge, !dbg !16

vec.alloc:                                        ; preds = %entry
  %0 = mul i64 %vec.current_cap, 2, !dbg !16
  %vec.new_cap = select i1 %vec.needs_alloc, i64 4, i64 %0, !dbg !16
  %vec.alloc_size = mul i64 %vec.new_cap, 8, !dbg !16
  %vec.new_data = call ptr @malloc(i64 %vec.alloc_size), !dbg !16
  %vec.has_data = icmp ne i64 %vec.current_size, 0, !dbg !16
  br i1 %vec.has_data, label %vec.copy, label %vec.no_copy, !dbg !16

vec.copy:                                         ; preds = %vec.alloc
  %vec.copy_size = mul i64 %vec.current_size, 8, !dbg !16
  %1 = call ptr @memcpy(ptr %vec.new_data, ptr %vec.data, i64 %vec.copy_size), !dbg !16
  br label %vec.no_copy, !dbg !16

vec.no_copy:                                      ; preds = %vec.copy, %vec.alloc
  store ptr %vec.new_data, ptr %vec.data_ptr, align 8, !dbg !16
  store i64 %vec.new_cap, ptr %vec.cap_ptr, align 4, !dbg !16
  br label %vec.merge, !dbg !16

vec.merge:                                        ; preds = %vec.no_copy, %entry
  %vec.final_data = load ptr, ptr %vec.data_ptr, align 8, !dbg !16
  %vec.reloaded_size = load i64, ptr %vec.size_ptr, align 4, !dbg !16
  %vec.offset = mul i64 %vec.reloaded_size, 8, !dbg !16
  %vec.element_ptr = getelementptr i8, ptr %vec.final_data, i64 %vec.offset, !dbg !16
  store i64 10, ptr %vec.element_ptr, align 4, !dbg !16
  %vec.new_size = add i64 %vec.reloaded_size, 1, !dbg !16
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4, !dbg !16
  %vec.data_ptr1 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !16
  %vec.size_ptr2 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !16
  %vec.cap_ptr3 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !16
  %vec.current_size4 = load i64, ptr %vec.size_ptr2, align 4, !dbg !16
  %vec.current_cap5 = load i64, ptr %vec.cap_ptr3, align 4, !dbg !16
  %vec.data6 = load ptr, ptr %vec.data_ptr1, align 8, !dbg !16
  %vec.needs_alloc7 = icmp eq i64 %vec.current_cap5, 0, !dbg !16
  %vec.needs_grow8 = icmp eq i64 %vec.current_size4, %vec.current_cap5, !dbg !16
  %vec.needs_realloc9 = or i1 %vec.needs_alloc7, %vec.needs_grow8, !dbg !16
  br i1 %vec.needs_realloc9, label %vec.alloc10, label %vec.merge13, !dbg !16

vec.alloc10:                                      ; preds = %vec.merge
  %2 = mul i64 %vec.current_cap5, 2, !dbg !16
  %vec.new_cap14 = select i1 %vec.needs_alloc7, i64 4, i64 %2, !dbg !16
  %vec.alloc_size15 = mul i64 %vec.new_cap14, 8, !dbg !16
  %vec.new_data16 = call ptr @malloc(i64 %vec.alloc_size15), !dbg !16
  %vec.has_data17 = icmp ne i64 %vec.current_size4, 0, !dbg !16
  br i1 %vec.has_data17, label %vec.copy11, label %vec.no_copy12, !dbg !16

vec.copy11:                                       ; preds = %vec.alloc10
  %vec.copy_size18 = mul i64 %vec.current_size4, 8, !dbg !16
  %3 = call ptr @memcpy(ptr %vec.new_data16, ptr %vec.data6, i64 %vec.copy_size18), !dbg !16
  br label %vec.no_copy12, !dbg !16

vec.no_copy12:                                    ; preds = %vec.copy11, %vec.alloc10
  store ptr %vec.new_data16, ptr %vec.data_ptr1, align 8, !dbg !16
  store i64 %vec.new_cap14, ptr %vec.cap_ptr3, align 4, !dbg !16
  br label %vec.merge13, !dbg !16

vec.merge13:                                      ; preds = %vec.no_copy12, %vec.merge
  %vec.final_data19 = load ptr, ptr %vec.data_ptr1, align 8, !dbg !16
  %vec.reloaded_size20 = load i64, ptr %vec.size_ptr2, align 4, !dbg !16
  %vec.offset21 = mul i64 %vec.reloaded_size20, 8, !dbg !16
  %vec.element_ptr22 = getelementptr i8, ptr %vec.final_data19, i64 %vec.offset21, !dbg !16
  store i64 20, ptr %vec.element_ptr22, align 4, !dbg !16
  %vec.new_size23 = add i64 %vec.reloaded_size20, 1, !dbg !16
  store i64 %vec.new_size23, ptr %vec.size_ptr2, align 4, !dbg !16
  %vec.data_ptr24 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 0, !dbg !16
  %vec.size_ptr25 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 1, !dbg !16
  %vec.cap_ptr26 = getelementptr inbounds { ptr, i64, i64 }, ptr %numbers, i32 0, i32 2, !dbg !16
  %vec.current_size27 = load i64, ptr %vec.size_ptr25, align 4, !dbg !16
  %vec.current_cap28 = load i64, ptr %vec.cap_ptr26, align 4, !dbg !16
  %vec.data29 = load ptr, ptr %vec.data_ptr24, align 8, !dbg !16
  %vec.needs_alloc30 = icmp eq i64 %vec.current_cap28, 0, !dbg !16
  %vec.needs_grow31 = icmp eq i64 %vec.current_size27, %vec.current_cap28, !dbg !16
  %vec.needs_realloc32 = or i1 %vec.needs_alloc30, %vec.needs_grow31, !dbg !16
  br i1 %vec.needs_realloc32, label %vec.alloc33, label %vec.merge36, !dbg !16

vec.alloc33:                                      ; preds = %vec.merge13
  %4 = mul i64 %vec.current_cap28, 2, !dbg !16
  %vec.new_cap37 = select i1 %vec.needs_alloc30, i64 4, i64 %4, !dbg !16
  %vec.alloc_size38 = mul i64 %vec.new_cap37, 8, !dbg !16
  %vec.new_data39 = call ptr @malloc(i64 %vec.alloc_size38), !dbg !16
  %vec.has_data40 = icmp ne i64 %vec.current_size27, 0, !dbg !16
  br i1 %vec.has_data40, label %vec.copy34, label %vec.no_copy35, !dbg !16

vec.copy34:                                       ; preds = %vec.alloc33
  %vec.copy_size41 = mul i64 %vec.current_size27, 8, !dbg !16
  %5 = call ptr @memcpy(ptr %vec.new_data39, ptr %vec.data29, i64 %vec.copy_size41), !dbg !16
  br label %vec.no_copy35, !dbg !16

vec.no_copy35:                                    ; preds = %vec.copy34, %vec.alloc33
  store ptr %vec.new_data39, ptr %vec.data_ptr24, align 8, !dbg !16
  store i64 %vec.new_cap37, ptr %vec.cap_ptr26, align 4, !dbg !16
  br label %vec.merge36, !dbg !16

vec.merge36:                                      ; preds = %vec.no_copy35, %vec.merge13
  %vec.final_data42 = load ptr, ptr %vec.data_ptr24, align 8, !dbg !16
  %vec.reloaded_size43 = load i64, ptr %vec.size_ptr25, align 4, !dbg !16
  %vec.offset44 = mul i64 %vec.reloaded_size43, 8, !dbg !16
  %vec.element_ptr45 = getelementptr i8, ptr %vec.final_data42, i64 %vec.offset44, !dbg !16
  store i64 30, ptr %vec.element_ptr45, align 4, !dbg !16
  %vec.new_size46 = add i64 %vec.reloaded_size43, 1, !dbg !16
  store i64 %vec.new_size46, ptr %vec.size_ptr25, align 4, !dbg !16
  store i64 0, ptr %sum, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %sum, metadata !11, metadata !DIExpression()), !dbg !18
  br label %for.init, !dbg !16

for.init:                                         ; preds = %vec.merge36
  %numbers47 = load { ptr, i64, i64 }, ptr %numbers, align 8, !dbg !16
  store { ptr, i64, i64 } %numbers47, ptr %__vec_tmp_num, align 8, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %__vec_tmp_num, metadata !12, metadata !DIExpression()), !dbg !19
  br label %for.cond, !dbg !16

for.cond:                                         ; preds = %for.update, %for.init
  br i1 true, label %for.body, label %for.exit, !dbg !16

for.body:                                         ; preds = %for.cond
  br label %for.init48, !dbg !16

for.update:                                       ; preds = %for.exit52
  br label %for.cond, !dbg !16

for.exit:                                         ; preds = %for.cond
  %sum71 = load i64, ptr %sum, align 4, !dbg !16
  %__vec_tmp_num_cleanup_load = load { ptr, i64, i64 }, ptr %__vec_tmp_num, align 8, !dbg !16
  %__vec_tmp_num_data_ptr = extractvalue { ptr, i64, i64 } %__vec_tmp_num_cleanup_load, 0, !dbg !16
  %__vec_tmp_num_null_check = icmp ne ptr %__vec_tmp_num_data_ptr, null, !dbg !16
  br i1 %__vec_tmp_num_null_check, label %__vec_tmp_num_free_block, label %__vec_tmp_num_continue, !dbg !16

for.init48:                                       ; preds = %for.body
  %vec.size_ptr53 = getelementptr inbounds { ptr, i64, i64 }, ptr %__vec_tmp_num, i32 0, i32 1, !dbg !16
  %vec.len = load i64, ptr %vec.size_ptr53, align 4, !dbg !16
  store i64 %vec.len, ptr %__len_num, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %__len_num, metadata !13, metadata !DIExpression()), !dbg !19
  br label %for.cond49, !dbg !16

for.cond49:                                       ; preds = %for.update51, %for.init48
  br i1 true, label %for.body50, label %for.exit52, !dbg !16

for.body50:                                       ; preds = %for.cond49
  br label %for.init54, !dbg !16

for.update51:                                     ; preds = %for.exit58
  br label %for.cond49, !dbg !16

for.exit52:                                       ; preds = %for.cond49
  br label %for.update, !dbg !16

for.init54:                                       ; preds = %for.body50
  store i64 0, ptr %__idx_num, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %__idx_num, metadata !14, metadata !DIExpression()), !dbg !19
  br label %for.cond55, !dbg !16

for.cond55:                                       ; preds = %for.update57, %for.init54
  %__idx_num59 = load i64, ptr %__idx_num, align 4, !dbg !16
  %__len_num60 = load i64, ptr %__len_num, align 4, !dbg !16
  %icmpslttmp = icmp slt i64 %__idx_num59, %__len_num60, !dbg !16
  br i1 %icmpslttmp, label %for.body56, label %for.exit58, !dbg !16

for.body56:                                       ; preds = %for.cond55
  %__idx_num61 = load i64, ptr %__idx_num, align 4, !dbg !16
  %vec.data_ptr62 = getelementptr inbounds { ptr, i64, i64 }, ptr %__vec_tmp_num, i32 0, i32 0, !dbg !16
  %vec.size_ptr63 = getelementptr inbounds { ptr, i64, i64 }, ptr %__vec_tmp_num, i32 0, i32 1, !dbg !16
  %vec.data64 = load ptr, ptr %vec.data_ptr62, align 8, !dbg !16
  %vec.size = load i64, ptr %vec.size_ptr63, align 4, !dbg !16
  %vec.offset65 = mul i64 %__idx_num61, 8, !dbg !16
  %vec.element_ptr66 = getelementptr i8, ptr %vec.data64, i64 %vec.offset65, !dbg !16
  %vec.element = load i64, ptr %vec.element_ptr66, align 4, !dbg !16
  store i64 %vec.element, ptr %num, align 4, !dbg !16
  call void @llvm.dbg.declare(metadata ptr %num, metadata !15, metadata !DIExpression()), !dbg !19
  %sum67 = load i64, ptr %sum, align 4, !dbg !16
  %num68 = load i64, ptr %num, align 4, !dbg !16
  %addtmp = add i64 %sum67, %num68, !dbg !16
  store i64 %addtmp, ptr %sum, align 4, !dbg !16
  br label %for.update57, !dbg !16

for.update57:                                     ; preds = %for.body56
  %__idx_num69 = load i64, ptr %__idx_num, align 4, !dbg !16
  %addtmp70 = add i64 %__idx_num69, 1, !dbg !16
  store i64 %addtmp70, ptr %__idx_num, align 4, !dbg !16
  br label %for.cond55, !dbg !16

for.exit58:                                       ; preds = %for.cond55
  br label %for.update51, !dbg !16

__vec_tmp_num_free_block:                         ; preds = %for.exit
  call void @free(ptr %__vec_tmp_num_data_ptr), !dbg !16
  br label %__vec_tmp_num_continue, !dbg !16

__vec_tmp_num_continue:                           ; preds = %__vec_tmp_num_free_block, %for.exit
  %numbers_cleanup_load = load { ptr, i64, i64 }, ptr %numbers, align 8, !dbg !16
  %numbers_data_ptr = extractvalue { ptr, i64, i64 } %numbers_cleanup_load, 0, !dbg !16
  %numbers_null_check = icmp ne ptr %numbers_data_ptr, null, !dbg !16
  br i1 %numbers_null_check, label %numbers_free_block, label %numbers_continue, !dbg !16

numbers_free_block:                               ; preds = %__vec_tmp_num_continue
  call void @free(ptr %numbers_data_ptr), !dbg !16
  br label %numbers_continue, !dbg !16

numbers_continue:                                 ; preds = %numbers_free_block, %__vec_tmp_num_continue
  ret i64 %sum71, !dbg !16
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
!1 = !DIFile(filename: "simple_vec.vyn.ll", directory: "/home/rick/Projects/Vyn/test/vec_for")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !13, !14, !15}
!9 = !DILocalVariable(name: "numbers", scope: !4, file: !1, line: 2, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64, i64 }", scope: !1, file: !1, size: 192, align: 8)
!11 = !DILocalVariable(name: "sum", scope: !4, file: !1, line: 7, type: !7)
!12 = !DILocalVariable(name: "__vec_tmp_num", scope: !4, file: !1, line: 8, type: !10)
!13 = !DILocalVariable(name: "__len_num", scope: !4, file: !1, line: 8, type: !7)
!14 = !DILocalVariable(name: "__idx_num", scope: !4, file: !1, line: 8, type: !7)
!15 = !DILocalVariable(name: "num", scope: !4, file: !1, line: 8, type: !7)
!16 = !DILocation(line: 1, column: 1, scope: !4)
!17 = !DILocation(line: 2, column: 1, scope: !4)
!18 = !DILocation(line: 7, column: 1, scope: !4)
!19 = !DILocation(line: 8, column: 10, scope: !4)
