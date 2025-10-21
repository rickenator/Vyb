; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [39 x i8] c"=== Test 1: ensure on success path ===\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [25 x i8] c"  Executing success path\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [32 x i8] c"  ** ENSURE cleanup executed **\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@3 = private unnamed_addr constant [30 x i8] c"  After ensure, result stored\00", align 1
@type_name.3 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@4 = private unnamed_addr constant [33 x i8] c"=== Test 2: ensure with trap ===\00", align 1
@type_name.4 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@5 = private unnamed_addr constant [30 x i8] c"  About to fail with error 99\00", align 1
@type_name.5 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@6 = private unnamed_addr constant [31 x i8] c"  Caught error in trap handler\00", align 1
@type_name.6 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@7 = private unnamed_addr constant [47 x i8] c"  ** ENSURE cleanup executed (failure path) **\00", align 1
@type_name.7 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@8 = private unnamed_addr constant [35 x i8] c"  After ensure+trap, result stored\00", align 1
@type_name.8 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1

define i64 @main() !dbg !4 {
entry:
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !10
  %result1 = alloca i64, align 8, !dbg !10
  store { ptr, i64 } { ptr @0, i64 38 }, ptr %serialize_temp, align 8, !dbg !10
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !10
  call void @__vyn_println(ptr %serialized_json), !dbg !10
  br label %block.normal, !dbg !10

block.normal:                                     ; preds = %entry
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @1, i64 24 }, ptr %serialize_temp1, align 8, !dbg !10
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.1), !dbg !10
  call void @__vyn_println(ptr %serialized_json2), !dbg !10
  br label %block.ensure, !dbg !10

block.ensure:                                     ; preds = %block.normal
  %serialize_temp3 = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @2, i64 31 }, ptr %serialize_temp3, align 8, !dbg !10
  %serialized_json4 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp3, ptr @type_name.2), !dbg !10
  call void @__vyn_println(ptr %serialized_json4), !dbg !10
  br label %block.continue, !dbg !10

block.continue:                                   ; preds = %block.ensure
  store i64 42, ptr %result1, align 4, !dbg !10
  call void @llvm.dbg.declare(metadata ptr %result1, metadata !9, metadata !DIExpression()), !dbg !11
  %serialize_temp5 = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @3, i64 29 }, ptr %serialize_temp5, align 8, !dbg !10
  %serialized_json6 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp5, ptr @type_name.3), !dbg !10
  call void @__vyn_println(ptr %serialized_json6), !dbg !10
  %serialize_temp7 = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @4, i64 32 }, ptr %serialize_temp7, align 8, !dbg !10
  %serialized_json8 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp7, ptr @type_name.4), !dbg !10
  call void @__vyn_println(ptr %serialized_json8), !dbg !10
  %trap_error_heap = call ptr @malloc(i64 8), !dbg !10
  store ptr null, ptr %trap_error_heap, align 8, !dbg !10
  br label %block.normal9, !dbg !10

block.normal9:                                    ; preds = %block.continue
  %serialize_temp12 = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @5, i64 29 }, ptr %serialize_temp12, align 8, !dbg !10
  %serialized_json13 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp12, ptr @type_name.5), !dbg !10
  call void @__vyn_println(ptr %serialized_json13), !dbg !10
  %error.heap = call ptr @malloc(i64 16), !dbg !10
  store i64 -3994496327427856726, ptr %error.heap, align 4, !dbg !10
  %error.data.ptr = getelementptr i8, ptr %error.heap, i64 8, !dbg !10
  store i64 99, ptr %error.data.ptr, align 4, !dbg !10
  store ptr %error.heap, ptr %trap_error_heap, align 8, !dbg !10
  br label %trap.landing, !dbg !10

block.ensure10:                                   ; preds = %trap.handler0
  %serialize_temp16 = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @7, i64 46 }, ptr %serialize_temp16, align 8, !dbg !10
  %serialized_json17 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp16, ptr @type_name.7), !dbg !10
  call void @__vyn_println(ptr %serialized_json17), !dbg !10
  br label %block.continue11, !dbg !10

block.continue11:                                 ; preds = %block.ensure10
  call void @free(ptr %trap_error_heap), !dbg !10
  %serialize_temp18 = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @8, i64 34 }, ptr %serialize_temp18, align 8, !dbg !10
  %serialized_json19 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp18, ptr @type_name.8), !dbg !10
  call void @__vyn_println(ptr %serialized_json19), !dbg !10
  ret i64 0, !dbg !10

trap.landing:                                     ; preds = %block.normal9
  %error.ptr = load ptr, ptr %trap_error_heap, align 8, !dbg !10
  %error.typeid = load i64, ptr %error.ptr, align 4, !dbg !10
  %type.matches = icmp eq i64 %error.typeid, -3994496327427856726, !dbg !10
  br i1 %type.matches, label %trap.handler0, label %trap.unmatched, !dbg !10

trap.unmatched:                                   ; preds = %trap.landing
  call void @__vyn_runtime_untrapped_error(ptr %error.ptr), !dbg !10
  unreachable, !dbg !10

trap.handler0:                                    ; preds = %trap.landing
  %error.data.i8ptr = getelementptr i8, ptr %error.ptr, i64 8, !dbg !10
  %error.value = load i64, ptr %error.data.i8ptr, align 4, !dbg !10
  %serialize_temp14 = alloca { ptr, i64 }, align 8, !dbg !10
  store { ptr, i64 } { ptr @6, i64 30 }, ptr %serialize_temp14, align 8, !dbg !10
  %serialized_json15 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp14, ptr @type_name.6), !dbg !10
  call void @__vyn_println(ptr %serialized_json15), !dbg !10
  call void @free(ptr %error.ptr), !dbg !10
  br label %block.ensure10, !dbg !10
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @malloc(i64)

declare void @free(ptr)

; Function Attrs: noreturn
declare void @__vyn_runtime_untrapped_error(ptr) #1

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test_ensure_simple.vyn.ll", directory: "/home/rick/Projects/Vyn/test/trap")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 3, type: !5, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DILocalVariable(name: "result1", scope: !4, file: !1, line: 7, type: !7)
!10 = !DILocation(line: 3, column: 1, scope: !4)
!11 = !DILocation(line: 7, column: 1, scope: !4)
