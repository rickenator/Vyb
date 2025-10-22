; ModuleID = 'VynModule'
source_filename = "VynModule"

%IOError = type { { ptr, i64 }, i64, { ptr, i64 } }
%ParseError = type { { ptr, i64 }, i64, i64, i64 }

@IOError_message.str = private unnamed_addr constant [16 x i8] c"IOError_message\00", align 1
@filepath.str = private unnamed_addr constant [33 x i8] c"test/stdlib/error_multi_trap.vyn\00", align 1
@IOError_code.str = private unnamed_addr constant [13 x i8] c"IOError_code\00", align 1
@filepath.str.1 = private unnamed_addr constant [33 x i8] c"test/stdlib/error_multi_trap.vyn\00", align 1
@ParseError_message.str = private unnamed_addr constant [19 x i8] c"ParseError_message\00", align 1
@filepath.str.2 = private unnamed_addr constant [33 x i8] c"test/stdlib/error_multi_trap.vyn\00", align 1
@ParseError_code.str = private unnamed_addr constant [16 x i8] c"ParseError_code\00", align 1
@filepath.str.3 = private unnamed_addr constant [33 x i8] c"test/stdlib/error_multi_trap.vyn\00", align 1
@processFile.str = private unnamed_addr constant [12 x i8] c"processFile\00", align 1
@filepath.str.4 = private unnamed_addr constant [33 x i8] c"test/stdlib/error_multi_trap.vyn\00", align 1
@0 = private unnamed_addr constant [10 x i8] c"IO failed\00", align 1
@1 = private unnamed_addr constant [10 x i8] c"/tmp/test\00", align 1
@2 = private unnamed_addr constant [13 x i8] c"Parse failed\00", align 1
@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str.5 = private unnamed_addr constant [33 x i8] c"test/stdlib/error_multi_trap.vyn\00", align 1

; Function Attrs: noinline
define { i64, ptr } @processFile(i64 %mode) #0 !dbg !4 {
entry:
  %mode1 = alloca i64, align 8, !dbg !11
  call void @__vyn_runtime_push_call_frame(ptr @processFile.str, ptr @filepath.str.4, i32 41, i32 1), !dbg !11
  store i64 %mode, ptr %mode1, align 4, !dbg !11
  call void @llvm.dbg.declare(metadata ptr %mode1, metadata !10, metadata !DIExpression()), !dbg !12
  %mode2 = load i64, ptr %mode1, align 4, !dbg !11
  %icmpeqtmp = icmp eq i64 %mode2, 1, !dbg !11
  br i1 %icmpeqtmp, label %then, label %ifcont, !dbg !11

then:                                             ; preds = %entry
  %IOError_obj = alloca %IOError, align 8, !dbg !11
  %message_ptr = getelementptr inbounds %IOError, ptr %IOError_obj, i32 0, i32 0, !dbg !11
  store { ptr, i64 } { ptr @0, i64 9 }, ptr %message_ptr, align 8, !dbg !11
  %code_ptr = getelementptr inbounds %IOError, ptr %IOError_obj, i32 0, i32 1, !dbg !11
  store i64 1, ptr %code_ptr, align 4, !dbg !11
  %path_ptr = getelementptr inbounds %IOError, ptr %IOError_obj, i32 0, i32 2, !dbg !11
  store { ptr, i64 } { ptr @1, i64 9 }, ptr %path_ptr, align 8, !dbg !11
  %IOError_val = load %IOError, ptr %IOError_obj, align 8, !dbg !11
  %error.heap = call ptr @malloc(i64 48), !dbg !11
  store i64 -4894357878115938473, ptr %error.heap, align 4, !dbg !11
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !11
  store %IOError %IOError_val, ptr %error.data.ptr, align 8, !dbg !11
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.heap, 1, !dbg !11
  ret { i64, ptr } %error.ptr, !dbg !11

