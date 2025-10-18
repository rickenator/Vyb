; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @test_int8() !dbg !4 {
entry:
  %zero = alloca i8, align 1
  %negative = alloca i8, align 1
  %small = alloca i8, align 1
  store i8 127, ptr %small, align 1, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %small, metadata !9, metadata !DIExpression()), !dbg !14
  store i8 -128, ptr %negative, align 1, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %negative, metadata !11, metadata !DIExpression()), !dbg !15
  store i8 0, ptr %zero, align 1, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %zero, metadata !12, metadata !DIExpression()), !dbg !16
  ret i64 8, !dbg !13
}

define i64 @test_int16() !dbg !17 {
entry:
  %zero = alloca i16, align 2
  %negative = alloca i16, align 2
  %medium = alloca i16, align 2
  store i16 32767, ptr %medium, align 2, !dbg !23
  call void @llvm.dbg.declare(metadata ptr %medium, metadata !19, metadata !DIExpression()), !dbg !24
  store i16 -32768, ptr %negative, align 2, !dbg !23
  call void @llvm.dbg.declare(metadata ptr %negative, metadata !21, metadata !DIExpression()), !dbg !25
  store i16 0, ptr %zero, align 2, !dbg !23
  call void @llvm.dbg.declare(metadata ptr %zero, metadata !22, metadata !DIExpression()), !dbg !26
  ret i64 16, !dbg !23
}

define i64 @test_int32() !dbg !27 {
entry:
  %zero = alloca i32, align 4
  %negative = alloca i32, align 4
  %large = alloca i32, align 4
  store i32 2147483647, ptr %large, align 4, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %large, metadata !29, metadata !DIExpression()), !dbg !34
  store i32 -2147483647, ptr %negative, align 4, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %negative, metadata !31, metadata !DIExpression()), !dbg !35
  store i32 0, ptr %zero, align 4, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %zero, metadata !32, metadata !DIExpression()), !dbg !36
  ret i64 32, !dbg !33
}

define i64 @test_int64() !dbg !37 {
entry:
  %zero = alloca i64, align 8
  %negative = alloca i64, align 8
  %huge = alloca i64, align 8
  store i64 9223372036854775807, ptr %huge, align 4, !dbg !42
  call void @llvm.dbg.declare(metadata ptr %huge, metadata !39, metadata !DIExpression()), !dbg !43
  store i64 -9223372036854775807, ptr %negative, align 4, !dbg !42
  call void @llvm.dbg.declare(metadata ptr %negative, metadata !40, metadata !DIExpression()), !dbg !44
  store i64 0, ptr %zero, align 4, !dbg !42
  call void @llvm.dbg.declare(metadata ptr %zero, metadata !41, metadata !DIExpression()), !dbg !45
  ret i64 64, !dbg !42
}

define i64 @main() !dbg !46 {
entry:
  %result = alloca i64, align 8
  store i64 0, ptr %result, align 4, !dbg !49
  call void @llvm.dbg.declare(metadata ptr %result, metadata !48, metadata !DIExpression()), !dbg !50
  %result1 = load i64, ptr %result, align 4, !dbg !49
  %calltmp = call i64 @test_int8(), !dbg !49
  %addtmp = add i64 %result1, %calltmp, !dbg !49
  store i64 %addtmp, ptr %result, align 4, !dbg !49
  %result2 = load i64, ptr %result, align 4, !dbg !49
  %calltmp3 = call i64 @test_int16(), !dbg !49
  %addtmp4 = add i64 %result2, %calltmp3, !dbg !49
  store i64 %addtmp4, ptr %result, align 4, !dbg !49
  %result5 = load i64, ptr %result, align 4, !dbg !49
  %calltmp6 = call i64 @test_int32(), !dbg !49
  %addtmp7 = add i64 %result5, %calltmp6, !dbg !49
  store i64 %addtmp7, ptr %result, align 4, !dbg !49
  %result8 = load i64, ptr %result, align 4, !dbg !49
  %calltmp9 = call i64 @test_int64(), !dbg !49
  %addtmp10 = add i64 %result8, %calltmp9, !dbg !49
  store i64 %addtmp10, ptr %result, align 4, !dbg !49
  %result11 = load i64, ptr %result, align 4, !dbg !49
  ret i64 %result11, !dbg !49
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "sized_integers_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/types")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_int8", linkageName: "test_int8", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12}
!9 = !DILocalVariable(name: "small", scope: !4, file: !1, line: 4, type: !10)
!10 = !DIBasicType(name: "i8", size: 8, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "negative", scope: !4, file: !1, line: 5, type: !10)
!12 = !DILocalVariable(name: "zero", scope: !4, file: !1, line: 6, type: !10)
!13 = !DILocation(line: 3, column: 1, scope: !4)
!14 = !DILocation(line: 4, column: 1, scope: !4)
!15 = !DILocation(line: 5, column: 1, scope: !4)
!16 = !DILocation(line: 6, column: 1, scope: !4)
!17 = distinct !DISubprogram(name: "test_int16", linkageName: "test_int16", scope: !1, file: !1, line: 10, type: !5, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!18 = !{!19, !21, !22}
!19 = !DILocalVariable(name: "medium", scope: !17, file: !1, line: 11, type: !20)
!20 = !DIBasicType(tag: DW_TAG_unspecified_type, name: "i16")
!21 = !DILocalVariable(name: "negative", scope: !17, file: !1, line: 12, type: !20)
!22 = !DILocalVariable(name: "zero", scope: !17, file: !1, line: 13, type: !20)
!23 = !DILocation(line: 10, column: 1, scope: !17)
!24 = !DILocation(line: 11, column: 1, scope: !17)
!25 = !DILocation(line: 12, column: 1, scope: !17)
!26 = !DILocation(line: 13, column: 1, scope: !17)
!27 = distinct !DISubprogram(name: "test_int32", linkageName: "test_int32", scope: !1, file: !1, line: 17, type: !5, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !28)
!28 = !{!29, !31, !32}
!29 = !DILocalVariable(name: "large", scope: !27, file: !1, line: 18, type: !30)
!30 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!31 = !DILocalVariable(name: "negative", scope: !27, file: !1, line: 19, type: !30)
!32 = !DILocalVariable(name: "zero", scope: !27, file: !1, line: 20, type: !30)
!33 = !DILocation(line: 17, column: 1, scope: !27)
!34 = !DILocation(line: 18, column: 1, scope: !27)
!35 = !DILocation(line: 19, column: 1, scope: !27)
!36 = !DILocation(line: 20, column: 1, scope: !27)
!37 = distinct !DISubprogram(name: "test_int64", linkageName: "test_int64", scope: !1, file: !1, line: 24, type: !5, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !38)
!38 = !{!39, !40, !41}
!39 = !DILocalVariable(name: "huge", scope: !37, file: !1, line: 25, type: !7)
!40 = !DILocalVariable(name: "negative", scope: !37, file: !1, line: 26, type: !7)
!41 = !DILocalVariable(name: "zero", scope: !37, file: !1, line: 27, type: !7)
!42 = !DILocation(line: 24, column: 1, scope: !37)
!43 = !DILocation(line: 25, column: 1, scope: !37)
!44 = !DILocation(line: 26, column: 1, scope: !37)
!45 = !DILocation(line: 27, column: 1, scope: !37)
!46 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 31, type: !5, scopeLine: 31, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !47)
!47 = !{!48}
!48 = !DILocalVariable(name: "result", scope: !46, file: !1, line: 32, type: !7)
!49 = !DILocation(line: 31, column: 1, scope: !46)
!50 = !DILocation(line: 32, column: 1, scope: !46)
