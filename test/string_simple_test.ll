; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [26 x i8] c"Testing String methods...\00", align 1
@1 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@2 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@3 = private unnamed_addr constant [6 x i8] c"World\00", align 1
@4 = private unnamed_addr constant [6 x i8] c"lo Wo\00", align 1
@5 = private unnamed_addr constant [21 x i8] c"All tests completed!\00", align 1

define i64 @main() !dbg !4 {
entry:
  %ch_lower = alloca i64, align 8, !dbg !24
  %lower = alloca { ptr, i64 }, align 8, !dbg !24
  %ch_upper = alloca i64, align 8, !dbg !24
  %upper = alloca { ptr, i64 }, align 8, !dbg !24
  %result3 = alloca i1, align 1, !dbg !24
  %sub2 = alloca { ptr, i64 }, align 8, !dbg !24
  %result2 = alloca i1, align 1, !dbg !24
  %suffix = alloca { ptr, i64 }, align 8, !dbg !24
  %result1 = alloca i1, align 1, !dbg !24
  %prefix = alloca { ptr, i64 }, align 8, !dbg !24
  %ch = alloca i64, align 8, !dbg !24
  %sub = alloca { ptr, i64 }, align 8, !dbg !24
  %msg = alloca { ptr, i64 }, align 8, !dbg !24
  call void @__vyn_println(ptr @0), !dbg !24
  store { ptr, i64 } { ptr @1, i64 11 }, ptr %msg, align 8, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !9, metadata !DIExpression()), !dbg !25
  %str.data_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !24
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !24
  %str.data = load ptr, ptr %str.data_ptr, align 8, !dbg !24
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !24
  %0 = icmp sle i64 0, %str.len, !dbg !24
  %1 = icmp sle i64 5, %str.len, !dbg !24
  %2 = and i1 true, %0, !dbg !24
  %3 = and i1 %2, true, !dbg !24
  %4 = and i1 %3, %1, !dbg !24
  br i1 %4, label %bounds_ok, label %bounds_fail, !dbg !24

bounds_ok:                                        ; preds = %entry
  %substr.data = call ptr @malloc(i64 6), !dbg !24
  %src.offset = getelementptr i8, ptr %str.data, i64 0, !dbg !24
  %5 = call ptr @memcpy(ptr %substr.data, ptr %src.offset, i64 5), !dbg !24
  %6 = getelementptr i8, ptr %substr.data, i64 5, !dbg !24
  store i8 0, ptr %6, align 1, !dbg !24
  %7 = insertvalue { ptr, i64 } undef, ptr %substr.data, 0, !dbg !24
  %8 = insertvalue { ptr, i64 } %7, i64 5, 1, !dbg !24
  br label %substr_merge, !dbg !24

bounds_fail:                                      ; preds = %entry
  br label %substr_merge, !dbg !24

substr_merge:                                     ; preds = %bounds_ok, %bounds_fail
  %substr.result = phi { ptr, i64 } [ zeroinitializer, %bounds_fail ], [ %8, %bounds_ok ], !dbg !24
  store { ptr, i64 } %substr.result, ptr %sub, align 8, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %sub, metadata !11, metadata !DIExpression()), !dbg !26
  %str.data_ptr1 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !24
  %str.len_ptr2 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !24
  %str.data3 = load ptr, ptr %str.data_ptr1, align 8, !dbg !24
  %str.len4 = load i64, ptr %str.len_ptr2, align 4, !dbg !24
  %9 = icmp slt i64 0, %str.len4, !dbg !24
  %10 = and i1 true, %9, !dbg !24
  br i1 %10, label %in_bounds, label %out_of_bounds, !dbg !24

in_bounds:                                        ; preds = %substr_merge
  %char.ptr = getelementptr i8, ptr %str.data3, i64 0, !dbg !24
  %char.val = load i8, ptr %char.ptr, align 1, !dbg !24
  br label %char_at_merge, !dbg !24