ifcont:                                           ; preds = %entry
  %ParseError_obj = alloca %ParseError, align 8, !dbg !11
  %message_ptr3 = getelementptr inbounds %ParseError, ptr %ParseError_obj, i32 0, i32 0, !dbg !11
  store { ptr, i64 } { ptr @2, i64 12 }, ptr %message_ptr3, align 8, !dbg !11
  %code_ptr4 = getelementptr inbounds %ParseError, ptr %ParseError_obj, i32 0, i32 1, !dbg !11
  store i64 2, ptr %code_ptr4, align 4, !dbg !11
  %line_ptr = getelementptr inbounds %ParseError, ptr %ParseError_obj, i32 0, i32 2, !dbg !11
  store i64 1, ptr %line_ptr, align 4, !dbg !11
  %column_ptr = getelementptr inbounds %ParseError, ptr %ParseError_obj, i32 0, i32 3, !dbg !11
  store i64 1, ptr %column_ptr, align 4, !dbg !11
  %ParseError_val = load %ParseError, ptr %ParseError_obj, align 8, !dbg !11
  %error.heap5 = call ptr @malloc(i64 48), !dbg !11
  store i64 2394716306558183473, ptr %error.heap5, align 4, !dbg !11
  %error.data.ptr6 = getelementptr i8, ptr %error.heap5, i64 8, !dbg !11
  store %ParseError %ParseError_val, ptr %error.data.ptr6, align 8, !dbg !11
  %error.ptr7 = insertvalue { i64, ptr } undef, ptr %error.heap5, 1, !dbg !11
  ret { i64, ptr } %error.ptr7, !dbg !11
}

; Function Attrs: noinline
define i64 @main() #0 !dbg !13 {
entry:
  %result = alloca i64, align 8, !dbg !18
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str.5, i32 48, i32 1), !dbg !18
  %trap_error_heap = call ptr @malloc(i64 8), !dbg !18
  store ptr null, ptr %trap_error_heap, align 8, !dbg !18
  br label %block.normal, !dbg !18

block.normal:                                     ; preds = %entry
  %calltmp = call { i64, ptr } @processFile(i64 2), !dbg !18
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !18
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !18
  %has.error = icmp ne ptr %call.error, null, !dbg !18
  br i1 %has.error, label %call.error1, label %call.success, !dbg !18

block.continue:                                   ; preds = %call.success
  %block.result = phi i64 [ %call.value, %call.success ], !dbg !18
  call void @free(ptr %trap_error_heap), !dbg !18
  store i64 %block.result, ptr %result, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %result, metadata !17, metadata !DIExpression()), !dbg !19
  %result4 = load i64, ptr %result, align 4, !dbg !18
  call void @__vyn_runtime_pop_call_frame(), !dbg !18
  ret i64 %result4, !dbg !18

trap.landing:                                     ; preds = %call.error1
  %error.ptr = load ptr, ptr %trap_error_heap, align 8, !dbg !18
  %error.typeid = load i64, ptr %error.ptr, align 4, !dbg !18
  %type.matches.IOError = icmp eq i64 %error.typeid, -4894357878115938473, !dbg !18
  %type.matches.or = or i1 false, %type.matches.IOError, !dbg !18
  %error.typeid2 = load i64, ptr %error.ptr, align 4, !dbg !18
  %type.matches.ParseError = icmp eq i64 %error.typeid2, 2394716306558183473, !dbg !18
  %type.matches.or3 = or i1 %type.matches.or, %type.matches.ParseError, !dbg !18
  br i1 %type.matches.or3, label %trap.handler0, label %trap.unmatched, !dbg !18

call.error1:                                      ; preds = %block.normal
  store ptr %call.error, ptr %trap_error_heap, align 8, !dbg !18
  br label %trap.landing, !dbg !18

call.success:                                     ; preds = %block.normal
  br label %block.continue, !dbg !18

