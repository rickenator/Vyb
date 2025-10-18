; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [29 x i8] c"=== Basic Ownership Test ===\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [29 x i8] c"Test 1: Created my<Int> = 42\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [36 x i8] c"Test 2: Borrowed and modified to 99\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@3 = private unnamed_addr constant [21 x i8] c"Value after borrow: \00", align 1
@4 = private unnamed_addr constant [34 x i8] c"Test 3: Viewed value through view\00", align 1
@type_name.3 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@5 = private unnamed_addr constant [14 x i8] c"Final value: \00", align 1

define i64 @main() !dbg !4 {
entry:
  %v = alloca ptr, align 8, !dbg !13
  %b = alloca ptr, align 8, !dbg !13
  %x = alloca ptr, align 8, !dbg !13
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !13
  store { ptr, i64 } { ptr @0, i64 28 }, ptr %serialize_temp, align 8, !dbg !13
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !13
  call void @__vyn_println(ptr %serialized_json), !dbg !13
  store ptr inttoptr (i64 42 to ptr), ptr %x, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %x, metadata !9, metadata !DIExpression()), !dbg !14
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !13
  store { ptr, i64 } { ptr @1, i64 28 }, ptr %serialize_temp1, align 8, !dbg !13
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.1), !dbg !13
  call void @__vyn_println(ptr %serialized_json2), !dbg !13
  store ptr %x, ptr %b, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %b, metadata !11, metadata !DIExpression()), !dbg !15
  store ptr inttoptr (i64 99 to ptr), ptr %b, align 8, !dbg !13
  %serialize_temp3 = alloca { ptr, i64 }, align 8, !dbg !13
  store { ptr, i64 } { ptr @2, i64 35 }, ptr %serialize_temp3, align 8, !dbg !13
  %serialized_json4 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp3, ptr @type_name.2), !dbg !13
  call void @__vyn_println(ptr %serialized_json4), !dbg !13
  %x5 = load ptr, ptr %x, align 8, !dbg !13
  %tostring = call ptr @__vyn_toString_string(ptr %x5), !dbg !13
  %strcattmp = call ptr @__vyn_string_concat(ptr @3, ptr %tostring), !dbg !13
  call void @__vyn_println(ptr %strcattmp), !dbg !13
  store ptr %x, ptr %v, align 8, !dbg !13
  call void @llvm.dbg.declare(metadata ptr %v, metadata !12, metadata !DIExpression()), !dbg !16
  %serialize_temp6 = alloca { ptr, i64 }, align 8, !dbg !13
  store { ptr, i64 } { ptr @4, i64 33 }, ptr %serialize_temp6, align 8, !dbg !13
  %serialized_json7 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp6, ptr @type_name.3), !dbg !13
  call void @__vyn_println(ptr %serialized_json7), !dbg !13
  %x8 = load ptr, ptr %x, align 8, !dbg !13
  %tostring9 = call ptr @__vyn_toString_string(ptr %x8), !dbg !13
  %strcattmp10 = call ptr @__vyn_string_concat(ptr @5, ptr %tostring9), !dbg !13
  call void @__vyn_println(ptr %strcattmp10), !dbg !13
  %x11 = load ptr, ptr %x, align 8, !dbg !13
  %ptrtoint_cast = ptrtoint ptr %x11 to i64, !dbg !13
  ret i64 %ptrtoint_cast, !dbg !13
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_toString_string(ptr)

declare ptr @__vyn_string_concat(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "ownership_simple_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/memory")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !11, !12}
!9 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 5, type: !10)
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!11 = !DILocalVariable(name: "b", scope: !4, file: !1, line: 10, type: !10)
!12 = !DILocalVariable(name: "v", scope: !4, file: !1, line: 19, type: !10)
!13 = !DILocation(line: 2, column: 1, scope: !4)
!14 = !DILocation(line: 5, column: 5, scope: !4)
!15 = !DILocation(line: 10, column: 1, scope: !4)
!16 = !DILocation(line: 19, column: 1, scope: !4)
