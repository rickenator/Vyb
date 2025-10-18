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
  store i64 0, ptr %s, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %s, metadata !11, metadata !DIExpression()), !dbg !19
  br label %for.init, !dbg !17

for.init:                                         ; preds = %entry
  store i1 true, ptr %__run_once_x, align 1, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %__run_once_x, metadata !12, metadata !DIExpression()), !dbg !20
  br label %for.cond, !dbg !17

for.cond:                                         ; preds = %for.update, %for.init
  %__run_once_x1 = load i1, ptr %__run_once_x, align 1, !dbg !17
  br i1 %__run_once_x1, label %for.body, label %for.exit, !dbg !17

for.body:                                         ; preds = %for.cond
  store i64 0, ptr %__idx_x, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %__idx_x, metadata !14, metadata !DIExpression()), !dbg !20
  %vec.size_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !17
  %vec.len = load i64, ptr %vec.size_ptr, align 4, !dbg !17
  store i64 %vec.len, ptr %__len_x, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %__len_x, metadata !15, metadata !DIExpression()), !dbg !20
  br label %loop.header, !dbg !17

for.update:                                       ; preds = %loop.exit
  store i1 false, ptr %__run_once_x, align 1, !dbg !17
  br label %for.cond, !dbg !17

for.exit:                                         ; preds = %for.cond
  %s10 = load i64, ptr %s, align 4, !dbg !17
  %v_cleanup_load = load { ptr, i64, i64 }, ptr %v, align 8, !dbg !17
  %v_data_ptr = extractvalue { ptr, i64, i64 } %v_cleanup_load, 0, !dbg !17
  %v_null_check = icmp ne ptr %v_data_ptr, null, !dbg !17
  br i1 %v_null_check, label %v_free_block, label %v_continue, !dbg !17

loop.header:                                      ; preds = %loop.body, %for.body
  %__idx_x2 = load i64, ptr %__idx_x, align 4, !dbg !17
  %__len_x3 = load i64, ptr %__len_x, align 4, !dbg !17
  %icmpslttmp = icmp slt i64 %__idx_x2, %__len_x3, !dbg !17
  br i1 %icmpslttmp, label %loop.body, label %loop.exit, !dbg !17

loop.body:                                        ; preds = %loop.header
  %__idx_x4 = load i64, ptr %__idx_x, align 4, !dbg !17
  %vec.data_ptr = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 0, !dbg !17
  %vec.size_ptr5 = getelementptr inbounds { ptr, i64, i64 }, ptr %v, i32 0, i32 1, !dbg !17
  %vec.data = load ptr, ptr %vec.data_ptr, align 8, !dbg !17
  %vec.size = load i64, ptr %vec.size_ptr5, align 4, !dbg !17
  %vec.offset = mul i64 %__idx_x4, 8, !dbg !17
  %vec.element_ptr = getelementptr i8, ptr %vec.data, i64 %vec.offset, !dbg !17
  %vec.element = load i64, ptr %vec.element_ptr, align 4, !dbg !17
  store i64 %vec.element, ptr %x, align 4, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %x, metadata !16, metadata !DIExpression()), !dbg !20
  %s6 = load i64, ptr %s, align 4, !dbg !17
  %x7 = load i64, ptr %x, align 4, !dbg !17
  %addtmp = add i64 %s6, %x7, !dbg !17
  store i64 %addtmp, ptr %s, align 4, !dbg !17
  %__idx_x8 = load i64, ptr %__idx_x, align 4, !dbg !17
  %addtmp9 = add i64 %__idx_x8, 1, !dbg !17
  store i64 %addtmp9, ptr %__idx_x, align 4, !dbg !17
  br label %loop.header, !dbg !17

loop.exit:                                        ; preds = %loop.header
  br label %for.update, !dbg !17

v_free_block:                                     ; preds = %for.exit
  call void @free(ptr %v_data_ptr), !dbg !17
  br label %v_continue, !dbg !17

v_continue:                                       ; preds = %v_free_block, %for.exit
  ret i64 %s10, !dbg !17
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare void @free(ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "empty_vec.vyn.ll", directory: "/home/rick/Projects/Vyn/test/vec_for")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !14, !15, !16}
!9 = !DILocalVariable(name: "v", scope: !4, file: !1, line: 2, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64, i64 }", scope: !1, file: !1, size: 192, align: 8)
!11 = !DILocalVariable(name: "s", scope: !4, file: !1, line: 3, type: !7)
!12 = !DILocalVariable(name: "__run_once_x", scope: !4, file: !1, line: 4, type: !13)
!13 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!14 = !DILocalVariable(name: "__idx_x", scope: !4, file: !1, line: 4, type: !7)
!15 = !DILocalVariable(name: "__len_x", scope: !4, file: !1, line: 4, type: !7)
!16 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 4, type: !7)
!17 = !DILocation(line: 1, column: 1, scope: !4)
!18 = !DILocation(line: 2, column: 1, scope: !4)
!19 = !DILocation(line: 3, column: 1, scope: !4)
!20 = !DILocation(line: 4, column: 10, scope: !4)