out_of_bounds:                                    ; preds = %substr_merge
  br label %char_at_merge, !dbg !24

char_at_merge:                                    ; preds = %in_bounds, %out_of_bounds
  %char.result = phi i8 [ 0, %out_of_bounds ], [ %char.val, %in_bounds ], !dbg !24
  %int_sext = sext i8 %char.result to i64, !dbg !24
  store i64 %int_sext, ptr %ch, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %ch, metadata !12, metadata !DIExpression()), !dbg !27
  store { ptr, i64 } { ptr @2, i64 5 }, ptr %prefix, align 8, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %prefix, metadata !13, metadata !DIExpression()), !dbg !28
  %prefix5 = load { ptr, i64 }, ptr %prefix, align 8, !dbg !24
  %str.data_ptr6 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !24
  %str.len_ptr7 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !24
  %str.data8 = load ptr, ptr %str.data_ptr6, align 8, !dbg !24
  %str.len9 = load i64, ptr %str.len_ptr7, align 4, !dbg !24
  %prefix.data = extractvalue { ptr, i64 } %prefix5, 0, !dbg !24
  %prefix.len = extractvalue { ptr, i64 } %prefix5, 1, !dbg !24
  %11 = icmp sle i64 %prefix.len, %str.len9, !dbg !24
  br i1 %11, label %len_ok, label %return_false, !dbg !24

len_ok:                                           ; preds = %char_at_merge
  %12 = icmp eq i64 %prefix.len, 0, !dbg !24
  br i1 %12, label %starts_with_merge, label %compare, !dbg !24

return_false:                                     ; preds = %char_at_merge
  br label %starts_with_merge, !dbg !24

compare:                                          ; preds = %len_ok
  %cmp.result = call i32 @memcmp(ptr %str.data8, ptr %prefix.data, i64 %prefix.len), !dbg !24
  %13 = icmp eq i32 %cmp.result, 0, !dbg !24
  br label %starts_with_merge, !dbg !24

starts_with_merge:                                ; preds = %return_false, %compare, %len_ok
  %starts_with.result = phi i1 [ false, %return_false ], [ true, %len_ok ], [ %13, %compare ], !dbg !24
  store i1 %starts_with.result, ptr %result1, align 1, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !14, metadata !DIExpression()), !dbg !29
  store { ptr, i64 } { ptr @3, i64 5 }, ptr %suffix, align 8, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %suffix, metadata !16, metadata !DIExpression()), !dbg !30
  %suffix10 = load { ptr, i64 }, ptr %suffix, align 8, !dbg !24
  %str.data_ptr11 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !24
  %str.len_ptr12 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !24
  %str.data13 = load ptr, ptr %str.data_ptr11, align 8, !dbg !24
  %str.len14 = load i64, ptr %str.len_ptr12, align 4, !dbg !24
  %suffix.data = extractvalue { ptr, i64 } %suffix10, 0, !dbg !24
  %suffix.len = extractvalue { ptr, i64 } %suffix10, 1, !dbg !24
  %14 = icmp sle i64 %suffix.len, %str.len14, !dbg !24
  br i1 %14, label %len_ok15, label %return_false16, !dbg !24

len_ok15:                                         ; preds = %starts_with_merge
  %15 = icmp eq i64 %suffix.len, 0, !dbg !24
  br i1 %15, label %ends_with_merge, label %compare17, !dbg !24

return_false16:                                   ; preds = %starts_with_merge
  br label %ends_with_merge, !dbg !24

compare17:                                        ; preds = %len_ok15
  %end.offset = sub i64 %str.len14, %suffix.len, !dbg !24
  %end.ptr = getelementptr i8, ptr %str.data13, i64 %end.offset, !dbg !24
  %cmp.result18 = call i32 @memcmp(ptr %end.ptr, ptr %suffix.data, i64 %suffix.len), !dbg !24
  %16 = icmp eq i32 %cmp.result18, 0, !dbg !24
  br label %ends_with_merge, !dbg !24