trap.unmatched:                                   ; preds = %trap.landing
  call void @__vyn_runtime_untrapped_error(ptr %error.ptr), !dbg !18
  unreachable, !dbg !18

trap.handler0:                                    ; preds = %trap.landing
  call void @__vyn_runtime_pop_call_frame(), !dbg !18
  ret i64 5, !dbg !18
}

; Function Attrs: noinline
define { ptr, i64 } @IOError_message(%IOError %self) #0 !dbg !20 {
entry:
  %self1 = alloca %IOError, align 8, !dbg !26
  call void @__vyn_runtime_push_call_frame(ptr @IOError_message.str, ptr @filepath.str, i32 22, i32 1), !dbg !26
  store %IOError %self, ptr %self1, align 8, !dbg !26
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !25, metadata !DIExpression()), !dbg !27
  %self2 = load %IOError, ptr %self1, align 8, !dbg !26
  %temp_struct = alloca %IOError, align 8, !dbg !26
  store %IOError %self2, ptr %temp_struct, align 8, !dbg !26
  %message_ptr = getelementptr inbounds %IOError, ptr %temp_struct, i32 0, i32 0, !dbg !26
  %member_load = load { ptr, i64 }, ptr %message_ptr, align 8, !dbg !26
  call void @__vyn_runtime_pop_call_frame(), !dbg !26
  ret { ptr, i64 } %member_load, !dbg !26
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @__vyn_runtime_pop_call_frame()

; Function Attrs: noinline
define i64 @IOError_code(%IOError %self) #0 !dbg !28 {
entry:
  %self1 = alloca %IOError, align 8, !dbg !33
  call void @__vyn_runtime_push_call_frame(ptr @IOError_code.str, ptr @filepath.str.1, i32 26, i32 5), !dbg !33
  store %IOError %self, ptr %self1, align 8, !dbg !33
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !32, metadata !DIExpression()), !dbg !34
  %self2 = load %IOError, ptr %self1, align 8, !dbg !33
  %temp_struct = alloca %IOError, align 8, !dbg !33
  store %IOError %self2, ptr %temp_struct, align 8, !dbg !33
  %code_ptr = getelementptr inbounds %IOError, ptr %temp_struct, i32 0, i32 1, !dbg !33
  %code_val = load i64, ptr %code_ptr, align 4, !dbg !33
  call void @__vyn_runtime_pop_call_frame(), !dbg !33
  ret i64 %code_val, !dbg !33
}

; Function Attrs: noinline
define { ptr, i64 } @ParseError_message(%ParseError %self) #0 !dbg !35 {
entry:
  %self1 = alloca %ParseError, align 8, !dbg !41
  call void @__vyn_runtime_push_call_frame(ptr @ParseError_message.str, ptr @filepath.str.2, i32 32, i32 1), !dbg !41
  store %ParseError %self, ptr %self1, align 8, !dbg !41
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !40, metadata !DIExpression()), !dbg !42
  %self2 = load %ParseError, ptr %self1, align 8, !dbg !41
  %temp_struct = alloca %ParseError, align 8, !dbg !41
  store %ParseError %self2, ptr %temp_struct, align 8, !dbg !41
  %message_ptr = getelementptr inbounds %ParseError, ptr %temp_struct, i32 0, i32 0, !dbg !41
  %member_load = load { ptr, i64 }, ptr %message_ptr, align 8, !dbg !41
  call void @__vyn_runtime_pop_call_frame(), !dbg !41
  ret { ptr, i64 } %member_load, !dbg !41
}

