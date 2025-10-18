; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@2 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@3 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@4 = private unnamed_addr constant [6 x i8] c"World\00", align 1
@5 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@6 = private unnamed_addr constant [6 x i8] c"World\00", align 1
@7 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@8 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@9 = private unnamed_addr constant [6 x i8] c"lo Wo\00", align 1
@10 = private unnamed_addr constant [4 x i8] c"xyz\00", align 1
@11 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@12 = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@13 = private unnamed_addr constant [28 x i8] c"=== String::substring() ===\00", align 1
@14 = private unnamed_addr constant [26 x i8] c"=== String::char_at() ===\00", align 1
@15 = private unnamed_addr constant [30 x i8] c"=== String::starts_with() ===\00", align 1
@16 = private unnamed_addr constant [28 x i8] c"=== String::ends_with() ===\00", align 1
@17 = private unnamed_addr constant [27 x i8] c"=== String::contains() ===\00", align 1
@18 = private unnamed_addr constant [27 x i8] c"=== String::to_upper() ===\00", align 1
@19 = private unnamed_addr constant [27 x i8] c"=== String::to_lower() ===\00", align 1

define void @test_substring() !dbg !4 {
entry:
  %sub3 = alloca { ptr, i64 }, align 8
  %sub2 = alloca { ptr, i64 }, align 8
  %sub1 = alloca { ptr, i64 }, align 8
  %msg = alloca { ptr, i64 }, align 8
  store { ptr, i64 } { ptr @0, i64 11 }, ptr %msg, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !8, metadata !DIExpression()), !dbg !14
  %str.data_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !13
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !13
  %str.data = load ptr, ptr %str.data_ptr, align 8, !dbg !13
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !13
  %0 = icmp sle i64 0, %str.len, !dbg !13
  %1 = icmp sle i64 5, %str.len, !dbg !13
  %2 = and i1 true, %0, !dbg !13
  %3 = and i1 %2, true, !dbg !13
  %4 = and i1 %3, %1, !dbg !13
  br i1 %4, label %bounds_ok, label %bounds_fail, !dbg !13

bounds_ok:                                        ; preds = %entry
  %substr.data = call ptr @malloc(i64 6), !dbg !13
  %src.offset = getelementptr i8, ptr %str.data, i64 0, !dbg !13
  %5 = call ptr @memcpy(ptr %substr.data, ptr %src.offset, i64 5), !dbg !13
  %6 = getelementptr i8, ptr %substr.data, i64 5, !dbg !13
  store i8 0, ptr %6, align 1, !dbg !13
  %7 = insertvalue { ptr, i64 } undef, ptr %substr.data, 0, !dbg !13
  %8 = insertvalue { ptr, i64 } %7, i64 5, 1, !dbg !13
  br label %substr_merge, !dbg !13

bounds_fail:                                      ; preds = %entry
  br label %substr_merge, !dbg !13

substr_merge:                                     ; preds = %bounds_ok, %bounds_fail
  %substr.result = phi { ptr, i64 } [ zeroinitializer, %bounds_fail ], [ %8, %bounds_ok ], !dbg !13
  store { ptr, i64 } %substr.result, ptr %sub1, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %sub1, metadata !10, metadata !DIExpression()), !dbg !15
  %str.data_ptr1 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !13
  %str.len_ptr2 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !13
  %str.data3 = load ptr, ptr %str.data_ptr1, align 8, !dbg !13
  %str.len4 = load i64, ptr %str.len_ptr2, align 4, !dbg !13
  %9 = icmp sle i64 6, %str.len4, !dbg !13
  %10 = icmp sle i64 11, %str.len4, !dbg !13
  %11 = and i1 true, %9, !dbg !13
  %12 = and i1 %11, true, !dbg !13
  %13 = and i1 %12, %10, !dbg !13
  br i1 %13, label %bounds_ok5, label %bounds_fail6, !dbg !13

bounds_ok5:                                       ; preds = %substr_merge
  %substr.data8 = call ptr @malloc(i64 6), !dbg !13
  %src.offset9 = getelementptr i8, ptr %str.data3, i64 6, !dbg !13
  %14 = call ptr @memcpy(ptr %substr.data8, ptr %src.offset9, i64 5), !dbg !13
  %15 = getelementptr i8, ptr %substr.data8, i64 5, !dbg !13
  store i8 0, ptr %15, align 1, !dbg !13
  %16 = insertvalue { ptr, i64 } undef, ptr %substr.data8, 0, !dbg !13
  %17 = insertvalue { ptr, i64 } %16, i64 5, 1, !dbg !13
  br label %substr_merge7, !dbg !13

bounds_fail6:                                     ; preds = %substr_merge
  br label %substr_merge7, !dbg !13

