; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %x = alloca i64, align 8, !dbg !17
  %__len_x = alloca i64, align 8, !dbg !17
  %__idx_x = alloca i64, align 8, !dbg !17
  %__run_once_x = alloca i1, align 1, !dbg !17
  %sum = alloca i64, align 8, !dbg !17
  %v = alloca { ptr, i64, i64 }, align 8, !dbg !17
  %vec.new = alloca { ptr, i64, i64 }, align 8, !dbg !17
  %vec.ptr_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 0, !dbg !17
  store ptr null, ptr %vec.ptr_field, align 8, !dbg !17
  %vec.size_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 1, !dbg !17
  store i64 0, ptr %vec.size_field, align 4, !dbg !17
  %vec.cap_field = getelementptr inbounds { ptr, i64, i64 }, ptr %vec.new, i32 0, i32 2, !dbg !17
  store i64 0, ptr %vec.cap_field, align 4, !dbg !17
  %vec.new.value = load { ptr, i64, i64 }, ptr %vec.new, align 8, !dbg !17
  store { ptr, i64, i64 } %vec.new.value, ptr %v, align 8, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %v, metadata !9, metadata !DIExpression()), !dbg !18
  %vec.data_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 0, !dbg !17
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !17
  %vec.cap_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 2, !dbg !17
  %vec.current_size = load i64, ptr %vec.size_ptr, align 4, !dbg !17
  %vec.current_cap = load i64, ptr %vec.cap_ptr, align 4, !dbg !17
  %vec.data = load ptr, ptr %vec.data_ptr, align 8, !dbg !17
  %vec.needs_alloc = icmp eq i64 %vec.current_cap, 0, !dbg !17
  %vec.needs_grow = icmp eq i64 %vec.current_size, %vec.current_cap, !dbg !17
  %vec.needs_realloc = or i1 %vec.needs_alloc, %vec.needs_grow, !dbg !17
  br i1 %vec.needs_realloc, label %vec.alloc, label %vec.merge, !dbg !17

vec.alloc:                                        ; preds = %entry
  %0 = mul i64 %vec.current_cap, 2, !dbg !17
  %vec.new_cap = select i1 %vec.needs_alloc, i64 4, i64 %0, !dbg !17
  %vec.alloc_size = mul i64 %vec.new_cap, 8, !dbg !17
  %vec.new_data = call ptr @malloc(i64 %vec.alloc_size), !dbg !17
  %vec.has_data = icmp ne i64 %vec.current_size, 0, !dbg !17
  br i1 %vec.has_data, label %vec.copy, label %vec.no_copy, !dbg !17

vec.copy:                                         ; preds = %vec.alloc
  %vec.copy_size = mul i64 %vec.current_size, 8, !dbg !17
  %1 = call ptr @memcpy(ptr %vec.new_data, ptr %vec.data, i64 %vec.copy_size), !dbg !17
  br label %vec.no_copy, !dbg !17

vec.no_copy:                                      ; preds = %vec.copy, %vec.alloc
  store ptr %vec.new_data, ptr %vec.data_ptr, align 8, !dbg !17
  store i64 %vec.new_cap, ptr %vec.cap_ptr, align 4, !dbg !17
  br label %vec.merge, !dbg !17

vec.merge:                                        ; preds = %vec.no_copy, %entry
  %vec.final_data = load ptr, ptr %vec.data_ptr, align 8, !dbg !17
  %vec.reloaded_size = load i64, ptr %vec.size_ptr, align 4, !dbg !17
  %vec.offset = mul i64 %vec.reloaded_size, 8, !dbg !17
  %vec.element_ptr = getelementptr i8, ptr %vec.final_data, i64 %vec.offset, !dbg !17
  store i64 1, ptr %vec.element_ptr, align 4, !dbg !17
  %vec.new_size = add i64 %vec.reloaded_size, 1, !dbg !17
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4, !dbg !17
  %vec.data_ptr1 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 0, !dbg !17
  %vec.size_ptr2 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !17
  %vec.cap_ptr3 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 2, !dbg !17
  %vec.current_size4 = load i64, ptr %vec.size_ptr2, align 4, !dbg !17
  %vec.current_cap5 = load i64, ptr %vec.cap_ptr3, align 4, !dbg !17
  %vec.data6 = load ptr, ptr %vec.data_ptr1, align 8, !dbg !17
  %vec.needs_alloc7 = icmp eq i64 %vec.current_cap5, 0, !dbg !17
  %vec.needs_grow8 = icmp eq i64 %vec.current_size4, %vec.current_cap5, !dbg !17
  %vec.needs_realloc9 = or i1 %vec.needs_alloc7, %vec.needs_grow8, !dbg !17
  br i1 %vec.needs_realloc9, label %vec.alloc10, label %vec.merge13, !dbg !17

