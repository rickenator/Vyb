; ModuleID = 'VynModule'
source_filename = "VynModule"

%Error = type { { ptr, i64 }, i64 }

@Error_message.str = private unnamed_addr constant [14 x i8] c"Error_message\00", align 1
@filepath.str = private unnamed_addr constant [28 x i8] c"test/stdlib/error_basic.vyn\00", align 1
@Error_code.str = private unnamed_addr constant [11 x i8] c"Error_code\00", align 1
@filepath.str.1 = private unnamed_addr constant [28 x i8] c"test/stdlib/error_basic.vyn\00", align 1
@Error_display.str = private unnamed_addr constant [14 x i8] c"Error_display\00", align 1
@filepath.str.2 = private unnamed_addr constant [28 x i8] c"test/stdlib/error_basic.vyn\00", align 1
@risky.str = private unnamed_addr constant [6 x i8] c"risky\00", align 1
@filepath.str.3 = private unnamed_addr constant [28 x i8] c"test/stdlib/error_basic.vyn\00", align 1
@0 = private unnamed_addr constant [21 x i8] c"Something went wrong\00", align 1
@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str.4 = private unnamed_addr constant [28 x i8] c"test/stdlib/error_basic.vyn\00", align 1

; Function Attrs: noinline
define { i64, ptr } @risky() #0 !dbg !4 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @risky.str, ptr @filepath.str.3, i32 40, i32 1), !dbg !8
  %Error_obj = alloca %Error, align 8, !dbg !8
  %message_ptr = getelementptr inbounds %Error, ptr %Error_obj, i32 0, i32 0, !dbg !8
  store { ptr, i64 } { ptr @0, i64 20 }, ptr %message_ptr, align 8, !dbg !8
  %code_ptr = getelementptr inbounds %Error, ptr %Error_obj, i32 0, i32 1, !dbg !8
  store i64 42, ptr %code_ptr, align 4, !dbg !8
  %Error_val = load %Error, ptr %Error_obj, align 8, !dbg !8
  %error.heap = call ptr @malloc(i64 32), !dbg !8
  store i64 2739431379549913418, ptr %error.heap, align 4, !dbg !8
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !8
  store %Error %Error_val, ptr %error.data.ptr, align 8, !dbg !8
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.heap, 1, !dbg !8
  ret { i64, ptr } %error.ptr, !dbg !8
}

; Function Attrs: noinline
define i64 @main() #0 !dbg !9 {
entry:
  %result = alloca i64, align 8, !dbg !15
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str.4, i32 44, i32 1), !dbg !15
  %trap_error_heap = call ptr @malloc(i64 8), !dbg !15
  store ptr null, ptr %trap_error_heap, align 8, !dbg !15
  br label %block.normal, !dbg !15

block.normal:                                     ; preds = %entry
  %calltmp = call { i64, ptr } @risky(), !dbg !15
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !15
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !15
  %has.error = icmp ne ptr %call.error, null, !dbg !15
  br i1 %has.error, label %call.error1, label %call.success, !dbg !15

block.continue:                                   ; preds = %call.success
  %block.result = phi i64 [ %call.value, %call.success ], !dbg !15
  call void @free(ptr %trap_error_heap), !dbg !15
  store i64 %block.result, ptr %result, align 4, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %result, metadata !14, metadata !DIExpression()), !dbg !16
  %result2 = load i64, ptr %result, align 4, !dbg !15
  call void @__vyn_runtime_pop_call_frame(), !dbg !15
  ret i64 %result2, !dbg !15

trap.landing:                                     ; preds = %call.error1
  %error.ptr = load ptr, ptr %trap_error_heap, align 8, !dbg !15
  %error.typeid = load i64, ptr %error.ptr, align 4, !dbg !15
  %type.matches = icmp eq i64 %error.typeid, 2739431379549913418, !dbg !15
  br i1 %type.matches, label %trap.handler0, label %trap.unmatched, !dbg !15

call.error1:                                      ; preds = %block.normal
  store ptr %call.error, ptr %trap_error_heap, align 8, !dbg !15
  br label %trap.landing, !dbg !15

call.success:                                     ; preds = %block.normal
  br label %block.continue, !dbg !15

trap.unmatched:                                   ; preds = %trap.landing
  call void @__vyn_runtime_untrapped_error(ptr %error.ptr), !dbg !15
  unreachable, !dbg !15

trap.handler0:                                    ; preds = %trap.landing
  %error.data.i8ptr = getelementptr i8, ptr %error.ptr, i64 8, !dbg !15
  %error.value = load %Error, ptr %error.data.i8ptr, align 8, !dbg !15
  call void @__vyn_runtime_pop_call_frame(), !dbg !15
  ret i64 1, !dbg !15
}

