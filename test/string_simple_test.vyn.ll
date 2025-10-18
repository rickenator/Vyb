; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [26 x i8] c"Testing String methods...\00", align 1
@1 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@2 = private unnamed_addr constant [19 x i8] c"substring length: \00", align 1
@3 = private unnamed_addr constant [13 x i8] c"char_at(0): \00", align 1
@4 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@5 = private unnamed_addr constant [14 x i8] c"starts_with: \00", align 1
@6 = private unnamed_addr constant [6 x i8] c"World\00", align 1
@7 = private unnamed_addr constant [12 x i8] c"ends_with: \00", align 1
@8 = private unnamed_addr constant [6 x i8] c"lo Wo\00", align 1
@9 = private unnamed_addr constant [11 x i8] c"contains: \00", align 1
@10 = private unnamed_addr constant [22 x i8] c"to_upper first char: \00", align 1
@11 = private unnamed_addr constant [22 x i8] c"to_lower first char: \00", align 1
@12 = private unnamed_addr constant [21 x i8] c"All tests completed!\00", align 1

define i64 @main() !dbg !4 {
entry:
  %ch_lower = alloca i64, align 8, !dbg !25
  %lower = alloca { ptr, i64 }, align 8, !dbg !25
  %ch_upper = alloca i64, align 8, !dbg !25
  %upper = alloca { ptr, i64 }, align 8, !dbg !25
  %result3 = alloca i1, align 1, !dbg !25
  %sub2 = alloca { ptr, i64 }, align 8, !dbg !25
  %result2 = alloca i1, align 1, !dbg !25
  %suffix = alloca { ptr, i64 }, align 8, !dbg !25
  %result1 = alloca i1, align 1, !dbg !25
  %prefix = alloca { ptr, i64 }, align 8, !dbg !25
  %ch = alloca i64, align 8, !dbg !25
  %len = alloca i64, align 8, !dbg !25
  %sub = alloca { ptr, i64 }, align 8, !dbg !25
  %msg = alloca { ptr, i64 }, align 8, !dbg !25
  call void @__vyn_println(ptr @0), !dbg !25
  store { ptr, i64 } { ptr @1, i64 11 }, ptr %msg, align 8, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !9, metadata !DIExpression()), !dbg !26
  %str.data_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !25
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !25
  %str.data = load ptr, ptr %str.data_ptr, align 8, !dbg !25
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !25
  %0 = icmp sle i64 0, %str.len, !dbg !25
  %1 = icmp sle i64 5, %str.len, !dbg !25
  %2 = and i1 true, %0, !dbg !25
  %3 = and i1 %2, true, !dbg !25
  %4 = and i1 %3, %1, !dbg !25
  br i1 %4, label %bounds_ok, label %bounds_fail, !dbg !25

bounds_ok:                                        ; preds = %entry
  %substr.data = call ptr @malloc(i64 6), !dbg !25
  %src.offset = getelementptr i8, ptr %str.data, i64 0, !dbg !25
  %5 = call ptr @memcpy(ptr %substr.data, ptr %src.offset, i64 5), !dbg !25
  %6 = getelementptr i8, ptr %substr.data, i64 5, !dbg !25
  store i8 0, ptr %6, align 1, !dbg !25
  %7 = insertvalue { ptr, i64 } undef, ptr %substr.data, 0, !dbg !25
  %8 = insertvalue { ptr, i64 } %7, i64 5, 1, !dbg !25
  br label %substr_merge, !dbg !25

bounds_fail:                                      ; preds = %entry
  br label %substr_merge, !dbg !25

