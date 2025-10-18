; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @test_float32() !dbg !4 {
entry:
  ret i64 32, !dbg !8
}

define i64 @test_float64() !dbg !9 {
entry:
  %scientific = alloca double, align 8
  %zero = alloca double, align 8
  %negative = alloca double, align 8
  %large = alloca double, align 8
  %precise = alloca double, align 8
  store double 0x400921FB54442D18, ptr %precise, align 8, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %precise, metadata !11, metadata !DIExpression()), !dbg !18
  store double 0x4132D687E3DF217D, ptr %large, align 8, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %large, metadata !13, metadata !DIExpression()), !dbg !19
  store double 0xC08F3FFFFF79C843, ptr %negative, align 8, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %negative, metadata !14, metadata !DIExpression()), !dbg !20
  store double 0.000000e+00, ptr %zero, align 8, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %zero, metadata !15, metadata !DIExpression()), !dbg !21
  store double 1.230000e+00, ptr %scientific, align 8, !dbg !17
  call void @llvm.dbg.declare(metadata ptr %scientific, metadata !16, metadata !DIExpression()), !dbg !22
  ret i64 64, !dbg !17
}

define i64 @main() !dbg !23 {
entry:
  %result = alloca i64, align 8
  store i64 0, ptr %result, align 4, !dbg !26
  call void @llvm.dbg.declare(metadata ptr %result, metadata !25, metadata !DIExpression()), !dbg !27
  %result1 = load i64, ptr %result, align 4, !dbg !26
  %calltmp = call i64 @test_float32(), !dbg !26
  %addtmp = add i64 %result1, %calltmp, !dbg !26
  store i64 %addtmp, ptr %result, align 4, !dbg !26
  %result2 = load i64, ptr %result, align 4, !dbg !26
  %calltmp3 = call i64 @test_float64(), !dbg !26
  %addtmp4 = add i64 %result2, %calltmp3, !dbg !26
  store i64 %addtmp4, ptr %result, align 4, !dbg !26
  %result5 = load i64, ptr %result, align 4, !dbg !26
  ret i64 %result5, !dbg !26
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
!1 = !DIFile(filename: "float_types_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/types")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "test_float32", linkageName: "test_float32", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !DILocation(line: 3, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "test_float64", linkageName: "test_float64", scope: !1, file: !1, line: 11, type: !5, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !10)
!10 = !{!11, !13, !14, !15, !16}
!11 = !DILocalVariable(name: "precise", scope: !9, file: !1, line: 12, type: !12)
!12 = !DIBasicType(name: "f64", size: 64, encoding: DW_ATE_float)
!13 = !DILocalVariable(name: "large", scope: !9, file: !1, line: 13, type: !12)
!14 = !DILocalVariable(name: "negative", scope: !9, file: !1, line: 14, type: !12)
!15 = !DILocalVariable(name: "zero", scope: !9, file: !1, line: 15, type: !12)
!16 = !DILocalVariable(name: "scientific", scope: !9, file: !1, line: 16, type: !12)
!17 = !DILocation(line: 11, column: 1, scope: !9)
!18 = !DILocation(line: 12, column: 1, scope: !9)
!19 = !DILocation(line: 13, column: 1, scope: !9)
!20 = !DILocation(line: 14, column: 1, scope: !9)
!21 = !DILocation(line: 15, column: 1, scope: !9)
!22 = !DILocation(line: 16, column: 1, scope: !9)
!23 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 20, type: !5, scopeLine: 20, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !24)
!24 = !{!25}
!25 = !DILocalVariable(name: "result", scope: !23, file: !1, line: 21, type: !7)
!26 = !DILocation(line: 20, column: 1, scope: !23)
!27 = !DILocation(line: 21, column: 1, scope: !23)
