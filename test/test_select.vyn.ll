; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [4 x i8] c"one\00", align 1
@1 = private unnamed_addr constant [4 x i8] c"one\00", align 1
@2 = private unnamed_addr constant [10 x i8] c"forty-two\00", align 1
@3 = private unnamed_addr constant [8 x i8] c"hundred\00", align 1
@4 = private unnamed_addr constant [6 x i8] c"other\00", align 1
@5 = private unnamed_addr constant [4 x i8] c"one\00", align 1
@6 = private unnamed_addr constant [4 x i8] c"one\00", align 1
@7 = private unnamed_addr constant [10 x i8] c"forty-two\00", align 1
@8 = private unnamed_addr constant [6 x i8] c"other\00", align 1
@9 = private unnamed_addr constant [9 x i8] c"Result: \00", align 1
@10 = private unnamed_addr constant [4 x i8] c"one\00", align 1
@11 = private unnamed_addr constant [4 x i8] c"one\00", align 1
@12 = private unnamed_addr constant [4 x i8] c"two\00", align 1
@13 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@14 = private unnamed_addr constant [4 x i8] c"one\00", align 1
@15 = private unnamed_addr constant [4 x i8] c"one\00", align 1
@16 = private unnamed_addr constant [19 x i8] c"wildcard caught it\00", align 1
@17 = private unnamed_addr constant [35 x i8] c"=== Testing Select Expressions ===\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@18 = private unnamed_addr constant [27 x i8] c"=== All tests complete ===\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define void @test_basic_select() !dbg !4 {
entry:
  %y = alloca i64, align 8
  %x = alloca i64, align 8
  store i64 42, ptr %x, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %x, metadata !8, metadata !DIExpression()), !dbg !12
  %x1 = load i64, ptr %x, align 4, !dbg !13
  %select.result = alloca { ptr, i64 }, align 8, !dbg !13
  %select.cmp = icmp eq i64 %x1, 1, !dbg !13
  br i1 %select.cmp, label %select.case, label %select.next, !dbg !13

select.case:                                      ; preds = %entry
  store { ptr, i64 } { ptr @1, i64 3 }, ptr %select.result, align 8, !dbg !13
  br label %select.end, !dbg !13

select.next:                                      ; preds = %entry
  %select.cmp3 = icmp eq i64 %x1, 42, !dbg !13
  br i1 %select.cmp3, label %select.case2, label %select.next4, !dbg !13

select.case2:                                     ; preds = %select.next
  store { ptr, i64 } { ptr @2, i64 9 }, ptr %select.result, align 8, !dbg !13
  br label %select.end, !dbg !13

select.next4:                                     ; preds = %select.next
  %select.cmp6 = icmp eq i64 %x1, 100, !dbg !13
  br i1 %select.cmp6, label %select.case5, label %select.next7, !dbg !13

select.case5:                                     ; preds = %select.next4
  store { ptr, i64 } { ptr @3, i64 7 }, ptr %select.result, align 8, !dbg !13
  br label %select.end, !dbg !13

select.next7:                                     ; preds = %select.next4
  store { ptr, i64 } { ptr @4, i64 5 }, ptr %select.result, align 8, !dbg !13
  br label %select.end, !dbg !13

select.end:                                       ; preds = %select.next7, %select.case5, %select.case2, %select.case
  %select.result.load = load { ptr, i64 }, ptr %select.result, align 8, !dbg !13
  store i64 1, ptr %y, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %y, metadata !10, metadata !DIExpression()), !dbg !14
  %y8 = load i64, ptr %y, align 4, !dbg !15
  %select.result9 = alloca { ptr, i64 }, align 8, !dbg !15
  %select.cmp11 = icmp eq i64 %y8, 1, !dbg !15
  br i1 %select.cmp11, label %select.case10, label %select.next12, !dbg !15

select.case10:                                    ; preds = %select.end
  store { ptr, i64 } { ptr @6, i64 3 }, ptr %select.result9, align 8, !dbg !15
  br label %select.end16, !dbg !15

select.next12:                                    ; preds = %select.end
  %select.cmp14 = icmp eq i64 %y8, 42, !dbg !15
  br i1 %select.cmp14, label %select.case13, label %select.next15, !dbg !15

select.case13:                                    ; preds = %select.next12
  store { ptr, i64 } { ptr @7, i64 9 }, ptr %select.result9, align 8, !dbg !15
  br label %select.end16, !dbg !15

