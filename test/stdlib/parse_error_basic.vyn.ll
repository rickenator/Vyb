; ModuleID = 'VynModule'
source_filename = "VynModule"

%ParseError = type { { ptr, i64 }, i64, i64, i64 }

@field_name_ParseError_message = private constant [8 x i8] c"message\00"
@field_type_ParseError_message = private constant [7 x i8] c"String\00"
@field_name_ParseError_code = private constant [5 x i8] c"code\00"
@field_type_ParseError_code = private constant [4 x i8] c"Int\00"
@field_name_ParseError_line = private constant [5 x i8] c"line\00"
@field_type_ParseError_line = private constant [4 x i8] c"Int\00"
@field_name_ParseError_column = private constant [7 x i8] c"column\00"
@field_type_ParseError_column = private constant [4 x i8] c"Int\00"
@__vyn_fields_ParseError = private constant [4 x { ptr, ptr, i64, i64, i1, i1, ptr }] [{ ptr, ptr, i64, i64, i1, i1, ptr } { ptr @field_name_ParseError_message, ptr @field_type_ParseError_message, i64 0, i64 16, i1 true, i1 false, ptr null }, { ptr, ptr, i64, i64, i1, i1, ptr } { ptr @field_name_ParseError_code, ptr @field_type_ParseError_code, i64 16, i64 8, i1 true, i1 false, ptr null }, { ptr, ptr, i64, i64, i1, i1, ptr } { ptr @field_name_ParseError_line, ptr @field_type_ParseError_line, i64 24, i64 8, i1 true, i1 false, ptr null }, { ptr, ptr, i64, i64, i1, i1, ptr } { ptr @field_name_ParseError_column, ptr @field_type_ParseError_column, i64 32, i64 8, i1 true, i1 false, ptr null }]
@type_name_ParseError = private constant [11 x i8] c"ParseError\00"
@__vyn_metadata_ParseError = constant { ptr, i64, i64, ptr, i64, ptr } { ptr @type_name_ParseError, i64 40, i64 4, ptr @__vyn_fields_ParseError, i64 0, ptr null }
@ParseError_message.str = private unnamed_addr constant [19 x i8] c"ParseError_message\00", align 1
@filepath.str = private unnamed_addr constant [34 x i8] c"test/stdlib/parse_error_basic.vyn\00", align 1
@ParseError_code.str = private unnamed_addr constant [16 x i8] c"ParseError_code\00", align 1
@filepath.str.1 = private unnamed_addr constant [34 x i8] c"test/stdlib/parse_error_basic.vyn\00", align 1
@ParseError_display.str = private unnamed_addr constant [19 x i8] c"ParseError_display\00", align 1
@filepath.str.2 = private unnamed_addr constant [34 x i8] c"test/stdlib/parse_error_basic.vyn\00", align 1
@parseJson.str = private unnamed_addr constant [10 x i8] c"parseJson\00", align 1
@filepath.str.3 = private unnamed_addr constant [34 x i8] c"test/stdlib/parse_error_basic.vyn\00", align 1
@0 = private unnamed_addr constant [17 x i8] c"Unexpected token\00", align 1
@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str.4 = private unnamed_addr constant [34 x i8] c"test/stdlib/parse_error_basic.vyn\00", align 1
@1 = private unnamed_addr constant [17 x i8] c"{ invalid json }\00", align 1
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__vyn_register_all_types, ptr null }]

; Function Attrs: noinline
define { i64, ptr } @parseJson({ ptr, i64 } %input) #0 !dbg !4 {
entry:
  %input1 = alloca { ptr, i64 }, align 8, !dbg !12
  call void @__vyn_runtime_push_call_frame(ptr @parseJson.str, ptr @filepath.str.3, i32 35, i32 1), !dbg !12
  store { ptr, i64 } %input, ptr %input1, align 8, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %input1, metadata !10, metadata !DIExpression()), !dbg !13
  %ParseError_obj = alloca %ParseError, align 8, !dbg !12
  %message_ptr = getelementptr inbounds %ParseError, ptr %ParseError_obj, i32 0, i32 0, !dbg !12
  store { ptr, i64 } { ptr @0, i64 16 }, ptr %message_ptr, align 8, !dbg !12
  %code_ptr = getelementptr inbounds %ParseError, ptr %ParseError_obj, i32 0, i32 1, !dbg !12
  store i64 1, ptr %code_ptr, align 4, !dbg !12
  %line_ptr = getelementptr inbounds %ParseError, ptr %ParseError_obj, i32 0, i32 2, !dbg !12
  store i64 10, ptr %line_ptr, align 4, !dbg !12
  %column_ptr = getelementptr inbounds %ParseError, ptr %ParseError_obj, i32 0, i32 3, !dbg !12
  store i64 5, ptr %column_ptr, align 4, !dbg !12
  %ParseError_val = load %ParseError, ptr %ParseError_obj, align 8, !dbg !12
  %error.heap = call ptr @malloc(i64 48), !dbg !12
  store i64 2394716306558183473, ptr %error.heap, align 4, !dbg !12
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !12
  store %ParseError %ParseError_val, ptr %error.data.ptr, align 8, !dbg !12
  %error.ptr = insertvalue { i64, ptr } undef, ptr %error.heap, 1, !dbg !12
  ret { i64, ptr } %error.ptr, !dbg !12
}

