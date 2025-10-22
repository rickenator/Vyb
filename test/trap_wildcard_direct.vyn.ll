; ModuleID = 'VynModule'
source_filename = "VynModule"

@risky_operation.str = private unnamed_addr constant [16 x i8] c"risky_operation\00", align 1
@filepath.str = private unnamed_addr constant [30 x i8] c"test/trap_wildcard_direct.vyn\00", align 1
@0 = private unnamed_addr constant [21 x i8] c"Something went wrong\00", align 1
@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str.1 = private unnamed_addr constant [30 x i8] c"test/trap_wildcard_direct.vyn\00", align 1
@1 = private unnamed_addr constant [23 x i8] c"Caught wildcard error!\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [22 x i8] c"This should not print\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

; Function Attrs: noinline
define { i64, ptr } @risky_operation() #0 !dbg !4 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @risky_operation.str, ptr @filepath.str, i32 3, i32 1), !dbg !8
  %error.heap = call ptr @malloc(i64 24), !dbg !8
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !8
  store { ptr, i64 } { ptr @0, i64 20 }, ptr %error.data.ptr, align 8, !dbg !8
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.heap, 1, !dbg !8
  ret { i64, ptr } %error.ptr, !dbg !8
}

; Function Attrs: noinline
define i64 @main() #0 !dbg !9 {
entry:
  %value = alloca i64, align 8, !dbg !15
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str.1, i32 7, i32 1), !dbg !15
  %trap_error_heap = call ptr @malloc(i64 8), !dbg !15
  store ptr null, ptr %trap_error_heap, align 8, !dbg !15
  br label %block.normal, !dbg !15

block.normal:                                     ; preds = %entry
  %calltmp = call { i64, ptr } @risky_operation(), !dbg !15
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !15
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !15
  %has.error = icmp ne ptr %call.error, null, !dbg !15
  br i1 %has.error, label %call.error1, label %call.success, !dbg !15

block.continue:                                   ; preds = %call.success
  %block.result = phi i64 [ %call.value, %call.success ], !dbg !15
  call void @free(ptr %trap_error_heap), !dbg !15
  store i64 %block.result, ptr %value, align 4, !dbg !15
  call void @llvm.dbg.declare(metadata ptr %value, metadata !14, metadata !DIExpression()), !dbg !16
  %serialize_temp2 = alloca { ptr, i64 }, align 8, !dbg !15
  store { ptr, i64 } { ptr @2, i64 21 }, ptr %serialize_temp2, align 8, !dbg !15
  %serialized_json3 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp2, ptr @type_name.2), !dbg !15
  call void @__vyn_println(ptr %serialized_json3), !dbg !15
  %value4 = load i64, ptr %value, align 4, !dbg !15
  call void @__vyn_runtime_pop_call_frame(), !dbg !15
  ret i64 %value4, !dbg !15

trap.landing:                                     ; preds = %call.error1
  %error.ptr = load ptr, ptr %trap_error_heap, align 8, !dbg !15
  br i1 true, label %trap.handler0, label %trap.unmatched, !dbg !15

call.error1:                                      ; preds = %block.normal
  store ptr %call.error, ptr %trap_error_heap, align 8, !dbg !15
  br label %trap.landing, !dbg !15

call.success:                                     ; preds = %block.normal
  br label %block.continue, !dbg !15

trap.unmatched:                                   ; preds = %trap.landing
  call void @__vyn_runtime_untrapped_error(ptr %error.ptr), !dbg !15
  unreachable, !dbg !15

trap.handler0:                                    ; preds = %trap.landing
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !15
  store { ptr, i64 } { ptr @1, i64 22 }, ptr %serialize_temp, align 8, !dbg !15
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !15
  call void @__vyn_println(ptr %serialized_json), !dbg !15
  call void @__vyn_runtime_pop_call_frame(), !dbg !15
  ret i64 99, !dbg !15
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

declare ptr @malloc(i64)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

declare void @__vyn_runtime_pop_call_frame()

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #1

declare void @free(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #2

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noinline }
attributes #1 = { noreturn }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "trap_wildcard_direct.vyn.ll", directory: "/home/rick/Projects/Vyn/test")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "risky_operation", linkageName: "risky_operation", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DILocation(line: 3, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 7, type: !10, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !13)
!10 = !DISubroutineType(types: !11)
!11 = !{!12}
!12 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!13 = !{!14}
!14 = !DILocalVariable(name: "value", scope: !9, file: !1, line: 8, type: !12)
!15 = !DILocation(line: 7, column: 1, scope: !9)
!16 = !DILocation(line: 8, column: 1, scope: !9)
