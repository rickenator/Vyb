; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @test_char() !dbg !4 {
entry:
  %newline = alloca i8, align 1
  %space = alloca i8, align 1
  %digit_0 = alloca i8, align 1
  %letter_z = alloca i8, align 1
  %letter_a = alloca i8, align 1
  store i8 97, ptr %letter_a, align 1, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %letter_a, metadata !9, metadata !DIExpression()), !dbg !16
  store i8 122, ptr %letter_z, align 1, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %letter_z, metadata !11, metadata !DIExpression()), !dbg !17
  store i8 48, ptr %digit_0, align 1, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %digit_0, metadata !12, metadata !DIExpression()), !dbg !18
  store i8 32, ptr %space, align 1, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %space, metadata !13, metadata !DIExpression()), !dbg !19
  store i8 10, ptr %newline, align 1, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %newline, metadata !14, metadata !DIExpression()), !dbg !20
  ret i64 309, !dbg !15
}

define i64 @test_rune() !dbg !21 {
entry:
  %latin_A = alloca i32, align 4
  %kanji = alloca i32, align 4
  %emoji_smile = alloca i32, align 4
  %ascii_a = alloca i32, align 4
  store i32 97, ptr %ascii_a, align 4, !dbg !28
  call void @llvm.dbg.declare(metadata ptr %ascii_a, metadata !23, metadata !DIExpression()), !dbg !29
  store i32 128512, ptr %emoji_smile, align 4, !dbg !28
  call void @llvm.dbg.declare(metadata ptr %emoji_smile, metadata !25, metadata !DIExpression()), !dbg !30
  store i32 26408, ptr %kanji, align 4, !dbg !28
  call void @llvm.dbg.declare(metadata ptr %kanji, metadata !26, metadata !DIExpression()), !dbg !31
  store i32 65, ptr %latin_A, align 4, !dbg !28
  call void @llvm.dbg.declare(metadata ptr %latin_A, metadata !27, metadata !DIExpression()), !dbg !32
  ret i64 4, !dbg !28
}

define i64 @main() !dbg !33 {
entry:
  %rune_result = alloca i64, align 8, !dbg !37
  %char_result = alloca i64, align 8, !dbg !37
  %calltmp = call i64 @test_char(), !dbg !37
  store i64 %calltmp, ptr %char_result, align 4, !dbg !37
  call void @llvm.dbg.declare(metadata ptr %char_result, metadata !35, metadata !DIExpression()), !dbg !38
  %calltmp1 = call i64 @test_rune(), !dbg !37
  store i64 %calltmp1, ptr %rune_result, align 4, !dbg !37
  call void @llvm.dbg.declare(metadata ptr %rune_result, metadata !36, metadata !DIExpression()), !dbg !39
  %char_result2 = load i64, ptr %char_result, align 4, !dbg !37
  %rune_result3 = load i64, ptr %rune_result, align 4, !dbg !37
  %addtmp = add i64 %char_result2, %rune_result3, !dbg !37
  ret i64 %addtmp, !dbg !37
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
!1 = !DIFile(filename: "char_types_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/types")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_char", linkageName: "test_char", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12, !13, !14}
!9 = !DILocalVariable(name: "letter_a", scope: !4, file: !1, line: 5, type: !10)
!10 = !DIBasicType(name: "i8", size: 8, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "letter_z", scope: !4, file: !1, line: 6, type: !10)
!12 = !DILocalVariable(name: "digit_0", scope: !4, file: !1, line: 7, type: !10)
!13 = !DILocalVariable(name: "space", scope: !4, file: !1, line: 8, type: !10)
!14 = !DILocalVariable(name: "newline", scope: !4, file: !1, line: 9, type: !10)
!15 = !DILocation(line: 3, column: 1, scope: !4)
!16 = !DILocation(line: 5, column: 1, scope: !4)
!17 = !DILocation(line: 6, column: 1, scope: !4)
!18 = !DILocation(line: 7, column: 1, scope: !4)
!19 = !DILocation(line: 8, column: 1, scope: !4)
!20 = !DILocation(line: 9, column: 1, scope: !4)
!21 = distinct !DISubprogram(name: "test_rune", linkageName: "test_rune", scope: !1, file: !1, line: 16, type: !5, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !22)
!22 = !{!23, !25, !26, !27}
!23 = !DILocalVariable(name: "ascii_a", scope: !21, file: !1, line: 18, type: !24)
!24 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!25 = !DILocalVariable(name: "emoji_smile", scope: !21, file: !1, line: 19, type: !24)
!26 = !DILocalVariable(name: "kanji", scope: !21, file: !1, line: 20, type: !24)
!27 = !DILocalVariable(name: "latin_A", scope: !21, file: !1, line: 21, type: !24)
!28 = !DILocation(line: 16, column: 1, scope: !21)
!29 = !DILocation(line: 18, column: 1, scope: !21)
!30 = !DILocation(line: 19, column: 1, scope: !21)
!31 = !DILocation(line: 20, column: 1, scope: !21)
!32 = !DILocation(line: 21, column: 1, scope: !21)
!33 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 28, type: !5, scopeLine: 28, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !34)
!34 = !{!35, !36}
!35 = !DILocalVariable(name: "char_result", scope: !33, file: !1, line: 29, type: !7)
!36 = !DILocalVariable(name: "rune_result", scope: !33, file: !1, line: 30, type: !7)
!37 = !DILocation(line: 28, column: 1, scope: !33)
!38 = !DILocation(line: 29, column: 1, scope: !33)
!39 = !DILocation(line: 30, column: 1, scope: !33)