ends_with_merge:                                  ; preds = %return_false16, %compare17, %len_ok15
  %ends_with.result = phi i1 [ false, %return_false16 ], [ true, %len_ok15 ], [ %16, %compare17 ], !dbg !24
  store i1 %ends_with.result, ptr %result2, align 1, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !17, metadata !DIExpression()), !dbg !31
  store { ptr, i64 } { ptr @4, i64 5 }, ptr %sub2, align 8, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %sub2, metadata !18, metadata !DIExpression()), !dbg !32
  %str.data_ptr19 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !24
  %str.data20 = load ptr, ptr %str.data_ptr19, align 8, !dbg !24
  %sub221 = load { ptr, i64 }, ptr %sub2, align 8, !dbg !24
  %substring.data = extractvalue { ptr, i64 } %sub221, 0, !dbg !24
  %strstr.result = call ptr @strstr(ptr %str.data20, ptr %substring.data), !dbg !24
  %contains.result = icmp ne ptr %strstr.result, null, !dbg !24
  store i1 %contains.result, ptr %result3, align 1, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %result3, metadata !19, metadata !DIExpression()), !dbg !33
  %str.data_ptr22 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !24
  %str.len_ptr23 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !24
  %str.data24 = load ptr, ptr %str.data_ptr22, align 8, !dbg !24
  %str.len25 = load i64, ptr %str.len_ptr23, align 4, !dbg !24
  %buffer.size = add i64 %str.len25, 1, !dbg !24
  %new.data = call ptr @malloc(i64 %buffer.size), !dbg !24
  %index.ptr = alloca i64, align 8, !dbg !24
  store i64 0, ptr %index.ptr, align 4, !dbg !24
  br label %loop.cond, !dbg !24

loop.cond:                                        ; preds = %loop.body, %ends_with_merge
  %index = load i64, ptr %index.ptr, align 4, !dbg !24
  %loop.cond26 = icmp slt i64 %index, %str.len25, !dbg !24
  br i1 %loop.cond26, label %loop.body, label %loop.end, !dbg !24

loop.body:                                        ; preds = %loop.cond
  %src.ptr = getelementptr i8, ptr %str.data24, i64 %index, !dbg !24
  %ch27 = load i8, ptr %src.ptr, align 1, !dbg !24
  %17 = icmp sle i8 %ch27, 122, !dbg !24
  %18 = icmp sge i8 %ch27, 97, !dbg !24
  %is.lower = and i1 %18, %17, !dbg !24
  %upper.ch = sub i8 %ch27, 32, !dbg !24
  %converted.ch = select i1 %is.lower, i8 %upper.ch, i8 %ch27, !dbg !24
  %dst.ptr = getelementptr i8, ptr %new.data, i64 %index, !dbg !24
  store i8 %converted.ch, ptr %dst.ptr, align 1, !dbg !24
  %next.index = add i64 %index, 1, !dbg !24
  store i64 %next.index, ptr %index.ptr, align 4, !dbg !24
  br label %loop.cond, !dbg !24

loop.end:                                         ; preds = %loop.cond
  %null.term.ptr = getelementptr i8, ptr %new.data, i64 %str.len25, !dbg !24
  store i8 0, ptr %null.term.ptr, align 1, !dbg !24
  %result.data = insertvalue { ptr, i64 } undef, ptr %new.data, 0, !dbg !24
  %result.len = insertvalue { ptr, i64 } %result.data, i64 %str.len25, 1, !dbg !24
  store { ptr, i64 } %result.len, ptr %upper, align 8, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %upper, metadata !20, metadata !DIExpression()), !dbg !34
  %str.data_ptr28 = getelementptr inbounds { ptr, i64 }, ptr %upper, i32 0, i32 0, !dbg !24
  %str.len_ptr29 = getelementptr inbounds { ptr, i64 }, ptr %upper, i32 0, i32 1, !dbg !24
  %str.data30 = load ptr, ptr %str.data_ptr28, align 8, !dbg !24
  %str.len31 = load i64, ptr %str.len_ptr29, align 4, !dbg !24
  %19 = icmp slt i64 0, %str.len31, !dbg !24
  %20 = and i1 true, %19, !dbg !24
  br i1 %20, label %in_bounds32, label %out_of_bounds33, !dbg !24