substr_merge:                                     ; preds = %bounds_ok, %bounds_fail
  %substr.result = phi { ptr, i64 } [ zeroinitializer, %bounds_fail ], [ %8, %bounds_ok ], !dbg !25
  store { ptr, i64 } %substr.result, ptr %sub, align 8, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %sub, metadata !11, metadata !DIExpression()), !dbg !27
  %str.len_ptr1 = getelementptr inbounds { ptr, i64 }, ptr %sub, i32 0, i32 1, !dbg !25
  %str.len2 = load i64, ptr %str.len_ptr1, align 4, !dbg !25
  store i64 %str.len2, ptr %len, align 4, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %len, metadata !12, metadata !DIExpression()), !dbg !28
  %len3 = load i64, ptr %len, align 4, !dbg !25
  %tostring = call ptr @__vyn_toString_int(i64 %len3), !dbg !25
  %strcattmp = call ptr @__vyn_string_concat(ptr @2, ptr %tostring), !dbg !25
  call void @__vyn_println(ptr %strcattmp), !dbg !25
  %str.data_ptr4 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !25
  %str.len_ptr5 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !25
  %str.data6 = load ptr, ptr %str.data_ptr4, align 8, !dbg !25
  %str.len7 = load i64, ptr %str.len_ptr5, align 4, !dbg !25
  %9 = icmp slt i64 0, %str.len7, !dbg !25
  %10 = and i1 true, %9, !dbg !25
  br i1 %10, label %in_bounds, label %out_of_bounds, !dbg !25

in_bounds:                                        ; preds = %substr_merge
  %char.ptr = getelementptr i8, ptr %str.data6, i64 0, !dbg !25
  %char.val = load i8, ptr %char.ptr, align 1, !dbg !25
  br label %char_at_merge, !dbg !25

out_of_bounds:                                    ; preds = %substr_merge
  br label %char_at_merge, !dbg !25

char_at_merge:                                    ; preds = %in_bounds, %out_of_bounds
  %char.result = phi i8 [ 0, %out_of_bounds ], [ %char.val, %in_bounds ], !dbg !25
  %int_sext = sext i8 %char.result to i64, !dbg !25
  store i64 %int_sext, ptr %ch, align 4, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %ch, metadata !13, metadata !DIExpression()), !dbg !29
  %ch8 = load i64, ptr %ch, align 4, !dbg !25
  %tostring9 = call ptr @__vyn_toString_int(i64 %ch8), !dbg !25
  %strcattmp10 = call ptr @__vyn_string_concat(ptr @3, ptr %tostring9), !dbg !25
  call void @__vyn_println(ptr %strcattmp10), !dbg !25
  store { ptr, i64 } { ptr @4, i64 5 }, ptr %prefix, align 8, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %prefix, metadata !14, metadata !DIExpression()), !dbg !30
  %prefix11 = load { ptr, i64 }, ptr %prefix, align 8, !dbg !25
  %str.data_ptr12 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !25
  %str.len_ptr13 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !25
  %str.data14 = load ptr, ptr %str.data_ptr12, align 8, !dbg !25
  %str.len15 = load i64, ptr %str.len_ptr13, align 4, !dbg !25
  %prefix.data = extractvalue { ptr, i64 } %prefix11, 0, !dbg !25
  %prefix.len = extractvalue { ptr, i64 } %prefix11, 1, !dbg !25
  %11 = icmp sle i64 %prefix.len, %str.len15, !dbg !25
  br i1 %11, label %len_ok, label %return_false, !dbg !25

len_ok:                                           ; preds = %char_at_merge
  %12 = icmp eq i64 %prefix.len, 0, !dbg !25
  br i1 %12, label %starts_with_merge, label %compare, !dbg !25

return_false:                                     ; preds = %char_at_merge
  br label %starts_with_merge, !dbg !25

compare:                                          ; preds = %len_ok
  %cmp.result = call i32 @memcmp(ptr %str.data14, ptr %prefix.data, i64 %prefix.len), !dbg !25
  %13 = icmp eq i32 %cmp.result, 0, !dbg !25
  br label %starts_with_merge, !dbg !25