vec.alloc10:                                      ; preds = %vec.merge
  %2 = mul i64 %vec.current_cap5, 2, !dbg !17
  %vec.new_cap14 = select i1 %vec.needs_alloc7, i64 4, i64 %2, !dbg !17
  %vec.alloc_size15 = mul i64 %vec.new_cap14, 8, !dbg !17
  %vec.new_data16 = call ptr @malloc(i64 %vec.alloc_size15), !dbg !17
  %vec.has_data17 = icmp ne i64 %vec.current_size4, 0, !dbg !17
  br i1 %vec.has_data17, label %vec.copy11, label %vec.no_copy12, !dbg !17

vec.copy11:                                       ; preds = %vec.alloc10
  %vec.copy_size18 = mul i64 %vec.current_size4, 8, !dbg !17
  %3 = call ptr @memcpy(ptr %vec.new_data16, ptr %vec.data6, i64 %vec.copy_size18), !dbg !17
  br label %vec.no_copy12, !dbg !17

vec.no_copy12:                                    ; preds = %vec.copy11, %vec.alloc10
  store ptr %vec.new_data16, ptr %vec.data_ptr1, align 8, !dbg !17
  store i64 %vec.new_cap14, ptr %vec.cap_ptr3, align 4, !dbg !17
  br label %vec.merge13, !dbg !17

vec.merge13:                                      ; preds = %vec.no_copy12, %vec.merge
  %vec.final_data19 = load ptr, ptr %vec.data_ptr1, align 8, !dbg !17
  %vec.reloaded_size20 = load i64, ptr %vec.size_ptr2, align 4, !dbg !17
  %vec.offset21 = mul i64 %vec.reloaded_size20, 8, !dbg !17
  %vec.element_ptr22 = getelementptr i8, ptr %vec.final_data19, i64 %vec.offset21, !dbg !17
  store i64 2, ptr %vec.element_ptr22, align 4, !dbg !17
  %vec.new_size23 = add i64 %vec.reloaded_size20, 1, !dbg !17
  store i64 %vec.new_size23, ptr %vec.size_ptr2, align 4, !dbg !17
  %vec.data_ptr24 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 0, !dbg !17
  %vec.size_ptr25 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !17
  %vec.cap_ptr26 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 2, !dbg !17
  %vec.current_size27 = load i64, ptr %vec.size_ptr25, align 4, !dbg !17
  %vec.current_cap28 = load i64, ptr %vec.cap_ptr26, align 4, !dbg !17
  %vec.data29 = load ptr, ptr %vec.data_ptr24, align 8, !dbg !17
  %vec.needs_alloc30 = icmp eq i64 %vec.current_cap28, 0, !dbg !17
  %vec.needs_grow31 = icmp eq i64 %vec.current_size27, %vec.current_cap28, !dbg !17
  %vec.needs_realloc32 = or i1 %vec.needs_alloc30, %vec.needs_grow31, !dbg !17
  br i1 %vec.needs_realloc32, label %vec.alloc33, label %vec.merge36, !dbg !17

vec.alloc33:                                      ; preds = %vec.merge13
  %4 = mul i64 %vec.current_cap28, 2, !dbg !17
  %vec.new_cap37 = select i1 %vec.needs_alloc30, i64 4, i64 %4, !dbg !17
  %vec.alloc_size38 = mul i64 %vec.new_cap37, 8, !dbg !17
  %vec.new_data39 = call ptr @malloc(i64 %vec.alloc_size38), !dbg !17
  %vec.has_data40 = icmp ne i64 %vec.current_size27, 0, !dbg !17
  br i1 %vec.has_data40, label %vec.copy34, label %vec.no_copy35, !dbg !17