; Function Attrs: noinline
define i64 @main() #0 !dbg !14 {
entry:
  %result = alloca i64, align 8, !dbg !20
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str.4, i32 44, i32 1), !dbg !20
  %trap_error_heap = call ptr @malloc(i64 8), !dbg !20
  store ptr null, ptr %trap_error_heap, align 8, !dbg !20
  br label %block.normal, !dbg !20

block.normal:                                     ; preds = %entry
  %calltmp = call { i64, ptr } @parseJson({ ptr, i64 } { ptr @1, i64 16 }), !dbg !20
  %call.value = extractvalue { i64, ptr } %calltmp, 0, !dbg !20
  %call.error = extractvalue { i64, ptr } %calltmp, 1, !dbg !20
  %has.error = icmp ne ptr %call.error, null, !dbg !20
  br i1 %has.error, label %call.error1, label %call.success, !dbg !20

block.continue:                                   ; preds = %call.success
  %block.result = phi i64 [ %call.value, %call.success ], !dbg !20
  call void @free(ptr %trap_error_heap), !dbg !20
  store i64 %block.result, ptr %result, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata ptr %result, metadata !19, metadata !DIExpression()), !dbg !21
  %result2 = load i64, ptr %result, align 4, !dbg !20
  call void @__vyn_runtime_pop_call_frame(), !dbg !20
  ret i64 %result2, !dbg !20

trap.landing:                                     ; preds = %call.error1
  %error.ptr = load ptr, ptr %trap_error_heap, align 8, !dbg !20
  %error.typeid = load i64, ptr %error.ptr, align 4, !dbg !20
  %type.matches = icmp eq i64 %error.typeid, 2394716306558183473, !dbg !20
  br i1 %type.matches, label %trap.handler0, label %trap.unmatched, !dbg !20

call.error1:                                      ; preds = %block.normal
  store ptr %call.error, ptr %trap_error_heap, align 8, !dbg !20
  br label %trap.landing, !dbg !20

call.success:                                     ; preds = %block.normal
  br label %block.continue, !dbg !20

trap.unmatched:                                   ; preds = %trap.landing
  call void @__vyn_runtime_untrapped_error(ptr %error.ptr), !dbg !20
  unreachable, !dbg !20

trap.handler0:                                    ; preds = %trap.landing
  %error.data.i8ptr = getelementptr i8, ptr %error.ptr, i64 8, !dbg !20
  %error.value = load %ParseError, ptr %error.data.i8ptr, align 8, !dbg !20
  call void @__vyn_runtime_pop_call_frame(), !dbg !20
  ret i64 3, !dbg !20
}

; Function Attrs: noinline
define { ptr, i64 } @ParseError_message(%ParseError %self) #0 !dbg !22 {
entry:
  %self1 = alloca %ParseError, align 8, !dbg !28
  call void @__vyn_runtime_push_call_frame(ptr @ParseError_message.str, ptr @filepath.str, i32 20, i32 1), !dbg !28
  store %ParseError %self, ptr %self1, align 8, !dbg !28
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !27, metadata !DIExpression()), !dbg !29
  %self2 = load %ParseError, ptr %self1, align 8, !dbg !28
  %temp_struct = alloca %ParseError, align 8, !dbg !28
  store %ParseError %self2, ptr %temp_struct, align 8, !dbg !28
  %message_ptr = getelementptr inbounds %ParseError, ptr %temp_struct, i32 0, i32 0, !dbg !28
  %message_val = load { ptr, i64 }, ptr %message_ptr, align 8, !dbg !28
  call void @__vyn_runtime_pop_call_frame(), !dbg !28
  ret { ptr, i64 } %message_val, !dbg !28
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @__vyn_runtime_pop_call_frame()