starts_with_merge:                                ; preds = %return_false, %compare, %len_ok
  %starts_with.result = phi i1 [ false, %return_false ], [ true, %len_ok ], [ %13, %compare ], !dbg !25
  store i1 %starts_with.result, ptr %result1, align 1, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !15, metadata !DIExpression()), !dbg !31
  %result116 = load i1, ptr %result1, align 1, !dbg !25
  %tostring17 = call ptr @__vyn_toString_bool(i1 %result116), !dbg !25
  %strcattmp18 = call ptr @__vyn_string_concat(ptr @5, ptr %tostring17), !dbg !25
  call void @__vyn_println(ptr %strcattmp18), !dbg !25
  store { ptr, i64 } { ptr @6, i64 5 }, ptr %suffix, align 8, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %suffix, metadata !17, metadata !DIExpression()), !dbg !32
  %suffix19 = load { ptr, i64 }, ptr %suffix, align 8, !dbg !25
  %str.data_ptr20 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !25
  %str.len_ptr21 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !25
  %str.data22 = load ptr, ptr %str.data_ptr20, align 8, !dbg !25
  %str.len23 = load i64, ptr %str.len_ptr21, align 4, !dbg !25
  %suffix.data = extractvalue { ptr, i64 } %suffix19, 0, !dbg !25
  %suffix.len = extractvalue { ptr, i64 } %suffix19, 1, !dbg !25
  %14 = icmp sle i64 %suffix.len, %str.len23, !dbg !25
  br i1 %14, label %len_ok24, label %return_false25, !dbg !25

len_ok24:                                         ; preds = %starts_with_merge
  %15 = icmp eq i64 %suffix.len, 0, !dbg !25
  br i1 %15, label %ends_with_merge, label %compare26, !dbg !25

return_false25:                                   ; preds = %starts_with_merge
  br label %ends_with_merge, !dbg !25

compare26:                                        ; preds = %len_ok24
  %end.offset = sub i64 %str.len23, %suffix.len, !dbg !25
  %end.ptr = getelementptr i8, ptr %str.data22, i64 %end.offset, !dbg !25
  %cmp.result27 = call i32 @memcmp(ptr %end.ptr, ptr %suffix.data, i64 %suffix.len), !dbg !25
  %16 = icmp eq i32 %cmp.result27, 0, !dbg !25
  br label %ends_with_merge, !dbg !25

ends_with_merge:                                  ; preds = %return_false25, %compare26, %len_ok24
  %ends_with.result = phi i1 [ false, %return_false25 ], [ true, %len_ok24 ], [ %16, %compare26 ], !dbg !25
  store i1 %ends_with.result, ptr %result2, align 1, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !18, metadata !DIExpression()), !dbg !33
  %result228 = load i1, ptr %result2, align 1, !dbg !25
  %tostring29 = call ptr @__vyn_toString_bool(i1 %result228), !dbg !25
  %strcattmp30 = call ptr @__vyn_string_concat(ptr @7, ptr %tostring29), !dbg !25
  call void @__vyn_println(ptr %strcattmp30), !dbg !25
  store { ptr, i64 } { ptr @8, i64 5 }, ptr %sub2, align 8, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %sub2, metadata !19, metadata !DIExpression()), !dbg !34
  %str.data_ptr31 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !25
  %str.data32 = load ptr, ptr %str.data_ptr31, align 8, !dbg !25
  %sub233 = load { ptr, i64 }, ptr %sub2, align 8, !dbg !25
  %substring.data = extractvalue { ptr, i64 } %sub233, 0, !dbg !25
  %strstr.result = call ptr @strstr(ptr %str.data32, ptr %substring.data), !dbg !25
  %contains.result = icmp ne ptr %strstr.result, null, !dbg !25
  store i1 %contains.result, ptr %result3, align 1, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %result3, metadata !20, metadata !DIExpression()), !dbg !35
  %result334 = load i1, ptr %result3, align 1, !dbg !25
  %tostring35 = call ptr @__vyn_toString_bool(i1 %result334), !dbg !25
  %strcattmp36 = call ptr @__vyn_string_concat(ptr @9, ptr %tostring35), !dbg !25
  call void @__vyn_println(ptr %strcattmp36), !dbg !25
  %str.data_ptr37 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !25
  %str.len_ptr38 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !25
  %str.data39 = load ptr, ptr %str.data_ptr37, align 8, !dbg !25
  %str.len40 = load i64, ptr %str.len_ptr38, align 4, !dbg !25
  %buffer.size = add i64 %str.len40, 1, !dbg !25
  %new.data = call ptr @malloc(i64 %buffer.size), !dbg !25
  %index.ptr = alloca i64, align 8, !dbg !25
  store i64 0, ptr %index.ptr, align 4, !dbg !25
  br label %loop.cond, !dbg !25

