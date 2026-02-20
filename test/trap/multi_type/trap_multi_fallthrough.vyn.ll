; ModuleID = 'VynModule'
source_filename = "VynModule"

@main.str = private unnamed_addr constant [5 x i8] c"main\00", align 1
@filepath.str = private unnamed_addr constant [48 x i8] c"test/trap/multi_type/trap_multi_fallthrough.vyn\00", align 1
@0 = private unnamed_addr constant [14 x i8] c"error message\00", align 1

; Function Attrs: noinline
define i64 @main() #0 !dbg !4 {
entry:
  call void @__vyn_runtime_push_call_frame(ptr @main.str, ptr @filepath.str, i32 4, i32 1), !dbg !8
  %trap_error_heap = call ptr @malloc(i64 8), !dbg !8
  store ptr null, ptr %trap_error_heap, align 8, !dbg !8
  br label %block.normal, !dbg !8

block.normal:                                     ; preds = %entry
  %error.heap = call ptr @malloc(i64 24), !dbg !8
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !8
  store { ptr, i64 } { ptr @0, i64 13 }, ptr %error.data.ptr, align 8, !dbg !8
  store ptr %error.heap, ptr %trap_error_heap, align 8, !dbg !8
  br label %trap.landing, !dbg !8

block.continue:                                   ; No predecessors!
  call void @free(ptr %trap_error_heap), !dbg !8
  call void @__vyn_runtime_pop_call_frame(), !dbg !8
  ret i64 undef, !dbg !8

trap.landing:                                     ; preds = %block.normal
  %error.ptr = load ptr, ptr %trap_error_heap, align 8, !dbg !8
  %error.typeid = load i64, ptr %error.ptr, align 4, !dbg !8
  %type.matches = icmp eq i64 %error.typeid, -3994496327427856726, !dbg !8
  br i1 %type.matches, label %trap.handler0, label %trap.check1, !dbg !8

trap.unmatched:                                   ; preds = %trap.check1
  call void @__vyn_runtime_untrapped_error(ptr %error.ptr), !dbg !8
  unreachable, !dbg !8

trap.handler0:                                    ; preds = %trap.landing
  %error.data.i8ptr = getelementptr i8, ptr %error.ptr, i64 8, !dbg !8
  %error.value = load i64, ptr %error.data.i8ptr, align 4, !dbg !8
  call void @__vyn_runtime_pop_call_frame(), !dbg !8
  ret i64 99, !dbg !8

trap.check1:                                      ; preds = %trap.landing
  %error.typeid1 = load i64, ptr %error.ptr, align 4, !dbg !8
  %type.matches.String = icmp eq i64 %error.typeid1, 7563291569981072216, !dbg !8
  %type.matches.or = or i1 false, %type.matches.String, !dbg !8
  %error.typeid2 = load i64, ptr %error.ptr, align 4, !dbg !8
  %type.matches.Bool = icmp eq i64 %error.typeid2, -1012572155697241872, !dbg !8
  %type.matches.or3 = or i1 %type.matches.or, %type.matches.Bool, !dbg !8
  br i1 %type.matches.or3, label %trap.handler1, label %trap.unmatched, !dbg !8

trap.handler1:                                    ; preds = %trap.check1
  call void @__vyn_runtime_pop_call_frame(), !dbg !8
  ret i64 50, !dbg !8
}

declare void @__vyn_runtime_push_call_frame(ptr, ptr, i32, i32)

declare ptr @malloc(i64)

declare void @__vyn_runtime_pop_call_frame()

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #1

declare void @free(ptr)

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { noinline }
attributes #1 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "trap_multi_fallthrough.vyn.ll", directory: "/home/runner/work/Vyn/Vyn/test/trap/multi_type")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 4, type: !5, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !DILocation(line: 4, column: 1, scope: !4)