select.next15:                                    ; preds = %select.next12
  store { ptr, i64 } { ptr @8, i64 5 }, ptr %select.result9, align 8, !dbg !15
  br label %select.end16, !dbg !15

select.end16:                                     ; preds = %select.next15, %select.case13, %select.case10
  %select.result.load17 = load { ptr, i64 }, ptr %select.result9, align 8, !dbg !15
  ret void, !dbg !15
}

define void @test_select_with_computation() !dbg !16 {
entry:
  %n = alloca i64, align 8
  store i64 3, ptr %n, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %n, metadata !18, metadata !DIExpression()), !dbg !20
  %n1 = load i64, ptr %n, align 4, !dbg !21
  %select.result = alloca i64, align 8, !dbg !21
  %select.cmp = icmp eq i64 %n1, 1, !dbg !21
  br i1 %select.cmp, label %select.case, label %select.next, !dbg !21

select.case:                                      ; preds = %entry
  store i64 2, ptr %select.result, align 4, !dbg !21
  br label %select.end, !dbg !21

select.next:                                      ; preds = %entry
  %select.cmp3 = icmp eq i64 %n1, 2, !dbg !21
  br i1 %select.cmp3, label %select.case2, label %select.next4, !dbg !21

select.case2:                                     ; preds = %select.next
  store i64 4, ptr %select.result, align 4, !dbg !21
  br label %select.end, !dbg !21

select.next4:                                     ; preds = %select.next
  %select.cmp6 = icmp eq i64 %n1, 3, !dbg !21
  br i1 %select.cmp6, label %select.case5, label %select.next7, !dbg !21

select.case5:                                     ; preds = %select.next4
  store i64 6, ptr %select.result, align 4, !dbg !21
  br label %select.end, !dbg !21

select.next7:                                     ; preds = %select.next4
  store i64 0, ptr %select.result, align 4, !dbg !21
  br label %select.end, !dbg !21

select.end:                                       ; preds = %select.next7, %select.case5, %select.case2, %select.case
  %select.result.load = load i64, ptr %select.result, align 4, !dbg !21
  ret void, !dbg !21
}

define void @test_select_as_expression() !dbg !22 {
entry:
  %x = alloca i64, align 8
  store i64 2, ptr %x, align 4, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %x, metadata !24, metadata !DIExpression()), !dbg !26
  %x1 = load i64, ptr %x, align 4, !dbg !27
  %select.result = alloca { ptr, i64 }, align 8, !dbg !27
  %select.cmp = icmp eq i64 %x1, 1, !dbg !27
  br i1 %select.cmp, label %select.case, label %select.next, !dbg !27

select.case:                                      ; preds = %entry
  store { ptr, i64 } { ptr @11, i64 3 }, ptr %select.result, align 8, !dbg !27
  br label %select.end, !dbg !27

select.next:                                      ; preds = %entry
  %select.cmp3 = icmp eq i64 %x1, 2, !dbg !27
  br i1 %select.cmp3, label %select.case2, label %select.next4, !dbg !27

select.case2:                                     ; preds = %select.next
  store { ptr, i64 } { ptr @12, i64 3 }, ptr %select.result, align 8, !dbg !27
  br label %select.end, !dbg !27

select.next4:                                     ; preds = %select.next
  store { ptr, i64 } { ptr @13, i64 7 }, ptr %select.result, align 8, !dbg !27
  br label %select.end, !dbg !27

select.end:                                       ; preds = %select.next4, %select.case2, %select.case
  %select.result.load = load { ptr, i64 }, ptr %select.result, align 8, !dbg !27
  %str2.data = extractvalue { ptr, i64 } %select.result.load, 0, !dbg !27
  %str2.len = extractvalue { ptr, i64 } %select.result.load, 1, !dbg !27
  %str.new_len = add i64 8, %str2.len, !dbg !27
  %str.alloc_size = add i64 %str.new_len, 1, !dbg !27
  %str.new_data = call ptr @malloc(i64 %str.alloc_size), !dbg !27
  %0 = call ptr @memcpy(ptr %str.new_data, ptr @9, i64 8), !dbg !27
  %str.offset = getelementptr i8, ptr %str.new_data, i64 8, !dbg !27
  %1 = call ptr @memcpy(ptr %str.offset, ptr %str2.data, i64 %str2.len), !dbg !27
  %str.null_pos = getelementptr i8, ptr %str.new_data, i64 %str.new_len, !dbg !27
  store i8 0, ptr %str.null_pos, align 1, !dbg !27
  %str.result_data = insertvalue { ptr, i64 } undef, ptr %str.new_data, 0, !dbg !27
  %str.result_len = insertvalue { ptr, i64 } %str.result_data, i64 %str.new_len, 1, !dbg !27
  ret void, !dbg !27
}

