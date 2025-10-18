; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() !dbg !4 {
entry:
  %x = alloca i64, align 8, !dbg !17
  %__len_x = alloca i64, align 8, !dbg !17
  %__idx_x = alloca i64, align 8, !dbg !17
  %__run_once_x = alloca i1, align 1, !dbg !17
  %s = alloca i64, align 8, !dbg !17
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
  store i64 5, ptr %vec.element_ptr, align 4, !dbg !17
  %vec.new_size = add i64 %vec.reloaded_size, 1, !dbg !17
  store i64 %vec.new_size, ptr %vec.size_ptr, align 4, !dbg !17
  store i64 0, ptr %s, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %s, metadata !11, metadata !DIExpression()), !dbg !19
  br label %for.init, !dbg !17

for.init:                                         ; preds = %vec.merge
  store i1 true, ptr %__run_once_x, align 1, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %__run_once_x, metadata !12, metadata !DIExpression()), !dbg !20
  br label %for.cond, !dbg !17

for.cond:                                         ; preds = %for.update, %for.init
  %__run_once_x1 = load i1, ptr %__run_once_x, align 1, !dbg !17
  br i1 %__run_once_x1, label %for.body, label %for.exit, !dbg !17

for.body:                                         ; preds = %for.cond
  store i64 0, ptr %__idx_x, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %__idx_x, metadata !14, metadata !DIExpression()), !dbg !20
  %vec.size_ptr2 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !17
  %vec.len = load i64, ptr %vec.size_ptr2, align 4, !dbg !17
  store i64 %vec.len, ptr %__len_x, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %__len_x, metadata !15, metadata !DIExpression()), !dbg !20
  br label %loop.header, !dbg !17

for.update:                                       ; preds = %loop.exit
  store i1 false, ptr %__run_once_x, align 1, !dbg !17
  br label %for.cond, !dbg !17

for.exit:                                         ; preds = %for.cond
  %s15 = load i64, ptr %s, align 4, !dbg !17
  %v_cleanup_load = load { ptr, i64, i64 }, ptr %v, align 8, !dbg !17
  %v_data_ptr = extractvalue { ptr, i64, i64 } %v_cleanup_load, 0, !dbg !17
  %v_null_check = icmp ne ptr %v_data_ptr, null, !dbg !17
  br i1 %v_null_check, label %v_free_block, label %v_continue, !dbg !17

loop.header:                                      ; preds = %loop.body, %for.body
  %__idx_x3 = load i64, ptr %__idx_x, align 4, !dbg !17
  %__len_x4 = load i64, ptr %__len_x, align 4, !dbg !17
  %icmpslttmp = icmp slt i64 %__idx_x3, %__len_x4, !dbg !17
  br i1 %icmpslttmp, label %loop.body, label %loop.exit, !dbg !17

loop.body:                                        ; preds = %loop.header
  %__idx_x5 = load i64, ptr %__idx_x, align 4, !dbg !17
  %vec.data_ptr6 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 0, !dbg !17
  %vec.size_ptr7 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !17
  %vec.data8 = load ptr, ptr %vec.data_ptr6, align 8, !dbg !17
  %vec.size = load i64, ptr %vec.size_ptr7, align 4, !dbg !17
  %vec.offset9 = mul i64 %__idx_x5, 8, !dbg !17
  %vec.element_ptr10 = getelementptr i8, ptr %vec.data8, i64 %vec.offset9, !dbg !17
  %vec.element = load i64, ptr %vec.element_ptr10, align 4, !dbg !17
  store i64 %vec.element, ptr %x, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %x, metadata !16, metadata !DIExpression()), !dbg !20
  %s11 = load i64, ptr %s, align 4, !dbg !17
  %x12 = load i64, ptr %x, align 4, !dbg !17
  %addtmp = add i64 %s11, %x12, !dbg !17
  store i64 %addtmp, ptr %s, align 4, !dbg !17
  %__idx_x13 = load i64, ptr %__idx_x, align 4, !dbg !17
  %addtmp14 = add i64 %__idx_x13, 1, !dbg !17
  store i64 %addtmp14, ptr %__idx_x, align 4, !dbg !17
  br label %loop.header, !dbg !17

loop.exit:                                        ; preds = %loop.header
  br label %for.update, !dbg !17

v_free_block:                                     ; preds = %for.exit
  call void @free(ptr %v_data_ptr), !dbg !17
  br label %v_continue, !dbg !17

v_continue:                                       ; preds = %v_free_block, %for.exit
  ret i64 %s15, !dbg !17
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
!1 = !DIFile(filename: "minimal_vec.vyn.ll", directory: "/home/rick/Projects/Vyn/test/vec_for")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !14, !15, !16}
!9 = !DILocalVariable(name: "v", scope: !4, file: !1, line: 3, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64, i64 }", scope: !1, file: !1, size: 192, align: 8)
!11 = !DILocalVariable(name: "s", scope: !4, file: !1, line: 6, type: !7)
!12 = !DILocalVariable(name: "__run_once_x", scope: !4, file: !1, line: 7, type: !13)
!13 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!14 = !DILocalVariable(name: "__idx_x", scope: !4, file: !1, line: 7, type: !7)
!15 = !DILocalVariable(name: "__len_x", scope: !4, file: !1, line: 7, type: !7)
!16 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 7, type: !7)
!17 = !DILocation(line: 2, column: 1, scope: !4)
!18 = !DILocation(line: 3, column: 1, scope: !4)
!19 = !DILocation(line: 6, column: 1, scope: !4)
!20 = !DILocation(line: 7, column: 10, scope: !4)
