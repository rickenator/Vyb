; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [25 x i8] c"Starting String tests...\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@2 = private unnamed_addr constant [26 x i8] c"Created string from bytes\00", align 1
@3 = private unnamed_addr constant [16 x i8] c"String length: \00", align 1
@4 = private unnamed_addr constant [7 x i8] c" World\00", align 1
@5 = private unnamed_addr constant [25 x i8] c"Combined string length: \00", align 1
@6 = private unnamed_addr constant [23 x i8] c"String tests completed\00", align 1

define i64 @main() !dbg !4 {
entry:
  %combinedLen = alloca i64, align 8, !dbg !15
  %combined = alloca { ptr, i64 }, align 8, !dbg !15
  %world = alloca { ptr, i64 }, align 8, !dbg !15
  %len = alloca i64, align 8, !dbg !15
  %msg = alloca { ptr, i64 }, align 8, !dbg !15
  call void @__vyn_println(ptr @0), !dbg !15
  store { ptr, i64 } { ptr @1, i64 5 }, ptr %msg, align 8, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !9, metadata !DIExpression()), !dbg !16
  call void @__vyn_println(ptr @2), !dbg !15
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !15
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !15
  store i64 %str.len, ptr %len, align 4, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %len, metadata !11, metadata !DIExpression()), !dbg !17
  %len1 = load i64, ptr %len, align 4, !dbg !15
  %tostring = call ptr @__vyn_toString_int(i64 %len1), !dbg !15
  %strcattmp = call ptr @__vyn_string_concat(ptr @3, ptr %tostring), !dbg !15
  call void @__vyn_println(ptr %strcattmp), !dbg !15
  store { ptr, i64 } { ptr @4, i64 6 }, ptr %world, align 8, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %world, metadata !12, metadata !DIExpression()), !dbg !18
  %msg2 = load { ptr, i64 }, ptr %msg, align 8, !dbg !15
  %world3 = load { ptr, i64 }, ptr %world, align 8, !dbg !15
  %str1.data = extractvalue { ptr, i64 } %msg2, 0, !dbg !15
  %str1.len = extractvalue { ptr, i64 } %msg2, 1, !dbg !15
  %str2.data = extractvalue { ptr, i64 } %world3, 0, !dbg !15
  %str2.len = extractvalue { ptr, i64 } %world3, 1, !dbg !15
  %str.new_len = add i64 %str1.len, %str2.len, !dbg !15
  %str.alloc_size = add i64 %str.new_len, 1, !dbg !15
  %str.new_data = call ptr @malloc(i64 %str.alloc_size), !dbg !15
  %0 = call ptr @memcpy(ptr %str.new_data, ptr %str1.data, i64 %str1.len), !dbg !15
  %str.offset = getelementptr i8, ptr %str.new_data, i64 %str1.len, !dbg !15
  %1 = call ptr @memcpy(ptr %str.offset, ptr %str2.data, i64 %str2.len), !dbg !15
  %str.null_pos = getelementptr i8, ptr %str.new_data, i64 %str.new_len, !dbg !15
  store i8 0, ptr %str.null_pos, align 1, !dbg !15
  %str.result_data = insertvalue { ptr, i64 } undef, ptr %str.new_data, 0, !dbg !15
  %str.result_len = insertvalue { ptr, i64 } %str.result_data, i64 %str.new_len, 1, !dbg !15
  store { ptr, i64 } %str.result_len, ptr %combined, align 8, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %combined, metadata !13, metadata !DIExpression()), !dbg !19
  %str.len_ptr4 = getelementptr inbounds { ptr, i64 }, ptr %combined, i32 0, i32 1, !dbg !15
  %str.len5 = load i64, ptr %str.len_ptr4, align 4, !dbg !15
  store i64 %str.len5, ptr %combinedLen, align 4, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %combinedLen, metadata !14, metadata !DIExpression()), !dbg !20
  %combinedLen6 = load i64, ptr %combinedLen, align 4, !dbg !15
  %tostring7 = call ptr @__vyn_toString_int(i64 %combinedLen6), !dbg !15
  %strcattmp8 = call ptr @__vyn_string_concat(ptr @5, ptr %tostring7), !dbg !15
  call void @__vyn_println(ptr %strcattmp8), !dbg !15
  call void @__vyn_println(ptr @6), !dbg !15
  ret i64 0, !dbg !15
}

declare void @__vyn_println(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "string_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !13, !14}
!9 = !DILocalVariable(name: "msg", scope: !4, file: !1, line: 6, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!11 = !DILocalVariable(name: "len", scope: !4, file: !1, line: 10, type: !7)
!12 = !DILocalVariable(name: "world", scope: !4, file: !1, line: 14, type: !10)
!13 = !DILocalVariable(name: "combined", scope: !4, file: !1, line: 17, type: !10)
!14 = !DILocalVariable(name: "combinedLen", scope: !4, file: !1, line: 19, type: !7)
!15 = !DILocation(line: 3, column: 1, scope: !4)
!16 = !DILocation(line: 6, column: 5, scope: !4)
!17 = !DILocation(line: 10, column: 5, scope: !4)
!18 = !DILocation(line: 14, column: 5, scope: !4)
!19 = !DILocation(line: 17, column: 5, scope: !4)
!20 = !DILocation(line: 19, column: 1, scope: !4)