vec.copy34:                                       ; preds = %vec.alloc33
  %vec.copy_size41 = mul i64 %vec.current_size27, 8, !dbg !17
  %5 = call ptr @memcpy(ptr %vec.new_data39, ptr %vec.data29, i64 %vec.copy_size41), !dbg !17
  br label %vec.no_copy35, !dbg !17

vec.no_copy35:                                    ; preds = %vec.copy34, %vec.alloc33
  store ptr %vec.new_data39, ptr %vec.data_ptr24, align 8, !dbg !17
  store i64 %vec.new_cap37, ptr %vec.cap_ptr26, align 4, !dbg !17
  br label %vec.merge36, !dbg !17

vec.merge36:                                      ; preds = %vec.no_copy35, %vec.merge13
  %vec.final_data42 = load ptr, ptr %vec.data_ptr24, align 8, !dbg !17
  %vec.reloaded_size43 = load i64, ptr %vec.size_ptr25, align 4, !dbg !17
  %vec.offset44 = mul i64 %vec.reloaded_size43, 8, !dbg !17
  %vec.element_ptr45 = getelementptr i8, ptr %vec.final_data42, i64 %vec.offset44, !dbg !17
  store i64 3, ptr %vec.element_ptr45, align 4, !dbg !17
  %vec.new_size46 = add i64 %vec.reloaded_size43, 1, !dbg !17
  store i64 %vec.new_size46, ptr %vec.size_ptr25, align 4, !dbg !17
  %vec.data_ptr47 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 0, !dbg !17
  %vec.size_ptr48 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !17
  %vec.cap_ptr49 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 2, !dbg !17
  %vec.current_size50 = load i64, ptr %vec.size_ptr48, align 4, !dbg !17
  %vec.current_cap51 = load i64, ptr %vec.cap_ptr49, align 4, !dbg !17
  %vec.data52 = load ptr, ptr %vec.data_ptr47, align 8, !dbg !17
  %vec.needs_alloc53 = icmp eq i64 %vec.current_cap51, 0, !dbg !17
  %vec.needs_grow54 = icmp eq i64 %vec.current_size50, %vec.current_cap51, !dbg !17
  %vec.needs_realloc55 = or i1 %vec.needs_alloc53, %vec.needs_grow54, !dbg !17
  br i1 %vec.needs_realloc55, label %vec.alloc56, label %vec.merge59, !dbg !17

vec.alloc56:                                      ; preds = %vec.merge36
  %6 = mul i64 %vec.current_cap51, 2, !dbg !17
  %vec.new_cap60 = select i1 %vec.needs_alloc53, i64 4, i64 %6, !dbg !17
  %vec.alloc_size61 = mul i64 %vec.new_cap60, 8, !dbg !17
  %vec.new_data62 = call ptr @malloc(i64 %vec.alloc_size61), !dbg !17
  %vec.has_data63 = icmp ne i64 %vec.current_size50, 0, !dbg !17
  br i1 %vec.has_data63, label %vec.copy57, label %vec.no_copy58, !dbg !17

vec.copy57:                                       ; preds = %vec.alloc56
  %vec.copy_size64 = mul i64 %vec.current_size50, 8, !dbg !17
  %7 = call ptr @memcpy(ptr %vec.new_data62, ptr %vec.data52, i64 %vec.copy_size64), !dbg !17
  br label %vec.no_copy58, !dbg !17

vec.no_copy58:                                    ; preds = %vec.copy57, %vec.alloc56
  store ptr %vec.new_data62, ptr %vec.data_ptr47, align 8, !dbg !17
  store i64 %vec.new_cap60, ptr %vec.cap_ptr49, align 4, !dbg !17
  br label %vec.merge59, !dbg !17