in_bounds32:                                      ; preds = %loop.end
  %char.ptr35 = getelementptr i8, ptr %str.data30, i64 0, !dbg !24
  %char.val36 = load i8, ptr %char.ptr35, align 1, !dbg !24
  br label %char_at_merge34, !dbg !24

out_of_bounds33:                                  ; preds = %loop.end
  br label %char_at_merge34, !dbg !24

char_at_merge34:                                  ; preds = %in_bounds32, %out_of_bounds33
  %char.result37 = phi i8 [ 0, %out_of_bounds33 ], [ %char.val36, %in_bounds32 ], !dbg !24
  %int_sext38 = sext i8 %char.result37 to i64, !dbg !24
  store i64 %int_sext38, ptr %ch_upper, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %ch_upper, metadata !21, metadata !DIExpression()), !dbg !35
  %str.data_ptr39 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !24
  %str.len_ptr40 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !24
  %str.data41 = load ptr, ptr %str.data_ptr39, align 8, !dbg !24
  %str.len42 = load i64, ptr %str.len_ptr40, align 4, !dbg !24
  %buffer.size43 = add i64 %str.len42, 1, !dbg !24
  %new.data44 = call ptr @malloc(i64 %buffer.size43), !dbg !24
  %index.ptr48 = alloca i64, align 8, !dbg !24
  store i64 0, ptr %index.ptr48, align 4, !dbg !24
  br label %loop.cond45, !dbg !24

loop.cond45:                                      ; preds = %loop.body46, %char_at_merge34
  %index49 = load i64, ptr %index.ptr48, align 4, !dbg !24
  %loop.cond50 = icmp slt i64 %index49, %str.len42, !dbg !24
  br i1 %loop.cond50, label %loop.body46, label %loop.end47, !dbg !24

loop.body46:                                      ; preds = %loop.cond45
  %src.ptr51 = getelementptr i8, ptr %str.data41, i64 %index49, !dbg !24
  %ch52 = load i8, ptr %src.ptr51, align 1, !dbg !24
  %21 = icmp sle i8 %ch52, 90, !dbg !24
  %22 = icmp sge i8 %ch52, 65, !dbg !24
  %is.upper = and i1 %22, %21, !dbg !24
  %lower.ch = add i8 %ch52, 32, !dbg !24
  %converted.ch53 = select i1 %is.upper, i8 %lower.ch, i8 %ch52, !dbg !24
  %dst.ptr54 = getelementptr i8, ptr %new.data44, i64 %index49, !dbg !24
  store i8 %converted.ch53, ptr %dst.ptr54, align 1, !dbg !24
  %next.index55 = add i64 %index49, 1, !dbg !24
  store i64 %next.index55, ptr %index.ptr48, align 4, !dbg !24
  br label %loop.cond45, !dbg !24

loop.end47:                                       ; preds = %loop.cond45
  %null.term.ptr56 = getelementptr i8, ptr %new.data44, i64 %str.len42, !dbg !24
  store i8 0, ptr %null.term.ptr56, align 1, !dbg !24
  %result.data57 = insertvalue { ptr, i64 } undef, ptr %new.data44, 0, !dbg !24
  %result.len58 = insertvalue { ptr, i64 } %result.data57, i64 %str.len42, 1, !dbg !24
  store { ptr, i64 } %result.len58, ptr %lower, align 8, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %lower, metadata !22, metadata !DIExpression()), !dbg !36
  %str.data_ptr59 = getelementptr inbounds { ptr, i64 }, ptr %lower, i32 0, i32 0, !dbg !24
  %str.len_ptr60 = getelementptr inbounds { ptr, i64 }, ptr %lower, i32 0, i32 1, !dbg !24
  %str.data61 = load ptr, ptr %str.data_ptr59, align 8, !dbg !24
  %str.len62 = load i64, ptr %str.len_ptr60, align 4, !dbg !24
  %23 = icmp slt i64 0, %str.len62, !dbg !24
  %24 = and i1 true, %23, !dbg !24
  br i1 %24, label %in_bounds63, label %out_of_bounds64, !dbg !24

