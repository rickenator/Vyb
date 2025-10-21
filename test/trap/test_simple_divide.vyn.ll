; ModuleID = 'VynModule'
source_filename = "VynModule"

define { i64, ptr } @divide(i64 %a, i64 %b) !dbg !4 {
entry:
  %b2 = alloca i64, align 8
  %a1 = alloca i64, align 8
  store i64 %a, ptr %a1, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %a1, metadata !10, metadata !DIExpression()), !dbg !13
  store i64 %b, ptr %b2, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %b2, metadata !11, metadata !DIExpression()), !dbg !14
  %b3 = load i64, ptr %b2, align 4, !dbg !12
  %icmpeqtmp = icmp eq i64 %b3, 0, !dbg !12
  br i1 %icmpeqtmp, label %then, label %ifcont, !dbg !12

then:                                             ; preds = %entry
  %error.alloc = alloca i64, align 8, !dbg !12
  store i64 42, ptr %error.alloc, align 4, !dbg !12
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.alloc, 1, !dbg !12
  ret { i64, ptr } %error.ptr, !dbg !12

ifcont:                                           ; preds = %entry
  %a4 = load i64, ptr %a1, align 4, !dbg !12
  %b5 = load i64, ptr %b2, align 4, !dbg !12
  %sdivtmp = sdiv i64 %a4, %b5, !dbg !12
  %result.value = insertvalue { i64, ptr } undef, i64 %sdivtmp, 0, !dbg !12
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !12
  ret { i64, ptr } %result.error, !dbg !12
}

define i64 @main() !dbg !15 {
entry:
  %result = alloca i64, align 8, !dbg !20
  %calltmp = call { i64, ptr } @divide(i64 10, i64 2), !dbg !20
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !20
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !20
  %has.error = icmp ne ptr %call.error, null, !dbg !20
  br i1 %has.error, label %call.error1, label %call.success, !dbg !20

call.error1:                                      ; preds = %entry
  call void @__vyn_runtime_untrapped_error(ptr null), !dbg !20
  unreachable, !dbg !20

call.success:                                     ; preds = %entry
  store i64 %call.value, ptr %result, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %result, metadata !19, metadata !DIExpression()), !dbg !21
  %result2 = load i64, ptr %result, align 4, !dbg !20
  ret i64 %result2, !dbg !20
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
!1 = !DIFile(filename: "test_simple_divide.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "divide", linkageName: "divide", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !9)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8, !8}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!9 = !{!10, !11}
!10 = !DILocalVariable(name: "a", scope: !4, file: !1, line: 2, type: !8)
!11 = !DILocalVariable(name: "b", scope: !4, file: !1, line: 2, type: !8)
!12 = !DILocation(line: 2, column: 1, scope: !4)
!13 = !DILocation(line: 2, column: 9, scope: !4)
!14 = !DILocation(line: 2, column: 17, scope: !4)
!15 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 9, type: !16, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!16 = !DISubroutineType(types: !17)
!17 = !{!8}
!18 = !{!19}
!19 = !DILocalVariable(name: "result", scope: !15, file: !1, line: 10, type: !8)
!20 = !DILocation(line: 9, column: 1, scope: !15)
!21 = !DILocation(line: 10, column: 1, scope: !15)
