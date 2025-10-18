; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1

define i64 @test_all_integers() !dbg !4 {
entry:
  %u64 = alloca i64, align 8
  %u32 = alloca i32, align 4
  %u16 = alloca i16, align 2
  %u8 = alloca i8, align 1
  %i64 = alloca i64, align 8
  %i32 = alloca i32, align 4
  %i16 = alloca i16, align 2
  %i8 = alloca i8, align 1
  store i8 127, ptr %i8, align 1, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %i8, metadata !9, metadata !DIExpression()), !dbg !21
  store i16 32767, ptr %i16, align 2, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %i16, metadata !11, metadata !DIExpression()), !dbg !22
  store i32 2147483647, ptr %i32, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %i32, metadata !13, metadata !DIExpression()), !dbg !23
  store i64 42, ptr %i64, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %i64, metadata !15, metadata !DIExpression()), !dbg !24
  store i8 -1, ptr %u8, align 1, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %u8, metadata !16, metadata !DIExpression()), !dbg !25
  store i16 -1, ptr %u16, align 2, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %u16, metadata !17, metadata !DIExpression()), !dbg !26
  store i32 100, ptr %u32, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %u32, metadata !18, metadata !DIExpression()), !dbg !27
  store i64 200, ptr %u64, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %u64, metadata !19, metadata !DIExpression()), !dbg !28
  ret i64 8, !dbg !20
}

define i64 @test_all_floats() !dbg !29 {
entry:
  %f64 = alloca double, align 8
  store double 2.718280e+00, ptr %f64, align 8, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %f64, metadata !31, metadata !DIExpression()), !dbg !34
  ret i64 16, !dbg !33
}

define i64 @test_all_chars() !dbg !35 {
entry:
  %rn = alloca i32, align 4
  %ch = alloca i8, align 1
  store i8 65, ptr %ch, align 1, !dbg !39
  call void @llvm.dbg.declare(metadata ptr %ch, metadata !37, metadata !DIExpression()), !dbg !40
  store i32 128512, ptr %rn, align 4, !dbg !39
  call void @llvm.dbg.declare(metadata ptr %rn, metadata !38, metadata !DIExpression()), !dbg !41
  ret i64 32, !dbg !39
}

define i64 @test_other_types() !dbg !42 {
entry:
  %message = alloca { ptr, i64 }, align 8
  %flag = alloca i1, align 1
  store i1 true, ptr %flag, align 1, !dbg !48
  call void @llvm.dbg.declare(metadata ptr %flag, metadata !44, metadata !DIExpression()), !dbg !49
  store { ptr, i64 } { ptr @0, i64 5 }, ptr %message, align 8, !dbg !48
  call void @llvm.dbg.declare(metadata ptr %message, metadata !46, metadata !DIExpression()), !dbg !50
  ret i64 64, !dbg !48
}

