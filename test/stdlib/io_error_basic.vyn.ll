; ModuleID = 'VynModule'
source_filename = "VynModule"

%IOError = type { { ptr, i64 }, i64, { ptr, i64 } }

@field_name_IOError_message = private constant [8 x i8] c"message\00"
@field_type_IOError_message = private constant [7 x i8] c"String\00"
@field_name_IOError_code = private constant [5 x i8] c"code\00"
@field_type_IOError_code = private constant [4 x i8] c"Int\00"
@field_name_IOError_path = private constant [5 x i8] c"path\00"
@field_type_IOError_path = private constant [7 x i8] c"String\00"
@__vyn_fields_IOError = private constant [3 x { ptr, ptr, i64, i64, i1, i1, ptr }] [{ ptr, ptr, i64, i64, i1, i1, ptr } { ptr @field_name_IOError_message, ptr @field_type_IOError_message, i64 0, i64 16, i1 true, i1 false, ptr null }, { ptr, ptr, i64, i64, i1, i1, ptr } { ptr @field_name_IOError_code, ptr @field_type_IOError_code, i64 16, i64 8, i1 true, i1 false, ptr null }, { ptr, ptr, i64, i64, i1, i1, ptr } { ptr @field_name_IOError_path, ptr @field_type_IOError_path, i64 24, i64 16, i1 true, i1 false, ptr null }]
@type_name_IOError = private constant [8 x i8] c"IOError\00"
@__vyn_metadata_IOError = constant { ptr, i64, i64, ptr, i64, ptr } { ptr @type_name_IOError, i64 40, i64 3, ptr @__vyn_fields_IOError, i64 0, ptr null }
@IOError_message.str = private unnamed_addr constant [16 x i8] c"IOError_message\00", align 1
@filepath.str = private unnamed_addr constant [31 x i8] c"test/stdlib/io_error_basic.vyn\00", align 1
@IOError_code.str = private unnamed_addr constant [13 x i8] c"IOError_code\00", align 1
@filepath.str.1 = private unnamed_addr constant [31 x i8] c"test/stdlib/io_error_basic.vyn\00", align 1
@IOError_display.str = private unnamed_addr constant [16 x i8] c"IOError_display\00", align 1
@filepath.str.2 = private unnamed_addr constant [31 x i8] c"test/stdlib/io_error_basic.vyn\00", align 1
@readFile.str = private unnamed_addr constant [9 x i8] c"readFile\00", align 1
@filepath.str.3 = private unnamed_addr constant [31 x i8] c"test/stdlib/io_error_basic.vyn\00", align 1
@0 = private unnamed_addr constant [15 x i8] c"File not found\00", align 1
@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str.4 = private unnamed_addr constant [31 x i8] c"test/stdlib/io_error_basic.vyn\00", align 1
@1 = private unnamed_addr constant [17 x i8] c"/tmp/missing.txt\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__vyn_register_all_types, ptr null }]

; Function Attrs: noinline
define { { ptr, i64 }, ptr } @readFile({ ptr, i64 } %path) #0 !dbg !4 {
entry:
  %path1 = alloca { ptr, i64 }, align 8, !dbg !12
  call void @__vyn_runtime_push_call_frame(ptr @readFile.str, ptr @filepath.str.3, i32 34, i32 1), !dbg !12
  store { ptr, i64 } %path, ptr %path1, align 8, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %path1, metadata !10, metadata !DIExpression()), !dbg !13
  %IOError_obj = alloca %IOError, align 8, !dbg !12
  %message_ptr = getelementptr inbounds %IOError, ptr %IOError_obj, i32 0, i32 0, !dbg !12
  store { ptr, i64 } { ptr @0, i64 14 }, ptr %message_ptr, align 8, !dbg !12
  %code_ptr = getelementptr inbounds %IOError, ptr %IOError_obj, i32 0, i32 1, !dbg !12
  store i64 2, ptr %code_ptr, align 4, !dbg !12
  %path2 = load { ptr, i64 }, ptr %path1, align 8, !dbg !12
  %path_ptr = getelementptr inbounds %IOError, ptr %IOError_obj, i32 0, i32 2, !dbg !12
  store { ptr, i64 } %path2, ptr %path_ptr, align 8, !dbg !12
  %IOError_val = load %IOError, ptr %IOError_obj, align 8, !dbg !12
  %error.heap = call ptr @malloc(i64 48), !dbg !12
  store i64 -4894357878115938473, ptr %error.heap, align 4, !dbg !12
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !12
  store %IOError %IOError_val, ptr %error.data.ptr, align 8, !dbg !12
  %error.ptr = insertvalue { { ptr, i64 }, ptr } undef, ptr %error.heap, 1, !dbg !12
  ret { { ptr, i64 }, ptr } %error.ptr, !dbg !12
}