substr_merge7:                                    ; preds = %bounds_ok5, %bounds_fail6
  %substr.result10 = phi { ptr, i64 } [ zeroinitializer, %bounds_fail6 ], [ %17, %bounds_ok5 ], !dbg !13
  store { ptr, i64 } %substr.result10, ptr %sub2, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %sub2, metadata !11, metadata !DIExpression()), !dbg !16
  %str.data_ptr11 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !13
  %str.len_ptr12 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !13
  %str.data13 = load ptr, ptr %str.data_ptr11, align 8, !dbg !13
  %str.len14 = load i64, ptr %str.len_ptr12, align 4, !dbg !13
  %18 = icmp sle i64 6, %str.len14, !dbg !13
  %19 = icmp sge i64 %str.len14, 6, !dbg !13
  %20 = icmp sle i64 %str.len14, %str.len14, !dbg !13
  %21 = and i1 true, %18, !dbg !13
  %22 = and i1 %21, %19, !dbg !13
  %23 = and i1 %22, %20, !dbg !13
  br i1 %23, label %bounds_ok15, label %bounds_fail16, !dbg !13

bounds_ok15:                                      ; preds = %substr_merge7
  %substr.len = sub i64 %str.len14, 6, !dbg !13
  %24 = add i64 %substr.len, 1, !dbg !13
  %substr.data18 = call ptr @malloc(i64 %24), !dbg !13
  %src.offset19 = getelementptr i8, ptr %str.data13, i64 6, !dbg !13
  %25 = call ptr @memcpy(ptr %substr.data18, ptr %src.offset19, i64 %substr.len), !dbg !13
  %26 = getelementptr i8, ptr %substr.data18, i64 %substr.len, !dbg !13
  store i8 0, ptr %26, align 1, !dbg !13
  %27 = insertvalue { ptr, i64 } undef, ptr %substr.data18, 0, !dbg !13
  %28 = insertvalue { ptr, i64 } %27, i64 %substr.len, 1, !dbg !13
  br label %substr_merge17, !dbg !13

bounds_fail16:                                    ; preds = %substr_merge7
  br label %substr_merge17, !dbg !13

substr_merge17:                                   ; preds = %bounds_ok15, %bounds_fail16
  %substr.result20 = phi { ptr, i64 } [ zeroinitializer, %bounds_fail16 ], [ %28, %bounds_ok15 ], !dbg !13
  store { ptr, i64 } %substr.result20, ptr %sub3, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %sub3, metadata !12, metadata !DIExpression()), !dbg !17
  ret void, !dbg !13
}

define void @test_char_at() !dbg !18 {
entry:
  %ch3 = alloca i64, align 8
  %ch2 = alloca i64, align 8
  %ch1 = alloca i64, align 8
  %msg = alloca { ptr, i64 }, align 8
  store { ptr, i64 } { ptr @1, i64 5 }, ptr %msg, align 8, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !20, metadata !DIExpression()), !dbg !26
  %str.data_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !25
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !25
  %str.data = load ptr, ptr %str.data_ptr, align 8, !dbg !25
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !25
  %0 = icmp slt i64 0, %str.len, !dbg !25
  %1 = and i1 true, %0, !dbg !25
  br i1 %1, label %in_bounds, label %out_of_bounds, !dbg !25

in_bounds:                                        ; preds = %entry
  %char.ptr = getelementptr i8, ptr %str.data, i64 0, !dbg !25
  %char.val = load i8, ptr %char.ptr, align 1, !dbg !25
  br label %char_at_merge, !dbg !25

out_of_bounds:                                    ; preds = %entry
  br label %char_at_merge, !dbg !25

char_at_merge:                                    ; preds = %in_bounds, %out_of_bounds
  %char.result = phi i8 [ 0, %out_of_bounds ], [ %char.val, %in_bounds ], !dbg !25
  %int_sext = sext i8 %char.result to i64, !dbg !25
  store i64 %int_sext, ptr %ch1, align 4, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %ch1, metadata !21, metadata !DIExpression()), !dbg !27
  %str.data_ptr1 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !25
  %str.len_ptr2 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !25
  %str.data3 = load ptr, ptr %str.data_ptr1, align 8, !dbg !25
  %str.len4 = load i64, ptr %str.len_ptr2, align 4, !dbg !25
  %2 = icmp slt i64 4, %str.len4, !dbg !25
  %3 = and i1 true, %2, !dbg !25
  br i1 %3, label %in_bounds5, label %out_of_bounds6, !dbg !25

in_bounds5:                                       ; preds = %char_at_merge
  %char.ptr8 = getelementptr i8, ptr %str.data3, i64 4, !dbg !25
  %char.val9 = load i8, ptr %char.ptr8, align 1, !dbg !25
  br label %char_at_merge7, !dbg !25

out_of_bounds6:                                   ; preds = %char_at_merge
  br label %char_at_merge7, !dbg !25