vec.merge59:                                      ; preds = %vec.no_copy58, %vec.merge36
  %vec.final_data65 = load ptr, ptr %vec.data_ptr47, align 8, !dbg !17
  %vec.reloaded_size66 = load i64, ptr %vec.size_ptr48, align 4, !dbg !17
  %vec.offset67 = mul i64 %vec.reloaded_size66, 8, !dbg !17
  %vec.element_ptr68 = getelementptr i8, ptr %vec.final_data65, i64 %vec.offset67, !dbg !17
  store i64 4, ptr %vec.element_ptr68, align 4, !dbg !17
  %vec.new_size69 = add i64 %vec.reloaded_size66, 1, !dbg !17
  store i64 %vec.new_size69, ptr %vec.size_ptr48, align 4, !dbg !17
  %vec.data_ptr70 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 0, !dbg !17
  %vec.size_ptr71 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !17
  %vec.cap_ptr72 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 2, !dbg !17
  %vec.current_size73 = load i64, ptr %vec.size_ptr71, align 4, !dbg !17
  %vec.current_cap74 = load i64, ptr %vec.cap_ptr72, align 4, !dbg !17
  %vec.data75 = load ptr, ptr %vec.data_ptr70, align 8, !dbg !17
  %vec.needs_alloc76 = icmp eq i64 %vec.current_cap74, 0, !dbg !17
  %vec.needs_grow77 = icmp eq i64 %vec.current_size73, %vec.current_cap74, !dbg !17
  %vec.needs_realloc78 = or i1 %vec.needs_alloc76, %vec.needs_grow77, !dbg !17
  br i1 %vec.needs_realloc78, label %vec.alloc79, label %vec.merge82, !dbg !17

vec.alloc79:                                      ; preds = %vec.merge59
  %8 = mul i64 %vec.current_cap74, 2, !dbg !17
  %vec.new_cap83 = select i1 %vec.needs_alloc76, i64 4, i64 %8, !dbg !17
  %vec.alloc_size84 = mul i64 %vec.new_cap83, 8, !dbg !17
  %vec.new_data85 = call ptr @malloc(i64 %vec.alloc_size84), !dbg !17
  %vec.has_data86 = icmp ne i64 %vec.current_size73, 0, !dbg !17
  br i1 %vec.has_data86, label %vec.copy80, label %vec.no_copy81, !dbg !17

vec.copy80:                                       ; preds = %vec.alloc79
  %vec.copy_size87 = mul i64 %vec.current_size73, 8, !dbg !17
  %9 = call ptr @memcpy(ptr %vec.new_data85, ptr %vec.data75, i64 %vec.copy_size87), !dbg !17
  br label %vec.no_copy81, !dbg !17

vec.no_copy81:                                    ; preds = %vec.copy80, %vec.alloc79
  store ptr %vec.new_data85, ptr %vec.data_ptr70, align 8, !dbg !17
  store i64 %vec.new_cap83, ptr %vec.cap_ptr72, align 4, !dbg !17
  br label %vec.merge82, !dbg !17

vec.merge82:                                      ; preds = %vec.no_copy81, %vec.merge59
  %vec.final_data88 = load ptr, ptr %vec.data_ptr70, align 8, !dbg !17
  %vec.reloaded_size89 = load i64, ptr %vec.size_ptr71, align 4, !dbg !17
  %vec.offset90 = mul i64 %vec.reloaded_size89, 8, !dbg !17
  %vec.element_ptr91 = getelementptr i8, ptr %vec.final_data88, i64 %vec.offset90, !dbg !17
  store i64 5, ptr %vec.element_ptr91, align 4, !dbg !17
  %vec.new_size92 = add i64 %vec.reloaded_size89, 1, !dbg !17
  store i64 %vec.new_size92, ptr %vec.size_ptr71, align 4, !dbg !17
  store i64 0, ptr %sum, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %sum, metadata !11, metadata !DIExpression()), !dbg !19
  br label %for.init, !dbg !17

for.init:                                         ; preds = %vec.merge82
  store i1 true, ptr %__run_once_x, align 1, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %__run_once_x, metadata !12, metadata !DIExpression()), !dbg !20
  br label %for.cond, !dbg !17

for.cond:                                         ; preds = %for.update, %for.init
  %__run_once_x93 = load i1, ptr %__run_once_x, align 1, !dbg !17
  br i1 %__run_once_x93, label %for.body, label %for.exit, !dbg !17

for.body:                                         ; preds = %for.cond
  store i64 0, ptr %__idx_x, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %__idx_x, metadata !14, metadata !DIExpression()), !dbg !20
  %vec.size_ptr94 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !17
  %vec.len = load i64, ptr %vec.size_ptr94, align 4, !dbg !17
  store i64 %vec.len, ptr %__len_x, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %__len_x, metadata !15, metadata !DIExpression()), !dbg !20
  br label %loop.header, !dbg !17

