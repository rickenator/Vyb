; ModuleID = 'VynModule'
source_filename = "VynModule"

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
  call void @__vyn_runtime_untrapped_error(ptr null), !dbg !11
  unreachable, !dbg !11

ifcont:                                           ; preds = %entry
  %a4 = load i64, ptr %a1, align 4, !dbg !11
  %b5 = load i64, ptr %b2, align 4, !dbg !11
  %sdivtmp = sdiv i64 %a4, %b5, !dbg !11
  ret i64 %sdivtmp, !dbg !11
}

define i64 @main() !dbg !14 {
entry:
  %result = alloca i64, align 8
  %trap_error = alloca i64, align 8
  br label %block.normal, !dbg !19

block.normal:                                     ; preds = %entry
  %calltmp = call i64 @divide(i64 10, i64 0), !dbg !19
  br label %block.continue, !dbg !19

block.continue:                                   ; preds = %block.normal
  %block.result = phi i64 [ %calltmp, %block.normal ], !dbg !19
  store i64 %block.result, ptr %result, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata ptr %result, metadata !18, metadata !DIExpression()), !dbg !20
  %result1 = load i64, ptr %result, align 4, !dbg !19
  ret i64 %result1, !dbg !19

trap.landing:                                     ; No predecessors!
  %caught_error = load i64, ptr %trap_error, align 4, !dbg !19
  ret i64 -1, !dbg !19
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #1

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "simple_fail_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "divide", linkageName: "divide", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !7, !7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10}
!9 = !DILocalVariable(name: "a", scope: !4, file: !1, line: 3, type: !7)
!10 = !DILocalVariable(name: "b", scope: !4, file: !1, line: 3, type: !7)
!11 = !DILocation(line: 3, column: 1, scope: !4)
!12 = !DILocation(line: 3, column: 9, scope: !4)
!13 = !DILocation(line: 3, column: 17, scope: !4)
!14 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 10, type: !15, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !17)
!15 = !DISubroutineType(types: !16)
!16 = !{!7}
!17 = !{!18}
!18 = !DILocalVariable(name: "result", scope: !14, file: !1, line: 11, type: !7)
!19 = !DILocation(line: 10, column: 1, scope: !14)
!20 = !DILocation(line: 11, column: 1, scope: !14)