char_at_merge7:                                   ; preds = %in_bounds5, %out_of_bounds6
  %char.result10 = phi i8 [ 0, %out_of_bounds6 ], [ %char.val9, %in_bounds5 ], !dbg !25
  %int_sext11 = sext i8 %char.result10 to i64, !dbg !25
  store i64 %int_sext11, ptr %ch2, align 4, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %ch2, metadata !23, metadata !DIExpression()), !dbg !28
  %str.data_ptr12 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !25
  %str.len_ptr13 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !25
  %str.data14 = load ptr, ptr %str.data_ptr12, align 8, !dbg !25
  %str.len15 = load i64, ptr %str.len_ptr13, align 4, !dbg !25
  %4 = icmp slt i64 10, %str.len15, !dbg !25
  %5 = and i1 true, %4, !dbg !25
  br i1 %5, label %in_bounds16, label %out_of_bounds17, !dbg !25

in_bounds16:                                      ; preds = %char_at_merge7
  %char.ptr19 = getelementptr i8, ptr %str.data14, i64 10, !dbg !25
  %char.val20 = load i8, ptr %char.ptr19, align 1, !dbg !25
  br label %char_at_merge18, !dbg !25

out_of_bounds17:                                  ; preds = %char_at_merge7
  br label %char_at_merge18, !dbg !25

char_at_merge18:                                  ; preds = %in_bounds16, %out_of_bounds17
  %char.result21 = phi i8 [ 0, %out_of_bounds17 ], [ %char.val20, %in_bounds16 ], !dbg !25
  %int_sext22 = sext i8 %char.result21 to i64, !dbg !25
  store i64 %int_sext22, ptr %ch3, align 4, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %ch3, metadata !24, metadata !DIExpression()), !dbg !29
  ret void, !dbg !25
}

define void @test_starts_with() !dbg !30 {
entry:
  %result2 = alloca i1, align 1
  %result1 = alloca i1, align 1
  %prefix2 = alloca { ptr, i64 }, align 8
  %prefix1 = alloca { ptr, i64 }, align 8
  %msg = alloca { ptr, i64 }, align 8
  store { ptr, i64 } { ptr @2, i64 11 }, ptr %msg, align 8, !dbg !38
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !32, metadata !DIExpression()), !dbg !39
  store { ptr, i64 } { ptr @3, i64 5 }, ptr %prefix1, align 8, !dbg !38
  call void @llvm.dbg.declare(metadata ptr %prefix1, metadata !33, metadata !DIExpression()), !dbg !40
  store { ptr, i64 } { ptr @4, i64 5 }, ptr %prefix2, align 8, !dbg !38
  call void @llvm.dbg.declare(metadata ptr %prefix2, metadata !34, metadata !DIExpression()), !dbg !41
  %prefix11 = load { ptr, i64 }, ptr %prefix1, align 8, !dbg !38
  %str.data_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !38
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !38
  %str.data = load ptr, ptr %str.data_ptr, align 8, !dbg !38
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !38
  %prefix.data = extractvalue { ptr, i64 } %prefix11, 0, !dbg !38
  %prefix.len = extractvalue { ptr, i64 } %prefix11, 1, !dbg !38
  %0 = icmp sle i64 %prefix.len, %str.len, !dbg !38
  br i1 %0, label %len_ok, label %return_false, !dbg !38

len_ok:                                           ; preds = %entry
  %1 = icmp eq i64 %prefix.len, 0, !dbg !38
  br i1 %1, label %starts_with_merge, label %compare, !dbg !38

return_false:                                     ; preds = %entry
  br label %starts_with_merge, !dbg !38

compare:                                          ; preds = %len_ok
  %cmp.result = call i32 @memcmp(ptr %str.data, ptr %prefix.data, i64 %prefix.len), !dbg !38
  %2 = icmp eq i32 %cmp.result, 0, !dbg !38
  br label %starts_with_merge, !dbg !38

starts_with_merge:                                ; preds = %return_false, %compare, %len_ok
  %starts_with.result = phi i1 [ false, %return_false ], [ true, %len_ok ], [ %2, %compare ], !dbg !38
  store i1 %starts_with.result, ptr %result1, align 1, !dbg !38
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !35, metadata !DIExpression()), !dbg !42
  %prefix22 = load { ptr, i64 }, ptr %prefix2, align 8, !dbg !38
  %str.data_ptr3 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !38
  %str.len_ptr4 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !38
  %str.data5 = load ptr, ptr %str.data_ptr3, align 8, !dbg !38
  %str.len6 = load i64, ptr %str.len_ptr4, align 4, !dbg !38
  %prefix.data7 = extractvalue { ptr, i64 } %prefix22, 0, !dbg !38
  %prefix.len8 = extractvalue { ptr, i64 } %prefix22, 1, !dbg !38
  %3 = icmp sle i64 %prefix.len8, %str.len6, !dbg !38
  br i1 %3, label %len_ok9, label %return_false10, !dbg !38

len_ok9:                                          ; preds = %starts_with_merge
  %4 = icmp eq i64 %prefix.len8, 0, !dbg !38
  br i1 %4, label %starts_with_merge12, label %compare11, !dbg !38

