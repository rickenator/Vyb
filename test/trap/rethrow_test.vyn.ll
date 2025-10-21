; ModuleID = 'VynModule'
source_filename = "VynModule"

%NetworkError = type { i64, { ptr, i64 } }

@0 = private unnamed_addr constant [10 x i8] c"Not found\00", align 1
@1 = private unnamed_addr constant [23 x i8] c"Caught network error: \00", align 1
@2 = private unnamed_addr constant [25 x i8] c"Caught transformed error\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define i64 @process_data() !dbg !4 {
entry:
  %trap_error = alloca ptr, align 8
  br label %block.normal, !dbg !8

block.normal:                                     ; preds = %entry
  %NetworkError_obj = alloca %NetworkError, align 8, !dbg !8
  %code_ptr = getelementptr inbounds %NetworkError, ptr %NetworkError_obj, i32 0, i32 0, !dbg !8
  store i64 404, ptr %code_ptr, align 4, !dbg !8
  %message_ptr = getelementptr inbounds %NetworkError, ptr %NetworkError_obj, i32 0, i32 1, !dbg !8
  store { ptr, i64 } { ptr @0, i64 9 }, ptr %message_ptr, align 8, !dbg !8
  %NetworkError_val = load %NetworkError, ptr %NetworkError_obj, align 8, !dbg !8
  store %NetworkError %NetworkError_val, ptr %trap_error, align 8, !dbg !8
  br label %trap.landing, !dbg !8

block.continue:                                   ; No predecessors!
  ret i64 undef, !dbg !8

trap.landing:                                     ; preds = %block.normal
  %caught_error = load ptr, ptr %trap_error, align 8, !dbg !8
  %rethrow_error = load ptr, ptr %trap_error, align 8, !dbg !8
  call void @__vyn_runtime_untrapped_error(ptr %rethrow_error), !dbg !8
  unreachable, !dbg !8
}

define i64 @main() !dbg !9 {
entry:
  %final_result = alloca i64, align 8
  %trap_error = alloca ptr, align 8
  br label %block.normal, !dbg !12

block.normal:                                     ; preds = %entry
  %calltmp = call i64 @process_data(), !dbg !12
  br label %block.continue, !dbg !12

block.continue:                                   ; preds = %block.normal
  %block.result = phi i64 [ %calltmp, %block.normal ], !dbg !12
  store i64 %block.result, ptr %final_result, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %final_result, metadata !11, metadata !DIExpression()), !dbg !13
  %final_result1 = load i64, ptr %final_result, align 4, !dbg !12
  ret i64 %final_result1, !dbg !12

trap.landing:                                     ; No predecessors!
  %caught_error = load ptr, ptr %trap_error, align 8, !dbg !12
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !12
  store { ptr, i64 } { ptr @2, i64 24 }, ptr %serialize_temp, align 8, !dbg !12
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !12
  call void @__vyn_println(ptr %serialized_json), !dbg !12
  ret i64 -1, !dbg !12
}

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #0

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noreturn }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "rethrow_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "process_data", linkageName: "process_data", scope: !1, file: !1, line: 13, type: !5, scopeLine: 13, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !DILocation(line: 13, column: 1, scope: !4)
!9 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 30, type: !5, scopeLine: 30, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !10)
!10 = !{!11}
!11 = !DILocalVariable(name: "final_result", scope: !9, file: !1, line: 31, type: !7)
!12 = !DILocation(line: 30, column: 1, scope: !9)
!13 = !DILocation(line: 31, column: 1, scope: !9)
