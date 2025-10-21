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
  %error.heap = call ptr @malloc(i64 16), !dbg !12
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !12
  store i64 42, ptr %error.data.ptr, align 4, !dbg !12
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.heap, 1, !dbg !12
  ret { i64, ptr } %error.ptr, !dbg !12

ifcont:                                           ; preds = %entry
  %a4 = load i64, ptr %a1, align 4, !dbg !12
  %b5 = load i64, ptr %b2, align 4, !dbg !12
  %sdivtmp = sdiv i64 %a4, %b5, !dbg !12
  %result.value = insertvalue { i64, ptr } undef, i64 %sdivtmp, 0, !dbg !12
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !12
  ret { i64, ptr } %result.error, !dbg !12
}

define { i64, ptr } @compute(i64 %x, i64 %y) !dbg !15 {
entry:
  %result = alloca i64, align 8
  %y2 = alloca i64, align 8
  %x1 = alloca i64, align 8
  store i64 %x, ptr %x1, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %x1, metadata !17, metadata !DIExpression()), !dbg !21
  store i64 %y, ptr %y2, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %y2, metadata !18, metadata !DIExpression()), !dbg !22
  %x3 = load i64, ptr %x1, align 4, !dbg !20
  %y4 = load i64, ptr %y2, align 4, !dbg !20
  %calltmp = call { i64, ptr } @divide(i64 %x3, i64 %y4), !dbg !20
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !20
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !20
  %has.error = icmp ne ptr %call.error, null, !dbg !20
  br i1 %has.error, label %call.error5, label %call.success, !dbg !20

call.error5:                                      ; preds = %entry
  %prop.error = insertvalue { i64, ptr } undef, ptr %call.error, 1, !dbg !20
  ret { i64, ptr } %prop.error, !dbg !20

call.success:                                     ; preds = %entry
  store i64 %call.value, ptr %result, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %result, metadata !19, metadata !DIExpression()), !dbg !23
  %result6 = load i64, ptr %result, align 4, !dbg !20
  %addtmp = add i64 %result6, 10, !dbg !20
  %result.value = insertvalue { i64, ptr } undef, i64 %addtmp, 0, !dbg !20
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !20
  ret { i64, ptr } %result.error, !dbg !20
}

define i64 @main() !dbg !24 {
entry:
  %val2 = alloca i64, align 8, !dbg !30
  %trap_error = alloca ptr, align 8, !dbg !30
  %val1 = alloca i64, align 8, !dbg !30
  %calltmp = call { i64, ptr } @compute(i64 10, i64 2), !dbg !30
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !30
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !30
  %has.error = icmp ne ptr %call.error, null, !dbg !30
  br i1 %has.error, label %call.error1, label %call.success, !dbg !30

call.error1:                                      ; preds = %entry
  call void @__vyn_runtime_untrapped_error(ptr null), !dbg !30
  unreachable, !dbg !30

call.success:                                     ; preds = %entry
  store i64 %call.value, ptr %val1, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %val1, metadata !28, metadata !DIExpression()), !dbg !31
  br label %block.normal, !dbg !30

block.normal:                                     ; preds = %call.success
  %calltmp2 = call { i64, ptr } @compute(i64 10, i64 0), !dbg !30
  %call.value3 = extractvalue { i64, ptr } %calltmp2, 0, !dbg !30
  %call.error4 = extractvalue { i64, ptr } %calltmp2, 1, !dbg !30
  %has.error5 = icmp ne ptr %call.error4, null, !dbg !30
  br i1 %has.error5, label %call.error6, label %call.success7, !dbg !30

block.continue:                                   ; preds = %trap.handler0, %call.success7
  %block.result = phi i64 [ %call.value3, %call.success7 ], [ -1, %trap.handler0 ], !dbg !30
  store i64 %block.result, ptr %val2, align 4, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %val2, metadata !29, metadata !DIExpression()), !dbg !32
  %val18 = load i64, ptr %val1, align 4, !dbg !30
  %val29 = load i64, ptr %val2, align 4, !dbg !30
  %addtmp = add i64 %val18, %val29, !dbg !30
  ret i64 %addtmp, !dbg !30

trap.landing:                                     ; preds = %call.error6
  %error.ptr = load ptr, ptr %trap_error, align 8, !dbg !30
  %error.typeid = load i64, ptr %error.ptr, align 4, !dbg !30
  %type.matches = icmp eq i64 %error.typeid, -3994496327427856726, !dbg !30
  br i1 %type.matches, label %trap.handler0, label %trap.unmatched, !dbg !30

call.error6:                                      ; preds = %block.normal
  store ptr %call.error4, ptr %trap_error, align 8, !dbg !30
  br label %trap.landing, !dbg !30

call.success7:                                    ; preds = %block.normal
  br label %block.continue, !dbg !30

trap.unmatched:                                   ; preds = %trap.landing
  call void @__vyn_runtime_untrapped_error(ptr %error.ptr), !dbg !30
  unreachable, !dbg !30

trap.handler0:                                    ; preds = %trap.landing
  %error.data.i8ptr = getelementptr i8, ptr %error.ptr, i64 8, !dbg !30
  %error.value = load i64, ptr %error.data.i8ptr, align 4, !dbg !30
  call void @free(ptr %error.ptr), !dbg !30
  br label %block.continue, !dbg !30
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #1

declare void @free(ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_success_then_trap.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
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
!15 = distinct !DISubprogram(name: "compute", linkageName: "compute", scope: !1, file: !1, line: 9, type: !5, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!16 = !{!17, !18, !19}
!17 = !DILocalVariable(name: "x", scope: !15, file: !1, line: 9, type: !8)
!18 = !DILocalVariable(name: "y", scope: !15, file: !1, line: 9, type: !8)
!19 = !DILocalVariable(name: "result", scope: !15, file: !1, line: 10, type: !8)
!20 = !DILocation(line: 9, column: 1, scope: !15)
!21 = !DILocation(line: 9, column: 10, scope: !15)
!22 = !DILocation(line: 9, column: 18, scope: !15)
!23 = !DILocation(line: 10, column: 1, scope: !15)
!24 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 14, type: !25, scopeLine: 14, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !27)
!25 = !DISubroutineType(types: !26)
!26 = !{!8}
!27 = !{!28, !29}
!28 = !DILocalVariable(name: "val1", scope: !24, file: !1, line: 15, type: !8)
!29 = !DILocalVariable(name: "val2", scope: !24, file: !1, line: 16, type: !8)
!30 = !DILocation(line: 14, column: 1, scope: !24)
!31 = !DILocation(line: 15, column: 1, scope: !24)
!32 = !DILocation(line: 16, column: 1, scope: !24)