loop.cond:                                        ; preds = %loop.body, %ends_with_merge
  %index = load i64, ptr %index.ptr, align 4, !dbg !25
  %loop.cond41 = icmp slt i64 %index, %str.len40, !dbg !25
  br i1 %loop.cond41, label %loop.body, label %loop.end, !dbg !25

loop.body:                                        ; preds = %loop.cond
  %src.ptr = getelementptr i8, ptr %str.data39, i64 %index, !dbg !25
  %ch42 = load i8, ptr %src.ptr, align 1, !dbg !25
  %17 = icmp sle i8 %ch42, 122, !dbg !25
  %18 = icmp sge i8 %ch42, 97, !dbg !25
  %is.lower = and i1 %18, %17, !dbg !25
  %upper.ch = sub i8 %ch42, 32, !dbg !25
  %converted.ch = select i1 %is.lower, i8 %upper.ch, i8 %ch42, !dbg !25
  %dst.ptr = getelementptr i8, ptr %new.data, i64 %index, !dbg !25
  store i8 %converted.ch, ptr %dst.ptr, align 1, !dbg !25
  %next.index = add i64 %index, 1, !dbg !25
  store i64 %next.index, ptr %index.ptr, align 4, !dbg !25
  br label %loop.cond, !dbg !25

loop.end:                                         ; preds = %loop.cond
  %null.term.ptr = getelementptr i8, ptr %new.data, i64 %str.len40, !dbg !25
  store i8 0, ptr %null.term.ptr, align 1, !dbg !25
  %result.data = insertvalue { ptr, i64 } undef, ptr %new.data, 0, !dbg !25
  %result.len = insertvalue { ptr, i64 } %result.data, i64 %str.len40, 1, !dbg !25
  store { ptr, i64 } %result.len, ptr %upper, align 8, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %upper, metadata !21, metadata !DIExpression()), !dbg !36
  %str.data_ptr43 = getelementptr inbounds { ptr, i64 }, ptr %upper, i32 0, i32 0, !dbg !25
  %str.len_ptr44 = getelementptr inbounds { ptr, i64 }, ptr %upper, i32 0, i32 1, !dbg !25
  %str.data45 = load ptr, ptr %str.data_ptr43, align 8, !dbg !25
  %str.len46 = load i64, ptr %str.len_ptr44, align 4, !dbg !25
  %19 = icmp slt i64 0, %str.len46, !dbg !25
  %20 = and i1 true, %19, !dbg !25
  br i1 %20, label %in_bounds47, label %out_of_bounds48, !dbg !25

in_bounds47:                                      ; preds = %loop.end
  %char.ptr50 = getelementptr i8, ptr %str.data45, i64 0, !dbg !25
  %char.val51 = load i8, ptr %char.ptr50, align 1, !dbg !25
  br label %char_at_merge49, !dbg !25

out_of_bounds48:                                  ; preds = %loop.end
  br label %char_at_merge49, !dbg !25

