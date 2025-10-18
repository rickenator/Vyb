; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [5 x i8] c"this\00", align 1
@1 = private unnamed_addr constant [5 x i8] c"that\00", align 1
@2 = private unnamed_addr constant [15 x i8] c"Concatenated: \00", align 1
@3 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@4 = private unnamed_addr constant [2 x i8] c" \00", align 1
@5 = private unnamed_addr constant [6 x i8] c"World\00", align 1
@6 = private unnamed_addr constant [17 x i8] c"Message length: \00", align 1

define i64 @main() !dbg !4 {
entry:
  %msg = alloca { ptr, i64 }, align 8, !dbg !12
  %data = alloca { ptr, i64 }, align 8, !dbg !12
  %str.new_data = call ptr @malloc(i64 9), !dbg !12
  %0 = call ptr @memcpy(ptr %str.new_data, ptr @0, i64 4), !dbg !12
  %str.offset = getelementptr i8, ptr %str.new_data, i64 4, !dbg !12
  %1 = call ptr @memcpy(ptr %str.offset, ptr @1, i64 4), !dbg !12
  %str.null_pos = getelementptr i8, ptr %str.new_data, i64 8, !dbg !12
  store i8 0, ptr %str.null_pos, align 1, !dbg !12
  %str.result_data = insertvalue { ptr, i64 } undef, ptr %str.new_data, 0, !dbg !12
  %str.result_len = insertvalue { ptr, i64 } %str.result_data, i64 8, 1, !dbg !12
  store { ptr, i64 } %str.result_len, ptr %data, align 8, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %data, metadata !9, metadata !DIExpression()), !dbg !13
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %data, i32 0, i32 1, !dbg !12
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !12
  %tostring = call ptr @__vyn_toString_int(i64 %str.len), !dbg !12
  %strcattmp = call ptr @__vyn_string_concat(ptr @2, ptr %tostring), !dbg !12
  call void @__vyn_println(ptr %strcattmp), !dbg !12
  %str.new_data1 = call ptr @malloc(i64 7), !dbg !12
  %2 = call ptr @memcpy(ptr %str.new_data1, ptr @3, i64 5), !dbg !12
  %str.offset2 = getelementptr i8, ptr %str.new_data1, i64 5, !dbg !12
  %3 = call ptr @memcpy(ptr %str.offset2, ptr @4, i64 1), !dbg !12
  %str.null_pos3 = getelementptr i8, ptr %str.new_data1, i64 6, !dbg !12
  store i8 0, ptr %str.null_pos3, align 1, !dbg !12
  %str.result_data4 = insertvalue { ptr, i64 } undef, ptr %str.new_data1, 0, !dbg !12
  %str.result_len5 = insertvalue { ptr, i64 } %str.result_data4, i64 6, 1, !dbg !12
  %str1.data = extractvalue { ptr, i64 } %str.result_len5, 0, !dbg !12
  %str1.len = extractvalue { ptr, i64 } %str.result_len5, 1, !dbg !12
  %str.new_len = add i64 %str1.len, 5, !dbg !12
  %str.alloc_size = add i64 %str.new_len, 1, !dbg !12
  %str.new_data6 = call ptr @malloc(i64 %str.alloc_size), !dbg !12
  %4 = call ptr @memcpy(ptr %str.new_data6, ptr %str1.data, i64 %str1.len), !dbg !12
  %str.offset7 = getelementptr i8, ptr %str.new_data6, i64 %str1.len, !dbg !12
  %5 = call ptr @memcpy(ptr %str.offset7, ptr @5, i64 5), !dbg !12
  %str.null_pos8 = getelementptr i8, ptr %str.new_data6, i64 %str.new_len, !dbg !12
  store i8 0, ptr %str.null_pos8, align 1, !dbg !12
  %str.result_data9 = insertvalue { ptr, i64 } undef, ptr %str.new_data6, 0, !dbg !12
  %str.result_len10 = insertvalue { ptr, i64 } %str.result_data9, i64 %str.new_len, 1, !dbg !12
  store { ptr, i64 } %str.result_len10, ptr %msg, align 8, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !11, metadata !DIExpression()), !dbg !14
  %str.len_ptr11 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !12
  %str.len12 = load i64, ptr %str.len_ptr11, align 4, !dbg !12
  %tostring13 = call ptr @__vyn_toString_int(i64 %str.len12), !dbg !12
  %strcattmp14 = call ptr @__vyn_string_concat(ptr @6, ptr %tostring13), !dbg !12
  call void @__vyn_println(ptr %strcattmp14), !dbg !12
  ret i64 0, !dbg !12
}

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "string_literal_simple.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11}
!9 = !DILocalVariable(name: "data", scope: !4, file: !1, line: 4, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!11 = !DILocalVariable(name: "msg", scope: !4, file: !1, line: 8, type: !10)
!12 = !DILocation(line: 3, column: 1, scope: !4)
!13 = !DILocation(line: 4, column: 5, scope: !4)
!14 = !DILocation(line: 8, column: 5, scope: !4)
