; ModuleID = 'VynModule'
source_filename = "VynModule"

%DivisionError = type { i64, i64 }

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
  %DivisionError_obj = alloca %DivisionError, align 8, !dbg !12
  %code_ptr = getelementptr inbounds %DivisionError, ptr %DivisionError_obj, i32 0, i32 0, !dbg !12
  store i64 42, ptr %code_ptr, align 4, !dbg !12
  %a4 = load i64, ptr %a1, align 4, !dbg !12
  %dividend_ptr = getelementptr inbounds %DivisionError, ptr %DivisionError_obj, i32 0, i32 1, !dbg !12
  store i64 %a4, ptr %dividend_ptr, align 4, !dbg !12
  %DivisionError_val = load %DivisionError, ptr %DivisionError_obj, align 4, !dbg !12
  %error.heap = call ptr @malloc(i64 24), !dbg !12
  %error.typeid.ptr = getelementptr i64, ptr %error.heap, i32 0, !dbg !12
  store i64 1794997878183821407, ptr %error.typeid.ptr, align 4, !dbg !12
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !12
  store %DivisionError %DivisionError_val, ptr %error.data.ptr, align 4, !dbg !12
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.heap, 1, !dbg !12
  ret { i64, ptr } %error.ptr, !dbg !12

ifcont:                                           ; preds = %entry
  %a5 = load i64, ptr %a1, align 4, !dbg !12
  %b6 = load i64, ptr %b2, align 4, !dbg !12
  %sdivtmp = sdiv i64 %a5, %b6, !dbg !12
  %result.value = insertvalue { i64, ptr } undef, i64 %sdivtmp, 0, !dbg !12
  %result.error = insertvalue { i64, ptr } %result.value, ptr null, 1, !dbg !12
  ret { i64, ptr } %result.error, !dbg !12
}

define i64 @main() !dbg !15 {
entry:
  %result = alloca i64, align 8
  %trap_error = alloca ptr, align 8
  br label %block.normal, !dbg !20

block.normal:                                     ; preds = %entry
  %calltmp = call { i64, ptr } @divide(i64 10, i64 0), !dbg !20
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !20
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !20
  %has.error = icmp ne ptr %call.error, null, !dbg !20
  br i1 %has.error, label %call.error1, label %call.success, !dbg !20

block.continue:                                   ; preds = %trap.unmatched, %trap.handler0, %call.success
  %block.result = phi i64 [ %call.value, %call.success ], [ %addtmp, %trap.handler0 ], [ 0, %trap.unmatched ], !dbg !20
  store i64 %block.result, ptr %result, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %result, metadata !19, metadata !DIExpression()), !dbg !21
  %result3 = load i64, ptr %result, align 4, !dbg !20
  ret i64 %result3, !dbg !20

trap.landing:                                     ; preds = %call.error1
  %error.ptr = load ptr, ptr %trap_error, align 8, !dbg !20
  %error.typeid = load i64, ptr %error.ptr, align 4, !dbg !20
  %type.matches = icmp eq i64 %error.typeid, 1794997878183821407, !dbg !20
  br i1 %type.matches, label %trap.handler0, label %trap.unmatched, !dbg !20

call.error1:                                      ; preds = %block.normal
  store ptr %call.error, ptr %trap_error, align 8, !dbg !20
  br label %trap.landing, !dbg !20

call.success:                                     ; preds = %block.normal
  br label %block.continue, !dbg !20

trap.unmatched:                                   ; preds = %trap.landing
  br label %block.continue, !dbg !20

trap.handler0:                                    ; preds = %trap.landing
  %error.data.i8ptr = getelementptr i8, ptr %error.ptr, i64 8, !dbg !20
  %error.value = load %DivisionError, ptr %error.data.i8ptr, align 4, !dbg !20
  %temp_struct = alloca %DivisionError, align 8, !dbg !20
  store %DivisionError %error.value, ptr %temp_struct, align 4, !dbg !20
  %code_ptr = getelementptr inbounds %DivisionError, ptr %temp_struct, i32 0, i32 0, !dbg !20
  %code_val = load i64, ptr %code_ptr, align 4, !dbg !20
  %temp_struct2 = alloca %DivisionError, align 8, !dbg !20
  store %DivisionError %error.value, ptr %temp_struct2, align 4, !dbg !20
  %dividend_ptr = getelementptr inbounds %DivisionError, ptr %temp_struct2, i32 0, i32 1, !dbg !20
  %dividend_val = load i64, ptr %dividend_ptr, align 4, !dbg !20
  %addtmp = add i64 %code_val, %dividend_val, !dbg !20
  br label %block.continue, !dbg !20
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_custom_error.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "divide", linkageName: "divide", scope: !1, file: !1, line: 9, type: !5, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !9)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8, !8}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!9 = !{!10, !11}
!10 = !DILocalVariable(name: "a", scope: !4, file: !1, line: 9, type: !8)
!11 = !DILocalVariable(name: "b", scope: !4, file: !1, line: 9, type: !8)
!12 = !DILocation(line: 9, column: 1, scope: !4)
!13 = !DILocation(line: 9, column: 9, scope: !4)
!14 = !DILocation(line: 9, column: 17, scope: !4)
!15 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 16, type: !16, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!16 = !DISubroutineType(types: !17)
!17 = !{!8}
!18 = !{!19}
!19 = !DILocalVariable(name: "result", scope: !15, file: !1, line: 17, type: !8)
!20 = !DILocation(line: 16, column: 1, scope: !15)
!21 = !DILocation(line: 17, column: 1, scope: !15)