return_false10:                                   ; preds = %starts_with_merge
  br label %starts_with_merge12, !dbg !38

compare11:                                        ; preds = %len_ok9
  %cmp.result13 = call i32 @memcmp(ptr %str.data5, ptr %prefix.data7, i64 %prefix.len8), !dbg !38
  %5 = icmp eq i32 %cmp.result13, 0, !dbg !38
  br label %starts_with_merge12, !dbg !38

starts_with_merge12:                              ; preds = %return_false10, %compare11, %len_ok9
  %starts_with.result14 = phi i1 [ false, %return_false10 ], [ true, %len_ok9 ], [ %5, %compare11 ], !dbg !38
  store i1 %starts_with.result14, ptr %result2, align 1, !dbg !38
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !37, metadata !DIExpression()), !dbg !43
  ret void, !dbg !38
}

define void @test_ends_with() !dbg !44 {
entry:
  %result2 = alloca i1, align 1
  %result1 = alloca i1, align 1
  %suffix2 = alloca { ptr, i64 }, align 8
  %suffix1 = alloca { ptr, i64 }, align 8
  %msg = alloca { ptr, i64 }, align 8
  store { ptr, i64 } { ptr @5, i64 11 }, ptr %msg, align 8, !dbg !51
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !46, metadata !DIExpression()), !dbg !52
  store { ptr, i64 } { ptr @6, i64 5 }, ptr %suffix1, align 8, !dbg !51
  call void @llvm.dbg.declare(metadata ptr %suffix1, metadata !47, metadata !DIExpression()), !dbg !53
  store { ptr, i64 } { ptr @7, i64 5 }, ptr %suffix2, align 8, !dbg !51
  call void @llvm.dbg.declare(metadata ptr %suffix2, metadata !48, metadata !DIExpression()), !dbg !54
  %suffix11 = load { ptr, i64 }, ptr %suffix1, align 8, !dbg !51
  %str.data_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !51
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !51
  %str.data = load ptr, ptr %str.data_ptr, align 8, !dbg !51
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !51
  %suffix.data = extractvalue { ptr, i64 } %suffix11, 0, !dbg !51
  %suffix.len = extractvalue { ptr, i64 } %suffix11, 1, !dbg !51
  %0 = icmp sle i64 %suffix.len, %str.len, !dbg !51
  br i1 %0, label %len_ok, label %return_false, !dbg !51

len_ok:                                           ; preds = %entry
  %1 = icmp eq i64 %suffix.len, 0, !dbg !51
  br i1 %1, label %ends_with_merge, label %compare, !dbg !51

return_false:                                     ; preds = %entry
  br label %ends_with_merge, !dbg !51

compare:                                          ; preds = %len_ok
  %end.offset = sub i64 %str.len, %suffix.len, !dbg !51
  %end.ptr = getelementptr i8, ptr %str.data, i64 %end.offset, !dbg !51
  %cmp.result = call i32 @memcmp(ptr %end.ptr, ptr %suffix.data, i64 %suffix.len), !dbg !51
  %2 = icmp eq i32 %cmp.result, 0, !dbg !51
  br label %ends_with_merge, !dbg !51

ends_with_merge:                                  ; preds = %return_false, %compare, %len_ok
  %ends_with.result = phi i1 [ false, %return_false ], [ true, %len_ok ], [ %2, %compare ], !dbg !51
  store i1 %ends_with.result, ptr %result1, align 1, !dbg !51
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !49, metadata !DIExpression()), !dbg !55
  %suffix22 = load { ptr, i64 }, ptr %suffix2, align 8, !dbg !51
  %str.data_ptr3 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !51
  %str.len_ptr4 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !51
  %str.data5 = load ptr, ptr %str.data_ptr3, align 8, !dbg !51
  %str.len6 = load i64, ptr %str.len_ptr4, align 4, !dbg !51
  %suffix.data7 = extractvalue { ptr, i64 } %suffix22, 0, !dbg !51
  %suffix.len8 = extractvalue { ptr, i64 } %suffix22, 1, !dbg !51
  %3 = icmp sle i64 %suffix.len8, %str.len6, !dbg !51
  br i1 %3, label %len_ok9, label %return_false10, !dbg !51

len_ok9:                                          ; preds = %ends_with_merge
  %4 = icmp eq i64 %suffix.len8, 0, !dbg !51
  br i1 %4, label %ends_with_merge12, label %compare11, !dbg !51

return_false10:                                   ; preds = %ends_with_merge
  br label %ends_with_merge12, !dbg !51

compare11:                                        ; preds = %len_ok9
  %end.offset13 = sub i64 %str.len6, %suffix.len8, !dbg !51
  %end.ptr14 = getelementptr i8, ptr %str.data5, i64 %end.offset13, !dbg !51
  %cmp.result15 = call i32 @memcmp(ptr %end.ptr14, ptr %suffix.data7, i64 %suffix.len8), !dbg !51
  %5 = icmp eq i32 %cmp.result15, 0, !dbg !51
  br label %ends_with_merge12, !dbg !51