; Function Attrs: noinline
define i64 @main() #0 !dbg !14 {
entry:
  %content = alloca { ptr, i64 }, align 8, !dbg !21
  %result = alloca i64, align 8, !dbg !21
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str.4, i32 42, i32 1), !dbg !21
  %trap_error_heap = call ptr @malloc(i64 8), !dbg !21
  store ptr null, ptr %trap_error_heap, align 8, !dbg !21
  br label %block.normal, !dbg !21

block.normal:                                     ; preds = %entry
  %calltmp = call { { ptr, i64 }, ptr } @readFile({ ptr, i64 } { ptr @1, i64 16 }), !dbg !21
  %call.value = extractvalue { { ptr, i64 }, ptr } %calltmp, 0, !dbg !21
  %call.error = extractvalue { { ptr, i64 }, ptr } %calltmp, 1, !dbg !21
  %has.error = icmp ne ptr %call.error, null, !dbg !21
  br i1 %has.error, label %call.error1, label %call.success, !dbg !21

block.continue:                                   ; No predecessors!
  call void @free(ptr %trap_error_heap), !dbg !21
  store i64 0, ptr %result, align 4, !dbg !21
  call void @llvm.dbg.declare(metadata ptr %result, metadata !20, metadata !DIExpression()), !dbg !22
  %result2 = load i64, ptr %result, align 4, !dbg !21
  call void @__vyn_runtime_pop_call_frame(), !dbg !21
  ret i64 %result2, !dbg !21

trap.landing:                                     ; preds = %call.error1
  %error.ptr = load ptr, ptr %trap_error_heap, align 8, !dbg !21
  %error.typeid = load i64, ptr %error.ptr, align 4, !dbg !21
  %type.matches = icmp eq i64 %error.typeid, -4894357878115938473, !dbg !21
  br i1 %type.matches, label %trap.handler0, label %trap.unmatched, !dbg !21

call.error1:                                      ; preds = %block.normal
  store ptr %call.error, ptr %trap_error_heap, align 8, !dbg !21
  br label %trap.landing, !dbg !21

call.success:                                     ; preds = %block.normal
  store { ptr, i64 } %call.value, ptr %content, align 8, !dbg !21
  call void @llvm.dbg.declare(metadata ptr %content, metadata !19, metadata !DIExpression()), !dbg !23
  call void @__vyn_runtime_pop_call_frame(), !dbg !21
  ret i64 0, !dbg !21

trap.unmatched:                                   ; preds = %trap.landing
  call void @__vyn_runtime_untrapped_error(ptr %error.ptr), !dbg !21
  unreachable, !dbg !21

trap.handler0:                                    ; preds = %trap.landing
  %error.data.i8ptr = getelementptr i8, ptr %error.ptr, i64 8, !dbg !21
  %error.value = load %IOError, ptr %error.data.i8ptr, align 8, !dbg !21
  call void @__vyn_runtime_pop_call_frame(), !dbg !21
  ret i64 2, !dbg !21
}

; Function Attrs: noinline
define { ptr, i64 } @IOError_message(%IOError %self) #0 !dbg !24 {
entry:
  %self1 = alloca %IOError, align 8, !dbg !30
  call void @__vyn_runtime_push_call_frame(ptr @IOError_message.str, ptr @filepath.str, i32 19, i32 1), !dbg !30
  store %IOError %self, ptr %self1, align 8, !dbg !30
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !29, metadata !DIExpression()), !dbg !31
  %self2 = load %IOError, ptr %self1, align 8, !dbg !30
  %temp_struct = alloca %IOError, align 8, !dbg !30
  store %IOError %self2, ptr %temp_struct, align 8, !dbg !30
  %message_ptr = getelementptr inbounds %IOError, ptr %temp_struct, i32 0, i32 0, !dbg !30
  %message_val = load { ptr, i64 }, ptr %message_ptr, align 8, !dbg !30
  call void @__vyn_runtime_pop_call_frame(), !dbg !30
  ret { ptr, i64 } %message_val, !dbg !30
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @__vyn_runtime_pop_call_frame()