char_at_merge49:                                  ; preds = %in_bounds47, %out_of_bounds48
  %char.result52 = phi i8 [ 0, %out_of_bounds48 ], [ %char.val51, %in_bounds47 ], !dbg !25
  %int_sext53 = sext i8 %char.result52 to i64, !dbg !25
  store i64 %int_sext53, ptr %ch_upper, align 4, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %ch_upper, metadata !22, metadata !DIExpression()), !dbg !37
  %ch_upper54 = load i64, ptr %ch_upper, align 4, !dbg !25
  %tostring55 = call ptr @__vyn_toString_int(i64 %ch_upper54), !dbg !25
  %strcattmp56 = call ptr @__vyn_string_concat(ptr @10, ptr %tostring55), !dbg !25
  call void @__vyn_println(ptr %strcattmp56), !dbg !25
  %str.data_ptr57 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !25
  %str.len_ptr58 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !25
  %str.data59 = load ptr, ptr %str.data_ptr57, align 8, !dbg !25
  %str.len60 = load i64, ptr %str.len_ptr58, align 4, !dbg !25
  %buffer.size61 = add i64 %str.len60, 1, !dbg !25
  %new.data62 = call ptr @malloc(i64 %buffer.size61), !dbg !25
  %index.ptr66 = alloca i64, align 8, !dbg !25
  store i64 0, ptr %index.ptr66, align 4, !dbg !25
  br label %loop.cond63, !dbg !25

loop.cond63:                                      ; preds = %loop.body64, %char_at_merge49
  %index67 = load i64, ptr %index.ptr66, align 4, !dbg !25
  %loop.cond68 = icmp slt i64 %index67, %str.len60, !dbg !25
  br i1 %loop.cond68, label %loop.body64, label %loop.end65, !dbg !25

loop.body64:                                      ; preds = %loop.cond63
  %src.ptr69 = getelementptr i8, ptr %str.data59, i64 %index67, !dbg !25
  %ch70 = load i8, ptr %src.ptr69, align 1, !dbg !25
  %21 = icmp sle i8 %ch70, 90, !dbg !25
  %22 = icmp sge i8 %ch70, 65, !dbg !25
  %is.upper = and i1 %22, %21, !dbg !25
  %lower.ch = add i8 %ch70, 32, !dbg !25
  %converted.ch71 = select i1 %is.upper, i8 %lower.ch, i8 %ch70, !dbg !25
  %dst.ptr72 = getelementptr i8, ptr %new.data62, i64 %index67, !dbg !25
  store i8 %converted.ch71, ptr %dst.ptr72, align 1, !dbg !25
  %next.index73 = add i64 %index67, 1, !dbg !25
  store i64 %next.index73, ptr %index.ptr66, align 4, !dbg !25
  br label %loop.cond63, !dbg !25

loop.end65:                                       ; preds = %loop.cond63
  %null.term.ptr74 = getelementptr i8, ptr %new.data62, i64 %str.len60, !dbg !25
  store i8 0, ptr %null.term.ptr74, align 1, !dbg !25
  %result.data75 = insertvalue { ptr, i64 } undef, ptr %new.data62, 0, !dbg !25
  %result.len76 = insertvalue { ptr, i64 } %result.data75, i64 %str.len60, 1, !dbg !25
  store { ptr, i64 } %result.len76, ptr %lower, align 8, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %lower, metadata !23, metadata !DIExpression()), !dbg !38
  %str.data_ptr77 = getelementptr inbounds { ptr, i64 }, ptr %lower, i32 0, i32 0, !dbg !25
  %str.len_ptr78 = getelementptr inbounds { ptr, i64 }, ptr %lower, i32 0, i32 1, !dbg !25
  %str.data79 = load ptr, ptr %str.data_ptr77, align 8, !dbg !25
  %str.len80 = load i64, ptr %str.len_ptr78, align 4, !dbg !25
  %23 = icmp slt i64 0, %str.len80, !dbg !25
  %24 = and i1 true, %23, !dbg !25
  br i1 %24, label %in_bounds81, label %out_of_bounds82, !dbg !25

in_bounds81:                                      ; preds = %loop.end65
  %char.ptr84 = getelementptr i8, ptr %str.data79, i64 0, !dbg !25
  %char.val85 = load i8, ptr %char.ptr84, align 1, !dbg !25
  br label %char_at_merge83, !dbg !25