; Function Attrs: noinline
define i64 @ParseError_code(%ParseError %self) #0 !dbg !43 {
entry:
  %self1 = alloca %ParseError, align 8, !dbg !48
  call void @__vyn_runtime_push_call_frame(ptr @ParseError_code.str, ptr @filepath.str.3, i32 36, i32 5), !dbg !48
  store %ParseError %self, ptr %self1, align 8, !dbg !48
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !47, metadata !DIExpression()), !dbg !49
  %self2 = load %ParseError, ptr %self1, align 8, !dbg !48
  %temp_struct = alloca %ParseError, align 8, !dbg !48
  store %ParseError %self2, ptr %temp_struct, align 8, !dbg !48
  %code_ptr = getelementptr inbounds %ParseError, ptr %temp_struct, i32 0, i32 1, !dbg !48
  %code_val = load i64, ptr %code_ptr, align 4, !dbg !48
  call void @__vyn_runtime_pop_call_frame(), !dbg !48
  ret i64 %code_val, !dbg !48
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
!1 = !DIFile(filename: "error_multi_trap.vyn.ll", directory: "/home/rick/Projects/Vyn/test/stdlib")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "processFile", linkageName: "processFile", scope: !1, file: !1, line: 41, type: !5, scopeLine: 41, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !9)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!9 = !{!10}
!10 = !DILocalVariable(name: "mode", scope: !4, file: !1, line: 41, type: !8)
!11 = !DILocation(line: 41, column: 1, scope: !4)
!12 = !DILocation(line: 41, column: 17, scope: !4)
!13 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 48, type: !14, scopeLine: 48, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !16)
!14 = !DISubroutineType(types: !15)
!15 = !{!8}
!16 = !{!17}
!17 = !DILocalVariable(name: "result", scope: !13, file: !1, line: 50, type: !8)
!18 = !DILocation(line: 48, column: 1, scope: !13)
!19 = !DILocation(line: 50, column: 1, scope: !13)
!20 = distinct !DISubprogram(name: "IOError_message", linkageName: "IOError_message", scope: !1, file: !1, line: 22, type: !21, scopeLine: 22, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !24)
!21 = !DISubroutineType(types: !22)
!22 = !{!7, !23}
!23 = !DICompositeType(tag: DW_TAG_structure_type, name: "IOError", scope: !1, file: !1, size: 192, align: 8)
!24 = !{!25}
!25 = !DILocalVariable(name: "self", scope: !20, file: !1, line: 22, type: !23)
!26 = !DILocation(line: 22, column: 1, scope: !20)
!27 = !DILocation(line: 22, column: 17, scope: !20)
!28 = distinct !DISubprogram(name: "IOError_code", linkageName: "IOError_code", scope: !1, file: !1, line: 26, type: !29, scopeLine: 26, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !31)
!29 = !DISubroutineType(types: !30)
!30 = !{!8, !23}
!31 = !{!32}
!32 = !DILocalVariable(name: "self", scope: !28, file: !1, line: 26, type: !23)
!33 = !DILocation(line: 26, column: 5, scope: !28)
!34 = !DILocation(line: 26, column: 14, scope: !28)
!35 = distinct !DISubprogram(name: "ParseError_message", linkageName: "ParseError_message", scope: !1, file: !1, line: 32, type: !36, scopeLine: 32, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !39)
!36 = !DISubroutineType(types: !37)
!37 = !{!7, !38}
!38 = !DICompositeType(tag: DW_TAG_structure_type, name: "ParseError", scope: !1, file: !1, size: 256, align: 8)
!39 = !{!40}
!40 = !DILocalVariable(name: "self", scope: !35, file: !1, line: 32, type: !38)
!41 = !DILocation(line: 32, column: 1, scope: !35)
!42 = !DILocation(line: 32, column: 17, scope: !35)
!43 = distinct !DISubprogram(name: "ParseError_code", linkageName: "ParseError_code", scope: !1, file: !1, line: 36, type: !44, scopeLine: 36, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !46)
!44 = !DISubroutineType(types: !45)
!45 = !{!8, !38}
!46 = !{!47}
!47 = !DILocalVariable(name: "self", scope: !43, file: !1, line: 36, type: !38)
!48 = !DILocation(line: 36, column: 5, scope: !43)
!49 = !DILocation(line: 36, column: 14, scope: !43)
