; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [10 x i8] c"Results: \00", align 1
@1 = private unnamed_addr constant [3 x i8] c", \00", align 1

define i64 @divide(i64 %a, i64 %b) !dbg !4 {
entry:
  %b2 = alloca i64, align 8
  %a1 = alloca i64, align 8
  store i64 %a, ptr %a1, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %a1, metadata !9, metadata !DIExpression()), !dbg !12
  store i64 %b, ptr %b2, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %b2, metadata !10, metadata !DIExpression()), !dbg !13
  %b3 = load i64, ptr %b2, align 4, !dbg !11
  %icmpeqtmp = icmp eq i64 %b3, 0, !dbg !11
  br i1 %icmpeqtmp, label %then, label %ifcont, !dbg !11

then:                                             ; preds = %entry
  br label %ifcont, !dbg !11

ifcont:                                           ; preds = %then, %entry
  %a4 = load i64, ptr %a1, align 4, !dbg !11
  %b5 = load i64, ptr %b2, align 4, !dbg !11
  %sdivtmp = sdiv i64 %a4, %b5, !dbg !11
  ret i64 %sdivtmp, !dbg !11
}

define i64 @safe_divide(i64 %a, i64 %b) !dbg !14 {
entry:
  %b2 = alloca i64, align 8
  %a1 = alloca i64, align 8
  store i64 %a, ptr %a1, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %a1, metadata !16, metadata !DIExpression()), !dbg !19
  store i64 %b, ptr %b2, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %b2, metadata !17, metadata !DIExpression()), !dbg !20
  %a3 = load i64, ptr %a1, align 4, !dbg !18
  %b4 = load i64, ptr %b2, align 4, !dbg !18
  %calltmp = call i64 @divide(i64 %a3, i64 %b4), !dbg !18
  ret i64 undef, !dbg !18
}

define i64 @main() !dbg !21 {
entry:
  %result2 = alloca i64, align 8, !dbg !27
  %result1 = alloca i64, align 8, !dbg !27
  %calltmp = call i64 @safe_divide(i64 10, i64 2), !dbg !27
  store i64 %calltmp, ptr %result1, align 4, !dbg !27
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !25, metadata !DIExpression()), !dbg !28
  %calltmp1 = call i64 @safe_divide(i64 10, i64 0), !dbg !27
  store i64 %calltmp1, ptr %result2, align 4, !dbg !27
  call void @llvm.dbg.declare(metadata ptr %result2, metadata !26, metadata !DIExpression()), !dbg !29
  %result12 = load i64, ptr %result1, align 4, !dbg !27
  %tostring = call ptr @__vyn_toString_int(i64 %result12), !dbg !27
  %strcattmp = call ptr @__vyn_string_concat(ptr @0, ptr %tostring), !dbg !27
  %strcattmp3 = call ptr @__vyn_string_concat(ptr %strcattmp, ptr @1), !dbg !27
  %result24 = load i64, ptr %result2, align 4, !dbg !27
  %tostring5 = call ptr @__vyn_toString_int(i64 %result24), !dbg !27
  %strcattmp6 = call ptr @__vyn_string_concat(ptr %strcattmp3, ptr %tostring5), !dbg !27
  call void @__vyn_println(ptr %strcattmp6), !dbg !27
  ret i64 0, !dbg !27
}

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
!1 = !DIFile(filename: "08_fail_in_function.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "divide", linkageName: "divide", scope: !1, file: !1, line: 4, type: !5, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !7, !7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10}
!9 = !DILocalVariable(name: "a", scope: !4, file: !1, line: 4, type: !7)
!10 = !DILocalVariable(name: "b", scope: !4, file: !1, line: 4, type: !7)
!11 = !DILocation(line: 4, column: 1, scope: !4)
!12 = !DILocation(line: 4, column: 9, scope: !4)
!13 = !DILocation(line: 4, column: 17, scope: !4)
!14 = distinct !DISubprogram(name: "safe_divide", linkageName: "safe_divide", scope: !1, file: !1, line: 11, type: !5, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!15 = !{!16, !17}
!16 = !DILocalVariable(name: "a", scope: !14, file: !1, line: 11, type: !7)
!17 = !DILocalVariable(name: "b", scope: !14, file: !1, line: 11, type: !7)
!18 = !DILocation(line: 11, column: 1, scope: !14)
!19 = !DILocation(line: 11, column: 14, scope: !14)
!20 = !DILocation(line: 11, column: 22, scope: !14)
!21 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 21, type: !22, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !24)
!22 = !DISubroutineType(types: !23)
!23 = !{!7}
!24 = !{!25, !26}
!25 = !DILocalVariable(name: "result1", scope: !21, file: !1, line: 22, type: !7)
!26 = !DILocalVariable(name: "result2", scope: !21, file: !1, line: 23, type: !7)
!27 = !DILocation(line: 21, column: 1, scope: !21)
!28 = !DILocation(line: 22, column: 1, scope: !21)
!29 = !DILocation(line: 23, column: 1, scope: !21)