ends_with_merge12:                                ; preds = %return_false10, %compare11, %len_ok9
  %ends_with.result16 = phi i1 [ false, %return_false10 ], [ true, %len_ok9 ], [ %5, %compare11 ], !dbg !51
  store i1 %ends_with.result16, ptr %result2, align 1, !dbg !51
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !50, metadata !DIExpression()), !dbg !56
  ret void, !dbg !51
}

define void @test_contains() !dbg !57 {
entry:
  %result2 = alloca i1, align 1
  %result1 = alloca i1, align 1
  %sub2 = alloca { ptr, i64 }, align 8
  %sub1 = alloca { ptr, i64 }, align 8
  %msg = alloca { ptr, i64 }, align 8
  store { ptr, i64 } { ptr @8, i64 11 }, ptr %msg, align 8, !dbg !64
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !59, metadata !DIExpression()), !dbg !65
  store { ptr, i64 } { ptr @9, i64 5 }, ptr %sub1, align 8, !dbg !64
  call void @llvm.dbg.declare(metadata ptr %sub1, metadata !60, metadata !DIExpression()), !dbg !66
  store { ptr, i64 } { ptr @10, i64 3 }, ptr %sub2, align 8, !dbg !64
  call void @llvm.dbg.declare(metadata ptr %sub2, metadata !61, metadata !DIExpression()), !dbg !67
  %str.data_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !64
  %str.data = load ptr, ptr %str.data_ptr, align 8, !dbg !64
  %sub11 = load { ptr, i64 }, ptr %sub1, align 8, !dbg !64
  %substring.data = extractvalue { ptr, i64 } %sub11, 0, !dbg !64
  %strstr.result = call ptr @strstr(ptr %str.data, ptr %substring.data), !dbg !64
  %contains.result = icmp ne ptr %strstr.result, null, !dbg !64
  store i1 %contains.result, ptr %result1, align 1, !dbg !64
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !62, metadata !DIExpression()), !dbg !68
  %str.data_ptr2 = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !64
  %str.data3 = load ptr, ptr %str.data_ptr2, align 8, !dbg !64
  %sub24 = load { ptr, i64 }, ptr %sub2, align 8, !dbg !64
  %substring.data5 = extractvalue { ptr, i64 } %sub24, 0, !dbg !64
  %strstr.result6 = call ptr @strstr(ptr %str.data3, ptr %substring.data5), !dbg !64
  %contains.result7 = icmp ne ptr %strstr.result6, null, !dbg !64
  store i1 %contains.result7, ptr %result2, align 1, !dbg !64
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !63, metadata !DIExpression()), !dbg !69
  ret void, !dbg !64
}

define void @test_to_upper() !dbg !70 {
entry:
  %ch6 = alloca i64, align 8
  %upper = alloca { ptr, i64 }, align 8
  %msg = alloca { ptr, i64 }, align 8
  store { ptr, i64 } { ptr @11, i64 11 }, ptr %msg, align 8, !dbg !75
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !72, metadata !DIExpression()), !dbg !76
  %str.data_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !75
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !75
  %str.data = load ptr, ptr %str.data_ptr, align 8, !dbg !75
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !75
  %buffer.size = add i64 %str.len, 1, !dbg !75
  %new.data = call ptr @malloc(i64 %buffer.size), !dbg !75
  %index.ptr = alloca i64, align 8, !dbg !75
  store i64 0, ptr %index.ptr, align 4, !dbg !75
  br label %loop.cond, !dbg !75

loop.cond:                                        ; preds = %loop.body, %entry
  %index = load i64, ptr %index.ptr, align 4, !dbg !75
  %loop.cond1 = icmp slt i64 %index, %str.len, !dbg !75
  br i1 %loop.cond1, label %loop.body, label %loop.end, !dbg !75

loop.body:                                        ; preds = %loop.cond
  %src.ptr = getelementptr i8, ptr %str.data, i64 %index, !dbg !75
  %ch = load i8, ptr %src.ptr, align 1, !dbg !75
  %0 = icmp sle i8 %ch, 122, !dbg !75
  %1 = icmp sge i8 %ch, 97, !dbg !75
  %is.lower = and i1 %1, %0, !dbg !75
  %upper.ch = sub i8 %ch, 32, !dbg !75
  %converted.ch = select i1 %is.lower, i8 %upper.ch, i8 %ch, !dbg !75
  %dst.ptr = getelementptr i8, ptr %new.data, i64 %index, !dbg !75
  store i8 %converted.ch, ptr %dst.ptr, align 1, !dbg !75
  %next.index = add i64 %index, 1, !dbg !75
  store i64 %next.index, ptr %index.ptr, align 4, !dbg !75
  br label %loop.cond, !dbg !75

