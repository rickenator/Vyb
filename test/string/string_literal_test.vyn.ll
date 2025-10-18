; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@1 = private unnamed_addr constant [26 x i8] c"String literal assigned: \00", align 1
@2 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@3 = private unnamed_addr constant [2 x i8] c" \00", align 1
@4 = private unnamed_addr constant [6 x i8] c"World\00", align 1
@5 = private unnamed_addr constant [22 x i8] c"Concatenated length: \00", align 1
@6 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@7 = private unnamed_addr constant [13 x i8] c"First char: \00", align 1
@8 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@9 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@10 = private unnamed_addr constant [20 x i8] c"Starts with Hello: \00", align 1

define i64 @main() !dbg !4 {
entry:
  %prefix = alloca { ptr, i64 }, align 8
  %test = alloca { ptr, i64 }, align 8
  %greeting = alloca { ptr, i64 }, align 8
  %combined = alloca { ptr, i64 }, align 8
  %data = alloca { ptr, i64 }, align 8
  store { ptr, i64 } { ptr @0, i64 5 }, ptr %data, align 8, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %data, metadata !9, metadata !DIExpression()), !dbg !16
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %data, i32 0, i32 1, !dbg !15
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !15
  %tostring = call ptr @__vyn_toString_int(i64 %str.len), !dbg !15
  %strcattmp = call ptr @__vyn_string_concat(ptr @1, ptr %tostring), !dbg !15
  call void @__vyn_println(ptr %strcattmp), !dbg !15
  %str.new_data = call ptr @malloc(i64 7), !dbg !15
  %0 = call ptr @memcpy(ptr %str.new_data, ptr @2, i64 5), !dbg !15
  %str.offset = getelementptr i8, ptr %str.new_data, i64 5, !dbg !15
  %1 = call ptr @memcpy(ptr %str.offset, ptr @3, i64 1), !dbg !15
  %str.null_pos = getelementptr i8, ptr %str.new_data, i64 6, !dbg !15
  store i8 0, ptr %str.null_pos, align 1, !dbg !15
  %str.result_data = insertvalue { ptr, i64 } undef, ptr %str.new_data, 0, !dbg !15
  %str.result_len = insertvalue { ptr, i64 } %str.result_data, i64 6, 1, !dbg !15
  %str1.data = extractvalue { ptr, i64 } %str.result_len, 0, !dbg !15
  %str1.len = extractvalue { ptr, i64 } %str.result_len, 1, !dbg !15
  %str.new_len = add i64 %str1.len, 5, !dbg !15
  %str.alloc_size = add i64 %str.new_len, 1, !dbg !15
  %str.new_data1 = call ptr @malloc(i64 %str.alloc_size), !dbg !15
  %2 = call ptr @memcpy(ptr %str.new_data1, ptr %str1.data, i64 %str1.len), !dbg !15
  %str.offset2 = getelementptr i8, ptr %str.new_data1, i64 %str1.len, !dbg !15
  %3 = call ptr @memcpy(ptr %str.offset2, ptr @4, i64 5), !dbg !15
  %str.null_pos3 = getelementptr i8, ptr %str.new_data1, i64 %str.new_len, !dbg !15
  store i8 0, ptr %str.null_pos3, align 1, !dbg !15
  %str.result_data4 = insertvalue { ptr, i64 } undef, ptr %str.new_data1, 0, !dbg !15
  %str.result_len5 = insertvalue { ptr, i64 } %str.result_data4, i64 %str.new_len, 1, !dbg !15
  store { ptr, i64 } %str.result_len5, ptr %combined, align 8, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %combined, metadata !11, metadata !DIExpression()), !dbg !17
  %str.len_ptr6 = getelementptr inbounds { ptr, i64 }, ptr %combined, i32 0, i32 1, !dbg !15
  %str.len7 = load i64, ptr %str.len_ptr6, align 4, !dbg !15
  %tostring8 = call ptr @__vyn_toString_int(i64 %str.len7), !dbg !15
  %strcattmp9 = call ptr @__vyn_string_concat(ptr @5, ptr %tostring8), !dbg !15
  call void @__vyn_println(ptr %strcattmp9), !dbg !15
  store { ptr, i64 } { ptr @6, i64 11 }, ptr %greeting, align 8, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %greeting, metadata !12, metadata !DIExpression()), !dbg !18
  %str.data_ptr = getelementptr inbounds { ptr, i64 }, ptr %greeting, i32 0, i32 0, !dbg !15
  %str.len_ptr10 = getelementptr inbounds { ptr, i64 }, ptr %greeting, i32 0, i32 1, !dbg !15
  %str.data = load ptr, ptr %str.data_ptr, align 8, !dbg !15
  %str.len11 = load i64, ptr %str.len_ptr10, align 4, !dbg !15
  %4 = icmp slt i64 0, %str.len11, !dbg !15
  %5 = and i1 true, %4, !dbg !15
  br i1 %5, label %in_bounds, label %out_of_bounds, !dbg !15