for.update:                                       ; preds = %loop.exit
  store i1 false, ptr %__run_once_x, align 1, !dbg !17
  br label %for.cond, !dbg !17

for.exit:                                         ; preds = %for.cond
  %sum108 = load i64, ptr %sum, align 4, !dbg !17
  %v_cleanup_load = load { ptr, i64, i64 }, ptr %v, align 8, !dbg !17
  %v_data_ptr = extractvalue { ptr, i64, i64 } %v_cleanup_load, 0, !dbg !17
  %v_null_check = icmp ne ptr %v_data_ptr, null, !dbg !17
  br i1 %v_null_check, label %v_free_block, label %v_continue, !dbg !17

loop.header:                                      ; preds = %ifcont, %for.body
  %__idx_x95 = load i64, ptr %__idx_x, align 4, !dbg !17
  %__len_x96 = load i64, ptr %__len_x, align 4, !dbg !17
  %icmpslttmp = icmp slt i64 %__idx_x95, %__len_x96, !dbg !17
  br i1 %icmpslttmp, label %loop.body, label %loop.exit, !dbg !17

loop.body:                                        ; preds = %loop.header
  %__idx_x97 = load i64, ptr %__idx_x, align 4, !dbg !17
  %vec.data_ptr98 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 0, !dbg !17
  %vec.size_ptr99 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !17
  %vec.data100 = load ptr, ptr %vec.data_ptr98, align 8, !dbg !17
  %vec.size = load i64, ptr %vec.size_ptr99, align 4, !dbg !17
  %vec.offset101 = mul i64 %__idx_x97, 8, !dbg !17
  %vec.element_ptr102 = getelementptr i8, ptr %vec.data100, i64 %vec.offset101, !dbg !17
  %vec.element = load i64, ptr %vec.element_ptr102, align 4, !dbg !17
  store i64 %vec.element, ptr %x, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %x, metadata !16, metadata !DIExpression()), !dbg !20
  %sum103 = load i64, ptr %sum, align 4, !dbg !17
  %icmpsgetmp = icmp sge i64 %sum103, 6, !dbg !17
  br i1 %icmpsgetmp, label %then, label %ifcont, !dbg !17

loop.exit:                                        ; preds = %then, %loop.header
  br label %for.update, !dbg !17

then:                                             ; preds = %loop.body
  br label %loop.exit, !dbg !17

ifcont:                                           ; preds = %loop.body
  %sum104 = load i64, ptr %sum, align 4, !dbg !17
  %x105 = load i64, ptr %x, align 4, !dbg !17
  %addtmp = add i64 %sum104, %x105, !dbg !17
  store i64 %addtmp, ptr %sum, align 4, !dbg !17
  %__idx_x106 = load i64, ptr %__idx_x, align 4, !dbg !17
  %addtmp107 = add i64 %__idx_x106, 1, !dbg !17
  store i64 %addtmp107, ptr %__idx_x, align 4, !dbg !17
  br label %loop.header, !dbg !17

v_free_block:                                     ; preds = %for.exit
  call void @free(ptr %v_data_ptr), !dbg !17
  br label %v_continue, !dbg !17

v_continue:                                       ; preds = %v_free_block, %for.exit
  ret i64 %sum108, !dbg !17
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
!1 = !DIFile(filename: "vec_with_break.vyn.ll", directory: "/home/rick/Projects/Vyn/test/vec_for")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !14, !15, !16}
!9 = !DILocalVariable(name: "v", scope: !4, file: !1, line: 3, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64, i64 }", scope: !1, file: !1, size: 192, align: 8)
!11 = !DILocalVariable(name: "sum", scope: !4, file: !1, line: 10, type: !7)
!12 = !DILocalVariable(name: "__run_once_x", scope: !4, file: !1, line: 11, type: !13)
!13 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!14 = !DILocalVariable(name: "__idx_x", scope: !4, file: !1, line: 11, type: !7)
!15 = !DILocalVariable(name: "__len_x", scope: !4, file: !1, line: 11, type: !7)
!16 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 11, type: !7)
!17 = !DILocation(line: 2, column: 1, scope: !4)
!18 = !DILocation(line: 3, column: 1, scope: !4)
!19 = !DILocation(line: 10, column: 1, scope: !4)
!20 = !DILocation(line: 11, column: 10, scope: !4)