; Function Attrs: noinline
define i64 @ParseError_code(%ParseError %self) #0 !dbg !30 {
entry:
  %self1 = alloca %ParseError, align 8, !dbg !35
  call void @__vyn_runtime_push_call_frame(ptr @ParseError_code.str, ptr @filepath.str.1, i32 24, i32 5), !dbg !35
  store %ParseError %self, ptr %self1, align 8, !dbg !35
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !34, metadata !DIExpression()), !dbg !36
  %self2 = load %ParseError, ptr %self1, align 8, !dbg !35
  %temp_struct = alloca %ParseError, align 8, !dbg !35
  store %ParseError %self2, ptr %temp_struct, align 8, !dbg !35
  %code_ptr = getelementptr inbounds %ParseError, ptr %temp_struct, i32 0, i32 1, !dbg !35
  %code_val = load i64, ptr %code_ptr, align 4, !dbg !35
  call void @__vyn_runtime_pop_call_frame(), !dbg !35
  ret i64 %code_val, !dbg !35
}

; Function Attrs: noinline
define { ptr, i64 } @ParseError_display(%ParseError %self) #0 !dbg !37 {
entry:
  %self1 = alloca %ParseError, align 8, !dbg !40
  call void @__vyn_runtime_push_call_frame(ptr @ParseError_display.str, ptr @filepath.str.2, i32 30, i32 1), !dbg !40
  store %ParseError %self, ptr %self1, align 8, !dbg !40
  call void @llvm.dbg.declare(metadata ptr %self1, metadata !39, metadata !DIExpression()), !dbg !41
  %self2 = load %ParseError, ptr %self1, align 8, !dbg !40
  %temp_struct = alloca %ParseError, align 8, !dbg !40
  store %ParseError %self2, ptr %temp_struct, align 8, !dbg !40
  %message_ptr = getelementptr inbounds %ParseError, ptr %temp_struct, i32 0, i32 0, !dbg !40
  %message_val = load { ptr, i64 }, ptr %message_ptr, align 8, !dbg !40
  call void @__vyn_runtime_pop_call_frame(), !dbg !40
  ret { ptr, i64 } %message_val, !dbg !40
}

declare ptr @malloc(i64)

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #2

declare void @free(ptr)

declare void @__vyn_register_type(ptr)

define void @__vyn_register_all_types() {
entry:
  call void @__vyn_register_type(ptr @__vyn_metadata_ParseError)
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
!1 = !DIFile(filename: "parse_error_basic.vyn.ll", directory: "/home/runner/work/Vyn/Vyn/test/stdlib")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "parseJson", linkageName: "parseJson", scope: !1, file: !1, line: 35, type: !5, scopeLine: 35, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !9)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8}
!7 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_return", scope: !1, file: !1, size: 128, align: 8)
!8 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_", scope: !1, file: !1, size: 128, align: 8)
!9 = !{!10}
!10 = !DILocalVariable(name: "input", scope: !4, file: !1, line: 35, type: !11)
!11 = !DICompositeType(tag: DW_TAG_structure_type, name: "struct_{ ptr, i64 }", scope: !1, file: !1, size: 128, align: 8)
!12 = !DILocation(line: 35, column: 1, scope: !4)
!13 = !DILocation(line: 35, column: 16, scope: !4)
!14 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 44, type: !15, scopeLine: 44, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!15 = !DISubroutineType(types: !16)
!16 = !{!17}
!17 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!18 = !{!19}
!19 = !DILocalVariable(name: "result", scope: !14, file: !1, line: 45, type: !17)
!20 = !DILocation(line: 44, column: 1, scope: !14)
!21 = !DILocation(line: 45, column: 1, scope: !14)
!22 = distinct !DISubprogram(name: "ParseError_message", linkageName: "ParseError_message", scope: !1, file: !1, line: 20, type: !23, scopeLine: 20, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !26)
!23 = !DISubroutineType(types: !24)
!24 = !{!7, !25}
!25 = !DICompositeType(tag: DW_TAG_structure_type, name: "ParseError", scope: !1, file: !1, size: 256, align: 8)
!26 = !{!27}
!27 = !DILocalVariable(name: "self", scope: !22, file: !1, line: 20, type: !25)
!28 = !DILocation(line: 20, column: 1, scope: !22)
!29 = !DILocation(line: 20, column: 17, scope: !22)
!30 = distinct !DISubprogram(name: "ParseError_code", linkageName: "ParseError_code", scope: !1, file: !1, line: 24, type: !31, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !33)
!31 = !DISubroutineType(types: !32)
!32 = !{!17, !25}
!33 = !{!34}
!34 = !DILocalVariable(name: "self", scope: !30, file: !1, line: 24, type: !25)
!35 = !DILocation(line: 24, column: 5, scope: !30)
!36 = !DILocation(line: 24, column: 14, scope: !30)
!37 = distinct !DISubprogram(name: "ParseError_display", linkageName: "ParseError_display", scope: !1, file: !1, line: 30, type: !23, scopeLine: 30, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !38)
!38 = !{!39}
!39 = !DILocalVariable(name: "self", scope: !37, file: !1, line: 30, type: !25)
!40 = !DILocation(line: 30, column: 1, scope: !37)
!41 = !DILocation(line: 30, column: 17, scope: !37)