out_of_bounds82:                                  ; preds = %loop.end65
  br label %char_at_merge83, !dbg !25

char_at_merge83:                                  ; preds = %in_bounds81, %out_of_bounds82
  %char.result86 = phi i8 [ 0, %out_of_bounds82 ], [ %char.val85, %in_bounds81 ], !dbg !25
  %int_sext87 = sext i8 %char.result86 to i64, !dbg !25
  store i64 %int_sext87, ptr %ch_lower, align 4, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %ch_lower, metadata !24, metadata !DIExpression()), !dbg !39
  %ch_lower88 = load i64, ptr %ch_lower, align 4, !dbg !25
  %tostring89 = call ptr @__vyn_toString_int(i64 %ch_lower88), !dbg !25
  %strcattmp90 = call ptr @__vyn_string_concat(ptr @11, ptr %tostring89), !dbg !25
  call void @__vyn_println(ptr %strcattmp90), !dbg !25
  call void @__vyn_println(ptr @12), !dbg !25
  ret i64 0, !dbg !25
}

declare void @__vyn_println(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare i32 @memcmp(ptr, ptr, i64)

declare ptr @__vyn_toString_bool(i1)

declare ptr @strstr(ptr, ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "string_simple_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !13, !14, !15, !17, !18, !19, !20, !21, !22, !23, !24}
!9 = !DILocalVariable(name: "msg", scope: !4, file: !1, line: 6, type: !10)
!10 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!11 = !DILocalVariable(name: "sub", scope: !4, file: !1, line: 8, type: !10)
!12 = !DILocalVariable(name: "len", scope: !4, file: !1, line: 9, type: !7)
!13 = !DILocalVariable(name: "ch", scope: !4, file: !1, line: 12, type: !7)
!14 = !DILocalVariable(name: "prefix", scope: !4, file: !1, line: 16, type: !10)
!15 = !DILocalVariable(name: "result1", scope: !4, file: !1, line: 18, type: !16)
!16 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!17 = !DILocalVariable(name: "suffix", scope: !4, file: !1, line: 21, type: !10)
!18 = !DILocalVariable(name: "result2", scope: !4, file: !1, line: 23, type: !16)
!19 = !DILocalVariable(name: "sub2", scope: !4, file: !1, line: 26, type: !10)
!20 = !DILocalVariable(name: "result3", scope: !4, file: !1, line: 28, type: !16)
!21 = !DILocalVariable(name: "upper", scope: !4, file: !1, line: 31, type: !10)
!22 = !DILocalVariable(name: "ch_upper", scope: !4, file: !1, line: 33, type: !7)
!23 = !DILocalVariable(name: "lower", scope: !4, file: !1, line: 36, type: !10)
!24 = !DILocalVariable(name: "ch_lower", scope: !4, file: !1, line: 38, type: !7)
!25 = !DILocation(line: 3, column: 1, scope: !4)
!26 = !DILocation(line: 6, column: 5, scope: !4)
!27 = !DILocation(line: 8, column: 1, scope: !4)
!28 = !DILocation(line: 9, column: 1, scope: !4)
!29 = !DILocation(line: 12, column: 5, scope: !4)
!30 = !DILocation(line: 16, column: 5, scope: !4)
!31 = !DILocation(line: 18, column: 1, scope: !4)
!32 = !DILocation(line: 21, column: 5, scope: !4)
!33 = !DILocation(line: 23, column: 1, scope: !4)
!34 = !DILocation(line: 26, column: 5, scope: !4)
!35 = !DILocation(line: 28, column: 1, scope: !4)
!36 = !DILocation(line: 31, column: 5, scope: !4)
!37 = !DILocation(line: 33, column: 1, scope: !4)
!38 = !DILocation(line: 36, column: 5, scope: !4)
!39 = !DILocation(line: 38, column: 1, scope: !4)