loop.end:                                         ; preds = %loop.cond
  %null.term.ptr = getelementptr i8, ptr %new.data, i64 %str.len, !dbg !75
  store i8 0, ptr %null.term.ptr, align 1, !dbg !75
  %result.data = insertvalue { ptr, i64 } undef, ptr %new.data, 0, !dbg !75
  %result.len = insertvalue { ptr, i64 } %result.data, i64 %str.len, 1, !dbg !75
  store { ptr, i64 } %result.len, ptr %upper, align 8, !dbg !75
  call void @llvm.dbg.declare(metadata ptr %upper, metadata !73, metadata !DIExpression()), !dbg !77
  %str.data_ptr2 = getelementptr inbounds { ptr, i64 }, ptr %upper, i32 0, i32 0, !dbg !75
  %str.len_ptr3 = getelementptr inbounds { ptr, i64 }, ptr %upper, i32 0, i32 1, !dbg !75
  %str.data4 = load ptr, ptr %str.data_ptr2, align 8, !dbg !75
  %str.len5 = load i64, ptr %str.len_ptr3, align 4, !dbg !75
  %2 = icmp slt i64 0, %str.len5, !dbg !75
  %3 = and i1 true, %2, !dbg !75
  br i1 %3, label %in_bounds, label %out_of_bounds, !dbg !75

in_bounds:                                        ; preds = %loop.end
  %char.ptr = getelementptr i8, ptr %str.data4, i64 0, !dbg !75
  %char.val = load i8, ptr %char.ptr, align 1, !dbg !75
  br label %char_at_merge, !dbg !75

out_of_bounds:                                    ; preds = %loop.end
  br label %char_at_merge, !dbg !75

char_at_merge:                                    ; preds = %in_bounds, %out_of_bounds
  %char.result = phi i8 [ 0, %out_of_bounds ], [ %char.val, %in_bounds ], !dbg !75
  %int_sext = sext i8 %char.result to i64, !dbg !75
  store i64 %int_sext, ptr %ch6, align 4, !dbg !75
  call void @llvm.dbg.declare(metadata ptr %ch6, metadata !74, metadata !DIExpression()), !dbg !78
  ret void, !dbg !75
}

define void @test_to_lower() !dbg !79 {
entry:
  %ch6 = alloca i64, align 8
  %lower = alloca { ptr, i64 }, align 8
  %msg = alloca { ptr, i64 }, align 8
  store { ptr, i64 } { ptr @12, i64 11 }, ptr %msg, align 8, !dbg !84
  call void @llvm.dbg.declare(metadata ptr %msg, metadata !81, metadata !DIExpression()), !dbg !85
  %str.data_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 0, !dbg !84
  %str.len_ptr = getelementptr inbounds { ptr, i64 }, ptr %msg, i32 0, i32 1, !dbg !84
  %str.data = load ptr, ptr %str.data_ptr, align 8, !dbg !84
  %str.len = load i64, ptr %str.len_ptr, align 4, !dbg !84
  %buffer.size = add i64 %str.len, 1, !dbg !84
  %new.data = call ptr @malloc(i64 %buffer.size), !dbg !84
  %index.ptr = alloca i64, align 8, !dbg !84
  store i64 0, ptr %index.ptr, align 4, !dbg !84
  br label %loop.cond, !dbg !84

loop.cond:                                        ; preds = %loop.body, %entry
  %index = load i64, ptr %index.ptr, align 4, !dbg !84
  %loop.cond1 = icmp slt i64 %index, %str.len, !dbg !84
  br i1 %loop.cond1, label %loop.body, label %loop.end, !dbg !84

loop.body:                                        ; preds = %loop.cond
  %src.ptr = getelementptr i8, ptr %str.data, i64 %index, !dbg !84
  %ch = load i8, ptr %src.ptr, align 1, !dbg !84
  %0 = icmp sle i8 %ch, 90, !dbg !84
  %1 = icmp sge i8 %ch, 65, !dbg !84
  %is.upper = and i1 %1, %0, !dbg !84
  %lower.ch = add i8 %ch, 32, !dbg !84
  %converted.ch = select i1 %is.upper, i8 %lower.ch, i8 %ch, !dbg !84
  %dst.ptr = getelementptr i8, ptr %new.data, i64 %index, !dbg !84
  store i8 %converted.ch, ptr %dst.ptr, align 1, !dbg !84
  %next.index = add i64 %index, 1, !dbg !84
  store i64 %next.index, ptr %index.ptr, align 4, !dbg !84
  br label %loop.cond, !dbg !84

loop.end:                                         ; preds = %loop.cond
  %null.term.ptr = getelementptr i8, ptr %new.data, i64 %str.len, !dbg !84
  store i8 0, ptr %null.term.ptr, align 1, !dbg !84
  %result.data = insertvalue { ptr, i64 } undef, ptr %new.data, 0, !dbg !84
  %result.len = insertvalue { ptr, i64 } %result.data, i64 %str.len, 1, !dbg !84
  store { ptr, i64 } %result.len, ptr %lower, align 8, !dbg !84
  call void @llvm.dbg.declare(metadata ptr %lower, metadata !82, metadata !DIExpression()), !dbg !86
  %str.data_ptr2 = getelementptr inbounds { ptr, i64 }, ptr %lower, i32 0, i32 0, !dbg !84
  %str.len_ptr3 = getelementptr inbounds { ptr, i64 }, ptr %lower, i32 0, i32 1, !dbg !84
  %str.data4 = load ptr, ptr %str.data_ptr2, align 8, !dbg !84
  %str.len5 = load i64, ptr %str.len_ptr3, align 4, !dbg !84
  %2 = icmp slt i64 0, %str.len5, !dbg !84
  %3 = and i1 true, %2, !dbg !84
  br i1 %3, label %in_bounds, label %out_of_bounds, !dbg !84

in_bounds:                                        ; preds = %loop.end
  %char.ptr = getelementptr i8, ptr %str.data4, i64 0, !dbg !84
  %char.val = load i8, ptr %char.ptr, align 1, !dbg !84
  br label %char_at_merge, !dbg !84

out_of_bounds:                                    ; preds = %loop.end
  br label %char_at_merge, !dbg !84

char_at_merge:                                    ; preds = %in_bounds, %out_of_bounds
  %char.result = phi i8 [ 0, %out_of_bounds ], [ %char.val, %in_bounds ], !dbg !84
  %int_sext = sext i8 %char.result to i64, !dbg !84
  store i64 %int_sext, ptr %ch6, align 4, !dbg !84
  call void @llvm.dbg.declare(metadata ptr %ch6, metadata !83, metadata !DIExpression()), !dbg !87
  ret void, !dbg !84
}

