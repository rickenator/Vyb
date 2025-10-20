; ModuleID = 'VynModule'
source_filename = "VynModule"

%MyError = type { i64, { ptr, i64 } }

@0 = private unnamed_addr constant [17 x i8] c"Division by zero\00", align 1
@1 = private unnamed_addr constant [15 x i8] c"Caught error: \00", align 1
@2 = private unnamed_addr constant [17 x i8] c"Cleanup executed\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

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
  %MyError_obj = alloca %MyError, align 8, !dbg !11
  %code_ptr = getelementptr inbounds %MyError, ptr %MyError_obj, i32 0, i32 0, !dbg !11
  store i64 1, ptr %code_ptr, align 4, !dbg !11
  %message_ptr = getelementptr inbounds %MyError, ptr %MyError_obj, i32 0, i32 1, !dbg !11
  store { ptr, i64 } { ptr @0, i64 16 }, ptr %message_ptr, align 8, !dbg !11
  %MyError_val = load %MyError, ptr %MyError_obj, align 8, !dbg !11
  %error_temp = alloca %MyError, align 8, !dbg !11
  store %MyError %MyError_val, ptr %error_temp, align 8, !dbg !11
  call void @__vyn_runtime_untrapped_error(ptr %error_temp), !dbg !11
  unreachable, !dbg !11

ifcont:                                           ; preds = %entry
  %a4 = load i64, ptr %a1, align 4, !dbg !11
  %b5 = load i64, ptr %b2, align 4, !dbg !11
  %sdivtmp = sdiv i64 %a4, %b5, !dbg !11
  ret i64 %sdivtmp, !dbg !11
}

define i64 @main() !dbg !14 {
entry:
  %trap_error = alloca %MyError, align 8
  br label %block.normal, !dbg !17

block.normal:                                     ; preds = %entry
  %calltmp = call i64 @divide(i64 10, i64 0), !dbg !17
  br label %block.ensure, !dbg !17

block.ensure:                                     ; preds = %block.normal
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !17
  store { ptr, i64 } { ptr @2, i64 16 }, ptr %serialize_temp, align 8, !dbg !17
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !17
  call void @__vyn_println(ptr %serialized_json), !dbg !17
  br label %block.continue, !dbg !17

block.continue:                                   ; preds = %block.ensure
  ret i64 undef, !dbg !17

trap.landing:                                     ; No predecessors!
  %caught_error = load %MyError, ptr %trap_error, align 8, !dbg !17
  %temp_struct = alloca %MyError, align 8, !dbg !17
  store %MyError %caught_error, ptr %temp_struct, align 8, !dbg !17
  %message_ptr = getelementptr inbounds %MyError, ptr %temp_struct, i32 0, i32 1, !dbg !17
  %strcattmp = call ptr @__vyn_string_concat(ptr @1, ptr %message_ptr), !dbg !17
  call void @__vyn_println(ptr %strcattmp), !dbg !17
  ret i64 -1, !dbg !17
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #1

declare ptr @__vyn_string_concat(ptr, ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "semantic_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "divide", linkageName: "divide", scope: !1, file: !1, line: 8, type: !5, scopeLine: 8, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !7, !7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10}
!9 = !DILocalVariable(name: "a", scope: !4, file: !1, line: 8, type: !7)
!10 = !DILocalVariable(name: "b", scope: !4, file: !1, line: 8, type: !7)
!11 = !DILocation(line: 8, column: 1, scope: !4)
!12 = !DILocation(line: 8, column: 9, scope: !4)
!13 = !DILocation(line: 8, column: 17, scope: !4)
!14 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 15, type: !15, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!15 = !DISubroutineType(types: !16)
!16 = !{!7}
!17 = !DILocation(line: 15, column: 1, scope: !14)