in_bounds63:                                      ; preds = %loop.end47
  %char.ptr66 = getelementptr i8, ptr %str.data61, i64 0, !dbg !24
  %char.val67 = load i8, ptr %char.ptr66, align 1, !dbg !24
  br label %char_at_merge65, !dbg !24

out_of_bounds64:                                  ; preds = %loop.end47
  br label %char_at_merge65, !dbg !24

char_at_merge65:                                  ; preds = %in_bounds63, %out_of_bounds64
  %char.result68 = phi i8 [ 0, %out_of_bounds64 ], [ %char.val67, %in_bounds63 ], !dbg !24
  %int_sext69 = sext i8 %char.result68 to i64, !dbg !24
  store i64 %int_sext69, ptr %ch_lower, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata ptr %ch_lower, metadata !23, metadata !DIExpression()), !dbg !37
  call void @__vyn_println(ptr @5), !dbg !24
  ret i64 0, !dbg !24
}

declare void @__vyn_println(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

declare i32 @memcmp(ptr, ptr, i64)

declare ptr @strstr(ptr, ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "string_simple_test.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !13, !14, !16, !17, !18, !19, !20, !21, !22, !23}
!9 = !DILocalVariable(name: "msg", scope: !4, file: !1, line: 6, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!11 = !DILocalVariable(name: "sub", scope: !4, file: !1, line: 8, type: !10)
!12 = !DILocalVariable(name: "ch", scope: !4, file: !1, line: 12, type: !7)
!13 = !DILocalVariable(name: "prefix", scope: !4, file: !1, line: 17, type: !10)
!14 = !DILocalVariable(name: "result1", scope: !4, file: !1, line: 19, type: !15)
!15 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!16 = !DILocalVariable(name: "suffix", scope: !4, file: !1, line: 23, type: !10)
!17 = !DILocalVariable(name: "result2", scope: !4, file: !1, line: 25, type: !15)
!18 = !DILocalVariable(name: "sub2", scope: !4, file: !1, line: 29, type: !10)
!19 = !DILocalVariable(name: "result3", scope: !4, file: !1, line: 31, type: !15)
!20 = !DILocalVariable(name: "upper", scope: !4, file: !1, line: 35, type: !10)
!21 = !DILocalVariable(name: "ch_upper", scope: !4, file: !1, line: 37, type: !7)
!22 = !DILocalVariable(name: "lower", scope: !4, file: !1, line: 41, type: !10)
!23 = !DILocalVariable(name: "ch_lower", scope: !4, file: !1, line: 43, type: !7)
!24 = !DILocation(line: 3, column: 1, scope: !4)
!25 = !DILocation(line: 6, column: 5, scope: !4)
!26 = !DILocation(line: 8, column: 1, scope: !4)
!27 = !DILocation(line: 12, column: 5, scope: !4)
!28 = !DILocation(line: 17, column: 5, scope: !4)
!29 = !DILocation(line: 19, column: 1, scope: !4)
!30 = !DILocation(line: 23, column: 5, scope: !4)
!31 = !DILocation(line: 25, column: 1, scope: !4)
!32 = !DILocation(line: 29, column: 5, scope: !4)
!33 = !DILocation(line: 31, column: 1, scope: !4)
!34 = !DILocation(line: 35, column: 5, scope: !4)
!35 = !DILocation(line: 37, column: 1, scope: !4)
!36 = !DILocation(line: 41, column: 5, scope: !4)
!37 = !DILocation(line: 43, column: 1, scope: !4)