; Function Attrs: noinline
define { ptr, i64 } @Error_message(%Error %self) #0 !dbg !17 {
entry:
  %self1 = alloca %Error, align 8, !dbg !23
  call void @__vyn_runtime_push_call_frame(ptr @Error_message.str, ptr @filepath.str, i32 24, i32 1), !dbg !23
  store %Error %self, ptr %self1, align 8, !dbg !23
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !22, metadata !DIExpression()), !dbg !24
  %self2 = load %Error, ptr %self1, align 8, !dbg !23
  %temp_struct = alloca %Error, align 8, !dbg !23
  store %Error %self2, ptr %temp_struct, align 8, !dbg !23
  %message_ptr = getelementptr inbounds %Error, ptr %temp_struct, i32 0, i32 0, !dbg !23
  %member_load = load { ptr, i64 }, ptr %message_ptr, align 8, !dbg !23
  call void @__vyn_runtime_pop_call_frame(), !dbg !23
  ret { ptr, i64 } %member_load, !dbg !23
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @__vyn_runtime_pop_call_frame()

; Function Attrs: noinline
define i64 @Error_code(%Error %self) #0 !dbg !25 {
entry:
  %self1 = alloca %Error, align 8, !dbg !30
  call void @__vyn_runtime_push_call_frame(ptr @Error_code.str, ptr @filepath.str.1, i32 28, i32 5), !dbg !30
  store %Error %self, ptr %self1, align 8, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !29, metadata !DIExpression()), !dbg !31
  %self2 = load %Error, ptr %self1, align 8, !dbg !30
  %temp_struct = alloca %Error, align 8, !dbg !30
  store %Error %self2, ptr %temp_struct, align 8, !dbg !30
  %code_ptr = getelementptr inbounds %Error, ptr %temp_struct, i32 0, i32 1, !dbg !30
  %code_val = load i64, ptr %code_ptr, align 4, !dbg !30
  call void @__vyn_runtime_pop_call_frame(), !dbg !30
  ret i64 %code_val, !dbg !30
}

; Function Attrs: noinline
define { ptr, i64 } @Error_display(%Error %self) #0 !dbg !32 {
entry:
  %self1 = alloca %Error, align 8, !dbg !35
  call void @__vyn_runtime_push_call_frame(ptr @Error_display.str, ptr @filepath.str.2, i32 34, i32 1), !dbg !35
  store %Error %self, ptr %self1, align 8, !dbg !35
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !34, metadata !DIExpression()), !dbg !36
  %self2 = load %Error, ptr %self1, align 8, !dbg !35
  %temp_struct = alloca %Error, align 8, !dbg !35
  store %Error %self2, ptr %temp_struct, align 8, !dbg !35
  %message_ptr = getelementptr inbounds %Error, ptr %temp_struct, i32 0, i32 0, !dbg !35
  %member_load = load { ptr, i64 }, ptr %message_ptr, align 8, !dbg !35
  call void @__vyn_runtime_pop_call_frame(), !dbg !35
  ret { ptr, i64 } %member_load, !dbg !35
}

declare ptr @malloc(i64)

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #2

declare void @free(ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noinline }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "error_basic.vyn.ll", directory: "/home/rick/Projects/Vyn/test/stdlib")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "risky", linkageName: "risky", scope: !1, file: !1, line: 40, type: !5, scopeLine: 40, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DILocation(line: 40, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 44, type: !10, scopeLine: 44, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!10 = !DISubroutineType(types: !11)
!11 = !{!12}
!12 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!13 = !{!14}
!14 = !DILocalVariable(name: "result", scope: !9, file: !1, line: 45, type: !12)
!15 = !DILocation(line: 44, column: 1, scope: !9)
!16 = !DILocation(line: 45, column: 1, scope: !9)
!17 = distinct !DISubprogram(name: "Error_message", linkageName: "Error_message", scope: !1, file: !1, line: 24, type: !18, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !21)
!18 = !DISubroutineType(types: !19)
!19 = !{!7, !20}
!20 = !DICompositeType(tag: DW_TAG_structure_type, name: "Error", scope: !1, file: !1, size: 128, align: 8)
!21 = !{!22}
!22 = !DILocalVariable(name: "self", scope: !17, file: !1, line: 24, type: !20)
!23 = !DILocation(line: 24, column: 1, scope: !17)
!24 = !DILocation(line: 24, column: 17, scope: !17)
!25 = distinct !DISubprogram(name: "Error_code", linkageName: "Error_code", scope: !1, file: !1, line: 28, type: !26, scopeLine: 28, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !28)
!26 = !DISubroutineType(types: !27)
!27 = !{!12, !20}
!28 = !{!29}
!29 = !DILocalVariable(name: "self", scope: !25, file: !1, line: 28, type: !20)
!30 = !DILocation(line: 28, column: 5, scope: !25)
!31 = !DILocation(line: 28, column: 14, scope: !25)
!32 = distinct !DISubprogram(name: "Error_display", linkageName: "Error_display", scope: !1, file: !1, line: 34, type: !18, scopeLine: 34, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!33 = !{!34}
!34 = !DILocalVariable(name: "self", scope: !32, file: !1, line: 34, type: !20)
!35 = !DILocation(line: 34, column: 1, scope: !32)
!36 = !DILocation(line: 34, column: 17, scope: !32)