define i64 @main() !dbg !88 {
entry:
  call void @__vyn_println(ptr @13), !dbg !91
  call void @test_substring(), !dbg !91
  call void @__vyn_println(ptr @14), !dbg !91
  call void @test_char_at(), !dbg !91
  call void @__vyn_println(ptr @15), !dbg !91
  call void @test_starts_with(), !dbg !91
  call void @__vyn_println(ptr @16), !dbg !91
  call void @test_ends_with(), !dbg !91
  call void @__vyn_println(ptr @17), !dbg !91
  call void @test_contains(), !dbg !91
  call void @__vyn_println(ptr @18), !dbg !91
  call void @test_to_upper(), !dbg !91
  call void @__vyn_println(ptr @19), !dbg !91
  call void @test_to_lower(), !dbg !91
  ret i64 0, !dbg !91
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

declare i32 @memcmp(ptr, ptr, i64)

declare ptr @strstr(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "string_methods_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_substring", linkageName: "test_substring", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
!5 = !DISubroutineType(types: !6)
!6 = !{null}
!7 = !{!8, !10, !11, !12}
!8 = !DILocalVariable(name: "msg", scope: !4, file: !1, line: 4, type: !9)
!9 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!10 = !DILocalVariable(name: "sub1", scope: !4, file: !1, line: 6, type: !9)
!11 = !DILocalVariable(name: "sub2", scope: !4, file: !1, line: 11, type: !9)
!12 = !DILocalVariable(name: "sub3", scope: !4, file: !1, line: 16, type: !9)
!13 = !DILocation(line: 3, column: 1, scope: !4)
!14 = !DILocation(line: 4, column: 1, scope: !4)
!15 = !DILocation(line: 6, column: 5, scope: !4)
!16 = !DILocation(line: 11, column: 5, scope: !4)
!17 = !DILocation(line: 16, column: 5, scope: !4)
!18 = distinct !DISubprogram(name: "test_char_at", linkageName: "test_char_at", scope: !1, file: !1, line: 22, type: !5, scopeLine: 22, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !19)
!19 = !{!20, !21, !23, !24}
!20 = !DILocalVariable(name: "msg", scope: !18, file: !1, line: 23, type: !9)
!21 = !DILocalVariable(name: "ch1", scope: !18, file: !1, line: 25, type: !22)
!22 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!23 = !DILocalVariable(name: "ch2", scope: !18, file: !1, line: 30, type: !22)
!24 = !DILocalVariable(name: "ch3", scope: !18, file: !1, line: 35, type: !22)
!25 = !DILocation(line: 22, column: 1, scope: !18)
!26 = !DILocation(line: 23, column: 1, scope: !18)
!27 = !DILocation(line: 25, column: 5, scope: !18)
!28 = !DILocation(line: 30, column: 5, scope: !18)
!29 = !DILocation(line: 35, column: 5, scope: !18)
!30 = distinct !DISubprogram(name: "test_starts_with", linkageName: "test_starts_with", scope: !1, file: !1, line: 41, type: !5, scopeLine: 41, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !31)
!31 = !{!32, !33, !34, !35, !37}
!32 = !DILocalVariable(name: "msg", scope: !30, file: !1, line: 42, type: !9)
!33 = !DILocalVariable(name: "prefix1", scope: !30, file: !1, line: 43, type: !9)
!34 = !DILocalVariable(name: "prefix2", scope: !30, file: !1, line: 44, type: !9)
!35 = !DILocalVariable(name: "result1", scope: !30, file: !1, line: 46, type: !36)
!36 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!37 = !DILocalVariable(name: "result2", scope: !30, file: !1, line: 51, type: !36)
!38 = !DILocation(line: 41, column: 1, scope: !30)
!39 = !DILocation(line: 42, column: 1, scope: !30)
!40 = !DILocation(line: 43, column: 1, scope: !30)
!41 = !DILocation(line: 44, column: 1, scope: !30)
!42 = !DILocation(line: 46, column: 5, scope: !30)
!43 = !DILocation(line: 51, column: 5, scope: !30)
!44 = distinct !DISubprogram(name: "test_ends_with", linkageName: "test_ends_with", scope: !1, file: !1, line: 57, type: !5, scopeLine: 57, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !45)
!45 = !{!46, !47, !48, !49, !50}
!46 = !DILocalVariable(name: "msg", scope: !44, file: !1, line: 58, type: !9)
!47 = !DILocalVariable(name: "suffix1", scope: !44, file: !1, line: 59, type: !9)
!48 = !DILocalVariable(name: "suffix2", scope: !44, file: !1, line: 60, type: !9)
!49 = !DILocalVariable(name: "result1", scope: !44, file: !1, line: 62, type: !36)
!50 = !DILocalVariable(name: "result2", scope: !44, file: !1, line: 67, type: !36)
!51 = !DILocation(line: 57, column: 1, scope: !44)
!52 = !DILocation(line: 58, column: 1, scope: !44)
!53 = !DILocation(line: 59, column: 1, scope: !44)
!54 = !DILocation(line: 60, column: 1, scope: !44)
!55 = !DILocation(line: 62, column: 5, scope: !44)
!56 = !DILocation(line: 67, column: 5, scope: !44)
!57 = distinct !DISubprogram(name: "test_contains", linkageName: "test_contains", scope: !1, file: !1, line: 73, type: !5, scopeLine: 73, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !58)
!58 = !{!59, !60, !61, !62, !63}
!59 = !DILocalVariable(name: "msg", scope: !57, file: !1, line: 74, type: !9)
!60 = !DILocalVariable(name: "sub1", scope: !57, file: !1, line: 75, type: !9)
!61 = !DILocalVariable(name: "sub2", scope: !57, file: !1, line: 76, type: !9)
!62 = !DILocalVariable(name: "result1", scope: !57, file: !1, line: 78, type: !36)
!63 = !DILocalVariable(name: "result2", scope: !57, file: !1, line: 83, type: !36)
!64 = !DILocation(line: 73, column: 1, scope: !57)
!65 = !DILocation(line: 74, column: 1, scope: !57)
!66 = !DILocation(line: 75, column: 1, scope: !57)
!67 = !DILocation(line: 76, column: 1, scope: !57)
!68 = !DILocation(line: 78, column: 5, scope: !57)
!69 = !DILocation(line: 83, column: 5, scope: !57)
!70 = distinct !DISubprogram(name: "test_to_upper", linkageName: "test_to_upper", scope: !1, file: !1, line: 89, type: !5, scopeLine: 89, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !71)
!71 = !{!72, !73, !74}
!72 = !DILocalVariable(name: "msg", scope: !70, file: !1, line: 90, type: !9)
!73 = !DILocalVariable(name: "upper", scope: !70, file: !1, line: 92, type: !9)
!74 = !DILocalVariable(name: "ch", scope: !70, file: !1, line: 96, type: !22)
!75 = !DILocation(line: 89, column: 1, scope: !70)
!76 = !DILocation(line: 90, column: 1, scope: !70)
!77 = !DILocation(line: 92, column: 1, scope: !70)
!78 = !DILocation(line: 96, column: 5, scope: !70)
!79 = distinct !DISubprogram(name: "test_to_lower", linkageName: "test_to_lower", scope: !1, file: !1, line: 102, type: !5, scopeLine: 102, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !80)
!80 = !{!81, !82, !83}
!81 = !DILocalVariable(name: "msg", scope: !79, file: !1, line: 103, type: !9)
!82 = !DILocalVariable(name: "lower", scope: !79, file: !1, line: 105, type: !9)
!83 = !DILocalVariable(name: "ch", scope: !79, file: !1, line: 109, type: !22)
!84 = !DILocation(line: 102, column: 1, scope: !79)
!85 = !DILocation(line: 103, column: 1, scope: !79)
!86 = !DILocation(line: 105, column: 1, scope: !79)
!87 = !DILocation(line: 109, column: 5, scope: !79)
!88 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 115, type: !89, scopeLine: 115, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!89 = !DISubroutineType(types: !90)
!90 = !{!22}
!91 = !DILocation(line: 115, column: 1, scope: !88)