define void @test_wildcard() !dbg !28 {
entry:
  %z = alloca i64, align 8
  store i64 999, ptr %z, align 4, !dbg !31
  call void @llvm.dbg.declare(metadata ptr %z, metadata !30, metadata !DIExpression()), !dbg !32
  %z1 = load i64, ptr %z, align 4, !dbg !33
  %select.result = alloca { ptr, i64 }, align 8, !dbg !33
  %select.cmp = icmp eq i64 %z1, 1, !dbg !33
  br i1 %select.cmp, label %select.case, label %select.next, !dbg !33

select.case:                                      ; preds = %entry
  store { ptr, i64 } { ptr @15, i64 3 }, ptr %select.result, align 8, !dbg !33
  br label %select.end, !dbg !33

select.next:                                      ; preds = %entry
  store { ptr, i64 } { ptr @16, i64 18 }, ptr %select.result, align 8, !dbg !33
  br label %select.end, !dbg !33

select.end:                                       ; preds = %select.next, %select.case
  %select.result.load = load { ptr, i64 }, ptr %select.result, align 8, !dbg !33
  ret void, !dbg !33
}

define void @main() !dbg !34 {
entry:
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !35
  store { ptr, i64 } { ptr @17, i64 34 }, ptr %serialize_temp, align 8, !dbg !35
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !35
  call void @__vyn_println(ptr %serialized_json), !dbg !35
  call void @test_basic_select(), !dbg !35
  call void @test_select_with_computation(), !dbg !35
  call void @test_select_as_expression(), !dbg !35
  call void @test_wildcard(), !dbg !35
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !35
  store { ptr, i64 } { ptr @18, i64 26 }, ptr %serialize_temp1, align 8, !dbg !35
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.1), !dbg !35
  call void @__vyn_println(ptr %serialized_json2), !dbg !35
  ret void, !dbg !35
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare ptr @memcpy(ptr, ptr, i64)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_select.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_basic_select", linkageName: "test_basic_select", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
!5 = !DISubroutineType(types: !6)
!6 = !{null}
!7 = !{!8, !10}
!8 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 4, type: !9)
!9 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!10 = !DILocalVariable(name: "y", scope: !4, file: !1, line: 14, type: !9)
!11 = !DILocation(line: 3, column: 1, scope: !4)
!12 = !DILocation(line: 4, column: 5, scope: !4)
!13 = !DILocation(line: 6, column: 20, scope: !4)
!14 = !DILocation(line: 14, column: 5, scope: !4)
!15 = !DILocation(line: 16, column: 21, scope: !4)
!16 = distinct !DISubprogram(name: "test_select_with_computation", linkageName: "test_select_with_computation", scope: !1, file: !1, line: 24, type: !5, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !17)
!17 = !{!18}
!18 = !DILocalVariable(name: "n", scope: !16, file: !1, line: 25, type: !9)
!19 = !DILocation(line: 24, column: 1, scope: !16)
!20 = !DILocation(line: 25, column: 5, scope: !16)
!21 = !DILocation(line: 27, column: 21, scope: !16)
!22 = distinct !DISubprogram(name: "test_select_as_expression", linkageName: "test_select_as_expression", scope: !1, file: !1, line: 36, type: !5, scopeLine: 36, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !23)
!23 = !{!24}
!24 = !DILocalVariable(name: "x", scope: !22, file: !1, line: 37, type: !9)
!25 = !DILocation(line: 36, column: 1, scope: !22)
!26 = !DILocation(line: 37, column: 5, scope: !22)
!27 = !DILocation(line: 39, column: 34, scope: !22)
!28 = distinct !DISubprogram(name: "test_wildcard", linkageName: "test_wildcard", scope: !1, file: !1, line: 47, type: !5, scopeLine: 47, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !29)
!29 = !{!30}
!30 = !DILocalVariable(name: "z", scope: !28, file: !1, line: 48, type: !9)
!31 = !DILocation(line: 47, column: 1, scope: !28)
!32 = !DILocation(line: 48, column: 5, scope: !28)
!33 = !DILocation(line: 50, column: 20, scope: !28)
!34 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 57, type: !5, scopeLine: 57, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!35 = !DILocation(line: 57, column: 1, scope: !34)