define i64 @main() !dbg !51 {
entry:
  %total = alloca i64, align 8
  store i64 0, ptr %total, align 4, !dbg !54
  call void @llvm.dbg.declare(metadata ptr %total, metadata !53, metadata !DIExpression()), !dbg !55
  %total1 = load i64, ptr %total, align 4, !dbg !54
  %calltmp = call i64 @test_all_integers(), !dbg !54
  %addtmp = add i64 %total1, %calltmp, !dbg !54
  store i64 %addtmp, ptr %total, align 4, !dbg !54
  %total2 = load i64, ptr %total, align 4, !dbg !54
  %calltmp3 = call i64 @test_all_floats(), !dbg !54
  %addtmp4 = add i64 %total2, %calltmp3, !dbg !54
  store i64 %addtmp4, ptr %total, align 4, !dbg !54
  %total5 = load i64, ptr %total, align 4, !dbg !54
  %calltmp6 = call i64 @test_all_chars(), !dbg !54
  %addtmp7 = add i64 %total5, %calltmp6, !dbg !54
  store i64 %addtmp7, ptr %total, align 4, !dbg !54
  %total8 = load i64, ptr %total, align 4, !dbg !54
  %calltmp9 = call i64 @test_other_types(), !dbg !54
  %addtmp10 = add i64 %total8, %calltmp9, !dbg !54
  store i64 %addtmp10, ptr %total, align 4, !dbg !54
  %total11 = load i64, ptr %total, align 4, !dbg !54
  ret i64 %total11, !dbg !54
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
!1 = !DIFile(filename: "all_primitives_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/types")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_all_integers", linkageName: "test_all_integers", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !13, !15, !16, !17, !18, !19}
!9 = !DILocalVariable(name: "i8", scope: !4, file: !1, line: 5, type: !10)
!10 = !DIBasicType(name: "i8", size: 8, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "i16", scope: !4, file: !1, line: 6, type: !12)
!12 = !DIBasicType(tag: DW_TAG_unspecified_type, name: "i16")
!13 = !DILocalVariable(name: "i32", scope: !4, file: !1, line: 7, type: !14)
!14 = !DIBasicType(name: "i32", size: 32, encoding: DW_ATE_signed)
!15 = !DILocalVariable(name: "i64", scope: !4, file: !1, line: 8, type: !7)
!16 = !DILocalVariable(name: "u8", scope: !4, file: !1, line: 11, type: !10)
!17 = !DILocalVariable(name: "u16", scope: !4, file: !1, line: 12, type: !12)
!18 = !DILocalVariable(name: "u32", scope: !4, file: !1, line: 13, type: !14)
!19 = !DILocalVariable(name: "u64", scope: !4, file: !1, line: 14, type: !7)
!20 = !DILocation(line: 3, column: 1, scope: !4)
!21 = !DILocation(line: 5, column: 1, scope: !4)
!22 = !DILocation(line: 6, column: 1, scope: !4)
!23 = !DILocation(line: 7, column: 1, scope: !4)
!24 = !DILocation(line: 8, column: 1, scope: !4)
!25 = !DILocation(line: 11, column: 1, scope: !4)
!26 = !DILocation(line: 12, column: 1, scope: !4)
!27 = !DILocation(line: 13, column: 1, scope: !4)
!28 = !DILocation(line: 14, column: 1, scope: !4)
!29 = distinct !DISubprogram(name: "test_all_floats", linkageName: "test_all_floats", scope: !1, file: !1, line: 19, type: !5, scopeLine: 19, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !30)
!30 = !{!31}
!31 = !DILocalVariable(name: "f64", scope: !29, file: !1, line: 22, type: !32)
!32 = !DIBasicType(name: "f64", size: 64, encoding: DW_ATE_float)
!33 = !DILocation(line: 19, column: 1, scope: !29)
!34 = !DILocation(line: 22, column: 1, scope: !29)
!35 = distinct !DISubprogram(name: "test_all_chars", linkageName: "test_all_chars", scope: !1, file: !1, line: 27, type: !5, scopeLine: 27, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !36)
!36 = !{!37, !38}
!37 = !DILocalVariable(name: "ch", scope: !35, file: !1, line: 29, type: !10)
!38 = !DILocalVariable(name: "rn", scope: !35, file: !1, line: 30, type: !14)
!39 = !DILocation(line: 27, column: 1, scope: !35)
!40 = !DILocation(line: 29, column: 1, scope: !35)
!41 = !DILocation(line: 30, column: 1, scope: !35)
!42 = distinct !DISubprogram(name: "test_other_types", linkageName: "test_other_types", scope: !1, file: !1, line: 35, type: !5, scopeLine: 35, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !43)
!43 = !{!44, !46}
!44 = !DILocalVariable(name: "flag", scope: !42, file: !1, line: 37, type: !45)
!45 = !DIBasicType(name: "bool", size: 1, encoding: DW_ATE_boolean)
!46 = !DILocalVariable(name: "message", scope: !42, file: !1, line: 38, type: !47)
!47 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!48 = !DILocation(line: 35, column: 1, scope: !42)
!49 = !DILocation(line: 37, column: 1, scope: !42)
!50 = !DILocation(line: 38, column: 1, scope: !42)
!51 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 43, type: !5, scopeLine: 43, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !52)
!52 = !{!53}
!53 = !DILocalVariable(name: "total", scope: !51, file: !1, line: 44, type: !7)
!54 = !DILocation(line: 43, column: 1, scope: !51)
!55 = !DILocation(line: 44, column: 1, scope: !51)