; Function Attrs: noinline
define i64 @IOError_code(%IOError %self) #0 !dbg !32 {
entry:
  %self1 = alloca %IOError, align 8, !dbg !37
  call void @__vyn_runtime_push_call_frame(ptr @IOError_code.str, ptr @filepath.str.1, i32 23, i32 5), !dbg !37
  store %IOError %self, ptr %self1, align 8, !dbg !37
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !36, metadata !DIExpression()), !dbg !38
  %self2 = load %IOError, ptr %self1, align 8, !dbg !37
  %temp_struct = alloca %IOError, align 8, !dbg !37
  store %IOError %self2, ptr %temp_struct, align 8, !dbg !37
  %code_ptr = getelementptr inbounds %IOError, ptr %temp_struct, i32 0, i32 1, !dbg !37
  %code_val = load i64, ptr %code_ptr, align 4, !dbg !37
  call void @__vyn_runtime_pop_call_frame(), !dbg !37
  ret i64 %code_val, !dbg !37
}

; Function Attrs: noinline
define { ptr, i64 } @IOError_display(%IOError %self) #0 !dbg !39 {
entry:
  %self1 = alloca %IOError, align 8, !dbg !42
  call void @__vyn_runtime_push_call_frame(ptr @IOError_display.str, ptr @filepath.str.2, i32 29, i32 1), !dbg !42
  store %IOError %self, ptr %self1, align 8, !dbg !42
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !41, metadata !DIExpression()), !dbg !43
  %self2 = load %IOError, ptr %self1, align 8, !dbg !42
  %temp_struct = alloca %IOError, align 8, !dbg !42
  store %IOError %self2, ptr %temp_struct, align 8, !dbg !42
  %message_ptr = getelementptr inbounds %IOError, ptr %temp_struct, i32 0, i32 0, !dbg !42
  %message_val = load { ptr, i64 }, ptr %message_ptr, align 8, !dbg !42
  call void @__vyn_runtime_pop_call_frame(), !dbg !42
  ret { ptr, i64 } %message_val, !dbg !42
}

declare ptr @malloc(i64)

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #2

declare void @free(ptr)

declare void @__vyn_register_type(ptr)

define void @__vyn_register_all_types() {
entry:
  call void @__vyn_register_type(ptr @__vyn_metadata_IOError)
  ret void
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noinline }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "io_error_basic.vyn.ll", directory: "/home/runner/work/Vyn/Vyn/test/stdlib")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "readFile", linkageName: "readFile", scope: !1, file: !1, line: 34, type: !5, scopeLine: 34, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !9)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_", scope: !1, file: !1, size: 128, align: 8)
!9 = !{!10}
!10 = !DILocalVariable(name: "path", scope: !4, file: !1, line: 34, type: !11)
!11 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!12 = !DILocation(line: 34, column: 1, scope: !4)
!13 = !DILocation(line: 34, column: 14, scope: !4)
!14 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 42, type: !15, scopeLine: 42, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!15 = !DISubroutineType(types: !16)
!16 = !{!17}
!17 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!18 = !{!19, !20}
!19 = !DILocalVariable(name: "content", scope: !14, file: !1, line: 44, type: !11)
!20 = !DILocalVariable(name: "result", scope: !14, file: !1, line: 43, type: !17)
!21 = !DILocation(line: 42, column: 1, scope: !14)
!22 = !DILocation(line: 43, column: 1, scope: !14)
!23 = !DILocation(line: 44, column: 1, scope: !14)
!24 = distinct !DISubprogram(name: "IOError_message", linkageName: "IOError_message", scope: !1, file: !1, line: 19, type: !25, scopeLine: 19, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !28)
!25 = !DISubroutineType(types: !26)
!26 = !{!7, !27}
!27 = !DICompositeType(tag: DW_TAG_structure_type, name: "IOError", scope: !1, file: !1, size: 192, align: 8)
!28 = !{!29}
!29 = !DILocalVariable(name: "self", scope: !24, file: !1, line: 19, type: !27)
!30 = !DILocation(line: 19, column: 1, scope: !24)
!31 = !DILocation(line: 19, column: 17, scope: !24)
!32 = distinct !DISubprogram(name: "IOError_code", linkageName: "IOError_code", scope: !1, file: !1, line: 23, type: !33, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !35)
!33 = !DISubroutineType(types: !34)
!34 = !{!17, !27}
!35 = !{!36}
!36 = !DILocalVariable(name: "self", scope: !32, file: !1, line: 23, type: !27)
!37 = !DILocation(line: 23, column: 5, scope: !32)
!38 = !DILocation(line: 23, column: 14, scope: !32)
!39 = distinct !DISubprogram(name: "IOError_display", linkageName: "IOError_display", scope: !1, file: !1, line: 29, type: !25, scopeLine: 29, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !40)
!40 = !{!41}
!41 = !DILocalVariable(name: "self", scope: !39, file: !1, line: 29, type: !27)
!42 = !DILocation(line: 29, column: 1, scope: !39)
!43 = !DILocation(line: 29, column: 17, scope: !39)