in_bounds:                                        ; preds = %entry
  %char.ptr = getelementptr i8, ptr %str.data, i64 0, !dbg !15
  %char.val = load i8, ptr %char.ptr, align 1, !dbg !15
  br label %char_at_merge, !dbg !15

out_of_bounds:                                    ; preds = %entry
  br label %char_at_merge, !dbg !15

char_at_merge:                                    ; preds = %in_bounds, %out_of_bounds
  %char.result = phi i8 [ 0, %out_of_bounds ], [ %char.val, %in_bounds ], !dbg !15
  %tostring12 = call ptr @__vyn_toString_int8(i8 %char.result), !dbg !15
  %strcattmp13 = call ptr @__vyn_string_concat(ptr @7, ptr %tostring12), !dbg !15
  call void @__vyn_println(ptr %strcattmp13), !dbg !15
  store { ptr, i64 } { ptr @8, i64 11 }, ptr %test, align 8, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %test, metadata !13, metadata !DIExpression()), !dbg !19
  store { ptr, i64 } { ptr @9, i64 5 }, ptr %prefix, align 8, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %prefix, metadata !14, metadata !DIExpression()), !dbg !20
  %prefix14 = load { ptr, i64 }, ptr %prefix, align 8, !dbg !15
  %str.data_ptr15 = getelementptr inbounds { ptr, i64 }, ptr %test, i32 0, i32 0, !dbg !15
  %str.len_ptr16 = getelementptr inbounds { ptr, i64 }, ptr %test, i32 0, i32 1, !dbg !15
  %str.data17 = load ptr, ptr %str.data_ptr15, align 8, !dbg !15
  %str.len18 = load i64, ptr %str.len_ptr16, align 4, !dbg !15
  %prefix.data = extractvalue { ptr, i64 } %prefix14, 0, !dbg !15
  %prefix.len = extractvalue { ptr, i64 } %prefix14, 1, !dbg !15
  %6 = icmp sle i64 %prefix.len, %str.len18, !dbg !15
  br i1 %6, label %len_ok, label %return_false, !dbg !15

len_ok:                                           ; preds = %char_at_merge
  %7 = icmp eq i64 %prefix.len, 0, !dbg !15
  br i1 %7, label %starts_with_merge, label %compare, !dbg !15

return_false:                                     ; preds = %char_at_merge
  br label %starts_with_merge, !dbg !15

compare:                                          ; preds = %len_ok
  %cmp.result = call i32 @memcmp(ptr %str.data17, ptr %prefix.data, i64 %prefix.len), !dbg !15
  %8 = icmp eq i32 %cmp.result, 0, !dbg !15
  br label %starts_with_merge, !dbg !15

starts_with_merge:                                ; preds = %return_false, %compare, %len_ok
  %starts_with.result = phi i1 [ false, %return_false ], [ true, %len_ok ], [ %8, %compare ], !dbg !15
  %tostring19 = call ptr @__vyn_toString_bool(i1 %starts_with.result), !dbg !15
  %strcattmp20 = call ptr @__vyn_string_concat(ptr @10, ptr %tostring19), !dbg !15
  call void @__vyn_println(ptr %strcattmp20), !dbg !15
  ret i64 0, !dbg !15
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

declare ptr @__vyn_toString_int8(i8)

declare i32 @memcmp(ptr, ptr, i64)

declare ptr @__vyn_toString_bool(i1)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "string_literal_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/string")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !13, !14}
!9 = !DILocalVariable(name: "data", scope: !4, file: !1, line: 4, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!11 = !DILocalVariable(name: "combined", scope: !4, file: !1, line: 8, type: !10)
!12 = !DILocalVariable(name: "greeting", scope: !4, file: !1, line: 12, type: !10)
!13 = !DILocalVariable(name: "test", scope: !4, file: !1, line: 16, type: !10)
!14 = !DILocalVariable(name: "prefix", scope: !4, file: !1, line: 18, type: !10)
!15 = !DILocation(line: 3, column: 1, scope: !4)
!16 = !DILocation(line: 4, column: 5, scope: !4)
!17 = !DILocation(line: 8, column: 5, scope: !4)
!18 = !DILocation(line: 12, column: 5, scope: !4)
!19 = !DILocation(line: 16, column: 5, scope: !4)
!20 = !DILocation(line: 18, column: 1, scope: !4)
