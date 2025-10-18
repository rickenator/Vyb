; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [43 x i8] c"=== Testing borrow() and view() syntax ===\00", align 1
@type_name = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@1 = private unnamed_addr constant [15 x i8] c"Created x = 42\00", align 1
@type_name.1 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@2 = private unnamed_addr constant [34 x i8] c"Modified x through pointer to 100\00", align 1
@type_name.2 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@3 = private unnamed_addr constant [14 x i8] c"Final value: \00", align 1

define i64 @main() !dbg !4 {
entry:
  %p = alloca ptr, align 8, !dbg !12
  %x = alloca i64, align 8, !dbg !12
  %serialize_temp = alloca { ptr, i64 }, align 8, !dbg !12
  store { ptr, i64 } { ptr @0, i64 42 }, ptr %serialize_temp, align 8, !dbg !12
  %serialized_json = call ptr @__vyn_serialize_to_json(ptr %serialize_temp, ptr @type_name), !dbg !12
  call void @__vyn_println(ptr %serialized_json), !dbg !12
  store i64 42, ptr %x, align 4, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %x, metadata !9, metadata !DIExpression()), !dbg !13
  %serialize_temp1 = alloca { ptr, i64 }, align 8, !dbg !12
  store { ptr, i64 } { ptr @1, i64 14 }, ptr %serialize_temp1, align 8, !dbg !12
  %serialized_json2 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp1, ptr @type_name.1), !dbg !12
  call void @__vyn_println(ptr %serialized_json2), !dbg !12
  %x3 = load i64, ptr %x, align 4, !dbg !12
  %loc_alloca = alloca i64, align 8, !dbg !12
  store i64 %x3, ptr %loc_alloca, align 4, !dbg !12
  store ptr %loc_alloca, ptr %p, align 8, !dbg !12
  call void @llvm.dbg.declare(metadata ptr %p, metadata !10, metadata !DIExpression()), !dbg !14
  %ptr_load = load ptr, ptr %p, align 8, !dbg !12
  %ptr.is_not_null = icmp ne ptr %ptr_load, null, !dbg !12
  br i1 %ptr.is_not_null, label %ptr.not_null, label %ptr.null, !dbg !12

ptr.not_null:                                     ; preds = %entry
  br label %ptr.merge, !dbg !12

ptr.null:                                         ; preds = %entry
  unreachable, !dbg !12

ptr.merge:                                        ; preds = %ptr.not_null
  store i64 100, ptr %ptr_load, align 4, !dbg !12
  %serialize_temp4 = alloca { ptr, i64 }, align 8, !dbg !12
  store { ptr, i64 } { ptr @2, i64 33 }, ptr %serialize_temp4, align 8, !dbg !12
  %serialized_json5 = call ptr @__vyn_serialize_to_json(ptr %serialize_temp4, ptr @type_name.2), !dbg !12
  call void @__vyn_println(ptr %serialized_json5), !dbg !12
  %x6 = load i64, ptr %x, align 4, !dbg !12
  %tostring = call ptr @__vyn_toString_int(i64 %x6), !dbg !12
  %strcattmp = call ptr @__vyn_string_concat(ptr @3, ptr %tostring), !dbg !12
  call void @__vyn_println(ptr %strcattmp), !dbg !12
  %x7 = load i64, ptr %x, align 4, !dbg !12
  ret i64 %x7, !dbg !12
}

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare void @__vyn_println(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare ptr @__vyn_toString_int(i64)

declare ptr @__vyn_string_concat(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "Vyn Compiler", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "syntax_test.vyn.ll", directory: "/home/rick/Projects/Vyn/test/memory")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "main", linkageName: "main", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !8)
!5 = !DISubroutineType(types: !6)
!6 = !{!7}
!7 = !DIBasicType(name: "i64", size: 64, encoding: DW_ATE_signed)
!8 = !{!9, !10}
!9 = !DILocalVariable(name: "x", scope: !4, file: !1, line: 5, type: !7)
!10 = !DILocalVariable(name: "p", scope: !4, file: !1, line: 10, type: !11)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!12 = !DILocation(line: 2, column: 1, scope: !4)
!13 = !DILocation(line: 5, column: 1, scope: !4)
!14 = !DILocation(line: 10, column: 1, scope: !4)
